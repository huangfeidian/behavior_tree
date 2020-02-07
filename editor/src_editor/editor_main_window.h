#pragma once


#include <graph/multi_instance_window.h>
QT_BEGIN_NAMESPACE
namespace Ui { class editor_main_window; }
QT_END_NAMESPACE

using json = nlohmann::json;

namespace spiritsaway::behavior_tree::editor
{
	class tree_instance;
	class node;
}
namespace bt_editor = spiritsaway::behavior_tree::editor;


class editor_main_window : public bt_editor::multi_instance_window
{
	Q_OBJECT

public:
	editor_main_window(QWidget *parent = nullptr);
	~editor_main_window();

public:
	Ui::editor_main_window *ui;

	std::size_t seq = 0;
	bt_editor::node* copy_cut_slice = nullptr;
private:
	void init_widgets();
	void init_actions();
public:

	bool is_read_only() const;
	std::size_t get_seq();
	std::string new_file_name();

public slots:


	void closeEvent(QCloseEvent* e);
	// file actions
	void action_new_handler();
	void action_save_handler();
	void action_save_all_handler();
	void action_export_handler();
	void action_export_all_handler();
	void action_save_as_handler();
	// node actions
	void action_insert_handler();
	void action_del_handler();
	void action_move_up_handler();
	void action_move_down_handler();
	void action_copy_handler();
	void action_paste_handler();
	void action_cut_handler();

	// config actions
	void action_agent_handler();
	//instance handle

};

