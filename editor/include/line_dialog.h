#pragma once
#include <QDialog>
#include <string>
#include <vector>
#include <utility>
#include <optional>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qboxlayout.h>
#include <optional>

namespace behavior_tree::editor
{

	class line_dialog : public QDialog
	{

		Q_OBJECT

	public:
		line_dialog(std::string title, std::string init_text, QWidget* parent = 0);

		std::string run();

	public slots:

		void confirm();
		void temp_confirm();

	private:
		std::string _text;
		QLabel* _label;
		QPushButton* _button;
		QLineEdit* _line_edit;
	};
}