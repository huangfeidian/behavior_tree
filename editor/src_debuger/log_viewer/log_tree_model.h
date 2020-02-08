#pragma once
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

#include <vector>
#include <utility>

namespace spiritsaway::behavior_tree::editor
{
    class log_item;

//! [0]
    class log_tree_model : public QAbstractItemModel
    {
        Q_OBJECT

    public:
        log_tree_model(const std::vector<std::pair<QString, Qt::ItemFlags>>& headers, QObject *parent = nullptr);
        ~log_tree_model();

        QVariant data(const QModelIndex &index, int role) const override;
        QVariant headerData(int section, Qt::Orientation orientation,
                            int role = Qt::DisplayRole) const override;

        QModelIndex index(int row, int column,
                        const QModelIndex &parent = QModelIndex()) const override;
        QModelIndex parent(const QModelIndex &index) const override;

        int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        int columnCount(const QModelIndex &parent = QModelIndex()) const override;

        Qt::ItemFlags flags(const QModelIndex &index) const override;
        bool setData(const QModelIndex &index, const QVariant &value,
                    int role = Qt::EditRole) override;
        bool setHeaderData(int section, Qt::Orientation orientation,
                        const QVariant &value, int role = Qt::EditRole) override;

        bool insertRows(int position, int rows,
                        const QModelIndex &parent = QModelIndex()) override;
        bool removeRows(int position, int rows,
                        const QModelIndex &parent = QModelIndex()) override;
		bool appendRow(const std::vector<QVariant>& new_row, const QModelIndex& parent = QModelIndex());

    private:

        log_item *getItem(const QModelIndex &index) const;

        log_item *rootItem;
		std::vector< Qt::ItemFlags> header_forbid_flag;
    };
}
