#ifndef VIEWEROPTIONWIDGET_H
#define VIEWEROPTIONWIDGET_H

#include <QWidget>

namespace Ui {
class ViewerOptionWidget;
}

class ViewerOptionWidget : public QWidget
{
    Q_OBJECT
    
    /* === methods =========================== */
public:
    explicit ViewerOptionWidget(QWidget *parent = 0);
    ~ViewerOptionWidget();

    /* === signals & slots =================== */
public slots:

    
private:
    Ui::ViewerOptionWidget *ui;
};

#endif // VIEWEROPTIONWIDGET_H
