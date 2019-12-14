#include <mainwindow.h>

#include <QApplication>
#include <qstylefactory.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
	a.setStyle(QStyleFactory::create("windowsvista"));
	std::string icon_path = "./resources/window_icon.png";
	auto the_icon = QIcon(QString::fromStdString(icon_path));
	a.setWindowIcon(the_icon);
    MainWindow w;
    w.show();
    return a.exec();
}
