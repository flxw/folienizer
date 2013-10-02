#ifndef DOCUMENTVIEWWIDGET_H
#define DOCUMENTVIEWWIDGET_H

# include <QWidget>
# include <QGraphicsScene>
# include <QGraphicsLinearLayout>
# include <QGraphicsWidget>
# include <QThread>
# include <QHash>

# include "pdfimagegenerator.h"
# include "tocdommodel.h"

namespace Ui {
class DocumentViewWidget;
}

class DocumentViewWidget : public QWidget
{
    Q_OBJECT
    
public:
    enum ZOOM {FITHEIGHT=-1, FITWIDTH, FIFTY, ONEHUNDRED, ONEHUNDREDFIFTY, TWOHUNDRED};

    /* === methods =========================== */
public:
    explicit DocumentViewWidget(void *lectureItem, QWidget *parent = 0);
    ~DocumentViewWidget();

    void openDocumentInitially();
    bool loadDocument(QString filename);

    QString getDocumentName(void) const;
    void setDocumentName(const QString &docname);

    ZOOM getZoom(void);
    void setZoom(ZOOM z);

    int getPageCount(void);
    int getCurrentPageIndex(void);

    void setContinuous(bool c=true);
    bool isContinuous(void);

    TocDomModel* getTocDomModel(void);
    bool hasTocDomModel(void);

    void gotoPageViaTocIndex(const QModelIndex &index);
    void reloadAfterResize(void);

    void* getLectureItem(void);

protected:
    void showEvent(QShowEvent *);
    void resizeEvent(QResizeEvent *event);

private:
    void updateViewportSize(void);
    
    /* === signals & slots =================== */
signals:
    void pageChanged(const int &newIndex, const int &oldIndex);

public slots:
    void gotoPage(const int &page, QRectF hl = QRectF());
    void showNextPage(void);
    void showPreviousPage(void);
    void scrollDocument(int scrollbarValue);
    void reactOnResize(void);

private slots:
    void reloadPage(void);
    void setImage(QImage img, int page);
    void setPageSize(QSize s);

    /* === attributes ======================== */
private:
    struct DisplayPage {
        int page;
        QGraphicsPixmapItem* gitem;
    } *upperPage, *middlePage, *lowerPage;

    PdfImageGenerator *pdfImageGenerator;
    TocDomModel *tocDomModel;

    QThread *generatorThread;
    QGraphicsScene *graphicsScene;

    QString documentLocation;
    QString documentName;
    QSize pageSize;
    ZOOM zoom;

    int currentPageIndex;
    int lastPageIndex;
    int pageCount;
    int pageBufferSpace;

    bool good;
    bool continuous;

    void* lItem;

    Ui::DocumentViewWidget *ui;
};

#endif // DOCUMENTVIEWWIDGET_H
