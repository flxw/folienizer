# include <QGraphicsPixmapItem>
# include <QScrollBar>
# include <QTimer>

# include <poppler-qt4.h>
# include <poppler-link.h>

# include "documentviewwidget.h"
# include "ui_documentviewwidget.h"


DocumentViewWidget::DocumentViewWidget(void* lectureItem, QWidget *parent) :
    QWidget(parent), ui(new Ui::DocumentViewWidget) {
    ui->setupUi(this);

    lItem           = lectureItem;

    continuous      = true;
    good            = false;
    zoom            = ONEHUNDRED;
    pageBufferSpace = 5;

    tocDomModel     = NULL;

    /* initialize stuff on heap */    
    generatorThread  = new QThread(this);
    graphicsScene = new QGraphicsScene(this);

    lowerPage  = new DisplayPage;
    middlePage = new DisplayPage;
    upperPage  = new DisplayPage;

    lowerPage->gitem  = graphicsScene->addPixmap(QPixmap());
    middlePage->gitem = graphicsScene->addPixmap(QPixmap());
    upperPage->gitem  = graphicsScene->addPixmap(QPixmap());

    this->graphicsScene->setBackgroundBrush(QBrush(QColor(128,128,128)));
    this->ui->graphicsView->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    this->ui->graphicsView->setScene(graphicsScene);

    /* connect signals and slots */
    connect(this->ui->graphicsView->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(scrollDocument(int)));
}

DocumentViewWidget::~DocumentViewWidget() {
    /* disconnect signals so we can clean up */
    disconnect(generatorThread);
    disconnect(pdfImageGenerator);

    delete ui;
    delete upperPage;
    delete middlePage;
    delete lowerPage;

    /* if table of contents DOM is present -> delete it*/
    if (tocDomModel)
        delete tocDomModel;

    /* if the thread is still running, terminate it and delete it */
    generatorThread->wait();
    delete generatorThread;
    delete pdfImageGenerator;
}

/* === public method definitions ============= */
bool DocumentViewWidget::loadDocument(QString filename) {
    /* allocate the generators */
    pdfImageGenerator  = new PdfImageGenerator(filename, physicalDpiX(), physicalDpiY());

    if (!pdfImageGenerator->isGood()) return false;

    /* check for DOM presence */
    QDomDocument *tocdom = pdfImageGenerator->getTocDomDoc();

    if (tocdom) {
        this->tocDomModel = new TocDomModel(*tocdom, this);
        delete tocdom;
    }

    /* make connections */
    connect(generatorThread,   SIGNAL(started()),                 pdfImageGenerator, SLOT(startGeneration()));
    connect(pdfImageGenerator, SIGNAL(imageFinished(QImage,int)), this,             SLOT(setImage(QImage,int)));
    connect(pdfImageGenerator, SIGNAL(generationFinished()),      generatorThread,   SLOT(quit()));
    connect(pdfImageGenerator, SIGNAL(newPageSize(QSize)),        this,             SLOT(setPageSize(QSize)));

    /* set runtime data */
    pdfImageGenerator->setScalefactor(((qreal)this->zoom)/2);
    pdfImageGenerator->moveToThread(generatorThread);

    this->pageCount = pdfImageGenerator->getPageCount();

    return true;
}

QString DocumentViewWidget::getDocumentName() const {
    return this->documentName;
}

void DocumentViewWidget::setDocumentName(const QString &docname) {
    this->documentName = docname;
}

DocumentViewWidget::ZOOM DocumentViewWidget::getZoom() {
    return this->zoom;
}

void DocumentViewWidget::setZoom(ZOOM z) {
    qreal scalefactor;
    zoom = z;

    if (z > 0) {
        /* normal scaling */
        scalefactor = (qreal)zoom/2;
    } else if (z == FITWIDTH){
        /* fit to page width */
        qreal viewPortWidth = this->ui->graphicsView->viewport()->size().width();
        scalefactor = pdfImageGenerator->getScaleFactor() * ((qreal)viewPortWidth/(qreal)pageSize.width());
    } else if (z == FITHEIGHT) {
        /* fit to page height */
        qreal viewPortHeight = this->ui->graphicsView->viewport()->size().height();
        scalefactor = pdfImageGenerator->getScaleFactor() * ((qreal)viewPortHeight/(qreal)pageSize.height());
    }

    /* delete the old images */
    //upperPage->gitem->setPixmap(QPixmap());
    //middlePage->gitem->setPixmap(QPixmap());
    //lowerPage->gitem->setPixmap(QPixmap());

    /* renew all images */
    if (pdfImageGenerator->getScaleFactor() != scalefactor) {
        generatorThread->terminate();
        pdfImageGenerator->clearBacklog();
        pdfImageGenerator->setScalefactor(scalefactor);
        gotoPage(currentPageIndex);
    }
}

int DocumentViewWidget::getPageCount() {
    return pageCount;
}

int DocumentViewWidget::getCurrentPageIndex() {
    return this->currentPageIndex;
}

void DocumentViewWidget::setContinuous(bool c) {
    this->continuous = c;
    this->updateViewportSize();

    if (c) this->ui->graphicsView->verticalScrollBar()->setValue(currentPageIndex * pageSize.height());
}

bool DocumentViewWidget::isContinuous() {
    return this->continuous;
}

TocDomModel* DocumentViewWidget::getTocDomModel() {
    return this->tocDomModel;
}

bool DocumentViewWidget::hasTocDomModel() {
    return this->tocDomModel != NULL;
}

void DocumentViewWidget::gotoPageViaTocIndex(const QModelIndex &index) {
    if (!index.isValid()) return;

    QString dest = this->tocDomModel->getLinkStringFromIndex(index);
    Poppler::LinkDestination ld(dest);

    gotoPage(ld.pageNumber()-1);

    if (this->continuous) {
        this->ui->graphicsView->verticalScrollBar()->setValue(pageSize.height()*(ld.pageNumber()-1));
    } else {
        this->graphicsScene->setSceneRect(0, currentPageIndex * pageSize.height(), pageSize.width(), pageSize.height());
    }
}

void DocumentViewWidget::reactOnResize() {
    setZoom(zoom);
}

/* === protected method definition =========== */
void DocumentViewWidget::showEvent(QShowEvent *) {
    this->updateViewportSize();
}

void DocumentViewWidget::resizeEvent(QResizeEvent *event) {
    static QTimer t;

    if (!t.isSingleShot()) {
        t.setSingleShot(true);
        connect(&t, SIGNAL(timeout()), this, SLOT(reactOnResize()));
    }

    if (zoom == FITHEIGHT || zoom == FITWIDTH) {
        t.stop();
        t.start(250);
    }

    QWidget::resizeEvent(event);
}

/* === private method definitions ============ */
void DocumentViewWidget::updateViewportSize() {
    if (this->continuous) {
        this->graphicsScene->setSceneRect(0, 0, pageSize.width(), pageCount * pageSize.height());
        this->ui->graphicsView->verticalScrollBar()->setPageStep(pageSize.height());
    } else {
        this->graphicsScene->setSceneRect(0, currentPageIndex * pageSize.height(), pageSize.width(), pageSize.height());
    }
}

/* === public slot definitions =============== */
void DocumentViewWidget::gotoPage(const int &page, QRectF hl) {
    /* the graphicsitems are moved around as a chain of three where
     * the current page is sandwiched between two other pages.
     * we put the item that is out of view at the place that is probably
     * going to be viewed next - so if we are scrolling down, take the item
     * that is the most up. analogue for scrolling up */

    if (currentPageIndex+1 == page && hl.isNull()) {
        DisplayPage *pd = lowerPage;

        upperPage->gitem->setY(pageSize.height()*(page + 1));
        lowerPage = upperPage;
        upperPage = middlePage;
        middlePage = pd;

        pdfImageGenerator->requestPage(page + 1);
    } else if (currentPageIndex-1 == page && hl.isNull()) {
        DisplayPage *pd = upperPage;

        lowerPage->gitem->setY(pageSize.height()*(page - 1));
        upperPage = lowerPage;
        lowerPage = middlePage;
        middlePage = pd;

        pdfImageGenerator->requestPage(page - 1);
    } else {
        upperPage->gitem->setY(pageSize.height() * (page - 1));
        middlePage->gitem->setY(pageSize.height() * page);
        lowerPage->gitem->setY(pageSize.height()* (page + 1));

        if (hl.isNull()) {
            pdfImageGenerator->requestPage(page);
        } else {
            disconnect(ui->graphicsView->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(scrollDocument(int)));

            /* gonna jump anyway so might as well clean up! */
            generatorThread->terminate();
            pdfImageGenerator->clearBacklog();
            pdfImageGenerator->requestPage(page, hl);

            ui->graphicsView->verticalScrollBar()->setValue(pageSize.height() * page);
            connect(ui->graphicsView->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(scrollDocument(int)));
        }

        pdfImageGenerator->requestPage(page + 1);
        pdfImageGenerator->requestPage(page - 1);
    }

    emit pageChanged(page, currentPageIndex);

    generatorThread->start();
    currentPageIndex = page;
}

void DocumentViewWidget::showNextPage() {
    if (currentPageIndex+1 < pageCount) {
        this->gotoPage(currentPageIndex+1);
    }
}

void DocumentViewWidget::showPreviousPage() {
    if (currentPageIndex > 0) {
        this->gotoPage(currentPageIndex-1);
    }
}

void DocumentViewWidget::scrollDocument(int scrollbarValue) {
    if (continuous) {
        int nextPageNo = (int)(scrollbarValue / pageSize.height());

        if (nextPageNo != currentPageIndex) {
            gotoPage(nextPageNo);
        }
    }
}

/* === private slot definition =============== */
void DocumentViewWidget::reloadPage() {}

void DocumentViewWidget::setImage(QImage img, int page) {
    if (page == currentPageIndex-1) {
        upperPage->gitem->setPixmap(QPixmap::fromImage(img));
        upperPage->page = page;
    } else if (page == currentPageIndex) {
        middlePage->gitem->setPixmap(QPixmap::fromImage(img));
        middlePage->page = page;
    } else if (page == currentPageIndex +1) {
        lowerPage->gitem->setPixmap(QPixmap::fromImage(img));
        lowerPage->page = page;
    }
}

void DocumentViewWidget::setPageSize(QSize s) {
    this->pageSize = s;

    pageSize.setHeight(pageSize.height() + pageBufferSpace);

    this->updateViewportSize();

    /* relocate the pixmaps */
    upperPage->gitem->setY(pageSize.height()  * (currentPageIndex - 1));
    middlePage->gitem->setY(pageSize.height() * currentPageIndex);
    lowerPage->gitem->setY(pageSize.height()  * (currentPageIndex + 1));

    this->ui->graphicsView->verticalScrollBar()->setValue(pageSize.height() * currentPageIndex);
}

void* DocumentViewWidget::getLectureItem() {
    return this->lItem;
}
