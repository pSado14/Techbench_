#include "sifremiunuttumdialog.h"
#include "ui_sifremiunuttumdialog.h"
#include <QMessageBox>

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

  ui->stackedWidget->setCurrentIndex(0); // İlk sayfa: E-posta girişi
}

SifremiUnuttumDialog::~SifremiUnuttumDialog() { delete ui; }

void SifremiUnuttumDialog::on_kodGonderButon_clicked() {
  QString email = ui->emailLineEdit->text().trimmed();

  if (email.isEmpty()) {
    QMessageBox::warning(this, "Hata", "Lütfen e-posta adresinizi girin.");
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
      QMessageBox::information(this, "Başarılı",
                               "Doğrulama kodu e-posta adresinize gönderildi.");
      ui->stackedWidget->setCurrentIndex(1); // İkinci sayfa: Kod ve Yeni Şifre
    } else {
      QMessageBox::critical(this, "Hata", message);
    }
  });
}

void SifremiUnuttumDialog::on_sifreYenileButon_clicked() {
  QString code = ui->kodLineEdit->text().trimmed();
  QString newPassword = ui->yeniSifreLineEdit->text();

  if (code.isEmpty() || newPassword.isEmpty()) {
    QMessageBox::warning(this, "Hata", "Lütfen tüm alanları doldurun.");
    return;
  }

  ui->sifreYenileButon->setEnabled(false);
  ui->sifreYenileButon->setText("İşleniyor...");

  m_networkManager->resetPassword(
      m_email, code, newPassword, [=](bool success, QString message) {
        ui->sifreYenileButon->setEnabled(true);
        ui->sifreYenileButon->setText("Şifreyi Yenile");

        if (success) {
          QMessageBox::information(
              this, "Başarılı",
              "Şifreniz başarıyla güncellendi. Şimdi giriş yapabilirsiniz.");
          accept(); // Diyaloğu kapat
        } else {
          QMessageBox::critical(this, "Hata", message);
        }
      });
}
