#ifndef BAGISISTEGIDIALOG_H
#define BAGISISTEGIDIALOG_H

#include <QDialog>
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

private slots:
  void onSelectClicked();

private:
  QTableWidget *tableWidget;
  Part selectedPart;
  void setupUi();
  void populateTable();
};

#endif // BAGISISTEGIDIALOG_H
