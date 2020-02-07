#pragma once

#include <QDialog>
#include <string>
#include <vector>
#include <utility>
#include <optional>

#include <QStringListModel>
#include <btree/btree_config.h>

namespace Ui
{
	class config_dialog;
}
class config_dialog :public QDialog
{
	Q_OBJECT

public:
	explicit config_dialog(QWidget* parent = nullptr);
	void run();
	bool valid = false;
private:
	Ui::config_dialog *ui;
	
public slots:
	void confirm_handler();
	void action_file_search();
	void btree_folder_search();
	void export_folder_search();

};
