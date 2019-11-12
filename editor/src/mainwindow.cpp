#include <mainwindow.h>

#include <qfiledialog.h>
#include <nodes.h>
#include <tree_instance.h>
#include <choice_manager.h>
#include <btree_config.h>
#include <goto_dialog.h>
#include <ui_mainwindow.h>


using namespace behavior_tree::editor;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , _logger(behavior_tree::common::logger_mgr::instance().create_logger("btree"))
{
    ui->setupUi(this);
	init_widgets();
	init_actions();
	load_config();
	showMaximized();
	
}

void MainWindow::init_widgets()
{
	seq = 0;
	cur_mdi = ui->mdiArea;
	cur_mdi->setViewMode(QMdiArea::TabbedView);
	cur_mdi->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	cur_mdi->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	cur_mdi->setTabsMovable(true);
	cur_mdi->setTabsClosable(true);
	cur_mdi->setTabShape(QTabWidget::Rounded);
	connect(cur_mdi, SIGNAL(subWindowActivated(QMdiSubWindow *)), this, SLOT(sub_window_activated(QMdiSubWindow*)));
	cur_docker = ui->dockAnchor;
	auto temp_layout = new QVBoxLayout();
	cur_docker->setLayout(temp_layout);
	temp_layout->setAlignment(Qt::AlignTop);
	dock_content_count = 0;
}
MainWindow::~MainWindow()
{
    delete ui;
}
bool MainWindow::load_config()
{
	auto& _config = btree_config::instance();
	_config.btree_folder = "../../data/btree/";
	_config.export_foler = "../../data/export/";
	if (!_config.load_actions("../../data/actions.json", _logger))
	{
		auto notify_info = fmt::format("cant load actions.json in ../../data/");
		QMessageBox::about(this, QString("Error"),
			QString::fromStdString(notify_info));
		exit(1);
		return false;
	}
	_config.base_agent_name = "action_agent";
	std::vector<std::string> node_types;
	node_types.push_back(std::string(magic_enum::enum_name(node_type::negative)));
	node_types.push_back(std::string(magic_enum::enum_name(node_type::sequence)));
	node_types.push_back(std::string(magic_enum::enum_name(node_type::always_seq)));
	node_types.push_back(std::string(magic_enum::enum_name(node_type::random_seq)));
	node_types.push_back(std::string(magic_enum::enum_name(node_type::select)));
	node_types.push_back(std::string(magic_enum::enum_name(node_type::probility)));
	node_types.push_back(std::string(magic_enum::enum_name(node_type::if_else)));
	node_types.push_back(std::string(magic_enum::enum_name(node_type::while_loop)));
	node_types.push_back(std::string(magic_enum::enum_name(node_type::wait_event)));
	node_types.push_back(std::string(magic_enum::enum_name(node_type::reset)));
	node_types.push_back(std::string(magic_enum::enum_name(node_type::sub_tree)));
	node_types.push_back(std::string(magic_enum::enum_name(node_type::parallel)));
	node_types.push_back(std::string(magic_enum::enum_name(node_type::action)));
	node_types.push_back(std::string(magic_enum::enum_name(node_type::always_true)));
	choice_manager::instance().add_choice("node_type", node_types, node_types);

	std::vector<std::string> node_arg_type_strs;
	node_arg_type_strs.push_back("plain");
	node_arg_type_strs.push_back("blackboard");
	choice_manager::instance().add_choice("action_arg_type", node_arg_type_strs, node_arg_type_strs);
	return true;
}
void MainWindow::sub_window_activated(QMdiSubWindow* cur_win)
{
	_logger->debug("main_window sub_window_activated");
	auto pre_instance = active_instance;
	if (cur_win && pre_instance && pre_instance->window == cur_win)
	{
		return;
	}
	tree_instance* next_instance = nullptr;
	for (const auto one_ins : _instances)
	{
		if (one_ins->window == cur_win)
		{
			next_instance = one_ins;
			break;
		}
	}
	if (pre_instance)
	{
		pre_instance->deactivate_handler();
	}
	if (next_instance)
	{
		next_instance->activate_handler();
	}
	active_instance = next_instance;

}

void MainWindow::init_actions()
{
	// file menu
	connect(ui->actionNew, SIGNAL(triggered()), this, SLOT(action_new_handler()));
	connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(action_open_handler()));
	connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(action_save_handler()));
	connect(ui->actionSaveAs, SIGNAL(triggered()), this, SLOT(action_save_as_handler()));
	connect(ui->actionSaveAll, SIGNAL(triggered()), this, SLOT(action_save_all_handler()));
	connect(ui->actionExport, SIGNAL(triggered()), this, SLOT(action_export_handler()));
	connect(ui->actionExportAll, SIGNAL(triggered()), this, SLOT(action_export_all_handler()));
	connect(ui->actionClose, SIGNAL(triggered()), this, SLOT(action_close_handler()));
	connect(ui->actionCloseAll, SIGNAL(triggered()), this, SLOT(action_close_all_handler()));

	// node menu
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

void MainWindow::add_dock_content(QWidget * cur_widget)
{
	_logger->debug("main_window add_dock_content");
	clear_dock_contents();
	ui->dockAnchor->layout()->addWidget(cur_widget);
}

std::vector<QWidget*> MainWindow::get_dock_contents()
{
	_logger->debug("main_window get_dock_contents");
	auto cur_layout = ui->dockAnchor->layout();
	std::vector<QWidget*> result;
	for (int i = 0; i < cur_layout->count(); i++)
	{
		result.push_back(cur_layout->itemAt(i)->widget());
	}
	return result;

}

void MainWindow::clear_dock_contents()
{
	_logger->debug("main_window clear_dock_contents");
	for (auto one_widget : get_dock_contents())
	{
		one_widget->deleteLater();
	}
}

std::size_t MainWindow::get_seq()
{
	return ++seq;
}

std::string MainWindow::new_file_name()
{
	std::string temp = fmt::format("new_btree_{}.btree", get_seq());
	while (already_open(temp))
	{
		temp = fmt::format("new_btree_{}.btree", get_seq());
	}
	return temp;
}

std::optional<std::size_t> MainWindow::already_open(const std::string & file_path)
{

	for (std::size_t i = 0; i < _instances.size(); i++)
	{
		_logger->info("cur instance path {} input_path {}", _instances[i]->file_name.string(), file_path);
		if (_instances[i]->file_name == file_path)
		{
			return i;
		}
	}
	return {};
}

void MainWindow::action_new_handler()
{
	_logger->debug("main_window action_new_handler");
	auto& _config = btree_config::instance();
	auto full_file = _config.btree_folder / new_file_name();
	node* cur_root = new root_node(_config.base_agent_name);
	cur_root->refresh_editable_items();
	tree_instance* cur_tree_instance = new tree_instance(full_file.string(), cur_root, this);
}

void MainWindow::action_open_handler()
{
	_logger->debug("main_window action_open_handler");
	auto& _config = btree_config::instance();
	auto fileName = QFileDialog::getOpenFileName(this,
		tr("Open Btree"), QString::fromStdString(_config.btree_folder.string()), tr("Btree File (*.btree)"));
	if (!fileName.size())
	{
		return;
	}
	if (already_open(fileName.toStdString()))
	{
		return;
	}
	node* cur_root = node::load(fileName.toStdString(), _logger);
	if (!cur_root)
	{
		auto notify_info = fmt::format("cant open {}, content invalid", fileName.toStdString());
		QMessageBox::about(this, QString("Error"),
			QString::fromStdString(notify_info));
	}
	else
	{
		tree_instance* cur_tree_instance = new tree_instance(fileName.toStdString(), cur_root, this);
	}

}

void MainWindow::action_save_handler()
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


void MainWindow::action_save_all_handler()
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

void MainWindow::action_export_handler()
{
	_logger->debug("main_window action_export_handler");
	action_save_handler();
}

void MainWindow::action_export_all_handler()
{
	_logger->debug("main_window action_export_all_handler");
	action_save_all_handler();
}

void MainWindow::action_save_as_handler()
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

void MainWindow::action_close_handler()
{
	_logger->debug("main_window action_close_handler");
	auto cur_ins = active_instance;

	if (!cur_ins)
	{
		return;
	}
	cur_ins->refresh();
	remove_instance(cur_ins);
	
}
void MainWindow::action_close_all_handler()
{
	_logger->debug("main_window action_close_all_handler");
	auto pre_instances = _instances;
	for (auto one_ins : pre_instances)
	{
		remove_instance(one_ins);
	}
}

void MainWindow::closeEvent(QCloseEvent* ev)
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
void MainWindow::action_insert_handler()
{
	_logger->debug("main_window action_insert_handler");
	auto cur_ins = active_instance;

	if (!cur_ins)
	{
		return;
	}
	cur_ins->insert_handler();
}
void MainWindow::action_copy_handler()
{
	_logger->debug("main_window action_copy_handler");
	auto cur_ins = active_instance;

	if (!cur_ins)
	{
		return;
	}
	copy_cut_slice = cur_ins->copy_handler();
}
void MainWindow::action_del_handler()
{
	_logger->debug("main_window action_del_handler");
	auto cur_ins = active_instance;

	if (!cur_ins)
	{
		return;
	}
	cur_ins->delete_handler();
}

void MainWindow::action_move_up_handler()
{
	_logger->debug("main_window action_move_up_handler");
	auto cur_ins = active_instance;

	if (!cur_ins)
	{
		return;
	}
	cur_ins->move_handler(true);
}
void MainWindow::action_move_down_handler()
{
	_logger->debug("main_window action_move_down_handler");
	auto cur_ins = active_instance;

	if (!cur_ins)
	{
		return;
	}
	cur_ins->move_handler(false);
}

void MainWindow::action_paste_handler()
{
	_logger->debug("main_window action_paste_handler");
	auto cur_ins = active_instance;

	if (!cur_ins)
	{
		return;
	}
	cur_ins->paste_handler(copy_cut_slice);
}

void MainWindow::action_cut_handler()
{
	_logger->debug("main_window action_cut_handler");
	auto cur_ins = active_instance;

	if (!cur_ins)
	{
		return;
	}
	copy_cut_slice = cur_ins->cut_handler();
}
void MainWindow::action_find_handler()
{
	_logger->debug("main_window action_find_handler");
	auto cur_ins = active_instance;

	if (!cur_ins)
	{
		return;
	}
	cur_ins->search_node();
}
void MainWindow::action_goto_handler()
{
	_logger->debug("main_window action_goto_handler");
	auto cur_ins = active_instance;

	if (!cur_ins)
	{
		return;
	}
	auto cur_goto_dialog = new goto_dialog(this);
	auto opt_idx = cur_goto_dialog->run();
	if (!opt_idx)
	{
		auto notify_info = fmt::format("cant get idx from input, shoule be an interger");
		QMessageBox::about(this, QString("Error"),
			QString::fromStdString(notify_info));
	}
	else
	{
		auto cur_idx = opt_idx.value();
		cur_ins->focus_on(cur_idx);
	}
	
	
}
void MainWindow::add_instance(tree_instance* _cur_instance)
{
	_logger->debug("main_window add_instance {}", _cur_instance->file_name.string());
	_instances.push_back(_cur_instance);
	cur_mdi->setActiveSubWindow(_cur_instance->window);
	_cur_instance->display_tree();
}
void MainWindow::remove_instance(tree_instance* _cur_instance)
{
	_logger->debug("main_window remove_instance {}", _cur_instance->file_name.string());
	std::size_t i = 0;
	for (; i < _instances.size(); i++)
	{
		if (_instances[i] == _cur_instance)
		{
			break;
		}
	}
	_cur_instance->_root->destroy();
	_instances.erase(_instances.begin() + i);
	if (_instances.empty())
	{
		return;
	}
	if (i >= _instances.size())
	{
		sub_window_activated(_instances.back()->window);
	}
	else
	{
		sub_window_activated(_instances[i]->window);
	}
}

void MainWindow::action_agent_handler()
{
	return;
}
