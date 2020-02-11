#pragma once

#include <QDialog>
#include <qwidget.h>

#include <string>
#include <vector>
#include <utility>
#include <optional>
#include <functional>

#include <QStringListModel>
#include <nlohmann/json.hpp>
#include <memory>
#include <qplaintextedit.h>


using json = nlohmann::json;
namespace spiritsaway::behavior_tree::editor
{

	enum class editable_item_type
	{
		invalid = 0,
		text_browser,
		line_text,
		single_line_text,
		multi_line_text,
		_bool,
		_color,
		_int,
		_float,
		_json,
		_choice,
		_list,
		_struct,
	};
	QColor color_from_uint(std::uint32_t rgba);
	std::uint32_t color_to_uint(QColor color);

	class editable_item;
	using modify_callback_func_t = std::function<void(std::shared_ptr<editable_item>)>;
	class editable_item : public std::enable_shared_from_this<editable_item>
	{
	public:
		virtual std::string display_name() const;
		std::string name() const;
		virtual json to_json() const;
		virtual std::string input_valid() const;
		virtual bool assign(const json& other);
		// to a writable widget
		virtual QWidget* to_editor( modify_callback_func_t modify_callback) = 0;
		// to a readonly widget
		virtual QWidget* to_dialog() = 0;
		std::string _name;
		std::string _comment;
		json _value;
		const editable_item_type _type;
		const bool _is_container;
		editable_item* parent;
		editable_item(editable_item_type _in_type, bool _in_is_container,
			const std::string& _in_name);
		static std::shared_ptr<editable_item> from_json(const json& data);

	};

	class text_browser : public editable_item
	{
	public:
		friend class editable_item;
		text_browser(const std::string& _in_name, const std::string& _in_value);
		QWidget* to_editor( modify_callback_func_t modify_callback);
		QWidget* to_dialog();
		static std::shared_ptr<text_browser> from_json(const json& data);
	};
	class line_text : public editable_item
	{
		friend class editable_item;
	public:
		using editable_item::editable_item;
		json to_json() const;
		QWidget* to_editor( modify_callback_func_t modify_callback);
		QWidget* to_dialog();
		std::string input_valid() const;
		bool assign(const json& other);
		virtual json str_convert(const QString& input) const = 0;
	};

	class single_line : public line_text
	{
		friend class editable_item;
	public:
		single_line(const std::string& _in_name, const std::string& _in_value);
		json str_convert(const QString& input) const;
		static std::shared_ptr<single_line> from_json(const json& data);
	};

	class multi_line :public line_text
	{
		friend class editable_item;
	public:
		std::string input_valid() const;
		multi_line(const std::string& _in_name, const std::string& _in_value);
		json str_convert(const QString& input) const;
		QWidget* to_editor( modify_callback_func_t modify_callback);
		static std::shared_ptr<multi_line> from_json(const json& data);
	};
	class multi_line_widget : public QPlainTextEdit
	{
		Q_OBJECT

	public:
		std::shared_ptr<multi_line> _editor;
		std::string _editor_id;
		void focusOutEvent(QFocusEvent* e)
		{
			QPlainTextEdit::focusOutEvent(e);
			_editor->_value = toPlainText().toStdString();
		}

	};
	
	class bool_item : public editable_item
	{
		friend class editable_item;
	public:
		bool_item(const std::string& _in_name, bool _in_value);
		QWidget* to_editor( modify_callback_func_t modify_callback);
		QWidget* to_dialog();
		std::string input_valid() const;
		bool assign(const json& other);
		static std::shared_ptr<bool_item> from_json(const json& data);

	};

	class color_item :public editable_item
	{
		friend class editable_item;
	public:
		QWidget* to_editor( modify_callback_func_t modify_callback);
		QWidget* to_dialog();
		color_item(const std::string& _in_name, std::uint32_t _in_rgba);
		std::string input_valid() const;
		bool assign(const json& other);
		static std::shared_ptr<color_item> from_json(const json& data);
	};
	class int_item :public line_text
	{
		friend class editable_item;
	public:
		int_item(const std::string& _in_name, std::int32_t _in_value);
		std::string input_valid() const;
		bool assign(const json& other);
		virtual json str_convert(const QString& input) const;
		static std::shared_ptr<int_item> from_json(const json& data);
		QWidget* int_item::to_editor( modify_callback_func_t modify_callback);
	};

	class float_item :public line_text
	{
		friend class editable_item;
	public:
		float_item(const std::string& _in_name, double _in_value);
		std::string input_valid() const;
		bool assign(const json& other);
		virtual json str_convert(const QString& input) const;
		static std::shared_ptr<float_item> from_json(const json& data);
		QWidget* float_item::to_editor( modify_callback_func_t modify_callback);
	};
	class json_item : public line_text
	{
		friend class editable_item;
	public:
		json_item(const std::string& _in_name, const json& _in_value);
		std::string input_valid() const;
		bool assign(const json& other);
		virtual json str_convert(const QString& input) const;
		static std::shared_ptr<json_item> from_json(const json& data);
		QWidget* json_item::to_editor( modify_callback_func_t modify_callback);
	};
	class choice_item : public editable_item
	{
		friend class editable_item;
	public:
		choice_item(const std::string& _in_name, const std::string& choice_type,
			const std::vector<std::string>& _in_choices,
			const std::vector<std::string>& _in_choice_text,
			const std::string& _in_value);
		std::string current_choice() const;
		std::uint32_t current_index() const;
		std::string input_valid() const;
		bool assign(const json& other);
		QWidget* to_editor( modify_callback_func_t modify_callback);
		QWidget* to_editor_short(modify_callback_func_t modify_callback);
		QWidget* to_editor_long(modify_callback_func_t modify_callback);
		QWidget* to_dialog();
		json to_json() const;
		static std::shared_ptr<choice_item> from_json(const json& data);
		const std::vector<std::string> _choices;
		const std::vector<std::string> _choice_text;
		const std::string _choice_type;
	};


	class list_items : public editable_item
	{
		friend class editable_item;
	public:
		list_items(const std::string& _in_name, const json& _in_item_base);

		std::vector<std::shared_ptr<editable_item>> _children;
		json item_base;
		std::shared_ptr<editable_item> push();
		void pop();
		QWidget* to_editor( modify_callback_func_t modify_callback);
		QWidget* to_dialog();
		static std::shared_ptr<list_items> from_json(const json& data);
		json to_json() const;
		std::string input_valid() const;
	};
	class struct_items : public editable_item
	{
		friend class editable_item;
	public:
		struct_items(const std::string& _in_name);
		std::vector<std::shared_ptr<editable_item>> _children;
		std::shared_ptr<editable_item> find(const std::string& _in_name) const;
		std::shared_ptr<editable_item> push(const json& item_base);
		std::shared_ptr<editable_item> pop(const std::string& _in_name);
		bool empty() const;
		json to_json() const;
		static std::shared_ptr<struct_items> from_json(const json& data);
		QWidget* to_editor( modify_callback_func_t modify_callback);
		QWidget* to_dialog();
		std::string input_valid() const;
	};
}