#include "log_dialog.h"
#include <qfile.h>
#include <chrono>
#include <ctime>

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