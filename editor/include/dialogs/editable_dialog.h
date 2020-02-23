#pragma once

#include <QDialog>
#include <string>
#include <vector>
#include <utility>
#include <optional>

#include <QStringListModel>

#include <dialogs/editable_item.h>

namespace spiritsaway::behavior_tree::editor
{
	class basic_node;
	class editable_dialog :public QDialog
	{
		Q_OBJECT

	public:
		editable_dialog(QWidget* parent, basic_node* edit_node);
		void closeEvent(QCloseEvent *event);
		void run();
		void refresh();
		void check_edit(std::shared_ptr<editable_item> change_item);
	private:
		void remove_pre_layout(QLayout* pre_layout);
		basic_node* edit_node;
		std::vector<QWidget*> widgets_to_delete;
	};
}