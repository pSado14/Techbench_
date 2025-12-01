#include "giriswidget.h"
#include "sifremiunuttumdialog.h"
#include "ui_giriswidget.h"
#include <QDebug>
#include <QMessageBox>

GirisWidget::GirisWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::GirisWidget) {
  ui->setupUi(this);

  // Modern Green Gradient for "Giriş Yap"
  ui->giris_yap_butonu_sayfa->setStyleSheet(
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

  // Modern Blue Gradient for "Kayıt Ol"
  // Transparent Style for "Kayıt Ol" (Like Forgot Password)
  ui->kayit_ol_butonu_sayfa->setStyleSheet(
      "QPushButton { "
      "   background-color: transparent; "
      "   color: white; "
      "   text-align: right; "
      "   padding: 0; "
      "   border: none; "
      "}"
      "QPushButton:hover { color: #4facfe; }");

  // Network Manager'ı Başlat
  netManager = new NetworkManager(this);

  // Şifre alanını gizli (yıldızlı) yap
  ui->sifre_input_sayfa->setEchoMode(QLineEdit::Password);
}

GirisWidget::~GirisWidget() { delete ui; }

// --- 1. GİRİŞ YAP BUTONU ---
void GirisWidget::on_giris_yap_butonu_sayfa_clicked() {
  // UI'daki verileri al
  QString username = ui->kullanici_adi_input_sayfa->text();
  QString password = ui->sifre_input_sayfa->text();

  // Basit Kontrol
  if (username.isEmpty() || password.isEmpty()) {
    QMessageBox::warning(this, "Hata",
                         "Lütfen kullanıcı adı ve şifre giriniz.");
    return;
  }

  qDebug() << "Sunucuya giriş isteği gönderiliyor..." << username;

  // --- NODE.JS SUNUCUSUNA GİRİŞ İSTEĞİ ---
  netManager->loginUser(
      username, password,
      [=](bool success, QString message, QVariantMap userData) {
        if (success) {
          // --- BAŞARILI ---
          qDebug() << "Giriş Başarılı!";

          // MainWindow'a sinyal gönderiyoruz ve KULLANICI ADINI da ekliyoruz
          emit girisBasarili(username, userData);

          // Inputları temizle
          ui->kullanici_adi_input_sayfa->clear();
          ui->sifre_input_sayfa->clear();
        } else {
          // --- HATA ---
          QMessageBox::warning(this, "Giriş Başarısız", message);
        }
      });
}

// --- 2. KAYIT OL BUTONU ---
void GirisWidget::on_kayit_ol_butonu_sayfa_clicked() {
  qDebug() << "Kayıt Ol butonuna basıldı -> Sinyal gönderiliyor.";
  emit kayitOlTiklandi();
}

// --- 3. ŞİFREMİ UNUTTUM BUTONU ---
void GirisWidget::on_sifre_unuttum_butonu_sayfa_clicked() {
  qDebug() << "Şifremi unuttum tıklandı.";

  // SifremiUnuttumDialog'u aç
  SifremiUnuttumDialog *dialog = new SifremiUnuttumDialog(netManager, this);
  dialog->exec(); // Modal olarak aç
  delete dialog;

  emit sifreUnuttumTiklandi();
}
