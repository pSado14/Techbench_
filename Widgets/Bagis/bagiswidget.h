#ifndef BAGISWIDGET_H
#define BAGISWIDGET_H

#include "../../Network/networkmanager.h" // --- YENİ ---
#include <QWidget>

class QGridLayout;

namespace Ui {
class BagisWidget;
}

class BagisWidget : public QWidget {
  Q_OBJECT

public:
  explicit BagisWidget(QWidget *parent = nullptr);
  ~BagisWidget();

  void setKullaniciAdi(const QString &username); // --- YENİ ---

private:
  Ui::BagisWidget *ui;

  void setupUiProgrammatically();
  void loadDonationRequests(); // --- YENİ ---

  struct DonationRequest {
    QString title;
    QString requester;
    QString category; // --- YENİ ---
    QString timeLeft;
    int currentAmount;
    int targetAmount;
    QString iconColor;
    bool isCompleted;
  };

  QString currentUsername;    // --- YENİ ---
  NetworkManager *netManager; // --- YENİ ---

private slots:
  void on_createRequestBtn_clicked();

private:
  QGridLayout *cardsLayout;
  void createDonationCard(const DonationRequest &request, int row, int col);
};

#endif // BAGISWIDGET_H
