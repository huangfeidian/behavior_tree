#pragma once

#include <QVariant>
#include <QVector>

//! [0]
namespace spiritsaway::behavior_tree::editor
{
    class log_item
{
public:
    explicit log_item(const QVector<QVariant> &data, log_item *parent = nullptr);
    ~log_item();

    log_item *child(int number);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    bool insertChildren(int position, int count, int columns);
    bool insertColumns(int position, int columns);
    log_item *parent();
    bool removeChildren(int position, int count);
    bool removeColumns(int position, int columns);
    int childNumber() const;
    bool setData(int column, const QVariant &value);

private:
    QVector<log_item*> childItems;
    QVector<QVariant> itemData;
    log_item *parentItem;
};
}
