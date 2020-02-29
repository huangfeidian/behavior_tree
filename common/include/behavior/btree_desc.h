#pragma once

#include <fstream>
#include <iostream>

#include <any_container/decode.h>

#include "node_enums.h"
#include "basic_node_desc.h"


namespace spiritsaway::behavior_tree::common
{

	class btree_desc
	{
	private:
		bool load_from_json(const json& btree_file_data, std::shared_ptr<spdlog::logger> _logger)
		{

			if (!btree_file_data.is_object())
			{
				_logger->error("cant parse to btree_desc, not str map");
				return false;
			}
			const auto& cur_map = btree_file_data.get<json::object_t>();

			auto tree_name_iter = cur_map.find("name");
			if (tree_name_iter == cur_map.end())
			{
				_logger->error("cant find key name in btree data ");
				return false;
			}
			if (!tree_name_iter->second.is_string())
			{
				_logger->error("value for key name is not str");
				return false;
			}
			tree_name = tree_name_iter->second.get<std::string>();

			

			auto extra_iter = cur_map.find("extra");
			if (extra_iter == cur_map.end())
			{
				_logger->error("cant find key extra in btree data ");
				return false;
			}
			if (!extra_iter->second.is_object())
			{
				_logger->error("value for key extra is not str map");
				return false;
			}
			extra = extra_iter->second.get<json::object_t>();

			auto nodes_iter = cur_map.find("nodes");
			if (nodes_iter == cur_map.end())
			{
				_logger->error("cant find key nodes in btree data ");
				return false;
			}
			auto& cur_nodes = nodes_iter->second;
			if (!cur_nodes.is_array())
			{
				_logger->error("value for key nodes is not is_vector");
				return false;
			}

			if (!serialize::decode(cur_nodes, nodes))
			{
				_logger->error("cant decode nodes");
				return false;
			}
			for (const auto& one_node : nodes)
			{
				if (one_node.type != "root")
				{
					continue;
				}
				auto agent_name_iter = one_node.extra.find("agent_name");
				if (agent_name_iter == one_node.extra.end())
				{
					_logger->error("cant find key agent_name in btree data ");
					return false;
				}
				if (!agent_name_iter->second.is_string())
				{
					_logger->error("value for key agent_name is not str");
					return false;
				}
				agent_name = agent_name_iter->second.get<std::string>();
			}
			if (agent_name.empty())
			{

				return false;
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
		std::vector<basic_node_desc> nodes;
		std::string tree_name;
		std::string agent_name;
		std::string signature;
		json::object_t extra;

		bool is_valid() const
		{
			return !agent_name.empty();
		}
	};
}