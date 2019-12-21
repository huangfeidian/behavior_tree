
#include <deque>
#include <fstream>
#include <iostream>
#include <ctime>

#include <qpainter.h>
#include <qpainterpath.h>
#include <qimage.h>


//#include <QGraphicsSvgItem>
//#include <QSvgGenerator>
//#include <QSvgRenderer>

#include <tree_instance.h>
#include <nodes.h>
#include <search_select_dialog.h>
#include <choice_manager.h>
#include <editable_item.h>


using namespace behavior_tree::editor;

tree_view::tree_view(QGraphicsScene* in_scene, tree_instance* in_graph_mgr)
	: QGraphicsView(in_scene)
	, _graph_mgr(in_graph_mgr)
{
	setRenderHints(QPainter::Antialiasing);
	setTransformationAnchor(QGraphicsView::NoAnchor);
	setDragMode(QGraphicsView::ScrollHandDrag);
}
void tree_view::wheelEvent(QWheelEvent* e)
{
	double step = 1.2;
	auto d = e->angleDelta();
	if (d.y() > 0)
	{
		scale(step, step);
	}
	else
	{
		scale(1 / step, 1 / step);
	}
	cur_trans = transform();
}
void tree_view::closeEvent(QCloseEvent* e)
{
	_graph_mgr->close_handler(e);
}
void tree_view::keyPressEvent(QKeyEvent* e)
{
	auto k = e->key();
	if (k == Qt::Key::Key_1)
	{
		update();
	}
	QGraphicsView::keyPressEvent(e);
}

void tree_instance::set_dirty()
{
	set_modfied(true);
}
void tree_instance::set_modfied(bool flag)
{
	modified = flag;
	window->setWindowModified(flag);
}
void tree_instance::update_title()
{
	std::string new_title_name = file_name.string() + "[*]";
	window->setWindowTitle(QString(new_title_name.c_str()));
}
void tree_instance::close_handler(QCloseEvent* e)
{
	if (modified)
	{
		QMessageBox::StandardButton  defaultBtn = QMessageBox::NoButton; //缺省按钮
		QMessageBox::StandardButton result;//返回选择的按钮
		result = QMessageBox::question(_view, QString("Confirm"),
			QString("close without saving"), QMessageBox::Yes|QMessageBox::No, defaultBtn);
		if (result == QMessageBox::No)
		{
			e->ignore();
			return;
		}
	}
	parent->remove_instance(this);
}
void tree_instance::focus_on(std::size_t node_idx)
{
	auto cur_node = find_node_by_idx(node_idx);
	if (cur_node == nullptr)
	{
		auto notify_info = fmt::format("cant find node with idx {}", node_idx);
		QMessageBox::about(_view, QString("Error"),
			QString::fromStdString(notify_info));
	}
	else
	{
		focus_on(cur_node);
	}
}
void tree_instance::focus_on(node* cur_node)
{
	auto temp_node = cur_node;
	while (temp_node->_parent)
	{
		temp_node->_is_collapsed = false;
		temp_node = temp_node->_parent;
	}
	refresh();
	auto cur_graph_node = find_graph_by_node(_graph_root, cur_node);
	focus_on(cur_graph_node);
	cancel_select();
	selected_node = cur_node;
	show_select_effect();
}
void tree_instance::focus_on(const node_graph* cur_node)
{
	if (!cur_node)
	{
		return;
	}
	/*auto scene_pos = cur_node->mapToScene(0, 0);
	auto view_pos = _view->mapFromScene(scene_pos);*/
	_view->ensureVisible(cur_node, 50);
	return;
}


tree_instance::tree_instance(const std::string& in_file_path, 
	node* in_root, MainWindow* _in_main)
	: _root(in_root)
	, parent(_in_main)
	, _scene(new QGraphicsScene())
	, _view(new tree_view(_scene, this))
	, file_path(in_file_path)
	, file_name(file_path.filename())
{
	_view->setAttribute(Qt::WA_DeleteOnClose);
	set_scene_background();
	window = parent->cur_mdi->addSubWindow(_view);
	window->setWindowTitle(QString::fromStdString(file_name.string()));
	std::deque<const node*>	all_nodes;
	all_nodes.push_back(in_root);
	while (!all_nodes.empty())
	{
		auto cur_node = all_nodes.front();
		all_nodes.pop_front();
		node_seq_idx = std::max(node_seq_idx, cur_node->_idx + 1);
		for (const auto one_child_node : cur_node->_children)
		{
			all_nodes.push_back(one_child_node);
		}
	}
	parent->add_instance(this);
	window->show();
}
std::uint32_t tree_instance::next_node_seq()
{
	return node_seq_idx++;
}

void tree_instance::display_tree()
{
	parent->_logger->debug("tree_instance display tree {}", file_name.string());
	update_title();
	_graph_root = _build_tree_impl(_root);
	auto layouter = tree_layouter(_graph_root);
	layouter.run();
	_display_links_impl(_graph_root);

}

node_graph* tree_instance::_build_tree_impl(node* cur_node)
{
	//parent->_logger->debug("tree_instance _build_tree_impl  {} for node {}", 
	//	file_name.string(), cur_node->_idx);
	auto new_node =  new node_graph(cur_node, this, QColor(Qt::black));
	if (!cur_node->_is_collapsed)
	{
		new_node->_children.reserve(cur_node->_children.size());
		for (auto one_child : cur_node->_children)
		{
			auto temp_child = _build_tree_impl(one_child);
			new_node->_children.push_back(temp_child);
		}
	}
	_scene->addItem(new_node);
	return new_node;
}
void tree_instance::_display_links_impl(node_graph* cur_node)
{
	
	auto s_pos = cur_node->right_pos();
	//parent->_logger->debug("tree_instance _display_links_impl  {} for node {} at x:{} y:{}",
	//	file_name.string(), cur_node->_model->_idx, s_pos.x(), s_pos.y());
	QPen pen;
	pen.setColor(Qt::black);
	pen.setWidth(1);
	pen.setStyle(Qt::SolidLine);
	auto b = QBrush(Qt::black, Qt::NoBrush);
	for (auto one_child : cur_node->_children)
	{
		auto d_pos = one_child->left_pos();
		QPainterPath path(s_pos);

		QPoint c1((s_pos + d_pos).x() / 2, s_pos.y());
		QPoint c2((s_pos + d_pos).x() / 2, d_pos.y());

		path.cubicTo(c1, c2, d_pos);

		auto cur_path = _scene->addPath(path, pen, b);
		cur_path->setZValue(1.0);
		//auto new_line = _scene->addLine(QLineF(s_pos, d_pos));
		//new_line->setZValue(1.0);
		_display_links_impl(one_child);
	}
	if (cur_node->_model->_is_collapsed)
	{
		cur_node->draw_cross(Qt::magenta);
	}
}


void tree_instance::refresh()
{
	parent->_logger->debug("tree_instance {} refresh ", file_name.string());
	set_dirty();
	clean_select_effect();
	_scene->clear();
	display_tree();
	if (selected_node)
	{
		auto cur_node = find_graph_by_node(_graph_root, selected_node);
		if (cur_node)
		{
			cur_node->setSelected(true);
		}
	}
	
}
void tree_instance::cancel_select()
{
	parent->_logger->debug("tree_instance {} cancel_select ", file_name.string());
	if (!selected_node)
	{
		return;
	}
	auto cur_node = find_graph_by_node(_graph_root, selected_node);
	cur_node->setSelected(false);
}
void tree_instance::select_changed(node_graph* cur_node, int state)
{
	parent->_logger->debug("tree_instance {} select_changed cur_node {} state {} ",
		file_name.string(), cur_node->_model->_idx, state);
	if (state == 0)
	{
		selected_node = nullptr;
		clean_select_effect();
	}
	else
	{
		selected_node = cur_node->_model;
		show_select_effect();
	}
}
void tree_instance::show_select_effect()
{
	parent->_logger->debug("tree_instance {} show_select_effect node {} ",
		file_name.string(), selected_node->_idx);
}
void tree_instance::clean_select_effect()
{
	parent->_logger->debug("tree_instance {} clean_select_effect ",
		file_name.string());
}
void tree_instance::insert_handler()
{
	parent->_logger->debug("tree_instance {} insert_handler begin",
		file_name.string());
	if (!selected_node)
	{
		return;
	}
	auto node_type_choices = choice_manager::instance().get_choice("node_type");
	auto cur_dialog = new search_select_dialog(*node_type_choices.first, parent);
	auto cur_choice = cur_dialog->run();
	if (cur_choice.empty())
	{
		return;
	}
	auto cur_type_opt = magic_enum::enum_cast<node_type>(cur_choice);
	if (!cur_type_opt.has_value())
	{
		return;
	}
	auto cur_type = cur_type_opt.value();
	if (cur_type == node_type::invalid)
	{
		return;
	}
	auto new_idx = next_node_seq();
	parent->_logger->debug("tree_instance {} get  new node idx {}",
		file_name.string(), new_idx);
	auto cur_new_node = node::default_node_by_type(cur_type, new_idx, selected_node);
	cur_new_node->refresh_editable_items();
	parent->_logger->debug("tree_instance {} create new node",
		file_name.string());
	selected_node->add_child(cur_new_node);
	refresh();
	parent->_logger->debug("tree_instance {} insert_handler finish",
		file_name.string());

}
void tree_instance::delete_handler()
{
	parent->_logger->debug("tree_instance {} delete_handler ",
		file_name.string());
	if (!selected_node)
	{
		return;
	}
	if (selected_node->_type == node_type::root)
	{
		return;
	}
	selected_node->_parent->remove_child(selected_node);
	selected_node->destroy();
	selected_node = nullptr;
	refresh();
}
node* tree_instance::copy_handler()
{
	parent->_logger->debug("tree_instance {} copy_handler ",
		file_name.string());
	if (!selected_node)
	{
		return nullptr;
	}
	auto result = selected_node->clone_recursive(nullptr);
	
	return result;

}
void tree_instance::paste_handler(node* cur_node)
{
	
	if (!selected_node)
	{
		return;
	}
	if (!cur_node)
	{
		return;
	}
	std::deque<node*> all_nodes;
	all_nodes.push_back(cur_node);
	while (!all_nodes.empty())
	{
		auto cur_node = all_nodes.front();
		all_nodes.pop_front();
		cur_node->_idx = next_node_seq();
		for (auto one_child : cur_node->_children)
		{
			all_nodes.push_back(one_child);
		}
	}
	parent->_logger->debug("tree_instance {} paste_handler parent {} child {}",
		file_name.string(), selected_node->_idx, cur_node->_idx);
	selected_node->add_child(cur_node);
	refresh();
}
node* tree_instance::cut_handler()
{
	parent->_logger->debug("tree_instance {} cut_handler ",
		file_name.string());
	if (!selected_node)
	{
		return nullptr;
	}
	if (selected_node->_type == node_type::root)
	{
		return nullptr;
	}
	selected_node->_parent->remove_child(selected_node);
	auto result = selected_node;
	selected_node = nullptr;
	refresh();
	return result;
}
void tree_instance::move_handler(bool is_up)
{
	parent->_logger->debug("tree_instance {} move_handler is_up {} ",
		file_name.string(), is_up);
	if (!selected_node)
	{
		return;
	}
	selected_node->_parent->move_child(selected_node, is_up);
	refresh();
}
node_graph* tree_instance::find_graph_by_node(node_graph* cur_graph, node* cur_node) const
{
	parent->_logger->debug("tree_instance {} find_graph_by_node cur_graph {} cur_node {} ",
		file_name.string(), cur_graph->_model->_idx, cur_node->_idx);
	if (cur_graph->_model == cur_node)
	{
		return cur_graph;
	}
	for (auto one_child : cur_graph->_children)
	{
		auto temp_result = find_graph_by_node(one_child, cur_node);
		if (temp_result)
		{
			return temp_result;
		}
	}
	return nullptr;
}
node_graph* tree_instance::find_graph_by_idx(node_graph* cur_graph, std::uint32_t cur_idx) const
{
	parent->_logger->debug("tree_instance {} find_graph_by_idx cur_graph {} cur_node {} ",
		file_name.string(), cur_graph->_model->_idx, cur_idx);
	if (cur_graph->_model->_idx == cur_idx)
	{
		return cur_graph;
	}
	for (auto one_child : cur_graph->_children)
	{
		auto temp_result = find_graph_by_idx(one_child, cur_idx);
		if (temp_result)
		{
			return temp_result;
		}
	}
	return nullptr;
}
node* tree_instance::find_node_by_idx(std::uint32_t idx)
{
	std::deque<node*> temp_nodes;
	temp_nodes.push_back(_root);
	while (!temp_nodes.empty())
	{
		auto cur_front = temp_nodes.front();
		temp_nodes.pop_front();
		if (cur_front->_idx == idx)
		{
			return cur_front;
		}
		else
		{
			for (auto onde_child : cur_front->_children)
			{
				temp_nodes.push_back(onde_child);
			}
		}
	}
	return nullptr;
}
std::string tree_instance::save_handler()
{
	parent->_logger->debug("tree_instance {} save_handler",
		file_name.string());
	if (!modified)
	{
		return "";
	}
	node_seq_idx = 0;
	std::deque<node*> all_nodes;
	std::vector<node*> indexed_nodes;
	indexed_nodes.reserve(20);
	all_nodes.push_back(_root);
	
	while (!all_nodes.empty())
	{
		auto cur_node = all_nodes.front();
		all_nodes.pop_front();
		cur_node->_idx = next_node_seq();
		indexed_nodes.push_back(cur_node);
		for (auto one_child : cur_node->_children)
		{
			all_nodes.push_back(one_child);
		}
	}
	auto invalid_info = _root->check_valid();
	if (invalid_info.empty())
	{
		json::array_t nodes_info;
		for (const auto one_node : indexed_nodes)
		{
			nodes_info.push_back(one_node->to_json());
		}
		json::object_t dump_json;
		dump_json["agent_name"] = dynamic_cast<root_node*>(_root)->agent_name;
		dump_json["nodes"] = nodes_info;
		dump_json["extra"] = json::object_t();
		dump_json["name"] = file_name.string();


		time_t rawtime;
		struct tm * timeinfo;
		char buffer[80];

		time(&rawtime);
		timeinfo = localtime(&rawtime);

		strftime(buffer, sizeof(buffer), "%d-%m-%Y %H:%M:%S", timeinfo);
		
		dump_json["signature"] = std::string(buffer);
		std::ofstream output_file(file_path);
		output_file << std::setw(4) << dump_json << std::endl;
		output_file.close();
		save_to_svg();
		save_to_png();
	}
	else
	{
		std::string notify_info = fmt::format("fail to save {} error is {}",
			file_name.string(), invalid_info);
		QMessageBox::about(_view, QString("Error"),
			QString::fromStdString(notify_info));
	}
	return invalid_info;
}

void tree_instance::deactivate_handler()
{
	parent->_logger->debug("tree_instance {} deactivate_handler ",
		file_name.string());
	return;
}
void tree_instance::activate_handler()
{
	parent->_logger->debug("tree_instance {} activate_handler ",
		file_name.string());
	return;
}

void tree_instance::set_scene_background()
{
	// 绘制网格北京
	QPolygonF myPolygon1;
	myPolygon1 << QPointF(0, 10) << QPointF(20, 10);
	QPolygonF myPolygon2;
	myPolygon2 << QPointF(10, 0) << QPointF(10, 20);
	QPixmap pixmap(20, 20);
	pixmap.fill(Qt::lightGray);
	QPainter painter(&pixmap);

	QVector<qreal> dashes;//line style--虚线  
	qreal space = 2;
	dashes << 2 << space << 2 << space;
	QPen pen(Qt::gray, 1);
	pen.setDashPattern(dashes);
	pen.setWidth(1);

	painter.setPen(pen);
	painter.translate(0, 0);
	painter.drawPolyline(myPolygon1);
	painter.drawPolyline(myPolygon2);
	_scene->setBackgroundBrush(pixmap);
}

void tree_instance::save_to_svg()
{
	return;
	//QSvgGenerator svgGen;
	//auto svg_path = file_path;
	//svg_path.replace_extension("svg");
	//svgGen.setFileName(QString::fromStdString(svg_path.string()));
	//std::uint32_t width = _scene->width() / 1.25;
	//std::uint32_t height = _scene->height() / 1.25;
	//svgGen.setSize(QSize(width, height));
	//svgGen.setViewBox(QRect(0, 0, width, height));
	//svgGen.setTitle(QObject::tr("SVG Generator Example Drawing"));
	//svgGen.setDescription(QObject::tr("An SVG drawing created by the SVG Generator "
	//	"Example provided with Qt."));

	//QPainter painter(&svgGen);
	//_scene->render(&painter);
}
void tree_instance::save_to_png()
{
	QImage image(_scene->width(), _scene->height() , QImage::Format_RGB32);
	QPainter painter(&image);
	_scene->render(&painter);
	auto png_path = file_path;
	png_path.replace_extension("png");
	image.save(QString::fromStdString(png_path.string()));
}

void tree_instance::search_node()
{
	std::vector<node*> all_nodes;
	std::deque<node*> queue_nodes;
	queue_nodes.push_back(_root);
	while (!queue_nodes.empty())
	{
		auto temp_node = queue_nodes.front();
		queue_nodes.pop_front();
		all_nodes.push_back(temp_node);
		for (auto one_child : temp_node->_children)
		{
			queue_nodes.push_back(one_child);
		}
	}
	std::vector<std::string> node_texts;
	for (const auto one_node : all_nodes)
	{
		node_texts.push_back(one_node->display_text());
	}
	auto cur_search_dialog = new search_select_dialog(node_texts, window);
	auto cur_choice = cur_search_dialog->run();
	if (cur_choice.empty())
	{
		return;
	}
	for (int i = 0; i < node_texts.size(); i++)
	{
		if (node_texts[i] == cur_choice)
		{
			focus_on(all_nodes[i]);
			return;
		}
	}

}
