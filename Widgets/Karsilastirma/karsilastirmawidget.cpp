#include "karsilastirmawidget.h"
#include "../../Network/networkmanager.h"
#include "ui_karsilastirmawidget.h"
#include <QDebug>
#include <QListWidgetItem>
#include <QMap>
#include <QMessageBox>
#include <QVariant>

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
      "   outline: none;"
      "}"
      "QPushButton:hover { background-color: qlineargradient(spread:pad, "
      "x1:0, y1:0, x2:1, y2:0, stop:0 #00f2fe, stop:1 #4facfe); }");
  ui->karsilastir_buton->setCursor(Qt::PointingHandCursor);

  // Başlangıçta puanları 0 yap
  ui->sizin_puan_deger->setText("0 pts");
  ui->rakip_puan_deger->setText("0 pts");

  setupComparisonChart();

  updateSizinSisteminizUI(); // Başlangıç durumunu ayarla
}

KarsilastirmaWidget::~KarsilastirmaWidget() { delete ui; }

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
    QString username = rival["kullanici_adi"].toString();

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
    ui->sistem_listesi->addItem(item); // <-- EKLENDİ
  }
}

void KarsilastirmaWidget::showLoginWarning() {
  if (!ui->sistem_listesi)
    return;

  ui->sistem_listesi->clear();

  QListWidgetItem *item =
      new QListWidgetItem("Rakipleri görmek için lütfen giriş yapınız.");
  item->setTextAlignment(Qt::AlignCenter);
  // Kullanıcının tıklamasını engellemek için flags ayarlayabiliriz
  item->setFlags(Qt::ItemIsEnabled);

  ui->sistem_listesi->addItem(item);
}

void KarsilastirmaWidget::on_karsilastir_buton_clicked() {
  QListWidgetItem *item = ui->sistem_listesi->currentItem();
  if (!item) {
    // Seçili öğe yoksa uyarı verilebilir veya işlem yapılmaz
    return;
  }

  // Item içinden saklanan veriyi al
  QVariantMap rivalData = item->data(Qt::UserRole).toMap();

  QString username = rivalData["kullanici_adi"].toString();
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
