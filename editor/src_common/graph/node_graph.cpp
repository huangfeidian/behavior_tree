

#include <qcolordialog.h>
#include <qgraphicssceneevent.h>
#include <graph/node_graph.h>
#include <graph/tree_instance.h>
#include <btree/nodes.h>
#include <dialogs/info_dialog.h>
#include <dialogs/editable_dialog.h>
#include <dialogs/line_dialog.h>



using namespace spiritsaway::behavior_tree::editor;



node_graph::node_graph(node* _in_model, tree_instance* _in_manager,
	QColor _text_color) :
	_model(_in_model),
	_manager(_in_manager)
{
	_outline = new box_outline(color_from_uint(_in_model->color));
	//std::cout << "node graph fill with color " << _in_model->color << std::endl;
	setFlag(QGraphicsItem::ItemIsSelectable, true);
	setFlag(QGraphicsItem::ItemIsFocusable, true);
	QGraphicsSimpleTextItem* _text = new QGraphicsSimpleTextItem(
		QString::fromStdString(_model->display_text()));
	_text->setBrush(_text_color);
	auto text_bound = _text->boundingRect();


	auto total_width = text_bound.width();

	auto total_radius = QPointF(total_width / 2, text_bound.height() / 2);
	
	auto text_center = _text->pos() + total_radius;
	_text->setPos(-1 * text_center);
	addToGroup(_text);

	auto empty_space = QPointF(node_unit, node_unit);
	auto new_r = empty_space + total_radius;
	cur_bounding = QRectF(-new_r, new_r);
	_outline->_rect = cur_bounding;
	addToGroup(_outline);
	_text->setZValue(1.0);
	_outline->setZValue(0.0);
}
QVariant node_graph::itemChange(GraphicsItemChange change, const QVariant &value)
{
	if (change == QGraphicsItem::ItemSelectedChange)
	{
		_manager->select_changed(this, value.toInt());
		return value;
	}
	else
	{
		return QGraphicsItemGroup::itemChange(change, value);
	}

}

QRectF node_graph::boundingRect() const
{
	return cur_bounding;
}
tree_layouter::tree_layouter(node_graph* in_root) :
	root(in_root)
{

}
void tree_layouter::run()
{
	//first get the y value of nodes;
	cacl_node_xy(root, 0);
	total_widths = std::vector<double>(node_widths.size() + 1, 0);
	double pre = 0;
	for (std::size_t i = 0; i < node_widths.size(); i++)
	{
		pre = pre + node_widths[i];
		total_widths[i + 1] = pre;
	}
	set_node_pos(root);


}
int tree_layouter::get_y()
{
	return ++next_y;
}
void tree_layouter::cacl_node_xy(node_graph* cur_node, int level)
{
	if (level == node_widths.size())
	{
		node_widths.push_back(-1);
	}
	auto cur_w = cur_node->get_width();
	//cur_node->_manager->parent->_logger->debug("node {} cur_w {}",
	//	cur_node->_model->_idx, cur_w);
	if (cur_w > node_widths[level])
	{
		node_widths[level] = cur_w;
	}
	cur_node->layout_x = level;
	if (cur_node->_children.empty())
	{
		cur_node->layout_y = get_y();
		return;
	}
	for (auto one_node : cur_node->_children)
	{
		cacl_node_xy(one_node, level + 1);
	}
	cur_node->layout_y = (cur_node->_children[0]->layout_y + cur_node->_children.back()->layout_y) * 0.5;

}
void tree_layouter::set_node_pos(node_graph* cur_node)
{

	double x = total_widths[cur_node->layout_x] + cur_node->layout_x * 4 * node_unit;
	double y = node_unit * 5 * (cur_node->layout_y - root->layout_y);
	//cur_node->_manager->parent->_logger->debug("node {} layout x:{} y:{}, pos x:{} y:{}",
	//cur_node->_model->_idx, cur_node->layout_x, cur_node->layout_y, x, y);
	auto cur_p = QPointF(x, y);
	cur_node->set_left_pos(cur_p);
	for (auto one_node : cur_node->_children)
	{
		set_node_pos(one_node);
	}
}
double node_graph::get_width() const
{
	return boundingRect().width();
}
void node_graph::set_left_pos(const QPointF& pos)
{
	auto new_pos = pos + QPoint(get_width() / 2.0, 0);
	setPos(new_pos);
}
QPointF node_graph::left_pos() const
{
	return pos() - QPointF(get_width() / 2.0, 0);
}
QPointF node_graph::right_pos() const
{
	return pos() + QPointF(get_width() / 2.0, 0);
}
QPointF node_graph::center_pos() const
{
	return pos();
}
void node_graph::draw_cross(QColor color)
{
	auto p = QPen(color);
	auto width = get_width();
	auto height = boundingRect().height();
	auto left_upper = pos() + QPointF(width * -0.5, height * 0.5);
	auto left_down = pos() + QPointF(width * -0.5, height * -0.5);
	auto right_upper = pos() + QPointF(width * 0.5, height * 0.5);
	auto right_down = pos() + QPointF(width * 0.5, height * -0.5);
	auto l1 = _manager->_scene->addLine(QLineF(left_upper, right_down), p);
	auto l2 = _manager->_scene->addLine(QLineF(left_down, right_upper), p);
	l1->setZValue(1.0);
	l2->setZValue(1.0);
}

void node_graph::draw_collapse_triangle()
{
	auto p = QPen(Qt::yellow);
	auto b = QBrush(Qt::magenta);
	b.setStyle(Qt::SolidPattern);
	QPointF begin_pos = right_pos();
	begin_pos += QPoint(node_unit, 0);
	QPolygonF polygon;
	auto height = boundingRect().height() / 2;
	auto width = node_unit;
	auto point_1 = QPointF(0, -1 * height);
	auto point_2 = QPointF(width, 0);
	auto point_3 = QPointF(0, height);
	polygon << point_1 << point_2 << point_3<<point_1;
	polygon.translate(begin_pos);
	auto cur_p = _manager->_scene->addPolygon(polygon, p, b);
	cur_p->setZValue(1.0);

}

void node_graph::draw_bound(QColor color)
{
	auto p = QPen(color);
	p.setWidth(3);
	auto b = QBrush(color, Qt::NoBrush);
	auto width = get_width();
	auto height = boundingRect().height();
	auto left_down = pos() + QPointF(width * -0.5, height * -0.5);
	selected_effect = _manager->_scene->addRect(left_down.x(), left_down.y(), width, height, p, b);
	_manager->_logger->info("bound is {}, {}, {}, {}", left_down.x(), left_down.y(), width, height);
}
void node_graph::set_collapsed()
{
	_model->_is_collapsed = true;
	_manager->refresh();
}
void node_graph::set_un_collapsed()
{
	_model->_is_collapsed = false;
	_manager->refresh();
}
void node_graph::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
	_manager->_logger->debug("node {} mouseDoubleClickEvent", _model->_idx);
	if (!_model->_show_widget->empty())
	{
		set_editable();
		return;
	}
	if (_model->can_collapse())
	{
		if (_model->_is_collapsed)
		{
			set_un_collapsed();
		}
		else
		{
			set_collapsed();
		}
		return;
	}
	
	
}
void node_graph::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
	auto node_menu = new QMenu();
	auto action_color = node_menu->addAction("Color");
	QObject::connect(action_color, &QAction::triggered, this, &node_graph::set_color);
	// action_color->triggered.connect(set_color);
	if (_model->can_collapse())
	{
		if (_model->_is_collapsed)
		{
			auto action_expand = node_menu->addAction("Expand");
			QObject::connect(action_expand, &QAction::triggered, this, &node_graph::set_un_collapsed);
			//action_expand->triggered.connect(set_un_collapsed);
		}
		else
		{
			auto action_collapsed = node_menu->addAction("Collapse");
			QObject::connect(action_collapsed, &QAction::triggered, this, &node_graph::set_collapsed);
			//action_collapsed->triggered.connect(set_collapsed);
		}
	}
	auto action_comment = node_menu->addAction("Comment");
	QObject::connect(action_comment, &QAction::triggered, this, &node_graph::set_comment);

	if (!_model->_show_widget->empty())
	{
		auto action_content = node_menu->addAction("Content");
		QObject::connect(action_content, &QAction::triggered, this, &node_graph::set_editable);
	}
	//node_menu->move(_manager->_view->mapFromScene(center_pos().x() + event->pos().x(), center_pos().y() + event->pos().y()));

	node_menu->move(_manager->_view->mapFromScene(center_pos().x() - event->pos().x(), center_pos().y() - event->pos().y()));
	node_menu->show();
}
void node_graph::set_color()
{
	QColor cur_color = QColorDialog::getColor(Qt::white, _manager->window);
	if (cur_color.isValid())
	{
		std::uint8_t r = cur_color.red();
		std::uint8_t g = cur_color.green();
		std::uint8_t b = cur_color.black();
		std::uint8_t a = cur_color.alpha();
		std::uint32_t final_value = (r << 24) + (g << 16) + (b << 8) + a;
		//std::cout << fmt::format("color_item set with color ({}, {}, {}, {} final {})",
		//	 r,g,b,a, final_value) << std::endl;

		_model->color = final_value;
		_outline->_color = color_from_uint(final_value);
		_outline->update();
		_manager->set_dirty();
	}
}

void node_graph::set_outline_color(QColor _color)
{
	_outline->_color = _color;
	_outline->update();
}
void node_graph::set_comment()
{
	auto comment_dialog = new line_dialog("change comment", _model->comment, _manager->window);
	auto new_comment = comment_dialog->run();
	_model->comment = new_comment;
	_manager->set_dirty();
	_manager->refresh();
}
void node_graph::set_editable()
{
	if (_manager->parent->is_read_only())
	{
		auto cur_info_dialog = new info_dialog(_manager->window, _model);
		cur_info_dialog->run();
		return;
	}
	auto edit_dialog = new editable_dialog(_manager->window, _model);
	edit_dialog->run();
	_manager->set_dirty();
	_manager->refresh();
	return;
}