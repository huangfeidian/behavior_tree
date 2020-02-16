#pragma once
#include <any_container/any_value.h>
#include <unordered_set>

#include "btree.h"
namespace spiritsaway::behavior_tree::common
{
	struct pair_int_hash
	{
		std::size_t operator()(std::pair<std::uint32_t, std::uint32_t> data) const
		{
			return data.first ^ data.second;
		}
	};
	enum class debug_mode
	{
		stop,
		run_through,
		run_once,
	};
	class btree_state
	{
	public:
		std::vector<agent_cmd_detail> _cmds;
		spiritsaway::serialize::any_str_map cur_blackboard;// poll begin blackboard
		std::vector<std::string> cur_tree_indexes; // poll begin tree_idxes
		std::unordered_set<std::pair<std::uint32_t, std::uint32_t>, pair_int_hash> cur_fronts;// poll begin fronts

		void run_one_cmd(agent_cmd_detail one_cmd);
		void run_cmd_to(std::size_t cmd_idx);
		btree_state copy_state() const;//without cmds
	};
	class btree_trace
	{
	public:
		std::vector<btree_state> _poll_states;
		btree_state _latest_state;

		bool push_cmd(agent_cmd_detail in_cmd);//return true if one new round begin
		btree_state run_cmd_to(std::size_t _poll_idx, std::size_t _cmd_idx) const;

	};
}