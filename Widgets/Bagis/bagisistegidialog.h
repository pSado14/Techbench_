#ifndef BAGISISTEGIDIALOG_H
#define BAGISISTEGIDIALOG_H

#include <QDialog>
#include <QMouseEvent>
#include <QPoint>
#include <QTableWidget>
#include <QWidget>


class BagisIstegiDialog : public QDialog {
  Q_OBJECT

public:
  explicit BagisIstegiDialog(QWidget *parent = nullptr);
  ~BagisIstegiDialog();

  struct Part {
    QString name;
    QString category;
    int price;
  };

  Part getSelectedPart() const;

protected:
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;

private slots:
  void onSelectClicked();

private:
  QPoint m_dragPosition;

private:
  QTableWidget *tableWidget;
  Part selectedPart;
  void setupUi();
  void populateTable();
};

#endif // BAGISISTEGIDIALOG_H
