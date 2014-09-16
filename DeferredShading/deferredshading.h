#ifndef DEFERREDSHADING_H
#define DEFERREDSHADING_H

#include <QtGui>
#include <QSlider>
#include "ui_deferredshading.h"

class DeferredShading : public QMainWindow
{
	Q_OBJECT

public:
	DeferredShading(QWidget *parent = 0, Qt::WFlags flags = 0);
	~DeferredShading();
	void keyPressEvent(QKeyEvent* e);
	void keyReleaseEvent(QKeyEvent* e);

private:
	Ui::DeferredShadingClass ui;
	QString nLights;

private slots:
	void loadModelDia();					// Open a file dialog and emits a signal(modelPathChange(string)) to the glwidget with the model path.
	void modifyNLights(QString);			// Sets the nLights variable.
	void genLights();						// Emits a signal(glGenLights(int)) to the glwidget with the nLights parameter.

signals:
	void modelPathChange(std::string);
	void keyPressed(int k);
	void keyReleased(int k);
	void glGenLights(int n);
};

#endif // DEFERREDSHADING_H
