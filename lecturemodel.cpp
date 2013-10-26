# include "lecturemodel.h"

# include <QFile>
# include <QXmlStreamWriter>

LectureModel::LectureModel(QObject *parent) : QAbstractItemModel(parent) {
    rootItem = new LectureItem(QString(), QString(), LectureItem::ROOTITEM, NULL);
}

LectureModel::~LectureModel() {
    delete rootItem;
}

/* --- method definitions -------------------- */
QVariant LectureModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        return getItem(index)->getDisplayName();
    }

    return QVariant();
}

QVariant LectureModel::headerData(int section, Qt::Orientation orientation, int role) const {
    return QVariant();
}

Qt::ItemFlags LectureModel::flags(const QModelIndex &index) const {
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

QModelIndex LectureModel::index(int row, int column, const QModelIndex &parent) const {
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();


    LectureItem* childItem = getItem(parent)->child(row);

    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex LectureModel::parent(const QModelIndex &child) const {
    if (!child.isValid())
        return QModelIndex();

    LectureItem *parentItem = getItem(child)->parent();

    if (parentItem == rootItem) {
        return QModelIndex();
    } else {
        return createIndex(parentItem->row(), 0, parentItem);
    }
}

int LectureModel::rowCount(const QModelIndex &parent) const {
    return getItem(parent)->childCount();
}

int LectureModel::columnCount(const QModelIndex&) const {
    return 1;
}

bool LectureModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (index.isValid()) {
        getItem(index)->setDisplayName(value);
        return true;
    } else {
        return false;
    }
}

bool LectureModel::insertSemester(const QString& name) {
    return insertLectureItem(QModelIndex(), LectureItem::SEMESTER, name);
}

bool LectureModel::insertLecture(const QModelIndex& par, const QString &name, const bool &wtasks, const bool &wbooks) {
    if (!par.isValid()) return false;

    bool ret       = insertLectureItem(par, LectureItem::LECTURE, name);

    if (!wtasks && !wbooks) return ret;

    LectureItem* it = getItem(par)->child(getItem(par)->childCount() - 1);

    it->appendChild(LectureItem::LECTSLIDES, tr("slides"), QString());

    if (wtasks) it->appendChild(LectureItem::LECTTASKS, tr("tasks"), QString());
    if (wbooks) it->appendChild(LectureItem::LECTBOOKS, tr("books"), QString());

    return ret;
}

bool LectureModel::insertLectureSlide(const QModelIndex &par, const QString &fn, const QString &fl) {
    return insertLectureItem(par, LectureItem::SLIDE, fn, fl);
}

bool LectureModel::insertExerciseSlide(const QModelIndex &par, const QString &fn, const QString &fl) {
    return insertLectureItem(par, LectureItem::TASK, fn, fl);
}

bool LectureModel::removeRow(const QModelIndex &index) {
    if (index.isValid()) {
        LectureItem* item = (LectureItem*)index.internalPointer();

        int begin = (item->row()) ? item->row() - 1 : 0;
        emit beginRemoveRows(parent(index), begin, begin + 1);
        item->parent()->removeChild(item);
        emit endRemoveRows();

        return true;
    } else {
        return false;
    }
}

bool LectureModel::saveToDisk(const QString &location) const {
    QFile out(location);
    out.open(QFile::WriteOnly | QFile::Text);
    QXmlStreamWriter xmlWriter(&out);

    xmlWriter.setAutoFormatting(true);

    /* work our way from root to every leaf of the tree,
     * after which we'll need to close tags again */
    xmlWriter.writeStartDocument();
    rootItem->writeToXmlStream(xmlWriter);
    xmlWriter.writeEndDocument();

    return true;
}

bool LectureModel::loadFromDisk(const QString &where) {
    QFile in(where);
    in.open(QFile::ReadOnly | QFile::Text);
    QXmlStreamReader xmlReader(&in);
    QXmlStreamReader::TokenType tt;

    LectureItem *lastItem = rootItem;
    LectureItem::TYPE itemType;

    while (!xmlReader.atEnd()) {
        tt = xmlReader.readNext();

        QXmlStreamAttributes attrs = xmlReader.attributes();

        if (tt == QXmlStreamReader::StartElement) {
            /* find out the tag name and set ROOTITEM if its the rootItem */
            if (xmlReader.name() == QString("SEMESTER")) {
                itemType = LectureItem::SEMESTER;
            } else if (xmlReader.name() == QString("LECTURE")) {
                itemType = LectureItem::LECTURE;
            } else if (xmlReader.name() == QString("LECTSLIDES")) {
                itemType = LectureItem::LECTSLIDES;
            } else if (xmlReader.name() == QString("SLIDE")) {
                itemType = LectureItem::SLIDE;
            } else if (xmlReader.name() == QString("LECTTASKS")) {
                itemType = LectureItem::LECTTASKS;
            } else if (xmlReader.name() == QString("TASK")) {
                itemType = LectureItem::TASK;
            } else if (xmlReader.name() == QString("LECTBOOKS")) {
                itemType = LectureItem::LECTBOOKS;
            } else if (xmlReader.name() == QString("BOOK")) {
                itemType = LectureItem::BOOK;
            } else if (xmlReader.name() == QString("COMMENT")) {
                lastItem->setComment(attrs.value("page").toString().toInt(), attrs.value("value").toString());
                continue;
            } else if (xmlReader.name() == QString("LASTPAGEINDEX")) {
                lastItem->setLastPageIndex(attrs.value("value").toString().toInt());
                continue;
            } else {
                itemType = LectureItem::ROOTITEM;
            }

            if (itemType != LectureItem::ROOTITEM) {
                lastItem = lastItem->appendChild(itemType, attrs.value("displayName").toString(), attrs.value("itemdata").toString());
            }
        } else if (tt == QXmlStreamReader::EndElement
                   && xmlReader.name() != QString("COMMENT")
                   && xmlReader.name() != QString("LASTPAGEINDEX")) {
            lastItem = lastItem->parent();
        }
    }

    if (xmlReader.hasError()) {
        qDebug(qPrintable(xmlReader.errorString()));
    }

    return true;
}

/* ---- private method definitions ----------- */
bool LectureModel::insertLectureItem(const QModelIndex &parent, LectureItem::TYPE t, const QString &display, const QString &data) {
    /* we assume that the root item is clicked
     * i.e. we always the click the parent to append a child */
    LectureItem* respectiveItem = getItem(parent);

    int begin = (respectiveItem->childCount()) ? respectiveItem->childCount() - 1 : 0;
    emit beginInsertRows(parent, begin, begin);
    bool ret = false;

    ret = respectiveItem->appendChild(t, display, data);

    emit endInsertRows();

    return ret;
}

LectureItem* LectureModel::getItem(const QModelIndex &i) const {

    if (i.isValid()) {
        LectureItem *item = static_cast<LectureItem*>(i.internalPointer());

        if (item) {
            return item;
        }
    }

    return rootItem;
}
