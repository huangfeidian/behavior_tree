#pragma once
namespace spiritsaway::behavior_tree::common
{
	enum class node_type
	{
		invalid = 0,
		root,
		negative,
		sequence,
		always_seq,
		random_seq,
		if_else,
		while_loop,
		wait_event,
		reset,
		sub_tree,
		parallel,
		action,
		always_true,
		select,
		probility

	};
	enum class action_arg_type
	{
		plain = 0,
		blackboard,
	};
	enum class node_state
	{
		init = 0,
		entering,
		awaken,
		revisiting,
		blocking,
		leaving,
		wait_child,
		dead,
	};
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
	};
	
}
