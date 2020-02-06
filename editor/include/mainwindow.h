#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <nlohmann/json.hpp>
#include <QMainWindow>
#include <optional>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QPen>
#include <qmdiarea.h>
#include <qevent.h>
#include <qmdisubwindow.h>
#include <qmessagebox.h>
#include <filesystem>
#include <QCloseEvent>
#include <logger.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

using json = nlohmann::json;

namespace spiritsaway::behavior_tree::editor
{
	class tree_instance;
	class node;
}
namespace bt_editor = spiritsaway::behavior_tree::editor;


class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(bool is_read_only, QWidget *parent = nullptr);
	~MainWindow();

public:
	Ui::MainWindow *ui;
	const bool is_read_only;
	QMdiArea* cur_mdi = nullptr;
	std::size_t seq = 0;
	std::vector<bt_editor::tree_instance*> _instances;
	bt_editor::tree_instance* active_instance = nullptr;
	bt_editor::node* copy_cut_slice = nullptr;
private:
	void init_widgets();
	void init_actions();
public:
	std::shared_ptr<spdlog::logger> _logger;

	std::size_t get_seq();
	std::string new_file_name();
	std::optional<std::size_t> already_open(const std::string& file_path);
public slots:
	void sub_window_activated(QMdiSubWindow* cur_window);

	void closeEvent(QCloseEvent* e);
	// file actions
	void action_new_handler();
	void action_open_handler();
	void action_save_handler();
	void action_save_all_handler();
	void action_export_handler();
	void action_export_all_handler();
	void action_save_as_handler();
	void action_close_handler();
	void action_close_all_handler();
	// node actions
	void action_insert_handler();
	void action_del_handler();
	void action_move_up_handler();
	void action_move_down_handler();
	void action_copy_handler();
	void action_paste_handler();
	void action_cut_handler();
	void action_find_handler();
	void action_goto_handler();
	// config actions
	void action_agent_handler();
	//instance handle
public:
	void add_instance(bt_editor::tree_instance* _cur_instance);
	void remove_instance(bt_editor::tree_instance* _cur_instance);
	bool load_config();
	void create_config();
};
#endif // MAINWINDOW_H
