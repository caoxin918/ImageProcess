#include "imageprocess.h"
#include <QtGui/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	ImageProcess w;
	w.show();
	return a.exec();
}
