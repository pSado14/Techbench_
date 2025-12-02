#ifndef PRICEFETCHER_H
#define PRICEFETCHER_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>


class PriceFetcher : public QObject {
  Q_OBJECT
public:
  explicit PriceFetcher(QObject *parent = nullptr);
  void searchPrice(QString productName); // Fiyatı aranacak ürün

signals:
  void priceFound(QString price); // Fiyat bulununca bu sinyali yayacak

private slots:
  void onResult(QNetworkReply *reply);

private:
  QNetworkAccessManager *manager;
};

#endif // PRICEFETCHER_H
