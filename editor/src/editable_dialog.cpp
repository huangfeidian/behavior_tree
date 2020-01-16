#include <qboxlayout.h>
#include <qmessagebox.h>
#include "editable_dialog.h"

using namespace spiritsaway::behavior_tree::editor;

editable_dialog::editable_dialog(QWidget* _in_parent, node* _in_edit_node)
	:QDialog(_in_parent)
	, edit_node(_in_edit_node)
{
	refresh();
	
}

void editable_dialog::run()
{
	exec();

}
void editable_dialog::refresh()
{
	auto pre_layout = layout();
	if (pre_layout)
	{
		QLayoutItem* item;
		while ((item = pre_layout->takeAt(0)) != 0)
		{
			pre_layout->removeItem(item);
			delete item;
		}
		delete pre_layout;
		std::cout << "refresh to dump pre layout" << std::endl;
		
	}
	QHBoxLayout *cur_layout = new QHBoxLayout;
	cur_layout->addWidget(edit_node->_show_widget->to_editor([&](std::shared_ptr<editable_item> change_item)
	{
		check_edit(change_item);
		return;
	}));
	setLayout(cur_layout);
}
void editable_dialog::check_edit(std::shared_ptr<editable_item> change_item)
{
	if (edit_node->check_item_edit_refresh(change_item))
	{
		refresh();
	}
}
void editable_dialog::closeEvent(QCloseEvent* event)
{
	auto notify_info = edit_node->check_valid();
	if (!notify_info.empty())
	{
		QMessageBox::about(this, QString("Error"),
			QString::fromStdString(notify_info));
		event->ignore();
		return;
	}
	else
	{
		event->accept();
	}

}