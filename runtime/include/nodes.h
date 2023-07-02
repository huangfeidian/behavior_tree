#pragma once

#include <algorithm>
#include <random>

#include "node_enums.h"
#include "btree_desc.h"


#include "agent.h"
namespace spiritsaway::behavior_tree::runtime
{
	using namespace spiritsaway::behavior_tree::common;
	class node;

	class node_closure
	{

	public:
		node_closure(node* in_node, const std::string& name, const json& data)
			: m_node(in_node)
			, m_data(data)
			, m_name(name)
		{

		}
		virtual ~node_closure() 
		{
			m_node = nullptr;
		}

	protected:
		node* m_node;
		
	public:
		const json m_data;
		const std::string m_name;
		static std::string closure_name() 
		{
			return "invalid";
		}


	};

	
	class node
	{
	protected:
		static std::mt19937 m_generator;
		static std::uniform_int_distribution<std::uint32_t> m_distribution;
	public:
		node* m_parent = nullptr;
		std::vector<node*> children;
		bool result = false;
		bool running = false;
		node_state m_state;
		node_type m_type;
		std::uint8_t next_child_idx = 0;
		agent* m_agent;
		const std::uint32_t m_node_idx;
		const std::uint32_t m_tree_idx;
		const btree_desc& btree_config;
		const basic_node_desc& node_config;
		// 主要用来处理异常中断时的任务释放 同时外部任务也可以通过weak_ptr的形式来判断任务是否已经被中止
		std::shared_ptr<node_closure> m_closure; 
		std::shared_ptr<spdlog::logger> m_logger;

		node(node* in_parent, agent* in_agent, std::uint32_t in_node_idx, 
			const btree_desc& in_btree, node_type in_type) :
			m_parent(in_parent),
			m_node_idx(in_node_idx),
			m_tree_idx(in_agent->get_tree_idx(in_btree.tree_name)),
			btree_config(in_btree),
			m_state(node_state::init),
			next_child_idx(0),
			m_agent(in_agent),
			m_type(in_type),
			node_config(in_btree.get_node(in_node_idx)),
			m_logger(in_agent->logger())
		{

		}
		bool node_state_is_ready()
		{
			return m_state == node_state::init || m_state == node_state::awaken;
		}
		bool node_state_is_forbid_enter()
		{
			return m_state == node_state::leaving || m_state == node_state::dead;
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
		virtual json encode() const;
		virtual bool decode(const json& data)
		{
			return true;
		}
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
		std::vector<std::uint32_t> m_shuffle;
		void on_enter();
		void on_revisit();
		friend class node;
	public:
		const std::vector<std::uint32_t>& shuffle_children() const
		{
			return m_shuffle;
		}
		json encode() const override;
		bool decode(const json& data) override;

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
		std::vector<std::uint32_t> m_probilities;
		
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
	public:
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

}
