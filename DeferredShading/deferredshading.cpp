#include <QBoxLayout>

#include "deferredshading.h"
#include "glwidget.h"

QSlider *DeferredShading::createSlider(Qt::Orientation x)
{
    QSlider *slider = new QSlider(x);
    slider->setRange(0, 360 * 16);
    slider->setSingleStep(16);
    slider->setPageStep(15 * 16);
    slider->setTickInterval(15 * 16);
    slider->setTickPosition(QSlider::TicksRight);
    return slider;
}

DeferredShading::DeferredShading(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags)
{
	ui.setupUi(this);
	QHBoxLayout *mainLayout = new QHBoxLayout;
    QVBoxLayout *secondLayout = new QVBoxLayout;
    QSlider *xSlider = createSlider(Qt::Vertical);
    QSlider *ySlider = createSlider(Qt::Vertical);
    QSlider *zSlider = createSlider(Qt::Vertical);
    QSlider *zoomSlider = createSlider(Qt::Horizontal);
    GLWidget *glWidget = new GLWidget;
	
    connect(xSlider,SIGNAL(valueChanged(int)), glWidget, SLOT(setXRotation(int)));
    connect(glWidget, SIGNAL(xRotationChanged(int)), xSlider, SLOT(setValue(int)));
    connect(ySlider,SIGNAL(valueChanged(int)), glWidget, SLOT(setYRotation(int)));
    connect(glWidget, SIGNAL(yRotationChanged(int)), ySlider, SLOT(setValue(int)));
    connect(zSlider,SIGNAL(valueChanged(int)), glWidget, SLOT(setZRotation(int)));
    connect(glWidget, SIGNAL(zRotationChanged(int)), zSlider, SLOT(setValue(int)));
    connect(zoomSlider,SIGNAL(valueChanged(int)), glWidget, SLOT(setZoomLevel(int)));
    connect(glWidget,SIGNAL(zoomChanged(int)), zoomSlider, SLOT(setValue(int)));
	
    mainLayout->addWidget(glWidget);
    mainLayout->addWidget(xSlider);
    mainLayout->addWidget(ySlider);
    mainLayout->addWidget(zSlider);

    secondLayout->addWidget(zoomSlider);
    secondLayout->addLayout(mainLayout);

    xSlider->setValue(10 * 16);
    ySlider->setValue(65 * 16);
    zSlider->setValue(0 * 16);
    zoomSlider->setValue(50 * 16);
    setWindowTitle("Deferred Shading");
    ui.centralWidget->setLayout(secondLayout);
}

DeferredShading::~DeferredShading()
{

}
