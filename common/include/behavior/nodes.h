#pragma once
#include <magic_enum.hpp>
#include <serialize/any_value.h>
#include <logger.h>

namespace spiritsaway::behavior_tree::common
{
	enum class node_type
	{
		invalid = 0,
		root,
		negative,
		sequence,
		always_seq,
		random_seq,
		if_else,
		while_loop,
		wait_event,
		reset,
		sub_tree,
		parallel,
		action,
		always_true,
		select,
		probility

	};
	enum class action_arg_type
	{
		plain = 0,
		blackboard,
	};
	enum class node_state
	{
		init = 0,
		entering,
		awaken,
		revisiting,
		blocking,
		leaving,
		wait_child,
		dead,
	};
	using node_idx_type = std::uint32_t;
	class node_desc
	{
	public:
		node_idx_type idx;
		node_type type;
		std::vector<node_idx_type> children;
		any_str_map extra;
		any_str_map editor;
		std::string comment;
		node_desc(const any_str_map &data, std::shared_ptr<spdlog::logger> _logger)
		{
			auto idx_iter = data.find("idx");
			if (idx_iter == data.end())
			{
				_logger->error("cant find key idx in node data ");
				return;
			}
			if (!idx_iter->second.is_int())
			{
				_logger->error("value for key idx is not int ");
				return;
			}
			idx = static_cast<node_idx_type>(std::get<any_int_type>(idx_iter->second));

			auto children_iter = data.find("children");
			if (children_iter == data.end())
			{
				_logger->error("cant find key children in node data ");
				return;
			}
			if (!children_iter->second.is_vector())
			{
				_logger->error("value for key children is not vec ");
				return;
			}
			for (auto& one_child : std::get<any_vector>(children_iter->second))
			{
				if (!one_child.is_int())
				{
					_logger->error("one_child idx is not int ");
					return;
				}
				children.push_back(static_cast<node_idx_type>(std::get<any_int_type>(one_child)));
			}
			auto extra_iter = data.find("extra");
			if (extra_iter == data.end())
			{
				_logger->error("cant find key extra in node data ");
				return;
			}
			if (!extra_iter->second.is_str_map())
			{
				_logger->error("value for key extra is not str_map ");
				return;
			}
			extra = std::get<any_str_map>(extra_iter->second);

			auto editor_iter = data.find("editor");
			if (editor_iter == data.end())
			{
				_logger->error("cant find key editor in node data ");
				return;
			}
			if (!editor_iter->second.is_str_map())
			{
				_logger->error("value for key editor is not str_map ");
				return;
			}
			editor = std::get<any_str_map>(editor_iter->second);

			auto comment_iter = data.find("comment");
			if (comment_iter == data.end())
			{
				_logger->error("cant find key comment in node data ");
				return;
			}
			if (!comment_iter->second.is_str())
			{
				_logger->error("value for key comment is not str ");
				return;
			}
			comment = std::get<std::string>(comment_iter->second);

			auto type_iter = data.find("type");
			if (type_iter == data.end())
			{
				_logger->error("cant find key type in node data ");
				return;
			}
			if (!type_iter->second.is_str())
			{
				_logger->error("value for key type is not str ");
				return;
			}
			auto type_str = std::get<std::string>(type_iter->second);
			auto type_result = magic_enum::enum_cast<node_type>(type_str);
			if (!type_result.has_value())
			{
				_logger->error("cant convert type str {} to node_type ", type_str);
				return;
			}
			type = type_result.value();

		}
		bool is_valid() const
		{
			return type != node_type::invalid;
		}
	};
}
