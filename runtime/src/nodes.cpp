#include <nodes.h>
#include <magic_enum.hpp>

namespace spiritsaway::behavior_tree::runtime
{
	using namespace spiritsaway::behavior_tree::common;
	void node::set_result(bool new_result)
	{
		if (node_state_is_forbid_enter())
		{
			m_logger->warn("current state is {} while set result {} at node {}", int(m_state), new_result, node_config.idx);
			m_agent->notify_stop();
			return;
		}
		result = new_result;
		m_state = node_state::dead;
		backtrace();
	}
	void node::visit()
	{

		switch (m_state)
		{
		case node_state::init:
			create_children();
			on_enter();
			if (m_state == node_state::entering)
			{
				m_logger->warn("btree {} on_enter node {} while after state {}",
					btree_config.tree_name, node_config.idx, int(m_state));
				m_agent->notify_stop();
			}
			break;
		case node_state::awaken:
			on_revisit();
			if (m_state == node_state::revisiting)
			{
				m_logger->warn("btree {} revisit node {} while after state {}",
					btree_config.tree_name, node_config.idx, int(m_state));
				m_agent->notify_stop();
			}
			break;

		default:
			m_logger->warn("btree {} visit node {} with invalid state {}",
				btree_config.tree_name, node_config.idx, int(m_state));
			m_agent->notify_stop();
			break;
		}
	}
	void node::create_children()
	{
		if (!node_config.children.empty() && m_children.empty())
		{
			// 初始化所有的子节点
			for (auto one_child_idx : node_config.children)
			{
				auto one_child = node::create_node_by_idx(btree_config, one_child_idx, this, m_agent);
				if (!one_child)
				{
					return;
				}
				m_children.push_back(one_child);
			}
		}
		if (m_type == node_type::sub_tree && m_children.empty())
		{
			auto sub_tree_iter = node_config.extra.find("sub_tree_name");
			if (sub_tree_iter == node_config.extra.end())
			{
				m_logger->error("fail to find sub_tree_name in node {} tree {}", m_node_idx, btree_config.tree_name);
				return;
			}
			if (!sub_tree_iter->second.is_string())
			{
				m_logger->error("fail to str value for sub_tree in node {} tree {}", m_node_idx, btree_config.tree_name);
				return;
			}
			auto sub_tree_name = sub_tree_iter->second.get<std::string>();

			auto new_root = m_agent->create_tree(sub_tree_name, this);
			if (!new_root)
			{
				m_logger->error("fail to create root for sub_tree in node {} tree {}", m_node_idx, btree_config.tree_name);
				return;
			}
			m_children.push_back(new_root);
		}
	}

	std::uint32_t node::index_in_parent() const
	{
		if (!m_parent)
		{
			return 0;
		}
		for (std::uint32_t i = 0; i < m_parent->m_children.size(); i++)
		{
			if (m_parent->m_children[i] == this)
			{
				return i;
			}
		}
		return m_parent->m_children.size();
	}
	void node::on_enter()
	{
		m_state = node_state::entering;
		next_child_idx = 0;
		result = false;
		for (auto one_child : m_children)
		{
			one_child->m_state = node_state::init;
		}
		if (m_agent->during_debug())
		{
			m_agent->push_cmd_queue(m_agent->get_tree_idx(btree_config.tree_name), node_config.idx, agent_cmd::node_enter, {});
		}
	}
	void node::leave()
	{
		m_closure.reset();
		if (m_agent->during_debug())
		{
			m_agent->push_cmd_queue(m_agent->get_tree_idx(btree_config.tree_name), node_config.idx, agent_cmd::node_leave, {});
		}
	}
	void node::on_revisit()
	{
		m_state = node_state::revisiting;
	}
	void node::visit_child(std::uint32_t child_idx)
	{
		if (child_idx >= m_children.size())
		{
			m_logger->warn("btree {} visit child {} at node {} while m_children size is {}",
				btree_config.tree_name, child_idx, node_config.idx, m_children.size());
			m_agent->notify_stop();
			return;
		}
		m_children[child_idx]->m_state = node_state::init;
		m_agent->add_to_front(m_children[child_idx]);
		m_state = node_state::wait_child;
	}
	void node::backtrace()
	{
		leave();
		if (m_parent)
		{
			m_agent->add_to_front(m_parent);
			m_parent->m_state = node_state::awaken;
		}
		else
		{
			m_agent->add_to_front(this);
			m_state = node_state::awaken;
		}
		m_agent->poll();
	}
	void node::interrupt()
	{
		m_closure.reset();
		if (m_state == node_state::dead)
		{
			return;
		}
		if (next_child_idx < m_children.size())
		{
			m_children[next_child_idx]->interrupt();
		}
		m_state = node_state::dead;
		next_child_idx = 0;
	}
	const std::string& node::tree_name() const
	{
		return btree_config.tree_name;
	}
	std::string node::debug_info() const
	{
		return fmt::format("btree {} node {} child_idx {}",
			btree_config.tree_name, node_config.idx, next_child_idx);
	}

	json node::encode() const
	{
		json result;
		if (m_closure)
		{
			result["closure_name"] = m_closure->m_name;
			result["closure_data"] = m_closure->m_data;
		}
		return result;
	}

	node::~node()
	{
		m_closure.reset();
		for (auto one_child : m_children)
		{
			delete one_child;
		}
		m_children.clear();
	}
	void root::on_enter()
	{
		node::on_enter();
		visit_child(0);
	}
	void root::on_revisit()
	{
		node::on_revisit();
		if (!m_parent)
		{
			on_enter();
		}
		else
		{
			bool result = m_children[0]->result;
			set_result(result);
		}
	}
	void sequence::on_enter()
	{
		node::on_enter();
		next_child_idx = 0;
		visit_child(0);
	}
	void sequence::on_revisit()
	{
		node::on_revisit();
		if (!m_children[next_child_idx]->result)
		{
			set_result(false);
			return;
		}
		next_child_idx += 1;
		if (next_child_idx == m_children.size())
		{
			set_result(true);
			return;
		}
		visit_child(next_child_idx);
	}
	void always_seq::on_revisit()
	{
		node::on_revisit();

		next_child_idx += 1;
		if (next_child_idx == m_children.size())
		{
			set_result(true);
			return;
		}
		visit_child(next_child_idx);
	}
	void random_seq::on_enter()
	{
		node::on_enter();
		if (m_shuffle.empty())
		{
			m_shuffle.reserve(m_children.size());
			for (std::uint32_t i = 0; i < m_children.size(); i++)
			{
				m_shuffle.push_back(i);
			}
		}
		std::shuffle(m_shuffle.begin(), m_shuffle.end(), m_agent->random_generator());
		visit_child(m_shuffle[0]);
	}
	void random_seq::on_revisit()
	{
		node::on_revisit();
		if (!m_children[m_shuffle[next_child_idx]]->result)
		{
			set_result(false);
			return;
		}
		next_child_idx += 1;
		if (next_child_idx == m_children.size())
		{
			set_result(true);
			return;
		}
		visit_child(m_shuffle[next_child_idx]);
	}

	void random_seq::interrupt()
	{
		if (m_state == node_state::dead)
		{
			return;
		}
		if (m_shuffle[next_child_idx] < m_children.size())
		{
			m_children[m_shuffle[next_child_idx]]->interrupt();
		}
		m_state = node_state::dead;
		next_child_idx = 0;
	}

	json random_seq::encode() const
	{
		json result = node::encode();
		result["shuffle"] = m_shuffle;
		return result;
	}

	bool random_seq::decode(const json& data)
	{
		if (!node::decode(data))
		{
			return false;
		}
		try
		{
			data.at("shuffle").get_to(m_shuffle);
			return true;
		}
		catch (std::exception& e)
		{
			m_logger->error("random_seq::decode fail with err {}", e.what());
			return false;
		}
	}
	void select::on_enter()
	{
		node::on_enter();
		next_child_idx = 0;
		visit_child(0);
	}
	void select::on_revisit()
	{
		node::on_revisit();
		if (m_children[next_child_idx]->result)
		{
			set_result(true);
			return;
		}
		next_child_idx += 1;
		if (next_child_idx == m_children.size())
		{
			set_result(false);
			return;
		}
		visit_child(next_child_idx);
	}
	void probility::on_enter()
	{
		node::on_enter();
		if (m_probilities.empty())
		{
			if (!init_prob_parameters())
			{
				m_logger->warn("{} init_prob_parameters fail", debug_info());
				m_probilities.clear();
				m_agent->notify_stop();
				return;
			}
		}
		next_child_idx = prob_choose_child_idx();
		visit_child(next_child_idx);
	}
	bool probility::init_prob_parameters()
	{
		auto prob_iter = node_config.extra.find("prob");
		if (prob_iter == node_config.extra.end())
		{
			return false;
		}
		if (!prob_iter->second.is_array())
		{
			return false;
		}
		for (const auto& one_item : prob_iter->second)
		{
			if (!one_item.is_number_unsigned())
			{
				return false;
			}
			m_probilities.push_back(one_item.get<std::uint32_t>());
		}
		
		if (m_probilities.size() != m_children.size())
		{
			return false;
		}

		return true;
	}
	std::uint32_t probility::prob_choose_child_idx()
	{
		std::uint32_t prob_sum = std::accumulate(m_probilities.begin(), m_probilities.end(), 0) * 100;
		auto cur_choice = m_agent->int_distribution()(m_agent->random_generator());
		std::uint32_t temp = cur_choice % prob_sum;
		for (std::uint32_t i = 0; i < m_children.size(); i++)
		{
			
			if (temp < m_probilities[i] * 100)
			{
				return i;
			}
			temp -= m_probilities[i] * 100;
		}
		return std::uint32_t(m_children.size() - 1);
	}
	void probility::on_revisit()
	{
		node::on_revisit();
		set_result(m_children[next_child_idx]->result);
	}
	void if_else::on_enter()
	{
		node::on_enter();
		visit_child(0);
	}
	void if_else::on_revisit()
	{
		switch (next_child_idx)
		{
		case 0:
			// the if node
			if (m_children[0]->result)
			{
				next_child_idx = 1;
			}
			else
			{
				next_child_idx = 2;
			}
			visit_child(next_child_idx);
			break;
		case 1:
			set_result(m_children[1]->result);
			break;
		case 2:
			set_result(m_children[2]->result);
			break;
		default:
			m_logger->warn("{} invalid state visit if else node ", debug_info());
			break;
		}
	}
	void while_loop::on_enter()
	{
		node::on_enter();
		visit_child(0);
	}
	void while_loop::on_revisit()
	{
		switch (next_child_idx)
		{
		case 0:
			// the if node
			if (m_children[0]->result)
			{
				next_child_idx = 1;
				visit_child(next_child_idx);
			}
			else
			{
				set_result(true);
			}
			break;
		case 1:
			next_child_idx = 0;
			visit_child(next_child_idx);
			break;
		default:
			m_logger->warn("{} invalid state visit while_node node ", debug_info());
			break;
		}
	}
	void negative::on_enter()
	{
		node::on_enter();
		visit_child(0);
	}
	void negative::on_revisit()
	{
		node::on_revisit();
		set_result(!m_children[0]->result);
	}
	void always_true::on_enter()
	{
		node::on_enter();
		visit_child(0);
	}
	void always_true::on_revisit()
	{
		node::on_revisit();
		set_result(true);
	}
	void sub_tree::on_enter()
	{
		node::on_enter();
		visit_child(0);
	}

	void sub_tree::on_revisit()
	{
		set_result(m_children[0]->result);
	}
	void parallel::on_enter()
	{
		node::on_enter();
		for (std::uint32_t i = 0; i < m_children.size(); i++)
		{
			visit_child(i);
		}
	}
	void parallel::on_revisit()
	{
		bool final_result = false;
		for (auto one_child : m_children)
		{
			if (one_child->m_state == node_state::dead)
			{
				final_result = one_child->result;
			}
			one_child->interrupt();
		}

		set_result(final_result);
	}

	void parallel::interrupt()
	{
		if (m_state == node_state::dead)
		{
			return;
		}
		for (auto one_child : m_children)
		{
			one_child->interrupt();
		}
		m_state = node_state::dead;
		next_child_idx = 0;
	}


	void action::on_enter()
	{
		node::on_enter();
		if (!load_action_config())
		{
			m_logger->warn("{} fail to load action args with extra {}", debug_info(), serialize::encode(node_config.extra).dump());
			m_agent->notify_stop();
			return;
		}
		json::array_t real_action_args;
		for (const auto& one_arg : action_args)
		{
			if (one_arg.first == action_arg_type::blackboard)
			{
				auto cur_key = one_arg.second.get<std::string>();
				if (!m_agent->blackboard_has(cur_key))
				{
					m_logger->warn("{} invalid blackboard arg name {}", debug_info(), cur_key);
					m_agent->notify_stop();
					return;
				}
				auto cur_bb_value = m_agent->blackboard_get(cur_key);
				real_action_args.push_back(cur_bb_value);
			}
			else
			{
				real_action_args.push_back(one_arg.second);
			}
		}
		if (m_agent->during_debug())
		{
			m_agent->push_cmd_queue(agent_cmd::node_action, {action_name, real_action_args});
		}
		std::optional<bool> action_result = m_agent->agent_action(action_name, real_action_args);
		if (m_agent->during_poll())
		{
			if (!action_result)
			{
				m_state = node_state::blocking;
				m_agent->add_to_front(this);
				return;
			}
			else
			{
				set_result(action_result.value());
			}
		}
		else
		{
			return;
		}
	}
	bool action::load_action_config()
	{
		if (!action_name.empty())
		{
			return true;
		}
		auto& extra = node_config.extra;
		auto action_iter = extra.find("action_name");
		if (action_iter == extra.end())
		{
			return false;
		}
		if (!action_iter->second.is_string())
		{
			return false;
		}
		action_name = action_iter->second.get<std::string>();
		auto action_args_iter = extra.find("action_args");
		if (action_args_iter == extra.end())
		{
			return false;
		}
		if (!action_args_iter->second.is_array())
		{
			return false;
		}
		for (auto& one_arg : action_args_iter->second)
		{
			if (!one_arg.is_object())
			{
				return false;
			}
			if (one_arg.size() != 2)
			{
				return false;
			}
			auto arg_type_iter = one_arg.find("arg_type");
			if (arg_type_iter == one_arg.end())
			{
				return false;
			}
			if (!arg_type_iter->is_string())
			{
				return false;
			}
			auto cur_arg_type = arg_type_iter->get<std::string>();
			auto arg_value_iter = one_arg.find("arg_value");
			if (arg_value_iter == one_arg.end())
			{
				return false;
			}

			if (cur_arg_type == "blackboard")
			{
				if (!arg_value_iter->is_string())
				{
					return false;
				}
				auto cur_arg_value = arg_value_iter->get<std::string>();
				action_args.emplace_back(action_arg_type::blackboard, cur_arg_value);
			}
			else if (cur_arg_type == "plain")
			{

				action_args.emplace_back(action_arg_type::plain, *arg_value_iter);
			}
			else
			{
				return false;
			}
		}
		return true;

	}
	void wait_event::on_enter()
	{
		node::on_enter();
		if (event.empty())
		{
			auto event_iter = node_config.extra.find("event");
			if (event_iter == node_config.extra.end())
			{
				m_logger->warn("{} fail to load event", debug_info());
				m_agent->notify_stop();
				return;
			}
			if (!event_iter->second.is_string())
			{
				m_logger->warn("{} fail to load event form value {}", debug_info(), 
					event_iter->second.dump());
				m_agent->notify_stop();
				return;
			}
			event = event_iter->second.get<std::string>();
		}
		m_state = node_state::blocking;
		m_agent->add_to_front(this);
	}

	bool wait_event::handle_event(const event_type& cur_event)
	{
		if(cur_event == event)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	void reset::on_enter()
	{
		node::on_enter();
		m_state = node_state::init;
		m_agent->reset();

	}
	node* node::create_node_by_idx(const btree_desc& btree_config, std::uint32_t node_idx, node* parent, agent* in_agent)
	{
		auto& cur_node_desc = btree_config.get_node(node_idx);
		auto opt_type = magic_enum::enum_cast<node_type>(cur_node_desc.type);
		if (!opt_type)
		{
			return nullptr;
		}
		switch (opt_type.value())
		{

		case node_type::root:
		{
			auto temp_node = new root(parent, in_agent, node_idx,
				btree_config, node_type::root);
			return temp_node;
		}
		case node_type::sequence:
		{
			auto temp_node = new sequence(parent, in_agent, node_idx,
				btree_config, node_type::sequence);
			return temp_node;
		}
		case node_type::always_seq:
		{
			auto temp_node = new always_seq(parent, in_agent, node_idx,
				btree_config, node_type::always_seq);
			return temp_node;
		}
		case node_type::random_seq:
		{
			auto temp_node = new random_seq(parent, in_agent, node_idx,
				btree_config, node_type::random_seq);
			return temp_node;
		}
		case node_type::if_else:
		{
			auto temp_node = new if_else(parent, in_agent, node_idx,
				btree_config, node_type::if_else);
			return temp_node;
		}
		case node_type::while_loop:
		{
			auto temp_node = new while_loop(parent, in_agent, node_idx,
				btree_config, node_type::while_loop);
			return temp_node;
		}
		case node_type::negative:
		{
			auto temp_node = new negative(parent, in_agent, node_idx,
				btree_config, node_type::negative);
			return temp_node;
		}
		case node_type::always_true:
		{
			auto temp_node = new always_true(parent, in_agent, node_idx,
				btree_config, node_type::always_true);
			return temp_node;
		}
		case node_type::action:
		{
			auto temp_node = new action(parent, in_agent, node_idx,
				btree_config, node_type::action);
			return temp_node;
		}
		case node_type::wait_event:
		{
			auto temp_node = new wait_event(parent, in_agent, node_idx,
				btree_config, node_type::wait_event);
			return temp_node;
		}
		case node_type::reset:
		{
			auto temp_node = new reset(parent, in_agent, node_idx,
				btree_config, node_type::reset);
			return temp_node;
		}
		case node_type::sub_tree:
		{
			auto temp_node = new sub_tree(parent, in_agent, node_idx,
				btree_config, node_type::sub_tree);
			return temp_node;
		}
		case node_type::select:
		{
			auto temp_node = new select(parent, in_agent, node_idx,
				btree_config, node_type::select);
			return temp_node;
		}
		case node_type::probility:
		{
			auto temp_node = new probility(parent, in_agent, node_idx,
				btree_config, node_type::probility);
			return temp_node;
		}
		case node_type::parallel:
		{
			auto temp_node = new parallel(parent, in_agent, node_idx,
				btree_config, node_type::parallel);
			return temp_node;
		}
		default:
		{
			if (parent)
			{
				parent->m_logger->warn("{} fail to create sub node idx {} with type {}",
					parent->debug_info(), node_idx, cur_node_desc.type);
				parent->m_agent->notify_stop();
			}
			return nullptr;
		}
		}
	}
}