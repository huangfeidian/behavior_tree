#include <btree_debug_window.h>
#include <behavior/btree_actions.h>
#include <behavior/btree_node.h>
#include <tree_editor/common/dialogs/path_config_dialog.h>
#include <tree_editor/common/choice_manager.h>
#include <tree_editor/common/graph/tree_instance.h>
#include <tree_editor/debugger/log_dialog.h>
#include <qfiledialog.h>

using namespace std;
using namespace spiritsaway::behavior_tree::debugger;
using namespace spiritsaway::behavior_tree::common;
using namespace spiritsaway::behavior_tree::editor;

using namespace spiritsaway::tree_editor;

bool btree_debug_window::load_config()
{
	auto config_file_name = "btree_node_config.json";
	std::string notify_info;
	std::string node_desc_path;
	std::string action_desc_path;
	std::string save_path;
	std::string agent_desc_path;
	std::string history_path;
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
		save_path_req.name = "btree data dir";
		save_path_req.tips = "directory to open btree files";
		save_path_req.extension = "";

		path_req_desc history_path_req;
		history_path_req.name = "history data dir";
		history_path_req.tips = "directory to open history files";
		history_path_req.extension = "";

		std::vector<path_req_desc> path_reqs;
		path_reqs.push_back(node_req);
		path_reqs.push_back(action_req);
		path_reqs.push_back(agent_req);
		path_reqs.push_back(save_path_req);
		path_reqs.push_back(history_path_req);


		auto cur_dialog = new path_config_dialog(path_reqs, config_file_name, this);
		auto temp_result = cur_dialog->run();
		if (!cur_dialog->m_valid)
		{
			QMessageBox::about(this, QString("Error"),
				QString::fromStdString("invalid btree config"));
			return false;
		}
		node_desc_path = temp_result[0];
		action_desc_path = temp_result[1];
		agent_desc_path = temp_result[2];
		save_path = temp_result[3];
		history_path = temp_result[4];
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
		std::vector<std::string> path_keys = { "behavior node types", "behavior action choices" , "behavior agent hierachy", "btree data dir", "history data dir" };
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
		history_path = temp_result[4];

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

	m_data_folder = save_path;
	history_folder = history_path;
	return true;
}
basic_node* btree_debug_window::create_node_from_desc(const basic_node_desc& cur_desc, basic_node* parent)
{
	return btree_node::create_node_from_desc(cur_desc, parent);
}
void btree_debug_window::action_open_handler()
{
	if (_debug_source != debug_source::no_debug)
	{
		auto notify_info = fmt::format("already during debug, please close all previous ");
		QMessageBox::about(this, QString("Error"),
			QString::fromStdString(notify_info));
		return;
	}

	auto fileName = QFileDialog::getOpenFileName(this,
		tr("Open Tree Debug File"), QString::fromStdString(history_folder.string()), tr("Json File (*.json)"));
	if (!fileName.size())
	{
		std::string error_info = "empty file name";
		QMessageBox::about(this, QString("Error"),
			QString::fromStdString(error_info));
		return;
	}
	std::vector<node_trace_cmd> cmds;

	auto cur_json_content = load_json_file(fileName.toStdString());
	if (std::holds_alternative<std::string>(cur_json_content))
	{
		std::string error_info = std::get<std::string>(cur_json_content);
		QMessageBox::about(this, QString("Error"),
			QString::fromStdString("file to open cmd history file " + error_info));
		return;
	}
	const auto& cur_file_content = std::get<json::object_t>(cur_json_content);
	auto entity_id_iter = cur_file_content.find("entity_id");
	if (entity_id_iter == cur_file_content.end() || !entity_id_iter->second.is_string())
	{
		std::string error_info = std::get<std::string>(cur_json_content);
		QMessageBox::about(this, QString("Error"),
			QString::fromStdString("file to open cmd history file "  ": valid entity_id not found"));
		return;
	}
	_log_viewer->reset();
	debug_entity_id = entity_id_iter->second.get<std::string>();
	auto cmds_iter = cur_file_content.find("debug_cmds");
	if (cmds_iter == cur_file_content.end() || !serialize::decode(cmds_iter->second, cmds))
	{
		std::string error_info = std::get<std::string>(cur_json_content);
		QMessageBox::about(this, QString("Error"),
			QString::fromStdString("file to open cmd history file "  ": valid debug_cmds not found"));
		return;
	}
	for (const auto& one_cmd : cmds)
	{
		_tree_cmds.push_back(one_cmd);
	}
	
}
void btree_debug_window::action_close_all_handler()
{
	debugger_main_window::action_close_all_handler();
	_log_viewer->reset();
	debug_entity_id = "";
	_tree_cmds.clear();
	if (_debug_source == spiritsaway::tree_editor::debug_source::http_debug)
	{
		_http_server.reset();
	}
	
	_debug_source = debug_source::no_debug;
}