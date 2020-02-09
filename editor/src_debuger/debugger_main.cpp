#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <QDialog>
#include <QApplication>
#include <qfile.h>

#include <any_container/decode.h>

#include "./log_viewer/log_dialog.h"

using namespace std;
using namespace spiritsaway::behavior_tree::editor;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    

	ifstream history_file = ifstream("../../data/history/btree_cmd_log.json");
	std::string file_content = std::string(std::istreambuf_iterator<char>(history_file),
		std::istreambuf_iterator<char>());
	history_file.close();
	if (!json::accept(file_content))
	{
		cout << "invalid history file" << endl;
		return 1;
	}
	auto config_json = json::parse(file_content);
	using temp_cmd_type = std::tuple<std::uint64_t, std::uint32_t, spiritsaway::serialize::any_vector>;
	std::vector<temp_cmd_type> cmds;
	std::deque<behavior_tree::common::agent_cmd_detail> cmd_queue;
	decode(config_json, cmds);
	for (auto one_cmd : cmds)
	{
		auto[ts, cmd_int, params] = one_cmd;
		behavior_tree::common::agent_cmd_detail cur_detail;
		cur_detail = make_tuple(ts, static_cast<behavior_tree::common::agent_cmd>(cmd_int), params);
		cmd_queue.push_back(cur_detail);
	}
	auto cur_dialog = new log_dialog(cmd_queue);
	cur_dialog->resize(500, 600);

    cur_dialog->show();
    return a.exec();
}