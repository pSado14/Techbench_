#ifndef LIDERLIKWIDGET_H
#define LIDERLIKWIDGET_H

#include <QFrame>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QVariantMap>
#include <QWidget>

class LiderlikWidget : public QWidget {
  Q_OBJECT
public:
  explicit LiderlikWidget(QWidget *parent = nullptr);
  void updateLeaderboard(const QList<QVariantMap> &data);

signals:
  void refreshRequested();

private:
  QTableWidget *tableWidget;
  QPushButton *refreshButton;
  void setupUi();
};

#endif // LIDERLIKWIDGET_H
