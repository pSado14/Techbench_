
#include "Widgets/Main/mainwindow.h"
#include <QApplication>
#include <QNetworkProxyFactory>
#include <cstdio>

int main(int argc, char *argv[]) {
  // Redirect stdout and stderr to a file for debugging
  freopen("startup_debug.txt", "w", stdout);
  freopen("startup_debug.txt", "w", stderr);

  printf("Starting application...\n");
  fflush(stdout);

  QApplication a(argc, argv);
  printf("QApplication created.\n");
  fflush(stdout);

  QNetworkProxyFactory::setUseSystemConfiguration(true);

  printf("Creating MainWindow...\n");
  fflush(stdout);
  MainWindow w;

  printf("Showing MainWindow...\n");
  fflush(stdout);
  w.show();

  printf("Entering event loop...\n");
  fflush(stdout);
  return a.exec();
}
