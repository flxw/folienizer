# include "lectureitem.h"

# define DEFAULT_LAST_PAGE 0

LectureItem::LectureItem(QString name, QString data, TYPE t, LectureItem *parent) {
    displayName = QVariant(name);
    itemdata    = QVariant(data);
    parentItem  = parent;
    type        = t;
    lastPage    = DEFAULT_LAST_PAGE;
    opened      = false;

    if (t == LectureItem::SLIDE) {
        commentHash = new QHash<int,QString>;
    } else {
        commentHash = NULL;
    }
}

LectureItem::~LectureItem() {
    qDeleteAll(childItems);

    if (commentHash) delete commentHash;
}


/* ---- public method definition ------------- */
LectureItem *LectureItem::parent() {
    return parentItem;
}

LectureItem *LectureItem::child(int i) {
    return childItems.value(i);
}

QVariant LectureItem::getDisplayName() const {
    return this->displayName;
}

bool LectureItem::setDisplayName(const QVariant &value) {
    this->displayName = value;

    return true;
}

QVariant LectureItem::data() const {
    return this->itemdata;
}

bool LectureItem::setData(const QVariant &value) {
    this->itemdata = value;

    return true;
}

QString LectureItem::getComment(const int &index) {
    return commentHash->value(index, QString());
}

void LectureItem::setComment(const int &index, const QString &comment) {
    commentHash->insert(index, comment);
}

int LectureItem::getLastPageIndex() {
    return this->lastPage;
}

void LectureItem::setLastPageIndex(const int &p) {
    this->lastPage = p;
}

bool LectureItem::getOpened() {
    return this->opened;
}

void LectureItem::setOpened(bool o) {
    this->opened = o;
}

LectureItem::TYPE LectureItem::getType() const {
    return type;
}

int LectureItem::row() const {
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<LectureItem*>(this));

    return 0;
}

int LectureItem::childCount() const {
    return childItems.size();
}

bool LectureItem::insertChild(int pos, TYPE t, const QString& dp, const QString& d) {
    if ((0 <= pos && pos < childItems.size()) || pos == 0) {
        childItems.insert(pos, new LectureItem(dp, d, t, this));
        return true;
    } else {
        return false;
    }
}

LectureItem* LectureItem::appendChild(TYPE t, const QString& disp, const QString& data) {
    childItems.append(new LectureItem(disp, data, t, this));

    return childItems.last();
}

bool LectureItem::removeChild(LectureItem *item)  {
    for (int i = 0; i < childItems.length(); ++i) {
        if (childItems.at(i) == item) {
            delete childItems.takeAt(i);
            return true;
        }
    }

    return false;
}

bool LectureItem::writeToXmlStream(QXmlStreamWriter &x) {
    QString tagName = "";

    switch (this->type) {
        case LECTURE:    tagName = "LECTURE";    break;
        case LECTSLIDES: tagName = "LECTSLIDES"; break;
        case SLIDE:      tagName = "SLIDE";      break;
        case LECTTASKS:  tagName = "LECTTASKS";  break;
        case TASK:       tagName = "TASK";       break;
        case LECTBOOKS:  tagName = "LECTBOOKS";  break;
        case BOOK:       tagName = "BOOK";       break;
        case SEMESTER:   tagName = "SEMESTER";   break;
        case ROOTITEM:   tagName = "ROOTITEM";   break;
    }

    x.writeStartElement(tagName);
    x.writeAttribute("displayName", this->displayName.toString());
    x.writeAttribute("itemdata", this->itemdata.toString());

    if (this->type == SLIDE) {
        for (QHash<int,QString>::const_iterator it = commentHash->begin(); it != commentHash->end(); ++it) {
            if (!it.value().isEmpty()) {
                x.writeStartElement("COMMENT");
                x.writeAttribute("page", QString::number(it.key()));
                x.writeAttribute("value", it.value());
                x.writeEndElement();
            }
        }

        if (lastPage != DEFAULT_LAST_PAGE) {
            /* only save last viewed page if different from default */
            x.writeStartElement("LASTPAGEINDEX");
            x.writeAttribute("value", QString::number(lastPage));
            x.writeEndElement();
        }
    } else {
        for (QList<LectureItem*>::iterator it = childItems.begin(); it != childItems.end(); ++it) {
            (*it)->writeToXmlStream(x);
        }
    }

    x.writeEndElement();

    return true;
}
