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

	// Menus
	// File
	QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
	QAction *loadModelAct = new QAction(tr("&Load Model"), this);
	fileMenu->addAction(loadModelAct);
	// Render
	QMenu *renderMenu = menuBar()->addMenu(tr("&Render"));
	QAction *forwardRenderAct = new QAction(tr("&Forward"), this);
	QAction *deferredRenderAct = new QAction(tr("&Deferred"), this);
	QAction *positionRenderAct = new QAction(tr("&Position"), this);
	QAction *diffuseRenderAct = new QAction(tr("&Diffuse"), this);
	QAction *normalRenderAct = new QAction(tr("&Normal"), this);
	renderMenu->addAction(forwardRenderAct);
	renderMenu->addAction(deferredRenderAct);
	renderMenu->addSeparator();
	renderMenu->addAction(positionRenderAct);
	renderMenu->addAction(diffuseRenderAct);
	renderMenu->addAction(normalRenderAct);

	// Validators
	QDoubleValidator *doubleValidator = new QDoubleValidator(0.01,10.0,2);
	doubleValidator->setNotation(QDoubleValidator::StandardNotation);
	QDoubleValidator *doubleValidator2 = new QDoubleValidator(0.01,10000.0,2);
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

	// Lights
	QLabel *lightingLabel = new QLabel();
	lightingLabel->setText("Lighting-> ");

	// Light bounding box scale
	QLabel *lightBBScaleLabel = new QLabel();
	lightBBScaleLabel->setText("Bounding box scale: ");

	QLineEdit *lighBBScaleIn = new QLineEdit();
	lighBBScaleIn->setValidator(doubleValidator);
	lighBBScaleIn->setText("1.0");

	// Light intensity
	QLabel *lightIntesityLabel = new QLabel();
	lightIntesityLabel->setText("Intensity: ");

	QLineEdit *lighIntensityIn = new QLineEdit();
	lighIntensityIn->setValidator(doubleValidator2);
	lighIntensityIn->setText("1.0");

	// Number of lights
	QLabel *nLightsLabel = new QLabel();
	nLightsLabel->setText("N: ");

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
	connect(forwardRenderAct,SIGNAL(triggered()), glWidget, SLOT(setForwardRenderMode()));
	connect(deferredRenderAct,SIGNAL(triggered()), glWidget, SLOT(setDeferredRenderMode()));
	connect(positionRenderAct,SIGNAL(triggered()), glWidget, SLOT(setPositionRenderMode()));
	connect(diffuseRenderAct,SIGNAL(triggered()), glWidget, SLOT(setDiffuseRenderMode()));
	connect(normalRenderAct,SIGNAL(triggered()), glWidget, SLOT(setNormalRenderMode()));
	connect(this,SIGNAL(keyPressed(int)), glWidget, SLOT(addKey(int)));
	connect(this,SIGNAL(keyReleased(int)), glWidget, SLOT(removeKey(int)));
	connect(cameraSensitivityIn,SIGNAL(textChanged(QString)),glWidget,SLOT(modifyCameraSensitivity(QString)));
	connect(cameraSpeedIn,SIGNAL(textChanged(QString)),glWidget,SLOT(modifyCameraSpeed(QString)));
	connect(glWidget, SIGNAL(updateFPSSignal(int)), fpsLabelNum, SLOT(setNum(int)));
	connect(nLightsIn,SIGNAL(textChanged(QString)),this,SLOT(modifyNLights(QString)));
	connect(genLightsButton, SIGNAL(clicked()), this, SLOT(genLights()));
	connect(this, SIGNAL(glGenLights(int)), glWidget, SLOT(genLightning(int)));
	connect(lighBBScaleIn, SIGNAL(textChanged(QString)), glWidget, SLOT(modifyBoundingBoxScale(QString)));
	connect(lighIntensityIn, SIGNAL(textChanged(QString)), glWidget, SLOT(modifyMaxIntensity(QString)));
	connect(glWidget, SIGNAL(updateLightIntensityIn(QString)), lighIntensityIn, SLOT(setText(QString)));
	
	// Set layout content
    mainLayout->addWidget(glWidget);

	thirdLayout->addWidget(sensibilityLabel);
	thirdLayout->addWidget(cameraSensitivityIn);
	thirdLayout->addWidget(speedLabel);
	thirdLayout->addWidget(cameraSpeedIn);
	thirdLayout->addWidget(lightingLabel);
	thirdLayout->addWidget(lightBBScaleLabel);
	thirdLayout->addWidget(lighBBScaleIn);
	thirdLayout->addWidget(lightIntesityLabel);
	thirdLayout->addWidget(lighIntensityIn);
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