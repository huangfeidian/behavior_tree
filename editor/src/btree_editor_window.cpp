#include <fstream>
#include <streambuf>

#include <qfiledialog.h>
#include <tree_editor/common/dialogs/path_config_dialog.h>
#include <tree_editor/common/choice_manager.h>
#include <tree_editor/common/graph/tree_instance.h>

#include <any_container/decode.h>

#include <btree_editor_window.h>
#include <behavior/btree_actions.h>
#include <behavior/btree_node.h>

using namespace spiritsaway::behavior_tree::editor;
using namespace spiritsaway::tree_editor;
using namespace std;

std::string btree_editor_window::new_file_name()
{
	std::string temp = fmt::format("new_btree_{}.json", get_seq());
	std::filesystem::path temp_path = data_folder / temp;
	while (already_open(temp) || std::filesystem::exists(temp_path))
	{
		temp = fmt::format("new_btree_{}.json", get_seq());
		temp_path = data_folder / temp;
	}
	return temp;
}

bool btree_editor_window::load_config()
{

	auto config_file_name = "btree_node_config.json";
	std::string notify_info;
	std::string node_desc_path;
	std::string action_desc_path;
	std::string save_path;
	std::string agent_desc_path;
	if (!std::filesystem::exists(std::filesystem::path(config_file_name)))
	{
		path_req_desc node_req;
		node_req.name = "behavior node types";
		node_req.tips = "file to provide all behavior nodes";
		node_req.extension = ".json";

		path_req_desc action_req;
		action_req.name = "behavior action choices";
		action_req.tips = "file to provide all actions";
		action_req.extension = ".json";

		path_req_desc agent_req;
		agent_req.name = "behavior agent hierachy";
		agent_req.tips = "file to provide all agent relations";
		agent_req.extension = ".json";

		path_req_desc save_path_req;
		save_path_req.name = "data save dir";
		save_path_req.tips = "directory to save data files";
		save_path_req.extension = "";

		std::vector<path_req_desc> path_reqs;
		path_reqs.push_back(node_req);
		path_reqs.push_back(action_req);
		path_reqs.push_back(agent_req);
		path_reqs.push_back(save_path_req);


		auto cur_dialog = new path_config_dialog(path_reqs, config_file_name, this);
		auto temp_result = cur_dialog->run();
		if (!cur_dialog->valid)
		{
			QMessageBox::about(this, QString("Error"),
				QString::fromStdString("invalid btree config"));
			return false;
		}
		node_desc_path = temp_result[0];
		action_desc_path = temp_result[1];
		agent_desc_path = temp_result[2];
		save_path = temp_result[3];
	}
	else
	{
		auto config_json_variant = load_json_file(config_file_name);
		if (std::holds_alternative<std::string>(config_json_variant))
		{
			auto notify_info = "config file: " + std::get<std::string>(config_json_variant);
			QMessageBox::about(this, QString("Error"),
				QString::fromStdString(notify_info));
			return false;
		}
		auto json_content = std::get<json::object_t>(config_json_variant);

		std::vector<std::string> temp_result;
		std::vector<std::string> path_keys = { "behavior node types", "behavior action choices" , "behavior agent hierachy", "data save dir" };
		for (auto one_key : path_keys)
		{
			auto cur_value_iter = json_content.find(one_key);
			if (cur_value_iter == json_content.end())
			{
				notify_info = "config content should be should has key " + one_key;
				QMessageBox::about(this, QString("Error"),
					QString::fromStdString(notify_info));
				return false;
			}
			if (!cur_value_iter->second.is_string())
			{
				notify_info = "config content should for key " + one_key + " should be str";
				QMessageBox::about(this, QString("Error"),
					QString::fromStdString(notify_info));
				return false;
			}
			temp_result.push_back(cur_value_iter->second.get<std::string>());
		}
		node_desc_path = temp_result[0];
		action_desc_path = temp_result[1];
		agent_desc_path = temp_result[2];
		save_path = temp_result[3];
	}
	std::vector<std::string> arg_type_choice = { std::string(magic_enum::enum_name(action_arg_type::blackboard)), std::string(magic_enum::enum_name(action_arg_type::plain)) };
	choice_manager::instance().add_choice("action_arg_type", arg_type_choice, arg_type_choice);

	// agent relation
	std::vector<std::string> agent_names;

	auto agent_json_variant = load_json_file(agent_desc_path);
	if (std::holds_alternative<std::string>(agent_json_variant))
	{
		auto notify_info = "agent relation file: " + std::get<std::string>(agent_json_variant);
		QMessageBox::about(this, QString("Error"),
			QString::fromStdString(notify_info));
		return false;
	}
	else
	{
		if (!agent_relation::instance().decode(std::get<json::object_t>(agent_json_variant)))
		{
			auto notify_info = "agent_relation file load content fail ";
			QMessageBox::about(this, QString("Error"),
				QString::fromStdString(notify_info));
			return false;
		}

		for (const auto& one_agent : agent_relation::instance().relations())
		{
			agent_names.push_back(one_agent.first);
		}
		choice_manager::instance().add_choice("agent_name", agent_names, agent_names);
	}
	// choice first 
	auto action_json_variant = load_json_file(action_desc_path);
	if (std::holds_alternative<std::string>(action_json_variant))
	{
		auto notify_info = "action file: " + std::get<std::string>(action_json_variant);
		QMessageBox::about(this, QString("Error"),
			QString::fromStdString(notify_info));
		return false;
	}
	else
	{
		if (!btree_action_repo::instance().decode(std::get<json::object_t>(action_json_variant)))
		{
			auto notify_info = "action file load content fail ";
			QMessageBox::about(this, QString("Error"),
				QString::fromStdString(notify_info));
			return false;
		}
		for (const auto& one_agent : agent_names)
		{
			std::vector<std::string> action_names;
			std::vector<std::string> action_comments;
			const auto& cur_actions = btree_action_repo::instance().get_actions_by_agent(one_agent);
			for (const auto& one_action : cur_actions)
			{
				action_names.push_back(one_action.first);
				action_comments.push_back(one_action.second.brief);
			}
			choice_manager::instance().add_choice(one_agent + "_actions", action_names, action_comments);
		}
	}
	
	//nodes depends on choice
	auto node_json_variant = load_json_file(node_desc_path);
	if (std::holds_alternative<std::string>(node_json_variant))
	{
		auto notify_info = "nodes file: " + std::get<std::string>(node_json_variant);
		QMessageBox::about(this, QString("Error"),
			QString::fromStdString(notify_info));
		return false;
	}
	else
	{
		node_config_repo::instance().load_config(std::get<json::object_t>(node_json_variant));
	}

	data_folder = save_path;
	return true;
}
basic_node* btree_editor_window::create_node_from_desc(const basic_node_desc& cur_desc, basic_node* parent)
{
	return btree_node::create_node_from_desc(cur_desc, parent);
}

