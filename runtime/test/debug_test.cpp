#include <action_agent.h>
#include <thread>
#include <ctime>
#include <filesystem>
#include <any_container/encode.h>
#include <tree_editor/common/debug_cmd.h>
#include "timer_manager.hpp"
#include <magic_enum.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

using namespace spiritsaway::behavior_tree::common;
using namespace spiritsaway::behavior_tree::runtime;
using namespace spiritsaway;
using namespace std;
using namespace std::chrono_literals;
using spiritsaway::serialize::encode;
using spiritsaway::tree_editor::node_trace_cmd;

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
std::shared_ptr<spdlog::logger> create_logger(const std::string& name)
{
	
	auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	console_sink->set_level(spdlog::level::debug);
	std::string pattern = "[" + name + "] [%^%l%$] %v";
	console_sink->set_pattern(pattern);

	auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(name + ".log", true);
	file_sink->set_level(spdlog::level::trace);
	auto logger = std::make_shared<spdlog::logger>(name, spdlog::sinks_init_list{ console_sink, file_sink });
	logger->set_level(spdlog::level::trace);
	return logger;
}

struct btree_debug_cmd_receiver : public cmd_receiver
{
	std::deque<node_trace_cmd> _cmd_buffer;
	void add(std::uint32_t tree_idx, std::uint32_t node_idx, agent_cmd _cmd, const json::array_t& _param)
	{
		auto microsecondsUTC = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		node_trace_cmd cur_cmd_trace;
		switch (_cmd)
		{
		case spiritsaway::behavior_tree::common::agent_cmd::poll_begin:
			cur_cmd_trace.cmd = magic_enum::enum_name(tree_editor::nodes_cmd::group_begin);
			cur_cmd_trace.detail = _param;
			break;
		case spiritsaway::behavior_tree::common::agent_cmd::snapshot:
			cur_cmd_trace.cmd = magic_enum::enum_name(tree_editor::nodes_cmd::snapshot);
			cur_cmd_trace.detail = _param;
			break;
		case spiritsaway::behavior_tree::common::agent_cmd::push_tree:
			cur_cmd_trace.cmd = magic_enum::enum_name(tree_editor::nodes_cmd::push_tree);
			cur_cmd_trace.detail = _param;
			break;
		case spiritsaway::behavior_tree::common::agent_cmd::node_enter:
			cur_cmd_trace.cmd = magic_enum::enum_name(tree_editor::nodes_cmd::enter);
			cur_cmd_trace.detail = _param;
			break;
		case spiritsaway::behavior_tree::common::agent_cmd::node_leave:
			cur_cmd_trace.cmd = magic_enum::enum_name(tree_editor::nodes_cmd::leave);
			cur_cmd_trace.detail = _param;

			break;
		case spiritsaway::behavior_tree::common::agent_cmd::node_action:
		{
			cur_cmd_trace.cmd = magic_enum::enum_name(tree_editor::nodes_cmd::mutate);
			json::array_t action_cmd_detail;
			action_cmd_detail.push_back(magic_enum::enum_name(agent_cmd::node_action));
			action_cmd_detail.push_back(_param);
			cur_cmd_trace.detail = action_cmd_detail;
			break;
		}
		case spiritsaway::behavior_tree::common::agent_cmd::bb_set:
		{
			cur_cmd_trace.cmd = magic_enum::enum_name(tree_editor::nodes_cmd::mutate);
			json::array_t action_cmd_detail;
			action_cmd_detail.push_back("variable");
			tree_editor::var_trace_cmd cur_bb_detail;
			cur_bb_detail._cmd = tree_editor::var_cmd::set;
			cur_bb_detail._detail = _param;

			action_cmd_detail.push_back(cur_bb_detail.encode());
			cur_cmd_trace.detail = action_cmd_detail;
			break;
		}
		case spiritsaway::behavior_tree::common::agent_cmd::bb_clear:
		{
			cur_cmd_trace.cmd = magic_enum::enum_name(tree_editor::nodes_cmd::mutate);
			json::array_t action_cmd_detail;
			action_cmd_detail.push_back("variable");
			tree_editor::var_trace_cmd cur_bb_detail;
			cur_bb_detail._cmd = tree_editor::var_cmd::reset;
			cur_bb_detail._detail = _param;

			action_cmd_detail.push_back(cur_bb_detail.encode());
			cur_cmd_trace.detail = action_cmd_detail;
			break;
		}

		case spiritsaway::behavior_tree::common::agent_cmd::bb_remove:
		{
			cur_cmd_trace.cmd = magic_enum::enum_name(tree_editor::nodes_cmd::mutate);
			json::array_t action_cmd_detail;
			action_cmd_detail.push_back("variable");
			tree_editor::var_trace_cmd cur_bb_detail;
			cur_bb_detail._cmd = tree_editor::var_cmd::remove;
			cur_bb_detail._detail = _param;

			action_cmd_detail.push_back(cur_bb_detail.encode());
			cur_cmd_trace.detail = action_cmd_detail;
			break;
		}

		default:
			break;
		}
		cur_cmd_trace.ts = microsecondsUTC;
		cur_cmd_trace.tree_idx = tree_idx;
		cur_cmd_trace.node_idx = node_idx;
		_cmd_buffer.push_back(cur_cmd_trace);
	}
};
int main()
{
	std::string test_btree_name = "new_btree_1.json";
	std::filesystem::path data_folder = "../../data/btree";
	auto cur_logger = create_logger("btree");
	action_agent cur_agent(data_folder, cur_logger);
	auto cur_receiver = new btree_debug_cmd_receiver();
	cur_agent.set_debug(cur_receiver);
	cur_agent.load_btree(test_btree_name);
	cur_agent.enable(true);
	auto cur_agent_state = cur_agent.encode();
	cur_agent.enable(false);
	cur_agent.decode(cur_agent_state);
	cur_agent.enable(true);
	spiritsaway::tree_editor::tree_state_traces _trace;
	std::vector<node_trace_cmd> total_logs;
	for (int i = 0; i < 5; i++)
	{
		while(!cur_receiver->_cmd_buffer.empty())
		{
			auto one_log = cur_receiver->_cmd_buffer.front();
			cur_receiver->_cmd_buffer.pop_front();
			total_logs.push_back(one_log);
			if (_trace.push_cmd(one_log) ||true)
			{
				cur_logger->info("ts: {} cmd: {}, args {} tree {} fronts {} blackboards {} ", format_timepoint(one_log.ts), one_log.cmd, encode(one_log.detail).dump(), encode(_trace._latest_state->tree_indexes).dump(), encode(_trace._latest_state->_current_nodes).dump(), encode(_trace._latest_state->_vars).dump());
			}
		}
		this_thread::sleep_for(1000ms);
		timer_manager::instance().poll(std::chrono::system_clock::now());
	}
	cur_logger->info("after run tree {} fronts {} blackboards {} ", encode(_trace._latest_state->tree_indexes).dump(), encode(_trace._latest_state->_current_nodes).dump(), encode(_trace._latest_state->_vars).dump());
	json::object_t dump_file_content;
	dump_file_content["entity_id"] = "sample";
	dump_file_content["debug_cmds"] = encode(total_logs);
	ofstream cmd_log_file = ofstream("../../data/history/btree_cmd_log.json");
	cmd_log_file << json(dump_file_content).dump(4) << endl;
	cmd_log_file.close();
}