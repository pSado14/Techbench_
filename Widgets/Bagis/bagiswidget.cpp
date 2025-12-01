#include "bagiswidget.h"
#include "bagisistegidialog.h"
#include "ui_bagiswidget.h"
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPixmap>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollArea>
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

void BagisWidget::setupUiProgrammatically() {
  // 1. Clean existing UI
  QList<QWidget *> children = findChildren<QWidget *>();
  for (QWidget *child : children) {
    child->deleteLater();
  }
  if (layout()) {
    delete layout();
  }

  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(20, 20, 20, 20);
  mainLayout->setSpacing(20);

  // --- Page Title ---
  QLabel *pageTitle = new QLabel("Bağış Sayfası");
  pageTitle->setStyleSheet(
      "color: white; font-size: 24px; font-weight: bold; margin-bottom: 10px;");
  mainLayout->addWidget(pageTitle);

  // --- Create Request Button ---
  QPushButton *createRequestBtn =
      new QPushButton("+ Bağış İsteği oluştur", this);
  createRequestBtn->setFixedSize(180, 40);
  createRequestBtn->setStyleSheet(
      "QPushButton { "
      "   background-color: #4facfe; "
      "   color: white; "
      "   border-radius: 5px; "
      "   font-weight: bold; "
      "}"
      "QPushButton:hover { background-color: #00f2fe; }");

  connect(createRequestBtn, &QPushButton::clicked, this,
          &BagisWidget::on_createRequestBtn_clicked);

  // Add button
  mainLayout->addWidget(createRequestBtn);

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
}

void BagisWidget::on_createRequestBtn_clicked() {
  BagisIstegiDialog dialog(this);
  if (dialog.exec() == QDialog::Accepted) {
    BagisIstegiDialog::Part part = dialog.getSelectedPart();

    // Veritabanına kaydet
    netManager->createDonationRequest(
        currentUsername, part.name, part.category, part.price,
        [=](bool success, QString message) {
          if (success) {
            QMessageBox::information(this, "Başarılı",
                                     "Bağış isteği oluşturuldu.");
            loadDonationRequests(); // Listeyi yenile
          } else {
            QMessageBox::critical(this, "Hata",
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
            req.title = reqData["title"].toString();
            req.requester = reqData["username"].toString();
            req.timeLeft = "30 Gün Kaldı"; // Backend'den gelirse oradan al
            req.currentAmount = 0;         // Backend'den gelirse oradan al
            req.targetAmount = reqData["price"].toInt();
            req.category = reqData["category"].toString(); // --- YENİ ---
            req.iconColor = "#4facfe";
            req.isCompleted = false;

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
  QLabel *requester = new QLabel(
      QString("İsteyen: %1   %2").arg(request.requester, request.timeLeft));
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
  QLabel *percentLabel = new QLabel("0%");
  percentLabel->setAlignment(Qt::AlignRight);
  percentLabel->setStyleSheet("color: white; font-size: 12px; font-weight: "
                              "bold; background: transparent;");
  cardLayout->addWidget(percentLabel);

  // Controls
  QHBoxLayout *controlsLayout = new QHBoxLayout();
  QLabel *amountLabel = new QLabel("Miktar\n(Puan)");
  amountLabel->setStyleSheet(
      "color: white; font-size: 11px; background: transparent;");

  QPushButton *minusBtn = new QPushButton("-");
  minusBtn->setFixedSize(30, 30);
  minusBtn->setStyleSheet("background-color: #333; color: white; "
                          "border-radius: 5px; border: none;");

  QLabel *valueLabel = new QLabel(QString::number(request.targetAmount));
  valueLabel->setAlignment(Qt::AlignCenter);
  valueLabel->setStyleSheet(
      "color: white; background-color: #2c2c2c; border-radius: 5px; padding: "
      "5px; min-width: 40px;");

  QPushButton *plusBtn = new QPushButton("+");
  plusBtn->setFixedSize(30, 30);
  plusBtn->setStyleSheet("background-color: #333; color: white; border-radius: "
                         "5px; border: none;");

  controlsLayout->addWidget(amountLabel);
  controlsLayout->addWidget(minusBtn);
  controlsLayout->addWidget(valueLabel);
  controlsLayout->addWidget(plusBtn);
  cardLayout->addLayout(controlsLayout);

  QPushButton *supportBtn = new QPushButton("Destek Ol");
  supportBtn->setFixedHeight(40);
  supportBtn->setStyleSheet(
      "QPushButton { background-color: #4facfe; color: white; border-radius: "
      "5px; font-weight: bold; border: none; } QPushButton:hover { "
      "background-color: #00f2fe; }");
  cardLayout->addWidget(supportBtn);

  cardsLayout->addWidget(card, row, col);
}
