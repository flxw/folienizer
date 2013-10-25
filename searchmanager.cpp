#include "searchmanager.h"

SearchManager::SearchManager(QObject *parent) : QObject(parent) {
}

/* --- method implementation ----------------- */
/* --- public slot implementation ------------ */
void SearchManager::searchDocumentForward(const QString &fpath, const QString &text, const int &page) {
    Poppler::Document* doc = documentHash.value(fpath, NULL);

    if (!doc) return;

    QRectF  matchLocation;
    double left,top,right,bottom;
    int     currentPage;

    /* search until end of document */
    for (currentPage = page; currentPage < doc->numPages(); ++currentPage) {
        if (doc->page(currentPage)->search(text, left, top, right, bottom,
                                    Poppler::Page::NextResult,
                                    Poppler::Page::CaseInsensitive)) {
            matchLocation = QRectF(QPoint(left,top), QPoint(right,bottom));

            if (matchLocation.isValid()) {
                emit matchFound(fpath, matchLocation, currentPage);
                return;
            }
        }

        matchLocation = QRectF();
    }

    /* after reaching the end of the doc wrap the search around to the beginning */
    for (currentPage = 0; currentPage < page; ++currentPage) {
        matchLocation = QRectF();

        if (doc->page(currentPage)->search(text, left, top, right, bottom,
                                    Poppler::Page::NextResult,
                                    Poppler::Page::CaseInsensitive)) {
            matchLocation = QRectF(QPoint(left,top), QPoint(right,bottom));

            if (matchLocation.isValid()) {
                emit matchFound(fpath, matchLocation, currentPage);
                return;
            }
        }
    }
}

void SearchManager::searchDocumentBackward(const QString &fpath, const QString &text, const int &page) {
    Poppler::Document* doc = documentHash.value(fpath, NULL);

    if (!doc) return;

    QRectF matchLocation;
    int    currentPage;

    /* search until begin of document */
    for (currentPage = page; currentPage > 0; --currentPage) {

        if (doc->page(page)->search(text, matchLocation,
                                    Poppler::Page::NextResult,
                                    Poppler::Page::CaseInsensitive)) {
            if (!matchLocation.isNull()) {
                emit matchFound(fpath, matchLocation, currentPage);
                return;
            }
        }

        matchLocation = QRectF();
    }

    /* after reaching the begin of the doc wrap the search around to the
     * end and go to the current page from there */
    for (currentPage = doc->numPages()-1; currentPage > page; --currentPage) {

        matchLocation = QRectF();

        if (doc->page(page)->search(text, matchLocation,
                                    Poppler::Page::PreviousResult,
                                    Poppler::Page::CaseInsensitive)) {
            if (!matchLocation.isNull()) {
                emit matchFound(fpath, matchLocation, currentPage);
                return;
            }
        }
    }
}

void SearchManager::openDocument(const QString &fpath) {
    Poppler::Document* doc = Poppler::Document::load(fpath);

    documentHash.insert(fpath, doc);
}
