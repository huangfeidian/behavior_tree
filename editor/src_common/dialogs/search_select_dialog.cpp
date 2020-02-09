
#include <qlistview.h>

#include <dialogs/search_select_dialog.h>
#include <ui_search_select_dialog.h>


search_select_dialog::search_select_dialog(const std::vector<choice_type>& _in_choices, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::search_select_dialog),
	_choices(_in_choices)
{
    ui->setupUi(this);
	
	q_choices.reserve(_choices.size());
	for (const auto& one_choice : _choices)
	{
		q_choices.push_back(QString::fromStdString(one_choice));
	}
	//q_choices.sort(Qt::CaseInsensitive);
	list_model = new QStringListModel();
	list_model->setStringList(q_choices);
	ui->listView->setModel(list_model);
	connect(ui->lineEdit, SIGNAL(returnPressed()), this, SLOT(search()));
	connect(ui->listView, SIGNAL(doubleClicked(const QModelIndex&)),
		this, SLOT(double_click(const QModelIndex&)));

}

search_select_dialog::~search_select_dialog()
{
    delete ui;
	delete list_model;
}

void search_select_dialog::search()
{
	auto temp_text = ui->lineEdit->text();
	QStringList result_list;
	for (const auto& one_str : q_choices)
	{
		if (one_str.indexOf(temp_text, 0, Qt::CaseInsensitive) != -1)
		{
			result_list.push_back(one_str);
		}
	}
	list_model->setStringList(result_list);

}
void search_select_dialog::double_click(const QModelIndex& _index)
{
	auto the_item = list_model->data(_index, 0);
	if (!the_item.isValid())
	{
		return;
	}
	_result = the_item.toString().toStdString();
	close_handler();
}
void search_select_dialog::close_handler()
{
	valid = true;
	close();
}
search_select_dialog::choice_type search_select_dialog::run()
{
	ui->lineEdit->setFocus();
	exec();
	if (!valid)
	{
		return choice_type();
	}
	return _result;
}
