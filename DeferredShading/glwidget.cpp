#include "glwidget.h"

const char* VSPath = "forward_shader.vs";
const char* FSPath = "forward_shader.fs";
const char* VSPathForwardPlusDebug = "forward_shader_depth_debug.vs";
const char* FSPathForwardPlusDebug = "forward_plus_debug.fs";
const char* VSPathBlend = "forward_shader.vs";
const char* FSPathBlend = "forward_shader_blend.fs";
const char* VSPathDeferredGeo = "forward_shader.vs";
const char* FSPathDeferredGeo = "deferred_shader_geo.fs";
const char* VSPathDeferredDebug = "forward_shader.vs";
const char* FSPathDeferredDebug = "deferred_shader_debug.fs";
const char* VSPathDeferredLight = "deferred_shader_light.vs";
const char* FSPathDeferredLight = "deferred_shader_light.fs";
const char* VSPathForwardDepthDebug = "forward_shader_depth_debug.vs";
const char* FSPathForwardDepthDebug = "forward_shader_depth_debug.fs";
const char* VSPathDeferredDepth = "forward_shader_depth_debug.vs";
const char* FSPathDeferredDepth = "deferred_shader_set_depth.fs";
const char* VSPathDeferredDirectionalLight = "forward_shader_depth_debug.vs";
const char* FSPathDeferredDirectionalLight = "deferred_directional_light.fs";
const char* VSPathForwardPlus = "forward_shader.vs";
const char* FSPathForwardPlus = "forward_plus_shader.fs";

GLWidget::GLWidget(QWidget *parent) :
    QGLWidget()
{
	// Qt
	setFocusPolicy(Qt::ClickFocus);			// Tells Qt how to manage the focues (mouse click).
	setMouseTracking(true);					// Enables tracking of the mouse
	// Camera
	xPos=zPos=0.0f;
	yPos=1.0f;
	alpha=beta=0.0f;
	cSensitivity = 0.1f;
	cSpeed = 1.0f;
	// Model
	modelPath = "../DeferredShading/Models/sponza/sponza.obj";
	// Lights
	nLights = INITIAL_LIGHTS;
	lightingBoundingBoxScale = 0.65f;
	maxIntensity = 0.0f;
	threshold = LIGHT_THRESHOLD;
	constAtt = ATTENUATION_CONSTANT;
	linearAtt = ATTENUATION_LINEAR;
	expAtt = ATTENUATION_EXP;
	lBillboards = false;
	// Timers
	inputTimerId = startTimer(1000/60);			// Camera refresh rate (60 FPS)
	drawTimerId = startTimer(0);				// Render refresh rate. V-SYNC MUST BE TURNED OFF.
	// Render
	renderMode = RENDER_FORWARD;
	// Buffers
	gBufferDS = new gbuffer;
	dBufferFR = new depthBuffer;
	dBufferDS = new depthBuffer;
}

GLWidget::~GLWidget()
{
}

void GLWidget::loadModel(std::string path)
{
	modelPath = path;
	maxIntensity = 0.0f;
	mainMesh = new Mesh();
    mainMesh->LoadMesh(modelPath);
	std::ifstream ifs(LIGHT_PATH);
	if (ifs.good()) {
		std::string s;
		std::getline(ifs,s);
		if (s == modelPath) {
			importLighting(ifs);
		}
		else initializeLighting();
	}
	else initializeLighting();
}

void GLWidget::genLightning(int n)
{
	nLights = n;
	initializeLighting();
}

void GLWidget::setForwardRenderMode()	
{
	renderMode = RENDER_FORWARD;
	emit updateRenderMode("Forward");
}

void GLWidget::setForwardBlendRenderMode()
{
	renderMode = RENDER_FORWARD_BLEND;
	emit updateRenderMode("Forward(Blend)");
}

void GLWidget::setPositionRenderMode()	
{
	renderMode = RENDER_POSITION;
	emit updateRenderMode("Positions");
}

void GLWidget::setNormalRenderMode()	
{
	renderMode = RENDER_NORMAL;
	emit updateRenderMode("Normals");
}

void GLWidget::setDiffuseRenderMode()	
{
	renderMode = RENDER_DIFFUSE;
	emit updateRenderMode("Diffuse");
}

void GLWidget::setAllRenderMode()		
{
	renderMode = RENDER_ALL;
	emit updateRenderMode("All");
}

void GLWidget::setDepthRenderMode()
{
	renderMode = RENDER_DEPTH;
	emit updateRenderMode("Depth");
}

void GLWidget::setDeferredRenderMode()	
{
	renderMode = RENDER_DEFERRED;
	emit updateRenderMode("Deferred");
}

void GLWidget::setForwardDebugRenderMode()	
{
	renderMode = RENDER_GRID;
	emit updateRenderMode("Grid");
}

void GLWidget::setForwardPlusRenderMode()
{
	renderMode = RENDER_FORWARD_PLUS;
	emit updateRenderMode("Forward+");
}

void GLWidget::initializeShaderProgram(const char *vsP, const char *fsP, GLuint *sp)
{
	*(sp) = glCreateProgram();

	// Vertex shader load
	std::string vertexShader;

	if (!utils::readFile(vsP, vertexShader)) exit(1);
	
	GLuint vShaderObj = glCreateShader(GL_VERTEX_SHADER);

	const char* shaderFiles[1];
	shaderFiles[0] = vertexShader.c_str();
	GLint lengths[1];
	lengths[0] = vertexShader.size();
	glShaderSource(vShaderObj, 1, shaderFiles, lengths);
	glCompileShader(vShaderObj);
	GLint success;
	glGetShaderiv(vShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		char InfoLog[1024];
		glGetShaderInfoLog(vShaderObj, sizeof(InfoLog), NULL, InfoLog);
		fprintf(stderr, "Error compiling shader: '%s'\n", InfoLog);
	}

	glAttachShader(*(sp),vShaderObj);

	// Fragment shader load
	std::string fragmentShader;

	if (!utils::readFile(fsP, fragmentShader)) exit(1);
	
	GLuint fShaderObj = glCreateShader(GL_FRAGMENT_SHADER);

	shaderFiles[0] = fragmentShader.c_str();
	lengths[0] = fragmentShader.size();
	glShaderSource(fShaderObj, 1, shaderFiles, lengths);
	glCompileShader(fShaderObj);
	glGetShaderiv(fShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		char InfoLog[1024];
		glGetShaderInfoLog(fShaderObj, sizeof(InfoLog), NULL, InfoLog);
		fprintf(stderr, "Error compiling shader: '%s'\n", InfoLog);
	}

	glAttachShader(*(sp),fShaderObj);
	glLinkProgram(*(sp));
	glGetProgramiv(*(sp),GL_LINK_STATUS,&success);
	if (!success) {
		char InfoLog[1024];
		glGetShaderInfoLog(shaderProgram, sizeof(InfoLog), NULL, InfoLog);
		fprintf(stderr, "Error linking shader program: '%s'\n", InfoLog);
	}
	glValidateProgram(*(sp));
	glGetProgramiv(*(sp),GL_VALIDATE_STATUS,&success);
	if (!success) {
		char InfoLog[1024];
		glGetShaderInfoLog(shaderProgram, sizeof(InfoLog), NULL, InfoLog);
		fprintf(stderr, "Error linking shader program: '%s'\n", InfoLog);
	}
}

void GLWidget::initLocations()
{
	// Forward Shaders
    positionLocation = glGetAttribLocation(shaderProgram,"position");
	texCoordLocation = glGetAttribLocation(shaderProgram,"texCoord");
	normLocation = glGetAttribLocation(shaderProgram,"norm");
	samplerLocation = glGetUniformLocation(shaderProgram,"sampler");
	ambientColorLocation = glGetUniformLocation(shaderProgram,"aLight.color");
	ambientIntensityLocation = glGetUniformLocation(shaderProgram,"aLight.intensity");
	directionalColorLocation = glGetUniformLocation(shaderProgram,"dLight.color");
	directionalIntensityLocation = glGetUniformLocation(shaderProgram,"dLight.intensity");
	directionalDirectionLocation = glGetUniformLocation(shaderProgram,"dLight.direction");
	lightsTexBufferLocation = glGetUniformLocation(shaderProgram,"lightsTexBuffer");
	depthDebugTextureLocation = glGetUniformLocation(shaderProgramForwardDepthDebug,"texture");
	nLightsLocation = glGetUniformLocation(shaderProgram,"nLights");

	//Forward Blend Shaders
	positionBlendLocation = glGetAttribLocation(shaderProgramBlend,"position");
	texCoordBlendLocation = glGetAttribLocation(shaderProgramBlend,"texCoord");
	normBlendLocation = glGetAttribLocation(shaderProgramBlend,"norm");
	samplerBlendLocation = glGetUniformLocation(shaderProgramBlend,"sampler");
	zOffsetBlendLocation = glGetUniformLocation(shaderProgramBlend,"zOffset");
	ambientColorBlendLocation = glGetUniformLocation(shaderProgramBlend,"aLight.color");
	ambientIntensityBlendLocation = glGetUniformLocation(shaderProgramBlend,"aLight.intensity");
	directionalColorBlendLocation = glGetUniformLocation(shaderProgramBlend,"dLight.color");
	directionalIntensityBlendLocation = glGetUniformLocation(shaderProgramBlend,"dLight.intensity");
	directionalDirectionBlendLocation = glGetUniformLocation(shaderProgramBlend,"dLight.direction"); 
	nLightsBlendLocation = glGetUniformLocation(shaderProgramBlend,"nLights");

	// Forward Blend Point Lights
	std::stringstream sstm;
	std::string uniformName;
	std::string pl_str = "pointLights[";
	std::string color_str = "].color";
	std::string intensity_str = "].intensity";
	std::string radius_str = "].radius";
	std::string position_str = "].position";
	std::string attenuation_str = "].attenuation";
	std::string str_arr[] = {color_str, intensity_str, radius_str, position_str, attenuation_str};
	for (unsigned int i = 0; i < FORWARD_LIGHTS_INTERVAL; i++) {
		for (unsigned int j = 0; j < 5; j++) {
			sstm << pl_str << i << str_arr[j];
			uniformName = sstm.str();
			if (j == 0) pointLightLocations[i].color = glGetUniformLocation(shaderProgramBlend,uniformName.c_str());
			else if (j == 1) pointLightLocations[i].intensity = glGetUniformLocation(shaderProgramBlend,uniformName.c_str());
			else if (j == 2) pointLightLocations[i].radius = glGetUniformLocation(shaderProgramBlend,uniformName.c_str());
			else if (j == 3) pointLightLocations[i].position = glGetUniformLocation(shaderProgramBlend,uniformName.c_str());
			else if (j == 4) pointLightLocations[i].attenuation = glGetUniformLocation(shaderProgramBlend,uniformName.c_str());
			sstm.str(std::string());
			sstm.clear();
		}
	}

	// Deferred Debug Shaders
	positionDeferredDebugLocation = glGetAttribLocation(shaderProgramDeferredDebug,"position");
	texCoordDeferredDebugLocation = glGetAttribLocation(shaderProgramDeferredDebug,"texCoord");
	normDeferredDebugLocation = glGetAttribLocation(shaderProgramDeferredDebug,"norm");
	samplerDeferredDebugLocation = glGetUniformLocation(shaderProgramDeferredDebug,"sampler");
	minPDeferredDebugLocation = glGetUniformLocation(shaderProgramDeferredDebug,"minP");
	maxPDeferredDebugLocation = glGetUniformLocation(shaderProgramDeferredDebug,"maxP");

	// Deferred Geometry Shaders
	positionDeferredGeoLocation = glGetAttribLocation(shaderProgramDeferredGeo,"position");
	texCoordDeferredGeoLocation = glGetAttribLocation(shaderProgramDeferredGeo,"texCoord");
	normDeferredGeoLocation = glGetAttribLocation(shaderProgramDeferredGeo,"norm");
	samplerDeferredGeoLocation = glGetUniformLocation(shaderProgramDeferredGeo,"sampler");
	depthSetSamplerLocation = glGetUniformLocation(shaderProgramDepthSet,"texture");

	//	Deferred Lighting Shaders
	screenSizeDeferredLightLocation = glGetUniformLocation(shaderProgramDeferredLight,"screenSize");
	positionBufferDeferredLightLocation = glGetUniformLocation(shaderProgramDeferredLight,"positionBuffer");
	normalBufferDeferredLightLocation = glGetUniformLocation(shaderProgramDeferredLight,"normalBuffer");
	diffuseBufferDeferredLightLocation = glGetUniformLocation(shaderProgramDeferredLight,"diffuseBuffer");
	pLightColorDeferredLightLocation = glGetUniformLocation(shaderProgramDeferredLight,"pLight.color");
	pLightIntensityDeferredLightLocation = glGetUniformLocation(shaderProgramDeferredLight,"pLight.intensity");
	pLightPositionDeferredLightLocation = glGetUniformLocation(shaderProgramDeferredLight,"pLight.position");
	pLightAttenuationDeferredLightLocation = glGetUniformLocation(shaderProgramDeferredLight,"pLight.attenuation");
	pLightRadiusDeferredLightLocation = glGetUniformLocation(shaderProgramDeferredLight,"pLight.radius");

	// Forward Debug Shaders
	scanSumForwardDebugLocation = glGetUniformLocation(shaderProgramForwardPlusDebug,"scanSum");
	maxLightsForwardDebugLocation = glGetUniformLocation(shaderProgramForwardPlusDebug,"maxTileLights");
	screenSizeForwardDebugLocation = glGetUniformLocation(shaderProgramForwardPlusDebug,"screenSize");

	// Deferred Directional light
	ambientColorDirectionalLocation = glGetUniformLocation(shaderProgramDeferredDirectionalLight,"aLight.color");
	ambientIntensityDirectionalLocation = glGetUniformLocation(shaderProgramDeferredDirectionalLight,"aLight.intensity");
	directionalColorDirectionalLocation = glGetUniformLocation(shaderProgramDeferredDirectionalLight,"dLight.color");
	directionalIntisityDirectionalLocation = glGetUniformLocation(shaderProgramDeferredDirectionalLight,"dLight.intensity");
	directionaldirectionDirectionalLocation = glGetUniformLocation(shaderProgramDeferredDirectionalLight,"dLight.direction");
	normalBufferDirectionalLocation = glGetUniformLocation(shaderProgramDeferredDirectionalLight,"normalBuffer");
	diffuseBufferDirectionalLocation = glGetUniformLocation(shaderProgramDeferredDirectionalLight,"diffuseBuffer");

	// Forward Plus
	positionForwardPlusLocation = glGetAttribLocation(shaderProgramForwardPlus,"position");;
	texCoordForwardPlusLocation = glGetAttribLocation(shaderProgramForwardPlus,"texCoord");;
	normForwardPlusLocation = glGetAttribLocation(shaderProgramForwardPlus,"norm");;
	screenSizeForwardPlusLocation = glGetUniformLocation(shaderProgramForwardPlus,"screenSize");
	ambientLightColorForwardPlusLocation = glGetUniformLocation(shaderProgramForwardPlus,"aLight.color");
	ambientLightIntensityForwardPlusLocation = glGetUniformLocation(shaderProgramForwardPlus,"aLight.intensity");
	directionalLightColorForwardPlusLocation = glGetUniformLocation(shaderProgramForwardPlus,"dLight.color");
	directionalLightIntensityForwardPlusLocation = glGetUniformLocation(shaderProgramForwardPlus,"dLight.intensity");
	directionalLightDirectionForwardPlusLocation = glGetUniformLocation(shaderProgramForwardPlus,"dLight.direction");
	samplerForwardPlusLocation = glGetUniformLocation(shaderProgramForwardPlus,"sampler");
	lightsTexBufferForwardPlusLocation = glGetUniformLocation(shaderProgramForwardPlus,"lightsTexBuffer");
	lightsGridForwardPlusLocation = glGetUniformLocation(shaderProgramForwardPlus,"lightsGrid");
	scanSumForwardPlusLocation = glGetUniformLocation(shaderProgramForwardPlus,"scanSum");
}

void GLWidget::initializeLighting()
{
	// Initialize generator
	std::default_random_engine generator((unsigned int)time(0));
	BoundingBox bb = mainMesh->getBoundingBox()*lightingBoundingBoxScale;

	if (maxIntensity == 0.0f) {
		maxIntensity = sqrt((bb.max.x-bb.min.x)*(bb.max.x-bb.min.x)+(bb.max.y-bb.min.y)*(bb.max.x-bb.min.z)+(bb.max.x-bb.min.z)*(bb.max.x-bb.min.z)+(bb.max.x-bb.min.z));
		emit updateLightIntensityIn(QString::number(maxIntensity));
	}
	std::uniform_real_distribution<float> distributionX(bb.min.x,bb.max.x);
	std::uniform_real_distribution<float> distributionY(bb.min.y,bb.max.y);
	std::uniform_real_distribution<float> distributionZ(bb.min.z,bb.max.z);
	std::uniform_real_distribution<float> distributionC(0.0,1.0);
	//std::uniform_real_distribution<float> distributionI(maxIntensity/2,maxIntensity);
	float pAttenuation[] = {constAtt, linearAtt, expAtt};

	// Populate pointLightsArr with nLight
	for (unsigned int i = 0; i < nLights; i++){
		bool nearGeometry = false;
		float pPosition[] = {distributionX(generator),distributionY(generator),distributionZ(generator)};
		float pColor[] = {distributionC(generator), distributionC(generator), distributionC(generator)};	
		float pIntensity = maxIntensity;
		pointLight p = pointLight(pColor, pIntensity, pPosition, pAttenuation);
		while (!utils::isLightNearGeo(p, &mainMesh->m_vPosition)) {
			pPosition[0] = distributionX(generator);
			pPosition[1] = distributionY(generator);
			pPosition[2] = distributionZ(generator);
			p = pointLight(pColor, pIntensity, pPosition, pAttenuation);
		}
		float pRadius = utils::calcLightRadius(p, threshold);
		p.intensity.i_ = pRadius;
		pointLightsArr[i] = p;
	}

	// Populate texture buffer
	GLuint TB;
	glGenBuffers(1,&TB);
	glBindBuffer(GL_TEXTURE_BUFFER,TB);
	glBufferData(GL_TEXTURE_BUFFER,nLights*sizeof(pointLight),&pointLightsArr[0],GL_STATIC_COPY);

	glBindTexture(GL_TEXTURE_BUFFER,LTB);
	glTexBuffer(GL_TEXTURE_BUFFER,GL_RGB32F,TB);

	glDeleteBuffers(1,&TB);

	utils::saveLightingToFile(pointLightsArr, nLights, modelPath);
}

void GLWidget::importLighting(std::ifstream& ifs)
{
	float wColor[] = {1.0f,1.0f,1.0f};
	aLight = ambientLight(wColor, 0.0f);

	std::string light = "";
	unsigned int i = 0;
	while (getline(ifs, light)) {
		std::stringstream ss(light);
		std::string param = "";
		unsigned int j = 0;
		while (getline(ss,param,',')) {
			if (j == 0) pointLightsArr[i].color.r = std::stof(param);
			if (j == 1) pointLightsArr[i].color.g = std::stof(param);
			if (j == 2) pointLightsArr[i].color.b = std::stof(param);
			if (j == 3) pointLightsArr[i].intensity.i = std::stof(param);
			if (j == 4) pointLightsArr[i].intensity.i_ = std::stof(param);
			if (j == 5) pointLightsArr[i].position.x = std::stof(param);
			if (j == 6) pointLightsArr[i].position.y = std::stof(param);
			if (j == 7) pointLightsArr[i].position.z = std::stof(param);
			if (j == 8) pointLightsArr[i].attenuation.constant = std::stof(param);
			if (j == 9) pointLightsArr[i].attenuation.linear = std::stof(param);
			if (j == 10) pointLightsArr[i].attenuation.exp = std::stof(param);
			++j;
		}
		++i;
	}
	ifs.close();
	nLights = i;

	// Populate texture buffer
	GLuint TB;
	glGenBuffers(1,&TB);
	glBindBuffer(GL_TEXTURE_BUFFER,TB);
	glBufferData(GL_TEXTURE_BUFFER,nLights*sizeof(pointLight),&pointLightsArr[0],GL_STATIC_COPY);

	glBindTexture(GL_TEXTURE_BUFFER,LTB);
	glTexBuffer(GL_TEXTURE_BUFFER,GL_RGB32F,TB);

	glDeleteBuffers(1,&TB);
}

void GLWidget::initializeGL()
{
	// GLEW
	glewInit();
	// OpenGL
    qglClearColor(QColor::fromRgb(0,0,0));
	glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

	#ifdef _WIN32
	utils::enableVSyncWin(0);
	#elif __linux__
	utils::enableVSyncLinux(0);
	#endif
	// Billboards
	glGenTextures(1,&LTB);
	glGenTextures(1,&LGTB);
	glGenTextures(1,&SSTB);
	loadModel(modelPath);
	lightBillboard = new Texture(GL_TEXTURE_2D, "../DeferredShading/billboard.png");
	lightBillboard->Load();
	// Shaders
	initializeShaderProgram(VSPath, FSPath, &shaderProgram);
	initializeShaderProgram(VSPathForwardPlusDebug, FSPathForwardPlusDebug, &shaderProgramForwardPlusDebug);
	initializeShaderProgram(VSPathBlend, FSPathBlend, &shaderProgramBlend);
	initializeShaderProgram(VSPathDeferredGeo, FSPathDeferredGeo, &shaderProgramDeferredGeo);
	initializeShaderProgram(VSPathDeferredLight, FSPathDeferredLight, &shaderProgramDeferredLight);
	initializeShaderProgram(VSPathDeferredDirectionalLight, FSPathDeferredDirectionalLight, &shaderProgramDeferredDirectionalLight);
	initializeShaderProgram(VSPathDeferredDebug, FSPathDeferredDebug, &shaderProgramDeferredDebug);
	initializeShaderProgram(VSPathForwardDepthDebug, FSPathForwardDepthDebug, &shaderProgramForwardDepthDebug);
	initializeShaderProgram(VSPathDeferredDepth, FSPathDeferredDepth, &shaderProgramDepthSet);
	initializeShaderProgram(VSPathForwardPlus, FSPathForwardPlus, &shaderProgramForwardPlus);
	initLocations();
	// FPS count
	frames = 0;
	t = new QTime();
	t->start();
	// Directional + Ambient lights
	float wColor[] = {1.0f,1.0f,1.0f};
	float dDirection[] = {-0.577f,-0.577f,-0.577f};
	
	aLight = ambientLight(wColor, 0.02f);
	dLight = directionalLight(wColor, 0.035f, dDirection);
}

void GLWidget::resizeGL(int width, int height)
{
    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60,(double)width/height,1.0,10000.0);
    glMatrixMode(GL_MODELVIEW);

	gBufferDS->init(width, height);
	dBufferFR->init(width, height);
	dBufferDS->init(width, height);

	gLightsCol = ceil((float)width/GRID_RES);
	gLightsRow = ceil((float)height/GRID_RES);
	lightsMatrix.resize(gLightsRow*gLightsCol);
}

void GLWidget::setLightUniforms()
{
	// Ambient light parameters
	glUniform3f(ambientColorLocation, aLight.color.r, aLight.color.g, aLight.color.b);
	glUniform1f(ambientIntensityLocation,aLight.intensity);
	// Directional light parameters
	glUniform3f(directionalColorLocation, dLight.color.r, dLight.color.g, dLight.color.b);
	glUniform1f(directionalIntensityLocation, dLight.intensity);
	glUniform3f(directionalDirectionLocation, dLight.direction.x, dLight.direction.y, dLight.direction.z);
	// Point light parameters
	glUniform1i(nLightsLocation, nLights);
}

void GLWidget::setLightUniformsBlend(unsigned int l, unsigned int h, float offset)
{	
	// Ambient Light
	glUniform3f(ambientColorBlendLocation, aLight.color.r, aLight.color.g, aLight.color.b);
	glUniform1f(ambientIntensityBlendLocation,aLight.intensity);
	// Directioal Light
	glUniform3f(directionalColorBlendLocation, dLight.color.r, dLight.color.g, dLight.color.b);
	glUniform1f(directionalIntensityBlendLocation, dLight.intensity);
	glUniform3f(directionalDirectionBlendLocation, dLight.direction.x, dLight.direction.y, dLight.direction.z);
	// Point lights parameters (note: the fragment shader allways uses the n first lights)
	for (unsigned int i = l; i < h; ++i) {
		glUniform3f(pointLightLocations[i-l].color, pointLightsArr[i].color.r, pointLightsArr[i].color.g, pointLightsArr[i].color.b);
		glUniform1f(pointLightLocations[i-l].intensity, pointLightsArr[i].intensity.i);
		glUniform1f(pointLightLocations[i-l].radius, pointLightsArr[i].intensity.i_);
		glUniform3f(pointLightLocations[i-l].position, pointLightsArr[i].position.x, pointLightsArr[i].position.y, pointLightsArr[i].position.z);
		glUniform3f(pointLightLocations[i-l].attenuation, pointLightsArr[i].attenuation.constant, pointLightsArr[i].attenuation.linear, pointLightsArr[i].attenuation.exp);
	}
	glUniform1i(nLightsBlendLocation, h-l);
	glUniform1f(zOffsetBlendLocation, offset);
 }

void GLWidget::setLightPassUniforms()
{
	glUniform1i(positionBufferDeferredLightLocation, gbuffer::GBUFFER_POSITION);
	glUniform1i(normalBufferDeferredLightLocation, gbuffer::GBUFFER_NORMAL);
	glUniform1i(diffuseBufferDeferredLightLocation, gbuffer::GBUFFER_DIFFUSE);
	glUniform2f(screenSizeDeferredLightLocation, width(), height());
}

void GLWidget::setForwardPlusUniforms() 
{
	glUniform2f(screenSizeForwardPlusLocation, width(), height());
	// Ambient light parameters
	glUniform3f(ambientLightColorForwardPlusLocation, aLight.color.r, aLight.color.g, aLight.color.b);
	glUniform1f(ambientLightIntensityForwardPlusLocation,aLight.intensity);
	// Directional light parameters
	glUniform3f(directionalLightColorForwardPlusLocation, dLight.color.r, dLight.color.g, dLight.color.b);
	glUniform1f(directionalLightIntensityForwardPlusLocation, dLight.intensity);
	glUniform3f(directionalLightDirectionForwardPlusLocation, dLight.direction.x, dLight.direction.y, dLight.direction.z);
	// Buffer textures
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_BUFFER, LTB);
	glUniform1i(lightsTexBufferForwardPlusLocation, 1);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_BUFFER, LGTB);
	glUniform1i(lightsGridForwardPlusLocation, 2);	
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_BUFFER, SSTB);
	glUniform1i(scanSumForwardPlusLocation, 3);
}

void drawAxis()
{
	glUseProgram(0);
	glBegin(GL_LINES);
		glColor3f(1.0f,0.0f,0.0f);
		glVertex3f(0.0f,0.0f,0.0f);
		glVertex3f(100.0f,0.0f,0.0f);
		glColor3f(0.0f,1.0f,0.0f);
		glVertex3f(0.0f,0.0f,0.0f);
		glVertex3f(0.0f,100.0f,0.0f);
		glColor3f(0.0f,0.0f,1.0f);
		glVertex3f(0.0f,0.0f,0.0f);
		glVertex3f(0.0f,0.0f,100.0f);
		glColor3f(0.5f,0.5f,0.5f);
	glEnd();
}

void renderAll(int const width, int const height)
{
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glBlitFramebuffer(0, 0, width, height, 0, height/2, width/2, height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	glReadBuffer(GL_COLOR_ATTACHMENT1);
	glBlitFramebuffer(0, 0, width, height, width/2, height/2, width, height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	glReadBuffer(GL_COLOR_ATTACHMENT2);
	glBlitFramebuffer(0, 0, width, height, 0, 0, width/2, height/2, GL_COLOR_BUFFER_BIT, GL_LINEAR);
}

void GLWidget::paintGL()
{
	const float PI = 3.1415927f;
	float alphaRad = PI*alpha/180;
	float betaRad = PI*beta/180;
    glLoadIdentity();
	gluLookAt(	xPos, yPos, zPos,
				xPos+cos(betaRad)*cos(alphaRad), yPos+sin(betaRad), zPos-cos(betaRad)*sin(alphaRad),
				0.0f, 1.0f, 0.0f);
	//drawAxis();

	if (renderMode == RENDER_FORWARD){
		// Set mode
		gBufferDS->bind(GBUFFER_DEFAULT);
		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(shaderProgram);

		// set lighting texture buffer
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_BUFFER, LTB);
		glUniform1i(lightsTexBufferLocation, 1);

		setLightUniforms();
		mainMesh->Render(positionLocation, texCoordLocation, normLocation, samplerLocation);

		//Draw ligh billboards
		if (lBillboards) {
			glUseProgram(0);
			for (unsigned int i = 0; i < nLights; i++) drawLightBillboard(pointLightsArr[i], 0.15f);
		}		
	} 
	else if (renderMode == RENDER_FORWARD_BLEND){
 		// Set mode
 		gBufferDS->bind(GBUFFER_DEFAULT);
 		glDepthMask(GL_TRUE);
 		glEnable(GL_DEPTH_TEST);
 		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 
 		glUseProgram(shaderProgramBlend);
		if (nLights < 100) {
			setLightUniformsBlend(0, nLights, 0.0f);
			mainMesh->Render(positionBlendLocation, texCoordBlendLocation, normBlendLocation, samplerBlendLocation);
		}
		else {
			unsigned int l_interval = 0;
			unsigned int h_interval = FORWARD_LIGHTS_INTERVAL;
			// First Render
			setLightUniformsBlend(l_interval, h_interval, 0.0f);
			mainMesh->Render(positionBlendLocation, texCoordBlendLocation, normBlendLocation, samplerBlendLocation);

			l_interval = h_interval;
			h_interval += FORWARD_LIGHTS_INTERVAL;
			if (h_interval > nLights) h_interval = nLights;

			glEnable(GL_BLEND);
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_ONE, GL_ONE);
			glDepthMask(GL_FALSE);

			// Blend the rest of the lights
			while (l_interval < nLights) {
				setLightUniformsBlend(l_interval, h_interval, -0.00001f);
				mainMesh->Render(positionBlendLocation, texCoordBlendLocation, normBlendLocation, samplerBlendLocation);
				l_interval = h_interval;
				h_interval += FORWARD_LIGHTS_INTERVAL;
				if (h_interval > nLights) h_interval = nLights;
			}
			glDisable(GL_BLEND);
			glDepthMask(GL_TRUE);
		}
 
 		//Draw ligh billboards
 		if (lBillboards) {
 			glUseProgram(0);
 			for (unsigned int i = 0; i < nLights; i++) drawLightBillboard(pointLightsArr[i], 0.15f);
 		}
	}
	else if (renderMode == RENDER_DEFERRED){
		if (lBillboards) { // We need to safe the z-buffer in order to correctly draw the billboards later
			glUseProgram(0);
			dBufferDS->bind();

			glDepthMask(GL_TRUE);
			glEnable(GL_DEPTH_TEST);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			mainMesh->simpleRender();

			dBufferDS->unBind();
		}
		// Set Mode
		glUseProgram(shaderProgramDeferredGeo);
		gBufferDS->bind(GBUFFER_DRAW);
		glDepthMask(GL_TRUE);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);	// we need depth test only here in order to get only the closest pixels on the Gbuffers
		glDisable(GL_BLEND);		// we dont need blending on the geo pass

		// Draw Geometry
		mainMesh->Render(positionDeferredGeoLocation, texCoordDeferredGeoLocation, normDeferredGeoLocation, samplerDeferredGeoLocation);

		glDepthMask(GL_FALSE);
		glDisable(GL_DEPTH_TEST);
		// setup light pass + bucle for each light set uniforms and draw
		
		glUseProgram(shaderProgramDeferredLight);
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);

		gBufferDS->bind(GBUFFER_READ_TEX);
		glClear(GL_COLOR_BUFFER_BIT);

		setLightPassUniforms();
		
		// Draw Lights
		for (unsigned int i = 0; i < nLights; i++) drawPointLight(pointLightsArr[i]);

		glUseProgram(shaderProgramDeferredDirectionalLight);
		directionalLightPass();

		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		
		if (lBillboards) {
			// Copy z-buffer from the gbuffer
			glClear(GL_DEPTH_BUFFER_BIT);
			glUseProgram(shaderProgramDepthSet);
			glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

			dBufferDS->bindTex();
			glUniform1i(depthSetSamplerLocation, 0);

			glBegin(GL_TRIANGLE_STRIP);
			glVertex2f(-1.0f,-1.0f);
			glVertex2f(1.0f,-1.0f);
			glVertex2f(-1.0f,1.0f);
			glVertex2f(1.0f,1.0f);
			glEnd();

			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			glDepthMask(GL_FALSE);

			glUseProgram(0);
			for (unsigned int i = 0; i < nLights; i++) drawLightBillboard(pointLightsArr[i], 0.15f);

			glDepthMask(GL_TRUE);
		}
	}
	else if (renderMode == RENDER_FORWARD_PLUS) {
		gBufferDS->bind(GBUFFER_DEFAULT);
		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(shaderProgramForwardPlus);

		updateLightsMatrix();

		setForwardPlusUniforms();
		mainMesh->Render(positionForwardPlusLocation, texCoordForwardPlusLocation, normForwardPlusLocation, samplerForwardPlusLocation);
	}
	else if (renderMode == RENDER_DEPTH) {
		DrawDepthPrepass();
	}
	else if (renderMode == RENDER_GRID) {
		updateLightsMatrix();
		DrawLightsGrid();
	}
	else if (renderMode >= RENDER_POSITION && renderMode <= RENDER_ALL) {
		// Set Mode
		glUseProgram(shaderProgramDeferredDebug);
		gBufferDS->bind(GBUFFER_DRAW);
		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		BoundingBox bb = mainMesh->getBoundingBox();
		glUniform3f(minPDeferredDebugLocation, bb.min.x, bb.min.y, bb.min.z);
		glUniform3f(maxPDeferredDebugLocation, bb.max.x, bb.max.y, bb.max.z);

		// Draw Geometry
		mainMesh->Render(positionDeferredDebugLocation, texCoordDeferredDebugLocation, normDeferredDebugLocation, samplerDeferredDebugLocation);

		// Copy G-buffer to main framebuffer
		gBufferDS->bind(GBUFFER_DEFAULT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		gBufferDS->bind(GBUFFER_READ);
		if (renderMode == RENDER_ALL) renderAll(width(),height());
		else {
			glReadBuffer(GL_COLOR_ATTACHMENT0+renderMode);
			glBlitFramebuffer(0, 0, width(), height(), 0, 0, width(), height(), GL_COLOR_BUFFER_BIT, GL_LINEAR);
		}
	}
	updateFPS();
}

void GLWidget::drawPointLight(pointLight l)
{
	float r = l.intensity.i_;
	glUniform1f(pLightRadiusDeferredLightLocation, r);
	glUniform3f(pLightColorDeferredLightLocation, l.color.r, l.color.g, l.color.b);
	glUniform1f(pLightIntensityDeferredLightLocation, l.intensity.i);
	glUniform3f(pLightPositionDeferredLightLocation, l.position.x, l.position.y, l.position.z);
	glUniform3f(pLightAttenuationDeferredLightLocation, l.attenuation.constant, l.attenuation.linear, l.attenuation.exp);
	glPushMatrix();
	glTranslatef(l.position.x, l.position.y, l.position.z);
	utils::drawSphere(r,16,16);
	glPopMatrix();
}

void GLWidget::drawLightBillboard(pointLight l, float w)
{
	glm::vec3 n = glm::vec3(xPos-l.position.x, yPos-l.position.y, zPos-l.position.z);
	n = glm::normalize(n);

	float gl_ModelViewMatrix[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, gl_ModelViewMatrix);
	glm::vec3 u;
	if ((glm::abs(n.y) > glm::abs(n.x)) && (glm::abs(n.y) > glm::abs(n.z))) {
		glm::mat4 m = glm::mat4(gl_ModelViewMatrix[0],gl_ModelViewMatrix[1],gl_ModelViewMatrix[2],gl_ModelViewMatrix[3],gl_ModelViewMatrix[4],gl_ModelViewMatrix[5],gl_ModelViewMatrix[6],gl_ModelViewMatrix[7],gl_ModelViewMatrix[8],gl_ModelViewMatrix[9],gl_ModelViewMatrix[10],gl_ModelViewMatrix[11],gl_ModelViewMatrix[12],gl_ModelViewMatrix[13],gl_ModelViewMatrix[14],gl_ModelViewMatrix[15]);
		glm::mat4 gl_ModelViewMatrixInverse = glm::inverse(m);
		glm::vec4 ox = gl_ModelViewMatrixInverse*glm::vec4(1.0,0.0,0.0,1.0);
		u = glm::cross(n,ox.xyz());
	}
	else {
		u = glm::cross(glm::vec3(0.0,1.0,0.0),n);
	}
	u = glm::normalize(u);

	glm::vec3 v = glm::cross(n,u);
	v = glm::normalize(v);

	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	lightBillboard->Bind(GL_TEXTURE0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor3f(l.color.r,l.color.g,l.color.b);

	glPushMatrix();
	glTranslatef(l.position.x, l.position.y, l.position.z);
	glBegin(GL_QUADS);
	glTexCoord2f(0,1);
	glm::vec3 p = (w/2)*u+(w/2)*v;
	glVertex3f(p.x,p.y,p.z);
	p = -(w/2)*u+(w/2)*v;
	glTexCoord2f(0,0);
	glVertex3f(p.x,p.y,p.z);
	p = -(w/2)*u-(w/2)*v;
	glTexCoord2f(1,0);
	glVertex3f(p.x,p.y,p.z);
	p = (w/2)*u-(w/2)*v;
	glTexCoord2f(1,1);
	glVertex3f(p.x,p.y,p.z);
	glEnd();
	glPopMatrix();

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);	
}
/*
uniform samplerBuffer scanSum;
uniform int maxTileLights;
uniform vec2 screenSize;

	GLuint scanSumForwardDebugLocation;
	GLuint maxLightsForwardDebugLocation;
	GLuint screenSizeForwardDebugLocation;
	std::vector<int> lightsScanSum;
*/
void GLWidget::DrawLightsGrid()
{
	// Populate texture buffer
	int siz = lightsScanSum.size();
	GLuint TBSS;
	glGenBuffers(1,&TBSS);
	glBindBuffer(GL_TEXTURE_BUFFER,TBSS);
	glBufferData(GL_TEXTURE_BUFFER,siz*sizeof(int),&lightsScanSum[0],GL_STATIC_COPY);

	GLuint LTBSS;
	glGenTextures(1,&LTBSS);
	glBindTexture(GL_TEXTURE_BUFFER,LTBSS);
	glTexBuffer(GL_TEXTURE_BUFFER,GL_R32I,TBSS);

	glDeleteBuffers(1,&TBSS);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shaderProgramForwardPlusDebug);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_BUFFER, LTBSS);
	glUniform1i(scanSumForwardDebugLocation, 1);
	glUniform1i(maxLightsForwardDebugLocation, nLights);
	glUniform2f(screenSizeForwardDebugLocation, width(), height());

	glBegin(GL_TRIANGLE_STRIP);
	glVertex2f(-1.0f,-1.0f);
	glVertex2f(1.0f,-1.0f);
	glVertex2f(-1.0f,1.0f);
	glVertex2f(1.0f,1.0f);
	glEnd();
}

void GLWidget::DrawDepthPrepass()
{
		glUseProgram(0);
		dBufferFR->bind();

		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		mainMesh->simpleRender();

		dBufferFR->unBind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(shaderProgramForwardDepthDebug);

		dBufferFR->bindTex();
		glUniform1i(depthDebugTextureLocation, 0);

		glBegin(GL_TRIANGLE_STRIP);
		glVertex2f(-1.0f,-1.0f);
		glVertex2f(1.0f,-1.0f);
		glVertex2f(-1.0f,1.0f);
		glVertex2f(1.0f,1.0f);
		glEnd();
}

void GLWidget::directionalLightPass()
{
	// Ambient Light
	glUniform3f(ambientColorDirectionalLocation, aLight.color.r, aLight.color.g, aLight.color.b);
	glUniform1f(ambientIntensityDirectionalLocation,aLight.intensity);
	// Directioal Light
	glUniform3f(directionalColorDirectionalLocation, dLight.color.r, dLight.color.g, dLight.color.b);
	glUniform1f(directionalIntisityDirectionalLocation, dLight.intensity);
	glUniform3f(directionaldirectionDirectionalLocation, dLight.direction.x, dLight.direction.y, dLight.direction.z);
	// G-Buffer
	glUniform1i(normalBufferDirectionalLocation, gbuffer::GBUFFER_NORMAL);
	glUniform1i(diffuseBufferDirectionalLocation, gbuffer::GBUFFER_DIFFUSE);

	// Draw
	glBegin(GL_TRIANGLE_STRIP);
	glVertex2f(-1.0f,-1.0f);
	glVertex2f(1.0f,-1.0f);
	glVertex2f(-1.0f,1.0f);
	glVertex2f(1.0f,1.0f);
	glEnd();
}

void GLWidget::updateLightsMatrix()
{
	/*
		gluLookAt(	xPos, yPos, zPos,
				xPos+cos(betaRad)*cos(alphaRad), yPos+sin(betaRad), zPos-cos(betaRad)*sin(alphaRad),
				0.0f, 1.0f, 0.0f);
				*/
	const float PI = 3.1415927f;
	float alphaRad = PI*alpha/180;
	float betaRad = PI*beta/180;
	glm::vec3 right = glm::vec3(cos(betaRad)*cos(alphaRad), sin(betaRad), -cos(betaRad)*sin(alphaRad));
	right = glm::normalize(right);
	right = glm::cross(right,glm::vec3(0.0,1.0,0.0));
	right = glm::normalize(right);

	float gl_ModelViewMatrix[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, gl_ModelViewMatrix);
	glm::mat4 m = glm::mat4(gl_ModelViewMatrix[0],gl_ModelViewMatrix[1],gl_ModelViewMatrix[2],gl_ModelViewMatrix[3],gl_ModelViewMatrix[4],gl_ModelViewMatrix[5],gl_ModelViewMatrix[6],gl_ModelViewMatrix[7],gl_ModelViewMatrix[8],gl_ModelViewMatrix[9],gl_ModelViewMatrix[10],gl_ModelViewMatrix[11],gl_ModelViewMatrix[12],gl_ModelViewMatrix[13],gl_ModelViewMatrix[14],gl_ModelViewMatrix[15]);

	float gl_ProjectionMatrix[16];
	glGetFloatv(GL_PROJECTION_MATRIX, gl_ProjectionMatrix);
	glm::mat4 proj = glm::mat4(gl_ProjectionMatrix[0],gl_ProjectionMatrix[1],gl_ProjectionMatrix[2],gl_ProjectionMatrix[3],gl_ProjectionMatrix[4],gl_ProjectionMatrix[5],gl_ProjectionMatrix[6],gl_ProjectionMatrix[7],gl_ProjectionMatrix[8],gl_ProjectionMatrix[9],gl_ProjectionMatrix[10],gl_ProjectionMatrix[11],gl_ProjectionMatrix[12],gl_ProjectionMatrix[13],gl_ProjectionMatrix[14],gl_ProjectionMatrix[15]);

	clearLigthsMatrix();
	for (unsigned int i = 0; i < nLights; i++) {
		glm::vec4 c = glm::vec4(pointLightsArr[i].position.x, pointLightsArr[i].position.y, pointLightsArr[i].position.z,1.0);
		float r = utils::calcLightRadius(pointLightsArr[i], threshold);
		glm::vec4 p = c+r*glm::vec4(right,1.0);

		glm::vec4 cp = proj*m*c;
		cp = cp/cp.w;
		cp.x = width()*(cp.x+1)/2;
		cp.y = height()*(cp.y+1)/2;
		glm::vec4 pp = proj*m*p;
		pp = pp/pp.w;
		pp.x = width()*(pp.x+1)/2;
		pp.y = height()*(pp.y+1)/2;
		float pRadius = glm::length(cp.xy-pp.xy);

		float x1,x2,y1,y2;
		for (unsigned int j = 0; j < gLightsRow; j++) {
			for (unsigned int k = 0; k < gLightsCol; k++) {
				x1 = k*GRID_RES; x2 = (k+1)*GRID_RES;
				y1 = j*GRID_RES; y2 = (j+1)*GRID_RES;
				if (cp.x >= x1 && cp.x < x2) {
					if (cp.y >= y1 && cp.y < y2) { //center inside tile
						lightsMatrix[j*gLightsCol+k].push_back(i);
					} 
					else if (cp.y > y2) { // down (r> y-y2)
						if (pRadius > abs(cp.y-y2)) lightsMatrix[j*gLightsCol+k].push_back(i);
					} 
					else if (cp.y < y1) { // up (r> y-y1)
						if (pRadius > abs(cp.y-y1)) lightsMatrix[j*gLightsCol+k].push_back(i);
					} 
				} 
				else if (cp.y >= y1 && cp.y < y2) { 
					if (cp.x < x1) { // left
						if (pRadius > abs(cp.x-x1)) lightsMatrix[j*gLightsCol+k].push_back(i);
					}
					else if (cp.x > x2) { // right
						if (pRadius > abs(cp.x-x2)) lightsMatrix[j*gLightsCol+k].push_back(i);
					}
				}
				else if (cp.x < x1 && cp.y < y1) { // upper-left
					if (pRadius > sqrt((cp.x-x1)*(cp.x-x1)+(cp.y-y1)*(cp.y-y1))) lightsMatrix[j*gLightsCol+k].push_back(i);
				} 
				else if (cp.x > x2 && cp.y < y1) { // upper-right
					if (pRadius > sqrt((cp.x-x2)*(cp.x-x2)+(cp.y-y1)*(cp.y-y1))) lightsMatrix[j*gLightsCol+k].push_back(i);
				}
				else if (cp.x < x1 && cp.y > y2) { // down-left
					if (pRadius > sqrt((cp.x-x1)*(cp.x-x1)+(cp.y-y2)*(cp.y-y2))) lightsMatrix[j*gLightsCol+k].push_back(i);
				}
				else if (cp.x > x2 && cp.y > y2) { // down-right
					if (pRadius >sqrt((cp.x-x2)*(cp.x-x2)+(cp.y-y2)*(cp.y-y2))) lightsMatrix[j*gLightsCol+k].push_back(i);
				}
			}
		}
	}

	lightsScanSum.clear();
	int scan = 0;
	maxTileLights = 0;
	for (unsigned int j = 0; j < gLightsRow; j++) {
		for (unsigned int k = 0; k < gLightsCol; k++) {
			if (lightsMatrix[j*gLightsCol+k].size() > maxTileLights) maxTileLights = lightsMatrix[j*gLightsCol+k].size();
			scan += lightsMatrix[j*gLightsCol+k].size();
			lightsScanSum.push_back(scan);
		}
	}

	int i = 0;
	for (std::vector<std::vector<int> >::iterator it = lightsMatrix.begin(); it != lightsMatrix.end(); ++it) {
		for (std::vector<int>::iterator it2 = it->begin(); it2 != it->end(); ++it2) {
			arr[i] = *it2;
			++i;
		}
	}

	// Populate texture buffer
	GLuint TB[2];
	glGenBuffers(2,TB);

	glBindBuffer(GL_TEXTURE_BUFFER,TB[0]);
	glBufferData(GL_TEXTURE_BUFFER,sizeof(arr),&arr[0],GL_DYNAMIC_COPY);

	glBindTexture(GL_TEXTURE_BUFFER,LGTB);
	glTexBuffer(GL_TEXTURE_BUFFER,GL_R32I,TB[0]);
	
	int siz = lightsScanSum.size();
	glBindBuffer(GL_TEXTURE_BUFFER,TB[1]);
	glBufferData(GL_TEXTURE_BUFFER,siz*sizeof(int),&lightsScanSum[0],GL_DYNAMIC_COPY);

	glBindTexture(GL_TEXTURE_BUFFER,SSTB);
	glTexBuffer(GL_TEXTURE_BUFFER,GL_R32I,TB[1]);

	glDeleteBuffers(2,TB);
}

void GLWidget::clearLigthsMatrix(){
	for (unsigned int i = 0; i < lightsMatrix.size(); i++) {
		lightsMatrix[i].clear();
	}
}

void GLWidget::updateFPS()
{
	if(frames >= 100) {
		int time = t->restart();
		fps = 100000/time;
		emit updateFPSSignal(fps);
		frames = 0;
	} else {
		frames++;
	}
}

void GLWidget::mousePressEvent(QMouseEvent *e)
{
    lastPos = e->pos();
}

void GLWidget::mouseMoveEvent(QMouseEvent * e)
{
    int dx = e->x() - lastPos.x();
    int dy = e->y() - lastPos.y();

    if (e->buttons() & Qt::LeftButton) {
		alpha -= dx*cSensitivity;
		beta -= dy*cSensitivity;
		if (beta > 90) beta = 90;
		else if (beta < -90) beta = -90;
    }
    lastPos = e->pos();
	timerEvent(NULL);
}

void GLWidget::addKey(int k)
{
	keys += k;
}

void GLWidget::removeKey(int k)
{
	keys -= k;
}

void GLWidget::modifyCameraSensitivity(QString s)
{
	cSensitivity = s.toFloat();
}

void GLWidget::modifyCameraSpeed(QString s)
{
	cSpeed = s.toFloat();
}

void GLWidget::modifyMaxIntensity(QString s)
{
	maxIntensity = s.toFloat();
}

void GLWidget::modifyThreshold(QString s)
{
	threshold = s.toInt();

	// update light radius
	for (unsigned int i = 0; i < nLights; i++){
		pointLightsArr[i].intensity.i_ = utils::calcLightRadius(pointLightsArr[i], threshold);
	}

	GLuint TB;
	glGenBuffers(1,&TB);
	glBindBuffer(GL_TEXTURE_BUFFER,TB);
	glBufferData(GL_TEXTURE_BUFFER,nLights*sizeof(pointLight),&pointLightsArr[0],GL_STATIC_COPY);

	glBindTexture(GL_TEXTURE_BUFFER,LTB);
	glTexBuffer(GL_TEXTURE_BUFFER,GL_RGB32F,TB);

	glDeleteBuffers(1,&TB);
}

void GLWidget::modifyConstantAttenuation(QString s)
{
	constAtt= s.toFloat();
}

void GLWidget::modifyLinearAttenuation(QString s)
{
	linearAtt= s.toFloat();
}

void GLWidget::modifyExpAttenuation(QString s)
{
	expAtt = s.toFloat();
}

void GLWidget::modifyBoundingBoxScale(QString s)
{
	lightingBoundingBoxScale = s.toFloat();
}

void GLWidget::enableBillboards(int s)
{
	if (s == Qt::Unchecked) lBillboards = false;
	else if (s == Qt::Checked) lBillboards = true;
}

void GLWidget::timerEvent(QTimerEvent* e)
{
	if (e && e->timerId() == inputTimerId) {
		const float PI = 3.1415927f;
		float alphaRad = PI*alpha/180;
		float betaRad = PI*beta/180;

		if (keys.contains(Qt::Key_W)){
			xPos += cSpeed*cos(betaRad)*cos(alphaRad);
			yPos += cSpeed*sin(betaRad);
			zPos -= cSpeed*cos(betaRad)*sin(alphaRad);
		}
		if (keys.contains(Qt::Key_A)) {
			xPos -= cSpeed*sin(alphaRad);
			zPos -= cSpeed*cos(alphaRad);
		}
		if (keys.contains(Qt::Key_S)){
			xPos -= cSpeed*cos(betaRad)*cos(alphaRad);
			yPos -= cSpeed*sin(betaRad);
			zPos += cSpeed*cos(betaRad)*sin(alphaRad);
		}
		if (keys.contains(Qt::Key_D)) {
			xPos += cSpeed*sin(alphaRad);
			zPos += cSpeed*cos(alphaRad);
		}
	}
	else if (e && e->timerId() == drawTimerId) updateGL();
}
