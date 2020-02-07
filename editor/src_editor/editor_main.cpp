
#include <QApplication>
#include <qstylefactory.h>

#include "editor_main_window.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
	a.setStyle(QStyleFactory::create("windowsvista"));
	std::string icon_path = "./resources/window_icon.png";
	auto the_icon = QIcon(QString::fromStdString(icon_path));
	a.setWindowIcon(the_icon);
	editor_main_window w = editor_main_window();
	w.showMaximized();
    w.show();
    return a.exec();
}
