#pragma once
#include <vector>
#include <unordered_map>
#include <deque>
#include <string>
#include <serialize/decode.h>
#include <serialize/any_value.h>

#include <spdlog/spdlog.h>
#include "timer_manager.hpp"

#include <behavior/btree.h>

#include <behavior/nodes.h>


namespace behavior_tree::runtime
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
	public:
		bool during_poll = false;
		std::vector<node*> _fronts; // node ready to run
		std::vector<node*> pre_fronts;
		std::vector<event_type> _events; // events to be handled;
		behavior_tree::common::any_str_map _blackboard;
		//std::unordered_map<const node*, timer_handler> _timers;
		virtual std::optional<bool> agent_action(const std::string& action_name, 
			const behavior_tree::common::any_vector& action_args);
		void reset();
		bool set_debug(bool debug_flag);
		bool enable(bool enable_flag);
	protected:
		node* current_poll_node = nullptr;
		bool reset_flag = false;
		node* cur_root_node = nullptr;
		std::shared_ptr<spdlog::logger> _logger;
		bool _enabled = false;
		bool _debug_on = false;
		std::unordered_set<timer_handler, timer_handler_hash> _timers;
	private:
		bool poll_fronts(); // run the nodes
		bool poll_events(); // handle the events;
		void poll_node(node* cur_node);// run one node

	};

}