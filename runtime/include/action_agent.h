﻿#pragma once
#include "agent.h"
#include "nodes.h"
#include <random>

namespace spiritsaway::behavior_tree::runtime
{
	class timeout_closure : public node_closure
	{
	public:
		const std::uint64_t m_timer_handler;
		timeout_closure(node* cur_node, const json& data);

		static std::string closure_name()
		{
			return "time_out";
		}
		virtual ~timeout_closure();


	};

	class action_agent : public agent
	{
	public:
		using action_func_type = std::function<std::optional<bool>(const std::vector<json>&)>;
		action_agent(const std::filesystem::path& in_data_folder, std::shared_ptr<spdlog::logger> in_logger);
		using node_closure_creator_type = std::function < std::shared_ptr<node_closure>(node*, const json&)>;
		// actions return true or false for immediate result
		// if the result is not immediate then return nullopt
		// all the arguments should take the form const T& or T
		std::optional<bool> agent_action(const std::string& action_name,
			const json::array_t& action_args);

		//! \fn	bool has_key(const std::string& bb_key);
		//!
		//! \brief	判断黑板内是否有特定key
		//!
		//! \author	Administrator
		//! \date	2019/11/24
		//!
		//! \param	bb_key	key的名字
		//! \return 如果有这个key返回true 否则返回false

		bool has_key(const std::string& bb_key);

		//! \fn	bool set_key_value(const std::string& bb_key, const json& new_value);
		//!
		//! \brief	将黑板里特定key设置为特定值
		//!
		//! \author	Administrator
		//! \date	2019/11/24
		//!
		//! \param	bb_key	设置的黑板的key名字.
		//! \param	new_value 设置的新的值
		//!	\return 永远返回true

		bool set_key_value(const std::string& bb_key,
			const json& new_value);

		//! \fn	bool has_key_value(const std::string& bb_key, const json& value);
		//!
		//! \brief	判断黑板里特定key是否等于特定值
		//!
		//! \author	Administrator
		//! \date	2019/11/24
		//!
		//! \param	bb_key	黑板key的名字.
		//! \param	value	要比较的值
		//! \return	如果没有这个key或者这个key的值不等于value 返回false 否则返回true

		bool has_key_value(const std::string& bb_key,
			const json& value);

		//! \fn	bool number_add(const std::string& bb_key, const json& value);
		//!
		//! \brief	将特定黑板值增加一定数值 然后用结果更新次黑板值
		//!
		//! \author	Administrator
		//! \date	2019/11/24
		//!
		//! \param	bb_key	黑板里的特定key
		//! \param	value	用来增加的值 需要为数值类型
		//! \return 如果黑板key不存在返回false 如果黑板key对应的值不是数值返回false 如果传入的value不是数值类型返回false 操作成功返回true				

		bool number_add(const std::string& bb_key,
			const json& value);

		//! \fn	bool number_dec(const std::string& bb_key, const json& value);
		//!
		//! \brief	将特定黑板值减去一定数值 然后用结果更新次黑板值
		//!
		//! \author	Administrator
		//! \date	2019/11/24
		//!
		//! \param	bb_key	黑板里的特定key
		//! \param	value	用来减去的值 需要为数值类型
		//! \return 如果黑板key不存在返回false 如果黑板key对应的值不是数值返回false 如果传入的value不是数值类型返回false 操作成功返回true	

		bool number_dec(const std::string& bb_key,
			const json& value);

		//! \fn	bool number_multiply(const std::string& bb_key, const json& value);
		//!
		//! \brief	将特定黑板值乘于一定数值 然后用结果更新次黑板值
		//!
		//! \author	Administrator
		//! \date	2019/11/24
		//!
		//! \param	bb_key	黑板里的特定key
		//! \param	value	用来乘以的值 需要为数值类型
		//! \return 如果黑板key不存在返回false 如果黑板key对应的值不是数值返回false 如果传入的value不是数值类型返回false 操作成功返回true	

		bool number_multiply(const std::string& bb_key,
			const json& value);

		//! \fn	bool number_div(const std::string& bb_key, const json& value);
		//!
		//! \brief	将特定黑板值除以一定数值 然后用结果更新次黑板值
		//!
		//! \author	Administrator
		//! \date	2019/11/24
		//!
		//! \param	bb_key	黑板里的特定key
		//! \param	value	用来乘以的值 需要为数值类型
		//! \return 如果黑板key不存在返回false 如果黑板key对应的值不是数值返回false 如果传入的value不是数值类型返回false 操作成功返回true	

		bool number_div(const std::string& bb_key,
			const json& value);

		//! \fn	bool number_larger_than(const std::string& bb_key, const json& other_value);
		//!
		//! \brief	判断特定黑板值是否大于传入的值
		//!
		//! \author	Administrator
		//! \date	2019/11/24
		//!
		//! \param	bb_key	黑板的名字
		//! \param	other_value	用来比较的值
		//! \return 如果黑板不存在 返回false 如果两个相比较的值不都是数值类型 返回false 如果黑板值大于传入的值 返回true	否则返回false

		bool number_larger_than(const std::string& bb_key,
			const json& other_value);

		//! \fn	bool number_less_than(const std::string& bb_key, const json& other_value);
		//!
		//! \brief	判断特定黑板值是否小于传入的值
		//!
		//! \author	Administrator
		//! \date	2019/11/24
		//!
		//! \param	bb_key	黑板的名字
		//! \param	other_value	用来比较的值
		//! \return 如果黑板不存在 返回false 如果两个相比较的值不都是数值类型 返回false 如果黑板值小于传入的值 返回true	否则返回false

		bool number_less_than(const std::string& bb_key,
			const json& other_value);

		//! \fn	std::optional<bool> wait_for_seconds(double duration);
		//!
		//! \brief	等待一段时间
		//!
		//! \author	Administrator
		//! \date	2019/11/24
		//!
		//! \param	duration	等待的秒数 为浮点类型
		//! \return 需要异步执行 永远返回true					

		std::optional<bool> wait_for_seconds(double duration);

		//! \fn	bool log(const std::string& log_level, const any_value_type& log_info);
		//!
		//! \brief	打印信息到log
		//!
		//! \author	Administrator
		//! \date	2019/11/24
		//!
		//! \param	log_level	打印的等级 可选 debug info error warn
		//! \param	log_info	需要打印的值
		//! \return 永远返回true

		bool log(const std::string& log_level, const json& log_info);

		//! \fn	bool log_bb(const std::string& log_level, const std::string& bb_key);
		//!
		//! \brief	打印黑板值到log 如果黑板key不存在 则打印key %s not exist
		//!
		//! \author	Administrator
		//! \date	2019/11/24
		//!
		//! \param	log_level	打印的等级 可选debug info warn error.
		//! \param	bb_key 需要打印的黑板的key
		//! \return 永远返回true	

		bool log_bb(const std::string& log_level, const std::string& bb_key);

		bool nop();

		bool get_array_size(const std::string& array_bb_key, const std::string& sz_bb_key);

		bool choose_random(const std::string& array_bb_key, const std::string& dest_bb_key, std::uint32_t choose_sz, bool remove_chosen);

		bool choose_one_random(const std::string& array_bb_key, const std::string& dest_bb_key, bool remove_chosen);

		bool choose_idx(std::uint32_t idx, const std::string& array_bb_key, const std::string& dest_bb_key);

	private:
		std::unordered_map<std::string, action_func_type> m_action_funcs_map;
		std::unordered_map<std::string, node_closure_creator_type> m_node_closure_creators;
	private:


		template <class C, class F, class Tuple, std::size_t... I>
		constexpr decltype(auto) apply_impl(C* c, F f, Tuple& t, std::index_sequence<I...>)
		{
			// This implementation is valid since C++20 (via P1065R2)
			// In C++17, a constexpr counterpart of std::invoke is actually needed here
			return std::invoke(f, c, std::get<I>(t)...);
		}
	public:
		template <typename T, typename... Args>
		void add_action(const std::string& name, T* c, bool (T::* action_func)(Args...))
		{
			auto cur_lambda = [=](const std::vector<json>& input_args) -> std::optional<bool>
			{
				if (sizeof...(Args) != input_args.size())
				{
					return false;
				}
				std::tuple<std::remove_const_t<std::remove_reference_t<Args>>...> temp_tuple;
				if (!spiritsaway::serialize::decode(input_args, temp_tuple))
				{
					return false;
				}
				return apply_impl(c, action_func, temp_tuple, std::index_sequence_for<Args...>{});
			};
			m_action_funcs_map[name] = cur_lambda;
		}

		template <typename T, typename... Args>
		void add_async_action(const std::string& name, T* c, std::optional<bool>(T::* action_func)(Args...))
		{
			auto cur_lambda = [=](const std::vector<json>& input_args)-> std::optional<bool>
			{
				if (sizeof...(Args) != input_args.size())
				{
					return false;
				}
				std::tuple<std::remove_const_t<std::remove_reference_t<Args>>...> temp_tuple;
				if (!spiritsaway::serialize::decode(input_args, temp_tuple))
				{
					return false;
				}
				return apply_impl(c, action_func, temp_tuple, std::index_sequence_for<Args...>{});
			};
			m_action_funcs_map[name] = cur_lambda;
		}

		template <typename T>
		void add_node_closure_creator()
		{
			auto cur_closure_name = T::closure_name();
			auto cur_lambda = [=](node* cur_node, const json& cur_info)
			{
				return std::make_shared<T>(cur_node, cur_info);
			};
			m_node_closure_creators[cur_closure_name] = cur_lambda;
		}
		virtual json encode() const;
		virtual bool decode(const json& data);
		virtual void do_remove_timer(std::uint64_t handler);
	protected:
		std::vector<std::pair<std::uint64_t, node*>> m_active_timers;
		std::uint64_t m_next_timer_seq = 0;
		
		
	public:
		virtual std::uint64_t create_timer(std::uint64_t expire_gap_ms);
		void add_timer(std::uint64_t timer_handler, node* cur_node);
		void invoke_timer(std::uint64_t timer_handler);

		void remove_timer(std::uint64_t handler, bool with_invoke);

	};
}