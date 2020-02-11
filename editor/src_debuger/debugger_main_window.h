#pragma once

#include <deque>

#include <graph/multi_instance_window.h>

QT_BEGIN_NAMESPACE
namespace Ui { class debugger_main_window; }
QT_END_NAMESPACE

using json = nlohmann::json;

namespace bt_editor = spiritsaway::behavior_tree::editor;

namespace spiritsaway::behavior_tree::editor
{
	class log_dialog;
	class debugger_main_window : public multi_instance_window
	{
		Q_OBJECT

	public:
		debugger_main_window(QWidget* parent = nullptr);
		~debugger_main_window();

	private:
		void init_widgets();
		void init_actions();
	private:
		Ui::debugger_main_window *ui;
		log_dialog* _log_viewer;
	public:
		std::deque<behavior_tree::common::agent_cmd_detail> _btree_cmds;
		bool focus_on(const std::string& tree_name, std::uint32_t node_idx);
		bool node_has_breakpoint(const std::string& tree_name, std::uint32_t node_idx) const;
		void highlight_node(const std::string& tree_name, std::uint32_t node_idx, QColor color);
	public:
		bool is_read_only() const;

	};
}