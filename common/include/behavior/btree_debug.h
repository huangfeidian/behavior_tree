#pragma once
#include <cstdint>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace spiritsaway::behavior_tree::common
{
    enum class agent_cmd
	{
		poll_begin = 0,
		snapshot, //tree_indexes node_idxes blackboard
		push_tree,// new tree name, tree_idx
		node_enter,//
		node_leave,//
		node_action,//action_name, action_args
		bb_set,//key, value
		bb_remove, //key
		bb_clear,
		reset,
	};
    struct cmd_receiver
    {
        virtual void add(std::uint32_t tree_idx, std::uint32_t node_idx, agent_cmd _cmd, const json::array_t& _param)
        {

        }
	};
}