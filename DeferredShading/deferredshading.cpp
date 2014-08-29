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

	QAction *loadModelAct = new QAction(tr("&Load Model"), this);
	QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
	fileMenu->addAction(loadModelAct);

    GLWidget *glWidget = new GLWidget;

	connect(loadModelAct,SIGNAL(triggered()), this, SLOT(loadModelDia()));
	connect(this,SIGNAL(modelPathChange(std::string)), glWidget, SLOT(loadModel(std::string)));
	connect(this,SIGNAL(keyPressed(int)), glWidget, SLOT(addKey(int)));
	connect(this,SIGNAL(keyReleased(int)), glWidget, SLOT(removeKey(int)));
	
    mainLayout->addWidget(glWidget);

    secondLayout->addLayout(mainLayout);

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