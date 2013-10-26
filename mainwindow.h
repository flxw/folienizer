#ifndef MAINWINDOW_H
#define MAINWINDOW_H

# include <QMainWindow>
# include <QLabel>
# include <QPlainTextEdit>

# include "versioninfo.h"
# include "lecturemodel.h"
# include "documentviewwidget.h"
# include "searchmanager.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
    /* --- methods --------------------------- */
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *ev);

private:
    DocumentViewWidget* currentlyVisibleDvw(void);

    void openDocumentTab(LectureItem *lItem);

    void toggleContinuousView(void);
    void toggleCommentWindow(void);
    void toggleSearchWindow(void);

    void enableToc(const bool &b);

    /* --- signals & slots ------------------- */
public slots:
    void closeDocumentTab(const int &tabIndex);

    void setZoom(QAction *act);
    void switchTabInfo(void);
    void updateCurrentPageInfo(const int &newIndex, const int &oldIndex = 0);

    void handleTocSelection(QModelIndex index);
    void setSidebarContent(QAction *act);
    void handleLectureSelection(QModelIndex index);
    void handleLectureCustomContext(QPoint where);

    void addLectureSlides(void);

    void LM_addLectureWithBooks(void);
    void LM_addLectureWithBooksWithExercises(void);
    void LM_addLectureWithExercises(void);
    void addLectureLecture(const bool &we = false, const bool &wb = false);

    void addLectureSemester(void);
    void addBooksToLecture(void);
    void addTasksToLecture(void);
    void removeLectureItem(void);
    void changeSlidePath(void);

    void relaySearchQuery(QString q, bool backward);
    void relayQueryHit(QString fpath, QRectF matchHighlight, int page);

    /* --- attributes ------------------------ */
private:
    Ui::MainWindow *ui;

    DocumentViewWidget* lastViewedDvw;
    LectureModel* lectureModel;
    SearchManager* searchMan;

    QHash<QString,DocumentViewWidget*> file2DisplayHash;

    QLabel* pageDisplayLabel;
    QString folienizerXmlLocation;
};

#endif // MAINWINDOW_H
