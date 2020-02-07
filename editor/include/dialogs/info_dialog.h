#pragma once

#include <QDialog>
#include <string>
#include <vector>
#include <utility>
#include <optional>

#include <QStringListModel>

#include <btree/nodes.h>


namespace spiritsaway::behavior_tree::editor
{

	class info_dialog :public QDialog
	{
		Q_OBJECT

	public:
		info_dialog(QWidget* parent, node* edit_node);
		void run();
	private:
		node* edit_node;
	};
}