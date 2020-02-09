#pragma once
#include <QtWidgets/QTreeView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <QDialog>
#include <qtimer.h>

#include <behavior/nodes.h>
#include <behavior/btree_trace.h>

#include <deque>
#include "log_item.h"
#include "log_tree_model.h"

namespace spiritsaway::behavior_tree::editor
{
	using namespace spiritsaway;
	class log_dialog: public QDialog
	{
		Q_OBJECT
	public:
		log_dialog(std::deque<behavior_tree::common::agent_cmd_detail>& cmd_queue, QWidget* parent = nullptr);
		bool push_cmd(behavior_tree::common::agent_cmd_detail one_cmd);

	public slots:
		void search_content();
		void goto_graph();
		void show_trees();
		void on_view_double_clicked(QModelIndex cur_idx);
		void on_view_context_menu(const QPoint& pos);
		void show_fronts(std::uint32_t top_row, std::uint32_t secondary_row);
		void show_blackboard(std::uint32_t top_row, std::uint32_t secondary_row);
		void timer_poll();
	private:
		std::deque<behavior_tree::common::agent_cmd_detail>& cmd_queue;
		QTimer* _poll_timer;
		log_tree_model* _model;
		QTreeView* _view;
		behavior_tree::common::btree_trace _btree_history;
		std::string get_comment(std::size_t top_row, std::size_t secondary_row) const;


	};
}