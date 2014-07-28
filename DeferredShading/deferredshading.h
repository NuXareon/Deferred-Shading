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

private:
	Ui::DeferredShadingClass ui;
	QSlider *createSlider(Qt::Orientation x);
};

#endif // DEFERREDSHADING_H
