
#include <sstream>
#include <fstream>
#include <deque>
#include <iostream>

#include <behavior/nodes.h>

#include <nodes.h>
#include <editable_item.h>
#include <choice_manager.h>
#include <btree_config.h>
#include <any_container/decode.h>

using namespace spiritsaway::behavior_tree::editor;

using namespace spiritsaway::behavior_tree::common;

static std::pair<std::uint32_t, std::uint32_t> node_type_child_limit(node_type in_type)
{
	switch (in_type)
	{
	case node_type::invalid:
		return { 0,0 };
		break;
	case node_type::root:
		return { 1,1 };
		break;
	case node_type::negative:
		return { 1,1 };
		break;
	case node_type::sequence:
		return { 1,20 };
		break;
	case node_type::always_seq:
		return { 1,20 };
		break;
	case node_type::random_seq:
		return { 1,20 };
		break;
	case node_type::if_else:
		return { 3,3 };
		break;
	case node_type::while_loop:
		return { 2,2 };
		break;
	case node_type::wait_event:
		return { 1,1 };
		break;
	case node_type::reset:
		return { 0,0 };
		break;
	case node_type::sub_tree:
		return { 0,0 };
		break;
	case node_type::parallel:
		return { 1,20 };
		break;
	case node_type::action:
		return { 0,0 };
		break;
	case node_type::always_true:
		return { 1,1 };
		break;
	default:
		return { 0, 0 };
		break;
	}
}


std::string node::check_valid() const
{
	const auto& cur_child_limit = node_type_child_limit(_type);
	if (_children.size() > cur_child_limit.second || _children.size() < cur_child_limit.first)
	{
		return fmt::format("node {} has {} children while min {} max {}", _idx, _children.size(), 
			cur_child_limit.first, cur_child_limit.second);
	}
	auto editable_valid = _show_widget->input_valid();
	if (!editable_valid.empty())
	{
		return fmt::format("node {} editable invalid for {}", _idx, editable_valid);
	}
	for (const auto i : _children)
	{
		auto temp_result = i->check_valid();
		if (temp_result.size())
		{
			return temp_result;
		}
	}
	return "";
}
bool node::check_item_edit_refresh(std::shared_ptr<editable_item> change_item)
{
	check_edit();
	return false;
}
void node::add_child(node* in_child)
{
	_children.push_back(in_child);
	in_child->_parent = this;
	auto pre_childrens = in_child->_children;
	std::deque<node*> all_nodes;
	all_nodes.push_back(in_child);
	while (!all_nodes.empty())
	{
		auto cur_node = all_nodes.front();
		all_nodes.pop_front();
		cur_node->refresh_editable_items();
		for (auto one_child : cur_node->_children)
		{
			all_nodes.push_back(one_child);
		}
	}
	
}
bool node::can_collapse() const
{
	const auto& cur_child_limit = node_type_child_limit(_type);
	return cur_child_limit.second > 0;
}

std::optional<std::uint32_t> node::index_of_child(const node* in_child) const
{
	for (std::size_t i = 0; i < _children.size(); i++)
	{
		if (_children[i] == in_child)
		{
			return i;
		}
	}
	return std::nullopt;
}
void node::remove_child(node* in_child)
{
	auto idx_opt = index_of_child(in_child);
	if (!idx_opt)
	{
		return;
	}
	_children.erase(_children.begin() + idx_opt.value());
	in_child->_parent = nullptr;

}
void node::move_child(node* in_child, bool is_up)
{
	auto idx_opt = index_of_child(in_child);
	if (!idx_opt)
	{
		return;
	}
	auto idx = idx_opt.value();
	if (is_up)
	{
		if (idx == 0)
		{
			return;
		}
		std::swap(_children[idx], _children[idx - 1]);
	}
	else
	{
		if (idx == _children.size() - 1)
		{
			return;
		}
		std::swap(_children[idx], _children[idx + 1]);
	}
	

}
node* node::get_root() const
{
	auto p = _parent;
	while (p && p->_type != node_type::root)
	{
		p = p->_parent;
	}
	if (p && p->_type == node_type::root)
	{
		return p;
	}
	else
	{
		return nullptr;
	}
}
node::node(node_type _in_type,  node* _in_parent, std::uint32_t _in_idx):
	_type(_in_type),
	_parent(_in_parent),
	_idx(_in_idx),
	_show_widget(std::make_shared<struct_items>("")),
	color(0xffffffff)

{
	
}

void node::refresh_editable_items()
{

}

json node::to_json() const
{
	json result;
	result["idx"] = _idx;
	result["type"] = magic_enum::enum_name(_type);
	result["comment"] = comment;
	std::vector<std::uint32_t> children_idx;
	children_idx.reserve(_children.size());
	for (const auto i : _children)
	{
		children_idx.push_back(i->_idx);
	}
	result["children"] = children_idx;
	if (_parent)
	{
		result["parent"] = _parent->_idx;
	}
	result["extra"] = json::object_t();
	json::object_t editor;
	editor["color"] = color;
	editor["is_collapsed"] = _is_collapsed;
	result["editor"] = editor;
	return result;
}
std::string node::display_text() const
{
	return fmt::format("{}:{}:{}", magic_enum::enum_name(_type), _idx, comment);
}

void node::destroy()
{
	for (auto one_child : _children)
	{
		one_child->destroy();
	}
	delete this;
}
node* node::clone_recursive(node* _in_parent) const
{
	auto new_node = clone_self(_in_parent);
	for (auto one_child : _children)
	{
		auto temp_child = one_child->clone_recursive(new_node);
		temp_child->comment = one_child->comment;
		temp_child->color = one_child->color;
		temp_child->_is_collapsed = one_child->_is_collapsed;
		new_node->add_child(temp_child);
	}
	return new_node;
}

bool node::check_edit()
{
	if (!_show_widget)
	{
		return false;
	}
	return true;
}
root_node::root_node(const std::string& _in_agent_name)
	:node(node_type::root, nullptr, 0)
	, agent_name(_in_agent_name)
{
	color = 0xff0000ff;
}


node* root_node::clone_self(node* _in_parent) const
{
	return nullptr;
}
json root_node::to_json() const
{
	auto result = node::to_json();
	result["extra"]["agent_name"] = agent_name;
	return result;
}
sequence_node::sequence_node(node* _in_parent, std::uint32_t _in_idx)
	: node(node_type::sequence, _in_parent, _in_idx)
{

}

node* sequence_node::clone_self(node* _in_parent) const
{
	auto new_node = new sequence_node(_in_parent, 0);
	return new_node;
}
always_seq_node::always_seq_node(node* _in_parent, std::uint32_t _in_idx)
	: node(node_type::always_seq, _in_parent, _in_idx)
{

}

node* always_seq_node::clone_self(node* _in_parent) const
{
	auto new_node = new always_seq_node(_in_parent, 0);
	
	return new_node;
}

random_seq_node::random_seq_node(node* _in_parent, std::uint32_t _in_idx)
	:node(node_type::random_seq, _in_parent, _in_idx)
{

}

node* random_seq_node::clone_self(node* _in_parent) const
{
	auto new_node = new random_seq_node(_in_parent, 0);
	return new_node;
}

negative_node::negative_node(node* _in_parent, std::uint32_t _in_idx)
	:node(node_type::negative, _in_parent, _in_idx)
{

}
node* negative_node::clone_self(node* _in_parent) const
{
	auto new_node = new negative_node(_in_parent, 0);
	return new_node;
}
always_true_node::always_true_node(node* _in_parent, std::uint32_t _in_idx)
	:node(node_type::always_true, _in_parent, _in_idx)
{

}
node* always_true_node::clone_self(node* _in_parent) const
{
	auto new_node = new always_true_node(_in_parent, 0);
	return new_node;
}

always_node::always_node(node* _in_parent, std::uint32_t _in_idx)
	:node(node_type::always_true, _in_parent, _in_idx)
{

}

node* always_node::clone_self(node* _in_parent) const
{
	auto new_node = new always_node(_in_parent, 0);
	return new_node;
}

select_node::select_node(node* _in_parent, std::uint32_t _in_idx)
	:node(node_type::select, _in_parent, _in_idx)
{

}

node* select_node::clone_self(node* _in_parent) const
{
	auto new_node = new select_node(_in_parent, 0);
	return new_node;
}

probility_node::probility_node(node* _in_parent, std::uint32_t _in_idx, 
	const std::vector<std::uint32_t>& _in_weights)
	:node(node_type::probility, _in_parent, _in_idx)
	, weights(_in_weights)
{
	
}
void probility_node::refresh_editable_items()
{
	node::refresh_editable_items();
	json item_base = json::object_t();
	item_base["type"] = magic_enum::enum_name(editable_item_type::_int);
	item_base["name"] = "item";
	item_base["value"] = 1;
	json list_item = json::object_t();
	list_item["type"] = magic_enum::enum_name(editable_item_type::_list);
	list_item["name"] = "weight";
	list_item["value"] = weights;
	list_item["item_base"] = item_base;

	_show_widget->push(list_item);
}
node* probility_node::clone_self(node* _in_parent) const
{
	auto new_node = new probility_node(_in_parent, 0, weights);
	return new_node;
}

bool probility_node::check_edit()
{
	if (!node::check_edit())
	{
		return false;
	}
	auto weight_info = _show_widget->find("weight");
	if (!weight_info)
	{
		return false;
	}
	auto cur_value = weight_info->to_json()["value"];
	std::vector<std::uint32_t> temp_weights;
	for (const auto & one_value : cur_value)
	{
		if (!one_value.is_number_unsigned())
		{
			return false;
		}
		else
		{
			temp_weights.push_back(one_value.get<std::uint32_t>());
		}
	}
	weights = temp_weights;
	return true;

}
std::string probility_node::check_valid() const
{
	if (weights.size() != _children.size() || weights.size() == 0)
	{
		return fmt::format("probility node {} has {} children while weight size {}", _idx, _children.size(),
			weights.size());
	}
	return node::check_valid();
}
json probility_node::to_json() const
{
	auto result = node::to_json();
	json::object_t extra;
	extra["prob"] = weights;
	result["extra"] = extra;
	return result;
}

if_else_node::if_else_node(node* _in_parent, std::uint32_t _in_idx)
	: node(node_type::if_else, _in_parent, _in_idx)
{

}

node* if_else_node::clone_self(node* _in_parent) const
{
	auto new_node = new if_else_node(_in_parent, 0);
	return new_node;
}

while_node::while_node(node* _in_parent, std::uint32_t _in_idx)
	:node(node_type::while_loop, _in_parent, _in_idx)
{
	color = 0xff00ff;
}

node* while_node::clone_self(node* _in_parent) const
{
	auto new_node = new while_node(_in_parent, 0);
	return new_node;
}

parallel_node::parallel_node(node* _in_parent, std::uint32_t _in_idx)
	:node(node_type::parallel, _in_parent, _in_idx)
{

}

node* parallel_node::clone_self(node* _in_parent) const
{
	auto new_node = new parallel_node(_in_parent, 0);
	return new_node;
}

reset_node::reset_node(node* _in_parent, std::uint32_t _in_idx)
	:node(node_type::reset, _in_parent, _in_idx)
{

}

node* reset_node::clone_self(node* _in_parent) const
{
	auto new_node = new reset_node(_in_parent, 0);
	return new_node;
}
wait_node::wait_node(node* _in_parent, std::uint32_t _in_idx, const std::string& _in_event)
	: node(node_type::wait_event, _in_parent, _in_idx)
	, cur_event(_in_event)
{
	
}
bool wait_node::check_edit()
{
	if (!node::check_edit())
	{
		return false;
	}
	auto event_widget = _show_widget->find("event");
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
	cur_event = temp_str;
	return true;
}

node* wait_node::clone_self(node* _in_parent) const
{
	auto new_node = new wait_node(_in_parent, 0, cur_event);
	return new_node;
}
void wait_node::refresh_editable_items()
{
	node::refresh_editable_items();
	json temp_item = json::object_t();
	temp_item["name"] = "event";
	temp_item["type"] = magic_enum::enum_name(editable_item_type::single_line_text);
	temp_item["value"] = cur_event;
	_show_widget->push(temp_item);

}
json wait_node::to_json() const
{
	auto result = node::to_json();
	result["extra"] = json::object({ {"event", cur_event} });
	return result;
}

sub_tree_node::sub_tree_node(node* _in_parent, std::uint32_t _in_idx, 
	const std::string& tree_name)
	:node(node_type::sub_tree, _in_parent, _in_idx)
	, sub_tree_name(tree_name)
{
	
}
void sub_tree_node::refresh_editable_items()
{
	node::refresh_editable_items();
	json temp_item = json::object_t();
	temp_item["name"] = "sub_tree";
	temp_item["type"] = magic_enum::enum_name(editable_item_type::single_line_text);
	temp_item["value"] = sub_tree_name;
	_show_widget->push(temp_item);

}

bool sub_tree_node::check_edit()
{
	if (!node::check_edit())
	{
		return false;
	}
	auto event_widget = _show_widget->find("tree_name");
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
node* sub_tree_node::clone_self(node* _in_parent) const
{
	auto new_node = new sub_tree_node(_in_parent, 0, sub_tree_name);
	return new_node;
}

std::string sub_tree_node::check_valid() const
{
	if (sub_tree_name.empty())
	{
		return fmt::format("sub_tree_node node {} has empty tree_name ", _idx);
	}
	return node::check_valid();
}

json sub_tree_node::to_json() const
{
	auto result = node::to_json();
	result["extra"] = json::object({ {"sub_tree_name", sub_tree_name} });
	return result;
}
std::string sub_tree_node::display_text() const
{
	if (comment.size())
	{
		return fmt::format("{}:{}:{}", magic_enum::enum_name(_type), _idx, comment);
	}
	else
	{
		return fmt::format("{}:{}:{}", magic_enum::enum_name(_type), _idx, sub_tree_name);
	}
	
}
action_node::action_node(node* _in_parent, std::uint32_t _in_idx, const std::string& _in_action_name,
	const std::vector<std::pair<action_arg_type, json>>& _in_action_args)
	: node(node_type::action, _in_parent, _in_idx)
	, action_name(_in_action_name)
	, action_args(_in_action_args)
	, agent_name(get_agent_name())
{
	color = 0x7f7f7fff;
}
void action_node::refresh_editable_items()
{
	node::refresh_editable_items();
	agent_name = get_agent_name();
	std::cout << "action_node refresh  with agent " << agent_name << " action " << 
		action_name << " args " << json(action_args).dump() << std::endl;
	if (!_parent || agent_name.empty())
	{
		return;
	}
	json::object_t action_base;
	action_base["name"] = "action";
	action_base["type"] = magic_enum::enum_name(editable_item_type::_choice);
	
	action_base["choice_type"] = agent_name;
	action_base["value"] = action_name;
	_show_widget->push(action_base);

	json::object_t arg_list;
	arg_list["name"] = "args";
	arg_list["type"] = magic_enum::enum_name(editable_item_type::_struct);
	arg_list["value"] = json::array_t();
	

	auto all_actions = btree_config::instance().actions_by_agent[agent_name];
	auto cur_action_iter = all_actions.find(action_name);
	if (cur_action_iter == all_actions.end())
	{
		std::cout << fmt::format("fail to construct action node {} invalid action {} for agent {}",
			_idx, action_name, agent_name) << std::endl;
		return;
	}
	json action_return_info = json::object_t();
	action_return_info["type"] = magic_enum::enum_name(editable_item_type::text_browser);
	action_return_info["name"] = "return";
	action_return_info["value"] = cur_action_iter->second.return_info;
	_show_widget->push(action_return_info);

	const auto& cur_arg_names = cur_action_iter->second.args;
	if (action_args.size() > cur_arg_names.size())
	{
		std::cout << fmt::format("fail to construct action node {} arg size {} exceed {}",
			_idx, action_args.size(), cur_arg_names.size()) << std::endl;
		return;
	}
	std::shared_ptr<struct_items> arg_list_widget = std::dynamic_pointer_cast<struct_items>(
		_show_widget->push(arg_list));

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
std::string action_node::get_agent_name() const
{
	auto cur_root = dynamic_cast<root_node*>(get_root());
	if (!cur_root)
	{
		return "";
	}
	return cur_root->agent_name;
}
bool action_node::check_edit()
{
	
	if (!node::check_edit())
	{
		return false;
	}
	auto action_widget = _show_widget->find("action");
	if (!action_widget)
	{
		return false;
	}
	std::cout << "action_node " << _idx << " check edit " << action_widget->_value.dump() << std::endl;
	if (!action_widget->_value.is_string())
	{
		return false;
	}
	auto temp_action_str = action_widget->_value.get<std::string>();
	if (temp_action_str.empty())
	{
		return false;
	}
	if (temp_action_str != action_name)
	{
		auto all_actions = btree_config::instance().actions_by_agent[agent_name];
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
		auto temp_args_widget = _show_widget->find("args");
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
		refresh_editable_items();
		return true;

	}
}


node* action_node::clone_self(node* _in_parent) const
{
	auto new_node = new action_node(_in_parent, 0, action_name, action_args);
	//new_node->agent_name = agent_name;
	return new_node;
}
std::string action_node::check_valid() const
{
	// check action_name in list
	// check action arg size meet action_name arg size require
	// check every action arg can be convert to 
	return node::check_valid();
}
json action_node::to_json() const
{
	auto result = node::to_json();
	json::object_t extra;
	extra["action_name"] = action_name;
	json::array_t temp_args;
	for (const auto& one_arg : action_args)
	{
		json one_arg_json;
		one_arg_json["arg_value"] = one_arg.second;
		if (one_arg.first == action_arg_type::blackboard)
		{
			one_arg_json["arg_type"] = "blackboard";
		}
		else
		{
			one_arg_json["arg_type"] = "plain";
		}
		temp_args.push_back(one_arg_json);
	}
	extra["action_args"] = temp_args;
	result["extra"] = extra;
	return result;
}
std::string action_node::display_text() const
{
	if (comment.size())
	{
		return fmt::format("{}:{}:{}", magic_enum::enum_name(_type), _idx, comment);
	}
	else
	{
		return fmt::format("{}:{}:{}", magic_enum::enum_name(_type), _idx, action_name);
	}
}

bool action_node::check_item_edit_refresh(std::shared_ptr<editable_item> change_item)
{
	auto result = _show_widget->find("action") == change_item;
	return check_edit() && result;
}

node* node::from_desc(const spiritsaway::behavior_tree::common::node_desc& data, std::shared_ptr<spdlog::logger> _logger)
{
	// comment is not add to node 
	// so the call site should add the comment
	switch (data.type)
	{
	case node_type::root:
	{
		auto agent_name_iter = data.extra.find("agent_name");
		if (agent_name_iter == data.extra.end())
		{
			return nullptr;
		}
		if (!agent_name_iter->second.is_str())
		{
			return nullptr;
		}
		std::string cur_agent_name = std::get<std::string>(agent_name_iter->second);
		auto agent_choice = choice_manager::instance().get_choice(cur_agent_name);
		if (!agent_choice.first)
		{
			return nullptr;
		}
		auto temp_node = new root_node(cur_agent_name);
		return temp_node;
	}
	case node_type::sequence:
	{
		auto temp_node = new sequence_node(nullptr, data.idx);
		return temp_node;
	}
	case node_type::always_seq:
	{
		auto temp_node = new always_seq_node(nullptr, data.idx);
		return temp_node;
	}
	case node_type::random_seq:
	{
		auto temp_node = new random_seq_node(nullptr, data.idx);
		return temp_node;
	}
	case node_type::select:
	{
		auto temp_node = new select_node(nullptr, data.idx);
		return temp_node;
	}
	case node_type::probility:
	{
		auto prob_iter = data.extra.find("prob");
		if (prob_iter == data.extra.end())
		{
			return nullptr;
		}
		if (!prob_iter->second.is_vector())
		{
			return nullptr;
		}
		std::vector<std::uint32_t> probs;
		for (const auto& one_prob : std::get<any_vector>(prob_iter->second))
		{
			if (!one_prob.is_int())
			{
				return nullptr;
			}
			probs.push_back(static_cast<node_idx_type>(std::get<any_int_type>(one_prob)));
		}
		auto temp_node = new probility_node(nullptr, data.idx, probs);
		return temp_node;
	}
	case node_type::if_else:
	{
		auto temp_node = new if_else_node(nullptr, data.idx);
		return temp_node;
	}
	case node_type::while_loop:
	{
		auto temp_node = new while_node(nullptr, data.idx);
		return temp_node;
	}
	case node_type::wait_event:
	{

		auto event_iter = data.extra.find("event");
		if (event_iter == data.extra.end())
		{
			return nullptr;
		}
		if (!event_iter->second.is_str())
		{
			return nullptr;
		}
		auto temp_node = new wait_node(nullptr, data.idx,
			std::get<std::string>(event_iter->second));

		return temp_node;
	}
	case node_type::reset:
	{
		auto temp_node = new reset_node(nullptr, data.idx);
		return temp_node;
	}
	case node_type::sub_tree:
	{
		auto tree_iter = data.extra.find("sub_tree");
		if (tree_iter == data.extra.end())
		{
			return nullptr;
		}
		if (!tree_iter->second.is_str())
		{
			return nullptr;
		}
		auto temp_node = new sub_tree_node(nullptr, data.idx,
			std::get<std::string>(tree_iter->second));
		return temp_node;
	}
	case node_type::parallel:
	{
		auto temp_node = new parallel_node(nullptr, data.idx);
		return temp_node;
	}
	case node_type::action:
	{
		auto name_iter = data.extra.find("action_name");
		if (name_iter == data.extra.end())
		{
			return nullptr;
		}
		if (!name_iter->second.is_str())
		{
			return nullptr;
		}
		std::vector<std::pair<action_arg_type, json>> action_args;
		auto args_iter = data.extra.find("action_args");
		if (args_iter == data.extra.end())
		{
			return nullptr;
		}
		if (!args_iter->second.is_vector())
		{
			return nullptr;
		}
		const any_vector& all_args = std::get<any_vector>(args_iter->second);
		for (const auto& one_arg : all_args)
		{
			if (!one_arg.is_str_map())
			{
				return nullptr;
			}
			const any_str_map& cur_arg_map = std::get<any_str_map>(one_arg);
			auto arg_type_iter = cur_arg_map.find("arg_type");
			if (arg_type_iter == cur_arg_map.end())
			{
				return nullptr;
			}
			if (!arg_type_iter->second.is_str())
			{
				return nullptr;
			}
			std::string cur_arg_type_str = std::get<std::string>(arg_type_iter->second);
			auto arg_type_opt = magic_enum::enum_cast<action_arg_type>(cur_arg_type_str);
			if (!arg_type_opt.has_value())
			{
				return nullptr;
			}
			auto arg_value_iter = cur_arg_map.find("arg_value");
			if (arg_value_iter == cur_arg_map.end())
			{
				return nullptr;
			}
			
			action_args.emplace_back(arg_type_opt.value(), spiritsaway::serialize::encode(arg_value_iter->second));
		}
		auto temp_node = new action_node(nullptr, data.idx, std::get<std::string>(name_iter->second),
			action_args);
		return temp_node;
	}
	case node_type::always_true:
	{
		auto temp_node = new always_node(nullptr, data.idx);
		return temp_node;
	}
	default:
	{
		return nullptr;
	}

	}

}
node* node::load(const std::string& btree_path, std::shared_ptr<spdlog::logger> _logger)
{
	auto cur_btree_desc = btree_desc(btree_path, _logger);
	if (!cur_btree_desc.is_valid())
	{
		_logger->error("fail to load btree {} content invalid", btree_path);
		return nullptr;
	}
	return node::from_btree(cur_btree_desc, _logger);
}
node* node::from_btree(const btree_desc& _btree_desc, std::shared_ptr<spdlog::logger> _logger)
{
	std::vector<node*> all_nodes;
	for (const auto& one_node_desc : _btree_desc.nodes)
	{
		auto new_node = node::from_desc(one_node_desc, _logger);
		if (!new_node)
		{
			_logger->error("fail to convert node_desc {} to node ", one_node_desc.idx);
			return nullptr;
		}
		new_node->comment = one_node_desc.comment;
		auto color_iter = one_node_desc.editor.find("color");
		if(color_iter == one_node_desc.editor.end() || !color_iter->second.is_int())
		{
			new_node->color = 0;
		}
		else
		{
			new_node->color = static_cast<std::uint32_t>(std::get<any_int_type>(color_iter->second));
		}

		auto collapsed_iter = one_node_desc.editor.find("is_collapsed");
		if (collapsed_iter == one_node_desc.editor.end() || !collapsed_iter->second.is_bool())
		{
			new_node->_is_collapsed = false;
		}
		else
		{
			new_node->_is_collapsed = std::get<bool>(collapsed_iter->second);
		}
		all_nodes.push_back(new_node);
	}
	if (all_nodes.empty())
	{
		return false;
	}
	std::deque<std::uint32_t> task_queue;
	task_queue.push_back(0);
	while (!task_queue.empty())
	{
		auto cur_idx = task_queue.front();
		task_queue.pop_front();
		if (cur_idx >= all_nodes.size())
		{
			_logger->error("fail to construct node queue, cur_idx {} exceed all_nodes_size {}", cur_idx, all_nodes.size());
			return nullptr;
		}
		for (const auto one_child_idx : _btree_desc.nodes[cur_idx].children)
		{
			if (one_child_idx >= all_nodes.size())
			{
				return nullptr;
			}
			all_nodes[cur_idx]->add_child(all_nodes[one_child_idx]);
			task_queue.push_back(one_child_idx);
		}

	}
	return all_nodes[0];
}
node* node::default_node_by_type(node_type cur_type, std::uint32_t idx, node* parent)
{
	switch (cur_type)
	{
	case node_type::always_seq:
		return new always_seq_node(parent, idx);
	case node_type::always_true:
		return new always_true_node(parent, idx);
	case node_type::if_else:
		return new if_else_node(parent, idx);
	case node_type::negative:
		return new negative_node(parent, idx);
	case node_type::parallel:
		return new parallel_node(parent, idx);
	case node_type::probility:
		return new probility_node(parent, idx, {});
	case node_type::random_seq:
		return new random_seq_node(parent, idx);
	case node_type::reset:
		return new reset_node(parent, idx);
	case node_type::select:
		return new select_node(parent, idx);
	case node_type::sequence:
		return new sequence_node(parent, idx);
	case node_type::sub_tree:
		return new sub_tree_node(parent, idx, {});
	case node_type::wait_event:
		return new wait_node(parent, idx, {});
	case node_type::while_loop:
		return new while_node(parent, idx);
	case node_type::action:
		return new action_node(parent, idx, "", {});
	default:
		return nullptr;
	}
}
