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

  // Her thread için görev fonksiyonu
  auto task = [&](int id) {
    // Fizik Testi: 5000/N Küp Çarpışma Simülasyonu
    struct Particle {
      float x, y, z;
      float vx, vy, vz;
    };

    int particleCount = 5000 / threadCount;
    std::vector<Particle> particles(particleCount);

    // Rastgele Başlangıç (Thread-safe rand lazım veya basit bir seed)
    unsigned int seed = id * 1000 + 1;
    for (int i = 0; i < particleCount; ++i) {
      particles[i].x = (float)(rand_r(&seed) % 100);
      particles[i].y = (float)(rand_r(&seed) % 100);
      particles[i].z = (float)(rand_r(&seed) % 100);
      particles[i].vx = (float)(rand_r(&seed) % 10 - 5) * 0.1f;
      particles[i].vy = (float)(rand_r(&seed) % 10 - 5) * 0.1f;
      particles[i].vz = (float)(rand_r(&seed) % 10 - 5) * 0.1f;
    }

    int steps = 2000;
    for (int s = 0; s < steps; ++s) {
      if (m_stop || stopFlag)
        return;

      for (int i = 0; i < particleCount; ++i) {
        particles[i].x += particles[i].vx;
        particles[i].y += particles[i].vy;
        particles[i].z += particles[i].vz;

        if (particles[i].x < 0 || particles[i].x > 100)
          particles[i].vx *= -1;
        if (particles[i].y < 0 || particles[i].y > 100)
          particles[i].vy *= -1;
        if (particles[i].z < 0 || particles[i].z > 100)
          particles[i].vz *= -1;

        // Etkileşim
        for (int k = 0; k < 10; k++) {
          int other = rand_r(&seed) % particleCount;
          float dx = particles[i].x - particles[other].x;
          float dy = particles[i].y - particles[other].y;
          float dz = particles[i].z - particles[other].z;
          float distSq = dx * dx + dy * dy + dz * dz;
          if (distSq < 1.0f && distSq > 0.0001f) {
            particles[i].vx += dx * 0.01f;
            particles[i].vy += dy * 0.01f;
            particles[i].vz += dz * 0.01f;
          }
        }
      }
    }
  };

  // Threadleri başlat
  for (int i = 0; i < threadCount; ++i) {
    futures.push_back(std::async(std::launch::async, task, i));
  }

  // İlerleme çubuğu simülasyonu (Ana thread beklerken)
  int steps = 2000;
  for (int s = 0; s < steps; s += steps / 100) {
    if (m_stop) {
      stopFlag = true;
      break;
    }
    QThread::msleep(10); // Biraz bekle
    emit progress((s * 100) / steps, 0);
  }

  // Threadlerin bitmesini bekle
  for (auto &f : futures) {
    f.wait();
  }

  qint64 elapsed = timer.elapsed();
  if (elapsed == 0)
    elapsed = 1;

  // Puanlama: Toplam iş / süre
  return (45000000 * threadCount) / elapsed;
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

  // 4. Render Döngüsü
  int frameCount = 0;
  int targetFrames = 200; // Kare sayısını artırdık (Daha akıcı test)

  float vertices[] = {-1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f};

  program.enableAttributeArray("position");
  program.setAttributeArray("position", GL_FLOAT, vertices, 2);
  program.setUniformValue("resolution", QVector2D(3840, 2160));

  while (frameCount < targetFrames) {
    if (m_stop)
      break;

    program.setUniformValue("time", (float)frameCount * 0.01f);

    // GPU'yu boğmak için arka arkaya çizim komutları
    // 200 Draw Call per Frame -> TDR süresini aşmadan GPU'yu meşgul et
    for (int k = 0; k < 200; k++) {
      context.functions()->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    context.functions()->glFinish(); // Tüm komutların bitmesini bekle

    frameCount++;

    // İlerleme
    if (frameCount % (targetFrames / 100) == 0) {
      emit progress((frameCount * 100) / targetFrames, 1);
    }
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

  // RAM Testi: Yüksek Bant Genişliği ve Kapasite
  // Hedef: 4 GB (veya sistem izin verirse)
  size_t targetSize = 4ULL * 1024 * 1024 * 1024; // 4 GB
  std::vector<char> memory;

  try {
    memory.resize(targetSize);
  } catch (const std::bad_alloc &) {
    // 4 GB ayrılamazsa 1 GB dene
    targetSize = 1ULL * 1024 * 1024 * 1024;
    try {
      memory.resize(targetSize);
    } catch (...) {
      emit logMessage("RAM Testi: Yetersiz Bellek!");
      return 0;
    }
  }

  // Yazma Testi (Bant Genişliği)
  // Belleği doldur (memset optimize edilmiştir, hızlıdır)
  // Ancak CPU cache'i aşmak için büyük bloklar halinde yazıyoruz
  size_t chunkSize = 1024 * 1024; // 1 MB chunks
  size_t chunks = targetSize / chunkSize;

  for (size_t i = 0; i < chunks; ++i) {
    if (m_stop)
      return 0;

    // Bloğu doldur
    std::fill(memory.begin() + i * chunkSize,
              memory.begin() + (i + 1) * chunkSize, (char)(i % 255));

    if (i % (chunks / 100) == 0) {
      emit progress((i * 50) / chunks, 2); // İlk %50 yazma
    }
  }

  // Okuma Testi (Doğrulama)
  volatile char val = 0;
  for (size_t i = 0; i < chunks; ++i) {
    if (m_stop)
      return 0;

    // Rastgele erişim yerine sıralı erişim (Bant genişliği ölçümü için)
    for (size_t j = 0; j < chunkSize; j += 4096) { // Her sayfadan bir bayt oku
      val = memory[i * chunkSize + j];
    }

    if (i % (chunks / 100) == 0) {
      emit progress(50 + (i * 50) / chunks, 2); // Son %50 okuma
    }
  }

  qint64 elapsed = timer.elapsed();
  if (elapsed == 0)
    elapsed = 1;

  // Puanlama: MB/s cinsinden hız
  double speedMBps =
      (double)(targetSize * 2) / (1024.0 * 1024.0) / (elapsed / 1000.0);
  return (int)speedMBps;
}
