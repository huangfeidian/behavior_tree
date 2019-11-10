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

	class goto_dialog : public QDialog
	{

		Q_OBJECT

	public:
		goto_dialog(QWidget *parent = 0);

		std::optional<std::uint32_t> run();

	public slots:

		void search();

	private:
		std::string _text;
		QLabel* _label;
		QPushButton* _button;
		QLineEdit* _line_edit;
	};
}