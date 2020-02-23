#pragma once

#include <QDialog>
#include <string>
#include <vector>
#include <utility>
#include <optional>

#include <QStringListModel>


namespace spiritsaway::behavior_tree::editor
{
	class basic_node;
	class info_dialog :public QDialog
	{
		Q_OBJECT

	public:
		info_dialog(QWidget* parent, basic_node* edit_node);
		void run();
	private:
		basic_node* edit_node;
	};
}