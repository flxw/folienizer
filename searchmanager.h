#ifndef SEARCHMANAGER_H
#define SEARCHMANAGER_H

# include <poppler-qt4.h>

# include <QHash>
# include <QObject>


class SearchManager : public QObject
{
    Q_OBJECT

    /* --- methods -------------------------------- */
public:
    explicit SearchManager(QObject *parent = 0);

    /* --- signals & slots ----------------------- */
signals:
    void matchFound(QString fpath, QRectF highlightArea, int page);

public slots:
    void searchDocumentForward(const QString& fpath, const QString &text, const int &page);
    void searchDocumentBackward(const QString& fpath, const QString &text, const int &page);

    void openDocument(const QString &fpath);

    /* --- attributes ---------------------------- */
private:
    QHash<QString, Poppler::Document*> documentHash;
};

#endif // SEARCHMANAGER_H
