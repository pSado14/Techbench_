#include "bagisistegidialog.h"
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

BagisIstegiDialog::BagisIstegiDialog(QWidget *parent) : QDialog(parent) {
  setWindowTitle("Bağış İsteği Oluştur");
  resize(600, 400);
  setStyleSheet("background-color: #121212; color: white;");
  setupUi();
  populateTable();
}

BagisIstegiDialog::~BagisIstegiDialog() {}

void BagisIstegiDialog::setupUi() {
  QVBoxLayout *mainLayout = new QVBoxLayout(this);

  // Table
  tableWidget = new QTableWidget(this);
  tableWidget->setColumnCount(3);
  tableWidget->setHorizontalHeaderLabels(
      {"Parça Adı", "Kategori", "Ortalama Fiyat (Puan)"});
  tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
  tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
  tableWidget->setStyleSheet(
      "QTableWidget { background-color: #1e1e1e; border: none; gridline-color: "
      "#333; }"
      "QHeaderView::section { background-color: #2c2c2c; padding: 5px; border: "
      "none; font-weight: bold; }"
      "QTableWidget::item { padding: 5px; }"
      "QTableWidget::item:selected { background-color: #4facfe; }");
  mainLayout->addWidget(tableWidget);

  // Buttons
  QHBoxLayout *btnLayout = new QHBoxLayout();
  QPushButton *cancelBtn = new QPushButton("İptal", this);
  cancelBtn->setStyleSheet("background-color: #333; color: white; "
                           "border-radius: 5px; padding: 8px;");
  connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

  QPushButton *selectBtn = new QPushButton("Seç ve Oluştur", this);
  selectBtn->setStyleSheet(
      "background-color: #4facfe; color: white; border-radius: 5px; padding: "
      "8px; font-weight: bold;");
  connect(selectBtn, &QPushButton::clicked, this,
          &BagisIstegiDialog::onSelectClicked);

  btnLayout->addStretch();
  btnLayout->addWidget(cancelBtn);
  btnLayout->addWidget(selectBtn);
  mainLayout->addLayout(btnLayout);
}

void BagisIstegiDialog::populateTable() {
  QList<Part> parts = {{"NVIDIA GeForce RTX 4090", "Ekran Kartı", 25000},
                       {"Intel Core i9-14900K", "İşlemci", 15000},
                       {"Corsair Vengeance RGB 32GB", "RAM", 4000}};

  tableWidget->setRowCount(parts.size());
  for (int i = 0; i < parts.size(); ++i) {
    tableWidget->setItem(i, 0, new QTableWidgetItem(parts[i].name));
    tableWidget->setItem(i, 1, new QTableWidgetItem(parts[i].category));
    tableWidget->setItem(i, 2,
                         new QTableWidgetItem(QString::number(parts[i].price)));
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
  selectedPart.price = tableWidget->item(row, 2)->text().toInt();

  accept();
}

BagisIstegiDialog::Part BagisIstegiDialog::getSelectedPart() const {
  return selectedPart;
}
