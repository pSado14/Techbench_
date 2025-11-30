#ifndef SIFREMIUNUTTUMDIALOG_H
#define SIFREMIUNUTTUMDIALOG_H

#include "networkmanager.h"
#include <QDialog>


namespace Ui {
class SifremiUnuttumDialog;
}

class SifremiUnuttumDialog : public QDialog {
  Q_OBJECT

public:
  explicit SifremiUnuttumDialog(NetworkManager *networkManager,
                                QWidget *parent = nullptr);
  ~SifremiUnuttumDialog();

private slots:
  void on_kodGonderButon_clicked();
  void on_sifreYenileButon_clicked();

private:
  Ui::SifremiUnuttumDialog *ui;
  NetworkManager *m_networkManager;
  QString m_email;
};

#endif // SIFREMIUNUTTUMDIALOG_H
