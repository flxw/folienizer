#ifndef LECTUREMODEL_H
#define LECTUREMODEL_H

# include <QAbstractItemModel>
# include <QModelIndex>

# include "lectureitem.h"

class LectureModel : public QAbstractItemModel {
    Q_OBJECT

    /* ---- methods -------------------------- */
public:
    LectureModel(QObject *parent = 0);
    ~LectureModel();

    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    Qt::ItemFlags flags(const QModelIndex &index) const;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &child) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex&) const;

    bool setData(const QModelIndex &index, const QVariant &value, int role);
    bool insertSemester(const QString& name);
    bool insertLecture(const QModelIndex& par, const QString &name);
    bool insertLectureSlide(const QModelIndex &par, const QString &fn, const QString &fl);
    bool insertExerciseSlide(const QModelIndex &par, QString fn);

    bool removeRow(const QModelIndex &index);

    bool saveToDisk(const QString& location) const;
    bool loadFromDisk(const QString& where);

private:
    bool insertLectureItem(const QModelIndex &parent, LectureItem::TYPE t, const QString &display, const QString &data = QString());
    LectureItem *getItem(const QModelIndex &i) const;

    /* --- attributes ------------------------ */
private:
    LectureItem *rootItem;
};

#endif // LECTUREMODEL_H
