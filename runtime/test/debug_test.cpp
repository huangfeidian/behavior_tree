#include <action_agent.h>
#include <thread>
#include <ctime>
#include <behavior/btree_trace.h>
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
	std::string test_btree_name = "new_btree_1.btree";
	const std::string btree_dir = "../../data/btree/";
	btree_desc::set_btree_directory(btree_dir);
	action_agent cur_agent;
	cur_agent.set_debug(true);
	cur_agent.load_btree(test_btree_name);
	cur_agent.enable(true);
	auto cur_logger = spiritsaway::behavior_tree::common::logger_mgr::instance().create_logger("btree");
	btree_trace _trace;
	std::vector<agent_cmd_detail> total_logs;
	for (int i = 0; i < 2; i++)
	{
		auto logs = cur_agent.dump_cmd_queue();
		for (auto one_log : logs)
		{
			total_logs.push_back(one_log);
			if (_trace.push_cmd(one_log))
			{
				auto[ts, cmd, params] = one_log;
				cur_logger->info("ts: {} cmd: {}, tree {} fronts {} blackboards {} ", format_timepoint(ts), magic_enum::enum_name(cmd), encode(_trace._latest_state.cur_tree_indexes).dump(), encode(_trace._latest_state.cur_fronts).dump(), encode(_trace._latest_state.cur_blackboard).dump());
			}
		}
		this_thread::sleep_for(100ms);
		timer_manager::instance().poll(std::chrono::system_clock::now());
	}
	cur_logger->info("after run tree {} fronts {} blackboards {} ",  encode(_trace._latest_state.cur_tree_indexes).dump(), encode(_trace._latest_state.cur_fronts).dump(), encode(_trace._latest_state.cur_blackboard).dump());
	ofstream cmd_log_file = ofstream("../../data/history/btree_cmd_log.json");
	cmd_log_file << encode(total_logs).dump(4) << endl;
	cmd_log_file.close();
}