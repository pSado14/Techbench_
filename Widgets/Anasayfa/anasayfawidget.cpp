#include "anasayfawidget.h"
#include "networkmanager.h"
#include "ui_anasayfawidget.h"
#include <QDebug>
#include <QLayoutItem>
#include <QMessageBox>
#include <QPainter>
#include <QProcess>
#include <QPushButton>
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

  ui->sistemibilgilerinitarabuton->setCursor(Qt::PointingHandCursor);
  ui->hesapSilButon->setCursor(Qt::PointingHandCursor);

  setupCharts(); // <-- Charts kurulumu
  bilgileriSifirla();

  // Varsayılan olarak laptop bilgisini gizle
  if (ui->laptop_info_frame)
    ui->laptop_info_frame->setVisible(false);

  // --- GPU SEÇİM UI KURULUMU ---
  gpuSelectionLabel = new QLabel("Tam fiyat için model seçin:", this);
  gpuModelComboBox = new QComboBox(this);
  gpuSelectionLabel->setVisible(false);
  gpuModelComboBox->setVisible(false);

  // GPU Label'ın olduğu layout'a ekle
  if (ui->gpu_model_label && ui->gpu_model_label->parentWidget() &&
      ui->gpu_model_label->parentWidget()->layout()) {
    ui->gpu_model_label->parentWidget()->layout()->addWidget(gpuSelectionLabel);
    ui->gpu_model_label->parentWidget()->layout()->addWidget(gpuModelComboBox);
  }

  connect(gpuModelComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &AnasayfaWidget::onGpuModelSelected);

  // Fiyat kontrol sinyalini bağla
  connect(this, &AnasayfaWidget::priceCheckRequested, this,
          &AnasayfaWidget::checkPrice);
}

void AnasayfaWidget::onGpuModelSelected(int index) {
  if (index < 0)
    return;

  QString price = gpuModelComboBox->itemData(index, Qt::UserRole).toString();
  QString source =
      gpuModelComboBox->itemData(index, Qt::UserRole + 1).toString();

  if (!price.isEmpty()) {
    setPrice("GPU", price, source);
  }
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
    if (cleanLine.startsWith("ChassisTypes", Qt::CaseInsensitive))
      continue;
    if (cleanLine.startsWith("L3CacheSize", Qt::CaseInsensitive))
      continue;
    if (cleanLine.startsWith("L2CacheSize", Qt::CaseInsensitive))
      continue;
    if (cleanLine.startsWith("SMBIOSMemoryType", Qt::CaseInsensitive))
      continue;
    if (cleanLine.startsWith("VideoMemoryType", Qt::CaseInsensitive))
      continue;

    return cleanLine; // İlk geçerli veri
  }
  return "0"; // Bulunamazsa 0 döndür
}

QStringList AnasayfaWidget::getAllHardwareInfo(const QString &cmd,
                                               const QStringList &args) {
  QProcess process;
  process.start(cmd, args);
  process.waitForFinished();

  QString result = QString::fromLocal8Bit(process.readAllStandardOutput());
  QStringList lines =
      result.split(QRegularExpression("[\r\n]+"), Qt::SkipEmptyParts);
  QStringList validLines;

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
    if (cleanLine.startsWith("ChassisTypes", Qt::CaseInsensitive))
      continue;
    if (cleanLine.startsWith("L3CacheSize", Qt::CaseInsensitive))
      continue;
    if (cleanLine.startsWith("L2CacheSize", Qt::CaseInsensitive))
      continue;
    if (cleanLine.startsWith("SMBIOSMemoryType", Qt::CaseInsensitive))
      continue;
    if (cleanLine.startsWith("VideoMemoryType", Qt::CaseInsensitive))
      continue;

    validLines.append(cleanLine);
  }
  return validLines;
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

  // Önbellek (L3 Cache)
  QString l3Cache = getHardwareInfo("wmic", {"cpu", "get", "L3CacheSize"});
  QString cacheText = "-";
  if (l3Cache != "0" && !l3Cache.isEmpty()) {
    double l3MB = l3Cache.toDouble() / 1024.0;
    if (l3MB >= 1.0) {
      cacheText = QString::number(l3MB, 'f', 1) + " MB";
    } else {
      cacheText = l3Cache + " KB";
    }
  } else {
    // L3 yoksa L2 dene
    QString l2Cache = getHardwareInfo("wmic", {"cpu", "get", "L2CacheSize"});
    if (l2Cache != "0" && !l2Cache.isEmpty()) {
      double l2MB = l2Cache.toDouble() / 1024.0;
      if (l2MB >= 1.0) {
        cacheText = QString::number(l2MB, 'f', 1) + " MB";
      } else {
        cacheText = l2Cache + " KB";
      }
    }
  }

  // Label'ları güncelle (Multi-line)
  // Label'ları güncelle
  ui->cpu_hiz_label->setText("Hız:");
  ui->cpu_cache_value_label->setText(cacheText);
  if (ui->cpu_clock_label->text() == "-") {
    ui->cpu_clock_label->setText("-");
  }

  // ==========================
  // 2. GPU (EKRAN KARTI) BİLGİLERİ
  // ==========================

  // Tüm GPU'ları çek
  QStringList gpuList = getAllHardwareInfo(
      "wmic", {"path", "win32_videocontroller", "get", "name"});

  QString selectedGpuName = "Bilinmiyor";
  int selectedGpuIndex = 0; // Varsayılan olarak ilk GPU

  if (!gpuList.isEmpty()) {
    selectedGpuName =
        gpuList.first(); // Varsayılan: İlk bulunan (Genelde Intel)

    // Harici GPU Önceliği (NVIDIA, AMD, Radeon, GeForce, RTX, GTX)
    for (int i = 0; i < gpuList.size(); ++i) {
      QString gpu = gpuList[i];
      if (gpu.contains("NVIDIA", Qt::CaseInsensitive) ||
          gpu.contains("AMD", Qt::CaseInsensitive) ||
          gpu.contains("Radeon", Qt::CaseInsensitive) ||
          gpu.contains("GeForce", Qt::CaseInsensitive) ||
          gpu.contains("RTX", Qt::CaseInsensitive) ||
          gpu.contains("GTX", Qt::CaseInsensitive)) {
        selectedGpuName = gpu;
        selectedGpuIndex = i;
        break; // İlk harici GPU'yu bul ve çık
      }
    }
  }

  ui->gpu_model_label->setText(selectedGpuName);

  // VRAM Bilgisi (Seçilen GPU'ya göre)
  QStringList vramList = getAllHardwareInfo(
      "wmic", {"path", "win32_videocontroller", "get", "AdapterRAM"});

  QString vramRaw = "0";
  if (selectedGpuIndex < vramList.size()) {
    vramRaw = vramList[selectedGpuIndex];
  } else if (!vramList.isEmpty()) {
    vramRaw = vramList.first();
  }

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

  // GPU Bellek Tipi (GDDR)
  QString gpuMemType = "-";
  // Önce nvidia-smi dene
  QString nvidiaMemType = getHardwareInfo(
      "nvidia-smi", {"--query-gpu=memory.type", "--format=csv,noheader"});

  if (nvidiaMemType != "0" && !nvidiaMemType.isEmpty() &&
      !nvidiaMemType.contains("not a valid field", Qt::CaseInsensitive) &&
      !nvidiaMemType.contains("failed", Qt::CaseInsensitive)) {
    gpuMemType = nvidiaMemType;
  } else {
    // Bulunamazsa genel bir tahmin veya boş
    // wmic ile VideoMemoryType 2 (Unknown) dönebilir, güvenilmez.
    // Harici GPU ise genelde GDDR'dır.
    // İsimden tahmin etme (User snippet logic)
    QString upperGpuName = selectedGpuName.toUpper();
    if (upperGpuName.contains("RTX 40"))
      gpuMemType = "GDDR6X / GDDR6";
    else if (upperGpuName.contains("RTX 30"))
      gpuMemType = "GDDR6"; // 3050 Ti GDDR6 kullanir
    else if (upperGpuName.contains("RTX 20"))
      gpuMemType = "GDDR6";
    else if (upperGpuName.contains("GTX 16"))
      gpuMemType = "GDDR5 / GDDR6";
    else if (upperGpuName.contains("GTX 10"))
      gpuMemType = "GDDR5 / GDDR5X";
    else if (selectedGpuName.contains("NVIDIA") ||
             selectedGpuName.contains("AMD") ||
             selectedGpuName.contains("Radeon") ||
             selectedGpuName.contains("RTX") ||
             selectedGpuName.contains("GTX")) {
      gpuMemType = "GDDR";
    }
  }

  ui->gpu_hiz_label->setText("Hız:");
  ui->gpu_memtype_value_label->setText(gpuMemType);

  QString clockText = "-";
  if (gpuClock != "0" && !gpuClock.isEmpty()) {
    clockText = gpuClock;
  }
  ui->gpu_clock_label->setText(clockText);

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

  // RAM Tipi (DDR)
  QString ramTypeRaw =
      getHardwareInfo("wmic", {"memorychip", "get", "SMBIOSMemoryType"});
  QString ramType = "DDR";
  bool ok;
  int typeInt = ramTypeRaw.toInt(&ok);
  if (ok) {
    if (typeInt == 20)
      ramType = "DDR";
    else if (typeInt == 21)
      ramType = "DDR2";
    else if (typeInt == 24)
      ramType = "DDR3";
    else if (typeInt == 26)
      ramType = "DDR4";
    else if (typeInt == 30)
      ramType = "LPDDR4";
    else if (typeInt == 34)
      ramType = "DDR5";
    else if (typeInt == 35)
      ramType = "LPDDR5";
    else if (typeInt == 0)
      ramType = "DDR5"; // Genelde yeni sistemlerde 0
    else
      ramType = "Bilinmiyor (" + QString::number(typeInt) + ")";
  }

  ui->ram_hiz_label->setText("Hız:");
  ui->ram_type_value_label->setText(ramType);
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

  emit sistemBilgileriGuncellendi(cpuName, selectedGpuName, ramString);

  // Butonu eski haline getir
  ui->sistemibilgilerinitarabuton->setText(
      "SİSTEM BİLGİLERİNİ GÜNCELLE / TARA");
  ui->sistemibilgilerinitarabuton->setEnabled(true);

  // --- FİYAT TARAMASI TETİKLEME ---
  // Laptop vs Desktop Kontrolü
  QString chassisType =
      getHardwareInfo("wmic", {"systemenclosure", "get", "chassistypes"});
  // Chassis Types: 8, 9, 10, 11, 12, 14, 18, 21, 30, 31, 32 -> Laptop/Portable
  bool isLaptop = false;
  if (chassisType.contains("8") || chassisType.contains("9") ||
      chassisType.contains("10") || chassisType.contains("11") ||
      chassisType.contains("18") || chassisType.contains("21") ||
      chassisType.contains("30") || chassisType.contains("31") ||
      chassisType.contains("32")) {
    isLaptop = true;
  }

  // laptop_info_frame'i görünür yap ve bilgileri güncelle
  if (ui->laptop_info_frame)
    ui->laptop_info_frame->setVisible(true);

  if (isLaptop) {
    // Laptop ise model ismini bul ve arat
    QString laptopModel = getHardwareInfo("wmic", {"csproduct", "get", "name"});

    // Laptop model label'ını güncelle
    if (ui->laptop_model_label) {
      if (laptopModel != "0" && !laptopModel.isEmpty())
        ui->laptop_model_label->setText("Laptop: " + laptopModel);
      else
        ui->laptop_model_label->setText("Laptop");
    }

    if (laptopModel != "0" && !laptopModel.isEmpty()) {
      emit priceCheckRequested(laptopModel, "LAPTOP");
    }

    if (ramModel != "Bilinmiyor") {
      emit priceCheckRequested(ramModel, "RAM");
    }
  } else {
    // Masaüstü ise
    QString desktopModel =
        getHardwareInfo("wmic", {"csproduct", "get", "name"});

    // Masaüstü model label'ını güncelle
    if (ui->laptop_model_label) {
      if (desktopModel != "0" && !desktopModel.isEmpty() &&
          desktopModel != "System Product Name")
        ui->laptop_model_label->setText("Masaüstü: " + desktopModel);
      else
        ui->laptop_model_label->setText("Masaüstü");
    }

    // Parça parça arat
    if (cpuName != "0" && !cpuName.isEmpty())
      emit priceCheckRequested(cpuName, "CPU");

    if (selectedGpuName != "0" && !selectedGpuName.isEmpty()) {
      // --- GPU MODEL SEÇİMİ ---
      QString searchQuery = selectedGpuName;
      // Gereksiz kelimeleri temizle
      searchQuery.replace("NVIDIA", "", Qt::CaseInsensitive);
      searchQuery.replace("GeForce", "", Qt::CaseInsensitive);
      searchQuery.replace("Laptop GPU", "", Qt::CaseInsensitive);
      searchQuery = searchQuery.trimmed();

      NetworkManager *nm = new NetworkManager(this);
      nm->searchProducts(
          searchQuery, "GPU",
          [=](bool success, QList<QVariantMap> results, QString message) {
            gpuModelComboBox->clear();
            gpuSelectionLabel->setVisible(false);
            gpuModelComboBox->setVisible(false);

            if (success && !results.isEmpty()) {
              if (results.size() > 1) {
                // Birden fazla sonuç varsa dropdown göster
                gpuSelectionLabel->setVisible(true);
                gpuModelComboBox->setVisible(true);

                gpuModelComboBox->addItem("Seçiniz...", ""); // Varsayılan boş

                for (const QVariantMap &product : results) {
                  QString name = product["product_name"].toString();
                  QString price = product["price"].toString();
                  QString source = product["source"].toString();

                  gpuModelComboBox->addItem(name + " (" + price + ")", price);
                  gpuModelComboBox->setItemData(gpuModelComboBox->count() - 1,
                                                source, Qt::UserRole + 1);
                }
              } else {
                // Tek sonuç varsa direkt fiyatı bas
                QString price = results[0]["price"].toString();
                QString source = results[0]["source"].toString();
                setPrice("GPU", price, source);
              }
              // Sonuç yoksa eski usül devam et
              emit priceCheckRequested(selectedGpuName, "GPU");
            }
            nm->deleteLater();
          });
    }

    if (ramModel != "Bilinmiyor")
      emit priceCheckRequested(ramModel, "RAM");
  }
}

void AnasayfaWidget::bilgileriSifirla() {
  ui->kullanici_adi_label->setText("Misafir Kullanıcı");

  ui->cpu_model_label->setText("Taranmadı");
  ui->cpu_cores_label->setText("-");
  ui->cpu_clock_label->setText("-");
  ui->cpu_hiz_label->setText("Hız:");
  ui->cpu_cache_value_label->setText("-");

  ui->gpu_model_label->setText("Taranmadı");
  ui->gpu_vram_label->setText("-");
  ui->gpu_clock_label->setText("-");
  ui->gpu_hiz_label->setText("Hız:");
  ui->gpu_memtype_value_label->setText("-");

  ui->ram_gb_label->setText("-");
  ui->ram_gb_label->setText("-");
  ui->ram_clock_label->setText("-");
  ui->ram_hiz_label->setText("Hız:");
  ui->ram_type_value_label->setText("-");
  if (ui->ram_model_label_2)
    ui->ram_model_label_2->setText("Taranmadı");

  // Puanları da sıfırla
  ui->cpu_score_label->setText("-");
  ui->gpu_score_label->setText("-");
  ui->ram_score_label_2->setText("-");

  m_cpuScore = 0;
  m_gpuScore = 0;
  m_ramScore = 0;
  m_gpuScore = 0;
  m_ramScore = 0;
  updateCharts(0, 0, 0);

  if (ui->laptop_price_label)
    ui->laptop_price_label->setText("-");
}

void AnasayfaWidget::setKullaniciBilgileri(const QString &ad) {
  ui->kullanici_adi_label->setText(ad);

  if (ad == "Misafir Kullanıcı") {
    if (ui->global_ranking_label) {
      ui->global_ranking_label->setText("Global Sıralama: -");
    }
    return;
  }

  // Global Sıralamayı Çek
  NetworkManager *nm = new NetworkManager(this);
  qDebug() << "Sıralama isteniyor için kullanıcı:" << ad;
  nm->getUserRanking(ad, [=](bool success, int ranking, QString message) {
    if (success) {
      qDebug() << "Sıralama başarılı:" << ranking;
      if (ui->global_ranking_label) {
        ui->global_ranking_label->setText("Global Sıralama: #" +
                                          QString::number(ranking));
      }
    } else {
      qDebug() << "Sıralama alınamadı:" << message;
      if (ui->global_ranking_label) {
        ui->global_ranking_label->setText("Sıralama: - (" + message + ")");
      }
    }
    nm->deleteLater();
  });
}

void AnasayfaWidget::setSistemBilgileri(const QString &cpu, const QString &gpu,
                                        const QString &ram) {
  ui->cpu_model_label->setText(cpu);
  ui->gpu_model_label->setText(gpu);

  // RAM bilgisini parse edip ekrana basabiliriz veya direkt basabiliriz
  // Veritabanında "Kingston (16 GB)" gibi tutuluyorsa direkt basmak mantıklı
  ui->ram_model_label_2->setText(ram);

  // Diğer detayları (Core count, clock speed vs) veritabanında tutmuyorsak "-"
  // yapabiliriz veya veritabanına bu detayları da ekleyebiliriz. Şimdilik
  // sadece model isimleri var.

  emit sistemBilgileriGuncellendi(cpu, gpu, ram);

  emit sistemBilgileriGuncellendi(cpu, gpu, ram);

  // --- LAPTOP VS DESKTOP KONTROLÜ ---
  QString chassisType =
      getHardwareInfo("wmic", {"systemenclosure", "get", "chassistypes"});
  // Chassis Types: 8, 9, 10, 11, 12, 14, 18, 21, 30, 31, 32 -> Laptop/Portable
  bool isLaptop = false;
  if (chassisType.contains("8") || chassisType.contains("9") ||
      chassisType.contains("10") || chassisType.contains("11") ||
      chassisType.contains("18") || chassisType.contains("21") ||
      chassisType.contains("30") || chassisType.contains("31") ||
      chassisType.contains("32")) {
    isLaptop = true;
  }

  if (isLaptop) {
    if (ui->laptop_info_frame)
      ui->laptop_info_frame->setVisible(true);

    QString laptopModel = getHardwareInfo("wmic", {"csproduct", "get", "name"});

    if (ui->laptop_model_label) {
      if (laptopModel != "0" && !laptopModel.isEmpty())
        ui->laptop_model_label->setText("Laptop: " + laptopModel);
      else
        ui->laptop_model_label->setText("Laptop");
    }

    // Laptop ise sadece model ismini arat
    if (laptopModel != "0" && !laptopModel.isEmpty()) {
      qDebug() << "Laptop Model Search Query:" << laptopModel;
      emit priceCheckRequested(laptopModel, "LAPTOP");

      // UI Hazırlığı
      if (ui->laptop_price_label)
        ui->laptop_price_label->setText("Aranıyor...");
    }

    if (ui->g_price_label)
      ui->g_price_label->setText("-"); // Laptopta ayrı GPU fiyatı olmaz genelde
    if (ui->r_price_label)
      ui->r_price_label->setText("-");
    if (ui->ram_price_label)
      ui->ram_price_label->setText("Fiyat:");

  } else {
    // Masaüstü ise
    if (ui->laptop_info_frame)
      ui->laptop_info_frame->setVisible(true);

    QString desktopModel =
        getHardwareInfo("wmic", {"csproduct", "get", "name"});

    if (ui->laptop_model_label) {
      if (desktopModel != "0" && !desktopModel.isEmpty() &&
          desktopModel != "System Product Name")
        ui->laptop_model_label->setText("Masaüstü: " + desktopModel);
      else
        ui->laptop_model_label->setText("Masaüstü");
    }

    // Masaüstü ise parça parça arat
    if (cpu != "Taranmadı" && cpu != "-")
      emit priceCheckRequested(cpu, "CPU");
    if (gpu != "Taranmadı" && gpu != "-")
      emit priceCheckRequested(gpu, "GPU");
    if (ram != "Taranmadı" && ram != "-")
      emit priceCheckRequested(ram, "RAM");
  }
}

void AnasayfaWidget::setPrice(const QString &type, const QString &price,
                              const QString &source) {
  QString priceText = price;
  // Source text removed as per user request
  // if (!source.isEmpty()) {
  //   priceText += " (" + source + ")";
  // }

  if (type == "CPU") {
    if (ui->cprice_label)
      ui->cprice_label->setText(priceText);
  } else if (type == "GPU") {
    if (ui->g_price_label)
      ui->g_price_label->setText(priceText);
  } else if (type == "RAM") {
    if (ui->r_price_label)
      ui->r_price_label->setText(priceText);
  } else if (type == "LAPTOP") {
    if (ui->laptop_price_label)
      ui->laptop_price_label->setText(priceText);
  }
}

void AnasayfaWidget::setToplamPuan(int score, bool emitSignal) {
  // Sadece toplam puanı güncelle, diğerlerini sıfırla
  m_cpuScore = 0;
  m_gpuScore = 0;
  m_ramScore = 0;

  // UI'da tekli puanları temizle
  ui->cpu_score_label->setText("-");
  ui->gpu_score_label->setText("-");
  ui->ram_score_label_2->setText("-");

  // Grafikleri temizle (0,0,0 ile çağırınca boş grafik çizer)
  updateCharts(0, 0, 0);

  // Ancak Gauge Chart (Genel Skor) için özel işlem yapmamız lazım.
  // updateCharts fonksiyonu içinde gauge chart da güncelleniyor ama
  // cpu+gpu+ram toplamına göre.
  // Bu yüzden updateCharts fonksiyonunu modifiye etmek yerine,
  // buraya özel bir gauge güncelleme kodu ekleyebiliriz veya
  // updateCharts'a opsiyonel parametre ekleyebiliriz.
  // En temiz yol: updateCharts fonksiyonunu "totalScoreOverride" parametresi
  // ile güncellemek. Ama şimdilik kodu kopyalayıp sadece gauge kısmını buraya
  // alalım, çünkü updateCharts çok iş yapıyor.

  // --- GAUGE CHART GÜNCELLEME (Kopyalanmış Mantık) ---
  int totalScore = score;
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

  if (ui->general_score_status_label) {
    ui->general_score_status_label->setText(
        QString("Genel Performans: <span style='color:%1; "
                "font-weight:bold;'>%2</span>")
            .arg(statusColor.name())
            .arg(statusText));
  }

  if (ui->gauge_chart_container) {
    if (!ui->gauge_chart_container->layout()) {
      new QVBoxLayout(ui->gauge_chart_container);
    }

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

    QPieSlice *scoreSlice = gaugeSeries->append("", totalScore);
    scoreSlice->setColor(statusColor);
    scoreSlice->setBorderColor(statusColor);
    scoreSlice->setLabelVisible(false);

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

    QLabel *centerLabel = new QLabel(QString::number(totalScore), gaugeView);
    centerLabel->setStyleSheet("color: white; font-size: 24pt; font-weight: "
                               "bold; background: transparent;");
    centerLabel->setAlignment(Qt::AlignCenter);
    centerLabel->resize(200, 100);
    centerLabel->move(50, 25);

    ui->gauge_chart_container->layout()->addWidget(gaugeView);

    QVBoxLayout *overlayLayout = new QVBoxLayout(gaugeView);
    overlayLayout->addWidget(centerLabel);
    overlayLayout->setAlignment(centerLabel, Qt::AlignCenter);
  }

  // Sinyal Gönder
  if (emitSignal) {
    emit puanlarGuncellendi(totalScore);
  }
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

void AnasayfaWidget::setPuanlar(int cpuScore, int gpuScore, int ramScore,
                                bool emitSignal) {
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

  if (emitSignal) {
    emit puanlarGuncellendi(totalScore);
  }
}

void AnasayfaWidget::checkPrice(const QString &productName,
                                const QString &type) {
  // Kullanıcıya geri bildirim ver
  setPrice(type, "Aranıyor...", "");

  NetworkManager *nm = new NetworkManager(this);
  nm->getPrice(productName, type,
               [=](bool success, QString price, QString source) {
                 if (success) {
                   setPrice(type, price, source);
                 } else {
                   setPrice(type, "Bulunamadı", "");
                 }
                 nm->deleteLater();
               });
}
