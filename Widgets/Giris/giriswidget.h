#ifndef GIRISWIDGET_H
#define GIRISWIDGET_H

#include <QWidget>
#include "networkmanager.h" // <-- 1. BURAYA EKLENDİ

namespace Ui {
class GirisWidget;
}

class GirisWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GirisWidget(QWidget *parent = nullptr);
    ~GirisWidget();

    // MainWindow'un dinleyeceği SİNYALLER
signals:
              // Giriş başarılı olursa
    void kayitOlTiklandi();        // "Kayıt Ol" butonuna basılırsa
    void sifreUnuttumTiklandi();   // "Şifremi Unuttum"a basılırsa
    void girisBasarili(QString username); // <-- Parametre ekledik

private slots:
    // UI dosyasındaki buton isimleriyle tam uyumlu otomatik slotlar:
    void on_giris_yap_butonu_sayfa_clicked();
    void on_kayit_ol_butonu_sayfa_clicked();
    void on_sifre_unuttum_butonu_sayfa_clicked();

private:
    Ui::GirisWidget *ui;
    NetworkManager *netManager; // <-- 2. BURAYA EKLENDİ
};

#endif // GIRISWIDGET_H
