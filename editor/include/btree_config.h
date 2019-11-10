#pragma once
#include <nlohmann/json.hpp>
#include <filesystem>
#include <logger.h>
using json = nlohmann::json;
namespace fs = std::filesystem;
namespace behavior_tree::editor
{

	struct action_arg
	{
		std::string name;
		std::string type;
		std::string comment;
	};

	struct action_desc
	{
	public:
		std::string name;
		std::string comment;
		std::vector<action_arg> args;
	};

	using agent_actions = std::unordered_map<std::string, action_desc>;
	class btree_config
	{
	public:
		fs::path btree_folder;
		fs::path export_foler;
		std::string base_agent_name;
		std::unordered_map<std::string,
			agent_actions> actions_by_agent;

		bool load_actions(const fs::path& action_file_path, const std::shared_ptr<spdlog::logger>& _logger);
		btree_config(const btree_config& other) = delete;
		static btree_config& instance();
	private:
		btree_config();

	};
}