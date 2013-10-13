#include "vieweroptionwidget.h"
#include "ui_vieweroptionwidget.h"

ViewerOptionWidget::ViewerOptionWidget(QWidget *parent) :
    QWidget(parent), ui(new Ui::ViewerOptionWidget) {
    ui->setupUi(this);

    /* connect signals and slots */

}

ViewerOptionWidget::~ViewerOptionWidget() {
    delete ui;
}

/* === public method definitions ============= */
/* === public slot definitions =============== */
