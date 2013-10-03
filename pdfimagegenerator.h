#ifndef PDFIMAGEGENERATOR_H
#define PDFIMAGEGENERATOR_H

# include <QObject>
# include <QImage>
# include <QList>
# include <QMatrix>

# include <poppler-qt4.h>

/* code freeze until tests are written! */

class PdfImageGenerator : public QObject
{
    Q_OBJECT

    /* === methods =========================== */
public:
    explicit PdfImageGenerator(const QString &docloc, const int &dpix, const int &dpiy, QObject *parent=NULL);
    ~PdfImageGenerator();
    
    void requestPage(const int &pnum, const QRectF &matchHighlight = QRectF());
    void clearBacklog(void);
    void setPageSize(const QSize &s);

    void setScalefactor(const qreal &sf);
    qreal getScaleFactor(void);

    const QImage& getImageR(void);
    int getPageCount(void);
    bool isGood(void);

    QDomDocument *getTocDomDoc(void);

private:
    QImage generateImage(const int &pno);

    QRect scaleHighlightRect(const QRectF &r) const;

    /* === signals & slots =================== */
signals:
    void generationFinished(void);
    void imageFinished(QImage, int);
    void newPageSize(QSize);
    
public slots:
    void startGeneration(void);

    /* === attributes ======================== */
private:
    int dpiX, dpiY, avgw, avgh, passes, minpasses;
    qreal scaleFactor;
    QList< QPair<int,QRectF> > requestedPages;
    QSize pageSize;

    Poppler::Document *pdfDocument;

    static QMatrix highlightScaleMatrix;
};

#endif // PDFIMAGEGENERATOR_H
