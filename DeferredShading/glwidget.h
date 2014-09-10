#include <QGLWidget>
#include <QTime>
#include "mesh.h"

#define N_MAX_LIGHTS	100
#define	INITIAL_LIGHTS	20

// NOTE: SHOULD MOVE STRUCT TO A MORE SUITABLE LOCATION
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

class GLWidget : public QGLWidget
{
    Q_OBJECT

public:
    GLWidget(QWidget *parent = 0);
    ~GLWidget();

public slots:
	// Keyboard
	void addKey(int k);
	void removeKey(int k);
	// Camera
	void modifyCameraSensitivity(QString s);
	void modifyCameraSpeed(QString s);
	// Misc.
	void loadModel(std::string path);					// Loads a model located on path.
	void genLightning(int n);							// Modifies nLights and regenerates lightning, see initializeLightning().

signals:
	void updateFPSSignal(int fps);

protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
	void timerEvent(QTimerEvent* event);

private:
	// Keyboard
	QSet<int> keys;
	// Camera
	float xPos, yPos, zPos;
	float alpha, beta;
	float cSpeed, cSensitivity;
    QPoint lastPos;
	// Mesh info
	Mesh *mainMesh;
	std::string modelPath;
	GLuint shaderProgram;
	// Timers
	QTime* t;
	int fps, frames;
	int inputTimerId, drawTimerId;
	// Shader atribute and uniform locations
	GLuint positionLocation;
	GLuint texCoordLocation;
	GLuint normLocation;
	GLuint samplerLocation;
	GLuint ambientColorLocation;
	GLuint ambientIntensityLocation;
	GLuint directionalColorLocation;
	GLuint directionalIntensityLocation;
	GLuint directionalDirectionLocation;
	struct {
		GLuint color;
		GLuint intensity;
		GLuint position;
		GLuint attenuation;
	} pointLightLocations[N_MAX_LIGHTS];
	GLuint nLightsLocation;
	// Lights
	ambientLight aLight;								// Global ambient light
	directionalLight dLight;							// Global directional light (will be removed at some point probably)
	pointLight pointLightsArr[N_MAX_LIGHTS];			// Array with the maximum number of lights
	unsigned int nLights;								// Actual number of lights (nLights < N_MAX_LIGHTS)
	// Functions
	bool readFile(const char* path, std::string& out);	// Reads a text file on path and returns its contents on out.
	void initializeLightingGL();						// Initializes lightning on openGL. (not used)(should be remove)
	void initializeLighting();							// Initializes nLights point lights with pseudo-random attributes.
	void initializeShaders();							// Reads and compiles the vertex and fragment shader.
	void updateFPS();									// Keeps track of the fps of the painGL function.
	void initLocations();								// Initializes the lovation variables from the shaders.
	void setLightUniforms();							// Sends lightning information to the shaders.
};
