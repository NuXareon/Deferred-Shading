#include <QMouseEvent>
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
    xRot = 0;
    yRot = 0;
    zRot = 0;
	Zoom = 0;
}

GLWidget::~GLWidget()
{
}

QSize GLWidget::minimumSizeHint() const
{
    return QSize(50,50);
}

QSize GLWidget::sizeHint() const
{
    return QSize(400,400);
}

static void qNormalizeAngle(int &angle)
{
    while (angle < 0)
        angle += 360 * 16;
    while (angle > 360 * 16)
        angle -= 360 * 16;
}

void GLWidget::setXRotation(int angle)
{
    qNormalizeAngle(angle);
    if (angle != xRot) {
        xRot = angle;
        emit xRotationChanged(angle);
        updateGL();
    }
}

void GLWidget::setYRotation(int angle)
{
    qNormalizeAngle(angle);
    if (angle != yRot) {
        yRot = angle;
        emit yRotationChanged(angle);
        updateGL();
    }
}

void GLWidget::setZRotation(int angle)
{
    qNormalizeAngle(angle);
    if (angle != zRot) {
        zRot = angle;
        emit zRotationChanged(angle);
        updateGL();
    }
}

void GLWidget::setZoomLevel(int z)
{
    if (z != Zoom) {
        Zoom = z;
        emit zoomChanged(z);
        updateGL();
    }
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
	/*
	//Test triangle array
	Vector3f Vertices[3];
    Vertices[0] = Vector3f(-1.0f, -1.0f, 0.0f);
    Vertices[1] = Vector3f(1.0f, -1.0f, 0.0f);
    Vertices[2] = Vector3f(0.0f, 1.0f, 0.0f);

	QGLFunctions glFuncs(QGLContext::currentContext());

 	glFuncs.glGenBuffers(1, &VBO);
	glFuncs.glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glFuncs.glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
    */
	// Load mesh from file
	mainMesh = new Mesh();
    mainMesh->LoadMesh("../DeferredShading/Models/sponza/sponza.obj");
	//mainMesh->LoadMesh("../DeferredShading/Models/crytek-sponza/sponza.obj");
	//Magick::InitializeMagick("C:/Users/NuXe/Documents/GitHub/Deferred-Shading/Win32/Debug");
	//texture T = texture(GL_TEXTURE_2D, "../DeferredShading/Models/sponza/KAMEN.jpeg");
	//Texture T = Texture(GL_TEXTURE_2D, "C:/Users/NuXe/Documents/GitHub/Deferred-Shading/DeferredShading/Models/sponza/KAMEN.JPG");
	//texture T = texture(GL_TEXTURE_2D, "C:/Users/NuXe/Desktop/ImageMagick-6.8.9/images/arc.png");
	//T.Load();
}

void GLWidget::resizeGL(int width, int height)
{
    int side = qMin(width, height);
    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60,width/height,1.0,1000.0);
    glMatrixMode(GL_MODELVIEW);
}

void GLWidget::paintGL()
{
	QGLFunctions glFuncs(QGLContext::currentContext());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glTranslatef(0.0f,0.0f,-Zoom/16.0);
    glRotatef(xRot / 16.0, 1.0, 0.0, 0.0);
    glRotatef(yRot / 16.0, 0.0, 1.0, 0.0);
    glRotatef(zRot / 16.0, 0.0, 0.0, 1.0);
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
	/*
	// print test triangle
	QGLFunctions glFuncs(QGLContext::currentContext());

	GLuint positionLocation = glFuncs.glGetAttribLocation(shaderProgram,"position");

    glFuncs.glEnableVertexAttribArray(positionLocation);
    glFuncs.glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glFuncs.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    glFuncs.glDisableVertexAttribArray(positionLocation);
	*/
    mainMesh->Render(shaderProgram);
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
    lastPos = event->pos();
}

void GLWidget::mouseMoveEvent(QMouseEvent * event)
{
    int dx = event->x() - lastPos.x();
    int dy = event->y() - lastPos.y();
    if (event->buttons() & Qt::LeftButton) {
        setXRotation(xRot + 8 * dy);
        setYRotation(yRot + 8 * dx);
    } else if (event->buttons() & Qt::RightButton) {
        setXRotation(xRot + 8 * dy);
        setZRotation(zRot + 8 * dx);
    }
    lastPos = event->pos();
}
