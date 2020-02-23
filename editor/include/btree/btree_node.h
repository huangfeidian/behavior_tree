#pragma once
#include <optional>
#include <behavior/nodes.h>
#include <behavior/btree.h>
#include <graph/basic_node.h>

namespace spiritsaway::behavior_tree::editor
{

	class struct_items;
	class editable_item;
	using btree_node_type = behavior_tree::common::node_type;
	using action_arg_type = behavior_tree::common::action_arg_type;
	class btree_node: public basic_node
	{
	public:
		btree_node_type _btree_node_type;
		btree_node(btree_node_type _in_type, btree_node* _in_parent, std::uint32_t _in_idx);
		basic_node* clone_self(basic_node* _in_parent) const;
		static btree_node* load(const std::string& file_path, std::shared_ptr<spdlog::logger> _logger);
		static btree_node* default_node_by_type(btree_node_type cur_type, std::uint32_t idx, btree_node* parent);

		static btree_node* from_desc(const behavior_tree::common::node_desc& _node_desc, std::shared_ptr<spdlog::logger> _logger);
		static btree_node* from_btree(const behavior_tree::common::btree_desc& _tree_desc, std::shared_ptr<spdlog::logger> _logger);
		std::size_t max_child_num() const;
		std::size_t min_child_num() const;
		basic_node* create_node(node_desc_t _type, basic_node* _in_parent, node_idx_t _idx);
	};

	class root_node : public btree_node
	{
	public:
		root_node(const std::string& _in_agent_name);
		basic_node* clone_self(basic_node* _in_parent) const;
		std::string agent_name;
		json to_json() const;
	};
	class sequence_node :public btree_node
	{
	public:
		sequence_node(btree_node* _in_parent, std::uint32_t _in_idx);
		basic_node* clone_self(basic_node* _in_parent) const;

	};
	class always_seq_node : public btree_node
	{
	public:

		always_seq_node(btree_node* _in_parent, std::uint32_t _in_idx);
		basic_node* clone_self(basic_node* _in_parent) const;

	};
	class random_seq_node : public btree_node
	{
	public:
		random_seq_node(btree_node* _in_parent, std::uint32_t _in_idx);
		basic_node* clone_self(basic_node* _in_parent) const;
	};

	class negative_node : public btree_node
	{
	public:
		negative_node(btree_node* _in_parent, std::uint32_t _in_idx);
		basic_node* clone_self(basic_node* _in_parent) const;
	};
	class always_true_node : public btree_node
	{
	public:
		always_true_node(btree_node* _in_parent, std::uint32_t _in_idx);
		basic_node* clone_self(basic_node* _in_parent) const;
	};
	class always_node :public btree_node
	{
	public:
		always_node(btree_node* _in_parent, std::uint32_t _in_idx);
		basic_node* clone_self(basic_node* _in_parent) const;
	};

	class select_node : public btree_node
	{
	public:
		select_node(btree_node* _in_parent, std::uint32_t _in_idx);
		basic_node* clone_self(basic_node* _in_parent) const;
	};

	class probility_node : public btree_node
	{
	public:
		probility_node(btree_node* _in_parent, std::uint32_t _in_idx, const std::vector<std::uint32_t>& weights);
		virtual std::string check_valid() const;
		virtual json to_json() const;
		std::vector<std::uint32_t> weights;
		basic_node* clone_self(basic_node* _in_parent) const;
		virtual bool check_edit();
		void refresh_editable_items();
		bool check_item_edit_refresh(std::shared_ptr<editable_item> change_item);
	};
	class if_else_node : public btree_node
	{
	public:
		if_else_node(btree_node* _in_parent, std::uint32_t _in_idx);
		basic_node* clone_self(basic_node* _in_parent) const;

	};
	class while_node : public btree_node
	{
	public:
		while_node(btree_node* _in_parent, std::uint32_t _in_idx);
		basic_node* clone_self(basic_node* _in_parent) const;
	};

	class parallel_node : public btree_node
	{
	public:
		parallel_node(btree_node* _in_parent, std::uint32_t _in_idx);
		basic_node* clone_self(basic_node* _in_parent) const;
	};

	class reset_node :public btree_node
	{
	public:
		reset_node(btree_node* _in_parent, std::uint32_t _in_idx);
		basic_node* clone_self(basic_node* _in_parent) const;
	};
	class wait_node :public btree_node
	{
	public:
		wait_node(btree_node* _in_parent, std::uint32_t _in_idx, const std::string& _in_event);
		basic_node* clone_self(basic_node* _in_parent) const;
		std::string cur_event;
		json to_json() const;
		virtual bool check_edit();
		void refresh_editable_items();
	};
	class sub_tree_node : public btree_node
	{
	public:
		sub_tree_node(btree_node* _in_parent, std::uint32_t _in_idx, const std::string& tree_name);
		std::string sub_tree_name;
		virtual std::string check_valid() const;
		virtual json to_json() const;
		virtual std::string display_text() const;
		basic_node* clone_self(basic_node* _in_parent) const;
		virtual bool check_edit();
		void refresh_editable_items();
	};

	class action_node :public btree_node
	{
	public:
		action_node(btree_node* _in_parent, std::uint32_t _in_idx,
			const std::string& _in_action_name,
			const std::vector<std::pair<action_arg_type, json>>& _in_action_args);
		std::string action_name;
		std::string agent_name;
		std::vector<std::pair<action_arg_type, json>> action_args;
		virtual std::string check_valid() const;
		virtual json to_json() const;
		virtual std::string display_text() const;
		basic_node* clone_self(basic_node* _in_parent) const;
		virtual bool check_edit();
		std::string get_agent_name() const;
		void refresh_editable_items();
		bool check_item_edit_refresh(std::shared_ptr<editable_item> change_item);
	};

}