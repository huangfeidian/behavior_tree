#include <qboxlayout.h>
#include <qmessagebox.h>
#include "info_dialog.h"

using namespace spiritsaway::behavior_tree::editor;

info_dialog::info_dialog(QWidget* _in_parent, node* _in_edit_node)
	:QDialog(_in_parent)
	, edit_node(_in_edit_node)
{
	QHBoxLayout *cur_layout = new QHBoxLayout;
	cur_layout->addWidget(edit_node->_show_widget->to_dialog());
	setLayout(cur_layout);
}

void info_dialog::run()
{
	exec();

}

