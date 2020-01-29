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
void editable_dialog::remove_pre_layout(QLayout* pre_layout)
{	
	//std::cout << "editable_dialog begin remove_pre_layout" << pre_layout << std::endl;
	if (!pre_layout)
	{
		return;
	}
	QLayoutItem* item;
	while (auto item = pre_layout->takeAt(0)) 
	{
		auto temp_widget = item->widget();
		if (temp_widget)
		{
			temp_widget->hide();
			pre_layout->removeItem(item);
			widgets_to_delete.push_back(temp_widget);
			//delete temp_widget;
		}
		auto temp_layout = item->layout();
		remove_pre_layout(temp_layout);
		if (temp_layout)
		{
			delete temp_layout;
		}
		if (!temp_widget && !temp_layout)
		{
			delete item;
		}
	}
	//std::cout << "editable_dialog end remove_pre_layout" << pre_layout<< std::endl;
}
void editable_dialog::refresh()
{
	//std::cout << "editable_dialog begin refresh" << std::endl;
	auto pre_layout = layout();
	if (!widgets_to_delete.empty())
	{
		for (auto one_widget : widgets_to_delete)
		{
			delete one_widget;
		}
		widgets_to_delete.clear();
	}
	remove_pre_layout(pre_layout);
	if (pre_layout)
	{
		delete pre_layout;
	}
	QHBoxLayout *cur_layout = new QHBoxLayout;
	cur_layout->addWidget(edit_node->_show_widget->to_editor([&](std::shared_ptr<editable_item> change_item)
	{
		check_edit(change_item);
		return;
	}));
	setLayout(cur_layout);
	//std::cout << "editable_dialog end refresh" << std::endl;

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
		if (!widgets_to_delete.empty())
		{
			for (auto one_widget : widgets_to_delete)
			{
				delete one_widget;
			}
			widgets_to_delete.clear();
		}
		event->accept();
	}

}