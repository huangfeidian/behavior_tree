#pragma once

#include <qmenu.h>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include "nodes.h"

namespace behavior_tree::editor
{

	class tree_instance;
	class box_outline : public QGraphicsItem
	{
	public:
		box_outline(const QColor& _in_color) :
			QGraphicsItem(),
			_color(_in_color)

		{

		}
		QPainterPath shape() const
		{
			auto cur_path = QPainterPath();
			cur_path.addRect(_rect);
			return cur_path;

		}
		QRectF boundingRect() const
		{
			return _rect;
		}
		void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr)
		{
			auto p = QPen(Qt::black);
			if (isSelected())
			{
				p.setWidth(5);
				painter->setPen(p);
			}
			else
			{
				p.setWidth(1);
				painter->setPen(p);
			}
			painter->setBrush(_color);
			float r = _rect.height() / 8.0;
			painter->drawRoundedRect(_rect, r, r);
		}
		QRectF _rect;
		QColor _color;
	};


	class node_graph :public QObject, public QGraphicsItemGroup
	{
		Q_OBJECT

	public:
		node* _model;
		tree_instance* _manager;
		std::vector<node_graph*> _children;
		box_outline* _outline;
	public:
		std::uint32_t layout_x;
		float layout_y;
		QRectF cur_bounding;
	public:
		node_graph(node* _in_model, tree_instance* _in_manager,
			QColor _text_color);

		QVariant itemChange(GraphicsItemChange change, const QVariant &value);
		QRectF boundingRect() const;
		double get_width() const;
		void set_left_pos(const QPointF& pos);
		QPointF left_pos() const;
		QPointF center_pos() const;
		QPointF right_pos() const;
		void draw_cross(QColor color);
		void draw_collapse_triangle();
		void draw_bound(QColor _color);
		
		void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

		void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
	public slots:
		void set_collapsed();
		void set_un_collapsed();
		void set_color();
		void set_comment();
		void set_editable();
	};

	class tree_layouter
	{
	public:
		tree_layouter(node_graph* in_root);
		std::vector<double> node_widths;
		std::vector<double> total_widths;
		node_graph* root;
		int next_y = -1;
		double y_offset;
		void run();
	private:
		void cacl_node_xy(node_graph* cur_node, int level);
		void set_node_pos(node_graph* cur_node);
		int get_y();

	};
}