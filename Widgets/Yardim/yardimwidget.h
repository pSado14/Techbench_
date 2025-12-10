#ifndef YARDIMWIDGET_H
#define YARDIMWIDGET_H

#include <QWidget>

class QTreeWidgetItem;

namespace Ui {
class YardimWidget;
}

class YardimWidget : public QWidget {
  Q_OBJECT

public:
  explicit YardimWidget(QWidget *parent = nullptr);
  ~YardimWidget();

private slots:
  void onItemClicked(QTreeWidgetItem *item, int column);

private:
  Ui::YardimWidget *ui;

  // Soruları eklemek için yardımcı fonksiyon
  void soruEkle(QString soru, QString cevap);
};

#endif // YARDIMWIDGET_H
