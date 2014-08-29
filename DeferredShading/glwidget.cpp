#include "glwidget.h"
#include <gl\GLU.h>
#include <GL\GL.h>
#include <fstream>
#include <iostream>

const char* VSPath = "shader.vs";
const char* FSPath = "shader.fs";
//GLuint VBO;

GLWidget::GLWidget(QWidget *parent) :
    QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{
	xPos=zPos=0.0f;
	yPos=1.0f;
	alpha=beta=0.0f;
	cSpeed = cSensivility = 0.1f;
	cSpeed = 1.0f;
	startTimer(1000/60); //60FPS for camera movement
	setMouseTracking(true);
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

void GLWidget::initializeLighting()
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

void GLWidget::initializeGL()
{
    qglClearColor(QColor::fromRgb(0,0,0));

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    //glShadeModel(GL_SMOOTH);
	//initializeLighting();
	initializeShaders();

	// Load mesh from file
	loadModel("../DeferredShading/Models/sponza/sponza.obj");
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
	glEnd();

	// Draw geometry
	glFuncs.glUseProgram(shaderProgram);
    mainMesh->Render(shaderProgram);
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
		alpha -= dx*cSensivility;
		beta -= dy*cSensivility;
		if (beta > 90) beta = 90;
		else if (beta < -90) beta = -90;
    }
    lastPos = e->pos();
	timerEvent(NULL);

	if (!(keys.contains(Qt::Key_W) || keys.contains(Qt::Key_A) || 
		keys.contains(Qt::Key_S) || keys.contains(Qt::Key_D))) 
		updateGL();
}

void GLWidget::addKey(int k)
{
	keys += k;
}

void GLWidget::removeKey(int k)
{
	keys -= k;
}

void GLWidget::timerEvent(QTimerEvent* e)
{
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

	if (keys.contains(Qt::Key_W) || keys.contains(Qt::Key_A) || 
		keys.contains(Qt::Key_S) || keys.contains(Qt::Key_D)) 
		updateGL();
}