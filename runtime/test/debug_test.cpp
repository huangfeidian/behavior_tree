#include <action_agent.h>
#include <thread>
#include <ctime>
#include <filesystem>
#include <any_container/encode.h>

using namespace spiritsaway::behavior_tree::common;
using namespace spiritsaway::behavior_tree::runtime;
using namespace std;
using namespace std::chrono_literals;


std::string format_timepoint(std::uint64_t milliseconds_since_epoch)
{
	auto epoch_begin = std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>();
	auto cur_timepoint = epoch_begin + std::chrono::milliseconds(milliseconds_since_epoch);
	auto cur_time_t = std::chrono::system_clock::to_time_t(cur_timepoint);

	struct tm * timeinfo;
	char buffer[80];

	timeinfo = localtime(&cur_time_t);

	strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S ", timeinfo);
	return std::string(buffer) + std::to_string(milliseconds_since_epoch % 1000) + "ms";
}
int main()
{
	std::string test_btree_name = "new_btree_1.json";
	std::filesystem::path data_folder = "../../data/btree";
	action_agent cur_agent(data_folder);
	cur_agent.set_debug(true);
	cur_agent.load_btree(test_btree_name);
	cur_agent.enable(true);
	auto cur_logger = spiritsaway::behavior_tree::common::logger_mgr::instance().create_logger("btree");
	spiritsaway::tree_editor::tree_state_traces _trace;
	std::vector<node_trace_cmd> total_logs;
	for (int i = 0; i < 2; i++)
	{
		auto logs = cur_agent.dump_cmd_queue();
		for (auto one_log : logs)
		{
			total_logs.push_back(one_log);
			if (_trace.push_cmd(one_log))
			{
				cur_logger->info("ts: {} cmd: {}, tree {} fronts {} blackboards {} ", format_timepoint(one_log.ts), one_log.cmd, spiritsaway::serialize::encode(_trace._latest_state->tree_indexes).dump(), spiritsaway::serialize::encode(_trace._latest_state->_current_nodes).dump(), spiritsaway::serialize::encode(_trace._latest_state->_vars).dump());
			}
		}
		this_thread::sleep_for(100ms);
		timer_manager::instance().poll(std::chrono::system_clock::now());
	}
	cur_logger->info("after run tree {} fronts {} blackboards {} ", spiritsaway::serialize::encode(_trace._latest_state->tree_indexes).dump(), spiritsaway::serialize::encode(_trace._latest_state->_current_nodes).dump(), spiritsaway::serialize::encode(_trace._latest_state->_vars).dump());
	ofstream cmd_log_file = ofstream("../../data/history/btree_cmd_log.json");
	cmd_log_file << spiritsaway::serialize::encode(total_logs).dump(4) << endl;
	cmd_log_file.close();
}