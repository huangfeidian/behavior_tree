#include <mainwindow.h>

#include <QApplication>
#include <qstylefactory.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
	a.setStyle(QStyleFactory::create("windowsvista"));
    MainWindow w;
    w.show();
    return a.exec();
}
