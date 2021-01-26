#ifndef DIALOGCONTROLPOINTCOMMENT_H
#define DIALOGCONTROLPOINTCOMMENT_H

#include <QDialog>
#include <QList>

namespace Ui {
class DialogControlPointComment;
}

class QCheckBox;

class DialogControlPointComment : public QDialog {
  Q_OBJECT

public:
  explicit DialogControlPointComment(QWidget *parent = nullptr);
  ~DialogControlPointComment();

  void SetComment(const QString &text);

  void SetLabel(const QString &text);

  QString GetComment();

public slots:
  void OnCheckBoxToggled(bool bChecked);
  void OnOk();

private:
  Ui::DialogControlPointComment *ui;

  QList<QCheckBox *> m_listCheckBoxes;
};

#endif // DIALOGCONTROLPOINTCOMMENT_H
