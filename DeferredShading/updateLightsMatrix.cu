#define GLM_FORCE_CUDA
#include <cuda.h>
#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include <thrust/scan.h>
#include <thrust/execution_policy.h>
#include "glm.hpp"

#define BLOCK_SIZE				256

int *__d_res, *__d_nLights, *__d_lightsScanSum, *__d_lightsMatrix, *__d_gridRes, *__d_lightsTile, *__d_gLightsCol, *__d_lightsMatrixCompact, *__d_lightsScanSumLength;
float *__d_pla, *__d_threshold;
glm::mat4 *__d_m, *__d_proj;
glm::vec3 *__d_lightsProj, *__d_r;

__device__ float calcLightRadius(float* l, float threshold)
{
	if (l[11] == 0.0f) return (threshold*l[3]*glm::max(glm::max(l[0],l[1]),l[2])-l[9])/l[10];
	else return (sqrt(-4*l[9]*l[11]+l[10]*l[10]+4*l[11]*threshold*l[3]*glm::max(glm::max(l[0],l[1]),l[2]))-l[10])/(2*l[11]);
}


__global__ void calcLightProj(float *d_pla, float* d_threshold, glm::vec3* d_r, glm::mat4* d_m, glm::mat4* d_proj, glm::vec3* d_lightsProj, int* d_res, int* nLights)
{
	int idx = blockIdx.x*blockDim.x+threadIdx.x;
	int n = *nLights;

	if (idx < n) {
		glm::vec4 c = glm::vec4(d_pla[idx*12+6], d_pla[idx*12+7], d_pla[idx*12+8],1.0);
		float r = calcLightRadius(&d_pla[idx*12], *d_threshold);
	
		glm::vec4 p = c+r*glm::vec4(*d_r,0.0);
		p.w=1.0f;

		glm::mat4 m = *d_m;
		glm::mat4 proj = *d_proj;
	
		glm::vec4 cp = proj*m*c;
		cp = cp/cp.w;
		cp.x = d_res[0]*(cp.x+1)/2;
		cp.y = d_res[1]*(cp.y+1)/2;
	
		glm::vec4 pp = proj*m*p;
		pp = pp/pp.w;
		pp.x = d_res[0]*(pp.x+1)/2;
		pp.y = d_res[1]*(pp.y+1)/2;
	
		float pRadius = glm::length(glm::vec2(cp.x,cp.y)-glm::vec2(pp.x,pp.y));

		d_lightsProj[idx] = glm::vec3(cp.x,cp.y,pRadius);
	}
}

__global__ void calcLightsMatrix(glm::vec3* lightsProj, int* lightsScanSum, int* lightsMatrix, int* _nLights, int* _gridRes, int* _lightsTile, int* _gLightsCol)
{
	int gridRes = *_gridRes;
	int lightsTile = *_lightsTile;
	int gLightsCol = *_gLightsCol;
	int nLights = *_nLights;

	int j = blockIdx.x;
	int k = blockIdx.y;
	int x1 = k*gridRes; int x2 = (k+1)*gridRes;
	int y1 = j*gridRes; int y2 = (j+1)*gridRes;
	int off;
	for (int i = 0; i < nLights ; ++i)
	{
		glm::vec2 cp = glm::vec2(lightsProj[i].x,lightsProj[i].y);
		float pRadius = lightsProj[i].z;
		if (cp.x >= x1 && cp.x < x2) {
			if (cp.y >= y1 && cp.y < y2) { //center inside tile
				off = lightsScanSum[j*gLightsCol+k]++; // comprovacio que no ens pasem de max num ligths per tile
				lightsMatrix[(j*gLightsCol+k)*lightsTile+off] = i;
			} 
			else if (cp.y > y2) { // down (r> y-y2)
				if (pRadius >= abs(cp.y-y2)) {
					off = lightsScanSum[j*gLightsCol+k]++;
					lightsMatrix[(j*gLightsCol+k)*lightsTile+off] = i;
				}
			} 
			else if (cp.y < y1) { // up (r> y-y1)
				if (pRadius >= abs(cp.y-y1)) {
					off = lightsScanSum[j*gLightsCol+k]++;
					lightsMatrix[(j*gLightsCol+k)*lightsTile+off] = i;
				}
			} 
		} 
		else if (cp.y >= y1 && cp.y < y2) { 
			if (cp.x < x1) { // left
				if (pRadius >= abs(cp.x-x1)) {
					off = lightsScanSum[j*gLightsCol+k]++;
					lightsMatrix[(j*gLightsCol+k)*lightsTile+off] = i;
				}
			}
			else if (cp.x > x2) { // right
				if (pRadius >= abs(cp.x-x2)) {
					off = lightsScanSum[j*gLightsCol+k]++;
					lightsMatrix[(j*gLightsCol+k)*lightsTile+off] = i;
				}
			}
		}
		else if (cp.x < x1 && cp.y < y1) { // upper-left
			if (pRadius >= sqrt((cp.x-x1)*(cp.x-x1)+(cp.y-y1)*(cp.y-y1))) {
				off = lightsScanSum[j*gLightsCol+k]++;
				lightsMatrix[(j*gLightsCol+k)*lightsTile+off] = i;
			}
		} 
		else if (cp.x > x2 && cp.y < y1) { // upper-right
			if (pRadius >= sqrt((cp.x-x2)*(cp.x-x2)+(cp.y-y1)*(cp.y-y1))) {
				off = lightsScanSum[j*gLightsCol+k]++;
				lightsMatrix[(j*gLightsCol+k)*lightsTile+off] = i;
			}
		}
		else if (cp.x < x1 && cp.y > y2) { // down-left
			if (pRadius >= sqrt((cp.x-x1)*(cp.x-x1)+(cp.y-y2)*(cp.y-y2))) {
				off = lightsScanSum[j*gLightsCol+k]++;
				lightsMatrix[(j*gLightsCol+k)*lightsTile+off] = i;
			}
		}
		else if (cp.x > x2 && cp.y > y2) { // down-right
			if (pRadius >= sqrt((cp.x-x2)*(cp.x-x2)+(cp.y-y2)*(cp.y-y2))) {
				off = lightsScanSum[j*gLightsCol+k]++;
				lightsMatrix[(j*gLightsCol+k)*lightsTile+off] = i;
			}
		}
	}
}

__global__ void compactLightsMatrix(int* lightsScanSum, int* lightsMatrix, int* lightsMatrixCompact, int* _lightsTile, int* _lightsScanSumLength)
{
	int lightsScanSumLength = *_lightsScanSumLength;
	int idx = blockIdx.x*blockDim.x+threadIdx.x;
	if (idx < lightsScanSumLength) {
		int lightsTile = *_lightsTile;
		int prev;
		if (idx == 0) prev = 0;
		else prev = lightsScanSum[idx-1];
		int act = lightsScanSum[idx];
		for (int i = 0; i < act-prev; ++i) {
			lightsMatrixCompact[prev+i] = lightsMatrix[idx*lightsTile+i];
		}
	}
}

extern "C" void initMemCUDA(int **d_res, int **d_nLights, int **d_lightsScanSum, int **d_lightsMatrix, int **d_gridRes, int **d_lightsTile, int **d_gLightsCol, int **d_lightsMatrixCompact, int **d_lightsScanSumLength,
	float **d_pla, float **d_threshold,
	glm::mat4 **d_m, glm::mat4 **d_proj,
	glm::vec3 **d_lightsProj, glm::vec3 **d_r,
	int nLights, int lightsScanSumLength, int lightsMatrixLength)
{
	cudaError_t err;
	cudaMalloc((void **) d_pla, nLights*12*sizeof(float)); //[r,g,b, i,i_,i__, x,y,z, con,lin,exp]
	cudaMalloc((void **) d_threshold, sizeof(float));
	cudaMalloc((void **) d_r, sizeof(glm::vec3));
	cudaMalloc((void **) d_m, sizeof(glm::mat4));
	cudaMalloc((void **) d_proj, sizeof(glm::mat4));
	cudaMalloc((void **) d_lightsProj, nLights*sizeof(glm::vec3)); // [x,y,radius]
	cudaMalloc((void **) d_res, 2*sizeof(int));
	cudaMalloc((void **) d_nLights, sizeof(int));
	cudaMalloc((void **) d_gridRes, sizeof(int));
	cudaMalloc((void **) d_lightsTile, sizeof(int));
	cudaMalloc((void **) d_gLightsCol, sizeof(int));
	cudaMalloc((void **) d_lightsScanSumLength, sizeof(int));
	cudaMalloc((void **) d_lightsScanSum, lightsScanSumLength*sizeof(int));
	cudaMalloc((void **) d_lightsMatrix, lightsMatrixLength*sizeof(int));
	cudaMalloc((void **) d_lightsMatrixCompact, lightsMatrixLength*sizeof(int));

	err=cudaMalloc((void **) &__d_pla, nLights*12*sizeof(float)); //[r,g,b, i,i_,i__, x,y,z, con,lin,exp]
	err=cudaMalloc((void **) &__d_threshold, sizeof(float));
	err=cudaMalloc((void **) &__d_r, sizeof(glm::vec3));
	err=cudaMalloc((void **) &__d_m, sizeof(glm::mat4));
	err=cudaMalloc((void **) &__d_proj, sizeof(glm::mat4));
	err=cudaMalloc((void **) &__d_lightsProj, nLights*sizeof(glm::vec3)); // [x,y,radius]
	err=cudaMalloc((void **) &__d_res, 2*sizeof(int));
	err=cudaMalloc((void **) &__d_nLights, sizeof(int));
	err=cudaMalloc((void **) &__d_gridRes, sizeof(int));
	err=cudaMalloc((void **) &__d_lightsTile, sizeof(int));
	err=cudaMalloc((void **) &__d_gLightsCol, sizeof(int));
	err=cudaMalloc((void **) &__d_lightsScanSumLength, sizeof(int));
	err=cudaMalloc((void **) &__d_lightsScanSum, lightsScanSumLength*sizeof(int));
	err=cudaMalloc((void **) &__d_lightsMatrix, lightsMatrixLength*sizeof(int));
	err=cudaMalloc((void **) &__d_lightsMatrixCompact, lightsMatrixLength*sizeof(int));
}

extern "C" void freeMemCUDA(int *d_res, int *d_nLights, int *d_lightsScanSum, int *d_lightsMatrix, int *d_gridRes, int *d_lightsTile, int *d_gLightsCol, int *d_lightsMatrixCompact, int *d_lightsScanSumLength,
	float *d_pla, float *d_threshold,
	glm::mat4 *d_m, glm::mat4 *d_proj,
	glm::vec3 *d_lightsProj, glm::vec3 *d_r)
{
	cudaFree(d_pla);
	cudaFree(d_threshold);
	cudaFree(d_r);
	cudaFree(d_m);
	cudaFree(d_proj);
	cudaFree(d_res);
	cudaFree(d_lightsProj);
	cudaFree(d_nLights);
	cudaFree(d_gridRes);
	cudaFree(d_gLightsCol);
	cudaFree(d_lightsScanSumLength);
	cudaFree(d_lightsMatrixCompact);
	cudaFree(d_lightsTile);
	cudaFree(d_lightsScanSum);
	cudaFree(d_lightsMatrix);
}

extern "C" void launch_kernel(void* pointLightsArr, int nLights, float threshold, glm::vec3* right, float* gl_ModelViewMatrix, float* gl_ProjectionMatrix, int w, int h,
	int gLightsRow, int gLightsCol, int* lightsScanSum, int lightsScanSumLength, int* lightsMatrix, int lightsMatrixLength, int gridRes, int lightsTile, int *d_lightsScanSumLength,
	int *d_res, int *d_nLights, int *d_lightsScanSum, int *d_lightsMatrix, int *d_gridRes, int *d_lightsTile, int *d_gLightsCol, int *d_lightsMatrixCompact,
	float *d_pla, float *d_threshold,
	glm::mat4 *d_m, glm::mat4 *d_proj,
	glm::vec3 *d_lightsProj, glm::vec3 *d_r)
{
	
	int *_d_res, *_d_nLights, *_d_lightsScanSum, *_d_lightsMatrix, *_d_gridRes, *_d_lightsTile, *_d_gLightsCol, *_d_lightsMatrixCompact, *_d_lightsScanSumLength;
	float *_d_pla, *_d_threshold;
	glm::mat4 *_d_m, *_d_proj;
	glm::vec3 *_d_lightsProj, *_d_r;
	
	// Calc lights projection
	glm::mat4 m = glm::mat4(gl_ModelViewMatrix[0],gl_ModelViewMatrix[1],gl_ModelViewMatrix[2],gl_ModelViewMatrix[3],gl_ModelViewMatrix[4],gl_ModelViewMatrix[5],gl_ModelViewMatrix[6],gl_ModelViewMatrix[7],gl_ModelViewMatrix[8],gl_ModelViewMatrix[9],gl_ModelViewMatrix[10],gl_ModelViewMatrix[11],gl_ModelViewMatrix[12],gl_ModelViewMatrix[13],gl_ModelViewMatrix[14],gl_ModelViewMatrix[15]);
	glm::mat4 proj = glm::mat4(gl_ProjectionMatrix[0],gl_ProjectionMatrix[1],gl_ProjectionMatrix[2],gl_ProjectionMatrix[3],gl_ProjectionMatrix[4],gl_ProjectionMatrix[5],gl_ProjectionMatrix[6],gl_ProjectionMatrix[7],gl_ProjectionMatrix[8],gl_ProjectionMatrix[9],gl_ProjectionMatrix[10],gl_ProjectionMatrix[11],gl_ProjectionMatrix[12],gl_ProjectionMatrix[13],gl_ProjectionMatrix[14],gl_ProjectionMatrix[15]);
	int h_res[2] = {w,h};

	cudaMalloc((void **) &_d_pla, nLights*12*sizeof(float)); //[r,g,b, i,i_,i__, x,y,z, con,lin,exp]
	cudaMalloc((void **) &_d_threshold, sizeof(float));
	cudaMalloc((void **) &_d_r, sizeof(glm::vec3));
	cudaMalloc((void **) &_d_m, sizeof(glm::mat4));
	cudaMalloc((void **) &_d_proj, sizeof(glm::mat4));
	cudaMalloc((void **) &_d_lightsProj, nLights*sizeof(glm::vec3)); // [x,y,radius]
	cudaMalloc((void **) &_d_res, 2*sizeof(int));
	cudaMalloc((void **) &_d_nLights, sizeof(int));
	cudaMalloc((void **) &_d_gridRes, sizeof(int));
	cudaMalloc((void **) &_d_lightsTile, sizeof(int));
	cudaMalloc((void **) &_d_gLightsCol, sizeof(int));
	cudaMalloc((void **) &_d_lightsScanSumLength, sizeof(int));
	cudaMalloc((void **) &_d_lightsScanSum, lightsScanSumLength*sizeof(int));
	cudaMalloc((void **) &_d_lightsMatrix, lightsMatrixLength*sizeof(int));
	cudaMalloc((void **) &_d_lightsMatrixCompact, lightsMatrixLength*sizeof(int));

	cudaMemcpy(_d_pla,pointLightsArr, nLights*12*sizeof(float),cudaMemcpyHostToDevice);
	cudaMemcpy(_d_threshold,&threshold, sizeof(float),cudaMemcpyHostToDevice);
	cudaMemcpy(_d_r,right, sizeof(glm::vec3),cudaMemcpyHostToDevice);
	cudaMemcpy(_d_m,&m, sizeof(glm::mat4),cudaMemcpyHostToDevice);
	cudaMemcpy(_d_proj,&proj, sizeof(glm::mat4),cudaMemcpyHostToDevice);
	cudaMemcpy(_d_res,&h_res, sizeof(h_res),cudaMemcpyHostToDevice);
	cudaMemcpy(_d_nLights,&nLights, sizeof(int),cudaMemcpyHostToDevice);
	cudaMemcpy(_d_gridRes,&gridRes, sizeof(int),cudaMemcpyHostToDevice);
	cudaMemcpy(_d_lightsTile,&lightsTile, sizeof(int),cudaMemcpyHostToDevice);
	cudaMemcpy(_d_gLightsCol,&gLightsCol, sizeof(int),cudaMemcpyHostToDevice);
	cudaMemcpy(_d_lightsScanSum,lightsScanSum, lightsScanSumLength*sizeof(int),cudaMemcpyHostToDevice);
	cudaMemcpy(_d_lightsScanSumLength,&lightsScanSumLength, sizeof(int),cudaMemcpyHostToDevice);
	
	int nBlocks = glm::ceil((float)nLights/BLOCK_SIZE);
	
	calcLightProj<<<nBlocks,BLOCK_SIZE>>>(_d_pla, _d_threshold, _d_r, _d_m, _d_proj, _d_lightsProj, _d_res, _d_nLights); //O(1)
	
	calcLightsMatrix<<<dim3(gLightsRow, gLightsCol),1>>>(_d_lightsProj, _d_lightsScanSum, _d_lightsMatrix, _d_nLights, _d_gridRes, _d_lightsTile, _d_gLightsCol); //O(n)
	
	// Scan and compact
	thrust::inclusive_scan(thrust::device, _d_lightsScanSum, _d_lightsScanSum + lightsScanSumLength, _d_lightsScanSum);
	/*
	int ligtshMatrixCompactSize = thrust::reduce(thrust::device,_d_lightsScanSum,_d_lightsScanSum+lightsScanSumLength,-1,thrust::maximum<int>());

	cudaMalloc((void **) &_d_lightsMatrixCompact, ligtshMatrixCompactSize*sizeof(int));
	*/
	nBlocks = glm::ceil((float)lightsScanSumLength/BLOCK_SIZE);

	compactLightsMatrix<<<nBlocks,BLOCK_SIZE>>>(_d_lightsScanSum, _d_lightsMatrix, _d_lightsMatrixCompact, _d_lightsTile, _d_lightsScanSumLength); //O(1)

	cudaMemcpy(lightsScanSum ,_d_lightsScanSum, lightsScanSumLength*sizeof(int),cudaMemcpyDeviceToHost);
	cudaMemcpy(lightsMatrix ,_d_lightsMatrixCompact, lightsMatrixLength*sizeof(int),cudaMemcpyDeviceToHost);

	cudaFree(_d_pla);
	cudaFree(_d_threshold);
	cudaFree(_d_r);
	cudaFree(_d_m);
	cudaFree(_d_proj);
	cudaFree(_d_res);
	cudaFree(_d_lightsProj);
	cudaFree(_d_nLights);
	cudaFree(_d_gridRes);
	cudaFree(_d_gLightsCol);
	cudaFree(_d_lightsScanSumLength);
	cudaFree(_d_lightsMatrixCompact);
	cudaFree(_d_lightsTile);
	cudaFree(_d_lightsScanSum);
	cudaFree(_d_lightsMatrix);
}