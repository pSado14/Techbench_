#include "liderlikwidget.h"
#include <QBrush>
#include <QColor>
#include <QFont>

LiderlikWidget::LiderlikWidget(QWidget *parent) : QWidget(parent) { setupUi(); }

void LiderlikWidget::setupUi() {
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setContentsMargins(20, 20, 20, 20);
  layout->setSpacing(15);

  // Title
  QLabel *titleLabel = new QLabel("Global Liderlik Tablosu", this);
  titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: "
                            "#ffffff; margin-bottom: 10px;");
  layout->addWidget(titleLabel);

  // Table
  tableWidget = new QTableWidget(this);
  tableWidget->setColumnCount(9);
  tableWidget->setHorizontalHeaderLabels({"Sıra", "Kullanıcı", "Toplam Puan",
                                          "CPU", "CPU Puan", "GPU", "GPU Puan",
                                          "RAM", "RAM Puan"});
  tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  tableWidget->horizontalHeader()->setSectionResizeMode(
      0, QHeaderView::ResizeToContents); // Rank
  tableWidget->horizontalHeader()->setSectionResizeMode(
      1, QHeaderView::ResizeToContents); // User
  tableWidget->horizontalHeader()->setSectionResizeMode(
      2, QHeaderView::ResizeToContents); // Total Score
  tableWidget->horizontalHeader()->setSectionResizeMode(
      4, QHeaderView::ResizeToContents); // CPU Score
  tableWidget->horizontalHeader()->setSectionResizeMode(
      6, QHeaderView::ResizeToContents); // GPU Score
  tableWidget->horizontalHeader()->setSectionResizeMode(
      8, QHeaderView::ResizeToContents); // RAM Score
  tableWidget->verticalHeader()->setVisible(false);
  tableWidget->verticalHeader()->setDefaultSectionSize(
      60);                        // Increase row height
  tableWidget->setWordWrap(true); // Enable word wrap
  tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
  tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
  tableWidget->setAlternatingRowColors(true);
  tableWidget->setStyleSheet(
      "QTableWidget { background-color: #2b2e38; color: white; border: none; "
      "border-radius: 8px; gridline-color: #3c404d; "
      "alternate-background-color: #323642; }"
      "QHeaderView::section { background-color: #3c404d; color: white; "
      "padding: 8px; border: none; font-weight: bold; }"
      "QTableWidget::item { padding: 5px; }"
      "QTableWidget::item:selected { background-color: #4facfe; color: white; "
      "}");
  layout->addWidget(tableWidget);

  // Refresh Button
  refreshButton = new QPushButton("Listeyi Yenile", this);
  refreshButton->setCursor(Qt::PointingHandCursor);
  refreshButton->setFixedWidth(150);
  refreshButton->setStyleSheet(
      "QPushButton { background-color: #4facfe; color: white; border-radius: "
      "8px; padding: 10px 20px; font-weight: bold; }"
      "QPushButton:hover { background-color: #00f2fe; }");
  connect(refreshButton, &QPushButton::clicked, this,
          &LiderlikWidget::refreshRequested);

  QHBoxLayout *btnLayout = new QHBoxLayout();
  btnLayout->addStretch();
  btnLayout->addWidget(refreshButton);
  layout->addLayout(btnLayout);
}

void LiderlikWidget::updateLeaderboard(const QList<QVariantMap> &data) {
  tableWidget->setRowCount(0);
  int rank = 1;
  for (const auto &user : data) {
    int row = tableWidget->rowCount();
    tableWidget->insertRow(row);

    QTableWidgetItem *rankItem = new QTableWidgetItem(QString::number(rank));
    rankItem->setTextAlignment(Qt::AlignCenter);

    // Top 3 styling
    if (rank == 1)
      rankItem->setForeground(QBrush(QColor("#FFD700"))); // Gold
    else if (rank == 2)
      rankItem->setForeground(QBrush(QColor("#C0C0C0"))); // Silver
    else if (rank == 3)
      rankItem->setForeground(QBrush(QColor("#CD7F32"))); // Bronze

    QTableWidgetItem *userItem =
        new QTableWidgetItem(user["kullanici_adi"].toString());
    userItem->setToolTip(user["kullanici_adi"].toString());
    userItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    QTableWidgetItem *scoreItem =
        new QTableWidgetItem(QString::number(user["score"].toInt()));
    scoreItem->setTextAlignment(Qt::AlignCenter);
    scoreItem->setFont(QFont("Segoe UI", 9, QFont::Bold));

    QTableWidgetItem *cpuItem = new QTableWidgetItem(user["cpu"].toString());
    cpuItem->setToolTip(user["cpu"].toString());
    cpuItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    QTableWidgetItem *cpuScoreItem =
        new QTableWidgetItem(QString::number(user["cpu_score"].toInt()));
    cpuScoreItem->setTextAlignment(Qt::AlignCenter);

    QTableWidgetItem *gpuItem = new QTableWidgetItem(user["gpu"].toString());
    gpuItem->setToolTip(user["gpu"].toString());
    gpuItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    QTableWidgetItem *gpuScoreItem =
        new QTableWidgetItem(QString::number(user["gpu_score"].toInt()));
    gpuScoreItem->setTextAlignment(Qt::AlignCenter);

    QTableWidgetItem *ramItem = new QTableWidgetItem(user["ram"].toString());
    ramItem->setToolTip(user["ram"].toString());
    ramItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    QTableWidgetItem *ramScoreItem =
        new QTableWidgetItem(QString::number(user["ram_score"].toInt()));
    ramScoreItem->setTextAlignment(Qt::AlignCenter);

    tableWidget->setItem(row, 0, rankItem);
    tableWidget->setItem(row, 1, userItem);
    tableWidget->setItem(row, 2, scoreItem);
    tableWidget->setItem(row, 3, cpuItem);
    tableWidget->setItem(row, 4, cpuScoreItem);
    tableWidget->setItem(row, 5, gpuItem);
    tableWidget->setItem(row, 6, gpuScoreItem);
    tableWidget->setItem(row, 7, ramItem);
    tableWidget->setItem(row, 8, ramScoreItem);

    rank++;
  }
}
