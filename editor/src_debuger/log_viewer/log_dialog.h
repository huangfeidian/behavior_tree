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
	using spiritsaway::behavior_tree::common::debug_mode;
	class debugger_main_window;
	class log_dialog: public QWidget
	{
		Q_OBJECT
	public:
		log_dialog(std::deque<behavior_tree::common::agent_cmd_detail>& cmd_queue, debugger_main_window* _window);
		bool push_cmd(behavior_tree::common::agent_cmd_detail one_cmd);

	public slots:
		void search_content();
		void goto_graph(std::uint32_t top_row, std::uint32_t secondary_row);
		void show_trees();
		void on_view_double_clicked(QModelIndex cur_idx);
		void on_view_context_menu(const QPoint& pos);
		void show_fronts(const behavior_tree::common::btree_state& cur_state);
		void show_blackboard(const behavior_tree::common::btree_state& cur_state);
		void timer_poll();
		void highlight_fronts(const behavior_tree::common::btree_state& cur_state);
		void debug_stop();
		void debug_run_through(std::size_t max_step = 1000);
		void debug_run_once();
	private:
		std::deque<behavior_tree::common::agent_cmd_detail>& cmd_queue;
		QTimer* _poll_timer;
		log_tree_model* _model;
		QTreeView* _view;
		debug_mode _cur_debug_mode;
		std::shared_ptr<spdlog::logger> _logger;
		debugger_main_window* _main_window;
		behavior_tree::common::btree_state _cur_running_state;
		std::size_t _cur_top_row = 0;
		std::size_t _cur_secondary_row = 0;
		behavior_tree::common::btree_trace _btree_history;
		decltype(_cur_running_state.cur_fronts) _pre_fronts;
		std::string get_comment(std::size_t top_row, std::size_t secondary_row) const;
		QModelIndex get_model_idx(std::size_t top_row, std::size_t secondary_row, std::size_t column) const;
		void increate_row_idx();


		std::optional<behavior_tree::common::agent_cmd_detail> run_once_impl();

	};
}