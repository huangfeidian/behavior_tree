
#include <fstream>
#include <iostream>

#include <spdlog/fmt/fmt.h>

#include <btree_config.h>
#include <choice_manager.h>

using namespace behavior_tree::editor;

bool btree_config::load_actions(const fs::path& action_file_path, 
	const std::shared_ptr<spdlog::logger>& _logger)
{
	auto action_file = std::ifstream(action_file_path);
	std::string file_content = std::string(std::istreambuf_iterator<char>(action_file),
		std::istreambuf_iterator<char>());
	action_file.close();
	auto config_json = json::parse(file_content);

	if (!config_json.is_object())
	{
		_logger->error("load_actions {} failed config data is not dict", action_file_path.string());
		return false;
	}
	for (json::iterator it = config_json.begin(); it != config_json.end(); it++)
	{
		std::string agent_name = it.key();
		json action_array = it.value();
		if (!action_array.is_array())
		{
			_logger->error("load_actions {} failed actions for agent {} is not array", 
				action_file_path.string(), agent_name);
			return false;
		}
		agent_actions temp_agent_actions;
		for (const auto& one_action : action_array)
		{
			action_desc temp_action;
			if (!one_action.is_object())
			{
				_logger->error("load_actions {} failed actions for agent {} temp action is not object",
					action_file_path.string(), agent_name);
				return false;
			}
			auto name_iter = one_action.find("name");
			if (name_iter == one_action.end())
			{
				_logger->error("load_actions {} failed actions for agent {} temp action cant find name",
					action_file_path.string(), agent_name);
				return false;
			}
			if (!name_iter->is_string())
			{
				_logger->error("load_actions {} failed actions for agent {} temp action cant find name",
					action_file_path.string(), agent_name);
				return false;
			}
			std::string cur_action_name = name_iter->get<std::string>();
			temp_action.name = cur_action_name;
			auto comment_iter = one_action.find("comment");
			if (comment_iter == one_action.end())
			{
				_logger->error("load_actions {} failed actions for agent {}  action  {} cant find comment",
					action_file_path.string(), agent_name, cur_action_name);
				return false;
			}
			if (!comment_iter->is_string())
			{
				_logger->error("load_actions {} failed actions for agent {}  action  {} cant find comment",
					action_file_path.string(), agent_name, cur_action_name);
				return false;
			}
			std::string cur_action_comment = comment_iter->get<std::string>();
			temp_action.comment = cur_action_comment;
			auto arg_array_iter = one_action.find("args");
			if (arg_array_iter == one_action.end())
			{
				_logger->error("load_actions {} failed actions for agent {}  action  {} cant find args",
					action_file_path.string(), agent_name, cur_action_name);
				return false;
			}
			if (!arg_array_iter->is_array())
			{
				_logger->error("load_actions {} failed actions for agent {}  action  {} args is not array",
					action_file_path.string(), agent_name, cur_action_name);
				return false;
			}
			
			json::array_t cur_arg_array = arg_array_iter->get<json::array_t>();
			
			for (const auto& one_arg_info : cur_arg_array)
			{
				if (!one_arg_info.is_array() || one_arg_info.size() != 3)
				{
					_logger->error("load_actions {} failed actions for agent {}  \
						action  {} one_arg is not array",
						action_file_path.string(), agent_name, cur_action_name);
					return false;
				}
				for (auto i = 0; i < 3; i++)
				{
					if (!one_arg_info[i].is_string())
					{
						_logger->error("load_actions {} failed actions for agent {}  \
						action  {} one_arg idx {} is not string",
							action_file_path.string(), agent_name, cur_action_name, i);
						return false;
					}
				}
				action_arg temp_arg;
				temp_arg.name = one_arg_info[0].get<std::string>();
				temp_arg.type = one_arg_info[1].get<std::string>();
				temp_arg.comment = one_arg_info[2].get<std::string>();
				temp_action.args.push_back(temp_arg);
			}
			temp_agent_actions[temp_action.name] = temp_action;
			//_logger->debug("agent {} add action {} with comment {}", agent_name, 
			//	temp_action.name, temp_action.comment);
		}
		actions_by_agent[agent_name] = temp_agent_actions;
	}
	for (const auto& [agent_name, agent_actions] : actions_by_agent)
	{
		std::string agent_key = agent_name;
		std::vector<std::string> actions;
		std::vector<std::string> action_comments;
		actions.push_back("");
		action_comments.push_back("invalid");
		for (const auto&[action_name, action_info] : agent_actions)
		{
			actions.push_back(action_name);
			action_comments.push_back(fmt::format("{}:{}", action_name, action_info.comment));
		}
		if (!choice_manager::instance().add_choice(agent_name, actions, action_comments))
		{
			_logger->error("load_actions {} failed add choice for  agent {}  failed",
				action_file_path.string(), agent_name);
			return false;
		}
	}
	return true;
}
btree_config::btree_config()
{

}
btree_config& btree_config::instance()
{
	static btree_config _instance;
	return _instance;
}