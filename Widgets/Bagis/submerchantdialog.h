#ifndef SUBMERCHANTDIALOG_H
#define SUBMERCHANTDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPoint>
#include <QTextEdit>
#include <QWidget>

class SubMerchantDialog : public QDialog {
  Q_OBJECT

public:
  explicit SubMerchantDialog(QWidget *parent = nullptr);
  ~SubMerchantDialog();

  struct SubMerchantInfo {
    QString name;
    QString surname;
    QString email;
    QString iban;
    QString identityNumber;
    QString phone;
    QString address;
  };

  SubMerchantInfo getSubMerchantInfo() const;
  void setEmail(const QString &email);

protected:
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;

private slots:
  void onSaveClicked();
  void validateInputs();

private:
  QPoint m_dragPosition;
  SubMerchantInfo info;

  QLineEdit *nameEdit;
  QLineEdit *surnameEdit;
  QLineEdit *emailEdit;
  QLineEdit *ibanEdit;
  QLineEdit *identityEdit;
  QLineEdit *phoneEdit;
  QTextEdit *addressEdit;

  void setupUi();
  bool validateIban(const QString &iban);
  bool validateIdentityNumber(const QString &tc);
};

#endif // SUBMERCHANTDIALOG_H
