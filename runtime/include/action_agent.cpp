#include "action_agent.h"
#include <any_container/decode.h>
using namespace nlohmann;

namespace spiritsaway::behavior_tree::runtime
{
	using namespace spiritsaway::behavior_tree::common;
	
	action_agent::action_agent(const std::filesystem::path& _in_data_folder)
		: agent(_in_data_folder)
	{
		add_action("has_key", this, &action_agent::has_key);
		add_action("has_key_value", this, &action_agent::has_key_value);
		add_action("number_add", this, &action_agent::number_add);
		add_action("number_dec", this, &action_agent::number_dec);
		add_action("set_key_value", this, &action_agent::set_key_value);
		add_action("number_multiply", this, &action_agent::number_multiply);
		add_action("number_div", this, &action_agent::number_div);
		add_action("number_larger_than", this, &action_agent::number_larger_than);
		add_action("number_less_than", this, &action_agent::number_less_than);
		add_action("log", this, &action_agent::log);
		add_action("log_bb", this, &action_agent::log_bb);
		add_async_action("wait_for_seconds", this, &action_agent::wait_for_seconds);
	}

	bool action_agent::has_key(const std::string& bb_key)
	{
		return blackboard_has(bb_key);
	}
	bool action_agent::set_key_value(const std::string& bb_key, 
		const json& new_value)
	{
		blackboard_set(bb_key, new_value);
		return true;
	}
	bool action_agent::has_key_value(const std::string& bb_key, 
		const json& value)
	{
		
		return blackboard_get(bb_key) == value;

	}
	bool action_agent::number_add(const std::string& bb_key, 
		const json& value)
	{
		auto pre_value = blackboard_get(bb_key);
		if (pre_value.is_null())
		{
			return false;
		}
		if (!pre_value.is_number() || !value.is_number())
		{
			return false;
		}
		double new_value = pre_value.get<double>() + value.get<double>();
		if (pre_value.is_number_float() || value.is_number_float())
		{
			blackboard_set(bb_key, new_value);
		}
		else
		{
			blackboard_set(bb_key, static_cast<int>(new_value));
		}
		
		return true;
	}
	bool action_agent::number_dec(const std::string& bb_key, 
		const json& value)
	{
		auto pre_value = blackboard_get(bb_key);
		if (pre_value.is_null())
		{
			return false;
		}
		if (!pre_value.is_number() || !value.is_number())
		{
			return false;
		}
		double new_value = pre_value.get<double>() - value.get<double>();
		if (pre_value.is_number_float() || value.is_number_float())
		{
			blackboard_set(bb_key, new_value);
		}
		else
		{
			blackboard_set(bb_key, static_cast<int>(new_value));
		}

		return true;
	}
	bool action_agent::number_multiply(const std::string& bb_key, 
		const json& value)
	{
		auto pre_value = blackboard_get(bb_key);
		if (pre_value.is_null())
		{
			return false;
		}
		if (!pre_value.is_number() || !value.is_number())
		{
			return false;
		}
		double new_value = pre_value.get<double>() * value.get<double>();
		if (pre_value.is_number_float() || value.is_number_float())
		{
			blackboard_set(bb_key, new_value);
		}
		else
		{
			blackboard_set(bb_key, static_cast<int>(new_value));
		}

		return true;
	}
	bool action_agent::number_div(const std::string& bb_key, 
		const json& value)
	{
		auto pre_value = blackboard_get(bb_key);
		if (pre_value.is_null())
		{
			return false;
		}
		if (!pre_value.is_number() || !value.is_number())
		{
			return false;
		}
		double new_value = pre_value.get<double>() + value.get<double>();
		blackboard_set(bb_key, new_value);

		return true;
	}



	bool action_agent::number_larger_than(const std::string& bb_key, 
		const json& other_value)
	{

		auto pre_value = blackboard_get(bb_key);
		if (!pre_value.is_number() || !other_value.is_number())
		{
			return false;
		}
		return pre_value.get<double>() > other_value.get<double>();

	}
	bool action_agent::number_less_than(const std::string& bb_key, 
		const json& other_value)
	{
		auto pre_value = blackboard_get(bb_key);
		if (!pre_value.is_number() || !other_value.is_number())
		{
			return false;
		}
		return pre_value.get<double>() < other_value.get<double>();
	}
	std::optional<bool> action_agent::wait_for_seconds(double duration)
	{
		auto cur_timeout_closure = std::make_shared<timeout_closure>(current_poll_node);
		std::weak_ptr<timeout_closure> weak_closure = cur_timeout_closure;
		current_poll_node->m_closure = std::move(cur_timeout_closure);
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
			std::chrono::microseconds(static_cast<int>(duration * 1000 * 1000)), timeout_lambda);
		return std::nullopt;
	}
	bool action_agent::log(const std::string& log_level, const json& log_info)
	{
		if (log_level == "debug")
		{
			m_logger->debug("agent {} log {}", reinterpret_cast<std::size_t>(this), log_info.dump());
		}
		else if (log_level == "info")
		{
			m_logger->info("agent {} log {}", reinterpret_cast<std::size_t>(this), log_info.dump());
		}
		else if (log_level == "warn")
		{
			m_logger->warn("agent {} log {}", reinterpret_cast<std::size_t>(this), log_info.dump());
		}
		else if (log_level == "error")
		{
			m_logger->error("agent {} log {}", reinterpret_cast<std::size_t>(this), log_info.dump());
		}
		return true;
	}
	bool action_agent::log_bb(const std::string& log_level, const std::string& bb_key)
	{
		auto log_info = blackboard_get(bb_key);
		if (log_level == "debug")
		{
			m_logger->debug("agent {} log {}", reinterpret_cast<std::size_t>(this), log_info.dump());
		}
		else if (log_level == "info")
		{
			m_logger->info("agent {} log {}", reinterpret_cast<std::size_t>(this), log_info.dump());
		}
		else if (log_level == "warn")
		{
			m_logger->warn("agent {} log {}", reinterpret_cast<std::size_t>(this), log_info.dump());
		}
		else if (log_level == "error")
		{
			m_logger->error("agent {} log {}", reinterpret_cast<std::size_t>(this), log_info.dump());
		}
		
		return true;
	}
	std::optional<bool> action_agent::agent_action(const std::string& action_name, 
		const json::array_t& action_args)
	{
		auto action_iter = m_action_funcs_map.find(action_name);
		if (action_iter == m_action_funcs_map.end())
		{
			m_logger->warn("cant find action {}", action_name);
			notify_stop();
			return std::nullopt;
		}
		return action_iter->second.operator()(action_args);
	}
}