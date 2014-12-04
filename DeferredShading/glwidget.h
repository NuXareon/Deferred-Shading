#include "mesh.h"
#include "gbuffer.h"
#include "depthBuffer.h"
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
	void modifyConstantAttenuation(QString s);
	void modifyLinearAttenuation(QString s);
	void modifyExpAttenuation(QString s);
	void enableBillboards(int s);
	// Render
	void setForwardRenderMode();
	void setForwardBlendRenderMode();
	void setPositionRenderMode();
	void setNormalRenderMode();
	void setDiffuseRenderMode();
	void setAllRenderMode();
	void setDepthRenderMode();
	void setDeferredRenderMode();
	void setForwardDebugRenderMode();
	void setForwardPlusRenderMode();
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
	GLuint shaderProgramBlend;
	GLuint shaderProgramDeferredGeo;
	GLuint shaderProgramDeferredLight;
	GLuint shaderProgramDeferredDirectionalLight;
	GLuint shaderProgramDeferredDebug;
	GLuint shaderProgramForwardDepthDebug;
	GLuint shaderProgramDepthSet;
	GLuint shaderProgramForwardPlusDebug;
	GLuint shaderProgramForwardPlus;

	renderModeType renderMode;

	// --- Locations ---
	// Forward
	GLuint positionLocation;
	GLuint texCoordLocation;
	GLuint normLocation;
	GLuint samplerLocation;
	GLuint depthDebugTextureLocation;
	GLuint lightsTexBufferLocation;
	GLuint ambientColorLocation;
	GLuint ambientIntensityLocation;
	GLuint directionalColorLocation;
	GLuint directionalIntensityLocation;
	GLuint directionalDirectionLocation;

	// Forward blend
	GLuint positionBlendLocation;
	GLuint texCoordBlendLocation;
	GLuint normBlendLocation;
	GLuint samplerBlendLocation;
	GLuint zOffsetBlendLocation;
	GLuint ambientColorBlendLocation;
	GLuint ambientIntensityBlendLocation;
	GLuint directionalColorBlendLocation;
	GLuint directionalIntensityBlendLocation;
	GLuint directionalDirectionBlendLocation;
	GLuint nLightsBlendLocation;
	struct {
		GLuint color;
		GLuint intensity;
		GLuint radius;
		GLuint position;
		GLuint attenuation;
	} pointLightLocations[FORWARD_LIGHTS_INTERVAL];
	GLuint nLightsLocation;

	// Deferred debug
	GLuint positionDeferredDebugLocation;
	GLuint texCoordDeferredDebugLocation;
	GLuint normDeferredDebugLocation;
	GLuint samplerDeferredDebugLocation;
	GLuint minPDeferredDebugLocation;
	GLuint maxPDeferredDebugLocation;

	// Deferred Geometry
	GLuint positionDeferredGeoLocation;
	GLuint texCoordDeferredGeoLocation;
	GLuint normDeferredGeoLocation;
	GLuint samplerDeferredGeoLocation;
	GLuint depthSetSamplerLocation;

	// Deferred Lighting
	GLuint screenSizeDeferredLightLocation;
	GLuint positionBufferDeferredLightLocation;
	GLuint normalBufferDeferredLightLocation;
	GLuint diffuseBufferDeferredLightLocation;
	GLuint pLightColorDeferredLightLocation;
	GLuint pLightIntensityDeferredLightLocation;
	GLuint pLightPositionDeferredLightLocation;
	GLuint pLightAttenuationDeferredLightLocation;
	GLuint pLightRadiusDeferredLightLocation;

	// Deferred directional light
	GLuint ambientColorDirectionalLocation;
	GLuint ambientIntensityDirectionalLocation;
	GLuint directionalColorDirectionalLocation;
	GLuint directionalIntisityDirectionalLocation;
	GLuint directionaldirectionDirectionalLocation;
	GLuint normalBufferDirectionalLocation;
	GLuint diffuseBufferDirectionalLocation;

	// Forward plus debug
	GLuint scanSumForwardDebugLocation;
	GLuint maxLightsForwardDebugLocation;
	GLuint screenSizeForwardDebugLocation;

	// Forward Plus
	GLuint positionForwardPlusLocation;
	GLuint texCoordForwardPlusLocation;
	GLuint normForwardPlusLocation;
	GLuint screenSizeForwardPlusLocation;
	GLuint ambientLightColorForwardPlusLocation;
	GLuint ambientLightIntensityForwardPlusLocation;
	GLuint directionalLightColorForwardPlusLocation;
	GLuint directionalLightIntensityForwardPlusLocation;
	GLuint directionalLightDirectionForwardPlusLocation;
	GLuint samplerForwardPlusLocation;
	GLuint lightsTexBufferForwardPlusLocation;
	GLuint lightsGridForwardPlusLocation;
	GLuint scanSumForwardPlusLocation;

	// Lights
	ambientLight aLight;								// Global ambient light
	directionalLight dLight;							// Global directional light
	pointLight pointLightsArr[N_MAX_LIGHTS];			// Array with the maximum number of lights
	unsigned int nLights;								// Actual number of lights (nLights < N_MAX_LIGHTS)
	float lightingBoundingBoxScale;
	float maxIntensity;
	int threshold;
	float constAtt, linearAtt, expAtt;
	Texture* lightBillboard;
	bool lBillboards;
	unsigned int gLightsCol, gLightsRow;
	std::vector<std::vector<int> > lightsMatrix;
	std::vector<int> lightsScanSum;

	static const int lightsMatrixLength = (MAX_WIN_WIDTH/GRID_RES)*(MAX_WIN_HEIGHT/GRID_RES)*LIGHTS_PER_TILE;
	static const int lightsScanSumLength = (MAX_WIN_WIDTH/GRID_RES)*(MAX_WIN_HEIGHT/GRID_RES);
	int _lightsMatrix[lightsMatrixLength];
	int _lightsScanSum[lightsScanSumLength];

	unsigned int maxTileLights;
	// Buffers
	gbuffer *gBufferDS;									// G-buffer: framebuffer with the textures we will use for deferred shading
	depthBuffer *dBufferFR;								// Depth Buffer: framebuffer used for the depth prepass in the forward reder
	depthBuffer *dBufferDS;
	GLuint LTB,LGTB,SSTB;								// Texture buffer containing the lighting information for forward rendering
	// Functions
	void initializeLighting();							// Initializes nLights point lights with pseudo-random attributes.
	void updateLightingBuffer();
	void importLighting(std::ifstream& ifs);
	void initializeShaders();							// Reads and compiles the vertex and fragment shader.
	void initializeShadersDeferred();					// same as initilizeShaders() but for deferred shading shaders.
	void initializeShaderProgram(const char *vsP, const char *fsP, GLuint *sp);
	void updateFPS();									// Keeps track of the fps of the painGL function.
	void initLocations();								// Initializes the lovation variables from the shaders.
	void setLightUniforms();							// Sends lightning information to the shaders. 
	void setLightPassUniforms();						// Sends gBuffer infor to the shaders
	void setForwardPlusUniforms();
	void drawPointLight(pointLight l);					// Draws a sphere equivalent to a point light
	void setLightUniformsBlend(unsigned int l, unsigned int h, float offset);
	void drawLightBillboard(pointLight l, float width);	// Draws a billboard for a point light
	void DrawDepthPrepass();
	void DrawLightsGrid();
	void directionalLightPass();
	void updateLightsMatrix();
	void _updateLightsMatrix();
	void clearLigthsMatrix();

	int arr[1800000];
};
