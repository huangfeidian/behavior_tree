#pragma once

#include <fstream>
#include <iostream>

#include <any_container/decode.h>

#include "node_enums.h"
#include "basic_node_desc.h"

#include <spdlog/logger.h>

namespace spiritsaway::behavior_tree::common
{

	class btree_desc
	{
	private:
		bool load_from_json(const json& btree_file_data, std::shared_ptr<spdlog::logger> in_logger)
		{
			if (!btree_file_data.is_object())
			{
				in_logger->error("cant parse to btree_desc, not str map");
				return false;
			}
			const auto& cur_map = btree_file_data.get<json::object_t>();

			auto tree_name_iter = cur_map.find("name");
			if (tree_name_iter == cur_map.end())
			{
				in_logger->error("cant find key name in btree data ");
				return false;
			}
			if (!tree_name_iter->second.is_string())
			{
				in_logger->error("value for key name is not str");
				return false;
			}
			tree_name = tree_name_iter->second.get<std::string>();

			

			auto extra_iter = cur_map.find("extra");
			if (extra_iter == cur_map.end())
			{
				in_logger->error("cant find key extra in btree data ");
				return false;
			}
			if (!extra_iter->second.is_object())
			{
				in_logger->error("value for key extra is not str map");
				return false;
			}
			extra = extra_iter->second.get<json::object_t>();

			auto nodes_iter = cur_map.find("nodes");
			if (nodes_iter == cur_map.end())
			{
				in_logger->error("cant find key nodes in btree data ");
				return false;
			}
			auto& cur_nodes = nodes_iter->second;
			if (!cur_nodes.is_array())
			{
				in_logger->error("value for key nodes is not is_vector");
				return false;
			}
			std::vector< basic_node_desc> temp_nodes;
			if (!serialize::decode(cur_nodes, temp_nodes))
			{
				in_logger->error("cant decode nodes");
				return false;
			}
			for (const auto& one_node : temp_nodes)
			{
				nodes[one_node.idx] = one_node;
				if (one_node.type != "root")
				{
					continue;
				}
				auto agent_name_iter = one_node.extra.find("agent_name");
				if (agent_name_iter == one_node.extra.end())
				{
					in_logger->error("cant find key agent_name in btree data ");
					return false;
				}
				if (!agent_name_iter->second.is_string())
				{
					in_logger->error("value for key agent_name is not str");
					return false;
				}
				agent_name = agent_name_iter->second.get<std::string>();
			}
			for (const auto& one_node : temp_nodes)
			{
				for (auto one_child : one_node.children)
				{
					if (nodes.find(one_child) == nodes.end())
					{
						in_logger->error("node {} has invalid child {}", one_node.idx, one_child);
						return false;
					}
				}
				if (one_node.parent && nodes.find(one_node.parent.value()) == nodes.end())
				{
					in_logger->error("node {} has invalid parent {}", one_node.idx, one_node.parent.value());
					return false;
				}
			}
			if (agent_name.empty())
			{
				in_logger->error("agent empty");
				return false;
			}
			return true;
		}
	public:
		btree_desc(const std::string& btree_file_path, std::shared_ptr<spdlog::logger> in_logger)
		{
			in_logger->info("loading btree {}", btree_file_path);
			auto btree_file_stream = std::ifstream(btree_file_path);
			std::string btree_file_str = std::string(std::istreambuf_iterator<char>(btree_file_stream),
				std::istreambuf_iterator<char>());
			json data = json::parse(btree_file_str, nullptr, false);


			if (!load_from_json(data, in_logger))
			{
				in_logger->error("fail to construct btree_desc from {}", data.dump());
				nodes.clear();
				return;
			}
		}
		const basic_node_desc& get_node(std::uint32_t cur_idx) const
		{
			auto cur_iter = nodes.find(cur_idx);
			if (cur_iter == nodes.end())
			{
				return invalid_node;
			}
			else
			{
				return cur_iter->second;
			}
		}
	public:
		std::unordered_map<std::uint32_t, basic_node_desc> nodes;
		basic_node_desc invalid_node;
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