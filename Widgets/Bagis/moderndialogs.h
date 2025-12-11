#ifndef MODERNDIALOGS_H
#define MODERNDIALOGS_H

#include <QDialog>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

class ModernInputDialog : public QDialog {
  Q_OBJECT
public:
  explicit ModernInputDialog(QWidget *parent = nullptr);
  static int getInt(QWidget *parent, const QString &title, const QString &label,
                    int value = 0, int min = -2147483647, int max = 2147483647,
                    int step = 1, bool *ok = nullptr);

private:
  void setupUi(const QString &title, const QString &labelText, int value,
               int min, int max, int step);
  QSpinBox *spinBox;
  QPushButton *okButton;
  QPushButton *cancelButton;
};

class ModernMessageBox : public QDialog {
  Q_OBJECT
public:
  explicit ModernMessageBox(QWidget *parent = nullptr);
  static void information(QWidget *parent, const QString &title,
                          const QString &text);
  static void critical(QWidget *parent, const QString &title,
                       const QString &text);
  static bool question(QWidget *parent, const QString &title,
                       const QString &text);
  static void warning(QWidget *parent, const QString &title,
                      const QString &text);

private:
  void setupUi(const QString &title, const QString &text, bool isError,
               bool isQuestion = false, bool isWarning = false);
};

#endif // MODERNDIALOGS_H
