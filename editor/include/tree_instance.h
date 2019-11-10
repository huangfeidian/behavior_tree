#pragma once
#include "mainwindow.h"
#include <filesystem>



namespace behavior_tree::editor
{
	const static int node_unit = 10;
	class node;
	class tree_instance;
	class tree_view :public QGraphicsView
	{
	public:
		tree_view(QGraphicsScene* scene, tree_instance* in_graph_mgr);
		void wheelEvent(QWheelEvent* e);
		void keyPressEvent(QKeyEvent* e);
		void closeEvent(QCloseEvent* e);
		QTransform cur_trans;
		tree_instance* _graph_mgr;
	};
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


	class node_graph :public QGraphicsItemGroup
	{
	public:
		node* _model;
		tree_instance* _manager;

		std::vector<node_graph*> _children;
	public:
		std::uint32_t layout_x;
		float layout_y;
	public:
		node_graph(node* _in_model, tree_instance* _in_manager,
			QColor _text_color);

		QVariant itemChange(GraphicsItemChange change, const QVariant &value);

		double get_width() const;
		void set_left_pos(const QPointF& pos);
		QPointF left_pos() const;
		QPointF center_pos() const;
		QPointF right_pos() const;
		void draw_cross(QColor color);
		void draw_bound(QColor _color);
		void set_collapsed();
		void set_un_collapsed();

		void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);


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
	class tree_instance
	{
	public:
		static const std::uint32_t area_width = 1080;
		static const std::uint32_t area_height = 1920;
		tree_instance(const std::string& in_file_path, node* in_root, MainWindow* _in_main);
		QGraphicsScene* _scene;
		tree_view* _view;
		node* _root;
		node_graph* _graph_root = nullptr;
		node* selected_node = nullptr;
		MainWindow* parent;
		QMdiSubWindow* window;
		std::filesystem::path file_path;
		std::filesystem::path file_name;
		std::uint32_t node_seq_idx = 1; // next node seq to use
		std::string agent_type;
		bool modified = true;
		void set_dirty();
		void set_modfied(bool flag);
		void close_handler(QCloseEvent* e);
		void focus_on(std::size_t node_idx);
		void focus_on(const node_graph* cur_node);
		void focus_on(node* cur_node);
		void update_title();
	public:
		void display_tree();
		void _display_links_impl(node_graph* cur_node);
		node_graph* _build_tree_impl(node* cur_node);
		node_graph* find_graph_by_node(node_graph* cur_graph, node* cur_node) const;
		node_graph* find_graph_by_idx(node_graph* cur_graph, std::uint32_t idx) const;
		node* find_node_by_idx(std::uint32_t idx);
		void refresh();
		void cancel_select();
		void select_changed(node_graph* cur_node, int state);
		void show_select_effect();
		void clean_select_effect();
		void insert_handler();
		void delete_handler();
		node* copy_handler();
		void paste_handler(node* cur_node);
		node* cut_handler();
		void move_handler(bool is_up);
		std::uint32_t next_node_seq();
		std::string save_handler();
		void deactivate_handler();
		void activate_handler();
		void set_scene_background();
		void save_to_svg();
		void save_to_png();
		void search_node();
	};
}