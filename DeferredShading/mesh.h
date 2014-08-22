#include <vector>
#include <QtOpenGL>

#include "scene.h"
#include "texture.h"

#define INVALID_BUFFER -1

// Vector structs (TODO: move to a more suitable location)
struct Vector3f
{
    float x;
    float y;
    float z;
    Vector3f(){}
    Vector3f(float _x, float _y, float _z)
    {
        x = _x;
        y = _y;
        z = _z;
    }
};

struct Vector2f
{
    float x;
    float y;
    Vector2f(){}
    Vector2f(float _x, float _y)
    {
        x = _x;
        y = _y;
    }
};

// Vertex structure, contains information about the position, texture coordinates and normal of a vertex
struct Vertex
{
    Vector3f m_pos;
    Vector2f m_tex;
    Vector3f m_norm;

    Vertex(){}

    Vertex(const Vector3f& pos, const Vector2f& tex, const Vector3f& norm)
    {
        m_pos = pos;
        m_tex = tex;
        m_norm = norm;
    }
};

// Mesh class, contains info and methods to load an render meshes
class Mesh
{
public:
    Mesh();

    ~Mesh();

    bool LoadMesh(const std::string& Filename);

    void Render(GLuint SP);

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
};
