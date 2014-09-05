#include "glwidget.h"
#include <gl\GLU.h>
#include <GL\GL.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <random>

const char* VSPath = "shader.vs";
const char* FSPath = "shader.fs";

GLWidget::GLWidget(QWidget *parent) :
    QGLWidget()
{
	setFocusPolicy(Qt::ClickFocus);
	setMouseTracking(true);
	xPos=zPos=0.0f;
	yPos=1.0f;
	alpha=beta=0.0f;
	cSensitivity = 0.1f;
	cSpeed = 1.0f;
	nLights = 0;
	inputTimerId = startTimer(1000/60); //30FPS for camera movement
	drawTimerId = startTimer(0); // render at max fps, must turn off v-sync
}

GLWidget::~GLWidget()
{
}

static void qNormalizeAngle(int &angle)
{
    while (angle < 0)
        angle += 360 * 16;
    while (angle > 360 * 16)
        angle -= 360 * 16;
}

void GLWidget::loadModel(std::string path)
{
	modelPath = path;
	mainMesh = new Mesh();
    mainMesh->LoadMesh(modelPath);
}

void GLWidget::initializeLightingGL()
{
	glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	//GLfloat light_ambient[4] = { 1.0, 1.0, 1.0, 0.1 };
	GLfloat light_diffuse1[4] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_diffuse2[4] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_specular[4] = { 1.0, 1.0, 1.0, 1.0 };
    //static GLfloat lightPosition[4] = { 0.5, 5.0, 7.0, 1.0};
	static GLfloat lightPosition[4] = { -100.0, 1.0, 10.0, 1.0};
	static GLfloat lightPosition2[4] = { 100.0, 1.0, 10.0, 1.0};
	//glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse1);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 0.3f);
	//glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse2);
	glLightfv(GL_LIGHT1, GL_POSITION, lightPosition2);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);
	glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 0.3f);
}

bool GLWidget::readFile(const char* path, std::string& out)
{
	std::string line;
	std::ifstream inFile(path);

	if (inFile.is_open()) {
		while (getline(inFile,line)) {
			out.append(line);
			out.append("\n");
		}
		inFile.close();
	} else {
		return false;
	}
	return true;
}

void GLWidget::initializeShaders()
{
	QGLFunctions glFuncs(QGLContext::currentContext());
	shaderProgram = glFuncs.glCreateProgram();

	// Vertex shader load
	std::string vertexShader;

	if (!readFile(VSPath, vertexShader)) exit(1);
	
	GLuint vShaderObj = glFuncs.glCreateShader(GL_VERTEX_SHADER);

	const char* shaderFiles[1];
	shaderFiles[0] = vertexShader.c_str();
	GLint lengths[1];
	lengths[0] = vertexShader.size();
	glFuncs.glShaderSource(vShaderObj, 1, shaderFiles, lengths);
	glFuncs.glCompileShader(vShaderObj);
	GLint success;
	glFuncs.glGetShaderiv(vShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		char InfoLog[1024];
		glFuncs.glGetShaderInfoLog(vShaderObj, sizeof(InfoLog), NULL, InfoLog);
		fprintf(stderr, "Error compiling shader: '%s'\n", InfoLog);
	}

	glFuncs.glAttachShader(shaderProgram,vShaderObj);

	// Fragment shader load
	std::string fragmentShader;

	if (!readFile(FSPath, fragmentShader)) exit(1);
	
	GLuint fShaderObj = glFuncs.glCreateShader(GL_FRAGMENT_SHADER);

	shaderFiles[0] = fragmentShader.c_str();
	lengths[0] = fragmentShader.size();
	glFuncs.glShaderSource(fShaderObj, 1, shaderFiles, lengths);
	glFuncs.glCompileShader(fShaderObj);
	glFuncs.glGetShaderiv(fShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		char InfoLog[1024];
		glFuncs.glGetShaderInfoLog(fShaderObj, sizeof(InfoLog), NULL, InfoLog);
		fprintf(stderr, "Error compiling shader: '%s'\n", InfoLog);
	}

	glFuncs.glAttachShader(shaderProgram,fShaderObj);
	glFuncs.glLinkProgram(shaderProgram);
	glFuncs.glGetProgramiv(shaderProgram,GL_LINK_STATUS,&success);
	glFuncs.glValidateProgram(shaderProgram);
	glFuncs.glGetProgramiv(shaderProgram,GL_VALIDATE_STATUS,&success);
	glFuncs.glUseProgram(shaderProgram);
}

void GLWidget::initializeLighting()
{
	std::default_random_engine generator((unsigned int)time(0));

	// -----sponza settings-----
	/*
	std::uniform_real_distribution<float> distributionX(-17.0,17.0);
	std::uniform_real_distribution<float> distributionY(-1.0,15.0);
	std::uniform_real_distribution<float> distributionZ(-7.0,7.0);
	std::uniform_real_distribution<float> distributionC(0.0,1.0);
	std::uniform_real_distribution<float> distributionI(0.0,2.0);
	float pAttenuation[] = {1.0f, 0.2f, 0.5f};
	*/
	// -----sponza-crytek settings-----
	
	std::uniform_real_distribution<float> distributionX(-1400.0,1400.0);
	std::uniform_real_distribution<float> distributionY(-125.0,1200.0);
	std::uniform_real_distribution<float> distributionZ(-700.0,700.0);
	std::uniform_real_distribution<float> distributionC(0.0,1.0);
	std::uniform_real_distribution<float> distributionI(0.0,1000.0);
	
	float pAttenuation[] = {2.5f, 5.0f, 0.015f};
	nLights = 100;
	
	for (unsigned int i = 0; i < nLights; i++){
		float pColor[] = {distributionC(generator), distributionC(generator), distributionC(generator)};
		float pPosition[] = {distributionX(generator),distributionY(generator),distributionZ(generator)};
		float pIntensity = distributionI(generator);
		pointLightsArr[i] = pointLight(pColor, pIntensity, pPosition, pAttenuation);
	}
	
	float wColor[] = {1.0f,1.0f,1.0f};
	float dDirection[] = {-0.577f,-0.577f,-0.577f};
	
	aLight = ambientLight(wColor, 0.1f);
	dLight = directionalLight(wColor, 0.0f, dDirection); 
	/*
	float rColor[] = {1.0f,0.0f,0.0f};
	float pos0[] = {0.0f,1.0f,1.0f};
	pointLightsArr[0] = pointLight(rColor, 1.0f, pos0, pAttenuation); 
	
	float gColor[] = {0.0f,1.0f,0.0f};
	float bColor[] = {0.0f,0.0f,1.0f};
	
	float pos1[] = {1.0f,1.0f,0.0f};
	float pos2[] = {-1.0f,1.0f,0.0f};
	float attenuation[] = {1.0f, 0.5f, 0.2f};
	
	
	pointLightsArr[1] = pointLight(gColor, 1.0f, pos1, attenuation);
	pointLightsArr[2] = pointLight(bColor, 1.0f, pos2, attenuation);
	*/
}

void GLWidget::initializeGL()
{
    qglClearColor(QColor::fromRgb(0,0,0));

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    //glShadeModel(GL_SMOOTH);
	//initializeLightingGL();
	initializeShaders();
	initLocations();
	initializeLighting();
	

	// Load mesh from file
	loadModel("../DeferredShading/Models/sponza/sponza.obj");
	frames = 0;
	t = new QTime();
	t->start();
}

void GLWidget::resizeGL(int width, int height)
{
    int side = qMin(width, height);
    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60,width/height,1.0,10000.0);
    glMatrixMode(GL_MODELVIEW);
}

void GLWidget::initLocations()
{
	QGLFunctions glFuncs(QGLContext::currentContext());

    positionLocation = glFuncs.glGetAttribLocation(shaderProgram,"position");
	texCoordLocation = glFuncs.glGetAttribLocation(shaderProgram,"texCoord");
	normLocation = glFuncs.glGetAttribLocation(shaderProgram,"norm");
	samplerLocation = glFuncs.glGetUniformLocation(shaderProgram,"sampler");
	ambientColorLocation = glFuncs.glGetUniformLocation(shaderProgram,"aLight.color");
	ambientIntensityLocation = glFuncs.glGetUniformLocation(shaderProgram,"aLight.intensity");
	directionalColorLocation = glFuncs.glGetUniformLocation(shaderProgram,"dLight.color");
	directionalIntensityLocation = glFuncs.glGetUniformLocation(shaderProgram,"dLight.intensity");
	directionalDirectionLocation = glFuncs.glGetUniformLocation(shaderProgram,"dLight.direction");
	std::stringstream sstm;
	std::string uniformName;
	std::string pl_str = "pointLights[";
	std::string color_str = "].color";
	std::string intensity_str = "].intensity";
	std::string position_str = "].position";
	std::string attenuation_str = "].attenuation";
	std::string str_arr[] = {color_str, intensity_str, position_str, attenuation_str};
	for (unsigned int i = 0; i < N_MAX_LIGHTS; i++) {
		for (unsigned int j = 0; j < 4; j++) {
			sstm << pl_str << i << str_arr[j];
			uniformName = sstm.str();
			if (j == 0) pointLightLocations[i].color = glFuncs.glGetUniformLocation(shaderProgram,uniformName.c_str());
			else if (j == 1) pointLightLocations[i].intensity = glFuncs.glGetUniformLocation(shaderProgram,uniformName.c_str());
			else if (j == 2) pointLightLocations[i].position = glFuncs.glGetUniformLocation(shaderProgram,uniformName.c_str());
			else if (j == 3) pointLightLocations[i].attenuation = glFuncs.glGetUniformLocation(shaderProgram,uniformName.c_str());
			sstm.str(std::string());
			sstm.clear();
		}

		/*
		sstm << pl_str << i << intensity_str;
		uniformName = sstm.str();
		pointLightLocations[i].color = glFuncs.glGetUniformLocation(shaderProgram,uniformName.c_str());
		sstm.str(std::string());
		sstm.clear();

		sstm << pl_str << i << position_str;
		uniformName = sstm.str();
		pointLightLocations[i].color = glFuncs.glGetUniformLocation(shaderProgram,uniformName.c_str());
		sstm.str(std::string());
		sstm.clear();

		sstm << pl_str << i << attenuation_str;
		uniformName = sstm.str();
		pointLightLocations[i].color = glFuncs.glGetUniformLocation(shaderProgram,uniformName.c_str());
		sstm.str(std::string());
		sstm.clear();
		*/
	}
	nLightsLocation = glFuncs.glGetUniformLocation(shaderProgram,"nLights");
}

void GLWidget::setLightUniforms()
{
	QGLFunctions glFuncs(QGLContext::currentContext());

	// Ambient light parameters
	glFuncs.glUniform3f(ambientColorLocation, aLight.color.r, aLight.color.g, aLight.color.b);
	glFuncs.glUniform1f(ambientIntensityLocation,aLight.intensity);

	// Directional light parameters
	glFuncs.glUniform3f(directionalColorLocation, dLight.color.r, dLight.color.g, dLight.color.b);
	glFuncs.glUniform1f(directionalIntensityLocation,dLight.intensity);
	glFuncs.glUniform3f(directionalDirectionLocation,dLight.direction.x,dLight.direction.y,dLight.direction.z); // must to be normilized

	for (unsigned int i = 0; i < nLights; ++i) {
		glFuncs.glUniform3f(pointLightLocations[i].color, pointLightsArr[i].color.r, pointLightsArr[i].color.g, pointLightsArr[i].color.b);
		glFuncs.glUniform1f(pointLightLocations[i].intensity, pointLightsArr[i].intensity);
		glFuncs.glUniform3f(pointLightLocations[i].position, pointLightsArr[i].position.x, pointLightsArr[i].position.y, pointLightsArr[i].position.z);
		glFuncs.glUniform3f(pointLightLocations[i].attenuation, pointLightsArr[i].attenuation.constant, pointLightsArr[i].attenuation.linear, pointLightsArr[i].attenuation.exp);
	}
	glFuncs.glUniform1i(nLightsLocation, nLights);

}

void GLWidget::paintGL()
{
	const float PI = 3.1415927f;
	float alphaRad = PI*alpha/180;
	float betaRad = PI*beta/180;
	QGLFunctions glFuncs(QGLContext::currentContext());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
	gluLookAt(xPos, yPos, zPos,
		xPos+cos(betaRad)*cos(alphaRad), yPos+sin(betaRad), zPos-cos(betaRad)*sin(alphaRad),
		0.0f, 1.0f, 0.0f);
	//glScalef(10,10,10);

	// Draw axis
	glFuncs.glUseProgram(0);
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
		glVertex3f(0.0f,0.0f,0.0f);
		glVertex3f(100.0f,100.0f,100.0f);
	glEnd();

	// Draw geometry
	glFuncs.glUseProgram(shaderProgram);
	setLightUniforms();
    mainMesh->Render(positionLocation, texCoordLocation, normLocation, samplerLocation);
	
	updateFPS();
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
	/*
	if (!(keys.contains(Qt::Key_W) || keys.contains(Qt::Key_A) || 
		keys.contains(Qt::Key_S) || keys.contains(Qt::Key_D))) 
		updateGL();
	*/
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
	/*
	if (keys.contains(Qt::Key_W) || keys.contains(Qt::Key_A) || 
		keys.contains(Qt::Key_S) || keys.contains(Qt::Key_D)) 
		updateGL();
	*/
	else updateGL();
}