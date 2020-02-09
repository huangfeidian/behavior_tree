#include <behavior/btree_trace.h>

using namespace spiritsaway::behavior_tree::common;

btree_state btree_state::copy_state() const
{
	btree_state result;
	result.cur_blackboard = cur_blackboard;
	result.cur_tree_indexes = cur_tree_indexes;
	result.cur_fronts = cur_fronts;
	return result;
}
void btree_state::run_one_cmd(agent_cmd_detail one_cmd)
{
	auto[ts, cmd, params] = one_cmd;
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

		break;
	}
	case agent_cmd::node_action:
	{
		//std::uint32_t tree_idx;
		//std::uint32_t node_idx;
		//std::string action_name;
		//any_decode(params[0], tree_idx);
		//any_decode(params[1], node_idx);
		//any_decode(params[2], action_name);
		//cur_logger->info("ts: {} cmd: {}, tree {} node {} action {}, action_args {}", format_timepoint(ts), magic_enum::enum_name(cmd), cur_tree_indexes[tree_idx], node_idx, action_name, encode(params[3]).dump());
		break;
	}
	case agent_cmd::bb_set:
	{
		std::string bb_key;
		any_value_type bb_value;
		any_decode(params[0], bb_key);
		bb_value = params[1];
		cur_blackboard[bb_key] = bb_value;
		break;
	}
	case agent_cmd::bb_clear:
	{
		cur_blackboard.clear();
	}
	default:
		break;
	}
}
void btree_state::run_cmd_to(std::size_t cmd_idx)
{
	for (std::size_t i = 0; i <= cmd_idx && i < _cmds.size(); i++)
	{
		run_one_cmd(_cmds[i]);
	}

}
bool btree_trace::push_cmd(agent_cmd_detail in_cmd)
{
	const auto& [ts, cmd, params] = in_cmd;
	switch (cmd)
	{
	case agent_cmd::snapshot:
	case agent_cmd::poll_begin:
	{
		auto new_state = _latest_state;
		new_state._cmds.push_back(in_cmd);
		_poll_states.push_back(new_state);
		_latest_state.run_one_cmd(in_cmd);
		return true;
	}
	default:
	{
		_latest_state.run_one_cmd(in_cmd);
		_poll_states.back()._cmds.push_back(in_cmd);
		return false;
	}
	}
}