#include "anasayfawidget.h"
#include "ui_anasayfawidget.h"
#include <QDebug>
#include <QMessageBox>
#include <QPainter>
#include <QProcess>
#include <QRegularExpression>
#include <QVBoxLayout>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QtCharts/QStackedBarSeries>
#include <QtCharts/QValueAxis>

AnasayfaWidget::AnasayfaWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::AnasayfaWidget) {
  ui->setupUi(this);

  setupCharts(); // <-- Charts kurulumu
  bilgileriSifirla();
}

AnasayfaWidget::~AnasayfaWidget() { delete ui; }

// --- DONANIM BİLGİSİ ÇEKME FONKSİYONU ---
QString AnasayfaWidget::getHardwareInfo(const QString &cmd,
                                        const QStringList &args) {
  QProcess process;
  process.start(cmd, args);
  process.waitForFinished();

  QString result = QString::fromLocal8Bit(process.readAllStandardOutput());
  QStringList lines =
      result.split(QRegularExpression("[\r\n]+"), Qt::SkipEmptyParts);

  for (const QString &line : lines) {
    QString cleanLine = line.trimmed();
    if (cleanLine.isEmpty())
      continue;
    // Başlıkları atla
    if (cleanLine.startsWith("Name", Qt::CaseInsensitive))
      continue;
    if (cleanLine.startsWith("TotalPhysicalMemory", Qt::CaseInsensitive))
      continue;
    if (cleanLine.startsWith("NumberOfCores", Qt::CaseInsensitive))
      continue;
    if (cleanLine.startsWith("MaxClockSpeed", Qt::CaseInsensitive))
      continue;
    if (cleanLine.startsWith("AdapterRAM", Qt::CaseInsensitive))
      continue;
    if (cleanLine.startsWith("Speed", Qt::CaseInsensitive))
      continue;
    if (cleanLine.startsWith("Caption", Qt::CaseInsensitive))
      continue;
    if (cleanLine.startsWith("OSArchitecture", Qt::CaseInsensitive))
      continue;
    if (cleanLine.startsWith("Manufacturer", Qt::CaseInsensitive))
      continue;
    if (cleanLine.startsWith("PartNumber", Qt::CaseInsensitive))
      continue;

    return cleanLine; // İlk geçerli veri
  }
  return "0"; // Bulunamazsa 0 döndür
}

void AnasayfaWidget::on_sistemibilgilerinitarabuton_clicked() {
  taraVeGuncelle();
}

void AnasayfaWidget::taraVeGuncelle() {
  ui->sistemibilgilerinitarabuton->setText("Detaylar Taranıyor...");
  ui->sistemibilgilerinitarabuton->setEnabled(false);
  qApp->processEvents();

  // ==========================
  // 1. CPU (İŞLEMCİ) BİLGİLERİ
  // ==========================

  QString cpuName = getHardwareInfo("wmic", {"cpu", "get", "name"});
  ui->cpu_model_label->setText(cpuName);

  QString coreCount = getHardwareInfo("wmic", {"cpu", "get", "NumberOfCores"});
  ui->cpu_cores_label->setText(coreCount);

  QString cpuSpeedRaw =
      getHardwareInfo("wmic", {"cpu", "get", "MaxClockSpeed"});
  if (cpuSpeedRaw != "0") {
    double speedGHz = cpuSpeedRaw.toDouble() / 1000.0;
    ui->cpu_clock_label->setText(QString::number(speedGHz, 'f', 1) + " GHz");
  }

  // ==========================
  // 2. GPU (EKRAN KARTI) BİLGİLERİ
  // ==========================

  QString gpuName =
      getHardwareInfo("wmic", {"path", "win32_videocontroller", "get", "name"});
  ui->gpu_model_label->setText(gpuName);

  QString vramRaw = getHardwareInfo(
      "wmic", {"path", "win32_videocontroller", "get", "AdapterRAM"});
  if (vramRaw != "0") {
    double vramBytes = vramRaw.toDouble();
    if (vramBytes > 0) {
      double vramGB = vramBytes / (1024.0 * 1024.0 * 1024.0);
      ui->gpu_vram_label->setText(QString::number(qRound(vramGB)) + " GB");
    } else {
      ui->gpu_vram_label->setText("Paylaşımlı");
    }
  }

  // NVIDIA GPU Clock Hızı (nvidia-smi ile)
  QString gpuClock = getHardwareInfo(
      "nvidia-smi", {"--query-gpu=clocks.gr", "--format=csv,noheader"});
  if (gpuClock != "0" && !gpuClock.isEmpty()) {
    ui->gpu_clock_label->setText(gpuClock);
  } else {
    ui->gpu_clock_label->setText("-");
  }

  // ==========================
  // 3. RAM (BELLEK) BİLGİLERİ
  // ==========================

  QString ramRaw =
      getHardwareInfo("wmic", {"computersystem", "get", "totalphysicalmemory"});
  if (ramRaw != "0") {
    double ramBytes = ramRaw.toDouble();
    double ramGB = ramBytes / (1024.0 * 1024.0 * 1024.0);

    // RAM Modül Sayısını Bul
    QString ramModulesRaw =
        getHardwareInfo("wmic", {"memorychip", "get", "Capacity"});
    // wmic çıktısı her modül için bir satır döndürür (başlık hariç).
    // getHardwareInfo sadece ilk satırı döndürdüğü için burada özel bir işlem
    // yapmamız lazım. Ancak getHardwareInfo'yu değiştirmek yerine, wmic
    // çıktısını satır satır sayan yeni bir yaklaşım kullanabiliriz. Veya
    // basitçe getHardwareInfo'yu modifiye etmeden, doğrudan QProcess ile
    // sayabiliriz.

    QProcess process;
    process.start("wmic", {"memorychip", "get", "Capacity"});
    process.waitForFinished();
    QString output = QString::fromLocal8Bit(process.readAllStandardOutput());
    QStringList lines =
        output.split(QRegularExpression("[\r\n]+"), Qt::SkipEmptyParts);
    int moduleCount = 0;
    for (const QString &line : lines) {
      if (line.trimmed().isEmpty())
        continue;
      if (line.startsWith("Capacity", Qt::CaseInsensitive))
        continue;
      moduleCount++;
    }

    ui->ram_gb_label->setText(QString::number(qRound(ramGB)) + " GB (" +
                              QString::number(moduleCount) + " Adet)");
  }

  QString ramSpeed = getHardwareInfo("wmic", {"memorychip", "get", "Speed"});
  ui->ram_clock_label->setText(ramSpeed + " MHz");

  // RAM Modelini Çek (Manufacturer + PartNumber)
  QString ramManuf =
      getHardwareInfo("wmic", {"memorychip", "get", "Manufacturer"});
  QString ramPart =
      getHardwareInfo("wmic", {"memorychip", "get", "PartNumber"});

  QString ramModel = "Bilinmiyor";
  if (ramManuf != "0" && ramPart != "0") {
    ramModel = ramManuf + " " + ramPart;
  } else if (ramManuf != "0") {
    ramModel = ramManuf;
  }

  if (ui->ram_model_label_2) {
    ui->ram_model_label_2->setText(ramModel);
  }

  // Fiyatlar (Manuel)
  ui->cprice_label->setText("-");
  ui->g_price_label->setText("-");
  ui->ram_price_label->setText("Fiyat:");
  ui->r_price_label->setText("-");

  // Sinyal Gönder
  QString ramString = "-";
  if (ramModel != "Bilinmiyor") {
    ramString = ramModel;
    // Boyutu da ekleyelim
    if (ramRaw != "0") {
      double ramBytes = ramRaw.toDouble();
      double ramGB = ramBytes / (1024.0 * 1024.0 * 1024.0);
      ramString += " (" + QString::number(qRound(ramGB)) + " GB)";
    }
  } else if (ramRaw != "0") {
    double ramBytes = ramRaw.toDouble();
    double ramGB = ramBytes / (1024.0 * 1024.0 * 1024.0);
    ramString = QString::number(qRound(ramGB)) + " GB";
  }

  emit sistemBilgileriGuncellendi(cpuName, gpuName, ramString);

  // Butonu eski haline getir
  ui->sistemibilgilerinitarabuton->setText(
      "SİSTEM BİLGİLERİNİ GÜNCELLE / TARA");
  ui->sistemibilgilerinitarabuton->setEnabled(true);
}

void AnasayfaWidget::bilgileriSifirla() {
  ui->kullanici_adi_label->setText("Misafir Kullanıcı");

  ui->cpu_model_label->setText("Taranmadı");
  ui->cpu_cores_label->setText("-");
  ui->cpu_clock_label->setText("-");

  ui->gpu_model_label->setText("Taranmadı");
  ui->gpu_vram_label->setText("-");

  ui->ram_gb_label->setText("-");
  ui->ram_gb_label->setText("-");
  ui->ram_clock_label->setText("-");
  if (ui->ram_model_label_2)
    ui->ram_model_label_2->setText("Taranmadı");

  // Puanları da sıfırla
  ui->cpu_score_label->setText("-");
  ui->gpu_score_label->setText("-");
  ui->ram_score_label_2->setText("-");

  m_cpuScore = 0;
  m_gpuScore = 0;
  m_ramScore = 0;
  updateCharts(0, 0, 0);
}

void AnasayfaWidget::setKullaniciBilgileri(const QString &ad) {
  ui->kullanici_adi_label->setText(ad);
}

void AnasayfaWidget::on_hesapSilButon_clicked() { emit hesapSilmeTiklandi(); }

void AnasayfaWidget::setupCharts() { updateCharts(0, 0, 0); }

void AnasayfaWidget::updateCharts(int cpuScore, int gpuScore, int ramScore) {
  // Helper lambda to create a chart for a specific container
  auto createChart = [&](QWidget *container, QString name, int score,
                         QColor color) {
    // Placeholder'ı temizle (Eğer varsa)
    if (container == ui->cpu_grafik_tutucu && ui->placeholder_cpu_grafik) {
      delete ui->placeholder_cpu_grafik;
      ui->placeholder_cpu_grafik = nullptr;
    }
    if (container == ui->gpu_grafik_tutucu && ui->placeholder_gpu_grafik) {
      delete ui->placeholder_gpu_grafik;
      ui->placeholder_gpu_grafik = nullptr;
    }
    if (container == ui->ram_grafik_tutucu && ui->placeholder_ram_grafik) {
      delete ui->placeholder_ram_grafik;
      ui->placeholder_ram_grafik = nullptr;
    }

    // Layout oluştur (Eğer yoksa)
    if (!container->layout()) {
      new QVBoxLayout(container);
    }

    // Layout temizle
    if (container->layout()) {
      QLayoutItem *item;
      while ((item = container->layout()->takeAt(0))) {
        delete item->widget();
        delete item;
      }
    }

    QChart *chart = new QChart();
    chart->setBackgroundVisible(false); // Şeffaf arka plan
    chart->setMargins(QMargins(0, 0, 0, 0));
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->legend()->setVisible(false);

    // 1. Progress Series
    QPieSeries *series = new QPieSeries();
    series->setHoleSize(0.60);
    series->setPieSize(0.80); // Küçük alanda büyük görünmesi için

    const int MAX_SCORE = 20000;

    if (score > 0) {
      QPieSlice *slice = series->append("", score);
      slice->setColor(color);
      slice->setBorderColor(color.lighter());
      slice->setLabelVisible(false);

      int remaining = MAX_SCORE - score;
      if (remaining < 0)
        remaining = 0;

      QPieSlice *empty = series->append("", remaining);
      empty->setColor(QColor("#2b2b2b"));
      empty->setBorderColor(QColor("#2b2b2b"));
      empty->setLabelVisible(false);
    } else {
      // Skor yoksa boş gri halka
      QPieSlice *empty = series->append("", 1);
      empty->setColor(QColor("#2b2b2b"));
      empty->setBorderColor(QColor("#2b2b2b"));
      empty->setLabelVisible(false);
    }
    chart->addSeries(series);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setStyleSheet("background: transparent;");

    container->layout()->addWidget(chartView);
  };

  // CPU Chart
  createChart(ui->cpu_grafik_tutucu, "CPU", cpuScore, QColor("#3498db"));

  // GPU Chart
  createChart(ui->gpu_grafik_tutucu, "GPU", gpuScore, QColor("#e74c3c"));

  // RAM Chart
  createChart(ui->ram_grafik_tutucu, "RAM", ramScore, QColor("#f1c40f"));

  // ==========================================
  // SİSTEM ÖZETİ (GENEL SKOR GAUGE & KARTLAR)
  // ==========================================

  // 1. Toplam Skor ve Durum
  int totalScore = cpuScore + gpuScore + ramScore;
  QString statusText;
  QColor statusColor;

  if (totalScore > 50000) {
    statusText = "Efsanevi";
    statusColor = QColor("#8e44ad");
  } else if (totalScore > 40000) {
    statusText = "Mükemmel";
    statusColor = QColor("#2ecc71");
  } else if (totalScore > 30000) {
    statusText = "Çok İyi";
    statusColor = QColor("#3498db");
  } else if (totalScore > 20000) {
    statusText = "İyi";
    statusColor = QColor("#f1c40f");
  } else if (totalScore > 10000) {
    statusText = "Orta";
    statusColor = QColor("#e67e22");
  } else {
    statusText = "Geliştirilmeli";
    statusColor = QColor("#e74c3c");
  }

  // Durum Label Güncelle
  if (ui->general_score_status_label) {
    ui->general_score_status_label->setText(
        QString("Genel Performans: <span style='color:%1; "
                "font-weight:bold;'>%2</span>")
            .arg(statusColor.name())
            .arg(statusText));
  }

  // 2. Gauge Chart (Halka Grafik)
  if (ui->gauge_chart_container) {
    if (!ui->gauge_chart_container->layout()) {
      new QVBoxLayout(ui->gauge_chart_container);
    }

    // Layout temizle
    QLayoutItem *item;
    while ((item = ui->gauge_chart_container->layout()->takeAt(0))) {
      delete item->widget();
      delete item;
    }

    QChart *gaugeChart = new QChart();
    gaugeChart->setBackgroundVisible(false);
    gaugeChart->setMargins(QMargins(0, 0, 0, 0));
    gaugeChart->legend()->setVisible(false);

    QPieSeries *gaugeSeries = new QPieSeries();
    gaugeSeries->setHoleSize(0.85);
    gaugeSeries->setPieSize(0.90);

    // Dolu Kısım (Toplam Skor)
    QPieSlice *scoreSlice = gaugeSeries->append("", totalScore);
    scoreSlice->setColor(statusColor);
    scoreSlice->setBorderColor(statusColor);
    scoreSlice->setLabelVisible(false);

    // Boş Kısım (Max 60000 varsayalım)
    int maxTotal = 60000;
    int remaining = maxTotal - totalScore;
    if (remaining < 0)
      remaining = 0;

    QPieSlice *emptySlice = gaugeSeries->append("", remaining);
    emptySlice->setColor(QColor("#444"));
    emptySlice->setBorderColor(QColor("#444"));
    emptySlice->setLabelVisible(false);

    gaugeChart->addSeries(gaugeSeries);

    QChartView *gaugeView = new QChartView(gaugeChart);
    gaugeView->setRenderHint(QPainter::Antialiasing);
    gaugeView->setStyleSheet("background: transparent;");

    // Ortaya Skor Yazısı (Overlay Label)
    QLabel *centerLabel = new QLabel(QString::number(totalScore), gaugeView);
    centerLabel->setStyleSheet("color: white; font-size: 24pt; font-weight: "
                               "bold; background: transparent;");
    centerLabel->setAlignment(Qt::AlignCenter);
    centerLabel->resize(200, 100);
    centerLabel->move(50, 25); // Yaklaşık konum

    ui->gauge_chart_container->layout()->addWidget(gaugeView);

    // Label'ı layout'a ekleyelim ki ortalansın
    QVBoxLayout *overlayLayout = new QVBoxLayout(gaugeView);
    overlayLayout->addWidget(centerLabel);
    overlayLayout->setAlignment(centerLabel, Qt::AlignCenter);
  }

  // 3. Mini Kartlar
  if (ui->mini_cpu_score)
    ui->mini_cpu_score->setText(QString::number(cpuScore));
  if (ui->mini_gpu_score)
    ui->mini_gpu_score->setText(QString::number(gpuScore));
  if (ui->mini_ram_score)
    ui->mini_ram_score->setText(QString::number(ramScore));
}

void AnasayfaWidget::setPuanlar(int cpuScore, int gpuScore, int ramScore) {
  // Gelen puanları üye değişkenlere kaydet (Sadece 0'dan büyükse günceller)
  // Böylece tekli test yapıldığında diğer puanlar silinmez.
  if (cpuScore > 0)
    m_cpuScore = cpuScore;
  if (gpuScore > 0)
    m_gpuScore = gpuScore;
  if (ramScore > 0)
    m_ramScore = ramScore;

  // Etiketleri güncelle
  if (m_cpuScore > 0) {
    ui->cpu_score_label->setText(QString::number(m_cpuScore) + " pts");
  }

  if (m_gpuScore > 0) {
    ui->gpu_score_label->setText(QString::number(m_gpuScore) + " pts");
  }

  if (m_ramScore > 0) {
    ui->ram_score_label_2->setText(QString::number(m_ramScore) + " pts");
  }

  // Grafikleri Güncelle (Saklanan puanlarla)
  updateCharts(m_cpuScore, m_gpuScore, m_ramScore);

  // Toplam puanı hesapla ve sinyal gönder
  int totalScore = m_cpuScore + m_gpuScore + m_ramScore;
  emit puanlarGuncellendi(totalScore);
}
