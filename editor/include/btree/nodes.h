#pragma once
#include <optional>
#include <behavior/nodes.h>
#include <behavior/btree.h>

namespace spiritsaway::behavior_tree::editor
{

	class struct_items;
	class editable_item;
	using node_type = behavior_tree::common::node_type;
	using node_idx_type = behavior_tree::common::node_idx_type;
	using action_arg_type = behavior_tree::common::action_arg_type;
	class node
	{
	public:
		const node_type _type;
		std::string comment;

		node_idx_type _idx;
		node* _parent;
		std::vector<node*> _children;
		bool _is_collapsed = false;
		bool _has_break_point = false;
		std::uint32_t color;//rgba
		std::uint32_t temp_color; // rgba
		std::shared_ptr<struct_items> _show_widget;
		void add_child(node* in_child);
		void remove_child(node* in_child);
		void move_child(node* in_child, bool is_up);
		std::optional<std::uint32_t> index_of_child(const node* in_child) const;
		node(node_type _in_type, node* _in_parent, std::uint32_t _in_idx);
		virtual std::string check_valid() const;
		virtual json to_json() const;
		virtual std::string display_text() const;
		virtual void destroy();
		node* clone_recursive(node* _in_parent) const;
		virtual node* clone_self(node* _in_parent) const = 0;
		static node* load(const std::string& file_path, std::shared_ptr<spdlog::logger> _logger);
		virtual bool check_edit();
		static node* default_node_by_type(node_type cur_type, std::uint32_t idx, node* parent);
		node* get_root() const;
		virtual void refresh_editable_items();
		static node* from_desc(const behavior_tree::common::node_desc& _node_desc, std::shared_ptr<spdlog::logger> _logger);
		static node* from_btree(const behavior_tree::common::btree_desc& _tree_desc, std::shared_ptr<spdlog::logger> _logger);
		bool can_collapse() const;

		virtual bool check_item_edit_refresh(std::shared_ptr<editable_item> change_item);
	};

	class root_node : public node
	{
	public:
		root_node(const std::string& _in_agent_name);
		node* clone_self(node* _in_parent) const;
		std::string agent_name;
		json to_json() const;
	};
	class sequence_node :public  node
	{
	public:
		sequence_node(node* _in_parent, std::uint32_t _in_idx);
		node* clone_self(node* _in_parent) const;

	};
	class always_seq_node : public node
	{
	public:

		always_seq_node(node* _in_parent, std::uint32_t _in_idx);
		node* clone_self(node* _in_parent) const;

	};
	class random_seq_node : public node
	{
	public:
		random_seq_node(node* _in_parent, std::uint32_t _in_idx);
		node* clone_self(node* _in_parent) const;
	};

	class negative_node : public node
	{
	public:
		negative_node(node* _in_parent, std::uint32_t _in_idx);
		node* clone_self(node* _in_parent) const;
	};
	class always_true_node : public node
	{
	public:
		always_true_node(node* _in_parent, std::uint32_t _in_idx);
		node* clone_self(node* _in_parent) const;
	};
	class always_node :public node
	{
	public:
		always_node(node* _in_parent, std::uint32_t _in_idx);
		node* clone_self(node* _in_parent) const;
	};

	class select_node : public node
	{
	public:
		select_node(node* _in_parent, std::uint32_t _in_idx);
		node* clone_self(node* _in_parent) const;
	};

	class probility_node : public node
	{
	public:
		probility_node(node* _in_parent, std::uint32_t _in_idx, const std::vector<std::uint32_t>& weights);
		virtual std::string check_valid() const;
		virtual json to_json() const;
		std::vector<std::uint32_t> weights;
		node* clone_self(node* _in_parent) const;
		virtual bool check_edit();
		void refresh_editable_items();
		bool check_item_edit_refresh(std::shared_ptr<editable_item> change_item);
	};
	class if_else_node : public node
	{
	public:
		if_else_node(node* _in_parent, std::uint32_t _in_idx);
		node* clone_self(node* _in_parent) const;

	};
	class while_node : public node
	{
	public:
		while_node(node* _in_parent, std::uint32_t _in_idx);
		node* clone_self(node* _in_parent) const;
	};

	class parallel_node : public node
	{
	public:
		parallel_node(node* _in_parent, std::uint32_t _in_idx);
		node* clone_self(node* _in_parent) const;
	};

	class reset_node :public node
	{
	public:
		reset_node(node* _in_parent, std::uint32_t _in_idx);
		node* clone_self(node* _in_parent) const;
	};
	class wait_node :public node
	{
	public:
		wait_node(node* _in_parent, std::uint32_t _in_idx, const std::string& _in_event);
		node* clone_self(node* _in_parent) const;
		std::string cur_event;
		json to_json() const;
		virtual bool check_edit();
		void refresh_editable_items();
	};
	class sub_tree_node : public node
	{
	public:
		sub_tree_node(node* _in_parent, std::uint32_t _in_idx, const std::string& tree_name);
		std::string sub_tree_name;
		virtual std::string check_valid() const;
		virtual json to_json() const;
		virtual std::string display_text() const;
		node* clone_self(node* _in_parent) const;
		virtual bool check_edit();
		void refresh_editable_items();
	};

	class action_node :public node
	{
	public:
		action_node(node* _in_parent, std::uint32_t _in_idx,
			const std::string& _in_action_name,
			const std::vector<std::pair<action_arg_type, json>>& _in_action_args);
		std::string action_name;
		std::string agent_name;
		std::vector<std::pair<action_arg_type, json>> action_args;
		virtual std::string check_valid() const;
		virtual json to_json() const;
		virtual std::string display_text() const;
		node* clone_self(node* _in_parent) const;
		virtual bool check_edit();
		std::string get_agent_name() const;
		void refresh_editable_items();
		bool check_item_edit_refresh(std::shared_ptr<editable_item> change_item);
	};

}