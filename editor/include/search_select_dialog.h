#ifndef SEARCH_SELECT_DIALOG_H
#define SEARCH_SELECT_DIALOG_H

#include <QDialog>
#include <string>
#include <vector>
#include <utility>
#include <optional>

#include <QStringListModel>

namespace Ui {
class search_select_dialog;
}

class search_select_dialog : public QDialog
{
	Q_OBJECT


		using choice_type = std::string;
public:
    explicit search_select_dialog(const std::vector<choice_type>& _in_choices,
		QWidget *parent = nullptr
		);
    ~search_select_dialog();
	choice_type run();

private:
    Ui::search_select_dialog *ui;
	const std::vector<choice_type>& _choices;
	QStringList q_choices;
	std::string _result;
public slots:

	void close_handler();
	void search();
	void double_click(const QModelIndex& _index);
public:
	bool valid = false;
	QStringListModel* list_model;
};

#endif // SEARCH_SELECT_DIALOG_H
