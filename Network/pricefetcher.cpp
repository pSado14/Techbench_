#include "pricefetcher.h"
#include <QDebug>
#include <QNetworkRequest>
#include <QRegularExpression>

PriceFetcher::PriceFetcher(QObject *parent) : QObject(parent) {
  manager = new QNetworkAccessManager(this);
  // İnternetten cevap geldiğinde onResult fonksiyonunu çalıştır
  connect(manager, &QNetworkAccessManager::finished, this,
          &PriceFetcher::onResult);
}

void PriceFetcher::searchPrice(QString productName) {
  // Örnek olarak Google Alışveriş veya Teknoloji Mağazası araması yapalım.
  // Not: Google HTML yapısı çok karışıktır, Vatan/Teknosa gibi sitelerin arama
  // linkleri daha kolay parse edilir. Burada örnek olarak "Cimri" benzeri bir
  // yapı varsayalım.

  QString urlStr = "https://www.google.com/search?tbm=shop&q=" + productName;
  QUrl url(urlStr);
  QNetworkRequest request(url);

  // KENDİMİZİ TARAYICI GİBİ GÖSTERMELİYİZ (Çok Önemli!)
  request.setHeader(
      QNetworkRequest::UserAgentHeader,
      "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, "
      "like Gecko) Chrome/115.0.0.0 Safari/537.36");

  qDebug() << "Aranıyor:" << productName;
  manager->get(request);
}

void PriceFetcher::onResult(QNetworkReply *reply) {
  if (reply->error()) {
    qDebug() << "PriceFetcher Hata:" << reply->errorString();
    emit priceFound("Hata: " + reply->errorString());
    reply->deleteLater();
    return;
  }

  QString html = reply->readAll();
  qDebug() << "PriceFetcher: HTML alındı, boyut:" << html.length();

  // HTML'in başını yazdıralım (Bloklanma kontrolü)
  qDebug() << "HTML Başlangıç:" << html.left(500);

  // Daha esnek regex:
  // 1. "15.000 TL"
  // 2. "15,000 TL"
  // 3. "15 000 TL"
  // 4. "TL 15.000"
  // Google Shopping genelde <span class="HRLxBb">15.000,00 TL</span> gibi
  // yapılar kullanır. Ancak class isimleri değişebilir. Basitçe rakam ve TL
  // arayan bir regex deneyelim.

  // Örnek: 15.000 TL, 15,000.00 TL, 15000 TL
  QRegularExpression regex("([\\d\\.,]+)\\s*TL");
  QRegularExpressionMatchIterator i = regex.globalMatch(html);

  if (i.hasNext()) {
    // İlk eşleşmeyi alalım (Genelde en üstteki sonuç)
    QRegularExpressionMatch match = i.next();
    QString price = match.captured(1);
    qDebug() << "PriceFetcher: Fiyat Bulundu:" << price;
    emit priceFound(price + " TL");
  } else {
    qDebug() << "PriceFetcher: Fiyat bulunamadı. HTML içinde 'TL' ile biten "
                "sayısal değer yok.";
    emit priceFound("Bulunamadı");
  }
  reply->deleteLater();
}
