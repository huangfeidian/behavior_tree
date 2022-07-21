#include <nodes.h>
#include <magic_enum.hpp>

namespace spiritsaway::behavior_tree::runtime
{
	using namespace spiritsaway::behavior_tree::common;
	std::mt19937 node::m_generator;
	std::uniform_int_distribution<std::uint32_t> node::m_distribution;
	void node::set_result(bool new_result)
	{
		if (node_state_is_forbid_enter())
		{
			m_logger->warn("current state is {} while set result {} at node {}", int(m_state), new_result, node_config.idx);
			m_agent->notify_stop();
			return;
		}
		result = new_result;
		if (m_agent->during_debug())
		{
			m_agent->push_cmd_queue(m_agent->get_tree_idx(btree_config.tree_name), node_config.idx, agent_cmd::node_leave, {});
		}
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
		if (!node_config.children.empty() && children.empty())
		{
			// 初始化所有的子节点
			for (auto one_child_idx : node_config.children)
			{
				auto one_child = node::create_node_by_idx(btree_config, one_child_idx, this, m_agent);
				if (!one_child)
				{
					return;
				}
				children.push_back(one_child);
			}
		}
	}
	void node::on_enter()
	{
		m_state = node_state::entering;
		next_child_idx = 0;
		result = false;
		for (auto one_child : children)
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
		m_state = node_state::leaving;
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
		if (child_idx >= children.size())
		{
			m_logger->warn("btree {} visit child {} at node {} while children size is {}",
				btree_config.tree_name, child_idx, node_config.idx, children.size());
			m_agent->notify_stop();
			return;
		}
		children[child_idx]->m_state = node_state::init;
		m_agent->m_fronts.push_back(children[child_idx]);
		m_state = node_state::wait_child;
	}
	void node::backtrace()
	{
		if (m_parent)
		{
			m_agent->m_fronts.push_back(m_parent);
			m_parent->m_state = node_state::awaken;
		}
		else
		{
			m_agent->m_fronts.push_back(this);
			m_state = node_state::awaken;
		}
		m_agent->poll();
	}
	void node::interrupt()
	{
		if (m_closure)
		{
			m_closure.reset();
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
			bool result = children[0]->result;
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
		if (!children[next_child_idx]->result)
		{
			set_result(false);
			return;
		}
		next_child_idx += 1;
		if (next_child_idx == children.size())
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
		if (next_child_idx == children.size())
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
			m_shuffle.reserve(children.size());
			for (std::size_t i = 0; i < children.size(); i++)
			{
				m_shuffle.push_back(i);
			}
		}
		std::shuffle(m_shuffle.begin(), m_shuffle.end(), m_generator);
		visit_child(m_shuffle[0]);
	}
	void random_seq::on_revisit()
	{
		node::on_revisit();
		if (!children[m_shuffle[next_child_idx]]->result)
		{
			set_result(false);
			return;
		}
		next_child_idx += 1;
		if (next_child_idx == children.size())
		{
			set_result(true);
			return;
		}
		visit_child(m_shuffle[next_child_idx]);
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
		if (children[next_child_idx]->result)
		{
			set_result(true);
			return;
		}
		next_child_idx += 1;
		if (next_child_idx == children.size())
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
		
		if (m_probilities.size() != children.size())
		{
			return false;
		}

		return true;
	}
	std::uint32_t probility::prob_choose_child_idx() const
	{
		std::uint32_t prob_sum = std::accumulate(m_probilities.begin(), m_probilities.end(), 0);
		auto cur_choice = m_distribution(m_generator);
		std::uint32_t temp = cur_choice % prob_sum;
		for (std::size_t i = 0; i < children.size(); i++)
		{
			temp -= m_probilities[0];
			if (temp <= 0)
			{
				return i;
			}
		}
		return 0;

	}
	void probility::on_revisit()
	{
		node::on_revisit();
		set_result(children[next_child_idx]->result);
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
			if (children[0]->result)
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
			set_result(children[1]->result);
			break;
		case 2:
			set_result(children[2]->result);
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
			if (children[0]->result)
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
		set_result(!children[0]->result);
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
		if (children.empty())
		{
			if (!create_sub_tree_node())
			{
				m_logger->warn("{} fail to load sub tree", debug_info());
				return;
			}

		}
		visit_child(0);
	}
	bool sub_tree::create_sub_tree_node()
	{
		auto sub_tree_iter = node_config.extra.find("sub_tree");
		if (sub_tree_iter == node_config.extra.end())
		{
			return false;
		}
		if (!sub_tree_iter->second.is_string())
		{
			return false;
		}
		auto sub_tree_name = sub_tree_iter->second.get<std::string>();
		
		auto new_root = m_agent->create_tree(sub_tree_name, this);
		if (!new_root)
		{
			return false;
		}
		children.push_back(new_root);
		return true;
	}
	void sub_tree::on_revisit()
	{
		set_result(children[0]->result);
	}
	void parallel::on_enter()
	{
		node::on_enter();
		for (std::size_t i = 0; i < children.size(); i++)
		{
			visit_child(i);
		}
	}
	void parallel::on_revisit()
	{
		bool final_result = false;
		for (auto one_child : children)
		{
			if (one_child->m_state == node_state::dead)
			{
				final_result = one_child->result;
			}
			one_child->interrupt();
		}

		set_result(final_result);
	}
	void action::on_enter()
	{
		node::on_enter();
		if (action_name.empty())
		{
			if (!load_action_config())
			{
				m_logger->warn("{} fail to load action args with extra {}", debug_info(), serialize::encode(node_config.extra).dump());
				m_agent->notify_stop();
				return;
			}
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
		if (m_agent->m_during_poll)
		{
			if (!action_result)
			{
				m_state = node_state::blocking;
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
	}
	void reset::on_enter()
	{
		node::on_enter();
		m_state = node_state::init;
		m_agent->reset();

	}
	node* node::create_node_by_idx(const btree_desc& btree_config, std::uint32_t node_idx, node* parent, agent* in_agent)
	{
		if (node_idx >= btree_config.nodes.size())
		{
			if (parent)
			{
				parent->m_logger->warn("{} fail to create sub node idx {} vector access fail",
					parent->debug_info(), node_idx);
				parent->m_agent->notify_stop();
			}
			return nullptr;
		}
		auto& cur_node_desc = btree_config.nodes[node_idx];
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