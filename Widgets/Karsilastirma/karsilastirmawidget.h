#ifndef KARSILASTIRMAWIDGET_H
#define KARSILASTIRMAWIDGET_H

#include <QWidget>

#include <QListWidget>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QValueAxis>

namespace Ui {
class KarsilastirmaWidget;
}

class KarsilastirmaWidget : public QWidget {
  Q_OBJECT

public:
  explicit KarsilastirmaWidget(QWidget *parent = nullptr);
  ~KarsilastirmaWidget();

  void setSizinSisteminiz(QString cpu, QString gpu, QString ram);
  void setKullaniciBilgileri(QString username);
  void setSizinPuaniniz(int totalScore);
  void updateRivalsList(const QList<QVariantMap> &rivals);
  void showLoginWarning(); // <-- YENİ

  QString getCpu() const { return m_cpu; }
  QString getGpu() const { return m_gpu; }
  QString getRam() const { return m_ram; }

private slots:
  void on_karsilastir_buton_clicked();
  void on_arama_kutusu_textChanged(const QString &arg1);

private:
  Ui::KarsilastirmaWidget *ui;

  QString m_username;
  QString m_cpu;
  QString m_gpu;
  QString m_ram;
  int m_myScore = 0;

  // Chart Members
  QChart *m_chart;
  QChartView *m_chartView;
  QBarSeries *m_series;
  QBarSet *m_mySet;
  QBarSet *m_rivalSet;
  QBarCategoryAxis *m_axisX;
  QValueAxis *m_axisY;

  void updateSizinSisteminizUI();
  void setupComparisonChart();
  void updateChart(int rivalScore);

  // UI Elements created programmatically
};

#endif // KARSILASTIRMAWIDGET_H
