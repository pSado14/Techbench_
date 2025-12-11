#include "sifremiunuttumdialog.h"
#include "moderndialogs.h"
#include "ui_sifremiunuttumdialog.h"
#include <QGraphicsDropShadowEffect>
#include <QPushButton>

SifremiUnuttumDialog::SifremiUnuttumDialog(NetworkManager *networkManager,
                                           QWidget *parent)
    : QDialog(parent), ui(new Ui::SifremiUnuttumDialog),
      m_networkManager(networkManager) {
  ui->setupUi(this);

  // Modern Blue Gradient for "Kod Gönder"
  ui->kodGonderButon->setStyleSheet(
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
      "x1:0, y1:0, x2:1, y2:0, stop:0 #00f2fe, stop:1 #4facfe); }"
      "QPushButton:disabled { background-color: #cccccc; color: #666666; }");
  ui->kodGonderButon->setCursor(Qt::PointingHandCursor);

  // Modern Blue Gradient for "Şifreyi Yenile"
  ui->sifreYenileButon->setStyleSheet(
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
      "x1:0, y1:0, x2:1, y2:0, stop:0 #00f2fe, stop:1 #4facfe); }"
      "QPushButton:disabled { background-color: #cccccc; color: #666666; }");
  ui->sifreYenileButon->setCursor(Qt::PointingHandCursor);

  ui->stackedWidget->setCurrentIndex(0); // İlk sayfa: E-posta girişi

  // --- MODERN POP-UP STİLİ ---
  setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
  setAttribute(Qt::WA_TranslucentBackground);

  // Ana widget'a stil verelim (veya yeni bir frame ekleyelim)
  // Mevcut yapıda direkt this->setStyleSheet çalışmayabilir çünkü translucent.
  // Bu yüzden içindeki her şeyi bir QFrame içine almak en iyisi ama UI
  // dosyasını değiştiremiyoruz. Alternatif: UI'daki en dış widget'a stil
  // verelim. UI dosyasında en dışta bir layout var. Biz kodla bir frame ekleyip
  // layout'u içine alabiliriz ama karmaşık. Basit çözüm: Arka plan rengini ve
  // kenarlığı stylesheet ile verelim.

  // Close Button (Sağ üst)
  QPushButton *closeBtn = new QPushButton("X", this);
  closeBtn->setGeometry(this->width() - 40, 10, 30, 30);
  closeBtn->setStyleSheet(
      "QPushButton { background-color: transparent; color: #bec3cd; "
      "font-weight: bold; font-size: 16px; border: none; }"
      "QPushButton:hover { color: #ff4d4d; }");
  connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);

  // Pencereyi sürükleyebilmek için event filter veya mouse event override
  // gerekir. Şimdilik basit bırakalım.

  // Stil
  this->setStyleSheet(
      "QDialog { background-color: #2b2e38; border: 1px solid #3c404d; "
      "border-radius: 10px; } "
      "QLabel { color: white; } "
      "QLineEdit { background-color: #1e1e1e; color: white; border: 1px solid "
      "#3c404d; border-radius: 5px; padding: 8px; }");
}

SifremiUnuttumDialog::~SifremiUnuttumDialog() { delete ui; }

void SifremiUnuttumDialog::on_kodGonderButon_clicked() {
  QString email = ui->emailLineEdit->text().trimmed();

  if (email.isEmpty()) {
    ModernMessageBox::critical(this, "Hata",
                               "Lütfen e-posta adresinizi girin.");
    return;
  }

  ui->kodGonderButon->setEnabled(false);
  ui->kodGonderButon->setText("Gönderiliyor...");

  m_networkManager->sendForgotPasswordCode(email, [=](bool success,
                                                      QString message) {
    ui->kodGonderButon->setEnabled(true);
    ui->kodGonderButon->setText("Kod Gönder");

    if (success) {
      m_email = email;
      ModernMessageBox::information(
          this, "Başarılı", "Doğrulama kodu e-posta adresinize gönderildi.");
      ui->stackedWidget->setCurrentIndex(1); // İkinci sayfa: Kod ve Yeni Şifre
    } else {
      ModernMessageBox::critical(this, "Hata", message);
    }
  });
}

void SifremiUnuttumDialog::on_sifreYenileButon_clicked() {
  QString code = ui->kodLineEdit->text().trimmed();
  QString newPassword = ui->yeniSifreLineEdit->text();

  if (code.isEmpty() || newPassword.isEmpty()) {
    ModernMessageBox::critical(this, "Hata", "Lütfen tüm alanları doldurun.");
    return;
  }

  ui->sifreYenileButon->setEnabled(false);
  ui->sifreYenileButon->setText("İşleniyor...");

  m_networkManager->resetPassword(
      m_email, code, newPassword, [=](bool success, QString message) {
        ui->sifreYenileButon->setEnabled(true);
        ui->sifreYenileButon->setText("Şifreyi Yenile");

        if (success) {
          ModernMessageBox::information(
              this, "Başarılı",
              "Şifreniz başarıyla güncellendi. Şimdi giriş yapabilirsiniz.");
          accept(); // Diyaloğu kapat
        } else {
          ModernMessageBox::critical(this, "Hata", message);
        }
      });
}
