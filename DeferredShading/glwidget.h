#include "mesh.h"
#include "gbuffer.h"
#include "utils.h"

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
	// Lighting
	void modifyMaxIntensity(QString s);
	void modifyBoundingBoxScale(QString s);
	void modifyThreshold(QString);
	// Render
	void setForwardRenderMode();
	void setPositionRenderMode();
	void setNormalRenderMode();
	void setDiffuseRenderMode();
	void setAllRenderMode();
	void setDeferredRenderMode();
	// Misc.
	void loadModel(std::string path);					// Loads a model located on path.
	void genLightning(int n);							// Modifies nLights and regenerates lightning, see initializeLightning().

signals:
	void updateFPSSignal(int fps);
	void updateLightIntensityIn(QString s);
	void updateRenderMode(QString s);

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
	// Timers
	QTime* t;
	int fps, frames;
	int inputTimerId, drawTimerId;
	// Shader atribute and uniform locations
	GLuint shaderProgram;
	GLuint shaderProgramDeferredGeo;
	GLuint shaderProgramDeferredLight;
	GLuint shaderProgramDeferredDebug;

	renderModeType renderMode;

	GLuint positionLocation;
	GLuint texCoordLocation;
	GLuint normLocation;
	GLuint samplerLocation;

	GLuint positionDeferredLocation;
	GLuint texCoordDeferredLocation;
	GLuint normDeferredLocation;
	GLuint samplerDeferredLocation;
	GLuint minPDeferredLocation;
	GLuint maxPDeferredLocation;

	GLuint positionDeferredDebugLocation;
	GLuint texCoordDeferredDebugLocation;
	GLuint normDeferredDebugLocation;
	GLuint samplerDeferredDebugLocation;
	GLuint minPDeferredDebugLocation;
	GLuint maxPDeferredDebugLocation;

	GLuint positionDeferredGeoLocation;
	GLuint texCoordDeferredGeoLocation;
	GLuint normDeferredGeoLocation;
	GLuint samplerDeferredGeoLocation;

	GLuint screenSizeDeferredLightLocation;
	GLuint positionBufferDeferredLightLocation;
	GLuint normalBufferDeferredLightLocation;
	GLuint diffuseBufferDeferredLightLocation;
	GLuint pLightColorDeferredLightLocation;
	GLuint pLightIntensityDeferredLightLocation;
	GLuint pLightPositionDeferredLightLocation;
	GLuint pLightAttenuationDeferredLightLocation;

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
	float lightingBoundingBoxScale;
	float maxIntensity;
	int threshold;
	// Deferred
	gbuffer *gBufferDS;									// G-buffer: framebuffer with the textures we will use for deferred shading
	// Functions
	void initializeLightingGL();						// Initializes lightning on openGL. (not used)(should be remove)
	void initializeLighting();							// Initializes nLights point lights with pseudo-random attributes.
	void initializeShaders();							// Reads and compiles the vertex and fragment shader.
	void initializeShadersDeferred();					// same as initilizeShaders() but for deferred shading shaders.
	void initializeShaderProgram(const char *vsP, const char *fsP, GLuint *sp);
	void updateFPS();									// Keeps track of the fps of the painGL function.
	void initLocations();								// Initializes the lovation variables from the shaders.
	void setLightUniforms();							// Sends lightning information to the shaders.
};
