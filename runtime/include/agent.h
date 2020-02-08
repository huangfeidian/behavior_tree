#pragma once
#include <vector>
#include <unordered_map>
#include <deque>
#include <string>
#include <any_container/decode.h>
#include <any_container/any_value.h>

#include <spdlog/spdlog.h>
#include "timer_manager.hpp"

#include <behavior/btree.h>

#include <behavior/nodes.h>

namespace spiritsaway::behavior_tree::runtime
{
	class node;
	class agent;
	class btree;
	using event_type = std::string;
	enum class arg_value_choice
	{
		plain_value = 0,
		blackboard_value,
	};

	using spiritsaway::behavior_tree::common::agent_cmd;
	using spiritsaway::behavior_tree::common::agent_cmd_detail;
	class agent
	{
	public:
		agent();
		friend class node;
	public:
		bool poll(); // first handle events then handle fronts
		void dispatch_event(const event_type& new_event);
		bool is_running() const
		{
			return _enabled;
		}
		void notify_stop();
		bool load_btree(const std::string& btree_name);
		spiritsaway::serialize::any_value_type blackboard_get(const std::string& key) const;
		void blackboard_set(const std::string& key, const spiritsaway::serialize::any_value_type& value);
		bool blackboard_has(const std::string& key) const;
	public:
		bool during_poll = false;
		std::vector<node*> _fronts; // node ready to run
		std::vector<node*> pre_fronts;
		std::vector<event_type> _events; // events to be handled;
		//std::unordered_map<const node*, timer_handler> _timers;
		virtual std::optional<bool> agent_action(const std::string& action_name, 
			const spiritsaway::serialize::any_vector& action_args);
		void reset();
		bool set_debug(bool debug_flag);
		bool enable(bool enable_flag);
		std::vector<agent_cmd_detail> dump_cmd_queue();
		void push_cmd_queue(agent_cmd _cmd, const spiritsaway::serialize::any_vector& _param);
		bool during_debug() const;
		std::uint32_t get_tree_idx(const std::string& tree_name);
	protected:
		

	protected:
		

		node* current_poll_node = nullptr;
		bool reset_flag = false;
		node* cur_root_node = nullptr;
		std::shared_ptr<spdlog::logger> _logger;
		bool _enabled = false;
		bool _debug_on = false;
		std::unordered_set<timer_handler, timer_handler_hash> _timers;
	private:
		spiritsaway::serialize::any_str_map _blackboard;
		std::vector<std::string> _tree_indexes;
		std::deque<agent_cmd_detail> _cmd_queue;
		bool poll_fronts(); // run the nodes
		bool poll_events(); // handle the events;
		void poll_node(node* cur_node);// run one node

	};

}