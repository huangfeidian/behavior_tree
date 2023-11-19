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
		wait_child,
		dead,
	};



}
