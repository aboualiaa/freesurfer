#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class QImage;

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

  void Load(int, char *[]);

  void keyPressEvent(QKeyEvent *);

protected slots:
  void OnCurrentImageChanged(const QImage &image, int n);

private:
  Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
