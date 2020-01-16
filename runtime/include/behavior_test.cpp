#include <action_agent.h>
#include <thread>
using namespace spiritsaway::behavior_tree::runtime;
using namespace std;
using namespace std::chrono_literals;
int main()
{
	std::string test_btree_name = "new_btree_1.btree";
	const std::string btree_dir = "../../data/btree/";
	btree_desc::set_btree_directory(btree_dir);
	action_agent cur_agent;
	cur_agent.set_debug(true);
	cur_agent.load_btree(test_btree_name);
	cur_agent.enable(true);
	for (int i = 0; i < 10; i++)
	{
		this_thread::sleep_for(100ms);
		timer_manager::instance().poll(std::chrono::system_clock::now());
	}
}