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
	QAction *forwardBlendRenderAct = new QAction(tr("&Forward(Blend)"), this);
	QAction *deferredRenderAct = new QAction(tr("&Deferred"), this);
	QAction *forwardPlusRenderAct = new QAction(tr("&Forward+"), this);
	QAction *positionRenderAct = new QAction(tr("&Position"), this);
	QAction *diffuseRenderAct = new QAction(tr("&Diffuse"), this);
	QAction *normalRenderAct = new QAction(tr("&Normal"), this);
	QAction *allRenderAct = new QAction(tr("&All"), this);
	QAction *depthRenderAct = new QAction(tr("&Depth"), this);
	QAction *gridRenderAct = new QAction(tr("&Lighting Grid"), this);
	renderMenu->addAction(forwardRenderAct);
	renderMenu->addAction(forwardBlendRenderAct);
	renderMenu->addAction(deferredRenderAct);
	renderMenu->addAction(forwardPlusRenderAct);
	renderMenu->addSeparator();
	renderMenu->addAction(positionRenderAct);
	renderMenu->addAction(diffuseRenderAct);
	renderMenu->addAction(normalRenderAct);
	renderMenu->addAction(allRenderAct);
	renderMenu->addSeparator();
	renderMenu->addAction(depthRenderAct);
	renderMenu->addAction(gridRenderAct);

	// Validators
	QDoubleValidator *_doubleValidator = new QDoubleValidator(0.01,10.0,2);
	_doubleValidator->setNotation(QDoubleValidator::StandardNotation);
	QDoubleValidator *doubleValidator = new QDoubleValidator(0.01,10000.0,2);
	doubleValidator->setNotation(QDoubleValidator::StandardNotation);

	QIntValidator *intValidator = new QIntValidator(0,N_MAX_LIGHTS);
	QIntValidator *intValidator2 = new QIntValidator(0,4096);

	// Separators
	QFrame *line = new QFrame();
	line->setFrameShape(QFrame::VLine);
	QFrame *line2 = new QFrame();
	line2->setFrameShape(QFrame::VLine);
	QFrame *line3 = new QFrame();
	line3->setFrameShape(QFrame::VLine);
	QFrame *line4 = new QFrame();
	line4->setFrameShape(QFrame::VLine);

	// Camera Sensitivity
	QLabel *sensibilityLabel = new QLabel();
	sensibilityLabel->setText("Sensitivity: ");

	QLineEdit *cameraSensitivityIn = new QLineEdit();
	cameraSensitivityIn->setValidator(doubleValidator);
	cameraSensitivityIn->setText("0.1");

	// Camera Speed
	QLabel *speedLabel = new QLabel();
	speedLabel->setText("Speed: ");

	QLineEdit *cameraSpeedIn = new QLineEdit();
	cameraSpeedIn->setValidator(doubleValidator);
	cameraSpeedIn->setText("1.0");

	// Threshold (deferred only)
	QLabel *thresholdLabel = new QLabel();
	thresholdLabel->setText("Threshold: ");

	QLineEdit *thresholdIn = new QLineEdit();
	thresholdIn->setValidator(intValidator2);
	thresholdIn->setText("256");

	// Billboards
	//QLabel *billboardsLabel = new QLabel();
	//billboardsLabel->setText("Billboards: ");

	QCheckBox *billboardsCheckBox = new QCheckBox("Billboards:");
	billboardsCheckBox->setLayoutDirection(Qt::RightToLeft);
	billboardsCheckBox->setChecked(false);

	// Light bounding box scale
	QLabel *lightBBScaleLabel = new QLabel();
	lightBBScaleLabel->setText(" Bb scale: ");

	QLineEdit *lighBBScaleIn = new QLineEdit();
	lighBBScaleIn->setValidator(doubleValidator);
	lighBBScaleIn->setText("1.0");

	// Attenuation
	QLabel *attenuationLabel = new QLabel();
	attenuationLabel->setText("Attenuation: ");

	QLineEdit *constantAttIn = new QLineEdit();
	constantAttIn->setValidator(doubleValidator);
	constantAttIn->setText("1.0");

	QLineEdit *linearAttIn = new QLineEdit();
	linearAttIn->setValidator(doubleValidator);
	linearAttIn->setText("60.0");

	QLineEdit *expAttIn = new QLineEdit();
	expAttIn->setValidator(doubleValidator);
	expAttIn->setText("0.0");

	// Light intensity
	QLabel *lightIntesityLabel = new QLabel();
	lightIntesityLabel->setText("Intensity: ");

	QLineEdit *lighIntensityIn = new QLineEdit();
	lighIntensityIn->setValidator(doubleValidator);
	lighIntensityIn->setText("1.0");

	// Number of lights
	QLabel *nLightsLabel = new QLabel();
	nLightsLabel->setText("N: ");
	std::stringstream sstm;
	sstm << INITIAL_LIGHTS;
	std::string s = sstm.str();
	// QString::fromStdString desnt work on release
#ifdef NDEBUG
	nLights = "20";
#else
	nLights = QString::fromStdString(s);
#endif
	
	QLineEdit *nLightsIn = new QLineEdit();
	nLightsIn->setValidator(intValidator);
	nLightsIn->setText(nLights);

	QPushButton *genLightsButton = new QPushButton("Gen. Lights");

	// Render Mode
	QLabel *renderModeLabel = new QLabel();
	renderModeLabel->setText("Forward");
	renderModeLabel->setAlignment(Qt::AlignRight);

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
	connect(forwardBlendRenderAct,SIGNAL(triggered()), glWidget, SLOT(setForwardBlendRenderMode()));
	connect(forwardPlusRenderAct,SIGNAL(triggered()), glWidget, SLOT(setForwardPlusRenderMode()));
	connect(deferredRenderAct,SIGNAL(triggered()), glWidget, SLOT(setDeferredRenderMode()));
	connect(positionRenderAct,SIGNAL(triggered()), glWidget, SLOT(setPositionRenderMode()));
	connect(diffuseRenderAct,SIGNAL(triggered()), glWidget, SLOT(setDiffuseRenderMode())); 
	connect(normalRenderAct,SIGNAL(triggered()), glWidget, SLOT(setNormalRenderMode()));
	connect(allRenderAct,SIGNAL(triggered()), glWidget, SLOT(setAllRenderMode()));
	connect(depthRenderAct,SIGNAL(triggered()), glWidget, SLOT(setDepthRenderMode()));
	connect(gridRenderAct,SIGNAL(triggered()), glWidget, SLOT(setForwardDebugRenderMode()));
	connect(this,SIGNAL(keyPressed(int)), glWidget, SLOT(addKey(int)));
	connect(this,SIGNAL(keyReleased(int)), glWidget, SLOT(removeKey(int)));
	connect(cameraSensitivityIn,SIGNAL(textChanged(QString)),glWidget,SLOT(modifyCameraSensitivity(QString)));
	connect(cameraSpeedIn,SIGNAL(textChanged(QString)),glWidget,SLOT(modifyCameraSpeed(QString)));
	connect(glWidget, SIGNAL(updateFPSSignal(int)), fpsLabelNum, SLOT(setNum(int)));
	connect(nLightsIn,SIGNAL(textChanged(QString)),this,SLOT(modifyNLights(QString)));
	connect(genLightsButton, SIGNAL(clicked()), this, SLOT(genLights()));
	connect(this, SIGNAL(glGenLights(int)), glWidget, SLOT(genLightning(int)));
	connect(billboardsCheckBox, SIGNAL(stateChanged(int)), glWidget, SLOT(enableBillboards(int)));
	connect(lighBBScaleIn, SIGNAL(textChanged(QString)), glWidget, SLOT(modifyBoundingBoxScale(QString)));
	connect(lighIntensityIn, SIGNAL(textChanged(QString)), glWidget, SLOT(modifyMaxIntensity(QString)));
	connect(glWidget, SIGNAL(updateLightIntensityIn(QString)), lighIntensityIn, SLOT(setText(QString)));
	connect(glWidget, SIGNAL(updateRenderMode(QString)), renderModeLabel, SLOT(setText(QString)));
	connect(thresholdIn, SIGNAL(textChanged(QString)), glWidget, SLOT(modifyThreshold(QString)));
	connect(constantAttIn, SIGNAL(textChanged(QString)), glWidget, SLOT(modifyConstantAttenuation(QString)));
	connect(linearAttIn, SIGNAL(textChanged(QString)), glWidget, SLOT(modifyLinearAttenuation(QString)));
	connect(expAttIn, SIGNAL(textChanged(QString)), glWidget, SLOT(modifyExpAttenuation(QString)));

	// Set layout content
    mainLayout->addWidget(glWidget);

	thirdLayout->addWidget(sensibilityLabel);
	thirdLayout->addWidget(cameraSensitivityIn);
	thirdLayout->addWidget(speedLabel);
	thirdLayout->addWidget(cameraSpeedIn);
	thirdLayout->addWidget(line);
	thirdLayout->addWidget(thresholdLabel);
	thirdLayout->addWidget(thresholdIn);
	thirdLayout->addWidget(line2);
	thirdLayout->addWidget(billboardsCheckBox);
	thirdLayout->addWidget(lightBBScaleLabel);
	thirdLayout->addWidget(lighBBScaleIn);
	thirdLayout->addWidget(attenuationLabel);
	thirdLayout->addWidget(constantAttIn);
	thirdLayout->addWidget(linearAttIn);
	thirdLayout->addWidget(expAttIn);
	thirdLayout->addWidget(lightIntesityLabel);
	thirdLayout->addWidget(lighIntensityIn);
	thirdLayout->addWidget(nLightsLabel);
	thirdLayout->addWidget(nLightsIn);
	thirdLayout->addWidget(genLightsButton);
	thirdLayout->addWidget(line3);
	thirdLayout->addWidget(renderModeLabel);
	thirdLayout->addWidget(line4);
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