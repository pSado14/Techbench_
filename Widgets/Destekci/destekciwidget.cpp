#include "destekciwidget.h"
#include "../../Network/networkmanager.h"
#include <QDebug>
#include <QHeaderView>

DestekciWidget::DestekciWidget(QWidget *parent) : QWidget(parent) { setupUi(); }

DestekciWidget::~DestekciWidget() {}

void DestekciWidget::setupUi() {
  // Ana layout
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(20, 20, 20, 20);

  // İçerik Container Frame - kenarlıklı, arka plan transparent
  QFrame *contentFrame = new QFrame(this);
  contentFrame->setStyleSheet("QFrame { "
                              "  background-color: transparent; "
                              "  border: 1px solid #3c404d; "
                              "  border-radius: 12px; "
                              "}");

  QVBoxLayout *layout = new QVBoxLayout(contentFrame);
  layout->setContentsMargins(30, 30, 30, 30);
  layout->setSpacing(20);

  // Başlık
  titleLabel = new QLabel(" Destekçilerim", contentFrame);
  titleLabel->setStyleSheet("font-size: 28px; font-weight: bold; color: "
                            "#ffffff; border: none; background: transparent;");
  layout->addWidget(titleLabel);

  // Açıklama
  QLabel *descLabel =
      new QLabel("Size en çok bağış yapan kişilerin listesi.", contentFrame);
  descLabel->setStyleSheet("font-size: 14px; color: #aaaaaa; border: none; "
                           "background: transparent;");
  layout->addWidget(descLabel);

  // Durum Label
  statusLabel = new QLabel("", contentFrame);
  statusLabel->setStyleSheet("font-size: 14px; color: #4facfe; border: none; "
                             "background: transparent;");
  layout->addWidget(statusLabel);

  // Tablo
  tableWidget = new QTableWidget(contentFrame);
  tableWidget->setColumnCount(3);
  tableWidget->setHorizontalHeaderLabels(
      {"Destekçi", "Toplam Bağış (₺)", "Bağış Sayısı"});
  tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  tableWidget->verticalHeader()->setVisible(false);
  tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
  tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
  tableWidget->setAlternatingRowColors(true);
  tableWidget->setStyleSheet(
      "QTableWidget { background-color: #2b2e38; color: white; border: 1px "
      "solid #3c404d; "
      "border-radius: 8px; gridline-color: #3c404d; "
      "alternate-background-color: #323642; }"
      "QHeaderView::section { background-color: #3c404d; color: white; "
      "padding: 10px; border: none; font-weight: bold; font-size: 14px; }"
      "QTableWidget::item { padding: 10px; font-size: 13px; }"
      "QTableWidget::item:selected { background-color: #4facfe; color: white; "
      "}");
  layout->addWidget(tableWidget);

  // Stretch - butonu aşağıya itmek için
  layout->addStretch();

  // Yenile Butonu (Aşağıda)
  refreshButton = new QPushButton("Listeyi Yenile", contentFrame);
  refreshButton->setCursor(Qt::PointingHandCursor);
  refreshButton->setFixedWidth(150);
  refreshButton->setStyleSheet(
      "QPushButton { background-color: #4facfe; color: white; border-radius: "
      "8px; padding: 10px 20px; font-weight: bold; border: none; }"
      "QPushButton:hover { background-color: #00f2fe; }");
  connect(refreshButton, &QPushButton::clicked, this,
          &DestekciWidget::onRefreshClicked);
  layout->addWidget(refreshButton, 0, Qt::AlignLeft);

  // Container'ı ana layout'a ekle
  mainLayout->addWidget(contentFrame);
}

void DestekciWidget::setUsername(const QString &username) {
  m_username = username;
  if (!username.isEmpty()) {
    onRefreshClicked(); // Otomatik olarak yükle
  }
}

void DestekciWidget::reset() {
  m_username = "";
  tableWidget->setRowCount(0);
  statusLabel->setText("Destekçileri görmek için giriş yapın.");
}

void DestekciWidget::onRefreshClicked() {
  if (m_username.isEmpty()) {
    statusLabel->setText("Kullanıcı bilgisi yok.");
    return;
  }

  statusLabel->setText("Yükleniyor...");
  emit refreshRequested();
}

void DestekciWidget::updateSupporters(const QList<QVariantMap> &supporters) {
  tableWidget->setRowCount(0);

  if (supporters.isEmpty()) {
    statusLabel->setText("Henüz destekçiniz bulunmuyor.");
    return;
  }

  statusLabel->setText(QString("%1 destekçi bulundu.").arg(supporters.size()));

  int row = 0;
  for (const QVariantMap &supporter : supporters) {
    tableWidget->insertRow(row);

    QString username = supporter["sender_username"].toString();
    double totalAmount = supporter["total_amount"].toDouble();
    int donationCount = supporter["donation_count"].toInt();

    QTableWidgetItem *usernameItem = new QTableWidgetItem(username);
    usernameItem->setTextAlignment(Qt::AlignCenter);

    QTableWidgetItem *amountItem =
        new QTableWidgetItem(QString::number(totalAmount, 'f', 2) + " ₺");
    amountItem->setTextAlignment(Qt::AlignCenter);
    amountItem->setForeground(QBrush(QColor("#2ecc71"))); // Yeşil

    QTableWidgetItem *countItem =
        new QTableWidgetItem(QString::number(donationCount));
    countItem->setTextAlignment(Qt::AlignCenter);

    tableWidget->setItem(row, 0, usernameItem);
    tableWidget->setItem(row, 1, amountItem);
    tableWidget->setItem(row, 2, countItem);

    row++;
  }
}
