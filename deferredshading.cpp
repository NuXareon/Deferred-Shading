#include "deferredshading.h"
#include "ui_deferredshading.h"

#include <QtWidgets>

#include "glwidget.h"

QSlider *DeferredShading::createSlider()
{
    QSlider *slider = new QSlider(Qt::Vertical);
    slider->setRange(0, 360 * 16);
    slider->setSingleStep(16);
    slider->setPageStep(15 * 16);
    slider->setTickInterval(15 * 16);
    slider->setTickPosition(QSlider::TicksRight);
    return slider;
}

DeferredShading::DeferredShading(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::DeferredShading)
{
    ui->setupUi(this);
    QHBoxLayout *mainLayout = new QHBoxLayout;
    QSlider *xSlider = createSlider();
    QSlider *ySlider = createSlider();
    QSlider *zSlider = createSlider();
    GLWidget *glWidget = new GLWidget;

    connect(xSlider,SIGNAL(valueChanged(int)), glWidget, SLOT(setXRotation(int)));
    connect(glWidget, SIGNAL(xRotationChanged(int)), xSlider, SLOT(setValue(int)));
    connect(ySlider,SIGNAL(valueChanged(int)), glWidget, SLOT(setYRotation(int)));
    connect(glWidget, SIGNAL(yRotationChanged(int)), ySlider, SLOT(setValue(int)));
    connect(zSlider,SIGNAL(valueChanged(int)), glWidget, SLOT(setZRotation(int)));
    connect(glWidget, SIGNAL(zRotationChanged(int)), zSlider, SLOT(setValue(int)));

    mainLayout->addWidget(glWidget);
    mainLayout->addWidget(xSlider);
    mainLayout->addWidget(ySlider);
    mainLayout->addWidget(zSlider);

    xSlider->setValue(15 * 16);
    ySlider->setValue(345 * 16);
    zSlider->setValue(0 * 16);
    setWindowTitle("Deferred Shading");
    ui->centralWidget->setLayout(mainLayout);
}

DeferredShading::~DeferredShading()
{
    delete ui;
}
