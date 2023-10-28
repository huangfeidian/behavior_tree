#pragma once
#include <vector>
#include <unordered_map>
#include <deque>
#include <string>
#include <any_container/decode.h>

#include <spdlog/spdlog.h>

#include <memory>
#include "btree_desc.h"
#include <filesystem>
#include <random>
#include "btree_debug.h"

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

	using spiritsaway::behavior_tree::common::cmd_receiver;
	using spiritsaway::behavior_tree::common::btree_desc;
	using spiritsaway::behavior_tree::common::agent_cmd;
	class agent
	{
	public:
		agent(const std::filesystem::path& in_data_folder, std::shared_ptr<spdlog::logger> in_logger);
		friend class node;
	public:
		bool poll(); // first handle events then handle fronts
		void dispatch_event(const event_type& new_event);
		bool is_running() const
		{
			return m_enabled;
		}
		void notify_stop();
		bool load_btree(const std::string& btree_name);
		json blackboard_get(const std::string& key) const;
		void blackboard_set(const std::string& key, const json& value);
		bool blackboard_has(const std::string& key) const;
		bool blackboard_pop(const std::string & key);
	public:

		virtual std::optional<bool> agent_action(const std::string& action_name, 
			const json::array_t& action_args);
		void reset();
		cmd_receiver* set_debug(cmd_receiver* _in_receiver);
		bool enable(bool enable_flag);
		void push_cmd_queue(std::uint32_t teee_idx, std::uint32_t node_idx, agent_cmd _cmd, const json::array_t& _param);
		void push_cmd_queue(agent_cmd _cmd, const json::array_t& _param) ;
		bool during_debug() const;
		std::uint32_t get_tree_idx(const std::string& tree_name);
		void add_to_front(node* cur_node);
		bool during_poll() const
		{
			return m_during_poll;
		}
		std::shared_ptr<spdlog::logger> logger() const
		{
			return m_logger;
		}
	public:
		node* create_tree(const std::string& btree_name, node* parent);
		std::mt19937& random_generator() { return m_random_generator; }
		auto& int_distribution() { return m_int_distribution; }
		virtual ~agent();
	protected:
		bool m_during_poll = false;
		std::vector<node*> m_fronts; // node ready to run
		std::vector<node*> m_pre_fronts;
		std::vector<event_type> m_events; // events to be handled;
		cmd_receiver* m_cmd_receiver = nullptr;
		node* current_poll_node = nullptr;
		bool reset_flag = false;
		node* cur_root_node = nullptr;
		std::shared_ptr<spdlog::logger> m_logger;
		bool m_enabled = false;
		std::filesystem::path m_data_folder;
		std::mt19937 m_random_generator;
		std::uniform_int_distribution<std::uint32_t> m_int_distribution;
	protected:
		json::object_t m_blackboard;
		std::vector<const btree_desc*> m_tree_descs;
		bool poll_fronts(); // run the nodes
		bool poll_events(); // handle the events;
		void poll_node(node* cur_node);// run one node

	};

}