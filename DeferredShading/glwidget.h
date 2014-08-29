#include <QGLWidget>
#include "mesh.h"

class GLWidget : public QGLWidget
{
    Q_OBJECT

public:
    GLWidget(QWidget *parent = 0);
    ~GLWidget();

public slots:
	void loadModel(std::string path);
	void addKey(int k);
	void removeKey(int k);

protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
	void timerEvent(QTimerEvent* event);

private:
	QSet<int> keys;
	float xPos, yPos, zPos;
	float alpha, beta;
	float cSpeed, cSensivility;
    QPoint lastPos;
	Mesh *mainMesh;
	GLuint shaderProgram;
	std::string modelPath;

	bool readFile(const char* path, std::string& out);
	void initializeLighting();
	void initializeShaders();
};
