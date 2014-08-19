#include "deferredshading.h"
#include <Magick++.h>
#include <QtGui/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	DeferredShading w;
	w.show();
	return a.exec();
}
