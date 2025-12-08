#ifndef ANASAYFAWIDGET_H
#define ANASAYFAWIDGET_H

#include <QComboBox>
#include <QLabel>
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
                          const QString &ram);
  void setPrice(const QString &type, const QString &price,
                const QString &source);
  void setToplamPuan(int score);
  void taraVeGuncelle();
  void setPuanlar(int cpuScore, int gpuScore, int ramScore);

  void bilgileriSifirla();
  void checkPrice(const QString &productName, const QString &type);

signals:
  void hesapSilmeTiklandi();
  void priceCheckRequested(QString componentName, QString type);
  void sistemBilgileriGuncellendi(QString cpu, QString gpu, QString ram);
  void puanlarGuncellendi(int totalScore);

private slots:
  void on_sistemibilgilerinitarabuton_clicked();
  void on_hesapSilButon_clicked();

private:
  Ui::AnasayfaWidget *ui;
  QString getHardwareInfo(const QString &cmd, const QStringList &args);
  QStringList getAllHardwareInfo(const QString &cmd, const QStringList &args);
  void setupCharts();
  void updateCharts(int cpuScore, int gpuScore, int ramScore);

  // Puanları saklamak için üye değişkenler
  int m_cpuScore = 0;
  int m_gpuScore = 0;
  int m_ramScore = 0;

  // GPU Seçimi için
  QComboBox *gpuModelComboBox;
  QLabel *gpuSelectionLabel;

private slots:
  void onGpuModelSelected(int index);
};

#endif // ANASAYFAWIDGET_H
