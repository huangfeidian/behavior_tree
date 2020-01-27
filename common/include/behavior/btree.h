#pragma once

#include <fstream>
#include <iostream>



#include "nodes.h"


namespace spiritsaway::behavior_tree::common
{

	class btree_desc
	{
	private:
		bool load_from_json(const json& btree_file_data, std::shared_ptr<spdlog::logger> _logger)
		{
			any_value_type parse_result = any_encode(btree_file_data);
			if (!parse_result.is_str_map())
			{
				_logger->error("cant parse to btree_desc, not str map");
				return false;
			}
			const auto& cur_map = std::get<any_str_map>(parse_result);

			auto tree_name_iter = cur_map.find("name");
			if (tree_name_iter == cur_map.end())
			{
				_logger->error("cant find key name in btree data ");
				return false;
			}
			if (!tree_name_iter->second.is_str())
			{
				_logger->error("value for key name is not str");
				return false;
			}
			tree_name = std::get<std::string>(tree_name_iter->second);

			auto agent_name_iter = cur_map.find("agent_name");
			if (agent_name_iter == cur_map.end())
			{
				_logger->error("cant find key agent_name in btree data ");
				return false;
			}
			if (!agent_name_iter->second.is_str())
			{
				_logger->error("value for key agent_name is not str");
				return false;
			}
			agent_name = std::get<std::string>(agent_name_iter->second);

			auto extra_iter = cur_map.find("extra");
			if (extra_iter == cur_map.end())
			{
				_logger->error("cant find key extra in btree data ");
				return false;
			}
			if (!extra_iter->second.is_str_map())
			{
				_logger->error("value for key extra is not str map");
				return false;
			}
			extra = std::get<any_str_map>(extra_iter->second);

			auto nodes_iter = cur_map.find("nodes");
			if (nodes_iter == cur_map.end())
			{
				_logger->error("cant find key nodes in btree data ");
				return false;
			}
			auto& cur_nodes = nodes_iter->second;
			if (!cur_nodes.is_vector())
			{
				_logger->error("value for key nodes is not is_vector");
				return false;
			}

			for (auto& one_node : std::get<any_vector>(cur_nodes))
			{
				if (!one_node.is_str_map())
				{
					_logger->error("one_node is not str_map");
					return false;
				}
				auto new_node = node_desc(std::get<any_str_map>(one_node), _logger);
				if (!new_node.is_valid())
				{
					_logger->error("cant construct node_desc");
					return false;
				}
				if (nodes.size() != new_node.idx)
				{
					_logger->error("nodes size {} should equal to cur_node {}", nodes.size(), new_node.idx);
					return false;
				}
				nodes.push_back(new_node);
			}

			return true;
		}
	public:
		btree_desc(const std::string& btree_file_path, std::shared_ptr<spdlog::logger> _logger)
		{
			auto btree_file_stream = std::ifstream(btree_file_path);
			std::string btree_file_str = std::string(std::istreambuf_iterator<char>(btree_file_stream),
				std::istreambuf_iterator<char>());
			json data = json::parse(btree_file_str, nullptr, false);


			if (!load_from_json(data, _logger))
			{
				_logger->error("fail to construct btree_desc from {}", data.dump());
				nodes.clear();
				return;
			}
		}
	public:
		std::vector<node_desc> nodes;
		std::string tree_name;
		std::string agent_name;
		std::string signature;
		any_str_map extra;

		bool is_valid() const
		{
			return !nodes.empty();
		}
		static std::unordered_map<std::string, const btree_desc*> tree_cache;
		static const btree_desc* load(const std::string& file_path, std::shared_ptr<spdlog::logger> _logger)
		{
			auto cur_iter = tree_cache.find(file_path);
			if (cur_iter != tree_cache.end())
			{
				return cur_iter->second;
			}
			auto new_btree = new btree_desc(btree_respository + file_path, _logger);
			if (new_btree->nodes.empty())
			{
				return nullptr;
			}
			tree_cache[file_path] = new_btree;
			return new_btree;
		}
		static std::string btree_respository;
		static void set_btree_directory(const std::string& cur_directory)
		{
			btree_respository = cur_directory;
		}
	};
	enum class agent_cmd
	{
		snapshot = 0,//tree_indexes node_idxes blackboard
		push_tree,// new tree name, tree_idx
		node_enter,//tree_idx, node_idx
		node_leave,//tree_idx, node_idx
		node_action,//tree_idx, node_idx, action_name, action_args
		bb_set,//key, value
		bb_clear,
	};
	using agent_cmd_detail = std::tuple<std::uint64_t, agent_cmd, spiritsaway::serialize::any_vector>;
}