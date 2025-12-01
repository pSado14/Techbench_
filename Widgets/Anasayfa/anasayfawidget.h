#ifndef ANASAYFAWIDGET_H
#define ANASAYFAWIDGET_H

#include <QProcess>
#include <QWidget>

namespace Ui {
class AnasayfaWidget;
}

class AnasayfaWidget : public QWidget {
  Q_OBJECT

public:
  explicit AnasayfaWidget(QWidget *parent = nullptr);
  ~AnasayfaWidget();

  // MainWindow'dan çağrılacak ve kullanıcı adını ekrana basacak
  void setKullaniciBilgileri(const QString &ad);
  void setSistemBilgileri(const QString &cpu, const QString &gpu,
                          const QString &ram); // <-- YENİ
  void taraVeGuncelle();
  void setPuanlar(int cpuScore, int gpuScore, int ramScore);
  void bilgileriSifirla();

signals:
  void hesapSilmeTiklandi();
  void sistemBilgileriGuncellendi(QString cpu, QString gpu, QString ram);
  void puanlarGuncellendi(int totalScore);

private slots:
  void on_hesapSilButon_clicked();
  void on_sistemibilgilerinitarabuton_clicked();

private:
  Ui::AnasayfaWidget *ui;

  // Donanım bilgisini çeken yardımcı fonksiyon
  QString getHardwareInfo(const QString &cmd, const QStringList &args);

  // Charts
  void setupCharts();
  void updateCharts(int cpuScore, int gpuScore, int ramScore);

  // Puanları saklamak için üye değişkenler
  int m_cpuScore = 0;
  int m_gpuScore = 0;
  int m_ramScore = 0;
};

#endif // ANASAYFAWIDGET_H
