#ifndef TOCDOMMODEL_H
#define TOCDOMMODEL_H

# include <QAbstractItemModel>
# include <QDomDocument>
# include <QModelIndex>
# include <QVariant>

# include "tocdomitem.h"

class TocDomModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    TocDomModel(QDomDocument document, QObject *parent = 0);
    ~TocDomModel();

    Qt::ItemFlags flags(const QModelIndex &index) const;

    QVariant headerData(int section, Qt::Orientation orientation,  int role = Qt::DisplayRole) const;
    QVariant data(const QModelIndex &index, int role) const;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &child) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    QString getLinkStringFromIndex(const QModelIndex &index);
private:
    QDomDocument domDocument;
    TocDomItem *rootItem;
};

#endif // TOCDOMMODEL_H
