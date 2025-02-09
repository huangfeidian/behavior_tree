#pragma once
#include <optional>
#include "node_enums.h"
#include <tree_editor/common/graph/basic_node.h>
#include "btree_actions.h"
#include <tree_editor/common/dialogs/editable_item.h>
#include <tree_editor/common/choice_manager.h>
#include <magic_enum/magic_enum.hpp>
#include <tree_editor/common/logger.h>

namespace spiritsaway::behavior_tree::editor
{


	using namespace spiritsaway;
	using namespace spiritsaway::tree_editor;
	using btree_node_type = behavior_tree::common::node_type;
	using action_arg_type = behavior_tree::common::action_arg_type;
	class btree_node: public config_node
	{
	public:
		btree_node_type _btree_node_type;
		btree_node(btree_node_type _node_type, btree_node* _in_parent, std::uint32_t _in_idx)
			:config_node(node_config_repo::instance().get_config(std::string(magic_enum::enum_name(_node_type))).value(), _in_parent, _in_idx)
			, _btree_node_type(_node_type)
		{

		}
		btree_node(const tree_editor::node_config& _config, btree_node* _in_parent, std::uint32_t _in_idx)
			: config_node(_config, _in_parent, _in_idx)
			, _btree_node_type(magic_enum::enum_cast<btree_node_type>(_config.node_type_name).value())
		{

		}
		basic_node* clone_self(basic_node* _in_parent) const override
		{
			assert(false);
			return nullptr;
		}
		bool set_extra(const json::object_t& data) override
		{
			return true;
		}
		basic_node* create_node(std::string _type, basic_node* _in_parent, std::uint32_t _idx);
		static basic_node* create_node_from_desc(const basic_node_desc& cur_desc, basic_node* parent);
		static std::shared_ptr<spdlog::logger> logger()
		{
			return tree_editor::logger_mgr::instance().create_logger("btree");
		}
	};

	class root_node : public btree_node
	{
	public:
		root_node(const std::string& _in_agent_name)
			: btree_node(btree_node_type::root, nullptr, 0)
			, agent_name(_in_agent_name)
		{

		}
		basic_node* clone_self(basic_node* _in_parent) const override
		{
			return nullptr;
		}
		std::string agent_name;
		json to_json() const
		{
			auto result = config_node::to_json();
			result["extra"]["agent_name"] = agent_name;
			return result;
		}
		bool set_extra(const json::object_t& data)
		{
			auto cur_iter = data.find("agent_name");
			if (cur_iter == data.end() || !cur_iter->second.is_string())
			{
				return false;
			}
			agent_name = cur_iter->second.get < std::string>();
			if (btree_action_repo::instance().get_actions_by_agent(agent_name).size() == 0)
			{
				return false;
			}
			return true;
		}
		virtual bool check_edit()
		{
			if (!basic_node::check_edit())
			{
				return false;
			}
			auto agent_widget = m_show_widget->find("agent_name");
			if (!agent_widget)
			{
				return false;
			}
			auto agent_value = agent_widget->to_json()["value"];
			if (!agent_value.is_string())
			{
				return false;
			}
			auto temp_str = agent_value.get<std::string>();
			if (temp_str.empty())
			{
				return false;
			}
			agent_name = temp_str;
			return true;
		}

		void refresh_editable_items() override
		{
			basic_node::refresh_editable_items();
			json temp_item = json::object_t();
			temp_item["name"] = "agent_name";
			temp_item["type"] = magic_enum::enum_name(editable_item_type::_choice);
			temp_item["choice_type"] = "agent_name";
			temp_item["value"] = agent_name;
			m_show_widget->push(temp_item);
		}
	};
	class sequence_node :public btree_node
	{
	public:
		sequence_node(btree_node* _in_parent, std::uint32_t _in_idx)
			: btree_node(btree_node_type::sequence, _in_parent, _in_idx)
		{

		}
		basic_node* clone_self(basic_node* _in_parent) const override
		{
			auto new_node = new sequence_node(reinterpret_cast<btree_node*>(_in_parent), 0);
			return new_node;
		}

	};
	class always_seq_node : public btree_node
	{
	public:

		always_seq_node(btree_node* _in_parent, std::uint32_t _in_idx)
			: btree_node(btree_node_type::always_seq, _in_parent, _in_idx)
		{

		}
		basic_node* clone_self(basic_node* _in_parent) const override
		{
			auto new_node = new always_seq_node(reinterpret_cast<btree_node*>(_in_parent), 0);

			return new_node;
		}

	};
	class random_seq_node : public btree_node
	{
	public:
		random_seq_node(btree_node* _in_parent, std::uint32_t _in_idx)
			:btree_node(btree_node_type::random_seq, _in_parent, _in_idx)
		{

		}
		basic_node* clone_self(basic_node* _in_parent) const override
		{
			auto new_node = new random_seq_node(reinterpret_cast<btree_node*>(_in_parent), 0);
			return new_node;
		}
	};

	class negative_node : public btree_node
	{
	public:
		negative_node(btree_node* _in_parent, std::uint32_t _in_idx)
			:btree_node(btree_node_type::negative, _in_parent, _in_idx)
		{

		}
		basic_node* clone_self(basic_node* _in_parent) const override
		{
			auto new_node = new negative_node(reinterpret_cast<btree_node*>(_in_parent), 0);
			return new_node;
		}
	};
	class always_true_node : public btree_node
	{
	public:
		always_true_node(btree_node* _in_parent, std::uint32_t _in_idx)
			:btree_node(btree_node_type::always_true, _in_parent, _in_idx)
		{

		}
		basic_node* clone_self(basic_node* _in_parent) const override
		{
			auto new_node = new always_true_node(reinterpret_cast<btree_node*>(_in_parent), 0);
			return new_node;
		}
	};
	class always_node :public btree_node
	{
	public:
		always_node(btree_node* _in_parent, std::uint32_t _in_idx)
			:btree_node(btree_node_type::always_true, _in_parent, _in_idx)
		{

		}
		basic_node* clone_self(basic_node* _in_parent) const override
		{
			auto new_node = new always_node(reinterpret_cast<btree_node*>(_in_parent), 0);
			return new_node;
		}
	};

	class select_node : public btree_node
	{
	public:
		select_node(btree_node* _in_parent, std::uint32_t _in_idx)
			:btree_node(btree_node_type::select, _in_parent, _in_idx)
		{

		}
		basic_node* clone_self(basic_node* _in_parent) const override
		{
			auto new_node = new select_node(reinterpret_cast<btree_node*>(_in_parent), 0);
			return new_node;
		}
	};

	class probility_node : public btree_node
	{
	public:
		probility_node(btree_node* _in_parent, std::uint32_t _in_idx, const std::vector<std::uint32_t>& weights)
			: btree_node(btree_node_type::probility, _in_parent, _in_idx)
			, weights(weights)
		{

		}
		std::string check_valid() const
		{
			if (weights.size() != m_children.size())
			{
				return fmt::format("probility node {} has {} children while weight size {}", m_idx, m_children.size(),
					weights.size());
			}
			return basic_node::check_valid();
		}
		virtual json to_json() const
		{
			auto result = basic_node::to_json();
			json::object_t extra;
			extra["prob"] = weights;
			result["extra"] = extra;
			return result;
		}
		std::vector<std::uint32_t> weights;
		basic_node* clone_self(basic_node* _in_parent) const override
		{
			auto new_node = new probility_node(reinterpret_cast<btree_node*>(_in_parent), 0, weights);
			return new_node;
		}
		bool set_extra(const json::object_t& data)
		{
			auto cur_iter = data.find("prob");
			if (cur_iter == data.end())
			{
				return false;
			}
			weights.clear();
			if (!cur_iter->second.is_array())
			{
				return false;
			}
			for (auto one_value : cur_iter->second)
			{
				if (!one_value.is_number_unsigned())
				{
					return false;
				}
				weights.push_back(one_value.get<std::uint32_t>());
			}

			return true;
		}
		virtual bool check_edit()
		{
			if (!basic_node::check_edit())
			{
				return false;
			}
			auto weight_info = m_show_widget->find("weight");
			if (!weight_info)
			{
				return false;
			}
			auto cur_value = weight_info->to_json()["value"];
			std::vector<std::uint32_t> temp_weights;
			for (const auto & one_value : cur_value)
			{
				if (!one_value.is_number_integer())
				{
					return false;
				}
				else
				{
					auto temp_weight = one_value.get<std::int32_t>();
					if (temp_weight < 0)
					{
						return false;
					}
					temp_weights.push_back(temp_weight);
				}
			}
			weights = temp_weights;
			return true;
		}
		void refresh_editable_items()override
		{
			basic_node::refresh_editable_items();
			json item_base = json::object_t();
			item_base["type"] = magic_enum::enum_name(editable_item_type::_int);
			item_base["name"] = "item";
			item_base["value"] = 1;
			json list_item = json::object_t();
			list_item["type"] = magic_enum::enum_name(editable_item_type::_list);
			list_item["name"] = "weight";
			list_item["value"] = weights;
			list_item["item_base"] = item_base;

			m_show_widget->push(list_item);
		}
		bool check_item_edit_refresh(std::shared_ptr<editable_item> change_item)
		{
			check_edit();
			return true;
		}
	};
	class if_else_node : public btree_node
	{
	public:
		if_else_node(btree_node* _in_parent, std::uint32_t _in_idx)
			: btree_node(btree_node_type::if_else, _in_parent, _in_idx)
		{

		}
		basic_node* clone_self(basic_node* _in_parent) const override
		{
			auto new_node = new if_else_node(reinterpret_cast<btree_node*>(_in_parent), 0);
			return new_node;
		}

	};
	class while_node : public btree_node
	{
	public:
		while_node(btree_node* _in_parent, std::uint32_t _in_idx)
			: btree_node(btree_node_type::while_loop, _in_parent, _in_idx)
		{

		}
		basic_node* clone_self(basic_node* _in_parent) const override
		{
			auto new_node = new while_node(reinterpret_cast<btree_node*>(_in_parent), 0);
			return new_node;
		}
	};

	class parallel_node : public btree_node
	{
	public:
		parallel_node(btree_node* _in_parent, std::uint32_t _in_idx)
			: btree_node(btree_node_type::parallel, _in_parent, _in_idx)
		{

		}
		basic_node* clone_self(basic_node* _in_parent) const override
		{
			auto new_node = new parallel_node(reinterpret_cast<btree_node*>(_in_parent), 0);
			return new_node;
		}
	};

	class reset_node :public btree_node
	{
	public:
		reset_node(btree_node* _in_parent, std::uint32_t _in_idx)
			: btree_node(btree_node_type::reset, _in_parent, _in_idx)
		{

		}
		basic_node* clone_self(basic_node* _in_parent) const override
		{
			auto new_node = new reset_node(reinterpret_cast<btree_node*>(_in_parent), 0);
			return new_node;
		}
	};
	class wait_node :public btree_node
	{
	public:
		wait_node(btree_node* _in_parent, std::uint32_t _in_idx, const std::string& _in_event)
			: btree_node(btree_node_type::wait_event, _in_parent, _in_idx)
			, m_wait_event(_in_event)
		{

		}
		basic_node* clone_self(basic_node* _in_parent) const override
		{
			auto new_node = new wait_node(reinterpret_cast<btree_node*>(_in_parent), 0, m_wait_event);
			return new_node;
		}
		std::string display_text() const override
		{
			if (m_comment.size())
			{
				return fmt::format("{}:{}:{}", m_idx, m_type, m_comment);
			}
			else
			{
				return fmt::format("{}:{}:{}", m_idx, m_type, m_wait_event);
			}
		}
		std::string m_wait_event;
		json to_json() const
		{
			auto result = basic_node::to_json();
			result["extra"] = json::object({ {"event", m_wait_event} });
			return result;
		}
		virtual bool check_edit()
		{
			if (!basic_node::check_edit())
			{
				return false;
			}
			auto event_widget = m_show_widget->find("event");
			if (!event_widget)
			{
				return false;
			}
			auto event_value = event_widget->to_json()["value"];
			if (!event_value.is_string())
			{
				return false;
			}
			auto temp_str = event_value.get<std::string>();
			if (temp_str.empty())
			{
				return false;
			}
			m_wait_event = temp_str;
			return true;
		}
		void refresh_editable_items()override
		{
			basic_node::refresh_editable_items();
			json temp_item = json::object_t();
			temp_item["name"] = "event";
			temp_item["type"] = magic_enum::enum_name(editable_item_type::single_line_text);
			temp_item["value"] = m_wait_event;
			m_show_widget->push(temp_item);
		}
		bool set_extra(const json::object_t& data)
		{
			auto cur_iter = data.find("event");
			if (cur_iter == data.end() || !cur_iter->second.is_string())
			{
				return false;
			}
			m_wait_event = cur_iter->second.get<std::string>();
			return true;
		}

	};
	class sub_tree_node : public btree_node
	{
	public:
		sub_tree_node(btree_node* _in_parent, std::uint32_t _in_idx, const std::string& tree_name)
			:btree_node(btree_node_type::sub_tree, _in_parent, _in_idx)
			, sub_tree_name(tree_name)
		{

		}
		std::string sub_tree_name;
		virtual std::string check_valid() const
		{
			if (sub_tree_name.empty())
			{
				return fmt::format("sub_tree_node node {} has empty tree_name ", m_idx);
			}
			return basic_node::check_valid();
		}
		virtual json to_json() const
		{
			auto result = basic_node::to_json();
			result["extra"] = json::object({ {"sub_tree_name", sub_tree_name} });
			return result;
		}
		virtual std::string display_text() const
		{
			if (m_comment.size())
			{
				return fmt::format("{}:{}:{}", m_idx, m_type, m_comment);
			}
			else
			{
				return fmt::format("{}:{}:{}", m_idx, m_type, sub_tree_name);
			}
		}
		basic_node* clone_self(basic_node* _in_parent) const override
		{
			auto new_node = new sub_tree_node(reinterpret_cast<btree_node*>(_in_parent), 0, sub_tree_name);
			return new_node;
		}
		virtual bool check_edit()
		{
			if (!basic_node::check_edit())
			{
				return false;
			}
			auto event_widget = m_show_widget->find("sub_tree");
			if (!event_widget)
			{
				return false;
			}
			auto event_value = event_widget->to_json()["value"];
			if (!event_value.is_string())
			{
				return false;
			}
			auto temp_str = event_value.get<std::string>();
			if (temp_str.empty())
			{
				return false;
			}
			sub_tree_name = temp_str;
			return true;
		}
		void refresh_editable_items()override
		{
			basic_node::refresh_editable_items();
			json temp_item = json::object_t();
			temp_item["name"] = "sub_tree";
			temp_item["type"] = magic_enum::enum_name(editable_item_type::single_line_text);
			temp_item["value"] = sub_tree_name;
			m_show_widget->push(temp_item);
		}
		bool set_extra(const json::object_t& data)
		{
			auto cur_iter = data.find("sub_tree_name");
			if (cur_iter == data.end() || !cur_iter->second.is_string())
			{
				return false;
			}
			sub_tree_name = cur_iter->second.get<std::string>();
			return true;
		}

	};

	class action_node :public btree_node
	{
	public:
		action_node(btree_node* _in_parent, std::uint32_t _in_idx,
			const std::string& _in_action_name,
			const std::vector<std::pair<action_arg_type, json>>& _in_action_args)
			:  btree_node(btree_node_type::action, _in_parent, _in_idx)
			, action_name(_in_action_name)
			, action_args(_in_action_args)
		{

		}
		std::string action_name;
		std::vector<std::pair<action_arg_type, json>> action_args;
		virtual std::string check_valid() const
		{
			return basic_node::check_valid();
		}
		virtual json to_json() const
		{
			auto result = basic_node::to_json();
			json::object_t extra;
			extra["action_name"] = action_name;
			json::array_t temp_args;
			for (const auto& one_arg : action_args)
			{
				json one_arg_json;
				one_arg_json["arg_value"] = one_arg.second;
				one_arg_json["arg_type"] = magic_enum::enum_name(one_arg.first);
				temp_args.push_back(one_arg_json);
			}
			extra["action_args"] = temp_args;
			result["extra"] = extra;
			return result;
		}
		virtual std::string display_text() const
		{
			if (m_comment.size())
			{
				return fmt::format("{}:{}:{}", m_idx, m_type , m_comment);
			}
			else
			{
				std::string show_text = action_name + "(";
				for (const auto& one_arg : action_args)
				{

					if (one_arg.first == action_arg_type::blackboard)
					{
						show_text += "B(" + one_arg.second.dump() + "),";
					}
					else
					{
						show_text += one_arg.second.dump() + ",";
					}
				}
				if (show_text.back() == ',')
				{
					show_text.back() = ')';
				}
				else
				{
					show_text += ")";
				}

				return fmt::format("{}:{}:{}", m_type, m_idx, show_text);
			}
		}
		basic_node* clone_self(basic_node* _in_parent) const override
		{
			auto new_node = new action_node(reinterpret_cast<btree_node*>(_in_parent), 0, action_name, action_args);
			//new_node->agent_name = agent_name;
			return new_node;
		}
		virtual bool check_edit()
		{
			if (!basic_node::check_edit())
			{
				return false;
			}
			std::string agent_name = get_agent_name();
			auto action_widget = m_show_widget->find("action");
			if (!action_widget)
			{
				return false;
			}
			logger()->debug("action_node {} check edit {}", m_idx, action_widget->m_value.dump());
			if (!action_widget->m_value.is_string())
			{
				return false;
			}
			auto temp_action_str = action_widget->m_value.get<std::string>();
			if (temp_action_str.empty())
			{
				return false;
			}
			if (temp_action_str != action_name)
			{
				auto all_actions = btree_action_repo::instance().get_actions_by_agent(agent_name);
				auto cur_action_iter = all_actions.find(temp_action_str);
				if (cur_action_iter == all_actions.end())
				{
					return false;
				}
				action_args.clear();
				for (const auto& one_arg : cur_action_iter->second.args)
				{
					action_args.emplace_back(action_arg_type::plain, "");
				}
				action_name = temp_action_str;
				refresh_editable_items();
				return true;
			}
			else
			{
				auto temp_args_widget = m_show_widget->find("args");
				if (!temp_args_widget)
				{
					return false;
				}
				auto args_values = temp_args_widget->to_json();
				std::vector<std::pair<action_arg_type, json>> temp_arg_values;
				// todo value check
				for (const auto& one_arg_item : args_values["value"])
				{
					auto one_arg_value = one_arg_item["value"];
					std::string cur_choice = one_arg_value[0]["value"].get<std::string>();
					json cur_value = one_arg_value[1]["value"];
					if (cur_choice == "plain")
					{
						temp_arg_values.emplace_back(action_arg_type::plain, cur_value);
					}
					else
					{
						temp_arg_values.emplace_back(action_arg_type::blackboard, cur_value);
					}

				}
				action_args.swap(temp_arg_values);
				// refresh_editable_items();
				return true;

			}
		}
		std::string get_agent_name() const
		{
			auto cur_root = dynamic_cast<const root_node*>(get_root());
			if (!cur_root)
			{
				return "";
			}
			return cur_root->agent_name;
		}
		void refresh_editable_items()
		{
			basic_node::refresh_editable_items();
			std::string agent_name = get_agent_name();
			logger()->debug("action_node refresh  with agent {} action {} args {}", agent_name,
				action_name, json(action_args).dump());
			if (!m_parent || agent_name.empty())
			{
				return;
			}
			json::object_t action_base;
			action_base["name"] = "action";
			action_base["type"] = magic_enum::enum_name(editable_item_type::_choice);

			action_base["choice_type"] = agent_name + "_actions";
			action_base["value"] = action_name;
			m_show_widget->push(action_base);

			json::object_t arg_list;
			arg_list["name"] = "args";
			arg_list["type"] = magic_enum::enum_name(editable_item_type::_struct);
			arg_list["value"] = json::array_t();


			auto all_actions = btree_action_repo::instance().get_actions_by_agent(agent_name);
			auto cur_action_iter = all_actions.find(action_name);
			if (cur_action_iter == all_actions.end())
			{
				logger()->error("fail to construct action node {} invalid action {} for agent {}",
					m_idx, action_name, agent_name);
				return;
			}
			json action_return_info = json::object_t();
			action_return_info["type"] = magic_enum::enum_name(editable_item_type::text_browser);
			action_return_info["name"] = "return";
			action_return_info["value"] = cur_action_iter->second.return_info;
			m_show_widget->push(action_return_info);

			const auto& cur_arg_names = cur_action_iter->second.args;
			if (action_args.size() > cur_arg_names.size())
			{
				logger()->error("fail to construct action node {} arg size {} exceed {}",
					m_idx, action_args.size(), cur_arg_names.size()) ;
				return;
			}
			std::shared_ptr<tree_editor::struct_items> arg_list_widget = std::dynamic_pointer_cast<tree_editor::struct_items>(
				m_show_widget->push(arg_list));

			json arg_base = json::object_t();
			arg_base["type"] = magic_enum::enum_name(editable_item_type::_struct);
			arg_base["name"] = "arg";
			arg_base["value"] = "";
			json arg_param_type = json::object_t();
			arg_param_type["name"] = "type";
			arg_param_type["type"] = magic_enum::enum_name(editable_item_type::_choice);
			arg_param_type["choice_type"] = "action_arg_type";
			arg_param_type["value"] = "";
			json arg_param_value = json::object_t();
			arg_param_value["name"] = "value";
			arg_param_value["type"] = magic_enum::enum_name(editable_item_type::_json);
			arg_param_value["value"] = "";
			json::array_t temp_array;
			temp_array.push_back(arg_param_type);
			temp_array.push_back(arg_param_value);
			arg_base["value"] = temp_array;

			for (std::size_t i = 0; i < cur_arg_names.size(); i++)
			{
				arg_base["name"] = cur_arg_names[i].name;
				arg_base["comment"] = cur_arg_names[i].comment;
				const auto& one_arg = action_args[i];
				if (one_arg.first == action_arg_type::plain)
				{
					arg_base["value"][0]["value"] = "plain";
				}
				else
				{
					arg_base["value"][0]["value"] = "blackboard";
				}
				arg_base["value"][1]["value"] = one_arg.second;
				arg_list_widget->push(arg_base);
			}
		}
		bool check_item_edit_refresh(std::shared_ptr<editable_item> change_item)
		{
			auto result = m_show_widget->find("action") == change_item;
			return check_edit() && result;
		}
		bool set_extra(const json::object_t& data)
		{
			auto name_iter = data.find("action_name");
			if (name_iter == data.end())
			{
				return false;
			}
			if (!name_iter->second.is_string())
			{
				return false;
			}
			action_name = name_iter->second.get<std::string>();
			action_args.clear();
			auto args_iter = data.find("action_args");
			if (args_iter == data.end())
			{
				return false;
			}
			if (!args_iter->second.is_array())
			{
				return false;
			}

			for (const auto& one_arg : args_iter->second)
			{
				if (!one_arg.is_object())
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
				std::string cur_arg_type_str = arg_type_iter->get<std::string>();
				auto arg_type_opt = magic_enum::enum_cast<action_arg_type>(cur_arg_type_str);
				if (!arg_type_opt.has_value())
				{
					return false;
				}
				auto arg_value_iter = one_arg.find("arg_value");
				if (arg_value_iter == one_arg.end())
				{
					return false;
				}

				action_args.emplace_back(arg_type_opt.value(), *arg_value_iter);
			}
			return true;
		}

	};
	basic_node* btree_node::create_node(std::string _type, basic_node* _in_parent, std::uint32_t _idx)
	{
		// comment is not add to node 
		// so the call site should add the comment
		auto cur_type_opt = magic_enum::enum_cast<btree_node_type>(_type);
		if (!cur_type_opt)
		{
			return nullptr;;
		}
		switch (cur_type_opt.value())
		{
		case btree_node_type::root:
		{

			auto temp_node = new root_node(agent_relation::instance().get_root_agent());
			return temp_node;
		}
		case btree_node_type::sequence:
		{
			auto temp_node = new sequence_node(dynamic_cast<btree_node*>(_in_parent), _idx);
			return temp_node;
		}
		case btree_node_type::always_seq:
		{
			auto temp_node = new always_seq_node(dynamic_cast<btree_node*>(_in_parent), _idx);
			return temp_node;
		}
		case btree_node_type::random_seq:
		{
			auto temp_node = new random_seq_node(dynamic_cast<btree_node*>(_in_parent), _idx);
			return temp_node;
		}
		case btree_node_type::select:
		{
			auto temp_node = new select_node(dynamic_cast<btree_node*>(_in_parent), _idx);
			return temp_node;
		}
		case btree_node_type::probility:
		{
			std::vector<std::uint32_t> probs;

			auto temp_node = new probility_node(dynamic_cast<btree_node*>(_in_parent), _idx, probs);
			return temp_node;
		}
		case btree_node_type::if_else:
		{
			auto temp_node = new if_else_node(dynamic_cast<btree_node*>(_in_parent), _idx);
			return temp_node;
		}
		case btree_node_type::while_loop:
		{
			auto temp_node = new while_node(dynamic_cast<btree_node*>(_in_parent), _idx);
			return temp_node;
		}
		case btree_node_type::wait_event:
		{
			auto temp_node = new wait_node(dynamic_cast<btree_node*>(_in_parent), _idx,
				"");

			return temp_node;
		}
		case btree_node_type::reset:
		{
			auto temp_node = new reset_node(dynamic_cast<btree_node*>(_in_parent), _idx);
			return temp_node;
		}
		case btree_node_type::sub_tree:
		{
			auto temp_node = new sub_tree_node(dynamic_cast<btree_node*>(_in_parent), _idx, "");
			return temp_node;
		}
		case btree_node_type::parallel:
		{
			auto temp_node = new parallel_node(dynamic_cast<btree_node*>(_in_parent), _idx);
			return temp_node;
		}
		case btree_node_type::action:
		{

			auto temp_node = new action_node(dynamic_cast<btree_node*>(_in_parent), _idx, "nop", {});
			return temp_node;
		}
		case btree_node_type::always_true:
		{
			auto temp_node = new always_node(dynamic_cast<btree_node*>(_in_parent), _idx);
			return temp_node;
		}
		default:
		{
			return nullptr;
		}

		}

	}
	basic_node* btree_node::create_node_from_desc(const basic_node_desc& cur_desc, basic_node* parent)
	{
		auto cur_config = node_config_repo::instance().get_config(cur_desc.type);
		if (!cur_config)
		{
			return nullptr;
		}
		basic_node* cur_node;

		if (!parent)
		{
			auto invalid_root = root_node("invalid");
			cur_node = invalid_root.create_node(cur_desc.type, parent, cur_desc.idx);
		}
		else
		{
			cur_node = parent->create_node(cur_desc.type, parent, cur_desc.idx);

		}
		cur_node->m_color = cur_desc.color;
		cur_node->m_is_collapsed = cur_desc.is_collpased;
		cur_node->m_comment = cur_desc.comment;
		json::object_t extra = json(cur_desc.extra);
		if (extra.empty())
		{
			if (cur_desc.type == "root")
			{
				extra["agent_name"] = agent_relation::instance().get_root_agent();
			}
			else if (cur_desc.type == "action")
			{
				extra["action_name"] = "nop";
				extra["action_args"] = json::array();
			}
		}

		if (!cur_node->set_extra(extra))
		{
			delete cur_node;
			return nullptr;
		}
		if (parent)
		{
			parent->add_child(cur_node);
		}
		cur_node->refresh_editable_items();

		return cur_node;
	}

}