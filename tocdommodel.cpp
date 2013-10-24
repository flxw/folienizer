# include "tocdommodel.h"

# include <QtGui>
# include <QtXml>


TocDomModel::TocDomModel(QDomDocument document, QObject *parent)
    : QAbstractItemModel(parent), domDocument(document) {
    rootItem = new TocDomItem(domDocument, 0);
}

TocDomModel::~TocDomModel() {
    delete rootItem;
}

int TocDomModel::columnCount(const QModelIndex &/*parent*/) const {
    return 1;
}

QVariant TocDomModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return QVariant();

    QVariant retVariant;

    if (role == Qt::DisplayRole) {
        TocDomItem *item = static_cast<TocDomItem*>(index.internalPointer());

        retVariant = item->node().toElement().tagName();

//        QDomNode node = item->node();
//        QDomNamedNodeMap attributeMap = node.attributes();
//        node.

//        for (int i = 0; i < attributeMap.count(); ++i) {
//            QDomNode attribute = attributeMap.item(i);
//            QString dbg = attribute.nodeName() + "=\"" + attribute.nodeValue() + "\"";
//            qDebug(qPrintable(dbg)); }
    }

    return retVariant;
}

Qt::ItemFlags TocDomModel::flags(const QModelIndex &index) const {
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant TocDomModel::headerData(int section, Qt::Orientation orientation, int role) const {
    return QVariant();
}

QModelIndex TocDomModel::index(int row, int column, const QModelIndex &parent) const {
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    TocDomItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<TocDomItem*>(parent.internalPointer());

    TocDomItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex TocDomModel::parent(const QModelIndex &child) const {
    if (!child.isValid())
        return QModelIndex();

    TocDomItem *childItem = static_cast<TocDomItem*>(child.internalPointer());
    TocDomItem *parentItem = childItem->parent();

    if (!parentItem || parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int TocDomModel::rowCount(const QModelIndex &parent) const {
    if (parent.column() > 0)
        return 0;

    TocDomItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<TocDomItem*>(parent.internalPointer());

    return parentItem->node().childNodes().count();
}

QString TocDomModel::getLinkStringFromIndex(const QModelIndex &index) {
    /* TODO: where does the info that the named item is called 'Destination' come from?! */
    if (!index.isValid()) return QString();

    TocDomItem *item = static_cast<TocDomItem*>(index.internalPointer());
    QDomNode node = item->node();
    QDomNamedNodeMap attributeMap = node.attributes();

    return attributeMap.namedItem("Destination").nodeValue();
}
