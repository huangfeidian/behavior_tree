#include <action_agent.h>
namespace behavior_tree::runtime
{
	using namespace behavior_tree::common;
	bool action_agent::has_key(const std::string& bb_key)
	{
		auto cur_iter = _blackboard.find(bb_key);
		return cur_iter != _blackboard.end();
	}
	bool action_agent::set_key_value(const std::string& bb_key, 
		const any_value_type& new_value)
	{
		_blackboard[bb_key] = new_value;
		return true;
	}
	bool action_agent::has_key_value(const std::string& bb_key, 
		const any_value_type& value)
	{
		auto cur_iter = _blackboard.find(bb_key);
		if (cur_iter == _blackboard.end())
		{
			return false;
		}

		const auto& cur_value = cur_iter->second;
		return cur_value == value;

	}
	bool action_agent::number_add(const std::string& bb_key, 
		const any_value_type& value)
	{
		auto cur_iter = _blackboard.find(bb_key);
		if (cur_iter == _blackboard.end())
		{
			return false;
		}
		auto& cur_value = cur_iter->second;
		auto result = cur_value.numeric_cal_add(value);
		return !!result;
	}
	bool action_agent::number_dec(const std::string& bb_key, 
		const any_value_type& value)
	{
		auto cur_iter = _blackboard.find(bb_key);
		if (cur_iter == _blackboard.end())
		{
			return false;
		}
		auto& cur_value = cur_iter->second;
		auto result = cur_value.numeric_cal_dec(value);
		return !!result;
	}
	bool action_agent::number_multiply(const std::string& bb_key, 
		const any_value_type& value)
	{
		auto cur_iter = _blackboard.find(bb_key);
		if (cur_iter == _blackboard.end())
		{
			return false;
		}
		auto& cur_value = cur_iter->second;
		auto result = cur_value.numeric_cal_multiply(value);
		return !!result;
	}
	bool action_agent::number_div(const std::string& bb_key, 
		const any_value_type& value)
	{
		auto cur_iter = _blackboard.find(bb_key);
		if (cur_iter == _blackboard.end())
		{
			return false;
		}
		auto& cur_value = cur_iter->second;
		auto result = cur_value.numeric_cal_div(value);
		return !!result;
	}



	bool action_agent::number_larger_than(const std::string& bb_key, 
		const any_value_type& other_value)
	{
		auto cur_iter = _blackboard.find(bb_key);
		if (cur_iter == _blackboard.end())
		{
			return false;
		}
		auto& cur_value = cur_iter->second;
		auto result = cur_value.numeric_larger_than(other_value);
		return result.value_or(false);
	}
	bool action_agent::number_less_than(const std::string& bb_key, 
		const any_value_type& other_value)
	{
		auto cur_iter = _blackboard.find(bb_key);
		if (cur_iter == _blackboard.end())
		{
			return false;
		}
		auto& cur_value = cur_iter->second;
		auto result = cur_value.numeric_less_than(other_value);
		return result.value_or(false);
	}
	std::optional<bool> action_agent::wait_for_seconds(double duration)
	{
		auto cur_timeout_closure = std::make_shared<timeout_closure>(current_poll_node);
		std::weak_ptr<timeout_closure> weak_closure = cur_timeout_closure;
		current_poll_node->_closure = std::move(cur_timeout_closure);
		auto timeout_lambda = [=]()
		{
			auto temp_callback = weak_closure.lock();
			if (temp_callback)
			{
				temp_callback->operator()();
			}
		};
		duration = std::max(0.5, duration);
		auto cur_timer_handler = timer_manager::instance().add_timer_with_gap(
			std::chrono::microseconds(static_cast<int>(duration * 1000)), timeout_lambda);
		_timers.insert(cur_timer_handler);
		return std::nullopt;
	}
	bool action_agent::log(const std::string& log_level, const any_value_type& log_info)
	{
		if (log_level == "debug")
		{
			_logger->debug("agent {} log {}", reinterpret_cast<std::size_t>(this), encode(log_info).dump());
		}
		else if (log_level == "info")
		{
			_logger->info("agent {} log {}", reinterpret_cast<std::size_t>(this), encode(log_info).dump());
		}
		else if (log_level == "warn")
		{
			_logger->warn("agent {} log {}", reinterpret_cast<std::size_t>(this), encode(log_info).dump());
		}
		else if (log_level == "error")
		{
			_logger->error("agent {} log {}", reinterpret_cast<std::size_t>(this), encode(log_info).dump());
		}
		return true;
	}
	std::optional<bool> action_agent::agent_action(const std::string& action_name, 
		const any_vector& action_args)
	{
		auto action_iter = action_funcs_map.find(action_name);
		if (action_iter == action_funcs_map.end())
		{
			_logger->warn("cant find action {}", action_name);
			notify_stop();
			return std::nullopt;
		}
		return (this->*(action_iter->second))(action_args);
	}
}
#include "action_agent.generated_cpp"