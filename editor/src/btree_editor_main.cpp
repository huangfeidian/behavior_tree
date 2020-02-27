#include <btree_editor_window.h>
#include <QApplication>

using namespace std;
using namespace spiritsaway::behavior_tree::editor;


int main(int argc, char* argv[])
{
	QApplication a(argc, argv);
	btree_editor_window w = btree_editor_window();
	w.showMaximized();
	w.show();
	if (!w.load_config())
	{
		return 0;
	}
	return a.exec();
}