#include "mesh.h"
#include "Importer.hpp"
#include "postprocess.h"

Mesh::MeshEntry::MeshEntry()
{
    VB = INVALID_BUFFER;
    IB = INVALID_BUFFER;
    NumIndices = 0;
    MaterialIndex = INVALID_BUFFER;
}

Mesh::MeshEntry::~MeshEntry()
{
    // Clean buffers
    QGLFunctions glFuncs(QGLContext::currentContext());
    if (VB != INVALID_BUFFER) glFuncs.glDeleteBuffers(1,&VB);
    if (IB != INVALID_BUFFER) glFuncs.glDeleteBuffers(1,&IB);
}

void Mesh::MeshEntry::Init(const std::vector<Vertex> &Vertices, const std::vector<unsigned int>& Indices)
{
    NumIndices = Indices.size();

    // Populate the array buffer BV
    QGLFunctions glFuncs(QGLContext::currentContext());
    glFuncs.glGenBuffers(1,&VB);
    glFuncs.glBindBuffer(GL_ARRAY_BUFFER,VB);
    glFuncs.glBufferData(GL_ARRAY_BUFFER,Vertices.size()*sizeof(Vertex),&Vertices[0],GL_STATIC_DRAW);

    // Populate the index buffer IB
    glFuncs.glGenBuffers(1,&IB);
    glFuncs.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,IB);
    glFuncs.glBufferData(GL_ELEMENT_ARRAY_BUFFER,NumIndices*sizeof(unsigned int),&Indices[0],GL_STATIC_DRAW);

}

Mesh::Mesh()
{
	bb = BoundingBox(Vector3f(-FLT_MAX,-FLT_MAX,-FLT_MAX),Vector3f(FLT_MAX,FLT_MAX,FLT_MAX));
}

bool Mesh::LoadMesh(const std::string& Filename)
{
    // Import data from file using ASSIMP
    bool ret = false;
    Assimp::Importer Importer;

    const aiScene* pScene = Importer.ReadFile(Filename.c_str(), aiProcess_Triangulate|aiProcess_GenSmoothNormals|aiProcess_FlipUVs);

    // Initialitzation of the Mesh from the data collected by ASSIMP
    if (pScene) ret = InitFromScene(pScene,Filename);

    return ret;
}

bool Mesh::InitFromScene(const aiScene *pScene, const std::string &Filename)
{
    // Resize the meshes and textures vectors
    m_Entries.resize(pScene->mNumMeshes);
    m_Textures.resize(pScene->mNumMaterials);

    // Initialitzation of all the meshes of the scene
    for (unsigned int i = 0; i < m_Entries.size(); ++i) {
        const aiMesh* paiMesh = pScene->mMeshes[i];
        InitMesh(i,paiMesh);
    }

    // Initialitzation of the materials for the scene
    return InitMaterials(pScene,Filename);
}

void Mesh::InitMesh(unsigned int Index, const aiMesh *paiMesh)
{
    // Material index
    m_Entries[Index].MaterialIndex = paiMesh->mMaterialIndex;

    //Vertices and indices vectors we will use to populate the buffer for this mesh entry
    std::vector<Vertex> Vertices;
    std::vector<unsigned int> Indices;

	aiVector3D Zeros = aiVector3D(0.0f,0.0f,0.0f);

    // For each vertex: position, normal and texture coordinates are stored as a Vertex at the Vertices vector
    for (unsigned int i = 0; i < paiMesh->mNumVertices; ++i) {
        const aiVector3D* pPos      = &(paiMesh->mVertices[i]);
        const aiVector3D* pNormal   = &(paiMesh->mNormals[i]);
        const aiVector3D* pTexCoord = paiMesh->HasTextureCoords(0) ? &(paiMesh->mTextureCoords[0][i]) : &Zeros;

        Vertex v(Vector3f(pPos->x,pPos->y,pPos->z),
                 Vector2f(pTexCoord->x,pTexCoord->y),
                 Vector3f(pNormal->x,pNormal->y,pNormal->z));

        Vertices.push_back(v);
		
		if (pPos->x > bb.max.x) bb.max.x = pPos->x;
		if (pPos->y > bb.max.y) bb.max.y = pPos->y;
		if (pPos->z > bb.max.z) bb.max.z = pPos->z;
		if (pPos->x < bb.min.x) bb.min.x = pPos->x;
		if (pPos->y < bb.min.y) bb.min.y = pPos->y;
		if (pPos->z < bb.min.z) bb.min.z = pPos->z;
		
    }

    // Indices for each face are stored at the Indices vector
    for (unsigned int i = 0; i < paiMesh->mNumFaces; ++i) {
        const aiFace& Face = paiMesh->mFaces[i];
        // Each face must have 3 indices (we forced assimp triangulation), otherwise it is descarted
        if (Face.mNumIndices == 3) {
			Indices.push_back(Face.mIndices[0]);
			Indices.push_back(Face.mIndices[1]);
			Indices.push_back(Face.mIndices[2]);
		}
    }

    // Finally we must initialize the buffer for the mesh entry with the data from Vertices and Indices
    m_Entries[Index].Init(Vertices,Indices);
}

bool Mesh::InitMaterials(const aiScene* pScene, const std::string& Filename){
   
    std::string Dir;
	utils::getDir(Filename,&Dir);

    bool Ret = true;

	for (unsigned int i = 0; i < pScene->mNumMaterials; i++) {
		const aiMaterial* pMaterial = pScene->mMaterials[i];
		m_Textures[i] = NULL;
		// We will only use the diffuse component of the material
		if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
			aiString Path;
			if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
				std::string FullPath = Dir + "/" + Path.data;
				m_Textures[i] = new Texture(GL_TEXTURE_2D, FullPath.c_str());
				
				if (!m_Textures[i]->Load()) {
					delete m_Textures[i];
					m_Textures[i] = NULL;
				}
			}
		}
	}
	return Ret;
}

void Mesh::Render(GLuint pLoc, GLuint tcLoc, GLuint nLoc, GLuint sLoc)
{
	QGLFunctions glFuncs(QGLContext::currentContext());

    glFuncs.glEnableVertexAttribArray(pLoc);
    glFuncs.glEnableVertexAttribArray(tcLoc);
    glFuncs.glEnableVertexAttribArray(nLoc);

    for (unsigned int i = 0; i < m_Entries.size(); ++i)
    {
        glFuncs.glBindBuffer(GL_ARRAY_BUFFER,m_Entries[i].VB);
        glFuncs.glVertexAttribPointer(pLoc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
		glFuncs.glVertexAttribPointer(tcLoc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)12);
		glFuncs.glVertexAttribPointer(nLoc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)20);

        glFuncs.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,m_Entries[i].IB);

		const unsigned int materialIndex = m_Entries[i].MaterialIndex;

		if (materialIndex < m_Textures.size() && m_Textures[materialIndex]) {
			m_Textures[materialIndex]->Bind(GL_TEXTURE0);
		}

		glFuncs.glUniform1i(sLoc, 0);

        glDrawElements(GL_TRIANGLES, m_Entries[i].NumIndices, GL_UNSIGNED_INT,0);
    }

    glFuncs.glDisableVertexAttribArray(pLoc);
	glFuncs.glDisableVertexAttribArray(tcLoc);
	glFuncs.glDisableVertexAttribArray(nLoc);
}

BoundingBox Mesh::getBoundingBox()
{
	return bb;
}