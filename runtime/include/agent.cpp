#include <agent.h>
#include <nodes.h>
#include <logger.h>

namespace spiritsaway::behavior_tree::runtime
{
	using namespace spiritsaway::behavior_tree::common;
	agent::agent():
		_logger(std::move(logger_mgr::instance().create_logger("btree")))
	{

	}
	bool agent::during_debug() const
	{
		return _debug_on;
	}
	bool agent::poll()
	{
		if (during_poll)
		{
			return false;
		}
		during_poll = true;
		std::size_t poll_count = 0;
		if (_debug_on)
		{
			_logger->info("agent {}: poll begins with fronts:", reinterpret_cast<intptr_t>(this));
			for (const auto one_node : _fronts)
			{
				_logger->info("{}", one_node->debug_info());
			}
		}
		while (true)
		{
			if (!_enabled)
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
				_fronts.push_back(cur_root_node);
			}
			poll_count += 1;
		}
		during_poll = false;
		if (_debug_on)
		{
			_logger->info("agent {}: poll end", reinterpret_cast<intptr_t>(this));
		}
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
		if (_events.empty())
		{
			return false;
		}
		for (const auto& one_event : _events)
		{
			for (auto one_node : _fronts)
			{
				if (one_node->handle_event(one_event))
				{
					break;
				}
			}
		}
		_events.clear();
		return true;
		
	}
	bool agent::poll_fronts()
	{
		if (_fronts.empty())
		{
			return false;
		}
		std::swap(pre_fronts, _fronts);
		int ready_count = 0;
		for (const auto& one_node : pre_fronts)
		{
			if (one_node->node_state_is_ready())
			{
				ready_count++;
				poll_node(one_node);
				if (!_enabled)
				{
					break;
					return false;
				}
			}
			else
			{
				_fronts.push_back(one_node);
			}
		}
		pre_fronts.clear();
		return _enabled && ready_count > 0;
	}
	void agent::poll_node(node* cur_node)
	{
		current_poll_node = cur_node;
		if (_debug_on)
		{
			_logger->info("poll node {}", cur_node->debug_info());
		}
		cur_node->visit();
		current_poll_node = nullptr;
	}
	void agent::dispatch_event(const event_type& new_event)
	{
		_events.push_back(new_event);
		poll();
	}
	void agent::notify_stop()
	{
		if (current_poll_node)
		{
			_logger->warn("btree stop at {}", current_poll_node->debug_info());
		}
		else
		{
			_logger->warn("btree stop while current_poll_node empty");
		}
		for (auto one_timer : _timers)
		{
			one_timer.cancel();
		}
		_logger->warn("fronts begin ");
		for (const auto i : pre_fronts)
		{
			_logger->warn(i->debug_info());
			i->interrupt();
		}
		_logger->warn("fronts ends ");
		_fronts.clear();
		_events.clear();
		current_poll_node = nullptr;
		_enabled = false;
	}
	any_value_type agent::blackboard_get(const std::string& key) const
	{
		auto cur_iter = _blackboard.find(key);
		if (cur_iter == _blackboard.end())
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
		auto cur_iter = _blackboard.find(key);
		return cur_iter != _blackboard.end();
	}
	void agent::blackboard_set(const std::string& key, const any_value_type& value)
	{
		_blackboard[key] = value;
		if (_debug_on)
		{
			any_vector params;
			params.push_back(key);
			params.push_back(value);
			push_cmd_queue(agent_cmd::bb_set, params);
		}
		
	}
	void agent::push_cmd_queue(agent_cmd _cmd, const any_vector& _param)
	{
		auto microsecondsUTC = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		_cmd_queue.emplace_back(microsecondsUTC, _cmd, _param);

	}
	void agent::dump_cmd_queue(std::deque<agent_cmd_detail>& dest)
	{
		while (!_cmd_queue.empty())
		{
			dest.push_back(_cmd_queue.front());
			_cmd_queue.pop_front();
		}
	}
	std::uint32_t agent::get_tree_idx(const std::string& tree_name)
	{
		for (std::uint32_t i = 0; i < _tree_indexes.size(); i++)
		{
			if (_tree_indexes[i] == tree_name)
			{
				return i;
			}
		}
		_tree_indexes.push_back(tree_name);
		if (_debug_on)
		{
			any_vector params;
			params.push_back(any_encode(tree_name));
			params.push_back(any_encode(_tree_indexes.size() - 1));
			push_cmd_queue(agent_cmd::push_tree, params);
		}
		return _tree_indexes.size() - 1;
	}
	bool agent::set_debug(bool debug_flag)
	{
		if (debug_flag == _debug_on)
		{
			return false;
		}
		if (_debug_on)
		{
			_cmd_queue.clear();
		}
		else
		{
			any_vector snapshot_param;
			snapshot_param.push_back(spiritsaway::serialize::any_encode(_tree_indexes));
			snapshot_param.push_back(_blackboard);
			any_vector node_fronts;
			for (const auto one_node : _fronts)
			{
				node_fronts.push_back(spiritsaway::serialize::any_encode(std::make_pair(get_tree_idx(one_node->tree_name()), one_node->_node_idx)));
			}
			snapshot_param.push_back(node_fronts);

			push_cmd_queue(agent_cmd::snapshot, snapshot_param);
		}
		return true;
	}
	std::optional<bool> agent::agent_action(const std::string& action_name, 
		const any_vector& action_args)
	{
		return std::nullopt;
	}
	void agent::reset()
	{
		_blackboard.clear();
		if (_debug_on)
		{
			push_cmd_queue(agent_cmd::bb_clear, {});
		}
		reset_flag = true;
		notify_stop();
	}

	bool agent::load_btree(const std::string& btree_name)
	{
		const btree_desc* cur_btree = btree_desc::load(btree_name, _logger);
		if (!cur_btree)
		{
			_logger->warn("fail to load btree {}", btree_name);
			return false;
		}
		get_tree_idx(btree_name);
		cur_root_node = node::create_node_by_idx(*cur_btree, 0, nullptr, this);
		if (!cur_root_node)
		{
			return false;
		}
		_fronts.push_back(cur_root_node);
		return true;

	}
	bool agent::enable(bool enable_flag)
	{
		if (enable_flag)
		{
			if (_enabled)
			{
				return false;
			}
			else
			{
				_enabled = true;
				poll();
				return true;
			}
		}
		else
		{
			if (!_enabled)
			{
				return false;
			}
			else
			{
				_enabled = false;
				notify_stop();
				return true;
			}
		}
	}
}