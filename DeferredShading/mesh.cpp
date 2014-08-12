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
}

bool Mesh::LoadMesh(const std::string& Filename)
{
    // Import data from file using ASSIMP
    bool ret = false;
    Assimp::Importer Importer;

    const aiScene* pScene = Importer.ReadFile(Filename.c_str(), aiProcess_Triangulate|aiProcess_GenSmoothNormals);

    // Initialitzation of the Mesh from the data collected by ASSIMP
    if (pScene) ret = InitFromScene(pScene,Filename);

    return ret;
}

bool Mesh::InitFromScene(const aiScene *pScene, const std::string &Filename)
{
    // Resize the meshes and textures vectors
    m_Entries.resize(pScene->mNumMeshes);
   // m_Textures.resize(pScene->mNumMaterials);

    // Initialitzation of all the meshes of the scene
    for (unsigned int i = 0; i < m_Entries.size(); ++i) {
        const aiMesh* paiMesh = pScene->mMeshes[i];
        InitMesh(i,paiMesh);
    }

    // Initialitzation of the materials for the scene
    return true; //InitMaterials(pScene,Filename);
}

void Mesh::InitMesh(unsigned int Index, const aiMesh *paiMesh)
{
    // Material index
    m_Entries[Index].MaterialIndex = paiMesh->mMaterialIndex;

    //Vertices and indices vectors we will use to populate the buffer for this mesh entry
    std::vector<Vertex> Vertices;
    std::vector<unsigned int> Indices;

	aiVector3D Zeros = aiVector3D(0.0f,0.0f,0.0f);

    // For each vertex, position, normal and texture coordinates are stored as a Vertex at the Vertices vector
    for (unsigned int i = 0; i < paiMesh->mNumVertices; ++i) {
        const aiVector3D* pPos      = &(paiMesh->mVertices[i]);
        const aiVector3D* pNormal   = &(paiMesh->mNormals[i]);
        const aiVector3D* pTexCoord = paiMesh->HasTextureCoords(0) ? &(paiMesh->mTextureCoords[0][i]) : &Zeros;

        Vertex v(Vector3f(pPos->x,pPos->y,pPos->z),
                 Vector2f(pTexCoord->x,pTexCoord->y),
                 Vector3f(pNormal->x,pNormal->y,pNormal->z));

        Vertices.push_back(v);
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

void Mesh::Render(GLuint SP)
{
	QGLFunctions glFuncs(QGLContext::currentContext());
    GLuint positionLocation = glFuncs.glGetAttribLocation(SP,"position");

    glFuncs.glEnableVertexAttribArray(positionLocation);
    //glEnableVertexAttribArray(1);
    //glEnableVertexAttribArray(2);

    for (unsigned int i = 0; i < m_Entries.size(); ++i)
    {
        glFuncs.glBindBuffer(GL_ARRAY_BUFFER,m_Entries[i].VB);
        glFuncs.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);

        glFuncs.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,m_Entries[i].IB);

        // Set texture here //

        glDrawElements(GL_TRIANGLES, m_Entries[i].NumIndices, GL_UNSIGNED_INT,0);
    }

    glFuncs.glDisableVertexAttribArray(positionLocation);
}
