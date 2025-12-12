#include "mainwindow.h"
#include "anasayfawidget.h"
#include "bagiswidget.h"
#include "benchmarkwidget.h"
#include "giriswidget.h"
#include "karsilastirmawidget.h"
#include "kayitwidget.h"
#include "liderlikwidget.h" // <-- EKLENDİ
#include "moderndialogs.h"
#include "networkmanager.h"
#include "ui_mainwindow.h"
#include "yardimwidget.h"
#include <QIcon>
#include <QPushButton>

// ===============================================
// CONSTRUCTOR / DESTRUCTOR
// ===============================================

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  // Buton boyutlarını eşitle
  ui->girisyapbuton->setFixedWidth(115);
  ui->girisyapbuton->setCursor(Qt::PointingHandCursor);
  ui->bagisyapbuton->setFixedWidth(115);
  ui->bagisyapbuton->setCursor(Qt::PointingHandCursor);

  // --- 1. NetworkManager'ı Başlat ---
  netManager = new NetworkManager(this);

  userIsLoggedIn = false;
  currentUsername = ""; // Başlangıçta boş

  setupPages();
  setupMenuButtons();

  ui->stackedWidget->setCurrentWidget(m_anasayfa);
  updateButtonStyles(ui->anasayfabuton);

  // Başlangıçta giriş yapılmamış durumu ayarla (Yeşil Buton)
  setLoginState(false);

  // Modern Blue Gradient for "Bağış Yap" (Top Right)
  ui->bagisyapbuton->setStyleSheet(
      "QPushButton { "
      "   background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, "
      "y2:0, stop:0 #11998e, stop:1 #38ef7d); "
      "   border-radius: 8px; "
      "   color: white; "
      "   padding: 6px 15px; "
      "   font-weight: bold; "
      "   font-size: 12px; "
      "   border: none; "
      "   outline: none; "
      "}"
      "QPushButton:hover { background-color: qlineargradient(spread:pad, "
      "x1:0, y1:0, x2:1, y2:0, stop:0 #38ef7d, stop:1 #11998e); }");
}

MainWindow::~MainWindow() { delete ui; }

// ===============================================
// HESAP SİLME SLOTU
// ===============================================
void MainWindow::on_hesapSilmeIstegi_geldi() {
  bool reply = ModernMessageBox::question(
      this, "Hesap Silme",
      "Hesabınızı kalıcı olarak silmek üzeresiniz!\nBu işlem geri alınamaz. "
      "Emin misiniz?");

  if (reply) {

    if (currentUsername.isEmpty()) {
      ModernMessageBox::critical(this, "Hata", "Kullanıcı bilgisi bulunamadı.");
      return;
    }

    // Node.js'e silme isteği gönder
    netManager->deleteAccount(currentUsername, [=](bool success,
                                                   QString message) {
      if (success) {
        ModernMessageBox::information(this, "Başarılı",
                                      "Hesabınız silindi. Çıkış yapılıyor.");

        // Çıkış işlemlerini yap
        setLoginState(false);
        resetAllPages(); // <-- YENİ: Tüm sayfaları sıfırla
        ui->stackedWidget->setCurrentWidget(m_anasayfa);
      } else {
        ModernMessageBox::critical(this, "Hata", "Silinemedi: " + message);
      }
    });
  }
}

// ===============================================
// GİRİŞ / ÇIKIŞ DURUMUNU DEĞİŞTİREN FONKSİYON
// ===============================================
void MainWindow::setLoginState(bool loggedIn) {
  userIsLoggedIn = loggedIn;

  if (loggedIn) {
    // --- DURUM: GİRİŞ YAPILDI (KIRMIZI BUTON) ---
    ui->girisyapbuton->setText("Çıkış Yap");

    ui->girisyapbuton->setStyleSheet(
        "QPushButton { "
        "   background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, "
        "y2:0, stop:0 #ff416c, stop:1 #ff4b2b); "
        "   color: white; "
        "   border-radius: 8px; "
        "   padding: 6px 15px; "
        "   text-align: center; "
        "   border: none; "
        "   outline: none; "
        "   font-weight: bold; "
        "   font-size: 12px; "
        "}"
        "QPushButton:hover { background-color: qlineargradient(spread:pad, "
        "x1:0, y1:0, x2:1, y2:0, stop:0 #ff4b2b, stop:1 #ff416c); }");

    qDebug() << "Durum: Giriş Yapıldı -> Kırmızı.";
  } else {
    // --- DURUM: ÇIKIŞ YAPILDI (YEŞİL GRADIENT) ---
    ui->girisyapbuton->setText("Giriş Yap");

    ui->girisyapbuton->setStyleSheet(
        "QPushButton { "
        "   background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, "
        "y2:0, stop:0 #00c6ff, stop:1 #0072ff); "
        "   color: white; "
        "   border-radius: 8px; "
        "   padding: 6px 15px; "
        "   text-align: center; "
        "   border: none; "
        "   outline: none; "
        "   font-weight: bold; "
        "   font-size: 12px; "
        "}"
        "QPushButton:hover { background-color: qlineargradient(spread:pad, "
        "x1:0, y1:0, x2:1, y2:0, stop:0 #0072ff, stop:1 #00c6ff); }");

    qDebug() << "Durum: Çıkış Yapıldı -> Yeşil.";
  }
}

void MainWindow::on_girisyapbuton_clicked() {
  if (userIsLoggedIn) {
    // --- ÇIKIŞ İŞLEMİ ---
    bool reply = ModernMessageBox::question(
        this, "Çıkış", "Çıkış yapmak istediğinize emin misiniz?");

    if (reply) {
      setLoginState(false);
      resetAllPages(); // <-- YENİ: Tüm sayfaları sıfırla

      ModernMessageBox::information(this, "Bilgi", "Başarıyla çıkış yapıldı.");

      ModernMessageBox::information(this, "Bilgi", "Başarıyla çıkış yapıldı.");
      ui->stackedWidget->setCurrentWidget(m_anasayfa);
    }
  } else {
    // --- GİRİŞ EKRANINA GİT ---
    ui->stackedWidget->setCurrentWidget(m_giris);
    updateButtonStyles(nullptr);

    // Butonun odaklanmasını kaldır
    ui->girisyapbuton->clearFocus();
  }
}

// ===============================================
// SAYFA KURULUMU
// ===============================================
void MainWindow::setupPages() {
  m_anasayfa = new AnasayfaWidget(this);
  m_benchmark = new BenchmarkWidget(this);
  m_karsilastirma = new KarsilastirmaWidget(this);
  m_yardim = new YardimWidget(this);
  m_giris = new GirisWidget(this);
  m_bagis = new BagisWidget(this);
  m_kayit = new KayitWidget(this);
  m_liderlik = new LiderlikWidget(this);

  ui->stackedWidget->addWidget(m_anasayfa);
  ui->stackedWidget->addWidget(m_benchmark);
  ui->stackedWidget->addWidget(m_karsilastirma);
  ui->stackedWidget->addWidget(m_yardim);
  ui->stackedWidget->addWidget(m_giris);
  ui->stackedWidget->addWidget(m_bagis);
  ui->stackedWidget->addWidget(m_kayit);
  ui->stackedWidget->addWidget(m_liderlik);

  // Connect refresh signal
  connect(m_liderlik, &LiderlikWidget::refreshRequested, this, [=]() {
    netManager->getLeaderboard(
        [=](bool success, QList<QVariantMap> data, QString msg) {
          if (success)
            m_liderlik->updateLeaderboard(data);
          else
            ModernMessageBox::critical(this, "Hata", msg);
        });
  });
}

// ===============================================
// MENÜ VE SİNYAL KURULUMU
// ===============================================
void MainWindow::setupMenuButtons() {
  // Create button dynamically
  ui_liderlikbuton = new QPushButton(" Liderlik Tablosu", this);
  ui_liderlikbuton->setIcon(QIcon(":/Assets/leaderboard.png"));
  ui_liderlikbuton->setIconSize(QSize(32, 32));
  ui_liderlikbuton->setCursor(Qt::PointingHandCursor);

  // Add to layout (Insert before Yardim)
  ui->solsayfalayout->insertWidget(ui->solsayfalayout->count() - 1,
                                   ui_liderlikbuton);

  menuButtons << ui->anasayfabuton << ui->benchmarkbuton
              << ui->karsilastirmabuton << ui_liderlikbuton << ui->yardimbuton;

  for (QPushButton *btn : menuButtons) {
    btn->setCursor(Qt::PointingHandCursor);
  }

  // --- BUTON BAĞLANTILARI ---
  connect(ui->anasayfabuton, &QPushButton::clicked, this,
          &MainWindow::on_anasayfabuton_clicked);
  connect(ui->benchmarkbuton, &QPushButton::clicked, this,
          &MainWindow::on_benchmarkbuton_clicked);
  connect(ui->karsilastirmabuton, &QPushButton::clicked, this,
          &MainWindow::on_karsilastirmabuton_clicked);
  connect(ui_liderlikbuton, &QPushButton::clicked, this,
          &MainWindow::on_liderlikbuton_clicked);
  connect(ui->yardimbuton, &QPushButton::clicked, this,
          &MainWindow::on_yardimbuton_clicked);
  connect(ui->girisyapbuton, &QPushButton::clicked, this,
          &MainWindow::on_girisyapbuton_clicked);
  connect(ui->bagisyapbuton, &QPushButton::clicked, this,
          &MainWindow::on_bagisyapbuton_clicked);

  // --- SİNYAL BAĞLANTILARI ---
  // 1. Giriş Başarılı
  connect(m_giris, &GirisWidget::girisBasarili, this,
          [=](QString username, QVariantMap userData) {
            currentUsername = username; // KULLANICI ADINI KAYDET

            m_anasayfa->setKullaniciBilgileri(username);
            m_karsilastirma->setKullaniciBilgileri(username);
            m_bagis->setKullaniciAdi(username);

            // Email bilgisini userData'dan al ve set et
            QString email = userData["email"].toString();
            m_bagis->setEmail(email);

            ui->stackedWidget->setCurrentWidget(m_anasayfa);
            updateButtonStyles(ui->anasayfabuton);
            setLoginState(true);

            if (!userData.isEmpty()) {
              QString cpu = userData["cpu"].toString();
              QString gpu = userData["gpu"].toString();
              QString ram = userData["ram"].toString();
              int score = userData["score"].toInt();

              qDebug() << "Login UserData Content:";
              qDebug() << "  CPU:" << cpu;
              qDebug() << "  GPU:" << gpu;
              qDebug() << "  RAM:" << ram;
              qDebug() << "  Score:" << score;

              if (!cpu.isEmpty() && !gpu.isEmpty() && !ram.isEmpty()) {
                qDebug() << "Veritabanından sistem bilgileri çekildi:" << cpu
                         << gpu << ram << "Skor:" << score;

                // Anasayfa'yı güncelle
                m_anasayfa->setSistemBilgileri(cpu, gpu, ram);

                // Puanları güncelle
                int cpuScore = userData["cpu_score"].toInt();
                int gpuScore = userData["gpu_score"].toInt();
                int ramScore = userData["ram_score"].toInt();

                qDebug() << "DB Scores - CPU:" << cpuScore
                         << " GPU:" << gpuScore << " RAM:" << ramScore
                         << " Total (DB):" << score;

                if (cpuScore > 0 || gpuScore > 0 || ramScore > 0) {
                  m_anasayfa->setPuanlar(cpuScore, gpuScore, ramScore, false);
                } else {
                  // Eski veriler için (Sadece toplam puan varsa)
                  m_anasayfa->setToplamPuan(score, false);
                }

                // Karşılaştırma Merkezi'ni güncelle
                m_karsilastirma->setSizinSisteminiz(cpu, gpu, ram);
                m_karsilastirma->setSizinPuaniniz(score);
              } else {
                qDebug() << "Sistem bilgileri eksik, arayüz güncellenmedi.";
              }
            } else {
              qDebug() << "UserData is empty!";
            }

            // --- YENİ: Test Geçmişini Çek ---
            netManager->getScoreHistory(
                username,
                [=](bool success, QList<QVariantMap> history, QString message) {
                  if (success) {
                    m_benchmark->updateHistoryList(history); // <-- GÜNCELLENDİ
                  } else {
                    qDebug() << "Geçmiş çekilemedi:" << message;
                  }
                });

            // Benchmark widget'a kullanıcı adını bildir (Silme işlemi için)
            m_benchmark->setUsername(username);
          });

  // 2. Anasayfa'daki "Hesabı Sil" butonu bağlantısı
  connect(m_anasayfa, &AnasayfaWidget::hesapSilmeTiklandi, this,
          &MainWindow::on_hesapSilmeIstegi_geldi);

  // 3. Sistem Bilgileri ve Puan Senkronizasyonu
  connect(m_anasayfa, &AnasayfaWidget::sistemBilgileriGuncellendi,
          m_karsilastirma, &KarsilastirmaWidget::setSizinSisteminiz);
  // Sistem bilgileri güncellenince Benchmark butonunu aktif et
  connect(m_anasayfa, &AnasayfaWidget::sistemBilgileriGuncellendi, m_benchmark,
          &BenchmarkWidget::enableStartButton);

  connect(m_anasayfa, &AnasayfaWidget::puanlarGuncellendi, m_karsilastirma,
          &KarsilastirmaWidget::setSizinPuaniniz);

  // --- YENİ: Puan Güncellenince Veritabanına Kaydet ---
  connect(m_anasayfa, &AnasayfaWidget::puanlarGuncellendi, this,
          [=](int totalScore) {
            if (userIsLoggedIn && !currentUsername.isEmpty()) {
              QString cpu = m_karsilastirma->getCpu();
              QString gpu = m_karsilastirma->getGpu();
              QString ram = m_karsilastirma->getRam();

              int cpuScore = m_anasayfa->getCpuScore();
              int gpuScore = m_anasayfa->getGpuScore();
              int ramScore = m_anasayfa->getRamScore();

              netManager->saveScore(
                  currentUsername, cpu, gpu, ram, totalScore, cpuScore,
                  gpuScore, ramScore, [=](bool success, QString message) {
                    if (success) {
                      qDebug() << "Skor kaydedildi:" << message;
                      // Skor kaydedilince listeyi güncelle
                      netManager->getRivals([=](bool success,
                                                QList<QVariantMap> rivals,
                                                QString message) {
                        if (success) {
                          m_karsilastirma->updateRivalsList(rivals);
                        } else {
                          qDebug() << "Rakipler alınamadı:" << message;
                        }
                      });

                      // --- YENİ: Geçmişi de güncelle ---
                      // --- YENİ: Geçmişi de güncelle ---
                      netManager->getScoreHistory(
                          currentUsername,
                          [=](bool success, QList<QVariantMap> history,
                              QString message) {
                            if (success) {
                              m_benchmark->updateHistoryList(history);
                            }
                          });

                      // --- YENİ: Global Sıralamayı Güncelle ---
                      m_anasayfa->updateGlobalRanking(currentUsername);
                    } else {
                      qDebug() << "Skor kaydedilemedi:" << message;
                    }
                  });
            }
          });

  // 4. Fiyat Sorgulama

  connect(m_benchmark, &BenchmarkWidget::testBitti, this,
          [=](int cpu, int gpu, int ram) {
            m_anasayfa->setPuanlar(cpu, gpu, ram, true);
          });

  // Diğer bağlantılar...
  connect(m_giris, &GirisWidget::kayitOlTiklandi, this, [=]() {
    ui->stackedWidget->setCurrentWidget(m_kayit);
    updateButtonStyles(nullptr);
  });
  connect(m_kayit, &KayitWidget::giriseDonTiklandi, this,
          [=]() { ui->stackedWidget->setCurrentWidget(m_giris); });
  connect(m_kayit, &KayitWidget::kayitBasarili, this,
          [=]() { ui->stackedWidget->setCurrentWidget(m_giris); });
  connect(m_giris, &GirisWidget::sifreUnuttumTiklandi, this, [=]() {
    ui->stackedWidget->setCurrentWidget(m_anasayfa);
    updateButtonStyles(ui->anasayfabuton);
  });
}

void MainWindow::updateButtonStyles(QPushButton *clickedButton) {
  const QString defaultStyle = "QPushButton { "
                               "   background-color: #2b2e38; "
                               "   border: none; "
                               "   border-radius: 8px; "
                               "   padding: 10px 20px; "
                               "   color: #bec3cd; "
                               "   font-size: 14px; "
                               "   font-weight: 500; "
                               "   text-align: left; "
                               "   padding-left: 20px; "
                               "   outline: none; "
                               "}"
                               "QPushButton:hover { "
                               "   background-color: #3c404d; "
                               "   color: white; "
                               "}";

  const QString selectedStyle =
      "QPushButton { "
      "   background-color: qlineargradient(spread:pad, x1:0, "
      "y1:0, x2:1, "
      "y2:0, stop:0 #4facfe, stop:1 #00f2fe); "
      "   border: none; "
      "   border-radius: 8px; "
      "   padding: 10px 20px; "
      "   color: white; "
      "   font-size: 14px; "
      "   font-weight: bold; "
      "   text-align: left; "
      "   padding-left: 20px; "
      "   outline: none; "
      "}";

  for (QPushButton *button : menuButtons) {
    QString style = defaultStyle;
    if (button == ui_liderlikbuton) {
      style.replace("padding-left: 20px;", "padding-left: 15px;");
    }
    button->setStyleSheet(style);
  }
  if (clickedButton) {
    QString style = selectedStyle;
    if (clickedButton == ui_liderlikbuton) {
      style.replace("padding-left: 20px;", "padding-left: 15px;");
    }
    clickedButton->setStyleSheet(style);
  }
}

void MainWindow::on_anasayfabuton_clicked() {
  ui->stackedWidget->setCurrentWidget(m_anasayfa);
  updateButtonStyles(ui->anasayfabuton);
}

void MainWindow::on_karsilastirmabuton_clicked() {
  if (!userIsLoggedIn) {
    ModernMessageBox::information(
        this, "Giriş Gerekli",
        "Karşılaştırma özelliğini kullanmak için lütfen giriş yapınız.");
    ui->stackedWidget->setCurrentWidget(m_giris);
    updateButtonStyles(nullptr);
    return;
  }

  ui->stackedWidget->setCurrentWidget(m_karsilastirma);
  updateButtonStyles(ui->karsilastirmabuton);

  // Sayfa açılınca listeyi güncelle
  qDebug() << "Karsilastirma butonu tiklandi. Kullanici giris yapmis. "
              "Rakipler isteniyor...";
  netManager->getRivals(
      [=](bool success, QList<QVariantMap> rivals, QString message) {
        if (success) {
          qDebug() << "Rakipler basariyla alindi. Sayi:" << rivals.size();
          m_karsilastirma->updateRivalsList(rivals);
        } else {
          qDebug() << "Rakipler alınamadı (Buton Tıklama):" << message;
        }
      });

  // Geçmişi de güncelle
  if (!currentUsername.isEmpty()) {
    netManager->getScoreHistory(
        currentUsername,
        [=](bool success, QList<QVariantMap> history, QString message) {
          if (success) {
          }
        });
  }
}

void MainWindow::on_benchmarkbuton_clicked() {
  ui->stackedWidget->setCurrentWidget(m_benchmark);
  updateButtonStyles(ui->benchmarkbuton);

  // Sayfa açılınca geçmişi güncelle
  if (userIsLoggedIn && !currentUsername.isEmpty()) {
    netManager->getScoreHistory(
        currentUsername,
        [=](bool success, QList<QVariantMap> history, QString message) {
          if (success) {
            m_benchmark->updateHistoryList(history);
          }
        });
  }
}

void MainWindow::on_liderlikbuton_clicked() {
  ui->stackedWidget->setCurrentWidget(m_liderlik);
  updateButtonStyles(ui_liderlikbuton);

  // Sayfa açılınca listeyi güncelle
  netManager->getLeaderboard(
      [=](bool success, QList<QVariantMap> data, QString message) {
        if (success) {
          m_liderlik->updateLeaderboard(data);
        } else {
          qDebug() << "Liderlik tablosu alınamadı:" << message;
        }
      });
}

void MainWindow::on_yardimbuton_clicked() {
  ui->stackedWidget->setCurrentWidget(m_yardim);
  updateButtonStyles(ui->yardimbuton);
}
void MainWindow::on_bagisyapbuton_clicked() {
  if (!userIsLoggedIn) {
    ModernMessageBox::critical(
        this, "Erişim Engellendi",
        "Bağış sayfasına erişmek için lütfen giriş yapınız.");
    ui->stackedWidget->setCurrentWidget(m_giris);
    updateButtonStyles(nullptr);
    return;
  }
  ui->stackedWidget->setCurrentWidget(m_bagis);
  updateButtonStyles(nullptr);
}

void MainWindow::resetAllPages() {
  // 1. Anasayfa
  m_anasayfa->bilgileriSifirla();

  // 2. Karşılaştırma
  m_karsilastirma->reset();

  // 3. Bağış
  m_bagis->reset();

  // 4. Benchmark
  m_benchmark->reset();

  // 5. Genel Değişkenler
  currentUsername = "";

  // 6. Liderlik
  m_liderlik->updateLeaderboard({});
}
