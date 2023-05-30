#pragma once

#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include <optional>
using json = nlohmann::json;

namespace spiritsaway::behavior_tree::common
{
    struct basic_node_desc
	{
		std::vector<std::uint32_t> children;
		std::uint32_t idx;
		std::string type;
		std::string comment;
		std::uint32_t color;
		std::optional<std::uint32_t> parent;
		bool is_collpased = false;
		json::object_t extra;
		virtual json encode() const
        {
            json::object_t result = json::object_t();
            result["idx"] = idx;
            result["children"] = children;
            if (parent)
            {
                result["parent"] = parent.value();
            }
            result["color"] = color;
            result["is_collapsed"] = is_collpased;
            result["extra"] = extra;
            result["type"] = type;
            result["comment"] = comment;
            return result;
        }
		virtual bool decode(const json& data)
		{
			auto temp_iter = data.find("idx");
			if (temp_iter == data.end())
			{
				return false;
			}

			if (!temp_iter->is_number_unsigned())
			{
				return false;
			}
			idx = temp_iter->get<std::uint32_t>();

			temp_iter = data.find("type");
			if (temp_iter == data.end())
			{
				return false;
			}
			if (!temp_iter->is_string())
			{
				return false;
			}
			type = temp_iter->get<std::string>();

			temp_iter = data.find("children");
			if (temp_iter == data.end())
			{
				return false;
			}
			if (!temp_iter->is_array())
			{
				return false;
			}
			for (auto one_item : *temp_iter)
			{
				if (!one_item.is_number_integer())
				{
					return false;
				}
				children.push_back(one_item.get<std::uint32_t>());
			}

            temp_iter = data.find("comment");
            if (temp_iter == data.end())
            {
                return false;
            }
			if (!temp_iter->is_string())
			{
				return false;
			}
			comment = temp_iter->get<std::string>();

            temp_iter = data.find("color");
            if (temp_iter == data.end())
            {
                return false;
            }
			if (!temp_iter->is_number_unsigned())
			{
				return false;
			}
			color = temp_iter->get<std::uint32_t>();

            temp_iter = data.find("is_collapsed");
            if (temp_iter == data.end())
            {
                return false;
            }
            if (!temp_iter->is_boolean())
            {
                return false;
            }
			is_collpased = temp_iter->get<bool>();

            temp_iter = data.find("extra");
            if (temp_iter == data.end())
            {
                return false;
            }
            if (!temp_iter->is_object())
            {
                return false;
            }
			extra = temp_iter->get<json::object_t>();
            
            temp_iter = data.find("parent");
            if (temp_iter != data.end())
            {
                if (!temp_iter->is_number_unsigned())
                {
                    return false;
                }
                parent = temp_iter->get<std::uint32_t>();
            }
            return true;
        }
	};
}