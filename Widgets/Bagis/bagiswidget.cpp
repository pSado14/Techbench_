#include "bagiswidget.h"
#include "bagisistegidialog.h"
#include "moderndialogs.h"
#include "submerchantdialog.h"
#include "ui_bagiswidget.h"
#include <QDesktopServices>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPixmap>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollArea>
#include <QSizePolicy>
#include <QTreeWidget>
#include <QUrl>
#include <QVBoxLayout>

BagisWidget::BagisWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::BagisWidget) {
  ui->setupUi(this);
  netManager = new NetworkManager(this);
  setupUiProgrammatically();
  loadDonationRequests();
}

BagisWidget::~BagisWidget() { delete ui; }

void BagisWidget::setKullaniciAdi(const QString &username) {
  currentUsername = username;
}

void BagisWidget::setEmail(const QString &email) { currentEmail = email; }

void BagisWidget::setupUiProgrammatically() {
  // 1. Clean existing UI
  QList<QWidget *> children = findChildren<QWidget *>();
  for (QWidget *child : children) {
    child->deleteLater();
  }
  if (layout()) {
    delete layout();
  }

  QVBoxLayout *rootLayout = new QVBoxLayout(this);
  rootLayout->setContentsMargins(20, 20, 20, 20);

  // --- Main Container Frame ---
  QFrame *mainFrame = new QFrame();
  // Dark background with border and rounded corners
  mainFrame->setStyleSheet(
      "QFrame { background-color: #2b2e38; border: 1px solid #3c404d; "
      "border-radius: 15px; }"
      "QLabel { color: #ffffff; font-family: 'Segoe UI', sans-serif; border: "
      "none; background: transparent; }");

  QVBoxLayout *mainLayout = new QVBoxLayout(mainFrame);
  mainLayout->setSpacing(20);
  mainLayout->setContentsMargins(20, 20, 20, 20);

  // --- Page Title ---
  QLabel *pageTitle = new QLabel("Bağış Sayfası");
  pageTitle->setStyleSheet(
      "color: white; font-size: 24px; font-weight: bold; margin-bottom: 10px; "
      "border: none; background: transparent;");
  mainLayout->addWidget(pageTitle);

  // --- Commission Info Label ---
  QLabel *commissionLabel = new QLabel(
      "Yapılan bağışlardan Iyzico tarafından %1.1 + 0.25 TL işlem ücreti "
      "kesilmektedir.");
  commissionLabel->setStyleSheet(
      "color: #888888; font-size: 12px; margin-bottom: "
      "15px; font-style: italic;");
  mainLayout->addWidget(commissionLabel);

  // --- Create Request Button ---
  QHBoxLayout *topButtonsLayout = new QHBoxLayout();
  topButtonsLayout->setSpacing(10);

  QPushButton *createRequestBtn =
      new QPushButton("+ Bağış İsteği oluştur", this);
  createRequestBtn->setCursor(Qt::PointingHandCursor);
  createRequestBtn->setFixedSize(180, 40);
  createRequestBtn->setStyleSheet(
      "QPushButton { "
      "   background-color: #4facfe; "
      "   color: white; "
      "   border-radius: 5px; "
      "   font-weight: bold; "
      "   border: none;"
      "   outline: none;"
      "}"
      "QPushButton:hover { background-color: #00f2fe; }");

  connect(createRequestBtn, &QPushButton::clicked, this,
          &BagisWidget::on_createRequestBtn_clicked);

  topButtonsLayout->addWidget(createRequestBtn);

  // --- Refresh Button ---
  QPushButton *refreshBtn = new QPushButton(this);
  refreshBtn->setCursor(Qt::PointingHandCursor);
  refreshBtn->setFixedSize(40, 40);
  refreshBtn->setIcon(QIcon(":/Assets/refresh.png"));
  refreshBtn->setIconSize(QSize(24, 24));
  refreshBtn->setStyleSheet("QPushButton { "
                            "   background-color: transparent; "
                            "   border-radius: 5px; "
                            "   border: none;"
                            "   outline: none;"
                            "}"
                            "QPushButton:hover { background-color: #444; }");

  connect(refreshBtn, &QPushButton::clicked, this,
          &BagisWidget::loadDonationRequests);

  topButtonsLayout->addWidget(refreshBtn);
  topButtonsLayout->addStretch(); // Push buttons to left

  // Add buttons layout
  mainLayout->addLayout(topButtonsLayout);

  // --- Scroll Area for Cards ---
  QScrollArea *scrollArea = new QScrollArea(this);
  scrollArea->setWidgetResizable(true);
  // Modern Scrollbar Stylesheet
  scrollArea->setStyleSheet(
      "QScrollArea { border: none; background: transparent; }"
      "QScrollBar:vertical { border: none; background: #2b2e38; width: 10px; "
      "margin: 0px; border-radius: 5px; }"
      "QScrollBar::handle:vertical { background: #4facfe; min-height: 20px; "
      "border-radius: 5px; }"
      "QScrollBar::add-line:vertical { height: 0px; subcontrol-position: "
      "bottom; subcontrol-origin: margin; }"
      "QScrollBar::sub-line:vertical { height: 0px; subcontrol-position: top; "
      "subcontrol-origin: margin; }"
      "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { "
      "background: none; }"
      "QScrollBar:horizontal { border: none; background: #2b2e38; height: "
      "10px; margin: 0px; border-radius: 5px; }"
      "QScrollBar::handle:horizontal { background: #4facfe; min-width: 20px; "
      "border-radius: 5px; }"
      "QScrollBar::add-line:horizontal { width: 0px; }"
      "QScrollBar::sub-line:horizontal { width: 0px; }");

  QWidget *scrollContent = new QWidget();
  scrollContent->setStyleSheet("background-color: transparent;");
  cardsLayout = new QGridLayout(scrollContent);
  cardsLayout->setSpacing(15); // Reduced spacing
  cardsLayout->setContentsMargins(0, 10, 0, 10);
  cardsLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);

  scrollArea->setWidget(scrollContent);
  mainLayout->addWidget(scrollArea);

  rootLayout->addWidget(mainFrame);
}

void BagisWidget::on_createRequestBtn_clicked() {
  // Önce kullanıcının sub-merchant kaydı olup olmadığını kontrol et
  netManager->checkSubMerchant(
      currentUsername, [=](bool success, bool hasSubMerchant, QString message) {
        if (!success) {
          ModernMessageBox::critical(this, "Hata",
                                     "Kontrol yapılamadı: " + message);
          return;
        }

        if (!hasSubMerchant) {
          // Sub-merchant kaydı yok, bilgi toplama dialogunu göster
          SubMerchantDialog subDialog(this);
          subDialog.setEmail(currentEmail);

          if (subDialog.exec() == QDialog::Accepted) {
            SubMerchantDialog::SubMerchantInfo info =
                subDialog.getSubMerchantInfo();

            // Sub-merchant kaydını oluştur
            // Parametre sırası: username, name, surname, email, phone,
            // identity, iban, address
            netManager->registerSubMerchant(
                currentUsername, info.name, info.surname, info.email,
                info.phone, info.identityNumber, info.iban, info.address,
                [=](bool regSuccess, QString regMessage) {
                  if (regSuccess) {
                    ModernMessageBox::information(
                        this, "Başarılı",
                        "Ödeme bilgileriniz kaydedildi. Şimdi bağış isteği "
                        "oluşturabilirsiniz.");
                    // Kayıt başarılı, bağış isteği dialogunu göster
                    showBagisIstegiDialog();
                  } else {
                    ModernMessageBox::critical(
                        this, "Hata",
                        "Ödeme bilgileri kaydedilemedi: " + regMessage);
                  }
                });
          }
        } else {
          // Sub-merchant kaydı var, direkt bağış isteği dialogunu göster
          showBagisIstegiDialog();
        }
      });
}

void BagisWidget::showBagisIstegiDialog() {
  BagisIstegiDialog dialog(this);
  if (dialog.exec() == QDialog::Accepted) {
    BagisIstegiDialog::Part part = dialog.getSelectedPart();

    // Veritabanına kaydet
    netManager->createDonationRequest(
        currentUsername, part.name, part.category, part.price,
        [=](bool success, QString message) {
          if (success) {
            ModernMessageBox::information(this, "Başarılı",
                                          "Bağış isteği oluşturuldu.");
            loadDonationRequests(); // Listeyi yenile
          } else {
            ModernMessageBox::critical(this, "Hata",
                                       "İstek oluşturulamadı: " + message);
          }
        });
  }
}

void BagisWidget::loadDonationRequests() {
  // Önce mevcut kartları temizle
  QLayoutItem *child;
  while ((child = cardsLayout->takeAt(0)) != 0) {
    if (child->widget())
      child->widget()->deleteLater();
    delete child;
  }

  netManager->getDonationRequests(
      [=](bool success, QList<QVariantMap> requests, QString message) {
        if (success) {
          int row = 0;
          int col = 0;
          int maxCols = 4;

          for (const QVariantMap &reqData : requests) {
            DonationRequest req;
            req.id = reqData["id"].toInt();
            req.title = reqData["title"].toString();
            req.requester = reqData["username"].toString();
            // req.timeLeft = "30 Gün Kaldı"; // Removed as per user request
            req.currentAmount =
                reqData["collected_amount"]
                    .toDouble(); // toInt() "910.00" için 0 dönebilir
            req.targetAmount = reqData["price"].toInt();
            req.category = reqData["category"].toString();
            req.iconColor = "#4facfe";
            req.isCompleted = false;

            // Varsayılan bağış miktarı (örneğin 100 TL veya kalan miktar)
            int remaining = req.targetAmount - req.currentAmount;
            req.selectedAmount = (remaining > 100) ? 100 : remaining;
            if (req.selectedAmount <= 0)
              req.selectedAmount = 10; // Minimum 10 TL

            createDonationCard(req, row, col);

            col++;
            if (col >= maxCols) {
              col = 0;
              row++;
            }
          }
        } else {
          qDebug() << "Bağış istekleri alınamadı: " << message;
        }
      });
}

void BagisWidget::createDonationCard(const DonationRequest &request, int row,
                                     int col) {
  QWidget *card = new QWidget();
  // Reduced card size
  card->setFixedSize(220, 300);
  card->setStyleSheet("background-color: #1e1e1e; border-radius: 10px;");

  QVBoxLayout *cardLayout = new QVBoxLayout(card);
  cardLayout->setContentsMargins(10, 10, 10, 10); // Reduced margins

  // Header
  QHBoxLayout *headerLayout = new QHBoxLayout();

  QLabel *icon = new QLabel();
  icon->setFixedSize(50, 50);

  QString iconPath;
  if (request.category == "İşlemci") {
    iconPath = ":/Assets/cpu_icon.png";
  } else if (request.category == "Ekran Kartı") {
    iconPath = ":/Assets/gpu_icon.png";
  } else if (request.category == "RAM") {
    iconPath = ":/Assets/ram_icon.png";
  } else {
    iconPath = ":/Assets/question.png"; // Fallback
  }

  QPixmap pixmap(iconPath);
  if (!pixmap.isNull()) {
    icon->setPixmap(
        pixmap.scaled(50, 50, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    icon->setStyleSheet("background: transparent;");
  } else {
    // Fallback if image load fails
    icon->setText("?");
    icon->setStyleSheet(
        "background-color: #4facfe; border-radius: 25px; color: white; "
        "font-weight: bold; qproperty-alignment: AlignCenter;");
  }

  QLabel *title = new QLabel(request.title);
  title->setWordWrap(true);
  title->setStyleSheet("color: white; font-weight: bold; font-size: 14px; "
                       "background: transparent;");

  headerLayout->addWidget(icon);
  headerLayout->addWidget(title);
  headerLayout->addStretch();
  cardLayout->addLayout(headerLayout);
  cardLayout->addStretch();

  // Info
  QLabel *requester = new QLabel(QString("İsteyen: %1").arg(request.requester));
  requester->setStyleSheet(
      "color: gray; font-size: 11px; background: transparent;");
  cardLayout->addWidget(requester);

  // Progress
  QProgressBar *progress = new QProgressBar();
  progress->setRange(0, request.targetAmount);
  progress->setValue(request.currentAmount);
  progress->setTextVisible(false);
  progress->setFixedHeight(6);
  progress->setStyleSheet(
      "QProgressBar { background-color: #333; border-radius: 3px; } "
      "QProgressBar::chunk { background-color: #4facfe; border-radius: 3px; }");
  cardLayout->addWidget(progress);

  // Percentage
  int percent = 0;
  if (request.targetAmount > 0) {
    percent = (request.currentAmount * 100) / request.targetAmount;
  }
  QLabel *percentLabel = new QLabel(QString("%1% Tamamlandı").arg(percent));
  percentLabel->setAlignment(Qt::AlignRight);
  percentLabel->setStyleSheet("color: white; font-size: 12px; font-weight: "
                              "bold; background: transparent;");
  cardLayout->addWidget(percentLabel);

  // --- YENİ: Toplanan ve Kalan Miktar ---
  QHBoxLayout *amountsLayout = new QHBoxLayout();

  QLabel *collectedLabel =
      new QLabel(QString("Gönderilen: %1").arg(request.currentAmount));
  collectedLabel->setStyleSheet("color: #4facfe; font-size: 11px; font-weight: "
                                "bold; background: transparent;");
  amountsLayout->addWidget(collectedLabel);

  amountsLayout->addStretch();

  int remainingAmount = request.targetAmount - request.currentAmount;
  if (remainingAmount < 0)
    remainingAmount = 0;
  QLabel *remainingLabel =
      new QLabel(QString("Kalan: %1 TL").arg(remainingAmount));
  remainingLabel->setStyleSheet("color: #ff6b6b; font-size: 11px; font-weight: "
                                "bold; background: transparent;");
  amountsLayout->addWidget(remainingLabel);

  cardLayout->addLayout(amountsLayout);

  // Controls
  QHBoxLayout *controlsLayout = new QHBoxLayout();
  QLabel *amountLabel = new QLabel("Miktar\n(TL)");
  amountLabel->setStyleSheet(
      "color: white; font-size: 11px; background: transparent;");

  QPushButton *minusBtn = new QPushButton("-");
  minusBtn->setCursor(Qt::PointingHandCursor);
  minusBtn->setFixedSize(30, 30);
  minusBtn->setStyleSheet("background-color: #333; color: white; "
                          "border-radius: 5px; border: none; outline: none;");

  // --- DEĞİŞİKLİK: Label yerine Button kullanıyoruz ---
  QPushButton *valueButton =
      new QPushButton(QString::number(request.selectedAmount));
  valueButton->setCursor(Qt::PointingHandCursor);
  valueButton->setFixedSize(60, 30); // Biraz daha geniş
  valueButton->setStyleSheet(
      "QPushButton { "
      "   color: white; background-color: #2c2c2c; border-radius: 5px; "
      "   border: 1px solid #3c404d; font-weight: bold;"
      "}"
      "QPushButton:hover { background-color: #3c404d; border: 1px solid "
      "#4facfe; }");

  QPushButton *plusBtn = new QPushButton("+");
  plusBtn->setCursor(Qt::PointingHandCursor);
  plusBtn->setFixedSize(30, 30);
  plusBtn->setStyleSheet("background-color: #333; color: white; border-radius: "
                         "5px; border: none; outline: none;");

  controlsLayout->addWidget(amountLabel);
  controlsLayout->addWidget(minusBtn);
  controlsLayout->addWidget(valueButton);
  controlsLayout->addWidget(plusBtn);
  cardLayout->addLayout(controlsLayout);

  // --- YENİ: Buton İşlevleri ---

  // 1. Manuel Giriş (Butona Tıklayınca)
  connect(valueButton, &QPushButton::clicked, [=]() {
    int current = valueButton->text().toInt();
    int remaining = request.targetAmount - request.currentAmount;
    if (remaining <= 0)
      remaining = 10000; // Eğer hedef tamamlandıysa üst sınır koymayalım

    bool ok;
    int val = ModernInputDialog::getInt(
        this, "Bağış Miktarı",
        QString("Lütfen bağış miktarını giriniz (Max: %1 TL):").arg(remaining),
        current, 10, remaining, 1, &ok);

    if (ok) {
      valueButton->setText(QString::number(val));
    }
  });

  // 2. Eksi Butonu
  connect(minusBtn, &QPushButton::clicked, [=]() {
    int current = valueButton->text().toInt();
    if (current > 10) { // Minimum 10 TL
      current -= 10;
      valueButton->setText(QString::number(current));
    }
  });

  // 3. Artı Butonu
  connect(plusBtn, &QPushButton::clicked, [=]() {
    int current = valueButton->text().toInt();
    int remaining = request.targetAmount - request.currentAmount;
    // Eğer hedef dolduysa veya kalan miktar çok azsa bile artırabilsin mi?
    // Kullanıcı isteğine göre sınırlandırıyoruz:
    if (current < remaining) {
      current += 10;
      valueButton->setText(QString::number(current));
    }
  });

  QPushButton *supportBtn = new QPushButton();
  supportBtn->setCursor(Qt::PointingHandCursor);
  supportBtn->setFixedHeight(40);

  if (percent >= 100) {
    supportBtn->setText("Tamamlandı");
    supportBtn->setStyleSheet(
        "QPushButton { background-color: #2ecc71; color: white; border-radius: "
        "5px; font-weight: bold; border: none; outline: none; } "
        "QPushButton:disabled { background-color: #2ecc71; opacity: 0.7; "
        "color: white; }");
    supportBtn->setEnabled(false);
  } else {
    supportBtn->setText("Destek Ol");
    supportBtn->setStyleSheet(
        "QPushButton { background-color: #4facfe; color: white; border-radius: "
        "5px; font-weight: bold; border: none; outline: none; } "
        "QPushButton:hover { "
        "background-color: #00f2fe; }");

    // Connect Support Button
    connect(supportBtn, &QPushButton::clicked, this, [=]() {
      int amountToDonate = valueButton->text().toInt();
      netManager->initializePayment(
          currentUsername, currentEmail, amountToDonate, request.title,
          request.requester, request.id,
          [=](bool success, QString paymentUrl, QString message) {
            if (success) {
              QDesktopServices::openUrl(QUrl(paymentUrl));
            } else {
              ModernMessageBox::critical(this, "Hata",
                                         "Ödeme başlatılamadı: " + message);
            }
          });
    });
  }

  cardLayout->addWidget(supportBtn);

  cardsLayout->addWidget(card, row, col);
}

void BagisWidget::reset() {
  currentUsername = "";
  currentEmail = "";
  loadDonationRequests(); // Listeyi yenile
}
