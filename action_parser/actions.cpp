#include "action_agent.h"

namespace spiritsaway::behavior_tree
{
	struct alternate_typenames
	{
		Meta(typename(std::string)) std::string a;
		Meta(typename(json)) json b;
		Meta(typename(std::vector<json>)) std::vector<json> c;
		Meta(typename(const json&)) const json& d;
	};
}
int main()
{
	return 0;
}