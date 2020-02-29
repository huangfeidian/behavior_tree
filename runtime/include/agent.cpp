#include <agent.h>
#include <nodes.h>
#include <logger.h>
#include <magic_enum.hpp>

namespace spiritsaway::behavior_tree::runtime
{
	using namespace spiritsaway::behavior_tree::common;
	using namespace spiritsaway;
	agent::agent(const std::filesystem::path& _in_data_folder)
		: _logger(std::move(logger_mgr::instance().create_logger("btree")))
		, data_folder(_in_data_folder)
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
		push_cmd_queue(agent_cmd::poll_begin, {});
		std::size_t poll_count = 0;
		
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
		for (const auto i : pre_fronts)
		{
			_logger->warn(i->debug_info());
			i->interrupt();
		}
		_fronts.clear();
		_events.clear();
		current_poll_node = nullptr;
		_enabled = false;
	}
	json agent::blackboard_get(const std::string& key) const
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
	void agent::blackboard_set(const std::string& key, const json& value)
	{
		_blackboard[key] = value;
		if (_debug_on)
		{
			json::array_t params;
			params.push_back(key);
			params.push_back(value);
			push_cmd_queue(agent_cmd::bb_set, params);
		}
		
	}
	bool agent::blackboard_pop(const std::string& key)
	{
		return _blackboard.erase(key) == 1;
	}
	void agent::push_cmd_queue(std::uint32_t tree_idx, std::uint32_t node_idx, agent_cmd _cmd, const json::array_t& _param)
	{

		auto microsecondsUTC = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		node_trace_cmd cur_cmd_trace;
		switch (_cmd)
		{
		case spiritsaway::behavior_tree::common::agent_cmd::poll_begin:
			cur_cmd_trace.cmd = "poll begin";
			cur_cmd_trace.detail = _param;
			break;
		case spiritsaway::behavior_tree::common::agent_cmd::snapshot:
			cur_cmd_trace.cmd = magic_enum::enum_name(tree_editor::nodes_cmd::snapshot);
			cur_cmd_trace.detail = _param;
			break;
		case spiritsaway::behavior_tree::common::agent_cmd::push_tree:
			cur_cmd_trace.cmd = magic_enum::enum_name(tree_editor::nodes_cmd::push_tree);
			cur_cmd_trace.detail = _param;
			break;
		case spiritsaway::behavior_tree::common::agent_cmd::node_enter:
			cur_cmd_trace.cmd = magic_enum::enum_name(tree_editor::nodes_cmd::enter);
			cur_cmd_trace.detail = _param;
			break;
		case spiritsaway::behavior_tree::common::agent_cmd::node_leave:
			cur_cmd_trace.cmd = magic_enum::enum_name(tree_editor::nodes_cmd::leave);
			cur_cmd_trace.detail = _param;

			break;
		case spiritsaway::behavior_tree::common::agent_cmd::node_action:
		{
			cur_cmd_trace.cmd = magic_enum::enum_name(tree_editor::nodes_cmd::mutate);
			json::array_t action_cmd_detail;
			action_cmd_detail.push_back(magic_enum::enum_name(agent_cmd::node_action));
			action_cmd_detail.push_back(_param);
			cur_cmd_trace.detail = action_cmd_detail;
			break;
		}
		case spiritsaway::behavior_tree::common::agent_cmd::bb_set:
		{
			cur_cmd_trace.cmd = magic_enum::enum_name(tree_editor::nodes_cmd::mutate);
			json::array_t action_cmd_detail;
			action_cmd_detail.push_back("variable");
			tree_editor::var_trace_cmd cur_bb_detail;
			cur_bb_detail._cmd = tree_editor::var_cmd::set;
			cur_bb_detail._detail = _param;

			action_cmd_detail.push_back(cur_bb_detail.encode());
			cur_cmd_trace.detail = action_cmd_detail;
			break;
		}
		case spiritsaway::behavior_tree::common::agent_cmd::bb_clear:
		{
			cur_cmd_trace.cmd = magic_enum::enum_name(tree_editor::nodes_cmd::mutate);
			json::array_t action_cmd_detail;
			action_cmd_detail.push_back("variable");
			tree_editor::var_trace_cmd cur_bb_detail;
			cur_bb_detail._cmd = tree_editor::var_cmd::reset;
			cur_bb_detail._detail = _param;

			action_cmd_detail.push_back(cur_bb_detail.encode());
			cur_cmd_trace.detail = action_cmd_detail;
			break;
		}
			
		case spiritsaway::behavior_tree::common::agent_cmd::bb_remove:
		{
			cur_cmd_trace.cmd = magic_enum::enum_name(tree_editor::nodes_cmd::mutate);
			json::array_t action_cmd_detail;
			action_cmd_detail.push_back("variable");
			tree_editor::var_trace_cmd cur_bb_detail;
			cur_bb_detail._cmd = tree_editor::var_cmd::remove;
			cur_bb_detail._detail = _param;

			action_cmd_detail.push_back(cur_bb_detail.encode());
			cur_cmd_trace.detail = action_cmd_detail;
			break;
		}
			
		default:
			break;
		}
		cur_cmd_trace.cmd = magic_enum::enum_name(_cmd);
		cur_cmd_trace.ts = microsecondsUTC;
		cur_cmd_trace.tree_idx = tree_idx;
		cur_cmd_trace.node_idx = node_idx;
		_cmd_queue.push_back(cur_cmd_trace);

	}
	void agent::push_cmd_queue(agent_cmd _cmd, const json::array_t& _param)
	{
		if (current_poll_node)
		{
			push_cmd_queue(get_tree_idx(current_poll_node->btree_config.tree_name), current_poll_node->_node_idx, _cmd, _param);
		}
		else
		{
			push_cmd_queue(0, 0, _cmd, _param);
		}
	}
	std::vector<node_trace_cmd> agent::dump_cmd_queue()
	{
		auto cur_queue_size = _cmd_queue.size();
		std::vector<node_trace_cmd> result;
		while (!_cmd_queue.empty())
		{
			result.push_back(_cmd_queue.front());
			_cmd_queue.pop_front();
		}
		return result;
	}
	std::uint32_t agent::get_tree_idx(const std::string& tree_name)
	{
		for (std::uint32_t i = 0; i < _tree_descs.size(); i++)
		{
			if (_tree_descs[i]->tree_name == tree_name)
			{
				return i;
			}
		}
		_logger->error("get_tree_idx for tree_name {} invalid", tree_name);
		return 0;
		
	}
	bool agent::set_debug(bool debug_flag)
	{
		if (debug_flag == _debug_on)
		{
			return false;
		}

		_debug_on = debug_flag;
		if (!debug_flag)
		{
			_cmd_queue.clear();
		}
		else
		{
			json::array_t snapshot_param;
			std::vector<std::string> _tree_names;
			for (const auto& one_tree : _tree_descs)
			{
				_tree_names.push_back(one_tree->tree_name);
			}
			snapshot_param.push_back(_tree_names);
			snapshot_param.push_back(_blackboard);
			json::array_t node_fronts;
			for (const auto one_node : _fronts)
			{
				node_fronts.push_back(std::make_pair(get_tree_idx(one_node->tree_name()), one_node->_node_idx));
			}
			snapshot_param.push_back(node_fronts);

			push_cmd_queue(agent_cmd::snapshot, snapshot_param);
		}
		return true;
	}
	std::optional<bool> agent::agent_action(const std::string& action_name, 
		const json::array_t& action_args)
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
		
		cur_root_node = create_tree(btree_name, nullptr);
		if (!cur_root_node)
		{
			return false;
		}
		_fronts.push_back(cur_root_node);
		return true;

	}
	node* agent::create_tree(const std::string& btree_name, node* parent)
	{
		const btree_desc* cur_btree = nullptr;
		for (const auto& one_item : _tree_descs)
		{
			if (one_item->tree_name == btree_name)
			{
				cur_btree = one_item;
				break;
			}
		}
		if (!cur_btree)
		{
			cur_btree = new btree_desc((data_folder / btree_name).string(), _logger);
			if (!cur_btree->is_valid())
			{
				_logger->warn("fail to load btree {}", btree_name);
				return nullptr;
			}
			_tree_descs.push_back(cur_btree);
		}
		
		auto pre_tree_size = _tree_descs.size();

		auto cur_tree_idx = get_tree_idx(cur_btree->tree_name);
		if (_debug_on && pre_tree_size == cur_tree_idx)
		{
			json::array_t params;
			params.push_back(cur_btree->tree_name);
			params.push_back(pre_tree_size);
			push_cmd_queue(agent_cmd::push_tree, params);
		}
		auto temp_root = node::create_node_by_idx(*cur_btree, 0, parent, this);
		return temp_root;
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