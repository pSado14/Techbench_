#include "mainwindow.h"
#include <QApplication>
#include <QIcon>

extern "C" {
__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);

  // Buradaki a.setWindowIcon... satırını SİL veya YORUMA AL.
  // a.setWindowIcon(QIcon(":/logo.png"));

  MainWindow w;

  // --- İKONU BURADA, PENCERE OLUŞTUKTAN SONRA VER ---
  w.setWindowIcon(QIcon(":/Assets/logo.png"));

  w.show();
  return a.exec();
}
