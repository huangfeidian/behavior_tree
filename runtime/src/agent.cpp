#include "agent.h"
#include "nodes.h"
#include <magic_enum.hpp>

namespace spiritsaway::behavior_tree::runtime
{
	using namespace spiritsaway::behavior_tree::common;
	using namespace spiritsaway;
	agent::agent(const std::filesystem::path& in_data_folder, std::shared_ptr<spdlog::logger> in_logger)
		: m_logger(in_logger)
		, m_data_folder(in_data_folder)
	{

	}
	bool agent::during_debug() const
	{
		return m_cmd_receiver != nullptr;
	}
	bool agent::poll()
	{
		if (m_during_poll)
		{
			return false;
		}
		m_during_poll = true;
		push_cmd_queue(agent_cmd::poll_begin, {});
		std::size_t poll_count = 0;
		
		while (true)
		{
			if (!m_enabled)
			{
				return false;
			}

			bool poll_result = false;
			poll_result |= poll_events();
			poll_result |= poll_fronts();
			if (!reset_flag)
			{
				if (!poll_result)
				{
					break;
				}
			}
			else
			{
				reset_flag = false;
				add_to_front(cur_root_node);
			}
			poll_count += 1;
		}
		m_during_poll = false;
		
		if (poll_count)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	bool agent::poll_events()
	{
		if (m_events.empty())
		{
			return false;
		}
		for (const auto& one_event : m_events)
		{
			for (auto one_node : m_fronts)
			{
				if (one_node->handle_event(one_event))
				{
					break;
				}
			}
		}
		m_events.clear();
		return true;
		
	}
	bool agent::poll_fronts()
	{
		if (m_fronts.empty())
		{
			return false;
		}
		m_pre_fronts.clear();
		std::swap(m_pre_fronts, m_fronts);
		int ready_count = 0;
		for (const auto& one_node : m_pre_fronts)
		{
			if (one_node->node_state_is_ready())
			{
				ready_count++;
				poll_node(one_node);
				if (!m_enabled)
				{
					break;
					return false;
				}
			}
			else
			{
				if (one_node->m_state == node_state::blocking)
				{
					add_to_front(one_node);
				}
			}
		}
		m_pre_fronts.clear();
		return m_enabled && ready_count > 0;
	}
	void agent::poll_node(node* cur_node)
	{
		current_poll_node = cur_node;
		cur_node->visit();
		current_poll_node = nullptr;
	}
	void agent::dispatch_event(const event_type& new_event)
	{
		m_events.push_back(new_event);
		poll();
	}
	void agent::notify_stop()
	{
		if (current_poll_node)
		{
			m_logger->warn("btree stop at {}", current_poll_node->debug_info());
		}
		else
		{
			m_logger->warn("btree stop while current_poll_node empty");
		}
		std::swap(m_pre_fronts, m_fronts);
		for (const auto i : m_pre_fronts)
		{
			m_logger->warn(i->debug_info());
			i->interrupt();
		}
		m_pre_fronts.clear();
		m_fronts.clear();
		m_events.clear();
		current_poll_node = nullptr;
		m_enabled = false;
	}
	json agent::blackboard_get(const std::string& key) const
	{
		auto cur_iter = m_blackboard.find(key);
		if (cur_iter == m_blackboard.end())
		{
			return {};
		}
		else
		{
			return cur_iter->second;
		}
	}
	bool agent::blackboard_has(const std::string& key) const
	{
		auto cur_iter = m_blackboard.find(key);
		return cur_iter != m_blackboard.end();
	}
	void agent::blackboard_set(const std::string& key, const json& value)
	{
		m_blackboard[key] = value;
		if (m_cmd_receiver)
		{
			json::array_t params;
			params.push_back(key);
			params.push_back(value);
			push_cmd_queue(agent_cmd::bb_set, params);
		}
		
	}
	bool agent::blackboard_pop(const std::string& key)
	{
		return m_blackboard.erase(key) == 1;
	}
	void agent::push_cmd_queue(std::uint32_t tree_idx, std::uint32_t node_idx, agent_cmd _cmd, const json::array_t& _param)
	{

		if (m_cmd_receiver)
		{
			m_cmd_receiver->add(tree_idx, node_idx, _cmd, _param);
		}

	}
	void agent::push_cmd_queue(agent_cmd _cmd, const json::array_t& _param)
	{
		if (current_poll_node)
		{
			push_cmd_queue(get_tree_idx(current_poll_node->btree_config.tree_name), current_poll_node->m_node_idx, _cmd, _param);
		}
		else
		{
			push_cmd_queue(0, 0, _cmd, _param);
		}
	}

	std::uint32_t agent::get_tree_idx(const std::string& tree_name)
	{
		for (std::uint32_t i = 0; i < m_tree_descs.size(); i++)
		{
			if (m_tree_descs[i]->tree_name == tree_name)
			{
				return i;
			}
		}
		m_logger->error("get_tree_idx for tree_name {} invalid", tree_name);
		return 0;
		
	}
	cmd_receiver* agent::set_debug(cmd_receiver* _in_cmd_receiver)
	{
		auto result = m_cmd_receiver;
		m_cmd_receiver = _in_cmd_receiver;
		if(m_cmd_receiver)
		{
			json::array_t snapshot_param;
			std::vector<std::string> _tree_names;
			for (const auto& one_tree : m_tree_descs)
			{
				_tree_names.push_back(one_tree->tree_name);
			}
			snapshot_param.push_back(_tree_names);
			snapshot_param.push_back(m_blackboard);
			json::array_t node_fronts;
			for (const auto one_node : m_fronts)
			{
				node_fronts.push_back(std::make_pair(get_tree_idx(one_node->tree_name()), one_node->m_node_idx));
			}
			snapshot_param.push_back(node_fronts);

			push_cmd_queue(agent_cmd::snapshot, snapshot_param);
		}
		return result;
	}
	std::optional<bool> agent::agent_action(const std::string& action_name, 
		const json::array_t& action_args)
	{
		return std::nullopt;
	}
	void agent::reset()
	{
		m_blackboard.clear();
		if (m_cmd_receiver)
		{
			push_cmd_queue(agent_cmd::reset, {});
		}
		reset_flag = true;
		notify_stop();
	}

	bool agent::load_btree(const std::string& btree_name)
	{
		if (cur_root_node)
		{
			reset();
			if (cur_root_node->tree_name() == btree_name)
			{
				return true;
			}
			else
			{
				delete cur_root_node;
				cur_root_node = nullptr;
			}
		}
		cur_root_node = create_tree(btree_name, nullptr);
		if (!cur_root_node)
		{
			return false;
		}
		add_to_front(cur_root_node);
		return true;

	}
	node* agent::create_tree(const std::string& btree_name, node* parent)
	{
		const btree_desc* cur_btree = nullptr;
		for (const auto& one_item : m_tree_descs)
		{
			if (one_item->tree_name == btree_name)
			{
				cur_btree = one_item;
				break;
			}
		}
		if (!cur_btree)
		{
			cur_btree = new btree_desc((m_data_folder / btree_name).string(), m_logger);
			if (!cur_btree->is_valid())
			{
				m_logger->warn("fail to load btree {}", btree_name);
				return nullptr;
			}
			m_tree_descs.push_back(cur_btree);
			if (m_cmd_receiver)
			{
				json::array_t params;
				params.push_back(cur_btree->tree_name);
				push_cmd_queue(agent_cmd::push_tree, params);
			}
		}
		
		auto temp_root = node::create_node_by_idx(*cur_btree, 0, parent, this);
		return temp_root;
	}
	bool agent::enable(bool enable_flag)
	{
		if (enable_flag)
		{
			if (m_enabled)
			{
				return false;
			}
			else
			{
				m_enabled = true;
				poll();
				return true;
			}
		}
		else
		{
			if (!m_enabled)
			{
				return false;
			}
			else
			{
				m_enabled = false;
				notify_stop();
				return true;
			}
		}
	}

	void agent::add_to_front(node* cur_node)
	{
		for (auto one_node : m_fronts)
		{
			if (one_node == cur_node)
			{
				return;
			}
		}
		m_fronts.push_back(cur_node);
	}

	agent::~agent()
	{
		notify_stop();
		if (cur_root_node)
		{
			delete cur_root_node;
			cur_root_node = nullptr;
		}
		for (auto one_tree : m_tree_descs)
		{
			delete one_tree;
		}
		m_tree_descs.clear();
		m_fronts.clear();
	}
}