#pragma once

#include <tree_editor/debugger/debugger_main_window.h>

namespace spiritsaway::behavior_tree::debugger
{
	using namespace spiritsaway::tree_editor;
	class btree_debug_window :public tree_editor::debugger_main_window
	{
		basic_node* create_node_from_desc(const basic_node_desc& desc, basic_node* parent) override;

	protected:
		std::string debug_entity_id;
		void action_open_handler() override;
		void action_close_all_handler() override;
	public:
		bool load_config() override;

	};
}