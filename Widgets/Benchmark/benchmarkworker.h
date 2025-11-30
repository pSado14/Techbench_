#ifndef BENCHMARKWORKER_H
#define BENCHMARKWORKER_H

// #include <CL/cl.h>
#include <QObject>
#include <atomic>

class BenchmarkWorker : public QObject {
  Q_OBJECT

public:
  explicit BenchmarkWorker(bool cpu, bool gpu, bool ram,
                           QObject *parent = nullptr);
  void requestInterruption();

public slots:
  void process();

signals:
  void finished(int cpuScore, int gpuScore, int ramScore);
  void canceled();
  void progress(int percent, int step); // step: 0=CPU, 1=GPU, 2=RAM
  void gpuDetected(QString gpuName);
  void logMessage(QString message);

private:
  bool m_runCpu;
  bool m_runGpu;
  bool m_runRam;
  std::atomic<bool> m_stop;

  int runCPUTest();
  int runGPUTest();
  int runRAMTest();
};

#endif // BENCHMARKWORKER_H
