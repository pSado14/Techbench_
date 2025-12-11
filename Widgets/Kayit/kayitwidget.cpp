#include "kayitwidget.h"
#include "moderndialogs.h"
#include "ui_kayitwidget.h"
#include <QDebug> // Konsola yazı yazmak için

KayitWidget::KayitWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::KayitWidget) {
  ui->setupUi(this);

  // Modern Blue Gradient for "Kayıt Ol"
  ui->kayit_tamamla_buton->setStyleSheet(
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
  ui->kayit_tamamla_buton->setCursor(Qt::PointingHandCursor);

  // --- 1. Network Manager'ı Başlatıyoruz ---
  netManager = new NetworkManager(this);

  // İsterseniz şifre alanlarını sansürlü (yıldızlı) yapabilirsiniz:
  ui->sifre_input->setEchoMode(QLineEdit::Password);
  ui->sifre_tekrar_input->setEchoMode(QLineEdit::Password);

  ui->girise_don_buton->setCursor(Qt::PointingHandCursor);
}

KayitWidget::~KayitWidget() { delete ui; }

// --- KAYIT OL BUTONU ---
void KayitWidget::on_kayit_tamamla_buton_clicked() {
  // 1. Verileri Al
  QString kAdi = ui->kullanici_adi_input->text();
  QString email = ui->email_input->text();
  QString sifre1 = ui->sifre_input->text();
  QString sifre2 = ui->sifre_tekrar_input->text();

  // 2. Boş Alan Kontrolü
  if (kAdi.isEmpty() || email.isEmpty() || sifre1.isEmpty() ||
      sifre2.isEmpty()) {
    ModernMessageBox::critical(this, "Hata", "Lütfen tüm alanları doldurunuz.");
    return;
  }

  // 3. Şifre Eşleşme Kontrolü
  if (sifre1 != sifre2) {
    ModernMessageBox::critical(this, "Hata",
                               "Girdiğiniz şifreler birbiriyle uyuşmuyor!");
    return;
  }

  // 4. --- NODE.JS SUNUCUSUNA İSTEK GÖNDERME ---
  qDebug() << "Sunucuya kayıt isteği gönderiliyor..." << kAdi;

  // Lambda fonksiyonu ile sunucudan gelecek cevabı bekliyoruz
  netManager->registerUser(
      kAdi, sifre1, email, [=](bool success, QString message) {
        if (success) {
          // --- BAŞARILI ---
          ModernMessageBox::information(
              this, "Başarılı",
              "Hesabınız başarıyla oluşturuldu!\nGiriş "
              "ekranına yönlendiriliyorsunuz.");

          // Formu Temizle
          ui->kullanici_adi_input->clear();
          ui->email_input->clear();
          ui->sifre_input->clear();
          ui->sifre_tekrar_input->clear();

          // MainWindow'a "İşlem Tamam" sinyali gönder
          emit kayitBasarili();
        } else {
          // --- HATA ---
          // Sunucudan gelen hatayı (örn: "Kullanıcı adı alınmış") göster
          ModernMessageBox::critical(this, "Kayıt Başarısız", message);
        }
      });
}

// --- GİRİŞE DÖN BUTONU ---
void KayitWidget::on_girise_don_buton_clicked() {
  qDebug() << "Kayıt sayfasından vazgeçildi, girişe dönülüyor.";

  // MainWindow'a "Geri Dön" sinyali gönder
  emit giriseDonTiklandi();
}
