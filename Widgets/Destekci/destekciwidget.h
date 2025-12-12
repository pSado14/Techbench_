#ifndef DESTEKCIWIDGET_H
#define DESTEKCIWIDGET_H

#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QWidget>

class DestekciWidget : public QWidget {
  Q_OBJECT

public:
  explicit DestekciWidget(QWidget *parent = nullptr);
  ~DestekciWidget();

  void setUsername(const QString &username);
  void reset();

signals:
  void refreshRequested();

public slots:
  void updateSupporters(const QList<QVariantMap> &supporters);

private slots:
  void onRefreshClicked();

private:
  void setupUi();

  QLabel *titleLabel;
  QLabel *statusLabel;
  QTableWidget *tableWidget;
  QPushButton *refreshButton;
  QString m_username;
};

#endif // DESTEKCIWIDGET_H
