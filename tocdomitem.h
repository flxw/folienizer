#ifndef TocDomItem_H
#define TocDomItem_H

#include <QDomNode>
#include <QHash>

class TocDomItem
{
public:
    TocDomItem(QDomNode &node, int row, TocDomItem *parent = 0);
    ~TocDomItem();

    TocDomItem *child(int i);
    TocDomItem *parent();
    QDomNode node() const;
    int row();

private:
    QDomNode domNode;
    QHash<int,TocDomItem*> childItems;
    TocDomItem *parentItem;
    int rowNumber;
};

#endif // TocDomItem_H
