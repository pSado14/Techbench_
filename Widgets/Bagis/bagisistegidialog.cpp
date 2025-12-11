#include "bagisistegidialog.h"
#include "moderndialogs.h"
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

BagisIstegiDialog::BagisIstegiDialog(QWidget *parent) : QDialog(parent) {
  setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
  setAttribute(Qt::WA_TranslucentBackground);
  resize(750, 550);
  setupUi();
  populateTable();
}

BagisIstegiDialog::~BagisIstegiDialog() {}

void BagisIstegiDialog::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    m_dragPosition = event->globalPos() - frameGeometry().topLeft();
    event->accept();
  }
}

void BagisIstegiDialog::mouseMoveEvent(QMouseEvent *event) {
  if (event->buttons() & Qt::LeftButton) {
    move(event->globalPos() - m_dragPosition);
    event->accept();
  }
}

void BagisIstegiDialog::setupUi() {
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(15, 15, 15, 15);

  // Main Frame (Rounded & Dark with gradient)
  QFrame *frame = new QFrame(this);
  frame->setObjectName("mainFrame");
  frame->setStyleSheet("#mainFrame { "
                       "   background-color: #2b2e38; "
                       "   border: 1px solid #3c404d; "
                       "   border-radius: 15px; "
                       "}");

  // Shadow effect
  QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
  shadow->setBlurRadius(30);
  shadow->setColor(QColor(0, 0, 0, 150));
  shadow->setOffset(0, 5);
  frame->setGraphicsEffect(shadow);

  mainLayout->addWidget(frame);

  QVBoxLayout *layout = new QVBoxLayout(frame);
  layout->setSpacing(20);
  layout->setContentsMargins(25, 25, 25, 25);

  // --- Title Bar ---
  QHBoxLayout *titleLayout = new QHBoxLayout();

  QLabel *titleLabel = new QLabel("Bağış İsteği Oluştur");
  titleLabel->setStyleSheet(
      "color: #ffffff; font-size: 18px; font-weight: bold; "
      "background: transparent;");

  QPushButton *closeBtn = new QPushButton("X");
  closeBtn->setFixedSize(30, 30);
  closeBtn->setCursor(Qt::PointingHandCursor);
  closeBtn->setStyleSheet("QPushButton { "
                          "   background-color: transparent; "
                          "   color: #bec3cd; "
                          "   border: none; "
                          "   font-weight: bold; "
                          "   font-size: 16px; "
                          "}"
                          "QPushButton:hover { "
                          "   color: #ff4d4d; "
                          "}");
  connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);

  titleLayout->addWidget(titleLabel);
  titleLayout->addStretch();
  titleLayout->addWidget(closeBtn);
  layout->addLayout(titleLayout);

  // --- Subtitle ---
  QLabel *subtitleLabel = new QLabel("Almak istediğiniz donanımı seçin:");
  subtitleLabel->setStyleSheet(
      "color: #8f94a8; font-size: 13px; background: transparent;");
  layout->addWidget(subtitleLabel);

  // --- Table ---
  tableWidget = new QTableWidget(this);
  tableWidget->setColumnCount(3);
  tableWidget->setHorizontalHeaderLabels(
      {"Parça Adı", "Kategori", "Ortalama Fiyat (TL)"});
  tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
  tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
  tableWidget->setShowGrid(false);
  tableWidget->verticalHeader()->setVisible(false);
  tableWidget->setFocusPolicy(Qt::NoFocus);
  tableWidget->setEditTriggers(
      QAbstractItemView::NoEditTriggers); // Düzenlemeyi engelle

  // Scrollbar Style - Uygulama temasına uygun
  QString scrollStyle =
      "QScrollBar:vertical { "
      "   border: none; "
      "   background: #2b2e38; "
      "   width: 8px; "
      "   margin: 0px; "
      "   border-radius: 4px; "
      "}"
      "QScrollBar::handle:vertical { "
      "   background: #4facfe; "
      "   min-height: 20px; "
      "   border-radius: 4px; "
      "}"
      "QScrollBar::handle:vertical:hover { "
      "   background: #00f2fe; "
      "}"
      "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { "
      "   height: 0px; "
      "}"
      "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { "
      "   background: none; "
      "}";

  tableWidget->setStyleSheet("QTableWidget { "
                             "   background-color: #1e2029; "
                             "   border: 1px solid #3c404d; "
                             "   border-radius: 10px; "
                             "   color: #e0e0e0; "
                             "   gridline-color: transparent; "
                             "   font-size: 13px; "
                             "}"
                             "QHeaderView::section { "
                             "   background-color: #2b2e38; "
                             "   color: #bec3cd; "
                             "   padding: 10px; "
                             "   border: none; "
                             "   border-bottom: 1px solid #3c404d; "
                             "   font-weight: bold; "
                             "   font-size: 13px; "
                             "}"
                             "QTableWidget::item { "
                             "   padding: 10px; "
                             "   border-bottom: 1px solid #3c404d; "
                             "}"
                             "QTableWidget::item:selected { "
                             "   background-color: #3c404d; "
                             "   color: #4facfe; "
                             "}"
                             "QTableWidget::item:hover { "
                             "   background-color: #363a45; "
                             "}" +
                             scrollStyle);

  layout->addWidget(tableWidget);

  // --- Buttons ---
  QHBoxLayout *btnLayout = new QHBoxLayout();
  btnLayout->addStretch();

  QPushButton *cancelBtn = new QPushButton("İptal", this);
  cancelBtn->setCursor(Qt::PointingHandCursor);
  cancelBtn->setStyleSheet("QPushButton { "
                           "   background-color: transparent; "
                           "   color: #bec3cd; "
                           "   border: 1px solid #3c404d; "
                           "   border-radius: 8px; "
                           "   padding: 8px 20px; "
                           "   font-weight: bold; "
                           "   font-size: 12px; "
                           "}"
                           "QPushButton:hover { "
                           "   color: white; "
                           "   border-color: #4facfe; "
                           "   background-color: #3c404d; "
                           "}");
  connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

  QPushButton *selectBtn = new QPushButton("Seç ve Oluştur", this);
  selectBtn->setCursor(Qt::PointingHandCursor);
  selectBtn->setStyleSheet(
      "QPushButton { "
      "   background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, "
      "       stop:0 #4facfe, stop:1 #00f2fe); "
      "   color: white; "
      "   border-radius: 8px; "
      "   padding: 8px 25px; "
      "   font-weight: bold; "
      "   font-size: 12px; "
      "   border: none; "
      "}"
      "QPushButton:hover { "
      "   background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, "
      "       stop:0 #00f2fe, stop:1 #4facfe); "
      "}");
  connect(selectBtn, &QPushButton::clicked, this,
          &BagisIstegiDialog::onSelectClicked);

  btnLayout->addWidget(cancelBtn);
  btnLayout->addSpacing(10);
  btnLayout->addWidget(selectBtn);
  layout->addLayout(btnLayout);
}

void BagisIstegiDialog::populateTable() {
  QList<Part> parts = {{"NVIDIA GeForce RTX 4090", "Ekran Kartı", 75000},
                       {"NVIDIA GeForce RTX 4080", "Ekran Kartı", 45000},
                       {"AMD Radeon RX 7900 XTX", "Ekran Kartı", 40000},
                       {"Intel Core i9-14900K", "İşlemci", 22000},
                       {"AMD Ryzen 9 7950X3D", "İşlemci", 20000},
                       {"Intel Core i7-14700K", "İşlemci", 15000},
                       {"Corsair Dominator Platinum 64GB", "RAM", 12000},
                       {"G.Skill Trident Z5 RGB 32GB", "RAM", 6000},
                       {"Kingston Fury Beast 32GB", "RAM", 4500}};

  tableWidget->setRowCount(parts.size());
  for (int i = 0; i < parts.size(); ++i) {
    QTableWidgetItem *nameItem = new QTableWidgetItem(parts[i].name);
    QTableWidgetItem *catItem = new QTableWidgetItem(parts[i].category);
    QTableWidgetItem *priceItem =
        new QTableWidgetItem(QString("%1 TL").arg(parts[i].price));

    // Tüm öğeleri düzenlenemez yap
    nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
    catItem->setFlags(catItem->flags() & ~Qt::ItemIsEditable);
    priceItem->setFlags(priceItem->flags() & ~Qt::ItemIsEditable);

    // Center align category and price
    catItem->setTextAlignment(Qt::AlignCenter);
    priceItem->setTextAlignment(Qt::AlignCenter);

    // Kategori renklerini ayarla
    if (parts[i].category == "Ekran Kartı") {
      catItem->setForeground(QColor("#4facfe"));
      priceItem->setForeground(QColor("#ffd93d"));
    } else if (parts[i].category == "İşlemci") {
      catItem->setForeground(QColor("#6bcb77"));
      priceItem->setForeground(QColor("#ffd93d"));
    } else if (parts[i].category == "RAM") {
      catItem->setForeground(QColor("#ff6b6b"));
      priceItem->setForeground(QColor("#ffd93d"));
    }

    tableWidget->setItem(i, 0, nameItem);
    tableWidget->setItem(i, 1, catItem);
    tableWidget->setItem(i, 2, priceItem);
  }

  // Satır yüksekliğini ayarla
  tableWidget->verticalHeader()->setDefaultSectionSize(45);
}

void BagisIstegiDialog::onSelectClicked() {
  int row = tableWidget->currentRow();
  if (row < 0) {
    ModernMessageBox::warning(this, "Uyarı", "Lütfen bir parça seçiniz.");
    return;
  }

  selectedPart.name = tableWidget->item(row, 0)->text();
  selectedPart.category = tableWidget->item(row, 1)->text();
  // "75000 TL" formatından sadece sayıyı al
  QString priceStr = tableWidget->item(row, 2)->text();
  priceStr.replace(" TL", "").replace(".", ""); // Nokta varsa temizle
  selectedPart.price = priceStr.toInt();

  accept();
}

BagisIstegiDialog::Part BagisIstegiDialog::getSelectedPart() const {
  return selectedPart;
}
