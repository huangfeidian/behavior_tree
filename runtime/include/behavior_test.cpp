#include <action_agent.h>
#include <thread>
using namespace spiritsaway::behavior_tree::common;
using namespace spiritsaway::behavior_tree::runtime;
using namespace std;
using namespace std::chrono_literals;
struct pair_int_hash
{
	std::size_t operator()(std::pair<std::uint32_t, std::uint32_t> data) const
	{
		return data.first ^ data.second;
	}
};
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
	std::vector<std::string> cur_tree_indexes;
	any_str_map cur_blackboard;
	std::unordered_set<std::pair<std::uint32_t, std::uint32_t>, pair_int_hash> cur_fronts;

	for (int i = 0; i < 1; i++)
	{
		auto logs = cur_agent.dump_cmd_queue();
		for (const auto& one_log : logs)
		{
			auto[ts, cmd, params] = one_log;
			switch (cmd)
			{
			case agent_cmd::snapshot:
			{
				cur_tree_indexes.clear();
				cur_blackboard.clear();
				cur_fronts.clear();
				std::vector< std::pair<std::uint32_t, std::uint32_t>> temp_fronts;
				any_decode(params[0], cur_tree_indexes);
				any_decode(params[1], temp_fronts);
				any_decode(params[2], cur_blackboard);
				for (auto one_node : temp_fronts)
				{
					cur_fronts.insert(one_node);
				}
				cur_logger->info("ts: {} cmd: {}, trees {} blackboards {} fronts {}", ts, magic_enum::enum_name(cmd), encode(cur_tree_indexes).dump(), encode(cur_blackboard).dump(), encode(cur_fronts).dump());
				break;
			}
				
			case agent_cmd::push_tree:
			{
				std::string tree_name;
				std::uint32_t tree_idx;
				any_decode(params[0], tree_name);
				any_decode(params[1], tree_idx);
				if (tree_idx == cur_tree_indexes.size())
				{
					cur_tree_indexes.push_back(tree_name);
					cur_logger->info("ts: {} cmd: {}, tree {} tree_idx {}", ts, magic_enum::enum_name(cmd), cur_tree_indexes[tree_idx], tree_idx);
				}
				break;
			}
			case agent_cmd::node_enter:
			{
				std::uint32_t tree_idx;
				std::uint32_t node_idx;
				any_decode(params[0], tree_idx);
				any_decode(params[1], node_idx);
				if (tree_idx >= cur_tree_indexes.size())
				{
					break;
				}
				cur_fronts.insert(std::make_pair(tree_idx, node_idx));
				cur_logger->info("ts: {} cmd: {}, tree {} node {}", ts, magic_enum::enum_name(cmd), cur_tree_indexes[tree_idx], node_idx);
				break;
			}
			case agent_cmd::node_leave:
			{
				std::uint32_t tree_idx;
				std::uint32_t node_idx;
				bool leave_result;
				any_decode(params[0], tree_idx);
				any_decode(params[1], node_idx);
				any_decode(params[2], leave_result);
				if (tree_idx >= cur_tree_indexes.size())
				{
					break;
				}
				cur_fronts.erase(std::make_pair(tree_idx, node_idx));

				cur_logger->info("ts: {} cmd: {}, tree {} node {} result {}", ts, magic_enum::enum_name(cmd), cur_tree_indexes[tree_idx], node_idx, leave_result);
				break;
			}
			case agent_cmd::node_action:
			{
				std::uint32_t tree_idx;
				std::uint32_t node_idx;
				std::string action_name;
				any_decode(params[0], tree_idx);
				any_decode(params[1], node_idx);
				any_decode(params[2], action_name);
				cur_logger->info("ts: {} cmd: {}, tree {} node {} action {}, action_args {}", ts, magic_enum::enum_name(cmd), cur_tree_indexes[tree_idx], node_idx, action_name, encode(params[3]).dump());
				break;
			}
			case agent_cmd::bb_set:
			{
				std::string bb_key;
				any_value_type bb_value;
				any_decode(params[0], bb_key);
				bb_value = params[1];
				cur_blackboard[bb_key] = bb_value;
				cur_logger->info("ts: {} cmd: {}, bb_key {} bb_value {}", ts, magic_enum::enum_name(cmd), bb_key, encode(bb_value).dump());
				break;
			}
			case agent_cmd::bb_clear:
			{
				cur_logger->info("ts: {} cmd: {}", ts, magic_enum::enum_name(cmd));
				cur_blackboard.clear();
			}
			default:
				break;
			}
		}
		this_thread::sleep_for(100ms);
		timer_manager::instance().poll(std::chrono::system_clock::now());
	}
}