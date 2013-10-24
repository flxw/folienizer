# include "tocdomitem.h"

# include <QtXml>

TocDomItem::TocDomItem(QDomNode &node, int row, TocDomItem *parent) {
    domNode = node;

    // Record the item's location within its parent.
    rowNumber = row;
    parentItem = parent;
}

TocDomItem::~TocDomItem() {
    QHash<int,TocDomItem*>::iterator it;
    for (it = childItems.begin(); it != childItems.end(); ++it)
        delete it.value();
}

QDomNode TocDomItem::node() const {
    return domNode;
}

TocDomItem *TocDomItem::parent() {
    return parentItem;
}

TocDomItem *TocDomItem::child(int i) {
    if (childItems.contains(i)) return childItems[i];

    if (i >= 0 && i < domNode.childNodes().count()) {
        QDomNode childNode = domNode.childNodes().item(i);
        TocDomItem *childItem = new TocDomItem(childNode, i, this);
        childItems[i] = childItem;
        return childItem;
    }
    return 0;
}

int TocDomItem::row() {
    return rowNumber;
}
