#include "karsilastirmawidget.h"
#include "ui_karsilastirmawidget.h"
#include <QDebug>

KarsilastirmaWidget::KarsilastirmaWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::KarsilastirmaWidget) {
  ui->setupUi(this);

  // Modern Blue Gradient for "Karşılaştır"
  ui->karsilastir_buton->setStyleSheet(
      "QPushButton { "
      "   background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, "
      "y2:0, stop:0 #4facfe, stop:1 #00f2fe); "
      "   border-radius: 8px; "
      "   color: white; "
      "   padding: 10px 20px; "
      "   font-weight: bold; "
      "   font-size: 14px; "
      "   border: none; "
      "}"
      "QPushButton:hover { background-color: qlineargradient(spread:pad, "
      "x1:0, y1:0, x2:1, y2:0, stop:0 #00f2fe, stop:1 #4facfe); }");

  // Başlangıçta puanları 0 yap
  ui->sizin_puan_deger->setText("0 pts");
  ui->rakip_puan_deger->setText("0 pts");

  setupComparisonChart();
  setupHistoryList();        // <-- YENİ
  updateSizinSisteminizUI(); // Başlangıç durumunu ayarla
}

KarsilastirmaWidget::~KarsilastirmaWidget() { delete ui; }

void KarsilastirmaWidget::setupHistoryList() {
  // Detaylı Sonuçlar Frame'inin layout'unu al
  QVBoxLayout *layout =
      qobject_cast<QVBoxLayout *>(ui->detayli_sonuclar_frame->layout());
  if (!layout)
    return;

  // Başlık
  QLabel *historyLabel = new QLabel("Sonuç Geçmişi");
  historyLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: "
                              "white; margin-top: 20px; margin-bottom: 5px;");
  layout->addWidget(historyLabel);

  // Liste
  m_historyList = new QListWidget();
  m_historyList->setStyleSheet(
      "QListWidget { background-color: #2b2e38; border: none; color: #e0e0e0; }"
      "QListWidget::item { padding: 8px; border-bottom: 1px solid #3c404d; }"
      "QListWidget::item:hover { background-color: #363a45; }");
  m_historyList->setFixedHeight(200); // Yükseklik sınırı
  layout->addWidget(m_historyList);

  // Geçmişi Temizle Butonu
  QPushButton *clearHistoryBtn = new QPushButton("Geçmişi Temizle");
  clearHistoryBtn->setStyleSheet(
      "QPushButton { "
      "   background-color: #dc3545; "
      "   border-radius: 5px; "
      "   color: white; "
      "   padding: 5px 10px; "
      "   font-weight: bold; "
      "   border: none; "
      "   margin-top: 5px; "
      "}"
      "QPushButton:hover { background-color: #c82333; }");
  layout->addWidget(clearHistoryBtn);

  connect(clearHistoryBtn, &QPushButton::clicked, this,
          &KarsilastirmaWidget::on_gecmisi_temizle_buton_clicked);
}

void KarsilastirmaWidget::updateHistoryList(const QList<QVariantMap> &history) {
  if (!m_historyList)
    return;
  m_historyList->clear();

  for (const QVariantMap &entry : history) {
    QString date = entry["created_at"].toDateTime().toString("yyyy-MM-dd");
    // Eğer tarih string geliyorsa (ISO format), dönüşüm gerekebilir.
    // Backend TIMESTAMP gönderiyor, genelde string gelir.
    QString dateStr = entry["created_at"].toString();
    if (dateStr.contains("T"))
      dateStr = dateStr.split("T")[0]; // Basit parse

    int score = entry["score"].toInt();

    QString itemText = QString("%1: - %2 pts").arg(dateStr).arg(score);
    m_historyList->addItem(itemText);
  }
}

void KarsilastirmaWidget::on_gecmisi_temizle_buton_clicked() {
  if (m_username.isEmpty()) {
    QMessageBox::warning(this, "Hata", "Önce giriş yapmalısınız.");
    return;
  }

  QMessageBox::StandardButton reply;
  reply = QMessageBox::question(
      this, "Geçmişi Sil",
      "Tüm test geçmişinizi silmek istediğinize emin misiniz?",
      QMessageBox::Yes | QMessageBox::No);
  if (reply == QMessageBox::No)
    return;

  NetworkManager *nm = new NetworkManager(this);
  nm->deleteScoreHistory(m_username, [this, nm](bool success, QString message) {
    if (success) {
      QMessageBox::information(this, "Başarılı", message);
      if (m_historyList)
        m_historyList->clear(); // Listeyi temizle
    } else {
      QMessageBox::critical(this, "Hata", message);
    }
    nm->deleteLater();
  });
}

void KarsilastirmaWidget::setupComparisonChart() {
  m_series = new QBarSeries();

  m_mySet = new QBarSet("Sizin Sisteminiz");
  m_mySet->setColor(QColor("#007bff")); // Mavi
  *m_mySet << 0;

  m_rivalSet = new QBarSet("Rakip Sistem");
  m_rivalSet->setColor(QColor("#dc3545")); // Kırmızı
  *m_rivalSet << 0;

  m_series->append(m_mySet);
  m_series->append(m_rivalSet);

  m_chart = new QChart();
  m_chart->addSeries(m_series);
  m_chart->setTitle("Performans Karşılaştırması");
  m_chart->setAnimationOptions(QChart::SeriesAnimations);
  m_chart->setTheme(QChart::ChartThemeDark);
  m_chart->legend()->setVisible(true);
  m_chart->legend()->setAlignment(Qt::AlignBottom);

  QStringList categories;
  categories << "Toplam Puan";
  m_axisX = new QBarCategoryAxis();
  m_axisX->append(categories);
  m_chart->addAxis(m_axisX, Qt::AlignBottom);
  m_series->attachAxis(m_axisX);

  m_axisY = new QValueAxis();
  m_axisY->setRange(0, 50000); // Varsayılan aralık
  m_chart->addAxis(m_axisY, Qt::AlignLeft);
  m_series->attachAxis(m_axisY);

  m_chartView = new QChartView(m_chart);
  m_chartView->setRenderHint(QPainter::Antialiasing);

  // Layout'a ekle
  if (ui->detayli_grafik_tutucu->layout()) {
    delete ui->detayli_grafik_tutucu->layout();
  }
  QVBoxLayout *layout = new QVBoxLayout(ui->detayli_grafik_tutucu);
  layout->addWidget(m_chartView);

  // Placeholder'ı gizle
  if (ui->placeholder_detayli_grafik) {
    ui->placeholder_detayli_grafik->hide();
  }
}

void KarsilastirmaWidget::updateChart(int rivalScore) {
  if (!m_mySet || !m_rivalSet || !m_axisY)
    return;

  m_mySet->replace(0, m_myScore);
  m_rivalSet->replace(0, rivalScore);

  // Y eksenini dinamik ayarla
  int maxScore = std::max(m_myScore, rivalScore);
  if (maxScore == 0)
    maxScore = 10000;
  m_axisY->setRange(0, maxScore * 1.2); // %20 boşluk bırak
}

void KarsilastirmaWidget::setSizinSisteminiz(QString cpu, QString gpu,
                                             QString ram) {
  m_cpu = cpu;
  m_gpu = gpu;
  m_ram = ram;
  updateSizinSisteminizUI();
}

void KarsilastirmaWidget::setKullaniciBilgileri(QString username) {
  m_username = username;
  updateSizinSisteminizUI();
}

void KarsilastirmaWidget::updateSizinSisteminizUI() {
  QString displayName = m_username.isEmpty() ? "Misafir Kullanıcı" : m_username;

  // Başlık kısmını güncelle (Sizin Sisteminiz yerine Kullanıcı Adı)
  if (ui->sizin_sisteminiz_baslik) {
    ui->sizin_sisteminiz_baslik->setText(
        QString("<html><head/><body><p><span style=\" font-size:12pt; "
                "font-weight:600; color:#ffffff;\">%1</span></p></body></html>")
            .arg(displayName));
  }

  QString systemInfo;
  if (m_cpu.isEmpty() || m_gpu.isEmpty()) {
    systemInfo = "Sistem Taranmadı";
  } else {
    systemInfo =
        QString("CPU: %1\nGPU: %2\nRAM: %3").arg(m_cpu).arg(m_gpu).arg(m_ram);
  }

  if (ui->sizin_sisteminiz_detay) {
    ui->sizin_sisteminiz_detay->setText(systemInfo);
  }
}

void KarsilastirmaWidget::setSizinPuaniniz(int totalScore) {
  m_myScore = totalScore;
  if (ui->sizin_puan_deger) {
    ui->sizin_puan_deger->setText(QString::number(totalScore) + " pts");
  }
  updateChart(0); // Rakip henüz seçilmedi
}

void KarsilastirmaWidget::updateRivalsList(const QList<QVariantMap> &rivals) {
  if (!ui->sistem_listesi)
    return;

  ui->sistem_listesi->clear();

  for (const QVariantMap &rival : rivals) {
    QString username = rival["username"].toString();

    // Mevcut kullanıcıyı listede gösterme
    if (username == m_username) {
      continue;
    }
    QString cpu = rival["cpu"].toString();
    QString gpu = rival["gpu"].toString();
    QString ram = rival["ram"].toString(); // RAM bilgisini al
    int score = rival["score"].toInt();

    // Liste formatı: " ( ) Username (CPU, GPU) - (Score pts) | Seç"
    QString itemText = QString(" ( ) %1 (%2, %3) - (%4 pts) | Seç")
                           .arg(username)
                           .arg(cpu)
                           .arg(gpu)
                           .arg(score);

    QListWidgetItem *item = new QListWidgetItem(itemText);
    // Tüm veriyi item içine sakla (Parsing ile uğraşmamak için)
    item->setData(Qt::UserRole, rival);

    ui->sistem_listesi->addItem(item);
  }
}

void KarsilastirmaWidget::on_karsilastir_buton_clicked() {
  QListWidgetItem *item = ui->sistem_listesi->currentItem();
  if (!item) {
    // Seçili öğe yoksa uyarı verilebilir veya işlem yapılmaz
    return;
  }

  // Item içinden saklanan veriyi al
  QVariantMap rivalData = item->data(Qt::UserRole).toMap();

  QString username = rivalData["username"].toString();
  QString cpu = rivalData["cpu"].toString();
  QString gpu = rivalData["gpu"].toString();
  QString ram = rivalData["ram"].toString();
  int score = rivalData["score"].toInt();

  // UI Güncelle - "Sizin Sisteminiz" ile aynı formatta
  if (ui->label) { // Rakip Sistem Detayı
    QString formattedText =
        QString("CPU: %1\nGPU: %2\nRAM: %3").arg(cpu).arg(gpu).arg(ram);
    ui->label->setText(formattedText);
  }

  // Başlığı da güncelle (Rakip Kullanıcı Adı)
  if (ui->rakip_sistem_baslik) {
    ui->rakip_sistem_baslik->setText(
        QString("<html><head/><body><p><span style=\" font-size:12pt; "
                "font-weight:600; color:#ffffff;\">%1</span></p></body></html>")
            .arg(username));
  }

  if (ui->rakip_puan_deger) {
    ui->rakip_puan_deger->setText(QString::number(score) + " pts");
  }

  updateChart(score);
}

void KarsilastirmaWidget::on_arama_kutusu_textChanged(const QString &arg1) {
  QString filter = arg1.trimmed();

  for (int i = 0; i < ui->sistem_listesi->count(); ++i) {
    QListWidgetItem *item = ui->sistem_listesi->item(i);
    // Item metninde arama yap (Case Insensitive)
    if (item->text().contains(filter, Qt::CaseInsensitive)) {
      item->setHidden(false);
    } else {
      item->setHidden(true);
    }
  }
}
