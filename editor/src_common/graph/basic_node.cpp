#include <graph/basic_node.h>
#include <dialogs/editable_item.h>
#include <any_container/decode.h>
#include <sstream>
#include <fstream>
#include <iostream>

using namespace spiritsaway::behavior_tree::editor;


basic_node::basic_node(node_desc_t _in_type, basic_node* _in_parent, node_idx_t _in_idx)
	: _type(_in_type)
	, _parent(_in_parent)
	,_idx(_in_idx)
{

}
std::string basic_node::check_valid() const
{
	auto max_size = max_child_num();
	auto min_size = min_child_num();
	if (_children.size() > max_size || _children.size() < min_size)
	{
		std::string info = "basic_node " + std::to_string(_idx) + " has " + std::to_string(_children.size()) + " children while min " + std::to_string(min_size) + " max " + std::to_string(max_size);
		return info;
	}
	auto editable_valid = _show_widget->input_valid();
	if (!editable_valid.empty())
	{
		return "basic_node " + std::to_string(_idx) + " editable invalid for " + editable_valid;
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
bool basic_node::check_item_edit_refresh(std::shared_ptr<editable_item> change_item)
{
	check_edit();
	return false;
}
void basic_node::add_child(basic_node* in_child)
{
	_children.push_back(in_child);
	in_child->_parent = this;
	auto pre_childrens = in_child->_children;
	std::deque<basic_node*> all_basic_nodes;
	all_basic_nodes.push_back(in_child);
	while (!all_basic_nodes.empty())
	{
		auto cur_basic_node = all_basic_nodes.front();
		all_basic_nodes.pop_front();
		cur_basic_node->refresh_editable_items();
		for (auto one_child : cur_basic_node->_children)
		{
			all_basic_nodes.push_back(one_child);
		}
	}

}
bool basic_node::can_collapse() const
{
	return max_child_num() > 0;

}

std::optional<std::uint32_t> basic_node::index_of_child(const basic_node* in_child) const
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

void basic_node::remove_child(basic_node* in_child)
{
	auto idx_opt = index_of_child(in_child);
	if (!idx_opt)
	{
		return;
	}
	_children.erase(_children.begin() + idx_opt.value());
	in_child->_parent = nullptr;

}
void basic_node::move_child(basic_node* in_child, bool is_up)
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

basic_node* basic_node::get_root() const
{
	auto p = _parent;
	while (p)
	{
		p = p->_parent;
	}
}
void basic_node::refresh_editable_items()
{

}

basic_node::basic_node(node_desc_t _in_type, basic_node* _in_parent, std::uint32_t _in_idx) :
	_type(_in_type),
	_parent(_in_parent),
	_idx(_in_idx),
	_show_widget(std::make_shared<struct_items>("")),
	color(0xffffffff)

{

}
json basic_node::to_json() const
{
	json result;
	result["idx"] = _idx;
	result["type"] = _type;
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
std::string basic_node::display_text() const
{
	return _type + ":" + std::to_string(_idx) + ":" + comment;
}
void basic_node::destroy()
{
	for (auto one_child : _children)
	{
		one_child->destroy();
	}
	delete this;
}

basic_node* basic_node::clone_recursive(basic_node* _in_parent) const
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

bool basic_node::check_edit()
{
	if (!_show_widget)
	{
		return false;
	}
	return true;
}