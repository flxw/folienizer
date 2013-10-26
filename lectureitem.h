#ifndef LectureItem_H
#define LectureItem_H

# include <QList>
# include <QVariant>
# include <QXmlStreamWriter>

class LectureItem {
public:
    enum TYPE {LECTURE, LECTSLIDES, SLIDE, LECTTASKS, TASK, LECTBOOKS, BOOK, SEMESTER, ROOTITEM};

    LectureItem(QString name, QString data, TYPE t, LectureItem *parent = 0);
    ~LectureItem();

    LectureItem *parent(void);
    LectureItem *child(int i);
    LectureItem *appendChild(TYPE t, const QString &disp, const QString &data);

    QVariant getDisplayName(void) const;
    bool setDisplayName(const QVariant &value);

    QVariant data(void) const;
    bool setData(const QVariant &value);

    QString getComment(const int &index);
    void setComment(const int &index, const QString &comment);

    int getLastPageIndex(void);
    void setLastPageIndex(const int &p);

    bool getOpened();
    void setOpened(bool o);

    TYPE getType(void) const;

    int row(void) const;
    int childCount(void) const;

    bool insertChild(int pos, TYPE t, const QString &dp, const QString& d);
    bool removeChild(LectureItem* item);

    bool writeToXmlStream(QXmlStreamWriter &x);

private:
    QList<LectureItem*> childItems;
    LectureItem *parentItem;

    /* data */
    QVariant displayName;
    QVariant itemdata;
    TYPE type;

    /* slide specific data */
    QHash<int,QString> *commentHash;
    int lastPage;
    bool opened;
};

#endif // LectureItem_H
