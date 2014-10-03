#include "utils.h"
#include "scene.h"
#include "texture.h"

// Mesh class, contains info and methods to load an render meshes
class Mesh
{
public:
    Mesh();

    ~Mesh();

    bool LoadMesh(const std::string& Filename);
    void Render(GLuint pLoc, GLuint tcLoc, GLuint nLoc, GLuint sLoc);
	void simpleRender();
	BoundingBox getBoundingBox();

private:
    bool InitFromScene(const aiScene* pScene, const std::string& Filename);
    void InitMesh(unsigned int Index, const aiMesh* paiMesh);
    bool InitMaterials(const aiScene* pScene, const std::string& Filename);

    struct MeshEntry {
        MeshEntry();

        ~MeshEntry();

        void Init(const std::vector<Vertex>& Vertices, const std::vector<unsigned int>& Indices);

        GLuint VB;
        GLuint IB;
        unsigned int NumIndices;
        unsigned int MaterialIndex;
    };

    std::vector<MeshEntry> m_Entries;
    std::vector<Texture*> m_Textures;
	BoundingBox bb;
};
