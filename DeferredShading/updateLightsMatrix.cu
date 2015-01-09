#define GLM_FORCE_CUDA
#include <cuda.h>
#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include <thrust/scan.h>
#include <thrust/execution_policy.h>
#include "glm.hpp"

#define BLOCK_SIZE				256

int *d_lightsMatrix, *d_lightsScanSum, *d_lightsMatrixCompact;
float *d_pla;
glm::vec3 *d_lightsProj;
cudaError_t err;
size_t size;

__device__ float calcLightRadius(float* l, float threshold)
{
	if (l[11] == 0.0f) return (threshold*l[3]*glm::max(glm::max(l[0],l[1]),l[2])-l[9])/l[10];
	else return (sqrt(-4*l[9]*l[11]+l[10]*l[10]+4*l[11]*threshold*l[3]*glm::max(glm::max(l[0],l[1]),l[2]))-l[10])/(2*l[11]);
}


__global__ void calcLightProj(float *d_pla, float threshold, glm::vec3 right, glm::mat4 m, glm::mat4 proj, glm::vec3* d_lightsProj, int w, int h, int nLights)
{
	int idx = blockIdx.x*blockDim.x+threadIdx.x;

	if (idx < nLights) {
		glm::vec4 c = glm::vec4(d_pla[idx*12+6], d_pla[idx*12+7], d_pla[idx*12+8],1.0);
		float r = calcLightRadius(&d_pla[idx*12], threshold);
	
		glm::vec4 p = c+r*glm::vec4(right,0.0);
		p.w=1.0f;

		glm::vec4 cp = proj*m*c;
		cp = cp/cp.w;
		cp.x = w*(cp.x+1)/2;
		cp.y = h*(cp.y+1)/2;
	
		glm::vec4 pp = proj*m*p;
		pp = pp/pp.w;
		pp.x = w*(pp.x+1)/2;
		pp.y = h*(pp.y+1)/2;
	
		float pRadius = glm::length(glm::vec2(cp.x,cp.y)-glm::vec2(pp.x,pp.y));

		d_lightsProj[idx] = glm::vec3(cp.x,cp.y,pRadius);
	}
}

__global__ void calcLightsMatrix(glm::vec3* lightsProj, int* lightsMatrix, int nLights, int gridRes, int lightsTile, int gLightsRow, int gLightsCol)
{
	int idx = blockIdx.x*blockDim.x+threadIdx.x;

	if (idx < nLights){
		glm::vec2 cp = glm::vec2(lightsProj[idx].x,lightsProj[idx].y);
		float pRadius = lightsProj[idx].z;

		float x1,x2,y1,y2;
		for (unsigned int j = 0; j < gLightsRow; j++) {
			for (unsigned int k = 0; k < gLightsCol; k++) {
				x1 = k*gridRes; x2 = (k+1)*gridRes;
				y1 = j*gridRes; y2 = (j+1)*gridRes;

				lightsMatrix[(j*gLightsCol+k)*lightsTile+idx] = 0;
				if (cp.x >= x1 && cp.x < x2) {
					if (cp.y >= y1 && cp.y < y2) { //center inside tile
						lightsMatrix[(j*gLightsCol+k)*lightsTile+idx] = 1;
					} 
					else if (cp.y > y2) { // down (r> y-y2)
						if (pRadius >= abs(cp.y-y2)) {
							lightsMatrix[(j*gLightsCol+k)*lightsTile+idx] = 1;
						}
					} 
					else if (cp.y < y1) { // up (r> y-y1)
						if (pRadius >= abs(cp.y-y1)) {
							lightsMatrix[(j*gLightsCol+k)*lightsTile+idx] = 1;
						}
					} 
				} 
				else if (cp.y >= y1 && cp.y < y2) { 
					if (cp.x < x1) { // left
						if (pRadius >= abs(cp.x-x1)) {
							lightsMatrix[(j*gLightsCol+k)*lightsTile+idx] = 1;
						}
					}
					else if (cp.x > x2) { // right
						if (pRadius >= abs(cp.x-x2)) {
							lightsMatrix[(j*gLightsCol+k)*lightsTile+idx] = 1;
						}
					}
				}
				else if (cp.x < x1 && cp.y < y1) { // upper-left
					if (pRadius >= sqrt((cp.x-x1)*(cp.x-x1)+(cp.y-y1)*(cp.y-y1))) {
						lightsMatrix[(j*gLightsCol+k)*lightsTile+idx] = 1;
					}
				} 
				else if (cp.x > x2 && cp.y < y1) { // upper-right
					if (pRadius >= sqrt((cp.x-x2)*(cp.x-x2)+(cp.y-y1)*(cp.y-y1))) {
						lightsMatrix[(j*gLightsCol+k)*lightsTile+idx] = 1;
					}
				}
				else if (cp.x < x1 && cp.y > y2) { // down-left
					if (pRadius >= sqrt((cp.x-x1)*(cp.x-x1)+(cp.y-y2)*(cp.y-y2))) {
						lightsMatrix[(j*gLightsCol+k)*lightsTile+idx] = 1;
					}
				}
				else if (cp.x > x2 && cp.y > y2) { // down-right
					if (pRadius >= sqrt((cp.x-x2)*(cp.x-x2)+(cp.y-y2)*(cp.y-y2))) {
						lightsMatrix[(j*gLightsCol+k)*lightsTile+idx] = 1;
					}
				}
			}
		}
	}
}

__global__ void preScanSum(int *lightsScanSum, int *lightsMatrix, int lightsScanSumLength, int lightsTile, int nLights)
{
	int idx = blockIdx.x*blockDim.x+threadIdx.x;

	if (idx < lightsScanSumLength) {
		for (int i = 0; i < nLights; ++i) {
			if (lightsMatrix[idx*lightsTile+i] == 1) lightsScanSum[idx]++;
		}
	}
}

__global__ void compactLightsMatrix(int* lightsScanSum, int* lightsMatrix, int* lightsMatrixCompact, int lightsTile, int lightsScanSumLength, int nLights)
{
	int idx = blockIdx.x*blockDim.x+threadIdx.x;
	if (idx < lightsScanSumLength) {
		int prev;
		if (idx == 0) prev = 0;
		else prev = lightsScanSum[idx-1];
		int act = lightsScanSum[idx];
		int off = 0;
		for (int i = 0; i < nLights; ++i) {
			if (lightsMatrix[idx*lightsTile+i] == 1) {
				lightsMatrixCompact[prev+off] = i;
				off++;
			}
		}
	}
}

extern "C" void initMemCUDA(int nLights, int lightsScanSumLength, int lightsMatrixLength)
{
	cudaMalloc((void **) &d_pla, nLights*12*sizeof(float)); //[r,g,b, i,i_,i__, x,y,z, con,lin,exp]
	cudaMalloc((void **) &d_lightsProj, nLights*sizeof(glm::vec3)); // [x,y,radius]
	cudaMalloc((void **) &d_lightsMatrix, lightsMatrixLength*sizeof(int));
}

extern "C" void freeMemCUDA()
{
	cudaFree(d_pla);;
	cudaFree(d_lightsProj);
	cudaFree(d_lightsMatrix);
}

extern "C" void launch_kernel(void* pointLightsArr, int nLights, float threshold, glm::vec3 right, float* gl_ModelViewMatrix, float* gl_ProjectionMatrix, int w, int h,
	int gLightsRow, int gLightsCol, int* lightsScanSum, int lightsScanSumLength, int* lightsMatrix, int lightsMatrixLength, int gridRes, int lightsTile, cudaGraphicsResource *lightsMatrixResource, cudaGraphicsResource *lightsScanSumResource)
{
	// Create modelview and projection matrix
	glm::mat4 m = glm::mat4(gl_ModelViewMatrix[0],gl_ModelViewMatrix[1],gl_ModelViewMatrix[2],gl_ModelViewMatrix[3],gl_ModelViewMatrix[4],gl_ModelViewMatrix[5],gl_ModelViewMatrix[6],gl_ModelViewMatrix[7],gl_ModelViewMatrix[8],gl_ModelViewMatrix[9],gl_ModelViewMatrix[10],gl_ModelViewMatrix[11],gl_ModelViewMatrix[12],gl_ModelViewMatrix[13],gl_ModelViewMatrix[14],gl_ModelViewMatrix[15]);
	glm::mat4 proj = glm::mat4(gl_ProjectionMatrix[0],gl_ProjectionMatrix[1],gl_ProjectionMatrix[2],gl_ProjectionMatrix[3],gl_ProjectionMatrix[4],gl_ProjectionMatrix[5],gl_ProjectionMatrix[6],gl_ProjectionMatrix[7],gl_ProjectionMatrix[8],gl_ProjectionMatrix[9],gl_ProjectionMatrix[10],gl_ProjectionMatrix[11],gl_ProjectionMatrix[12],gl_ProjectionMatrix[13],gl_ProjectionMatrix[14],gl_ProjectionMatrix[15]);

	// Update lights and initialize scan sum _d_lightScanSum, _d_lightsMatrix;
	cudaGraphicsResourceGetMappedPointer((void**) &d_lightsScanSum, &size, lightsScanSumResource);
	cudaGraphicsResourceGetMappedPointer((void**) &d_lightsMatrixCompact, &size, lightsMatrixResource);
	cudaMemcpy(d_pla,pointLightsArr, nLights*12*sizeof(float),cudaMemcpyHostToDevice);
	cudaMemset(d_lightsScanSum, 0, lightsScanSumLength*sizeof(int));

	// Begin kernell calls
	int nBlocks = glm::ceil((float)nLights/BLOCK_SIZE);
	
	calcLightProj<<<nBlocks,BLOCK_SIZE>>>(d_pla, threshold, right, m, proj, d_lightsProj, w, h, nLights); 

	calcLightsMatrix<<<nBlocks,BLOCK_SIZE>>>(d_lightsProj, d_lightsMatrix, nLights, gridRes, lightsTile, gLightsRow, gLightsCol); 

	nBlocks = glm::ceil((float)lightsScanSumLength/BLOCK_SIZE);

	preScanSum<<<nBlocks,BLOCK_SIZE>>>(d_lightsScanSum, d_lightsMatrix, lightsScanSumLength, lightsTile, nLights);

	thrust::inclusive_scan(thrust::device, d_lightsScanSum, d_lightsScanSum + lightsScanSumLength, d_lightsScanSum);

	compactLightsMatrix<<<nBlocks,BLOCK_SIZE>>>(d_lightsScanSum, d_lightsMatrix, d_lightsMatrixCompact, lightsTile, lightsScanSumLength, nLights); 
}