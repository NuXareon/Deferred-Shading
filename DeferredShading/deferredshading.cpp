#include <QBoxLayout>

#include "deferredshading.h"
#include "glwidget.h"

void DeferredShading::loadModelDia()
{
	QString path = QFileDialog::getOpenFileName(this, tr("Load Model"), "", tr("OBJ files (*.obj);;All Files (*.*)"));
	if (path != "") emit modelPathChange(path.toStdString());
}

DeferredShading::DeferredShading(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags)
{
	ui.setupUi(this);
	QHBoxLayout *mainLayout = new QHBoxLayout;
    QVBoxLayout *secondLayout = new QVBoxLayout;
	QHBoxLayout *thirdLayout = new QHBoxLayout;

	QAction *loadModelAct = new QAction(tr("&Load Model"), this);
	QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
	fileMenu->addAction(loadModelAct);

	QDoubleValidator *doubleValidator = new QDoubleValidator(0.01,10.0,2);
	doubleValidator->setNotation(QDoubleValidator::StandardNotation);

	QLabel *sensibilityLabel = new QLabel();
	sensibilityLabel->setText("Camera sensitivity: ");

	QLineEdit *cameraSensitivityIn = new QLineEdit();
	cameraSensitivityIn->setValidator(doubleValidator);
	cameraSensitivityIn->setText("0.1");

	QLabel *speedLabel = new QLabel();
	speedLabel->setText("Camera speed: ");

	QLineEdit *cameraSpeedIn = new QLineEdit();
	cameraSpeedIn->setValidator(doubleValidator);
	cameraSpeedIn->setText("1.0");

	QLabel *fpsLabelNum = new QLabel();
	fpsLabelNum->setNum(0);
	fpsLabelNum->setAlignment(Qt::AlignRight);
	fpsLabelNum->setMaximumHeight(20);

	QLabel *fpsLabel = new QLabel();
	fpsLabel->setText("FPS: ");
	fpsLabel->setAlignment(Qt::AlignRight);

    GLWidget *glWidget = new GLWidget;

	connect(loadModelAct,SIGNAL(triggered()), this, SLOT(loadModelDia()));
	connect(this,SIGNAL(modelPathChange(std::string)), glWidget, SLOT(loadModel(std::string)));
	connect(this,SIGNAL(keyPressed(int)), glWidget, SLOT(addKey(int)));
	connect(this,SIGNAL(keyReleased(int)), glWidget, SLOT(removeKey(int)));
	connect(cameraSensitivityIn,SIGNAL(textChanged(QString)),glWidget,SLOT(modifyCameraSensitivity(QString)));
	connect(cameraSpeedIn,SIGNAL(textChanged(QString)),glWidget,SLOT(modifyCameraSpeed(QString)));
	connect(glWidget, SIGNAL(updateFPSSignal(int)), fpsLabelNum, SLOT(setNum(int)));
	
    mainLayout->addWidget(glWidget);

	thirdLayout->addWidget(sensibilityLabel);
	thirdLayout->addWidget(cameraSensitivityIn);
	thirdLayout->addWidget(speedLabel);
	thirdLayout->addWidget(cameraSpeedIn);
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