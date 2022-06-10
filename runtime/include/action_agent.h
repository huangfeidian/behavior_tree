#pragma once
#include "agent.h"
#include "nodes.h"
#include <meta.h>
namespace spiritsaway::behavior_tree::runtime
{
	class action_agent : public agent
	{
	public:
		action_agent(const std::filesystem::path& _in_data_folder) :
			agent(_in_data_folder)
		{

		}
		// actions return true or false for immediate result
		// if the result is not immediate then return nullopt
		// all the arguments should take the form const T& or T
		std::optional<bool> agent_action(const std::string& action_name, 
			const json::array_t& action_args);

		//! \fn	Meta(behavior_action) bool has_key(const std::string& bb_key);
		//!
		//! \brief	判断黑板内是否有特定key
		//!
		//! \author	Administrator
		//! \date	2019/11/24
		//!
		//! \param	bb_key	key的名字
		//! \return 如果有这个key返回true 否则返回false

		Meta(behavior_action) bool has_key(const std::string& bb_key);

		//! \fn	Meta(behavior_action) bool set_key_value(const std::string& bb_key, const json& new_value);
		//!
		//! \brief	将黑板里特定key设置为特定值
		//!
		//! \author	Administrator
		//! \date	2019/11/24
		//!
		//! \param	bb_key	设置的黑板的key名字.
		//! \param	new_value 设置的新的值
		//!	\return 永远返回true

		Meta(behavior_action) bool set_key_value(const std::string& bb_key, 
			const json& new_value);

		//! \fn	Meta(behavior_action) bool has_key_value(const std::string& bb_key, const json& value);
		//!
		//! \brief	判断黑板里特定key是否等于特定值
		//!
		//! \author	Administrator
		//! \date	2019/11/24
		//!
		//! \param	bb_key	黑板key的名字.
		//! \param	value	要比较的值
		//! \return	如果没有这个key或者这个key的值不等于value 返回false 否则返回true

		Meta(behavior_action) bool has_key_value(const std::string& bb_key, 
			const json& value);

		//! \fn	Meta(behavior_action) bool number_add(const std::string& bb_key, const json& value);
		//!
		//! \brief	将特定黑板值增加一定数值 然后用结果更新次黑板值
		//!
		//! \author	Administrator
		//! \date	2019/11/24
		//!
		//! \param	bb_key	黑板里的特定key
		//! \param	value	用来增加的值 需要为数值类型
		//! \return 如果黑板key不存在返回false 如果黑板key对应的值不是数值返回false 如果传入的value不是数值类型返回false 操作成功返回true				

		Meta(behavior_action) bool number_add(const std::string& bb_key, 
			const json& value);

		//! \fn	Meta(behavior_action) bool number_dec(const std::string& bb_key, const json& value);
		//!
		//! \brief	将特定黑板值减去一定数值 然后用结果更新次黑板值
		//!
		//! \author	Administrator
		//! \date	2019/11/24
		//!
		//! \param	bb_key	黑板里的特定key
		//! \param	value	用来减去的值 需要为数值类型
		//! \return 如果黑板key不存在返回false 如果黑板key对应的值不是数值返回false 如果传入的value不是数值类型返回false 操作成功返回true	

		Meta(behavior_action) bool number_dec(const std::string& bb_key, 
			const json& value);

		//! \fn	Meta(behavior_action) bool number_multiply(const std::string& bb_key, const json& value);
		//!
		//! \brief	将特定黑板值乘于一定数值 然后用结果更新次黑板值
		//!
		//! \author	Administrator
		//! \date	2019/11/24
		//!
		//! \param	bb_key	黑板里的特定key
		//! \param	value	用来乘以的值 需要为数值类型
		//! \return 如果黑板key不存在返回false 如果黑板key对应的值不是数值返回false 如果传入的value不是数值类型返回false 操作成功返回true	

		Meta(behavior_action) bool number_multiply(const std::string& bb_key, 
			const json& value);

		//! \fn	Meta(behavior_action) bool number_div(const std::string& bb_key, const json& value);
		//!
		//! \brief	将特定黑板值除以一定数值 然后用结果更新次黑板值
		//!
		//! \author	Administrator
		//! \date	2019/11/24
		//!
		//! \param	bb_key	黑板里的特定key
		//! \param	value	用来乘以的值 需要为数值类型
		//! \return 如果黑板key不存在返回false 如果黑板key对应的值不是数值返回false 如果传入的value不是数值类型返回false 操作成功返回true	

		Meta(behavior_action) bool number_div(const std::string& bb_key, 
			const json& value);

		//! \fn	Meta(behavior_action) bool number_larger_than(const std::string& bb_key, const json& other_value);
		//!
		//! \brief	判断特定黑板值是否大于传入的值
		//!
		//! \author	Administrator
		//! \date	2019/11/24
		//!
		//! \param	bb_key	黑板的名字
		//! \param	other_value	用来比较的值
		//! \return 如果黑板不存在 返回false 如果两个相比较的值不都是数值类型 返回false 如果黑板值大于传入的值 返回true	否则返回false

		Meta(behavior_action) bool number_larger_than(const std::string& bb_key, 
			const json& other_value);

		//! \fn	Meta(behavior_action) bool number_less_than(const std::string& bb_key, const json& other_value);
		//!
		//! \brief	判断特定黑板值是否小于传入的值
		//!
		//! \author	Administrator
		//! \date	2019/11/24
		//!
		//! \param	bb_key	黑板的名字
		//! \param	other_value	用来比较的值
		//! \return 如果黑板不存在 返回false 如果两个相比较的值不都是数值类型 返回false 如果黑板值小于传入的值 返回true	否则返回false

		Meta(behavior_action) bool number_less_than(const std::string& bb_key, 
			const json& other_value);

		//! \fn	Meta(behavior_action) std::optional<bool> wait_for_seconds(double duration);
		//!
		//! \brief	等待一段时间
		//!
		//! \author	Administrator
		//! \date	2019/11/24
		//!
		//! \param	duration	等待的秒数 为浮点类型
		//! \return 需要异步执行 永远返回true					

		Meta(behavior_action) std::optional<bool> wait_for_seconds(double duration);

		//! \fn	Meta(behavior_action)bool log(const std::string& log_level, const any_value_type& log_info);
		//!
		//! \brief	打印信息到log
		//!
		//! \author	Administrator
		//! \date	2019/11/24
		//!
		//! \param	log_level	打印的等级 可选 debug info error warn
		//! \param	log_info	需要打印的值
		//! \return 永远返回true

		Meta(behavior_action)bool log(const std::string& log_level, const json& log_info);

		//! \fn	Meta(behavior_action) bool log_bb(const std::string& log_level, const std::string& bb_key);
		//!
		//! \brief	打印黑板值到log 如果黑板key不存在 则打印key %s not exist
		//!
		//! \author	Administrator
		//! \date	2019/11/24
		//!
		//! \param	log_level	打印的等级 可选debug info warn error.
		//! \param	bb_key 需要打印的黑板的key
		//! \return 永远返回true	

		Meta(behavior_action) bool log_bb(const std::string& log_level, const std::string& bb_key);
#ifndef __meta_parse__
#include "action_agent.generated_h"
#endif
	} Meta(behavior);
}