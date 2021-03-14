#ifndef MACHELPER_H
#define MACHELPER_H

#include <QIcon>
#include <QObject>
#include <QPixmap>
#include <QPointer>

class QWidget;

class MacHelper : public QObject {
  Q_OBJECT
public:
  MacHelper(QObject *parent = 0);

  static QPixmap InvertPixmap(const QPixmap &pix);

  static QIcon InvertIcon(const QIcon &icn_in, const QSize &size = QSize(),
                          bool bTwoStates = false);
  static bool  IsDarkMode();
};

#endif // MACHELPER_H
