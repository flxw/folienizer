# include "mainwindow.h"
# include "ui_mainwindow.h"
# include "lectureitem.h"

# include <QFileDialog>
# include <QDesktopServices>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow) {
    /* init some attributes */
    searchMan     = new SearchManager(this);
    lastViewedDvw = NULL;

    /* init lecture model and load it from disk */
    lectureModel = new LectureModel(this);
    folienizerXmlLocation = QDesktopServices::storageLocation(QDesktopServices::DataLocation).append("/lecturemodel");

    if (QFile::exists(folienizerXmlLocation)) {
        lectureModel->loadFromDisk(folienizerXmlLocation);
    } else {
        QDir d = QDir::current();
        d.mkpath(QDesktopServices::storageLocation(QDesktopServices::DataLocation));
    }

    /* setup ui */
    ui->setupUi(this);
    ui->commentWindow->hide();
    ui->searchWindow->hide();
    ui->treeView->setModel(lectureModel);

    /* integrate qlabel for current/total page display to statusbar */
    pageDisplayLabel = new QLabel(this);
    this->ui->statusBar->addWidget(pageDisplayLabel, 1);

    /* setup the combobox with options for zooming */
    QActionGroup *ag = new QActionGroup(this);

    ag->setExclusive(true);
    ag->addAction(this->ui->action50);
    ag->addAction(this->ui->action100);
    ag->addAction(this->ui->action150);
    ag->addAction(this->ui->action200);
    ag->addAction(this->ui->actionFit_page_height);
    ag->addAction(this->ui->actionFit_page_width);

    this->ui->action50->setData(QVariant((int)DocumentViewWidget::FIFTY));
    this->ui->action100->setData(QVariant((int)DocumentViewWidget::ONEHUNDRED));
    this->ui->action150->setData(QVariant((int)DocumentViewWidget::ONEHUNDREDFIFTY));
    this->ui->action200->setData(QVariant((int)DocumentViewWidget::TWOHUNDRED));
    this->ui->actionFit_page_height->setData(QVariant((int)DocumentViewWidget::FITHEIGHT));
    this->ui->actionFit_page_width->setData(QVariant((int)DocumentViewWidget::FITWIDTH));
    this->ui->menuZoom->setDisabled(true);

    /* setup the combobox that gives the user possibility to select what is displayed in the sidebar */
    this->ui->actionBookmarks->setData(QVariant(0));
    this->ui->actionLectures->setData(QVariant(1));
    this->ui->actionLectures->setChecked(true);
    this->ui->actionTOC->setData(QVariant(2));
    this->ui->actionTOC->setEnabled(false);
    this->ui->actionBookmarks->setEnabled(false);

    /* >> connect signals and slots <<<<<<<<<< */
    /* menu connections */
    connect(this->ui->actionQuit,  SIGNAL(triggered()),         this, SLOT(close()));
    connect(this->ui->menuZoom,    SIGNAL(triggered(QAction*)), this, SLOT(setZoom(QAction*)));
    connect(this->ui->menuSidebar, SIGNAL(triggered(QAction*)), this, SLOT(setSidebarContent(QAction*)));
    /* search box */
    connect(this->ui->searchWindow, SIGNAL(searchQueryFired(QString, bool)), this, SLOT(relaySearchQuery(QString, bool)));
    connect(searchMan, SIGNAL(matchFound(QString,QRectF,int)), this, SLOT(relayQueryHit(QString, QRectF, int)));
    /* tab widget connections */
    connect(this->ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(switchTabInfo()));
    connect(this->ui->tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(closeDocumentTab(int)));
    /* lecture tree connections */
    connect(this->ui->treeView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(handleLectureCustomContext(QPoint)));
    connect(this->ui->treeView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(handleLectureSelection(QModelIndex)));
}

MainWindow::~MainWindow()  {
    /* save the data from the lecture tree
     * but close any remaining tabs first */
    while (ui->tabWidget->count()) {
        closeDocumentTab(0);
    }

    lectureModel->saveToDisk(folienizerXmlLocation);

    delete ui;
}


/* === protected method definition =========== */
void MainWindow::keyPressEvent(QKeyEvent *ev) {
    switch (ev->key()) {
    case Qt::Key_C: toggleContinuousView(); ev->ignore(); break;
    case Qt::Key_X: toggleCommentWindow(); ev->ignore(); break;
    case Qt::Key_F: toggleSearchWindow(); ev->ignore(); break;
    }
}

/* === private methods definition ============ */
DocumentViewWidget* MainWindow::currentlyVisibleDvw() {
    return static_cast<DocumentViewWidget*>(this->ui->tabWidget->currentWidget());
}

void MainWindow::toggleContinuousView(void) {
    DocumentViewWidget *dvw = static_cast<DocumentViewWidget*>(this->ui->tabWidget->currentWidget());

    if (!dvw) return;

    dvw->setContinuous(!dvw->isContinuous());
}

void MainWindow::toggleCommentWindow() {
    ui->searchWindow->setVisible(false);
    ui->commentWindow->setVisible(!ui->commentWindow->isVisible());
}

void MainWindow::toggleSearchWindow() {
    ui->commentWindow->setVisible(false);
    ui->searchWindow->setVisible(!ui->searchWindow->isVisible());
}

/* === public slot definition ================ */
void MainWindow::openDocumentTab(LectureItem *lItem) {
    DocumentViewWidget *dvw = new DocumentViewWidget((void*) lItem, this);

    if (!dvw->loadDocument(lItem->data().toString())) {
        delete dvw;
        return;
    }

    /* check whether document has a table of contents */
    if (dvw->hasTocDomModel()) {
        this->ui->actionTOC->setEnabled(true);
    } else {
        this->ui->actionTOC->setEnabled(false);
    }

    /* go to last viewed page */
    dvw->gotoPage(lItem->getLastPageIndex());

    /* mark document as open */
    lItem->setOpened(true);

    this->ui->tabWidget->addTab(dvw, lItem->getDisplayName().toString());
    this->ui->menuZoom->setEnabled(true);

    /* from here: integrating the documentviewwidget into the main ui functionality
     *** allow current page to be changed on signal
     *** sync back comments on page change */
    searchMan->openDocument(lItem->data().toString());
    file2DisplayHash.insert(lItem->data().toString(), dvw);
    //connect(dvw, SIGNAL(pageChanged(int,int)), this, SLOT(updateCurrentPageInfo(int,int)));
}

void MainWindow::closeDocumentTab(const int &tabIndex) {
    /* pages persist in memory after removal from tab widget
     * see: http://qt-project.org/doc/qt-4.8/qtabwidget.html#removeTab
     *
     * thats why we'll save a pointer here and then manipulate some
     * things before deletion */
    DocumentViewWidget* dvw = (DocumentViewWidget*)ui->tabWidget->widget(tabIndex);

    /* remove tab, save its last page and mark as closed */
    ui->tabWidget->removeTab(tabIndex);
    ((LectureItem*)dvw->getLectureItem())->setLastPageIndex(dvw->getCurrentPageIndex());
    ((LectureItem*)dvw->getLectureItem())->setOpened(false);

    /* put the document inside the event loop and kiss it goodbuey */
    dvw->deleteLater();
    lastViewedDvw = NULL;
}

void MainWindow::setZoom(QAction *act) {
    DocumentViewWidget* dvw = currentlyVisibleDvw();

    if (!dvw) return;

    dvw->setZoom((DocumentViewWidget::ZOOM)act->data().toInt());
}

void MainWindow::switchTabInfo() {
    DocumentViewWidget *dvw  = currentlyVisibleDvw();

    if (!dvw) return;
    /* everything that accesses dvw needs to be below this check! */
    LectureItem* dvwLectItem = (LectureItem*) dvw->getLectureItem();

    /* set current zoom levels in zoom menu */
    switch (dvw->getZoom()) {
        case DocumentViewWidget::FITHEIGHT:       this->ui->actionFit_page_height->setChecked(true); break;
        case DocumentViewWidget::FITWIDTH:        this->ui->actionFit_page_width->setChecked(true);  break;
        case DocumentViewWidget::ONEHUNDRED:      this->ui->action100->setChecked(true);             break;
        case DocumentViewWidget::ONEHUNDREDFIFTY: this->ui->action150->setChecked(true);             break;
        case DocumentViewWidget::TWOHUNDRED:      this->ui->action200->setChecked(true);             break;
        default: Q_ASSERT("Some not supported zoom level!");                                          break;
    }

    /* show current page in statusbar */
    this->pageDisplayLabel->setText(tr("page %1 of %2").arg(dvw->getCurrentPageIndex()+1).arg(dvw->getPageCount()));

    /* enable toc button if toc is present */
    this->ui->actionTOC->setEnabled(dvw->hasTocDomModel());

    /* update comments shown */
    QString tmp = ui->commentWindow->toPlainText();

    if (lastViewedDvw) {
        ((LectureItem*)lastViewedDvw->getLectureItem())->setComment(lastViewedDvw->getCurrentPageIndex(), tmp);
    }

    ui->commentWindow->setPlainText(dvwLectItem->getComment(dvw->getCurrentPageIndex()));

    /* renew signal connections */
    disconnect(dvw, SIGNAL(pageChanged(int,int)));
    lastViewedDvw = dvw;
    connect(dvw, SIGNAL(pageChanged(int,int)), this, SLOT(updateCurrentPageInfo(int,int)));
}

void MainWindow::updateCurrentPageInfo(const int &newIndex, const int&oldIndex) {
    DocumentViewWidget* dvw = currentlyVisibleDvw();

    /* the check whether the dvw is NULL can be omitted here because
     * this slot is only called by signals that are emitted from dvws - and thus are visible */
    this->pageDisplayLabel->setText(tr("page %1 of %2").arg(newIndex+1).arg(dvw->getPageCount()));

    /* update the comments for the current page and save the ones of the last page */
    LectureItem* lItem = (LectureItem*)dvw->getLectureItem();

    lItem->setComment(oldIndex, ui->commentWindow->toPlainText());
    ui->commentWindow->setPlainText(lItem->getComment(newIndex));
}

void MainWindow::handleTocSelection(QModelIndex index) {
    if (!index.isValid()) return;

    DocumentViewWidget *dvw = currentlyVisibleDvw();
    dvw->gotoPageViaTocIndex(index);
}

void MainWindow::setSidebarContent(QAction *act) {
    bool checked = act->isChecked();

    /* clear previous bindings and selections because the group should be exclusively checkable */
    foreach (QAction *a, this->ui->menuSidebar->actions()) a->setChecked(false);
    disconnect(this->ui->treeView);

    switch (act->data().toInt()) {
    case 0: break;

    case 1:
        this->ui->treeView->setModel(lectureModel);
        connect(this->ui->treeView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(handleLectureCustomContext(QPoint)));
        connect(this->ui->treeView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(handleLectureSelection(QModelIndex)));
        break;

    case 2:
        DocumentViewWidget *dvw = currentlyVisibleDvw();
        if (!dvw) { checked = false; break; }
        this->ui->treeView->setModel(dvw->getTocDomModel());
        connect(this->ui->treeView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(handleTocSelection(QModelIndex)));
        break;
    }

    act->setChecked(checked);
    this->ui->treeView->setVisible(checked);

    if (!checked) ui->treeView->setModel(NULL);
}

void MainWindow::handleLectureSelection(QModelIndex index) {
    LectureItem* litem = (LectureItem*)index.internalPointer();
    bool        isGood = litem->getType() == LectureItem::SLIDE ||
            litem->getType() == LectureItem::BOOK  ||
            litem->getType() == LectureItem::TASK;

    if (isGood && !litem->getOpened()) {
        openDocumentTab(litem);
    }
}

void MainWindow::handleLectureCustomContext(QPoint where) {
    QMenu menu;

    LectureItem* currentItem = (LectureItem*)this->ui->treeView->selectionModel()->currentIndex().internalPointer();

    if (currentItem) {
        switch (currentItem->getType()) {
            case LectureItem::SEMESTER:
                menu.addAction(tr("Add lecture"), this, SLOT(addLectureLecture()));
                menu.addAction(tr("Add lecture /w books"), this, SLOT(LM_addLectureWithBooks()));
                menu.addAction(tr("Add lecture /w books /w exercises"), this, SLOT(LM_addLectureWithBooksWithExercises()));
                menu.addAction(tr("Add lecture /w exercises"), this, SLOT(LM_addLectureWithExercises()));

                menu.addAction(tr("Remove semester"), this, SLOT(removeLectureItem()));
                break;

            case LectureItem::LECTURE:
                menu.addAction(tr("Add slide(s)"), this, SLOT(addLectureSlides()));
                menu.addAction(tr("Remove lecture"), this, SLOT(removeLectureItem()));
                break;

            case LectureItem::LECTSLIDES:
                menu.addAction(tr("Add slide(s)"), this, SLOT(addLectureSlides()));
                break;

            case LectureItem::SLIDE:
                menu.addAction(tr("Change PDF"), this, SLOT(changeSlidePath()));
                menu.addAction(tr("Remove slide"), this, SLOT(removeLectureItem()));
                break;

            case LectureItem::LECTTASKS:
                menu.addAction(tr("Add exercises"), this, SLOT(addTasksToLecture()));
                break;

            case LectureItem::TASK:
                menu.addAction(tr("Change PDF"), this, SLOT(changeSlidePath()));
                menu.addAction(tr("Remove slide"), this, SLOT(removeLectureItem()));
                break;

            case LectureItem::LECTBOOKS:
                menu.addAction(tr("Add books"), this, SLOT(addBooksToLecture()));
                break;

            case LectureItem::BOOK:
                menu.addAction(tr("Change PDF"), this, SLOT(changeSlidePath()));
                menu.addAction(tr("Remove slide"), this, SLOT(removeLectureItem()));
                break;

            default: break;
        }
    }

    menu.addAction(tr("Add Semester"), this, SLOT(addLectureSemester()));

    menu.exec(ui->treeView->mapToGlobal(where));
}

void MainWindow::addLectureSlides() {
    QStringList fnList;

# ifdef QT_DEBUG
    fnList.append(QString("/home/felix/archive/studies/second_semester/dbs1/DBS1_01_Einfuehrung.pdf"));
# else
    fnList = QFileDialog::getOpenFileNames(this,
                                           tr("Select one or several slide sets to add"),
                                           QDir::homePath(),
                                           QString("*.pdf"));
# endif

    /* get active item - because the dialog is blocking, we can do this */
    QModelIndex curIndex = this->ui->treeView->selectionModel()->currentIndex();

    for (QStringList::const_iterator it = fnList.begin(); it != fnList.end(); ++it) {
        QFileInfo fi(*it);

        lectureModel->insertLectureSlide(curIndex, fi.fileName().left(fi.fileName().lastIndexOf(".")), *it);
    }
}

void MainWindow::LM_addLectureWithBooks(void) {
    addLectureLecture(false, true);
}

void MainWindow::LM_addLectureWithBooksWithExercises(void) {
    addLectureLecture(true, true);
}

void MainWindow::LM_addLectureWithExercises(void) {
    addLectureLecture(true, false);
}

void MainWindow::addLectureLecture(const bool &we, const bool &wb) {
    QString lecName;
    QModelIndex curIndex = this->ui->treeView->selectionModel()->currentIndex();

    lecName = tr("New lecture");

    lectureModel->insertLecture(curIndex, lecName, we, wb);
}

void MainWindow::addLectureSemester() {
    QString sname;

    sname = tr("New semester");

    lectureModel->insertSemester(sname);
}

void MainWindow::addBooksToLecture() {
    QStringList fnList;

# ifdef QT_DEBUG
    fnList.append(QString("/home/felix/archive/studies/second_semester/dbs1/DBS1_01_Einfuehrung.pdf"));
# else
    fnList = QFileDialog::getOpenFileNames(this,
                                           tr("Select one or several books to add"),
                                           QDir::homePath(),
                                           QString("*.pdf"));
# endif

    /* get active item - because the dialog is blocking, we can do this */
    QModelIndex curIndex = this->ui->treeView->selectionModel()->currentIndex();

    for (QStringList::const_iterator it = fnList.begin(); it != fnList.end(); ++it) {
        QFileInfo fi(*it);

        lectureModel->insertBook(curIndex, fi.fileName().left(fi.fileName().lastIndexOf(".")), *it);
    }
}

void MainWindow::addTasksToLecture() {
    QStringList fnList;

# ifdef QT_DEBUG
    fnList.append(QString("/home/felix/archive/studies/second_semester/dbs1/DBS1_01_Einfuehrung.pdf"));
# else
    fnList = QFileDialog::getOpenFileNames(this,
                                           tr("Select one or several exercises to add"),
                                           QDir::homePath(),
                                           QString("*.pdf"));
# endif

    /* get active item - because the dialog is blocking, we can do this */
    QModelIndex curIndex = this->ui->treeView->selectionModel()->currentIndex();

    for (QStringList::const_iterator it = fnList.begin(); it != fnList.end(); ++it) {
        QFileInfo fi(*it);

        lectureModel->insertExerciseSlide(curIndex, fi.fileName().left(fi.fileName().lastIndexOf(".")), *it);
    }
}

void MainWindow::removeLectureItem() {
    QModelIndex curIndex = this->ui->treeView->selectionModel()->currentIndex();

    lectureModel->removeRow(curIndex);
}

void MainWindow::changeSlidePath() {
    LectureItem* currentItem = (LectureItem*)this->ui->treeView->selectionModel()->currentIndex().internalPointer();

    QString newPath = QFileDialog::getOpenFileName(this,
                                                   tr("Select new lecture file"),
                                                   currentItem->data().toString(),
                                                   QString("*.pdf"));

    if (newPath.length()) {
        currentItem->setDisplayName(QFileInfo(newPath).fileName());
        currentItem->setData(newPath);
    }
}

void MainWindow::relaySearchQuery(QString q, bool backward) {
    DocumentViewWidget *dvw = currentlyVisibleDvw();
    QString fpath           = file2DisplayHash.key(dvw, QString());

    if(!dvw || fpath.isNull()) return;

    if (backward) {
        searchMan->searchDocumentBackward(fpath, q, dvw->getCurrentPageIndex());
    } else {
        searchMan->searchDocumentForward(fpath, q, dvw->getCurrentPageIndex());
    }
}

void MainWindow::relayQueryHit(QString fpath, QRectF matchHighlight, int page) {
    DocumentViewWidget* hitWidget = file2DisplayHash.value(fpath, NULL);

    if (!hitWidget) return;

    hitWidget->gotoPage(page, matchHighlight);
}
