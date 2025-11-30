#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include "networkmanager.h" // <-- 1. EKLENDİ

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class AnasayfaWidget;
class BenchmarkWidget;
class KarsilastirmaWidget;
class YardimWidget;
class GirisWidget;
class BagisWidget;
class KayitWidget;
class QPushButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Mevcut slotlar...
    void on_anasayfabuton_clicked();
    void on_benchmarkbuton_clicked();
    void on_karsilastirmabuton_clicked();
    void on_yardimbuton_clicked();
    void on_girisyapbuton_clicked();
    void on_bagisyapbuton_clicked();

    // --- 2. YENİ: HESAP SİLME SLOTU ---
    void on_hesapSilmeIstegi_geldi();

private:
    Ui::MainWindow *ui;

    AnasayfaWidget      *m_anasayfa;
    BenchmarkWidget     *m_benchmark;
    KarsilastirmaWidget *m_karsilastirma;
    YardimWidget        *m_yardim;
    GirisWidget         *m_giris;
    BagisWidget         *m_bagis;
    KayitWidget         *m_kayit;

    QList<QPushButton*> menuButtons;
    void updateButtonStyles(QPushButton* clickedButton);
    void setupPages();
    void setupMenuButtons();

    bool userIsLoggedIn;
    void setLoginState(bool loggedIn);

    // --- 3. YENİ DEĞİŞKENLER ---
    QString currentUsername;    // Kim giriş yaptı?
    NetworkManager *netManager; // Silme isteği göndermek için
};
#endif // MAINWINDOW_H
