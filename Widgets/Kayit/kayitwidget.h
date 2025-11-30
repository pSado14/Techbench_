#ifndef KAYITWIDGET_H
#define KAYITWIDGET_H

#include <QWidget>
#include "networkmanager.h" // NetworkManager'ı dahil ediyoruz

namespace Ui {
class KayitWidget;
}

class KayitWidget : public QWidget
{
    Q_OBJECT

public:
    explicit KayitWidget(QWidget *parent = nullptr);
    ~KayitWidget();

    // MainWindow'un dinleyeceği sinyaller
signals:
    void kayitBasarili();      // Kayıt başarılı olursa tetiklenir
    void giriseDonTiklandi();  // "Giriş Yap" butonuna basılırsa tetiklenir

private slots:
    // UI dosyasındaki buton isimlerine göre otomatik slotlar
    void on_kayit_tamamla_buton_clicked();
    void on_girise_don_buton_clicked();

private:
    Ui::KayitWidget *ui;

    // --- DÜZELTME: Sınıfın içine taşıdık ---
    NetworkManager *netManager;
};

#endif // KAYITWIDGET_H
