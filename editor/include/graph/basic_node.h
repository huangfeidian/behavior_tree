#pragma once
#include <optional>
#include <memory>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace spiritsaway::behavior_tree::editor
{
	class struct_items;
	class editable_item;
	class basic_node
	{
	public:
		using node_idx_t = std::size_t;
		using node_desc_t = std::string;
		const node_desc_t _type;
		std::string comment;

		node_idx_t _idx;
		basic_node* _parent;
		std::vector<basic_node*> _children;
		bool _is_collapsed = false;
		bool _has_break_point = false;
		std::uint32_t color;//rgba
		std::uint32_t temp_color; // rgba
		std::shared_ptr<struct_items> _show_widget;
		void add_child(basic_node* in_child);
		void remove_child(basic_node* in_child);
		void move_child(basic_node* in_child, bool is_up);
		std::optional<std::uint32_t> index_of_child(const basic_node* in_child) const;
		basic_node(node_desc_t _in_type, basic_node* _in_parent, node_idx_t _in_idx);
		virtual std::string check_valid() const;
		virtual json to_json() const;
		virtual std::string display_text() const;
		virtual void destroy();
		basic_node* clone_recursive(basic_node* _in_parent) const;
		virtual basic_node* clone_self(basic_node* _in_parent) const = 0;

		virtual bool check_edit();
		
		basic_node* get_root() const;
		virtual void refresh_editable_items();
		virtual std::size_t max_child_num() const = 0;
		virtual std::size_t min_child_num() const = 0;

		virtual bool can_collapse() const;

		virtual bool check_item_edit_refresh(std::shared_ptr<editable_item> change_item);
		virtual basic_node* create_node(node_desc_t _type, basic_node* _in_parent, node_idx_t _idx) = 0;
	};

}