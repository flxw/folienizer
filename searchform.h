#ifndef SEARCHFORM_H
#define SEARCHFORM_H

#include <QWidget>

namespace Ui {
class SearchForm;
}

class SearchForm : public QWidget
{
    Q_OBJECT

public:
    explicit SearchForm(QWidget *parent = 0);
    ~SearchForm();

signals:
    void searchQueryFired(QString query, bool backwards);

public slots:
private slots:
    void fireSearchQuery(void);

private:
    Ui::SearchForm *ui;
};

#endif // SEARCHFORM_H
