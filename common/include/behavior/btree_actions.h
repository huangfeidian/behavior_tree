#pragma once
#include <nlohmann/json.hpp>
#include <filesystem>
#include <any_container/decode.h>
using json = nlohmann::json;
namespace fs = std::filesystem;
namespace spiritsaway::behavior_tree::editor
{

	struct action_arg
	{
		std::string name;
		std::string type;
		std::string comment;
		json encode() const
		{
			json::object_t result;
			result["name"] = name;
			result["type"] = type;
			result["comment"] = comment;
			return result;
		}
		bool decode(const json& data)
		{
			std::vector<std::string> keys = {"name", "type", "comment"};
			std::vector<std::string> values;
			for(auto one_key: keys)
			{
				auto cur_iter = data.find(one_key);
				if(cur_iter == data.end())
				{
					return false;
				}
				if(!cur_iter->is_string())
				{
					return false;
				}
				values.push_back(cur_iter->get<std::string>());
			}
			name = values[0];
			type = values[1];
			comment = values[2];
			return true;
		}
	};

	struct action_desc
	{
	public:
		std::string name;
		std::string brief;
		std::vector<action_arg> args;
		std::string return_info;
		json encode() const
		{
			json::object_t result;
			result["name"] = name;
			result["brief"] = brief;
			result["return"] = return_info;
			result["args"] = serialize::encode(args);
			return result;
		}
		bool decode(const json& data)
		{
			if(!data.is_object())
			{
				return false;
			}
			std::vector<std::string> keys = {"name", "brief", "return"};
			std::vector<std::string> values;
			for(auto one_key: keys)
			{
				auto cur_iter = data.find(one_key);
				if(cur_iter == data.end())
				{
					return false;
				}
				if(!cur_iter->is_string())
				{
					return false;
				}
				values.push_back(cur_iter->get<std::string>());
			}
			name = values[0];
			brief = values[1];
			return_info = values[2];
			auto args_iter = data.find("args");
			if(args_iter == data.end())
			{
				return false;
			}
			if(!serialize::decode(*args_iter, args))
			{
				return false;
			}
			return true;
		}
	};

	using agent_actions = std::unordered_map<std::string, action_desc>;
	class btree_action_repo
	{
	private:
		btree_action_repo()
		{

		}		
	public:
		std::unordered_map<std::string,
			agent_actions> actions_by_agent;
		bool decode(const json& data)
		{
			return serialize::decode(data, actions_by_agent);
		}
		json encode() const
		{
			return serialize::encode(actions_by_agent);
		}
		const agent_actions& get_actions_by_agent(const std::string& agent_name) const
		{
			static agent_actions invalid_actions;
			auto cur_iter = actions_by_agent.find(agent_name);
			if(cur_iter == actions_by_agent.end())
			{
				return invalid_actions;
			}
			else{
				return cur_iter->second;
			}
		}
		static btree_action_repo& instance()
		{
			static btree_action_repo _instance;
			return _instance;
		}

	};
	class agent_relation
	{
		std::unordered_map<std::string, std::string> _parent_relation;
		agent_relation()
		{

		}
	public:
		std::string get_root_agent() const
		{
			for(const auto& one_item: _parent_relation)
			{
				if(one_item.second.size() == 0)
				{
					return one_item.first;
				}
			}
			return "";
		}
		static agent_relation& instance()
		{
			static agent_relation _instance;
			return _instance;
		}
		bool decode(const json::object_t& data)
		{
			return serialize::decode(data, _parent_relation);
		}
		const std::unordered_map<std::string, std::string>& relations() const
		{
			return _parent_relation;
		}
	};
}