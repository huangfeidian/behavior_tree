#include <qfile.h>
#include <chrono>
#include <ctime>
#include <qmenu.h>

#include <any_container/encode.h>

#include <dialogs/search_select_dialog.h>
#include <sstream>

#include "log_dialog.h"

using namespace spiritsaway::behavior_tree::editor;
namespace
{
	std::string format_timepoint(std::uint64_t milliseconds_since_epoch)
	{
		auto epoch_begin = std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>();
		auto cur_timepoint = epoch_begin + std::chrono::milliseconds(milliseconds_since_epoch);
		auto cur_time_t = std::chrono::system_clock::to_time_t(cur_timepoint);

		struct tm * timeinfo;
		char buffer[80];

		timeinfo = localtime(&cur_time_t);

		strftime(buffer, sizeof(buffer), "%H:%M:%S ", timeinfo);
		return std::string(buffer) + std::to_string(milliseconds_since_epoch % 1000) + "ms";
	}
}
log_dialog::log_dialog(QWidget* parent)
	:QDialog(parent)
{
	auto _view = new QTreeView(this);
	auto vboxLayout = new QVBoxLayout(this);
	vboxLayout->setSpacing(0);
	vboxLayout->setContentsMargins(0, 0, 0, 0);

	vboxLayout->addWidget(_view);
	setLayout(vboxLayout);
	std::vector<std::pair<QString, Qt::ItemFlags>> headers;
	headers.emplace_back("Timestamp", Qt::ItemFlag::NoItemFlags);
	headers.emplace_back("Cmd", Qt::ItemFlag::NoItemFlags);
	headers.emplace_back("Param", Qt::ItemFlag::NoItemFlags);

	headers.emplace_back("comment", Qt::ItemFlag::ItemIsEditable);

	_model = new log_tree_model(headers);

	_view->setModel(_model);
	_view->setColumnWidth(0, 150);
	vboxLayout->setSizeConstraint(QLayout::SetMaximumSize);

}
bool log_dialog::push_cmd(behavior_tree::common::agent_cmd_detail one_cmd)
{
	std::vector<std::string> cmd_str;
	auto[ts, cmd, param] = one_cmd;
	cmd_str.push_back(format_timepoint(ts));
	cmd_str.push_back(std::string(magic_enum::enum_name(cmd)));
	cmd_str.push_back(encode(param).dump());
	std::vector<QVariant> row_data;
	for (const auto& one_data : cmd_str)
	{
		row_data.push_back(QString::fromStdString(one_data));
	}
	if (_btree_history.push_cmd(one_cmd))
	{
		_model->appendRow(row_data);
	}
	else
	{
		_model->appendRow(row_data, _model->index(_btree_history._poll_states.size() - 1, 0));
	}
	return true;
}
void log_dialog::contextMenuEvent(QContextMenuEvent *) //右键菜单项编辑

{
	QCursor cur = this->cursor();
	QMenu *menu = new QMenu(this);
	auto search_action = menu->addAction("search");
	QObject::connect(search_action, &QAction::triggered, this, &log_dialog::search_content);
	auto trees_action = menu->addAction("trees");
	QObject::connect(trees_action, &QAction::triggered, this, &log_dialog::show_trees);
	menu->exec(cur.pos()); //关联到光标
}
void log_dialog::search_content()
{
	std::vector<std::string> select_strings;
	for (std::size_t i = 0; i < _btree_history._poll_states.size(); i++)
	{
		for (std::size_t j = 0; j < _btree_history._poll_states[i]._cmds.size(); j++)
		{

			std::string cmd_str;
			const auto& [ts, cmd, param] = _btree_history._poll_states[i]._cmds[j];
			//cmd_str += std::to_string(i) + " " + std::to_string(j) + " ";
			cmd_str += format_timepoint(ts) + " ";
			cmd_str += std::string(magic_enum::enum_name(cmd)) + " ";
			cmd_str += encode(param).dump() + " ";
			cmd_str += get_comment(i, j) + " ";
			select_strings.push_back(cmd_str);
		}
	}
	auto cur_search_dialog = new search_select_dialog(select_strings, this);
	auto result = cur_search_dialog->run();
	if (result.empty())
	{
		return;
	}
	std::size_t cur_row_idx;
	for (cur_row_idx = 0u; cur_row_idx < select_strings.size(); cur_row_idx++)
	{
		if (select_strings[cur_row_idx] == result)
		{
			break;
		}
	}
	std::uint32_t top_row, secondary_row;
	for (top_row = 0; top_row < _btree_history._poll_states.size(); top_row++)
	{
		if (cur_row_idx < _btree_history._poll_states[top_row]._cmds.size())
		{
			secondary_row = cur_row_idx;
			break;
		}
		else
		{
			cur_row_idx -= _btree_history._poll_states[top_row]._cmds.size();
		}
	}

}
std::string log_dialog::get_comment(std::size_t top_row, std::size_t secondary_row) const
{
	QModelIndex cur_row_comment_idx;
	if (secondary_row != 0)
	{
		auto cur_row_idx = _model->index(top_row, 0);
		cur_row_comment_idx = _model->index(secondary_row - 1, 3, cur_row_idx);
		
	}
	else
	{
		cur_row_comment_idx = _model->index(top_row, 3);
	}
	auto cur_comment_data = _model->data(cur_row_comment_idx, Qt::DisplayRole);
	QString comment_str = cur_comment_data.toString();
	return comment_str.toStdString();
	
}
void log_dialog::show_trees()
{
	std::vector<std::string> trees;
	const auto& all_trees = _btree_history._latest_state.cur_tree_indexes;
	for (std::size_t i = 0; i < all_trees.size(); i++)
	{
		trees.push_back(std::to_string(i) + " " + all_trees[i]);
	}
	auto cur_search_dialog = new search_select_dialog(trees, this);
	auto result = cur_search_dialog->run();
}
void log_dialog::goto_graph()
{
	return;
}