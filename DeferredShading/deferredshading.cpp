#include <QBoxLayout>

#include "deferredshading.h"
#include "glwidget.h"
#include <sstream>

void DeferredShading::loadModelDia()
{
	QString path = QFileDialog::getOpenFileName(this, tr("Load Model"), "", tr("OBJ files (*.obj);;All Files (*.*)"));
	if (path != "") emit modelPathChange(path.toStdString());
}

void DeferredShading::modifyNLights(QString s)
{
	nLights = s;
}

void DeferredShading::genLights() 
{
	emit glGenLights(nLights.toInt());
}

DeferredShading::DeferredShading(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags)
{
	ui.setupUi(this);

	// Layouts
	QHBoxLayout *mainLayout = new QHBoxLayout;
    QVBoxLayout *secondLayout = new QVBoxLayout;
	QHBoxLayout *thirdLayout = new QHBoxLayout;

	// Menu
	QAction *loadModelAct = new QAction(tr("&Load Model"), this);
	QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
	fileMenu->addAction(loadModelAct);

	// Validators
	QDoubleValidator *doubleValidator = new QDoubleValidator(0.01,10.0,2);
	doubleValidator->setNotation(QDoubleValidator::StandardNotation);

	QIntValidator *intValidator = new QIntValidator(0,N_MAX_LIGHTS);

	// Camera Sensitivity
	QLabel *sensibilityLabel = new QLabel();
	sensibilityLabel->setText("Camera sensitivity: ");

	QLineEdit *cameraSensitivityIn = new QLineEdit();
	cameraSensitivityIn->setValidator(doubleValidator);
	cameraSensitivityIn->setText("0.1");

	// Camera Speed
	QLabel *speedLabel = new QLabel();
	speedLabel->setText("Camera speed: ");

	QLineEdit *cameraSpeedIn = new QLineEdit();
	cameraSpeedIn->setValidator(doubleValidator);
	cameraSpeedIn->setText("1.0");
	
	// Number of lights
	QLabel *nLightsLabel = new QLabel();
	nLightsLabel->setText("N lights: ");

	std::stringstream sstm;
	sstm << INITIAL_LIGHTS;
	nLights = QString::fromStdString(sstm.str());
	QLineEdit *nLightsIn = new QLineEdit();
	nLightsIn->setValidator(intValidator);
	nLightsIn->setText(nLights);

	QPushButton *genLightsButton = new QPushButton("Gen. Lights");

	// FPS
	QLabel *fpsLabelNum = new QLabel();
	fpsLabelNum->setNum(0);
	fpsLabelNum->setAlignment(Qt::AlignRight);
	fpsLabelNum->setMaximumHeight(20);

	QLabel *fpsLabel = new QLabel();
	fpsLabel->setText("FPS: ");
	fpsLabel->setAlignment(Qt::AlignRight);

	// GLWidget
    GLWidget *glWidget = new GLWidget;

	// Connect signals-slots
	connect(loadModelAct,SIGNAL(triggered()), this, SLOT(loadModelDia()));
	connect(this,SIGNAL(modelPathChange(std::string)), glWidget, SLOT(loadModel(std::string)));
	connect(this,SIGNAL(keyPressed(int)), glWidget, SLOT(addKey(int)));
	connect(this,SIGNAL(keyReleased(int)), glWidget, SLOT(removeKey(int)));
	connect(cameraSensitivityIn,SIGNAL(textChanged(QString)),glWidget,SLOT(modifyCameraSensitivity(QString)));
	connect(cameraSpeedIn,SIGNAL(textChanged(QString)),glWidget,SLOT(modifyCameraSpeed(QString)));
	connect(glWidget, SIGNAL(updateFPSSignal(int)), fpsLabelNum, SLOT(setNum(int)));
	connect(nLightsIn,SIGNAL(textChanged(QString)),this,SLOT(modifyNLights(QString)));
	connect(genLightsButton, SIGNAL(clicked()), this, SLOT(genLights()));
	connect(this, SIGNAL(glGenLights(int)), glWidget, SLOT(genLightning(int)));
	
	// Set layout content
    mainLayout->addWidget(glWidget);

	thirdLayout->addWidget(sensibilityLabel);
	thirdLayout->addWidget(cameraSensitivityIn);
	thirdLayout->addWidget(speedLabel);
	thirdLayout->addWidget(cameraSpeedIn);
	thirdLayout->addWidget(nLightsLabel);
	thirdLayout->addWidget(nLightsIn);
	thirdLayout->addWidget(genLightsButton);
	thirdLayout->addWidget(fpsLabel);
	thirdLayout->addWidget(fpsLabelNum);

	secondLayout->addLayout(thirdLayout);
    secondLayout->addLayout(mainLayout);

	cameraSpeedIn->setFocus();

    setWindowTitle("Deferred Shading");
    ui.centralWidget->setLayout(secondLayout);
	setMinimumSize(1024,576);
}

DeferredShading::~DeferredShading()
{

}

void DeferredShading::keyPressEvent(QKeyEvent* e)
{
	emit keyPressed(e->key());
}

void DeferredShading::keyReleaseEvent(QKeyEvent* e)
{
	emit keyReleased(e->key());
}