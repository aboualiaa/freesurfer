#include "MainWindow.h"
#include <QtGui/QApplication>

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);
  a.setApplicationName("dummy_qt");
  MainWindow w;
  w.show();

  return a.exec();
}
