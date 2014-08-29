#ifndef DEFERREDSHADING_H
#define DEFERREDSHADING_H

#include <QtGui/QMainWindow>
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

private slots:
	void loadModelDia();

signals:
	void modelPathChange(std::string);
	void keyPressed(int k);
	void keyReleased(int k);
};

#endif // DEFERREDSHADING_H
