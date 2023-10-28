#include "action_agent.h"
#include <any_container/decode.h>
#include "timer_manager.hpp"

using namespace nlohmann;

namespace spiritsaway::behavior_tree::runtime
{
	using namespace spiritsaway::behavior_tree::common;
	
	action_agent::action_agent(const std::filesystem::path& in_data_folder, std::shared_ptr<spdlog::logger> in_logger)
		: agent(in_data_folder, in_logger)
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
		add_action("nop", this, &action_agent::nop);
		add_action("get_array_size", this, &action_agent::get_array_size);
		add_action("choose_random", this, &action_agent::choose_random);
		add_action("choose_one_random", this, &action_agent::choose_one_random);
		add_action("choose_idx", this, &action_agent::choose_idx);
		add_async_action("wait_for_seconds", this, &action_agent::wait_for_seconds);
		add_node_closure_creator< timeout_closure>();
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
		duration = std::max(0.5, duration);

		auto cur_timer_handler = create_timer(int(duration*1000));

		auto cur_timeout_closure = std::make_shared<timeout_closure>(current_poll_node, json(cur_timer_handler));
		current_poll_node->m_closure = cur_timeout_closure;
		add_timer(cur_timer_handler, current_poll_node);
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

	json action_agent::encode() const
	{
		json result;
		if (!cur_root_node)
		{
			return result;
		}
		result["tree_name"] = cur_root_node->tree_name();
		result["blackboards"] = m_blackboard;
		std::vector<std::vector<std::pair<std::uint8_t, json>>> front_nodes_chain;
		front_nodes_chain.reserve(m_fronts.size());
		for (auto one_node : m_fronts)
		{
			front_nodes_chain.push_back({});
			auto& cur_chain_vec = front_nodes_chain.back();
			while (one_node)
			{
				cur_chain_vec.push_back(std::make_pair(one_node->next_child_idx, one_node->encode()));
				one_node = one_node->m_parent;
			}
			std::reverse(cur_chain_vec.begin(), cur_chain_vec.end());
		}
		result["front_nodes"] = front_nodes_chain;
		result["next_timer_seq"] = m_next_timer_seq;
		result["enabled"] = m_enabled;
		return result;
	}

	bool action_agent::decode(const json& data)
	{

		if (data.is_null())
		{
			return true;
		}
		m_active_timers.clear();
		std::string tree_name;
		std::vector<std::vector<std::pair<std::uint8_t,json>>> front_nodes_chain;
		try
		{
			data.at("blackboards").get_to(m_blackboard);
			data.at("tree_name").get_to(tree_name);
			data.at("front_nodes").get_to(front_nodes_chain);
			data.at("next_timer_seq").get_to(m_next_timer_seq);
		}
		catch (std::exception& e)
		{
			m_logger->error("fail to action_agent::decode with exception {}", e.what());
			return false;
		}
		load_btree(tree_name);
		m_fronts.clear();
		
		for (int i = 0; i < front_nodes_chain.size(); i++)
		{
			const auto& cur_chain = front_nodes_chain[i];
			auto cur_node = cur_root_node;
			for (int j = 0; j < cur_chain.size(); j++)
			{
				cur_node->create_children();
				cur_node->decode(cur_chain[j].second);
				cur_node->next_child_idx = cur_chain[j].first;
				cur_node->m_state = node_state::wait_child;
				if (cur_node->m_type == node_type::random_seq)
				{
					cur_node = cur_node->m_children[dynamic_cast<random_seq*>(cur_node)->shuffle_children()[cur_chain[j].first]];
					
				}
				else if(cur_node->m_type == node_type::action)
				{
					dynamic_cast<action*>(cur_node)->load_action_config();
				}
				else
				{
					if (!cur_node->m_children.empty())
					{
						cur_node = cur_node->m_children[cur_chain[j].first];
					}
				}
				
			}
			cur_node->m_state = node_state::blocking;
			auto closure_name_iter = cur_chain.back().second.find("closure_name");
			if (closure_name_iter != cur_chain.back().second.end())
			{
				std::string closure_name;
				json closure_data;
				try
				{
					closure_name_iter->get_to(closure_name);
					cur_chain.back().second.at("closure_data").get_to(closure_data);
				}
				catch (std::exception& e)
				{
					m_logger->error("action_agent::decode fail to decode closure with err {}", e.what());
					return false;
				}
				auto closure_ctor_iter = m_node_closure_creators.find(closure_name);
				if (closure_ctor_iter == m_node_closure_creators.end())
				{
					return false;
				}
				cur_node->m_closure = closure_ctor_iter->second(dynamic_cast<node*>(cur_node), closure_data);
			}
			m_fronts.push_back(cur_node);
		}
		return true;
		
	}

	timeout_closure::timeout_closure(node* cur_node, const json& data)
		: node_closure(cur_node, timeout_closure::closure_name(), data)
		, m_timer_handler(data.get<std::uint64_t>())
	{
		
		auto cur_agent = dynamic_cast<action_agent*>(m_node->m_agent);
		cur_agent->add_timer(m_timer_handler, cur_node);
	}

	timeout_closure::~timeout_closure()
	{
		auto cur_agent = dynamic_cast<action_agent*>(m_node->m_agent);
		cur_agent->remove_timer(m_timer_handler, false);
	}

	std::uint64_t action_agent::create_timer(std::uint64_t expire_gap_ms)
	{
		auto result_handler = m_next_timer_seq + 1;
		m_next_timer_seq++;
		timer_manager::instance().add_timer_with_gap(std::chrono::milliseconds(expire_gap_ms), [=]()
			{
				invoke_timer(result_handler);
			});
		return result_handler;
	}

	void action_agent::add_timer(std::uint64_t timer_handler, node* cur_node)
	{
		m_active_timers.push_back(std::make_pair(timer_handler, cur_node));
	}

	void action_agent::invoke_timer(std::uint64_t timer_handler)
	{
		remove_timer(timer_handler, true);
	}

	void action_agent::remove_timer(std::uint64_t timer_handler, bool with_invoke)
	{
		for (int i = 0; i < m_active_timers.size(); i++)
		{
			if (m_active_timers[i].first == timer_handler)
			{
				auto cur_node = m_active_timers[i].second;
				m_active_timers[i].second = nullptr;
				if (i + 1 != m_active_timers.size())
				{
					std::swap(m_active_timers.back(), m_active_timers[i]);
				}
				m_active_timers.pop_back();
				if (with_invoke && cur_node)
				{
					cur_node->set_result(true);
				}
				do_remove_timer(timer_handler);
				return;
			}
		}
	}

	void action_agent::do_remove_timer(std::uint64_t handler)
	{
		timer_manager::instance().cancel_timer(handler);
	}

	bool action_agent::choose_random(const std::string& array_bb_key, const std::string& dest_bb_key, std::uint32_t choose_sz, bool remove_chosen)
	{
		auto from_val = blackboard_get(array_bb_key);
		if (!from_val.is_array())
		{
			return false;
		}
		if (from_val.size() < choose_sz)
		{
			return false;
		}
		std::shuffle(from_val.begin(), from_val.end(), m_random_generator);
		json result(from_val.begin(), from_val.begin() + choose_sz);
		blackboard_set(dest_bb_key, std::move(result));
		if (remove_chosen)
		{
			blackboard_set(array_bb_key, json(from_val.begin() + choose_sz, from_val.end()));
		}
		return true;
	}
	bool action_agent::choose_one_random(const std::string& array_bb_key, const std::string& dest_bb_key, bool remove_chosen)
	{
		auto from_val = blackboard_get(array_bb_key);
		if (!from_val.is_array())
		{
			return false;
		}
		if (from_val.size() < 1)
		{
			return false;
		}
		std::shuffle(from_val.begin(), from_val.end(), m_random_generator);

		blackboard_set(dest_bb_key, from_val[0]);
		if (remove_chosen)
		{
			blackboard_set(array_bb_key, json(from_val.begin() + 1, from_val.end()));
		}
		return true;
	}

	bool action_agent::choose_idx(std::uint32_t idx, const std::string& array_bb_key, const std::string& dest_bb_key)
	{
		const auto& pre_val = blackboard_get(array_bb_key);
		if (!pre_val.is_array() || pre_val.size() < idx)
		{
			return false;
		}
		blackboard_set(dest_bb_key, pre_val[idx]);
		return true;
	}

	bool action_agent::get_array_size(const std::string& array_bb_key, const std::string& sz_bb_key)
	{
		auto cur_val = blackboard_get(array_bb_key);
		if (!cur_val.is_array())
		{
			return false;
		}
		blackboard_set(sz_bb_key, cur_val.size());
		return true;
	}

	bool action_agent::nop()
	{
		return true;
	}
}