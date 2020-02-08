#pragma once
#include <QtWidgets/QTreeView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <QDialog>

#include <behavior/nodes.h>
#include <behavior/btree_trace.h>
#include "log_item.h"
#include "log_tree_model.h"

namespace spiritsaway::behavior_tree::editor
{
	using namespace spiritsaway;
	class log_dialog: public QDialog
	{
		Q_OBJECT
	public:
		log_dialog(QWidget* parent = nullptr);
		bool push_cmd(behavior_tree::common::agent_cmd_detail one_cmd);

	private:
		log_tree_model* _model;
		QTreeView* _view;
		behavior_tree::common::btree_trace _btree_history;

	};
}