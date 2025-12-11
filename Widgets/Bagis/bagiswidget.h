#ifndef BAGISWIDGET_H
#define BAGISWIDGET_H

#include "../../Network/networkmanager.h"
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
  void setEmail(const QString &email);           // --- YENİ ---

private:
  Ui::BagisWidget *ui;

  void setupUiProgrammatically();
  void loadDonationRequests(); // --- YENİ ---
  void
  showDonationDialog(); // --- YENİ: Sub-merchant kontrolü sonrası açılan dialog

  struct DonationRequest {
    int id; // --- YENİ ---
    QString title;
    QString requester;
    QString category;
    QString timeLeft;
    int currentAmount;
    int targetAmount;
    int selectedAmount; // --- YENİ ---
    QString iconColor;
    bool isCompleted;
  };

  QString currentUsername;
  QString currentEmail;
  NetworkManager *netManager;

private slots:
  void on_createRequestBtn_clicked();

private:
  QGridLayout *cardsLayout;
  void createDonationCard(const DonationRequest &request, int row, int col);
  void showBagisIstegiDialog(); // Bağış isteği dialogunu göster
};

#endif // BAGISWIDGET_H
