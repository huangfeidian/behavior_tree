#pragma once
#include <nlohmann/json.hpp>
#include <filesystem>
#include <logger.h>
using json = nlohmann::json;
namespace fs = std::filesystem;
namespace spiritsaway::behavior_tree::editor
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
		std::string brief;
		std::vector<action_arg> args;
		std::string return_info;
	};

	using agent_actions = std::unordered_map<std::string, action_desc>;
	class btree_config
	{
	public:
		fs::path btree_folder;
		fs::path export_folder;
		fs::path action_path;
		std::string base_agent_name;
		std::unordered_map<std::string,
			agent_actions> actions_by_agent;
		void dump_config() const;
		bool load_actions(const std::shared_ptr<spdlog::logger>& _logger);
		std::string load_config(fs::path config_path);
		btree_config(const btree_config& other) = delete;
		static btree_config& instance();
	private:
		btree_config();

	};
}