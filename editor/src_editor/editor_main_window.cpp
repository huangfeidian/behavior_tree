
#include <qfiledialog.h>
#include <btree/nodes.h>
#include <graph/tree_instance.h>
#include <choice_manager.h>
#include <btree/btree_config.h>
#include <dialogs/line_dialog.h>
#include <ui_editor_main_window.h>
#include <filesystem>
#include <dialogs/config_dialog.h>

#include "editor_main_window.h"

using namespace spiritsaway::behavior_tree::editor;

editor_main_window::editor_main_window(QWidget *parent)
    : multi_instance_window(parent)
    , ui(new Ui::editor_main_window)

{
	init_widgets();
	init_actions();
	
}

void editor_main_window::init_widgets()
{
	ui->setupUi(this);
	cur_mdi = ui->mdiArea;
	cur_mdi->setViewMode(QMdiArea::TabbedView);
	cur_mdi->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	cur_mdi->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	cur_mdi->setTabsMovable(true);
	cur_mdi->setTabsClosable(true);
	cur_mdi->setTabShape(QTabWidget::Rounded);
	connect(cur_mdi, SIGNAL(subWindowActivated(QMdiSubWindow *)), this, SLOT(sub_window_activated(QMdiSubWindow*)));
}

editor_main_window::~editor_main_window()
{
    delete ui;
}
bool editor_main_window::is_read_only() const
{
	return false;
}
void editor_main_window::init_actions()
{
	// file menu
	
	connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(action_open_handler()));
	
	connect(ui->actionClose, SIGNAL(triggered()), this, SLOT(action_close_handler()));
	connect(ui->actionCloseAll, SIGNAL(triggered()), this, SLOT(action_close_all_handler()));

	connect(ui->actionNew, SIGNAL(triggered()), this, SLOT(action_new_handler()));
	connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(action_save_handler()));
	connect(ui->actionSaveAs, SIGNAL(triggered()), this, SLOT(action_save_as_handler()));
	connect(ui->actionSaveAll, SIGNAL(triggered()), this, SLOT(action_save_all_handler()));
	connect(ui->actionExport, SIGNAL(triggered()), this, SLOT(action_export_handler()));
	connect(ui->actionExportAll, SIGNAL(triggered()), this, SLOT(action_export_all_handler()));

	connect(ui->actionInsert, SIGNAL(triggered()), this, SLOT(action_insert_handler()));
	connect(ui->actionDelete, SIGNAL(triggered()), this, SLOT(action_del_handler()));
	connect(ui->actionMoveUp, SIGNAL(triggered()), this, SLOT(action_move_up_handler()));
	connect(ui->actionMoveDown, SIGNAL(triggered()), this, SLOT(action_move_down_handler()));
	connect(ui->actionCopy, SIGNAL(triggered()), this, SLOT(action_copy_handler()));
	connect(ui->actionPaste, SIGNAL(triggered()), this, SLOT(action_paste_handler()));
	connect(ui->actionCut, SIGNAL(triggered()), this, SLOT(action_cut_handler()));
	connect(ui->actionGoto, SIGNAL(triggered()), this, SLOT(action_goto_handler()));
	connect(ui->actionFind, SIGNAL(triggered()), this, SLOT(action_find_handler()));
	// agent menu
	connect(ui->actionAgent, SIGNAL(triggered()), this, SLOT(action_agent_handler()));

}


std::size_t editor_main_window::get_seq()
{
	return ++seq;
}

std::string editor_main_window::new_file_name()
{
	std::string temp = fmt::format("new_btree_{}.btree", get_seq());
	while (already_open(temp))
	{
		temp = fmt::format("new_btree_{}.btree", get_seq());
	}
	return temp;
}


void editor_main_window::action_new_handler()
{
	_logger->debug("main_window action_new_handler");
	auto& _config = btree_config::instance();
	auto full_file = _config.btree_folder / new_file_name();
	node* cur_root = new root_node(_config.base_agent_name);
	cur_root->refresh_editable_items();
	tree_instance* cur_tree_instance = new tree_instance(full_file.string(), cur_root, this);
}


void editor_main_window::action_save_handler()
{
	_logger->debug("main_window action_save_handler");
	auto cur_window = cur_mdi->activeSubWindow();
	for (const auto one_tree : _instances)
	{
		if (one_tree->window == cur_window)
		{
			auto invalid_info = one_tree->save_handler();
			if (invalid_info.empty())
			{
				one_tree->refresh();
				one_tree->set_modfied(false);
				one_tree->update_title();
			}
			return;
		}
	}
	return;
}


void editor_main_window::action_save_all_handler()
{
	_logger->debug("main_window action_save_all_handler");
	auto cur_window = cur_mdi->activeSubWindow();
	for (const auto one_tree : _instances)
	{
		auto invalid_info = one_tree->save_handler();
		if (!invalid_info.empty())
		{
			return;
		}
	}
	auto cur_ins = active_instance;
	if (cur_ins)
	{
		cur_ins->refresh();
		cur_ins->set_modfied(false);
		cur_ins->update_title();
	}
	return;
}

void editor_main_window::action_export_handler()
{
	_logger->debug("main_window action_export_handler");
	action_save_handler();
}

void editor_main_window::action_export_all_handler()
{
	_logger->debug("main_window action_export_all_handler");
	action_save_all_handler();
}

void editor_main_window::action_save_as_handler()
{
	_logger->debug("main_window action_save_as_handler");
	auto cur_ins = active_instance;

	if (!cur_ins)
	{
		return;
	}
	auto& _config = btree_config::instance();
	auto fileName = QFileDialog::getOpenFileName(this,
		tr("Save Btree"), QString::fromStdString(_config.btree_folder.string()), tr("Btree File (*.btree)"));
	if (!fileName.size())
	{
		return;
	}
	auto cur_path = std::filesystem::path(fileName.toStdString());
	cur_ins->file_name = cur_path.filename();
	cur_ins->file_path = cur_path;
	cur_ins->window->setWindowTitle(QString::fromStdString(cur_ins->file_name.string()));
	auto invalid_info = cur_ins->save_handler();
	cur_ins->refresh();
}



void editor_main_window::closeEvent(QCloseEvent* ev)
{
	_logger->debug("main_window closeEvent");
	bool modified = false;
	for (auto one_ins : _instances)
	{
		if (one_ins->modified)
		{
			modified = true;
			break;
		}
	}
	if (modified)
	{
		QMessageBox::StandardButton  defaultBtn = QMessageBox::NoButton; //缺省按钮
		QMessageBox::StandardButton result;//返回选择的按钮
		result = QMessageBox::question(this, QString("Confirm"),
			QString("close without saving"), QMessageBox::Yes | QMessageBox::No, defaultBtn);
		if (result == QMessageBox::No)
		{
			ev->ignore();
			return;
		}
	}
	action_close_all_handler();
}
void editor_main_window::action_insert_handler()
{
	_logger->debug("main_window action_insert_handler");
	auto cur_ins = active_instance;

	if (!cur_ins)
	{
		return;
	}
	cur_ins->insert_handler();
}
void editor_main_window::action_copy_handler()
{
	_logger->debug("main_window action_copy_handler");
	auto cur_ins = active_instance;

	if (!cur_ins)
	{
		return;
	}
	copy_cut_slice = cur_ins->copy_handler();
}
void editor_main_window::action_del_handler()
{
	_logger->debug("main_window action_del_handler");
	auto cur_ins = active_instance;

	if (!cur_ins)
	{
		return;
	}
	cur_ins->delete_handler();
}

void editor_main_window::action_move_up_handler()
{
	_logger->debug("main_window action_move_up_handler");
	auto cur_ins = active_instance;

	if (!cur_ins)
	{
		return;
	}
	cur_ins->move_handler(true);
}
void editor_main_window::action_move_down_handler()
{
	_logger->debug("main_window action_move_down_handler");
	auto cur_ins = active_instance;

	if (!cur_ins)
	{
		return;
	}
	cur_ins->move_handler(false);
}

void editor_main_window::action_paste_handler()
{
	_logger->debug("main_window action_paste_handler");
	auto cur_ins = active_instance;

	if (!cur_ins)
	{
		return;
	}
	cur_ins->paste_handler(copy_cut_slice);
}

void editor_main_window::action_cut_handler()
{
	_logger->debug("main_window action_cut_handler");
	auto cur_ins = active_instance;

	if (!cur_ins)
	{
		return;
	}
	copy_cut_slice = cur_ins->cut_handler();
}


void editor_main_window::action_agent_handler()
{
	return;
}
