#include <config_dialog.h>
#include <ui_config_dialog.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qevent.h>


config_dialog::config_dialog(QWidget* parent)
	:QDialog(parent)
	,ui(new Ui::config_dialog)

{
	ui->setupUi(this);
	connect(ui->action_search, SIGNAL(clicked()), this, SLOT(action_file_search()));
	connect(ui->btree_search, SIGNAL(clicked()), this, SLOT(btree_folder_search()));
	connect(ui->export_search, SIGNAL(clicked()), this, SLOT(export_folder_search()));
	connect(ui->confirm_button, SIGNAL(clicked()), this, SLOT(confirm_handler()));
}

void config_dialog::confirm_handler()
{
	if (ui->action_label->text().isEmpty())
	{
		auto notify_info = fmt::format("empty path for {}", "action path");
		QMessageBox::about(this, QString("Error"),
			QString::fromStdString(notify_info));
		return;
	}
	if (ui->btree_label->text().isEmpty())
	{
		auto notify_info = fmt::format("empty path for {}", "btree folder");
		QMessageBox::about(this, QString("Error"),
			QString::fromStdString(notify_info));
		return;
	}
	if (ui->export_label->text().isEmpty())
	{
		auto notify_info = fmt::format("empty path for {}", "export folder");
		QMessageBox::about(this, QString("Error"),
			QString::fromStdString(notify_info));
		return;
	}
	if (ui->agent_name_edit->text().isEmpty())
	{
		auto notify_info = fmt::format("empty base agent name ");
		QMessageBox::about(this, QString("Error"),
			QString::fromStdString(notify_info));
		return;
	}
	auto& cur_btree_config = spiritsaway::behavior_tree::editor::btree_config::instance();
	cur_btree_config.base_agent_name = ui->agent_name_edit->text().toStdString();
	cur_btree_config.action_path = ui->action_label->text().toStdString();
	cur_btree_config.btree_folder = ui->btree_label->text().toStdString();
	cur_btree_config.export_folder = ui->export_label->text().toStdString();
	cur_btree_config.dump_config();
	valid = true;
	close();

}
void config_dialog::action_file_search()
{
	auto fileName = QFileDialog::getOpenFileName(this,
		tr("choose action file"), QString::fromStdString("./"), tr("Json File (*.json)"));
	if (!fileName.size())
	{
		return;
	}
	ui->action_label->setText(fileName);
}

void config_dialog::btree_folder_search()
{
	auto folder_name = QFileDialog::getExistingDirectory(this,
		tr("choose btree folder"), QString::fromStdString("./"));
	if (!folder_name.size())
	{
		return;
	}
	ui->btree_label->setText(folder_name);
}

void config_dialog::export_folder_search()
{
	auto folder_name = QFileDialog::getExistingDirectory(this,
		tr("choose export folder"), QString::fromStdString("./"));
	if (!folder_name.size())
	{
		return;
	}
	ui->export_label->setText(folder_name);
}
void config_dialog::run()
{
	ui->agent_name_edit->setFocus();
	exec();
}