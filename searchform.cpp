#include "searchform.h"
#include "ui_searchform.h"

SearchForm::SearchForm(QWidget *parent) : QWidget(parent), ui(new Ui::SearchForm) {
    ui->setupUi(this);

    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(fireSearchQuery()));
    connect(ui->lineEdit, SIGNAL(returnPressed()), this, SLOT(fireSearchQuery()));
}

SearchForm::~SearchForm() {
    delete ui;
}

/* --- private slot implementation ----------- */
/* fires the signal with the search query string as argument
 * and also guarantees that this query string is nonempty */
void SearchForm::fireSearchQuery() {
    QString query = this->ui->lineEdit->text();

    if (!query.isEmpty()) emit searchQueryFired(query, false);
}
