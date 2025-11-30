#ifndef BAGISWIDGET_H
#define BAGISWIDGET_H

#include <QWidget>

namespace Ui {
class BagisWidget;
}

class BagisWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BagisWidget(QWidget *parent = nullptr);
    ~BagisWidget();

private:
    Ui::BagisWidget *ui;
};

#endif // BAGISWIDGET_H
