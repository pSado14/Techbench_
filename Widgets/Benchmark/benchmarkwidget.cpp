#include "benchmarkwidget.h"
#include "benchmarkworker.h"
#include "moderndialogs.h"
#include "networkmanager.h"
#include "ui_benchmarkwidget.h"

#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>

BenchmarkWidget::BenchmarkWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::BenchmarkWidget), m_thread(nullptr),
      m_worker(nullptr), m_chart(nullptr), m_chartView(nullptr),
      m_cpuSeries(nullptr), m_gpuSeries(nullptr), m_ramSeries(nullptr),
      m_detectedGpuName("GPU") {
  ui->setupUi(this);

  // Modern Blue Gradient for "Testi Başlat"
  ui->testi_baslat_buton->setStyleSheet(
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
      "x1:0, y1:0, x2:1, y2:0, stop:0 #00f2fe, stop:1 #4facfe); }"
      "QPushButton:disabled { background-color: #555; color: #aaa; }");
  ui->testi_baslat_buton->setCursor(Qt::PointingHandCursor);

  // Modern Red Gradient for "Testi İptal Et" (if it exists, assuming it does
  // based on grep) Note: If testi_iptal_et_buton is not in the UI file yet,
  // this might fail compilation. But grep found it, so it should be fine.
  // Modern Red Gradient for "Testi İptal Et"
  ui->testi_iptal_et_buton->setStyleSheet(
      "QPushButton { "
      "   background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, "
      "y2:0, stop:0 #ff416c, stop:1 #ff4b2b); "
      "   border-radius: 8px; "
      "   color: white; "
      "   padding: 8px 16px; "
      "   font-weight: bold; "
      "   font-size: 12px; "
      "   border: none; "
      "   outline: none;"
      "}"
      "QPushButton:hover { background-color: qlineargradient(spread:pad, "
      "x1:0, y1:0, x2:1, y2:0, stop:0 #ff4b2b, stop:1 #ff416c); }"
      "QPushButton:disabled { background-color: #555; color: #aaa; }");
  ui->testi_iptal_et_buton->setCursor(Qt::PointingHandCursor);

  // Modern Green Gradient for "Oyun Modu"
  ui->oyun_modu_buton->setStyleSheet(
      "QPushButton { "
      "   background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, "
      "y2:0, stop:0 #11998e, stop:1 #38ef7d); "
      "   border-radius: 8px; "
      "   color: white; "
      "   padding: 8px 16px; "
      "   font-weight: bold; "
      "   font-size: 12px; "
      "   border: none; "
      "   outline: none;"
      "}"
      "QPushButton:hover { background-color: qlineargradient(spread:pad, "
      "x1:0, y1:0, x2:1, y2:0, stop:0 #38ef7d, stop:1 #11998e); }"
      "QPushButton:disabled { background-color: #555; color: #aaa; }");
  ui->oyun_modu_buton->setCursor(Qt::PointingHandCursor);

  // Başlangıçta butonları ayarla
  ui->testi_baslat_buton->setEnabled(false);
  ui->testi_baslat_buton->setText("Sistem Bilgilerini Tara");
  ui->testi_iptal_et_buton->setEnabled(
      false);                             // Test başlayana kadar devre dışı
  ui->oyun_modu_buton->setEnabled(false); // Sistem taranana kadar devre dışı

  setupCharts();
  setupHistoryList();
}

BenchmarkWidget::~BenchmarkWidget() {
  if (m_thread) {
    if (m_worker)
      m_worker->requestInterruption();
    m_thread->quit();
    m_thread->wait();
  }
  delete ui;
}

void BenchmarkWidget::setupCharts() {
  // Önceki layout'u temizle (Eğer sonuç ekranından geliyorsak)
  if (ui->canli_grafik_tutucu->layout()) {
    clearLayout(ui->canli_grafik_tutucu->layout());
    delete ui->canli_grafik_tutucu->layout();
  }

  // Grafik Görünümü Oluştur
  m_chart = new QChart();
  m_chart->setTitle("Benchmark Performansı");
  m_chart->setAnimationOptions(QChart::SeriesAnimations);
  m_chart->setTheme(QChart::ChartThemeDark); // Koyu tema

  // Eksenleri oluştur
  m_chart->createDefaultAxes();

  m_chartView = new QChartView(m_chart);
  m_chartView->setRenderHint(QPainter::Antialiasing);

  QVBoxLayout *layout = new QVBoxLayout(ui->canli_grafik_tutucu);
  layout->addWidget(m_chartView);
}

void BenchmarkWidget::enableStartButton() {
  ui->testi_baslat_buton->setEnabled(true);
  ui->testi_baslat_buton->setText("Testi Başlat");
  ui->oyun_modu_buton->setEnabled(true); // Sistem tarandı, oyun modu aktif
}

// --- TESTİ BAŞLAT BUTONU ---
void BenchmarkWidget::on_testi_baslat_buton_clicked() {
  qDebug() << "BenchmarkWidget: Test Başlatılıyor...";

  // 0. KONTROL: En az bir kutucuk seçili mi?
  if (!ui->cpu_testi_cb->isChecked() && !ui->gpu_testi_cb_2->isChecked() &&
      !ui->bellek_testi_cb->isChecked()) {
    ModernMessageBox::critical(
        this, "Uyarı", "Lütfen test etmek için en az bir donanım seçiniz!");
    return;
  }

  // 1. Arayüzü güncelle
  ui->testi_baslat_buton->setText("Test Yapılıyor... Lütfen Bekleyin");
  ui->testi_baslat_buton->setEnabled(false);
  ui->testi_iptal_et_buton->setEnabled(true);

  if (ui->henuz_test_baslatilmadi_label) {
    ui->henuz_test_baslatilmadi_label->setText(
        "Donanım performansı ölçülüyor...");
  }

  // Grafiği Sıfırla (Line Chart'a dön)
  setupCharts();

  // Serileri Oluştur
  m_chart->removeAllSeries();

  if (ui->cpu_testi_cb->isChecked()) {
    m_cpuSeries = new QLineSeries();
    m_cpuSeries->setName("CPU");
    m_cpuSeries->setColor(Qt::blue);
    m_chart->addSeries(m_cpuSeries);
  } else {
    m_cpuSeries = nullptr;
  }

  if (ui->gpu_testi_cb_2->isChecked()) {
    m_gpuSeries = new QLineSeries();
    m_gpuSeries->setName("GPU");
    m_gpuSeries->setColor(Qt::red);
    m_chart->addSeries(m_gpuSeries);
  } else {
    m_gpuSeries = nullptr;
  }

  if (ui->bellek_testi_cb->isChecked()) {
    m_ramSeries = new QLineSeries();
    m_ramSeries->setName("RAM");
    m_chart->addSeries(m_ramSeries);
  } else {
    m_ramSeries = nullptr;
  }

  m_chart->createDefaultAxes();
  if (!m_chart->axes(Qt::Horizontal).isEmpty())
    m_chart->axes(Qt::Horizontal).first()->setRange(0, 100);
  if (!m_chart->axes(Qt::Vertical).isEmpty())
    m_chart->axes(Qt::Vertical).first()->setRange(0, 100);
  m_chart->setTitle("Canlı Test İlerlemesi");

  // RAM rengini tema uygulandıktan ve eksenler oluşturulduktan SONRA ayarla
  if (m_ramSeries) {
    QPen pen = m_ramSeries->pen();
    pen.setColor(QColor("#f1c40f")); // Sarı
    pen.setWidth(2);
    m_ramSeries->setPen(pen);
  }

  // 2. Thread ve Worker Hazırlığı
  m_thread = new QThread;
  m_worker = new BenchmarkWorker(ui->cpu_testi_cb->isChecked(),
                                 ui->gpu_testi_cb_2->isChecked(),
                                 ui->bellek_testi_cb->isChecked());

  m_worker->moveToThread(m_thread);

  // Sinyal-Slot Bağlantıları
  connect(m_thread, &QThread::started, m_worker, &BenchmarkWorker::process);
  connect(m_worker, &BenchmarkWorker::finished, this,
          &BenchmarkWidget::onWorkerFinished);
  connect(m_worker, &BenchmarkWorker::canceled, this,
          &BenchmarkWidget::onWorkerCanceled);
  connect(m_worker, &BenchmarkWorker::progress, this,
          &BenchmarkWidget::onWorkerProgress); // İlerleme
  connect(m_worker, &BenchmarkWorker::gpuDetected, this,
          &BenchmarkWidget::onGpuDetected); // GPU İsmi
  connect(m_worker, &BenchmarkWorker::logMessage, this,
          &BenchmarkWidget::onLogMessage); // Log mesajları
  connect(m_worker, &BenchmarkWorker::finished, m_thread, &QThread::quit);
  connect(m_worker, &BenchmarkWorker::finished, m_worker,
          &BenchmarkWorker::deleteLater);
  connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);

  connect(m_worker, &QObject::destroyed, this,
          [this]() { m_worker = nullptr; });
  connect(m_thread, &QObject::destroyed, this,
          [this]() { m_thread = nullptr; });

  m_thread->start();
}

void BenchmarkWidget::onWorkerProgress(int percent, int step) {
  // step: 0=CPU, 1=GPU, 2=RAM
  if (step == 0 && m_cpuSeries) {
    m_cpuSeries->append(percent, percent);
  } else if (step == 1 && m_gpuSeries) {
    m_gpuSeries->append(percent, percent);
  } else if (step == 2 && m_ramSeries) {
    m_ramSeries->append(percent, percent);
  }

  if (ui->henuz_test_baslatilmadi_label) {
    QString part = (step == 0) ? "CPU" : (step == 1 ? "GPU" : "RAM");
    ui->henuz_test_baslatilmadi_label->setText(
        QString("%1 Testi: %2%").arg(part).arg(percent));
  }
}

// --- TEST BİTTİĞİNDE ---
void BenchmarkWidget::onWorkerFinished(int cpuScore, int gpuScore,
                                       int ramScore) {
  showResultCharts(cpuScore, gpuScore, ramScore);

  QString sonucMesaji = "Benchmark Testi Tamamlandı!\n\n";
  if (cpuScore > 0)
    sonucMesaji += QString("CPU Puanı: %1\n").arg(cpuScore);
  if (gpuScore > 0)
    sonucMesaji += QString("GPU Puanı: %1\n").arg(gpuScore);
  if (ramScore > 0)
    sonucMesaji += QString("RAM Puanı: %1\n").arg(ramScore);
  sonucMesaji += "\nSonuçlar Ana Sayfa'ya aktarıldı.";

  ui->testi_baslat_buton->setText("Testi Başlat");
  ui->testi_baslat_buton->setEnabled(true);
  ui->testi_iptal_et_buton->setEnabled(
      false); // Test bitti, iptal butonu devre dışı

  if (ui->henuz_test_baslatilmadi_label) {
    ui->henuz_test_baslatilmadi_label->setText("Test tamamlandı.");
  }

  ModernMessageBox::information(this, "Test Tamamlandı", sonucMesaji);

  emit testBitti(cpuScore, gpuScore, ramScore);
}

// --- TEST İPTAL EDİLDİĞİNDE ---
void BenchmarkWidget::onWorkerCanceled() {
  ui->testi_baslat_buton->setText("Testi Başlat");
  ui->testi_baslat_buton->setEnabled(true);
  ui->testi_iptal_et_buton->setEnabled(
      false); // Test iptal edildi, iptal butonu devre dışı

  if (ui->henuz_test_baslatilmadi_label) {
    ui->henuz_test_baslatilmadi_label->setText(
        "Test kullanıcı tarafından iptal edildi.");
  }

  if (m_thread) {
    m_thread->quit();
    m_thread->wait();
  }
}

// --- TESTİ İPTAL ET BUTONU ---
void BenchmarkWidget::on_testi_iptal_et_buton_clicked() {
  qDebug() << "BenchmarkWidget: Test İptal İsteği...";

  if (m_worker) {
    m_worker->requestInterruption();
    ui->testi_iptal_et_buton->setEnabled(false);
    ui->henuz_test_baslatilmadi_label->setText("İptal ediliyor...");
  }
}

void BenchmarkWidget::showResultCharts(int cpu, int gpu, int ram) {
  // Mevcut grafiği/layout'u temizle
  if (ui->canli_grafik_tutucu->layout()) {
    clearLayout(ui->canli_grafik_tutucu->layout());
    delete ui->canli_grafik_tutucu->layout();
  }

  // Tek bir Chart oluştur
  QChart *chart = new QChart();
  chart->setTitle("Benchmark Sonuçları");
  chart->setTheme(QChart::ChartThemeDark);
  chart->setAnimationOptions(QChart::SeriesAnimations);
  chart->legend()->setVisible(true);
  chart->legend()->setAlignment(Qt::AlignBottom);
  chart->legend()->setMarkerShape(QLegend::MarkerShapeCircle);

  // Maksimum skor (Halkanın tamamı)
  const int MAX_SCORE = 20000;

  auto addPieSeries = [&](QString name, int score, QColor color, double xPos) {
    // 1. Progress Series (Skor + Arka Plan) - Etiketsiz
    QPieSeries *progressSeries = new QPieSeries();
    progressSeries->setName(name);
    progressSeries->setHoleSize(0.50);
    progressSeries->setPieSize(0.35);
    progressSeries->setHorizontalPosition(xPos);
    progressSeries->setVerticalPosition(0.5);

    if (score > 0) {
      // Skor Dilimi
      QPieSlice *slice = progressSeries->append("", score);
      slice->setColor(color);
      slice->setBorderColor(color.lighter());
      slice->setLabelVisible(false); // Etiket kapalı

      // Kalan Dilimi
      int remaining = MAX_SCORE - score;
      if (remaining < 0)
        remaining = 0;

      QPieSlice *empty = progressSeries->append("", remaining);
      empty->setColor(QColor("#2b2b2b"));
      empty->setBorderColor(QColor("#2b2b2b"));
      empty->setLabelVisible(false);
    } else {
      QPieSlice *slice = progressSeries->append("", 1);
      slice->setColor(Qt::gray);
      slice->setLabelVisible(false);
    }
    chart->addSeries(progressSeries);

    // 2. Label Series (Sadece Etiket İçin) - Sabit Pozisyon (Sağ Üst)
    QPieSeries *labelSeries = new QPieSeries();
    labelSeries->setHoleSize(0.50);
    labelSeries->setPieSize(0.35);
    labelSeries->setHorizontalPosition(xPos);
    labelSeries->setVerticalPosition(0.5);
    labelSeries->setPieStartAngle(30); // Sağ üst köşe (1-2 saat yönü)
    labelSeries->setPieEndAngle(60);

    // Görünmez dilim ama etiketli
    QPieSlice *labelSlice =
        labelSeries->append(QString("%1\n%2").arg(name).arg(score), 1);
    labelSlice->setColor(Qt::transparent);
    labelSlice->setBorderColor(Qt::transparent);
    labelSlice->setLabelVisible(true);
    labelSlice->setLabelColor(Qt::white);
    labelSlice->setLabelPosition(QPieSlice::LabelOutside);

    chart->addSeries(labelSeries);
  };

  // CPU (Sol) - Flat Blue
  if (ui->cpu_testi_cb->isChecked()) {
    addPieSeries("CPU", cpu, QColor("#3498db"), 0.2);
  }

  // GPU (Orta) - Flat Red
  if (ui->gpu_testi_cb_2->isChecked()) {
    addPieSeries("GPU", gpu, QColor("#e74c3c"), 0.5);
  }

  // RAM (Sağ) - Flat Yellow (Resimdeki gibi)
  if (ui->bellek_testi_cb->isChecked()) {
    addPieSeries("RAM", ram, QColor("#f1c40f"), 0.8);
  }

  QChartView *view = new QChartView(chart);
  view->setRenderHint(QPainter::Antialiasing);

  QVBoxLayout *layout = new QVBoxLayout(ui->canli_grafik_tutucu);
  layout->addWidget(view);
}

void BenchmarkWidget::clearLayout(QLayout *layout) {
  if (!layout)
    return;
  QLayoutItem *item;
  while ((item = layout->takeAt(0))) {
    if (item->widget()) {
      delete item->widget();
    }
    if (item->layout()) {
      clearLayout(item->layout());
      delete item->layout();
    }
    delete item;
  }
}

void BenchmarkWidget::onLogMessage(QString message) {
  if (ui->henuz_test_baslatilmadi_label) {
    ui->henuz_test_baslatilmadi_label->setText(message);
  }
}

void BenchmarkWidget::onGpuDetected(QString gpuName) {
  m_detectedGpuName = gpuName;
  // İsim çok uzunsa kısaltabiliriz veya olduğu gibi bırakabiliriz
  qDebug() << "Arayüz GPU ismini aldı:" << m_detectedGpuName;
}

void BenchmarkWidget::setupHistoryList() {
  // UI dosyasında zaten var, sadece butonu ekleyeceğiz
  QVBoxLayout *layout =
      qobject_cast<QVBoxLayout *>(ui->sonuc_gecmisi_frame->layout());
  if (layout) {
    QPushButton *clearBtn = new QPushButton("Geçmişi Temizle");
    clearBtn->setCursor(Qt::PointingHandCursor);
    clearBtn->setStyleSheet("QPushButton { "
                            "   background-color: #e74c3c; "
                            "   border-radius: 5px; "
                            "   color: white; "
                            "   padding: 8px 15px; "
                            "   font-weight: bold; "
                            "   border: none; "
                            "   margin-top: 5px; "
                            "   outline: none;"
                            "}"
                            "QPushButton:hover { background-color: #c0392b; }");
    layout->addWidget(clearBtn);

    connect(clearBtn, &QPushButton::clicked, this,
            &BenchmarkWidget::on_gecmisi_temizle_buton_clicked);
  }
}

// --- TESTİ İPTAL ET BUTONU ---

void BenchmarkWidget::on_gecmisi_temizle_buton_clicked() {
  if (m_username.isEmpty()) {
    ModernMessageBox::critical(this, "Hata", "Önce giriş yapmalısınız.");
    return;
  }

  bool reply = ModernMessageBox::question(
      this, "Geçmişi Sil",
      "Tüm test geçmişinizi silmek istediğinize emin misiniz?");
  if (!reply)
    return;

  // NetworkManager üzerinden silme isteği gönder
  NetworkManager::instance()->deleteScoreHistory(
      m_username, [=](bool success, QString message) {
        if (success) {
          ModernMessageBox::information(this, "Başarılı",
                                        "Test geçmişiniz başarıyla silindi.");
          // Listeyi temizle
          ui->sonuc_gecmisi_listesi->clear();
          // Grafikleri sıfırla (isteğe bağlı, ama mantıklı)
          setupCharts();
        } else {
          ModernMessageBox::critical(this, "Hata",
                                     "Geçmiş silinemedi: " + message);
        }
      });
}

void BenchmarkWidget::updateHistoryList(const QList<QVariantMap> &history) {
  if (!ui->sonuc_gecmisi_listesi)
    return;
  ui->sonuc_gecmisi_listesi->clear();

  for (const QVariantMap &entry : history) {
    QString date = entry["created_at"].toDateTime().toString("yyyy-MM-dd");
    QString dateStr = entry["created_at"].toString();
    if (dateStr.contains("T"))
      dateStr = dateStr.split("T")[0];

    int score = entry["score"].toInt();
    QString itemText = QString("%1: - %2 pts").arg(dateStr).arg(score);
    ui->sonuc_gecmisi_listesi->addItem(itemText);
  }
}

void BenchmarkWidget::reset() {
  m_username = "";
  m_detectedGpuName = "GPU";

  // Listeyi temizle
  if (ui->sonuc_gecmisi_listesi) {
    ui->sonuc_gecmisi_listesi->clear();
  }

  // Grafikleri sıfırla
  setupCharts();

  // Buton durumunu başlangıç haline getir
  ui->testi_baslat_buton->setEnabled(false);
  ui->testi_baslat_buton->setText("Sistem Bilgilerini Tara");
  ui->testi_iptal_et_buton->setEnabled(false); // Reset durumunda da devre dışı

  if (ui->henuz_test_baslatilmadi_label) {
    ui->henuz_test_baslatilmadi_label->setText("Henüz test başlatılmadı.");
  }
}

// --- OYUN MODU (GAME BOOSTER) ---
void BenchmarkWidget::on_oyun_modu_buton_clicked() {
  qDebug() << "Oyun Modu: RAM Optimizasyonu Başlatılıyor...";

  ui->oyun_modu_buton->setEnabled(false);
  ui->oyun_modu_buton->setText("Optimize ediliyor...");
  if (ui->henuz_test_baslatilmadi_label) {
    ui->henuz_test_baslatilmadi_label->setText("RAM temizleniyor...");
  }
  qApp->processEvents();

  // Lambda: RAM miktarını ölç
  auto getFreeRAM = []() -> double {
    QProcess proc;
    proc.start("wmic", {"OS", "get", "FreePhysicalMemory", "/value"});
    proc.waitForFinished(3000);
    QString output = proc.readAllStandardOutput();
    QRegularExpression re("FreePhysicalMemory=(\\d+)");
    QRegularExpressionMatch match = re.match(output);
    if (match.hasMatch()) {
      return match.captured(1).toDouble() / 1024.0; // KB -> MB
    }
    return 0;
  };

  // ÖNCEKİ RAM
  double beforeMB = getFreeRAM();

  // 1. Tüm işlemlerin Working Set'lerini boşalt (EmptyWorkingSet benzeri)
  // PowerShell ile daha etkili bir temizlik
  QProcess cleanProcess;
  cleanProcess.start(
      "powershell",
      {"-Command",
       "Get-Process | ForEach-Object { "
       "try { $_.MinWorkingSet = 1; $_.MaxWorkingSet = 1 } catch {} }"});
  cleanProcess.waitForFinished(10000);

  // 2. .NET Garbage Collector'ü zorla çalıştır
  QProcess gcProcess;
  gcProcess.start(
      "powershell",
      {"-Command",
       "[System.GC]::Collect(); [System.GC]::WaitForPendingFinalizers(); "
       "[System.GC]::Collect()"});
  gcProcess.waitForFinished(5000);

  // 3. Windows Prefetch/Superfetch cache'i temizle (yönetici gerekebilir)
  QProcess cacheProcess;
  cacheProcess.start("cmd", {"/c", "del /q /f %temp%\\*.tmp 2>nul"});
  cacheProcess.waitForFinished(3000);

  // Biraz bekle ki temizlik etkili olsun
  QThread::msleep(1000);

  // SONRAKİ RAM
  double afterMB = getFreeRAM();

  // Farkı hesapla
  double freedMB = afterMB - beforeMB;
  QString freedText;
  if (freedMB > 0) {
    freedText = QString(" %1 MB RAM kazanıldı!").arg(freedMB, 0, 'f', 0);
  } else {
    freedText = "RAM zaten optimize durumda.";
  }

  QString resultMessage = QString(" Oyun Modu Tamamlandı!\n\n"
                                  "Önceki Boş RAM: %1 MB\n"
                                  "Şimdiki Boş RAM: %2 MB\n\n"
                                  "%3\n\n"
                                  "Sistem teste hazır!")
                              .arg(beforeMB, 0, 'f', 0)
                              .arg(afterMB, 0, 'f', 0)
                              .arg(freedText);

  ModernMessageBox::information(this, "Oyun Modu", resultMessage);

  ui->oyun_modu_buton->setEnabled(true);
  ui->oyun_modu_buton->setText("Oyun Modu");

  if (ui->henuz_test_baslatilmadi_label) {
    ui->henuz_test_baslatilmadi_label->setText(
        QString("Optimize edildi! +%1 MB RAM")
            .arg(qMax(0.0, freedMB), 0, 'f', 0));
  }
}
