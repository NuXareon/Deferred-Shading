#pragma once
#include <glew.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <random>
#include <qset.h>
#include <qevent.h>
#include <QGLWidget>
#include <QTime>
#include "glext.h"
#ifdef _WIN32
#include "wglext.h"
#elif __linux__
#include "glxext.h"
#endif

#define INVALID_BUFFER -		1
#define N_MAX_LIGHTS			100
#define	INITIAL_LIGHTS			20
#define	GBUFFER_DEFAULT			0
#define GBUFFER_READ			1
#define GBUFFER_READ_TEX		2
#define GBUFFER_DRAW			3
#define LIGHT_THRESHOLD			256		// 256 = 8 bit color channel
#define ATTENUATION_CONSTANT	1.0
#define ATTENUATION_LINEAR		60.0
#define ATTENUATION_EXP			0.0

// Vector structs
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

struct BoundingBox
{
	Vector3f max;
	Vector3f min;
	BoundingBox(){}
	BoundingBox (Vector3f _max, Vector3f _min)
	{
		max = _max;
		min = _min;
	}
	BoundingBox operator*(const float i) const
	{
		return BoundingBox(Vector3f(max.x*i,max.y*i,max.z*i), Vector3f(min.x*i,min.y*i,min.z*i));
	}
};

struct ambientLight{
	struct {
		float r; float g; float b;
	} color;
	float intensity;
	ambientLight(){}
	ambientLight(float c[3], float i)
	{
		color.r = c[0]; color.g = c[1]; color.b = c[2];
		intensity = i;
	}
};

struct directionalLight{
		struct {
		float r; float g; float b;
	} color;
	float intensity;
	struct {
		float x; float y; float z;
	} direction;
	directionalLight(){}
	directionalLight(float c[3], float i, float d[3])
	{
		color.r = c[0]; color.g = c[1]; color.b = c[2];
		intensity = i;
		direction.x = d[0]; direction.y = d[1]; direction.z = d[2];
	}
};

// Contains infromation about the color, intensity, position and attenuation of a point light.
struct pointLight{
	struct {
		float r; float g; float b;
	} color;
	float intensity;
	struct {
		float x; float y; float z;
	} position;
	struct {
		float constant; float linear; float exp;
	} attenuation;
	pointLight(){};
	pointLight(float c[3], float i, float p[3], float a[3])
	{
		color.r = c[0]; color.g = c[1]; color.b = c[2];
		intensity = i;
		position.x = p[0]; position.y = p[1]; position.z = p[2];
		attenuation.constant = a[0]; attenuation.linear = a[1]; attenuation.exp = a[2];
	}
};

enum renderModeType {RENDER_POSITION, RENDER_NORMAL, RENDER_DIFFUSE, RENDER_ALL, RENDER_FORWARD, RENDER_DEFERRED};

class utils
{
public:
	static void getDir(const std::string path, std::string* dir);
	static bool readFile(const char* path, std::string& out);
	static void enableVSyncWin(int i);
	static void enableVSyncLinux(int i);
	static bool checkExtension(char* ext);
	static void drawSphere(double r, int lats, int longs);
	static float calcLightRadius(pointLight l, float threshold);
};