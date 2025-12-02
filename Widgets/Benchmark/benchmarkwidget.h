#ifndef BENCHMARKWIDGET_H
#define BENCHMARKWIDGET_H

#include <QWidget>

#include <QChartView>
#include <QLineSeries>
#include <QPieSeries>
#include <QtCharts>

namespace Ui {
class BenchmarkWidget;
}

class BenchmarkWidget : public QWidget {
  Q_OBJECT

public:
  explicit BenchmarkWidget(QWidget *parent = nullptr);
  ~BenchmarkWidget();

signals:
  void testBitti(int cpuScore, int gpuScore, int ramScore);

public slots:
  void enableStartButton();
  void updateHistoryList(const QList<QVariantMap> &history);
  void setUsername(QString username) { m_username = username; }

private slots:
  void on_testi_baslat_buton_clicked();
  void on_testi_iptal_et_buton_clicked();
  void on_gecmisi_temizle_buton_clicked();
  void onWorkerProgress(int percent, int step);
  void onWorkerFinished(int cpuScore, int gpuScore, int ramScore);
  void onWorkerCanceled();
  void onLogMessage(QString message);
  void onGpuDetected(QString gpuName);

private:
  Ui::BenchmarkWidget *ui;
  QThread *m_thread;
  class BenchmarkWorker *m_worker;

  // Grafikler
  QChart *m_chart;
  QChartView *m_chartView;
  QLineSeries *m_cpuSeries;
  QLineSeries *m_gpuSeries;
  QLineSeries *m_ramSeries;

  QString m_detectedGpuName;
  QString m_username;
  QListWidget *m_historyList; // UI'dan gelecek ama pointer olarak tutabiliriz
                              // veya direkt ui-> kullaniriz.
  // UI dosyasinda tanimli oldugu icin m_historyList'e gerek yok aslinda, direkt
  // ui->sonuc_gecmisi_listesi kullanilabilir. Ancak kod tutarliligi icin
  // setupHistoryList icinde atama yapabiliriz.

  void setupHistoryList();

  void setupCharts();
  void showResultCharts(int cpu, int gpu, int ram);
  void clearLayout(QLayout *layout);
};

#endif // BENCHMARKWIDGET_H
