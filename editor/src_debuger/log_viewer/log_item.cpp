#include "log_item.h"

using namespace std;
using namespace spiritsaway::behavior_tree::editor;

log_item::log_item(const QVector<QVariant> &data, log_item *parent)
    : itemData(data),
      parentItem(parent)
{}
//! [0]

//! [1]
log_item::~log_item()
{
    qDeleteAll(childItems);
}
//! [1]

//! [2]
log_item *log_item::child(int number)
{
    if (number < 0 || number >= childItems.size())
        return nullptr;
    return childItems.at(number);
}
//! [2]

//! [3]
int log_item::childCount() const
{
    return childItems.count();
}
//! [3]

//! [4]
int log_item::childNumber() const
{
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<log_item*>(this));
    return 0;
}
//! [4]

//! [5]
int log_item::columnCount() const
{
    return itemData.count();
}
//! [5]

//! [6]
QVariant log_item::data(int column) const
{
    if (column < 0 || column >= itemData.size())
        return QVariant();
    return itemData.at(column);
}
//! [6]

//! [7]
bool log_item::insertChildren(int position, int count, int columns)
{
    if (position < 0 || position > childItems.size())
        return false;

    for (int row = 0; row < count; ++row) {
        QVector<QVariant> data(columns);
        log_item *item = new log_item(data, this);
        childItems.insert(position, item);
    }

    return true;
}
//! [7]

//! [8]
bool log_item::insertColumns(int position, int columns)
{
    if (position < 0 || position > itemData.size())
        return false;

    for (int column = 0; column < columns; ++column)
        itemData.insert(position, QVariant());

    for (log_item *child : qAsConst(childItems))
        child->insertColumns(position, columns);

    return true;
}
//! [8]

//! [9]
log_item *log_item::parent()
{
    return parentItem;
}
//! [9]

//! [10]
bool log_item::removeChildren(int position, int count)
{
    if (position < 0 || position + count > childItems.size())
        return false;

    for (int row = 0; row < count; ++row)
        delete childItems.takeAt(position);

    return true;
}
//! [10]

bool log_item::removeColumns(int position, int columns)
{
    if (position < 0 || position + columns > itemData.size())
        return false;

    for (int column = 0; column < columns; ++column)
        itemData.remove(position);

    for (log_item *child : qAsConst(childItems))
        child->removeColumns(position, columns);

    return true;
}

//! [11]
bool log_item::setData(int column, const QVariant &value)
{
    if (column < 0 || column >= itemData.size())
        return false;

    itemData[column] = value;
    return true;
}
//! [11]