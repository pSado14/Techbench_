#include "benchmarkworker.h"
#include <QDebug>
#include <QElapsedTimer>
#include <QThread>
#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdlib>
#include <future>
#include <iostream>
#include <vector>

// Windows için rand_r implementasyonu (MinGW'de olmayabilir)
#ifndef rand_r
int rand_r(unsigned int *seedp) {
  *seedp = *seedp * 1103515245 + 12345;
  return (unsigned int)(*seedp / 65536) % 32768;
}
#endif

BenchmarkWorker::BenchmarkWorker(bool cpu, bool gpu, bool ram, QObject *parent)
    : QObject(parent), m_runCpu(cpu), m_runGpu(gpu), m_runRam(ram),
      m_stop(false) {}

void BenchmarkWorker::requestInterruption() { m_stop = true; }

void BenchmarkWorker::process() {
  int cpuScore = 0;
  int gpuScore = 0;
  int ramScore = 0;

  // CPU Testi
  if (m_runCpu && !m_stop) {
    cpuScore = runCPUTest();
  }

  // GPU Testi
  if (m_runGpu && !m_stop) {
    gpuScore = runGPUTest();
  }

  // RAM Testi
  if (m_runRam && !m_stop) {
    ramScore = runRAMTest();
  }

  if (m_stop) {
    emit canceled();
  } else {
    emit finished(cpuScore, gpuScore, ramScore);
  }
}

int BenchmarkWorker::runCPUTest() {
  QElapsedTimer timer;
  timer.start();

  int threadCount = QThread::idealThreadCount();
  if (threadCount < 1)
    threadCount = 1;

  std::vector<std::future<void>> futures;
  std::atomic<bool> stopFlag(false);
  std::atomic<int> totalPrimes(0);
  std::atomic<long long> totalOps(0);

  // Her thread için asal sayı kontrolü + FPU işlemleri + matris çarpımı
  auto task = [&](int id) {
    // Asal sayı aralığı: 2 - 30.000.000 (~15000 puan hedefi)
    int start = 2 + (id * 30000000 / threadCount);
    int end = 2 + ((id + 1) * 30000000 / threadCount);
    int localPrimes = 0;
    volatile double fpuResult = 0.0;
    long long localOps = 0;

    // Mini matris (4x4) için
    double matrix1[4][4], matrix2[4][4], result[4][4];
    for (int i = 0; i < 4; i++)
      for (int j = 0; j < 4; j++) {
        matrix1[i][j] = (i + j) * 0.1;
        matrix2[i][j] = (i - j) * 0.2;
      }

    for (int n = start; n < end; ++n) {
      if (m_stop || stopFlag)
        return;

      // Asal sayı kontrolü
      bool isPrime = true;
      if (n < 2)
        isPrime = false;
      else if (n == 2)
        isPrime = true;
      else if (n % 2 == 0)
        isPrime = false;
      else {
        for (int i = 3; i * i <= n; i += 2) {
          if (n % i == 0) {
            isPrime = false;
            break;
          }
        }
      }

      if (isPrime)
        localPrimes++;

      // Ağır FPU işlemleri (sin, cos, tan, sqrt, pow, log, exp)
      double x = (double)n * 0.0001;
      fpuResult += std::sin(x);
      fpuResult += std::cos(x);
      fpuResult += std::tan(x * 0.1);
      fpuResult += std::sqrt((double)n);
      fpuResult += std::pow(x, 1.5);
      fpuResult += std::log(n + 1.0);
      fpuResult += std::exp(x * 0.0001);
      localOps += 7;

      // Her 10000 sayıda bir matris çarpımı yap
      if (n % 10000 == 0) {
        for (int i = 0; i < 4; i++) {
          for (int j = 0; j < 4; j++) {
            result[i][j] = 0;
            for (int k = 0; k < 4; k++) {
              result[i][j] += matrix1[i][k] * matrix2[k][j];
            }
            fpuResult += result[i][j];
          }
        }
        localOps += 64;
      }
    }

    totalPrimes += localPrimes;
    totalOps += localOps;
  };

  emit logMessage(
      QString("CPU Testi: %1 thread kullanılıyor").arg(threadCount));

  // Threadleri başlat
  for (int i = 0; i < threadCount; ++i) {
    futures.push_back(std::async(std::launch::async, task, i));
  }

  // Threadlerin bitmesini bekle (gerçek iş süresi ölçülecek)
  int completed = 0;
  for (auto &f : futures) {
    f.wait();
    completed++;
    emit progress((completed * 100) / threadCount, 0);
  }

  qint64 elapsed = timer.elapsed();
  if (elapsed == 0)
    elapsed = 1;

  emit logMessage(QString("CPU Testi: %1 asal, %2M işlem tamamlandı")
                      .arg(totalPrimes.load())
                      .arg(totalOps.load() / 1000000));

  // Puanlama: 45.000.000 / süre (ms)
  return 45000000 / elapsed;
}

#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>

int BenchmarkWorker::runGPUTest() {
  QElapsedTimer timer;
  timer.start();

  // 1. OpenGL Context Oluştur
  QOpenGLContext context;
  if (!context.create()) {
    emit logMessage("OpenGL Context oluşturulamadı!");
    return 0;
  }

  QOffscreenSurface surface;
  surface.setFormat(context.format());
  surface.create();

  if (!context.makeCurrent(&surface)) {
    emit logMessage("OpenGL Context aktif edilemedi!");
    return 0;
  }

  // GPU İsmini Al
  const GLubyte *renderer = context.functions()->glGetString(GL_RENDERER);
  QString gpuName = QString::fromLatin1((const char *)renderer);
  emit gpuDetected(gpuName);
  emit logMessage("GPU Tespit Edildi: " + gpuName);

  // 2. Shader Hazırla (Ağır Yük)
  QOpenGLShaderProgram program;
  program.addShaderFromSourceCode(QOpenGLShader::Vertex, R"(
        attribute vec4 position;
        void main() {
            gl_Position = position;
        }
    )");

  // Mandelbrot Fraktalı - GPU'yu yormak için (BALANCED EXTREME MODE)
  program.addShaderFromSourceCode(QOpenGLShader::Fragment, R"(
        uniform vec2 resolution;
        uniform float time;
        void main() {
            vec2 uv = gl_FragCoord.xy / resolution.xy;
            vec2 c = (uv - 0.5) * 2.0;
            c.x -= 0.5;
            vec2 z = vec2(0.0);
            int iter = 0;
            // Iterasyon sayısını 25.000'e düşürdük (TDR önlemek için)
            for(int i=0; i<25000; i++) { 
                float x = (z.x * z.x - z.y * z.y) + c.x;
                float y = (z.y * z.x + z.x * z.y) + c.y;
                
                // Ekstra Matematiksel Yük
                float heavy = sin(x*10.0 + time) * cos(y*10.0);
                heavy = pow(abs(heavy), 1.5); 
                x += heavy * 0.000001; 

                if((x * x + y * y) > 4.0) break;
                z.x = x;
                z.y = y;
                iter++;
            }
            gl_FragColor = vec4(float(iter)/25000.0, 0.0, 0.0, 1.0);
        }
    )");

  if (!program.link()) {
    emit logMessage("Shader Link Hatası: " + program.log());
    return 0;
  }

  if (!program.bind()) {
    emit logMessage("Shader Bind Hatası!");
    return 0;
  }

  // 3. Framebuffer (Render Hedefi) - 4K Çözünürlük
  QOpenGLFramebufferObjectFormat fboFormat;
  fboFormat.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
  QOpenGLFramebufferObject fbo(3840, 2160, fboFormat); // 4K Render
  fbo.bind();

  // 4. Render Döngüsü (~10000 puan hedefi, ~4.5 saniye)
  int frameCount = 0;
  int targetFrames = 40; // Kare sayısı

  float vertices[] = {-1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f};

  program.enableAttributeArray("position");
  program.setAttributeArray("position", GL_FLOAT, vertices, 2);
  program.setUniformValue("resolution", QVector2D(3840, 2160));

  while (frameCount < targetFrames) {
    if (m_stop)
      break;

    program.setUniformValue("time", (float)frameCount * 0.01f);

    // GPU işlemi
    for (int k = 0; k < 30; k++) {
      context.functions()->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    context.functions()->glFinish();

    frameCount++;

    // İlerleme
    emit progress((frameCount * 100) / targetFrames, 1);
  }

  fbo.release();

  qint64 elapsed = timer.elapsed();
  if (elapsed == 0)
    elapsed = 1;

  // Puanlama
  // 4K'da 200 kareyi ne kadar hızlı çizerse o kadar iyi
  // Referans: İyi bir GPU 2-3 saniyede bitirebilir -> ~15000 puan
  return 45000000 / elapsed;
}

int BenchmarkWorker::runRAMTest() {
  QElapsedTimer timer;
  timer.start();

  // RAM Testi: 64 MB veri transfer hızı (~4000 puan hedefi, ~2 saniye)
  size_t targetSize = 64ULL * 1024 * 1024; // 64 MB
  std::vector<char> memory;

  try {
    memory.resize(targetSize);
  } catch (...) {
    emit logMessage("RAM Testi: Yetersiz Bellek!");
    return 0;
  }

  emit logMessage(QString("RAM Testi: %1 MB buffer ayrıldı")
                      .arg(targetSize / (1024 * 1024)));

  // Pass 1: Sıralı Yazma
  for (size_t i = 0; i < targetSize; ++i) {
    if (m_stop)
      return 0;
    memory[i] = (char)((i * 7) % 256);
    if (i % (targetSize / 50) == 0) {
      emit progress((i * 50) / targetSize, 2);
    }
  }

  // Pass 2: Sıralı Okuma
  volatile long long sum = 0;
  for (size_t i = 0; i < targetSize; ++i) {
    if (m_stop)
      return 0;
    sum += memory[i];
    if (i % (targetSize / 50) == 0) {
      emit progress(50 + (i * 50) / targetSize, 2);
    }
  }

  qint64 elapsed = timer.elapsed();
  if (elapsed == 0)
    elapsed = 1;

  emit logMessage(QString("RAM Testi: %1 MB okuma/yazma tamamlandı")
                      .arg(targetSize / (1024 * 1024)));

  // Puanlama: 8.000.000 / süre (ms)
  return 8000000 / elapsed;
}
