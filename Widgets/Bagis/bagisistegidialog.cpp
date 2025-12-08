#include "bagisistegidialog.h"
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
  resize(700, 500);
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
  mainLayout->setContentsMargins(0, 0, 0, 0);

  // Main Frame (Rounded & Dark)
  QFrame *frame = new QFrame(this);
  frame->setObjectName("mainFrame");
  frame->setStyleSheet(
      "#mainFrame { background-color: #2b2e38; border: 1px solid #3c404d; "
      "border-radius: 15px; }");
  mainLayout->addWidget(frame);

  QVBoxLayout *layout = new QVBoxLayout(frame);
  layout->setSpacing(20);
  layout->setContentsMargins(20, 20, 20, 20);

  // --- Title Bar ---
  QHBoxLayout *titleLayout = new QHBoxLayout();

  QLabel *titleLabel = new QLabel("Bağış İsteği Oluştur");
  titleLabel->setStyleSheet(
      "color: white; font-size: 18px; font-weight: bold;");

  QPushButton *closeBtn = new QPushButton("X");
  closeBtn->setFixedSize(30, 30);
  closeBtn->setCursor(Qt::PointingHandCursor);
  closeBtn->setStyleSheet(
      "QPushButton { background-color: transparent; color: #bec3cd; border: "
      "none; font-weight: bold; font-size: 16px; }"
      "QPushButton:hover { color: #ff4d4d; }");
  connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);

  titleLayout->addWidget(titleLabel);
  titleLayout->addStretch();
  titleLayout->addWidget(closeBtn);
  layout->addLayout(titleLayout);

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
  tableWidget->setFocusPolicy(Qt::NoFocus); // Remove dotted selection line

  // Custom Scrollbar Style (Same as BagisWidget)
  QString scrollStyle =
      "QScrollBar:vertical { border: none; background: #2b2e38; width: 8px; "
      "margin: 0px; border-radius: 4px; }"
      "QScrollBar::handle:vertical { background: #4facfe; min-height: 20px; "
      "border-radius: 4px; }"
      "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: "
      "0px; }"
      "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { "
      "background: none; }";

  tableWidget->setStyleSheet("QTableWidget { "
                             "   background-color: #1e1e1e; "
                             "   border: none; "
                             "   border-radius: 10px; "
                             "   color: #e0e0e0; "
                             "   gridline-color: transparent; "
                             "}"
                             "QHeaderView::section { "
                             "   background-color: #2b2e38; "
                             "   color: #bec3cd; "
                             "   padding: 10px; "
                             "   border: none; "
                             "   font-weight: bold; "
                             "   font-size: 13px;"
                             "}"
                             "QTableWidget::item { "
                             "   padding: 10px; "
                             "   border-bottom: 1px solid #2b2e38; "
                             "}"
                             "QTableWidget::item:selected { "
                             "   background-color: #3c404d; "
                             "   color: white; "
                             "}"
                             "QTableWidget::item:hover { "
                             "   background-color: #2c2c2c; "
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
                           "   border-radius: 5px; "
                           "   padding: 8px 20px; "
                           "   font-weight: bold;"
                           "}"
                           "QPushButton:hover { "
                           "   color: white; "
                           "   border-color: white; "
                           "}");
  connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

  QPushButton *selectBtn = new QPushButton("Seç ve Oluştur", this);
  selectBtn->setCursor(Qt::PointingHandCursor);
  selectBtn->setStyleSheet("QPushButton { "
                           "   background-color: #4facfe; "
                           "   color: white; "
                           "   border-radius: 5px; "
                           "   padding: 8px 25px; "
                           "   font-weight: bold; "
                           "   border: none;"
                           "}"
                           "QPushButton:hover { "
                           "   background-color: #00f2fe; "
                           "}");
  connect(selectBtn, &QPushButton::clicked, this,
          &BagisIstegiDialog::onSelectClicked);

  btnLayout->addWidget(cancelBtn);
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

    // Center align category and price
    catItem->setTextAlignment(Qt::AlignCenter);
    priceItem->setTextAlignment(Qt::AlignCenter);

    tableWidget->setItem(i, 0, nameItem);
    tableWidget->setItem(i, 1, catItem);
    tableWidget->setItem(i, 2, priceItem);
  }
}

void BagisIstegiDialog::onSelectClicked() {
  int row = tableWidget->currentRow();
  if (row < 0) {
    QMessageBox::warning(this, "Uyarı", "Lütfen bir parça seçiniz.");
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
