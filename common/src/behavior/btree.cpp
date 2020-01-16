#include <behavior/btree.h>
using namespace spiritsaway::behavior_tree::common;
std::unordered_map<std::string, const btree_desc*> btree_desc::tree_cache = {};
std::string btree_desc::btree_respository = "";