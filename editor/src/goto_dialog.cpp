#include <goto_dialog.h>

using namespace behavior_tree::editor;
goto_dialog::goto_dialog(QWidget* parent)
{
	QHBoxLayout *cur_layout = new QHBoxLayout;
	_label = new QLabel(tr(u8"节点编号"));
	_line_edit = new QLineEdit();
	_button = new QPushButton(tr(u8"跳转"));
	cur_layout->addWidget(_label);
	cur_layout->addWidget(_line_edit);
	cur_layout->addWidget(_button);
	setLayout(cur_layout);
	connect(_button, SIGNAL(clicked()), this, SLOT(search()));
}

void goto_dialog::search()
{
	auto temp_text = _line_edit->text();
	_text = temp_text.toStdString();
	close();
}
std::optional<std::uint32_t> goto_dialog::run()
{
	_line_edit->setFocus();
	exec();
	std::uint32_t result = 0;
	for (const auto i : _text)
	{
		if (i < '0' || i > '9')
		{
			return {};
		}
		auto cur_digit = i - '0';
		result = result * 10 + cur_digit;
	}
	return result;
}