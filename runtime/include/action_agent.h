#pragma once
#include "agent.h"
#include "nodes.h"
#include <meta.h>
namespace behavior_tree::runtime
{
	class action_agent : public agent
	{
	public:
		action_agent() :
			agent()
		{

		}
		// actions return true or false for immediate result
		// if the result is not immediate then return nullopt
		// all the arguments should take the form const T& or T
		std::optional<bool> agent_action(const std::string& action_name, 
			const behavior_tree::common::any_vector& action_args);
		Meta(behavior_action) bool has_key(const std::string& bb_key);
		Meta(behavior_action) bool set_key_value(const std::string& bb_key, 
			const behavior_tree::common::any_value_type& new_value);
		Meta(behavior_action) bool has_key_value(const std::string& bb_key, 
			const behavior_tree::common::any_value_type& value);
		Meta(behavior_action) bool number_add(const std::string& bb_key, 
			const behavior_tree::common::any_value_type& value);
		Meta(behavior_action) bool number_dec(const std::string& bb_key, 
			const behavior_tree::common::any_value_type& value);
		Meta(behavior_action) bool number_multiply(const std::string& bb_key, 
			const behavior_tree::common::any_value_type& value);
		Meta(behavior_action) bool number_div(const std::string& bb_key, 
			const behavior_tree::common::any_value_type& value);
		Meta(behavior_action) bool number_larger_than(const std::string& bb_key, 
			const behavior_tree::common::any_value_type& other_value);
		Meta(behavior_action) bool number_less_than(const std::string& bb_key, 
			const behavior_tree::common::any_value_type& other_value);
		Meta(behavior_action) std::optional<bool> wait_for_seconds(double duration);
		Meta(behavior_action)bool log(const std::string& log_level, const any_value_type& log_info);
#include "action_agent.generated_h"
	} Meta(behavior);
}