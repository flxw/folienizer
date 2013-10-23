# include <QPainter>

# include "pdfimagegenerator.h"

QMatrix PdfImageGenerator::highlightScaleMatrix = QMatrix();

PdfImageGenerator::PdfImageGenerator(const QString &docloc, const int &dpix, const int &dpiy, QObject *parent) : QObject(parent) {
    this->pdfDocument = Poppler::Document::load(docloc);

    if (pdfDocument) {
        pdfDocument->setRenderHint(Poppler::Document::Antialiasing);
        pdfDocument->setRenderHint(Poppler::Document::TextAntialiasing);
        minpasses = (pdfDocument->numPages() >=2)? 2 : 1;
    }

    dpiX = dpix;
    dpiY = dpiy;
    avgw = avgh = passes = 0;
}

PdfImageGenerator::~PdfImageGenerator() {
    delete this->pdfDocument;
}

void PdfImageGenerator::requestPage(const int &pnum, const QRectF &matchHighlight) {
    if (pnum >= 0) {
        if (requestedPages.size() >= 3) {
            requestedPages.pop_front();
        }

        this->requestedPages.append(QPair<int,QRectF>(pnum, matchHighlight));
    }
}

void PdfImageGenerator::clearBacklog() {
    this->requestedPages.clear();
}

void PdfImageGenerator::setScalefactor(const qreal &sf) {
    scaleFactor = sf;
    highlightScaleMatrix = QMatrix(scaleFactor * dpiX / 72.0, 0, 0, scaleFactor * dpiY / 72.0, 0, 0);


    /* upon resolution change, the pagesize is invalidated */
    pageSize = QSize(-1,-1);
    avgw = avgh = passes = 0;
}

qreal PdfImageGenerator::getScaleFactor() {
    return this->scaleFactor;
}

void PdfImageGenerator::setPageSize(const QSize &s) {
    this->pageSize = s;
}

int PdfImageGenerator::getPageCount() {
    return this->pdfDocument->numPages();
}

bool PdfImageGenerator::isGood() {
    return this->pdfDocument != 0;
}

QDomDocument* PdfImageGenerator::getTocDomDoc() {
    return this->pdfDocument->toc();
}

/* === private method implementation ========= */
QImage PdfImageGenerator::generateImage(const int &pno) {
    Poppler::Page* pdfPage = this->pdfDocument->page(pno);
    QImage image;

    if (pdfPage) {
        if (!pageSize.isValid()) {
            image = pdfPage->renderToImage(dpiX * scaleFactor, dpiY * scaleFactor);
        } else {
            image = pdfPage->renderToImage(dpiX * scaleFactor, dpiY * scaleFactor, -1, -1, pageSize.width(), pageSize.height());
        }

        delete pdfPage;
    }

    return image;
}

void PdfImageGenerator::startGeneration(void) {
    int page;
    QRectF highlight;
    QImage img;
    QPair<int,QRectF> demand;

    while (!this->requestedPages.isEmpty()) {
        demand    = requestedPages.takeLast();
        page      = demand.first;
        highlight = demand.second;
        img       = generateImage(page);

        if (!img.isNull()) {
            /* image size estimation */
            if (this->passes < this->minpasses) {
                ++passes;
                avgw += img.width();
                avgh += img.height();
            }

            /* search result highlight
             * cuts out the highlighted word and darkens the rest of the page */
            if (highlight.isValid()) {
                QRect highlightRect = scaleHighlightRect(highlight);
                highlightRect.adjust(-2, -2, 2, 2);
                QImage hImage = img.copy(highlightRect);
                QPainter painter;

                painter.begin(&img);
                painter.fillRect(img.rect(), QColor(0, 0, 0, 32));
                painter.drawImage(highlightRect, hImage);
                painter.end();
            }

            emit imageFinished(img, page);
        }
    }

    if (this->passes == minpasses) {
        pageSize.setWidth((int)(avgw/passes));
        pageSize.setHeight((int)(avgh/passes));
        ++passes;
        emit newPageSize(pageSize);
    }

    emit generationFinished();
}

QRect PdfImageGenerator::scaleHighlightRect(const QRectF &r) const {
    return highlightScaleMatrix.mapRect(r).toRect();
}
