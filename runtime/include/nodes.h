#pragma once

#include <algorithm>
#include <random>

#include <behavior/node_enums.h>
#include <logger.h>
#include <behavior/btree_desc.h>


#include "agent.h"
namespace spiritsaway::behavior_tree::runtime
{
	using namespace spiritsaway::behavior_tree::common;
	class node;

	class node_closure
	{

	public:
		node_closure(node* _in_node):
			_node(_in_node)
		{

		}
		virtual void operator()()
		{

		}
		virtual ~node_closure() 
		{
			_node = nullptr;
		}
	protected:
		node* _node;
	};

	
	class node
	{
	protected:
		static std::mt19937 _generator;
		static std::uniform_int_distribution<std::uint32_t> _distribution;
	public:
		bool in_fronts = false;
		node* _parent = nullptr;
		std::vector<node*> children;
		bool result = false;
		bool running = false;
		node_state _state;
		node_type _type;
		std::uint8_t next_child_idx = 0;
		agent* _agent;
		const std::uint32_t _node_idx;
		const btree_desc& btree_config;
		const basic_node_desc& node_config;
		std::shared_ptr<node_closure> _closure;
		std::shared_ptr<spdlog::logger> _logger;

		node(node* in_parent, agent* in_agent, std::uint32_t in_node_idx, 
			const btree_desc& in_btree, node_type in_type) :
			_parent(in_parent),
			_node_idx(in_node_idx),
			btree_config(in_btree),
			_state(node_state::init),
			next_child_idx(0),
			_agent(in_agent),
			_type(in_type),
			node_config(in_btree.nodes[in_node_idx]),
			_logger(std::move(logger_mgr::instance().create_logger("btree")))
		{

		}
		bool node_state_is_ready()
		{
			return _state == node_state::init || _state == node_state::awaken;
		}
		bool node_state_is_forbid_enter()
		{
			return _state == node_state::leaving || _state == node_state::dead;
		}
		void set_result(bool new_result);
		virtual bool handle_event(const event_type& cur_event)
		{
			return false;
		}
		void visit();
		void create_children();
		virtual void on_enter();
		virtual void on_revisit();
		virtual void visit_child(std::uint32_t child_idx);
		virtual void leave();
		virtual void interrupt();
		virtual void backtrace();
		const std::string& tree_name() const;
		std::string debug_info() const;
		static node* create_node_by_idx(const btree_desc& btree_config,
			std::uint32_t node_idx, node* parent, agent* in_agent);
	};

	
	
	class root :public node
	{
		using node::node;
	private:
		void on_enter();
		void on_revisit();
		friend class node;
	};
	class sequence :public node
	{
	protected:
		using node::node;
	private:
		void on_enter();
		void on_revisit();
		friend class node;
	};
	class always_seq :public sequence
	{
	protected:
		using sequence::sequence;
	private:
		void on_revisit();
		friend class node;
	};
	class random_seq : public node
	{
	protected:
		using node::node;
	private:
		std::vector<std::uint32_t> _shuffle;
		
		void on_enter();
		void on_revisit();
		friend class node;

	};
	class select : public node
	{
	protected:
		using node::node;
	private:
		void on_enter();
		void on_revisit();
		friend class node;
	};
	class probility : public node
	{
		using node::node;
	private:
		std::vector<std::uint32_t> _probilities;
		
		void on_enter();
		bool init_prob_parameters();
		std::uint32_t prob_choose_child_idx() const;
		void on_revisit();
		friend class node;

	};
	class if_else : public node
	{
		using node::node;
		void on_enter();
		void on_revisit();
		friend class node;
	};
	class while_loop : public node
	{
		using node::node;
		void on_enter();
		void on_revisit();
		friend class node;
	};
	class negative : public node
	{
		using node::node;
		void on_enter();
		void on_revisit();
		friend class node;
	};
	class always_true : public node
	{
		using node::node;
		void on_enter();
		void on_revisit();
		friend class node;
	};
	class sub_tree : public node
	{
		using node::node;
		void on_enter();
		bool create_sub_tree_node();
		void on_revisit();
		friend class node;

	};

	class parallel : public node
	{
		using node::node;
		void on_enter();
		void on_revisit();
		friend class node;
	};
	class action : public node
	{
		using node::node;
		std::string action_name;
		std::vector<std::pair<action_arg_type, json>> action_args;
		void on_enter();
		bool load_action_config();
		friend class node;
	};
	class wait_event : public node
	{
	public:
		event_type event;
		using node::node;
		void on_enter();
		virtual bool handle_event(const event_type& cur_event)
		{
			return cur_event == event;
		}
		friend class node;
	};
	class reset : public node
	{
		using node::node;
		reset(node* in_parent, agent* in_agent, std::uint32_t in_node_idx,
			const btree_desc& in_btree, node_type in_type) :
			node(in_parent, in_agent, in_node_idx, in_btree, in_type)
		{

		}
		void on_enter();
		friend class node;
	};
	class timeout_closure : public node_closure
	{
	public:
		timeout_closure(node* cur_node) :
			node_closure(cur_node)
		{

		}
		virtual void operator()()
		{
			_node->set_result(true);

		}
		virtual ~timeout_closure()
		{

		}
	};
}
