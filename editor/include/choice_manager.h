#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
namespace spiritsaway::behavior_tree::editor
{

	class choice_manager
	{
	private:
		choice_manager()
		{

		}
		std::unordered_map<std::string, std::pair<std::shared_ptr<std::vector<std::string>>,
			std::shared_ptr<std::vector<std::string>>>> _choices;
	public:
		bool add_choice(const std::string& _choice_type,
			const std::vector<std::string>& _in_choice, const std::vector<std::string>& _in_choice_text)
		{
			auto cur_iter = _choices.find(_choice_type);
			if (cur_iter != _choices.end())
			{
				return false;
			}
			else
			{
				auto choice_ptr = std::make_shared<std::vector<std::string>>(_in_choice);
				auto choice_text_ptr = std::make_shared<std::vector<std::string>>(_in_choice_text);
				_choices[_choice_type] = std::make_pair(choice_ptr, choice_text_ptr);
				return true;
			}
		}
		std::pair<std::shared_ptr<const std::vector<std::string>>,
			std::shared_ptr<const std::vector<std::string>>> get_choice(const std::string& _choice_type)
		{
			auto cur_iter = _choices.find(_choice_type);
			if (cur_iter == _choices.end())
			{
				return {};
			}
			else
			{
				return cur_iter->second;
			}
		}
		static choice_manager& instance()
		{
			static choice_manager _instance;
			return _instance;
		}
	};
}