
#include <spdlog/formatter.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qgroupbox.h>
#include <memory>
#include <qformlayout.h>
#include <qlabel.h>
#include <qcolordialog.h>
#include <qstylefactory.h>
#include <qtextbrowser.h>

#include <iostream>

#include <choice_manager.h>
#include <editable_item.h>
#include <search_select_dialog.h>
#include <magic_enum.hpp>
using namespace behavior_tree::editor;



QColor behavior_tree::editor::color_from_uint(std::uint32_t rgba)
{
	std::uint8_t a = rgba & 0xff;
	rgba >>= 8;
	std::uint8_t b = rgba & 0xff;
	rgba >>= 8;
	std::uint8_t g = rgba & 0xff;
	rgba >>= 8;
	std::uint8_t r = rgba & 0xff;
	rgba >>= 8;
	return QColor(r, g, b, a);
}

editable_item::editable_item(editable_item_type _in_type, bool _in_is_container,
	const std::string& _in_name)
	: _type(_in_type)
	, _is_container(_in_is_container)
	, _name(_in_name)
{

}

std::string editable_item::name() const
{
	return _name;
}
std::string editable_item::display_name() const
{
	if (_comment.empty())
	{
		return _name;
	}
	else
	{
		return fmt::format("{}:{}", _name, _comment);
	}
}

std::string editable_item::input_valid() const
{
	return "";
}
bool editable_item::assign(const json& other)
{
	_value = other;
	return true;
}

json editable_item::to_json() const
{
	json result;
	result["name"] = _name;
	result["type"] = magic_enum::enum_name(_type);
	result["value"] = _value;
	return result;
}

std::string line_text::input_valid() const
{
	if (!_value.is_string())
	{
		return fmt::format("name: {} require text ", _name);
	}
	std::string temp = _value.get<std::string>();
	if (temp.size() > 50)
	{
		return fmt::format("name: {} has text length larger than 50", _name);
	}
	return "";
}
bool line_text::assign(const json& other)
{
	if (!other.is_string())
	{
		return false;
	}
	_value = other;
	return true;
}
json line_text::to_json() const
{
	return editable_item::to_json();
}

QWidget* line_text::to_editor(std::string _cur_name,
	modify_callback_func_t modify_callback)
{
	auto cur_window = new QLineEdit();
	auto cur_qstr = QString::fromStdString(_value.get<std::string>());
	cur_window->setText(cur_qstr);
	QObject::connect(cur_window, &QLineEdit::editingFinished, [=, self = shared_from_this()]()
		{
			auto new_qstr = cur_window->text();
			assign(str_convert(new_qstr));
			modify_callback(_cur_name, self, cur_window);
		});
	return cur_window;
}

single_line::single_line(const std::string& _in_name, const std::string& _in_value)
	:line_text(editable_item_type::single_line_text, false, _in_name)
{
	_value = _in_value;
}
json single_line::str_convert(const QString& input) const
{
	return input.toStdString();
}
std::shared_ptr<single_line> single_line::from_json(const json& data)
{
	auto value_iter = data.find("value");
	if (value_iter == data.end())
	{
		return {};
	}
	if (!value_iter->is_string())
	{
		return {};
	}
	auto name_iter = data.find("name");
	if (name_iter == data.end())
	{
		return {};
	}
	if (!name_iter->is_string())
	{
		return {};
	}

	return std::make_shared<single_line>(name_iter->get<std::string>(), 
		value_iter->get<std::string>());
}
json multi_line::str_convert(const QString& input) const
{
	return input.toStdString();
}
multi_line::multi_line(const std::string& _in_name, const std::string& _in_value)
	:line_text(editable_item_type::multi_line_text, false, _in_name)
{
	_value = _in_value;
}
std::shared_ptr<multi_line> multi_line::from_json(const json& data)
{
	auto value_iter = data.find("value");
	if (value_iter == data.end())
	{
		return {};
	}
	if (!value_iter->is_string())
	{
		return {};
	}
	auto name_iter = data.find("name");
	if (name_iter == data.end())
	{
		return {};
	}
	if (!name_iter->is_string())
	{
		return {};
	}

	return std::make_shared<multi_line>(name_iter->get<std::string>(),
		value_iter->get<std::string>());
}
QWidget* multi_line::to_editor(std::string _identifier, modify_callback_func_t modify_callback)
{
	auto cur_text = new multi_line_widget();
	auto cur_size_policy = QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
	cur_text->setSizePolicy(cur_size_policy);
	cur_text->setPlainText(QString::fromStdString(_value.get<std::string>()));
	cur_text->_editor_id = _identifier;
	cur_text->_editor = std::dynamic_pointer_cast<multi_line>(shared_from_this());
	cur_text->_modify_func = modify_callback;
	return cur_text;
}
std::string multi_line::input_valid() const
{
	return "";
}
bool_item::bool_item(const std::string& _in_name, bool _in_value)
	: editable_item(editable_item_type::_bool, false, _in_name)
{
	_value = _in_value;
}
std::string bool_item::input_valid() const
{
	if (!_value.is_boolean())
	{
		return fmt::format("name: {} require bool ", _name);
	}
	return "";
}
bool bool_item::assign(const json& other)
{
	if (!other.is_boolean())
	{
		return false;
	}
	_value = other;
	
	return true;
}
std::shared_ptr<bool_item> bool_item::from_json(const json& data)
{
	auto value_iter = data.find("value");
	if (value_iter == data.end())
	{
		return {};
	}
	if (!value_iter->is_boolean())
	{
		return {};
	}
	auto name_iter = data.find("name");
	if (name_iter == data.end())
	{
		return {};
	}
	if (!name_iter->is_string())
	{
		return {};
	}

	return std::make_shared<bool_item>(name_iter->get<std::string>(),
		value_iter->get<bool>());
}
QWidget* bool_item::to_editor(std::string _identifier,
	modify_callback_func_t modify_callback)
{
	auto cur_window = new QCheckBox();
	cur_window->setChecked(_value.get<bool>());
	QObject::connect(cur_window, &QCheckBox::stateChanged, [=, self = shared_from_this()]()
		{
			auto new_value = cur_window->isChecked();
			assign(new_value);
			modify_callback(_identifier, self, cur_window);
		});
	return cur_window;
}


color_item::color_item(const std::string& _in_name, std::uint32_t _in_value)
	: editable_item(editable_item_type::_color, false, _in_name)
{
	_value = _in_value;
}
std::string color_item::input_valid() const
{
	if (!_value.is_number_unsigned())
	{
		return fmt::format("name: {} require unsigned int ", _name);
	}
	return "";
}
bool color_item::assign(const json& other)
{
	if (!other.is_number_unsigned())
	{
		return false;
	}
	_value = other;

	return true;
}
std::shared_ptr<color_item> color_item::from_json(const json& data)
{
	auto value_iter = data.find("value");
	if (value_iter == data.end())
	{
		return {};
	}
	if (!value_iter->is_number_unsigned())
	{
		return {};
	}
	auto name_iter = data.find("name");
	if (name_iter == data.end())
	{
		return {};
	}
	if (!name_iter->is_string())
	{
		return {};
	}

	return std::make_shared<color_item>(name_iter->get<std::string>(),
		value_iter->get<std::uint32_t>());
}
QWidget* color_item::to_editor(std::string _identifier,
	modify_callback_func_t modify_callback)
{
	auto cur_dialog = new QDialog();
	auto cur_button = new QPushButton(u8"选择颜色");
	cur_button->setStyleSheet("border:1px groove gray;border-radius:5px;padding:1px 2px;");
	auto cur_frame = new QFrame();
	cur_frame->setFrameShape(QFrame::Box);
	cur_frame->setAutoFillBackground(true);
	cur_frame->setPalette(QPalette(color_from_uint(_value.get<std::uint32_t>())));
	auto cur_layout = new QHBoxLayout();
	cur_layout->addWidget(cur_frame);
	cur_layout->addWidget(cur_button);
	cur_dialog->setLayout(cur_layout);
	
	cur_button->connect(cur_button, &QPushButton::clicked, [=, self = shared_from_this()]()
		{

			QColor cur_color = QColorDialog::getColor();
			if (cur_color.isValid()) 
			{
				cur_frame->setPalette(QPalette(cur_color));
				std::uint8_t r = cur_color.red();
				std::uint8_t g = cur_color.green();
				std::uint8_t b = cur_color.black();
				std::uint8_t a = cur_color.alpha();
				std::uint32_t final_value = (r <<24) + (g <<16) + (b << 8) + a;
				//std::cout << fmt::format("color_item set with color ({}, {}, {}, {} final {})",
				//	 r,g,b,a, final_value) << std::endl;
				
				_value = final_value;
			}
			modify_callback(_identifier, self, cur_dialog);

		});
	return cur_dialog;
}

int_item::int_item(const std::string& _in_name, std::int32_t _in_value)
	:line_text(editable_item_type::_int, false, _in_name)
{
	_value = _in_value;
}
std::string int_item::input_valid() const
{
	if (!_value.is_number_integer())
	{
		return fmt::format("name: {} require number ", _name);
	}
	return "";
}
std::shared_ptr<int_item> int_item::from_json(const json& data)
{
	auto value_iter = data.find("value");
	if (value_iter == data.end())
	{
		return {};
	}
	if (!value_iter->is_number_integer())
	{
		return {};
	}
	auto name_iter = data.find("name");
	if (name_iter == data.end())
	{
		return {};
	}
	if (!name_iter->is_string())
	{
		return {};
	}

	return std::make_shared<int_item>(name_iter->get<std::string>(),
		value_iter->get<std::int32_t>());
}
bool int_item::assign(const json& other)
{
	if (!other.is_number_integer())
	{
		return false;
	}
	_value = other;
	return true;
}

json int_item::str_convert(const QString& input) const
{
	bool ok;
	int result = input.toInt(&ok);
	if (!ok)
	{
		return json();
	}
	else
	{
		return result;
	}
}

float_item::float_item(const std::string& _in_name, double _in_value)
	:line_text(editable_item_type::_float, false, _in_name)
{
	_value = _in_value;
}
std::string float_item::input_valid() const
{
	if (!_value.is_number_float())
	{
		return fmt::format("name: {} require number float", _name);
	}
	return "";
}
bool float_item::assign(const json& other)
{
	if (!other.is_number_float())
	{
		return false;
	}
	_value = other;
	return true;
}

std::shared_ptr<float_item> float_item::from_json(const json& data)
{
	auto value_iter = data.find("value");
	if (value_iter == data.end())
	{
		return {};
	}
	if (!value_iter->is_number_float())
	{
		return {};
	}
	auto name_iter = data.find("name");
	if (name_iter == data.end())
	{
		return {};
	}
	if (!name_iter->is_string())
	{
		return {};
	}

	return std::make_shared<float_item>(name_iter->get<std::string>(),
		value_iter->get<double>());
}
json float_item::str_convert(const QString& input) const
{
	bool ok;
	double result = input.toDouble(&ok);
	if (!ok)
	{
		return json();
	}
	else
	{
		return result;
	}
}

choice_item::choice_item(const std::string& _in_name, const std::string& choice_type,
	const std::vector<std::string>& _in_choices,
	const std::vector<std::string>& _in_choice_text,
	const std::string& _in_value)
	: editable_item(editable_item_type::_choice, false, _in_name)
	, _choice_type(choice_type)
	, _choices(_in_choices)
	, _choice_text(_in_choice_text)
{
	_value = _in_value;
}
std::uint32_t choice_item::current_index() const
{
	if (!_value.is_string())
	{
		return 0;
	}
	auto temp_str = _value.get<std::string>();
	for (std::uint32_t i = 0; i < _choice_text.size(); i++)
	{
		if (temp_str == _choices[i])
		{
			return i;
		}
	}
	return 0;
}
std::string choice_item::current_choice() const
{
	return _choices[current_index()];
}
std::string choice_item::input_valid() const
{
	if (!_value.is_string())
	{
		return fmt::format("name: {} require string ", _name);;
	}
	auto temp_str = _value.get<std::string>();
	if (temp_str.empty())
	{
		return fmt::format("name: {} choice should not empty", _name);
	}
	return "";

}
json choice_item::to_json() const
{
	auto result = editable_item::to_json();
	result["value"] = current_choice();
	result["choice_type"] = _choice_type;
	return result;
}


std::shared_ptr<choice_item> choice_item::from_json(const json& data)
{
	auto value_iter = data.find("value");
	if (value_iter == data.end())
	{
		return {};
	}
	if (!value_iter->is_string())
	{
		return {};
	}
	std::string cur_value = value_iter->get<std::string>();
	auto name_iter = data.find("name");
	if (name_iter == data.end())
	{
		return {};
	}
	if (!name_iter->is_string())
	{
		return {};
	}
	auto choice_iter = data.find("choice_type");
	if (choice_iter == data.end())
	{
		return {};
	}
	if (!choice_iter->is_string())
	{
		return {};
	}
	std::string cur_choice_type = choice_iter->get<std::string>();
	auto choice_ptrs = choice_manager::instance().get_choice(cur_choice_type);
	if (!choice_ptrs.first || !choice_ptrs.second)
	{
		std::cout << "choice_item::from_json with invalid choice_type " << cur_choice_type << std::endl;
		return {};
	}
	if (choice_ptrs.first->empty()|| choice_ptrs.second->empty())
	{
		return {};
	}
	if (std::find(choice_ptrs.first->begin(), choice_ptrs.first->end(), cur_value) 
		== choice_ptrs.first->end())
	{
		return {};
	}
	return std::make_shared<choice_item>(name_iter->get<std::string>(),
		cur_choice_type, *choice_ptrs.first, *choice_ptrs.second, cur_value);
}
bool choice_item::assign(const json& data)
{
	//std::cout << "choice item begin set with value " << data.dump() << std::endl;
	if (!data.is_string())
	{
		return false;
	}
	auto c_iter = std::find(_choices.begin(), _choices.end(), data.get<std::string>());
	if (c_iter == _choices.end())
	{
		return false;
	}
	//std::cout << "choice item end set with value " << data.dump() << std::endl;
	_value = data;
	return true;
}

QWidget* choice_item::to_editor(std::string _identifier,
	modify_callback_func_t modify_callback)
{
	if (_choices.size() > 3)
	{
		return to_editor_long(_identifier, modify_callback);
	}
	else
	{
		return to_editor_short(_identifier, modify_callback);
	}
}
QWidget* choice_item::to_editor_short(std::string _identifier,
	modify_callback_func_t modify_callback)
{
	auto cur_combo = new QComboBox();
	//cur_combo->setStyle(QStyleFactory::create("Fusion"));

	QStringList cur_list;
	for (const auto& one_item : _choice_text)
	{
		cur_list << QString::fromStdString(one_item);
	}
	cur_combo->addItems(cur_list);
	cur_combo->setCurrentIndex(current_index());

	cur_combo->connect(cur_combo, qOverload<int>(&QComboBox::currentIndexChanged), [=, self = shared_from_this()](int index)
	{
		//std::cout << "combo set with index " << index << std::endl;
		assign(_choices[index]);
		modify_callback(_identifier, self, cur_combo);
	});
	return cur_combo;

}

QWidget* choice_item::to_editor_long(std::string _identifier,
	modify_callback_func_t modify_callback)
{
	auto cur_dialog = new QDialog();
	auto cur_button = new QPushButton(u8"搜索");
	cur_button->setStyleSheet("border:1px groove gray;border-radius:5px;padding:1px 2px;");
	auto cur_text = new QPushButton(QString::fromStdString(_value.get<std::string>()));
	cur_text->setStyleSheet("border:1px groove gray;border-radius:5px;padding:1px 2px;");
	QPalette pa;
	pa.setColor(QPalette::WindowText, Qt::red);
	cur_text->setPalette(pa);
	auto cur_layout = new QHBoxLayout();
	cur_layout->addWidget(cur_text);
	cur_layout->addWidget(cur_button);
	auto cur_tooltips = new QTextBrowser();
	cur_tooltips->setText(QString::fromStdString(_choice_text[current_index()]));
	auto final_layout = new QVBoxLayout();
	final_layout->addLayout(cur_layout);
	final_layout->addWidget(cur_tooltips);
	cur_dialog->setLayout(final_layout);
	cur_button->connect(cur_button, &QPushButton::clicked, [=, self = shared_from_this()]()
		{

			auto cur_search_dialog = new search_select_dialog(_choice_text);
			auto result = cur_search_dialog->run();
			if (result.empty())
			{
				return;
			}
			for (std::size_t i = 0; i < _choice_text.size(); i++)
			{
				if (_choice_text[i] == result)
				{
					//std::cout << "search button get index " << i << " for text " << result << std::endl;
					assign(_choices[i]);
					cur_text->setText(QString::fromStdString(_choices[i]));
					modify_callback(_identifier, self, cur_dialog);
					return;
				}
			}
			
		});
	//cur_combo->connect(cur_combo, qOverload<int>(&QComboBox::currentIndexChanged), [=, self = shared_from_this()](int index)
	//	{
	//	//std::cout << "combo set with index " << index << std::endl;
	//		assign(_choices[index]);
	//		modify_callback(_identifier, self, cur_dialog);
	//	});
	return cur_dialog;

}
QWidget* list_items::to_editor(std::string _identifier,
	modify_callback_func_t modify_callback)
{
	auto cur_box = new QGroupBox();
	auto size_policy = cur_box->sizePolicy();
	size_policy.setVerticalPolicy(QSizePolicy::Maximum);
	cur_box->setSizePolicy(size_policy);
	auto cur_layout = new QVBoxLayout();
	cur_box->setLayout(cur_layout);
	std::shared_ptr<std::vector<QWidget*>> child_widgets = std::make_shared< std::vector<QWidget*>>();
	for (std::size_t i = 0; i < _children.size(); i++)
	{
		auto cur_child_widget = _children[i]->to_editor(
			fmt::format("{}[{}]", _identifier, i), modify_callback);
		child_widgets->push_back(cur_child_widget);
		cur_layout->addWidget(cur_child_widget);
	}
	auto button_add = new QPushButton("+");
	button_add->setStyleSheet("border:1px groove gray;border-radius:5px;padding:1px 2px;");
	button_add->connect(button_add, &QPushButton::clicked, [=, self = shared_from_this()]()
		{
			auto new_child_item = push();
			std::size_t idx = _children.size();
			auto temp_editor = new_child_item->to_editor(fmt::format("{}[{}]", _identifier, idx),
				modify_callback);
			child_widgets->push_back(temp_editor);

			modify_callback(_identifier, self, cur_box);

		});
	auto button_delete = new QPushButton("-");
	button_delete->setStyleSheet("border:1px groove gray;border-radius:5px;padding:1px 2px;");
	button_delete->connect(button_delete, &QPushButton::clicked, [=, self = shared_from_this()]()
		{
			if (child_widgets->empty())
			{
				return;
			}
			pop();
			child_widgets->back()->setParent(nullptr);
			child_widgets->pop_back();

			modify_callback(_identifier, self, cur_box);
		});
	auto hlayout = new QHBoxLayout();
	hlayout->addWidget(button_add);
	hlayout->addWidget(button_delete);
	cur_layout->addLayout(hlayout);
	cur_box->setTitle(QString::fromStdString(display_name()));
	return cur_box;
}
std::shared_ptr<editable_item> list_items::push()
{
	return editable_item::from_json(item_base);
}
void list_items::pop()
{
	if (_children.empty())
	{
		return;
	}
	_children.pop_back();
}
json list_items::to_json() const
{
	auto result = editable_item::to_json();
	json::array_t temp_values;
	for (const auto& one_item : _children)
	{
		temp_values.push_back(one_item->to_json()["value"]);
	}
	result["value"] = temp_values;
	result["item_base"] = item_base;
	return result;
}
list_items::list_items(const std::string& _in_name, const json& _in_item_base)
	:editable_item(editable_item_type::_list, true, _in_name)
	, item_base(_in_item_base)
{
	_value = json::array_t();
}
std::string list_items::input_valid() const
{
	for (const auto& one_child : _children)
	{
		auto temp_result = one_child->input_valid();
		if (!temp_result.empty())
		{
			return temp_result;
		}
	}
	return "";
}
std::shared_ptr<list_items> list_items::from_json(const json& data)
{
	auto value_iter = data.find("value");
	if (value_iter == data.end())
	{
		return {};
	}
	if (!value_iter->is_array())
	{
		return {};
	}
	auto name_iter = data.find("name");
	if (name_iter == data.end())
	{
		return {};
	}
	if (!name_iter->is_string())
	{
		return {};
	}
	auto base_iter = data.find("item_base");
	if (base_iter == data.end())
	{
		return {};
	}
	if (!base_iter->is_object())
	{
		return {};
	}
	auto child_item = editable_item::from_json(base_iter->object());
	if (!child_item)
	{
		return {};
	}
	auto temp = std::make_shared<list_items>(name_iter->get<std::string>(), base_iter->object());
	for (const auto& one_item : value_iter->get<json::array_t>())
	{
		auto cur_child = temp->push();
		cur_child->assign(one_item);
	}
	return temp;
}
json struct_items::to_json() const
{
	auto result = editable_item::to_json();
	json::array_t child_json;
	for (const auto& one_child : _children)
	{
		child_json.push_back(one_child->to_json());
	}
	result["value"] = child_json;
	return result;
}
std::shared_ptr<editable_item> struct_items::find(const std::string& _in_name) const
{
	for (const auto& one_child : _children)
	{
		if (one_child->name() == _in_name)
		{
			return one_child;
		}
	}
	return std::shared_ptr<editable_item>();
}
std::shared_ptr<editable_item> struct_items::push(const json& item_base)
{
	/*std::cout << "struct item push " << item_base.dump() << std::endl;*/
	auto temp_child = editable_item::from_json(item_base);
	if (!temp_child)
	{
		std::cout << "fail to construct editable item from " << item_base.dump() << std::endl;
		return {};
	}
	for (std::size_t i = 0; i < _children.size(); i++)
	{
		if (_children[i]->_name == temp_child->_name)
		{
			_children[i] = temp_child;
			return temp_child;
		}
	}
	_children.push_back(temp_child);
	return temp_child;
}

std::shared_ptr<editable_item> struct_items::pop(const std::string& _in_name)
{
	for (std::size_t i = 0; i < _children.size(); i++)
	{
		if (_children[i]->_name == _in_name)
		{
			std::shared_ptr<editable_item> result = _children[i];
			_children.erase(_children.begin() + i);
			return result;
		}
	}
	return std::shared_ptr<editable_item>();
}

struct_items::struct_items(const std::string& _in_name)
	:editable_item(editable_item_type::_struct, true, _in_name)
{
	_value = json::array_t();
}

QWidget* struct_items::to_editor(std::string _identifier, modify_callback_func_t modify_callback)
{
	auto cur_box = new QGroupBox();
	auto size_policy = cur_box->sizePolicy();
	size_policy.setVerticalPolicy(QSizePolicy::Maximum);
	
	cur_box->setSizePolicy(size_policy);
	auto cur_layout = new QFormLayout();
	cur_box->setLayout(cur_layout);
	cur_layout->setLabelAlignment(Qt::AlignRight);
	cur_layout->setSpacing(10);
	for (auto& one_child : _children)
	{

		//std::cout << fmt::format("struct items {} to editor with child {} data {} is_container {}", 
		//	_name,  one_child->_name , one_child->to_json().dump(), 
		//	one_child->_is_container)<< std::endl;
		auto cur_widget = one_child->to_editor(
			fmt::format("{}.{}", _identifier, one_child->_name), modify_callback);
		if (one_child->_is_container)
		{
			cur_layout->addRow(cur_widget);
		}
		else
		{
			cur_layout->addRow(QString::fromStdString(one_child->_name), cur_widget);
		}
	}
	cur_box->setTitle(QString::fromStdString(display_name()));
	return cur_box;
}
std::shared_ptr<struct_items> struct_items::from_json(const json& data)
{
	auto value_iter = data.find("value");
	if (value_iter == data.end())
	{
		return {};
	}
	if (!value_iter->is_array())
	{
		return {};
	}
	auto name_iter = data.find("name");
	if (name_iter == data.end())
	{
		return {};
	}
	if (!name_iter->is_string())
	{
		return {};
	}

	auto temp = std::make_shared<struct_items>(name_iter->get<std::string>());
	auto comment_iter = data.find("comment");
	if (comment_iter != data.end() && comment_iter->is_string())
	{
		temp->_comment = comment_iter->get<std::string>();
	}

	for (const auto& one_item : value_iter->get<json::array_t>())
	{
		auto cur_child = temp->push(one_item);
	}
	return temp;
}
std::string struct_items::input_valid() const
{
	for (const auto& one_child : _children)
	{
		auto temp_result = one_child->input_valid();
		if (!temp_result.empty())
		{
			return temp_result;
		}
	}
	return "";
}
std::shared_ptr<editable_item> editable_item::from_json(const json& data)
{
	std::vector<std::string> keys = { "type", "name", "value" };
	if (!data.is_object())
	{
		return {};
	}
	auto type_iter = data.find("type");
	if (type_iter == data.end())
	{
		return {};
	}
	if (!type_iter->is_string())
	{
		return {};
	}
	auto cur_type_opt = magic_enum::enum_cast<editable_item_type>(type_iter->get<std::string>());
	if (!cur_type_opt)
	{
		return {};
	}
	editable_item_type cur_type = cur_type_opt.value();
	if (cur_type == editable_item_type::invalid)
	{
		return {};
	}
	auto name_iter = data.find("name");
	if (name_iter == data.end())
	{
		return {};
	}
	if (!name_iter->is_string())
	{
		return {};
	}
	std::string cur_name = name_iter->get<std::string>();
	auto value_iter = data.find("value");
	if (value_iter == data.end())
	{
		return {};
	}
	switch (cur_type)
	{
	case editable_item_type::invalid:
		return {};
	case editable_item_type::line_text:
		return {};
	case editable_item_type::single_line_text:
	{
		return single_line::from_json(data);
	}
		
	case editable_item_type::multi_line_text:
	{
		return multi_line::from_json(data);
	}
		
	case editable_item_type::_bool:
	{
		return bool_item::from_json(data);
	}
	case editable_item_type::_color:
	{
		return color_item::from_json(data);
	}
	case editable_item_type::_int:
	{
		return int_item::from_json(data);
	}
		
	case editable_item_type::_float:
	{
		return float_item::from_json(data);
	}
	case editable_item_type::_choice:
	{
		return choice_item::from_json(data);
	}
	case editable_item_type::_list:
	{
		return list_items::from_json(data);
	}
	case editable_item_type::_struct:
	{
		return struct_items::from_json(data);
	}
		
	default:
		return {};
	}
	
}