

#include "log_tree_model.h"
#include "log_item.h"

#include <QtWidgets>

using namespace  spiritsaway::behavior_tree::editor;
//! [0]
log_tree_model::log_tree_model(const std::vector<std::pair<QString, Qt::ItemFlags>>& headers, QObject *parent)
    : QAbstractItemModel(parent)
{
    QVector<QVariant> rootData;
	for (const auto &header : headers)
	{
		rootData << header.first;
		header_forbid_flag.push_back(header.second);
	}

    rootItem = new log_item(rootData);
}
//! [0]

//! [1]
log_tree_model::~log_tree_model()
{
    delete rootItem;
}
//! [1]

//! [2]
int log_tree_model::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return rootItem->columnCount();
}
//! [2]

QVariant log_tree_model::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    log_item *item = getItem(index);

    return item->data(index.column());
}

//! [3]
Qt::ItemFlags log_tree_model::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return header_forbid_flag[index.column()] ^ QAbstractItemModel::flags(index);
}
//! [3]

//! [4]
log_item *log_tree_model::getItem(const QModelIndex &index) const
{
    if (index.isValid()) {
        log_item *item = static_cast<log_item*>(index.internalPointer());
        if (item)
            return item;
    }
    return rootItem;
}
//! [4]

QVariant log_tree_model::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}

//! [5]
QModelIndex log_tree_model::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();
//! [5]

//! [6]
    log_item *parentItem = getItem(parent);
    if (!parentItem)
        return QModelIndex();

    log_item *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    return QModelIndex();
}
//! [6]


bool log_tree_model::insertRows(int position, int rows, const QModelIndex &parent)
{
    log_item *parentItem = getItem(parent);
    if (!parentItem)
        return false;

    beginInsertRows(parent, position, position + rows - 1);
    const bool success = parentItem->insertChildren(position,
                                                    rows,
                                                    rootItem->columnCount());
    endInsertRows();

    return success;
}

//! [7]
QModelIndex log_tree_model::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    log_item *childItem = getItem(index);
    log_item *parentItem = childItem ? childItem->parent() : nullptr;

    if (parentItem == rootItem || !parentItem)
        return QModelIndex();

    return createIndex(parentItem->childNumber(), 0, parentItem);
}
//! [7]



bool log_tree_model::removeRows(int position, int rows, const QModelIndex &parent)
{
    log_item *parentItem = getItem(parent);
    if (!parentItem)
        return false;

    beginRemoveRows(parent, position, position + rows - 1);
    const bool success = parentItem->removeChildren(position, rows);
    endRemoveRows();

    return success;
}

//! [8]
int log_tree_model::rowCount(const QModelIndex &parent) const
{
    const log_item *parentItem = getItem(parent);

    return parentItem ? parentItem->childCount() : 0;
}
//! [8]

bool log_tree_model::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole)
        return false;

    log_item *item = getItem(index);
    bool result = item->setData(index.column(), value);

    if (result)
        emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});

    return result;
}

bool log_tree_model::setHeaderData(int section, Qt::Orientation orientation,
                              const QVariant &value, int role)
{
    if (role != Qt::EditRole || orientation != Qt::Horizontal)
        return false;

    const bool result = rootItem->setData(section, value);

    if (result)
        emit headerDataChanged(orientation, section, section);

    return result;
}

bool log_tree_model::appendRow(const std::vector<QVariant>& row_data, const QModelIndex& parent)
{
	log_item *parentItem = getItem(parent);
	if (!parentItem)
	{
		return false;
	}
	insertRows(parentItem->childCount(), 1, parent);

	auto cur_row = parentItem->child(parentItem->childCount() - 1);
	for (std::size_t column = 0; column < row_data.size() && column < rootItem->columnCount(); column++)
	{
		cur_row->setData(column, row_data[column]);
	}
	return true;
}
