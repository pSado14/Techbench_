#include "networkmanager.h"
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QUrlQuery>

NetworkManager *NetworkManager::m_instance = nullptr;

NetworkManager *NetworkManager::instance() {
  if (!m_instance) {
    m_instance = new NetworkManager();
  }
  return m_instance;
}

NetworkManager::NetworkManager(QObject *parent) : QObject(parent) {
  manager = new QNetworkAccessManager(this);
}

// --- KAYIT İŞLEMİ ---
void NetworkManager::registerUser(const QString &username,
                                  const QString &password, const QString &email,
                                  std::function<void(bool, QString)> callback) {
  QUrl url(BASE_URL + "/register"); // Node.js'deki endpoint
  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

  // Gönderilecek Veri (JSON)
  QJsonObject json;
  json["username"] = username;
  json["password"] = password;
  json["email"] = email;

  QJsonDocument doc(json);
  QByteArray data = doc.toJson();

  // POST İsteği Gönder
  QNetworkReply *reply = manager->post(request, data);

  // Cevap Geldiğinde
  connect(reply, &QNetworkReply::finished, [=]() {
    QByteArray responseData = reply->readAll();
    QJsonDocument jsonResponse = QJsonDocument::fromJson(responseData);
    QJsonObject jsonObject = jsonResponse.object();

    bool success = false;
    QString message = "Sunucu Hatası";

    if (reply->error() == QNetworkReply::NoError) {
      if (!jsonObject.isEmpty()) {
        success = jsonObject["success"].toBool();
        message = jsonObject["message"].toString();
      } else {
        success = true;
        message = "Kayıt Başarılı";
      }
    } else {
      if (!jsonObject.isEmpty()) {
        message = jsonObject["message"].toString();
      } else {
        message = "Bağlantı Hatası: " + reply->errorString();
      }
    }

    callback(success, message);
    reply->deleteLater();
  });
}

// --- GİRİŞ İŞLEMİ ---
void NetworkManager::loginUser(
    const QString &username, const QString &password,
    std::function<void(bool, QString, QVariantMap)> callback) {
  QUrl url(BASE_URL + "/login"); // Node.js'deki endpoint
  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

  QJsonObject json;
  json["username"] = username;
  json["password"] = password;

  QNetworkReply *reply = manager->post(request, QJsonDocument(json).toJson());

  connect(reply, &QNetworkReply::finished, [=]() {
    QByteArray responseData = reply->readAll();
    QJsonDocument jsonResponse = QJsonDocument::fromJson(responseData);
    QJsonObject jsonObject = jsonResponse.object();

    bool success = false;
    QString message = "Giriş Hatası";
    QVariantMap userData;

    if (reply->error() == QNetworkReply::NoError) {
      if (!jsonObject.isEmpty()) {
        success = jsonObject["success"].toBool();
        message = jsonObject["message"].toString();

        if (success && jsonObject.contains("user")) {
          userData = jsonObject["user"].toObject().toVariantMap();
        }
      } else {
        success = true;
        message = "Giriş Başarılı";
      }
    } else {
      if (!jsonObject.isEmpty()) {
        message = jsonObject["message"].toString();
      } else {
        message = "Sunucuya Ulaşılamadı: " + reply->errorString();
      }
    }

    callback(success, message, userData);
    reply->deleteLater();
  });
}

// --- HESAP SİLME İŞLEMİ (YENİ EKLENDİ) ---
void NetworkManager::deleteAccount(
    const QString &username, std::function<void(bool, QString)> callback) {
  QUrl url(BASE_URL + "/delete-account"); // Node.js'deki yeni endpoint
  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

  QJsonObject json;
  json["username"] = username; // Silinecek kullanıcı adı

  QNetworkReply *reply = manager->post(request, QJsonDocument(json).toJson());

  connect(reply, &QNetworkReply::finished, [=]() {
    QByteArray responseData = reply->readAll();
    QJsonDocument jsonResponse = QJsonDocument::fromJson(responseData);
    QJsonObject jsonObject = jsonResponse.object();

    bool success = false;
    QString message = "Silme İşlemi Hatası";

    if (reply->error() == QNetworkReply::NoError) {
      if (!jsonObject.isEmpty()) {
        success = jsonObject["success"].toBool();
        message = jsonObject["message"].toString();
      } else {
        // JSON boşsa bile status 200 geldiyse başarılı sayabiliriz
        // ama backend genelde JSON döner.
        success = true;
        message = "İşlem Başarılı";
      }
    } else {
      if (!jsonObject.isEmpty()) {
        message = jsonObject["message"].toString();
      } else {
        message = "Bağlantı Hatası: " + reply->errorString();
      }
    }

    callback(success, message);
    reply->deleteLater();
  });
}

// --- SKOR KAYDETME İŞLEMİ ---
void NetworkManager::saveScore(const QString &username, const QString &cpu,
                               const QString &gpu, const QString &ram,
                               int score, int cpuScore, int gpuScore,
                               int ramScore,
                               std::function<void(bool, QString)> callback) {
  QUrl url(BASE_URL + "/save-score");
  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

  QJsonObject json;
  json["username"] = username;
  json["cpu"] = cpu;
  json["gpu"] = gpu;
  json["ram"] = ram;
  json["score"] = score;
  json["cpu_score"] = cpuScore;
  json["gpu_score"] = gpuScore;
  json["ram_score"] = ramScore;

  QNetworkReply *reply = manager->post(request, QJsonDocument(json).toJson());

  connect(reply, &QNetworkReply::finished, [=]() {
    QByteArray responseData = reply->readAll();
    QJsonDocument jsonResponse = QJsonDocument::fromJson(responseData);
    QJsonObject jsonObject = jsonResponse.object();

    bool success = false;
    QString message = "Skor Kaydetme Hatası";

    if (reply->error() == QNetworkReply::NoError) {
      if (!jsonObject.isEmpty()) {
        success = jsonObject["success"].toBool();
        message = jsonObject["message"].toString();
      } else {
        success = true;
        message = "Skor Kaydedildi";
      }
    } else {
      if (!jsonObject.isEmpty()) {
        message = jsonObject["message"].toString();
      } else {
        message = "Bağlantı Hatası: " + reply->errorString();
      }
    }

    callback(success, message);
    reply->deleteLater();
  });
}

// --- RAKİP SİSTEMLERİ GETİRME İŞLEMİ ---
void NetworkManager::getRivals(
    std::function<void(bool, QList<QVariantMap>, QString)> callback) {
  QUrl url(BASE_URL + "/rivals");
  QNetworkRequest request(url);

  QNetworkReply *reply = manager->get(request);

  connect(reply, &QNetworkReply::finished, [=]() {
    QByteArray responseData = reply->readAll();
    QJsonDocument jsonResponse = QJsonDocument::fromJson(responseData);

    bool success = false;
    QString message = "Veri Çekme Hatası";
    QList<QVariantMap> rivalsList;

    if (reply->error() == QNetworkReply::NoError) {
      if (jsonResponse.isArray()) {
        QJsonArray jsonArray = jsonResponse.array();
        for (const QJsonValue &value : jsonArray) {
          rivalsList.append(value.toObject().toVariantMap());
        }
        success = true;
        message = "Rakipler Getirildi";
      } else if (jsonResponse.isObject()) {
        // Belki { success: false, message: "..." } döndü
        QJsonObject obj = jsonResponse.object();
        if (obj.contains("success") && !obj["success"].toBool()) {
          success = false;
          message = obj["message"].toString();
        } else {
          // Beklenmedik format
          success = false;
          message = "Sunucu yanıtı geçersiz formatta.";
        }
      }
    } else {
      message = "Bağlantı Hatası: " + reply->errorString();
    }

    callback(success, rivalsList, message);
    reply->deleteLater();
  });
}

// --- BAĞIŞ İSTEĞİ OLUŞTURMA ---
void NetworkManager::createDonationRequest(
    const QString &username, const QString &title, const QString &category,
    int price, std::function<void(bool, QString)> callback) {
  QUrl url(BASE_URL + "/create-donation-request");
  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

  QJsonObject json;
  json["username"] = username;
  json["title"] = title;
  json["category"] = category;
  json["price"] = price;

  QNetworkReply *reply = manager->post(request, QJsonDocument(json).toJson());

  connect(reply, &QNetworkReply::finished, [=]() {
    QByteArray responseData = reply->readAll();
    QJsonDocument jsonResponse = QJsonDocument::fromJson(responseData);
    QJsonObject jsonObject = jsonResponse.object();

    bool success = false;
    QString message = "Hata oluştu";

    if (reply->error() == QNetworkReply::NoError) {
      if (!jsonObject.isEmpty()) {
        success = jsonObject["success"].toBool();
        message = jsonObject["message"].toString();
      } else {
        success = true;
        message = "İstek oluşturuldu";
      }
    } else {
      if (!jsonObject.isEmpty()) {
        message = jsonObject["message"].toString();
      } else {
        message = "Bağlantı Hatası: " + reply->errorString();
      }
    }

    callback(success, message);
    reply->deleteLater();
  });
}

// --- BAĞIŞ İSTEKLERİNİ GETİRME ---
void NetworkManager::getDonationRequests(
    std::function<void(bool, QList<QVariantMap>, QString)> callback) {
  QUrl url(BASE_URL + "/donation-requests");
  QNetworkRequest request(url);

  QNetworkReply *reply = manager->get(request);

  connect(reply, &QNetworkReply::finished, [=]() {
    QByteArray responseData = reply->readAll();
    QJsonDocument jsonResponse = QJsonDocument::fromJson(responseData);

    bool success = false;
    QString message = "Veri Çekme Hatası";
    QList<QVariantMap> requestsList;

    if (reply->error() == QNetworkReply::NoError) {
      if (jsonResponse.isArray()) {
        QJsonArray jsonArray = jsonResponse.array();
        for (const QJsonValue &value : jsonArray) {
          requestsList.append(value.toObject().toVariantMap());
        }
        success = true;
        message = "İstekler Getirildi";
      } else if (jsonResponse.isObject()) {
        QJsonObject obj = jsonResponse.object();
        if (obj.contains("success") && !obj["success"].toBool()) {
          success = false;
          message = obj["message"].toString();
        }
      }
    } else {
      message = "Bağlantı Hatası: " + reply->errorString();
    }

    callback(success, requestsList, message);
    reply->deleteLater();
  });
}

// --- TEST GEÇMİŞİNİ GETİRME ---
void NetworkManager::getScoreHistory(
    const QString &username,
    std::function<void(bool, QList<QVariantMap>, QString)> callback) {
  QUrl url(BASE_URL + "/score-history?username=" + username);
  QNetworkRequest request(url);

  QNetworkReply *reply = manager->get(request);

  connect(reply, &QNetworkReply::finished, [=]() {
    QByteArray responseData = reply->readAll();
    QJsonDocument jsonResponse = QJsonDocument::fromJson(responseData);

    bool success = false;
    QString message = "Veri Çekme Hatası";
    QList<QVariantMap> historyList;

    if (reply->error() == QNetworkReply::NoError) {
      if (jsonResponse.isArray()) {
        QJsonArray jsonArray = jsonResponse.array();
        for (const QJsonValue &value : jsonArray) {
          historyList.append(value.toObject().toVariantMap());
        }
        success = true;
        message = "Geçmiş Getirildi";
      } else if (jsonResponse.isObject()) {
        QJsonObject obj = jsonResponse.object();
        if (obj.contains("success") && !obj["success"].toBool()) {
          success = false;
          message = obj["message"].toString();
        }
      }
    } else {
      message = "Bağlantı Hatası: " + reply->errorString();
    }

    callback(success, historyList, message);
    reply->deleteLater();
  });
}

// --- TEST GEÇMİŞİNİ SİLME ---
void NetworkManager::deleteScoreHistory(
    const QString &username, std::function<void(bool, QString)> callback) {
  QUrl url(BASE_URL + "/delete-history");
  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

  QJsonObject json;
  json["username"] = username;

  QNetworkReply *reply = manager->post(request, QJsonDocument(json).toJson());

  connect(reply, &QNetworkReply::finished, [=]() {
    QByteArray responseData = reply->readAll();
    QJsonDocument jsonResponse = QJsonDocument::fromJson(responseData);
    QJsonObject jsonObject = jsonResponse.object();

    bool success = false;
    QString message = "Hata oluştu";

    if (reply->error() == QNetworkReply::NoError) {
      if (!jsonObject.isEmpty()) {
        success = jsonObject["success"].toBool();
        message = jsonObject["message"].toString();
      } else {
        success = true;
        message = "Geçmiş silindi";
      }
    } else {
      if (!jsonObject.isEmpty()) {
        message = jsonObject["message"].toString();
      } else {
        message = "Bağlantı Hatası: " + reply->errorString();
      }
    }

    callback(success, message);
    reply->deleteLater();
  });
}

// --- ŞİFREMİ UNUTTUM - KOD GÖNDERME ---
void NetworkManager::sendForgotPasswordCode(
    const QString &email, std::function<void(bool, QString)> callback) {
  QUrl url(BASE_URL + "/forgot-password");
  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

  QJsonObject json;
  json["email"] = email;

  QNetworkReply *reply = manager->post(request, QJsonDocument(json).toJson());

  connect(reply, &QNetworkReply::finished, [=]() {
    QByteArray responseData = reply->readAll();
    QJsonDocument jsonResponse = QJsonDocument::fromJson(responseData);
    QJsonObject jsonObject = jsonResponse.object();

    bool success = false;
    QString message = "Hata oluştu";

    if (reply->error() == QNetworkReply::NoError) {
      if (!jsonObject.isEmpty()) {
        success = jsonObject["success"].toBool();
        message = jsonObject["message"].toString();
      } else {
        success = true;
        message = "Kod gönderildi";
      }
    } else {
      if (!jsonObject.isEmpty()) {
        message = jsonObject["message"].toString();
      } else {
        message = "Bağlantı Hatası: " + reply->errorString();
      }
    }

    callback(success, message);
    reply->deleteLater();
  });
}

// --- KULLANICI SIRALAMASI ---
void NetworkManager::getUserRanking(
    const QString &username,
    std::function<void(bool success, int ranking, QString message)> callback) {
  QUrl url(BASE_URL + "/ranking");
  QUrlQuery query;
  query.addQueryItem("username", username);
  url.setQuery(query);

  qDebug() << "Ranking Request URL:" << url.toString();

  QNetworkRequest request(url);
  QNetworkReply *reply = manager->get(request);

  connect(reply, &QNetworkReply::finished, [=]() {
    if (reply->error() == QNetworkReply::NoError) {
      QByteArray responseData = reply->readAll();
      QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
      QJsonObject jsonObj = jsonDoc.object();

      if (jsonObj["success"].toBool()) {
        int ranking = jsonObj["ranking"].toInt();
        callback(true, ranking, "Sıralama alındı.");
      } else {
        callback(false, 0, jsonObj["message"].toString());
      }
    } else {
      callback(false, 0, "Ağ hatası: " + reply->errorString());
    }
    reply->deleteLater();
  });
}

// --- ŞİFRE SIFIRLAMA ---
void NetworkManager::resetPassword(
    const QString &email, const QString &code, const QString &newPassword,
    std::function<void(bool, QString)> callback) {
  QUrl url(BASE_URL + "/reset-password");
  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

  QJsonObject json;
  json["email"] = email;
  json["code"] = code;
  json["newPassword"] = newPassword;

  QNetworkReply *reply = manager->post(request, QJsonDocument(json).toJson());

  connect(reply, &QNetworkReply::finished, [=]() {
    QByteArray responseData = reply->readAll();
    QJsonDocument jsonResponse = QJsonDocument::fromJson(responseData);
    QJsonObject jsonObject = jsonResponse.object();

    bool success = false;
    QString message = "Hata oluştu";

    if (reply->error() == QNetworkReply::NoError) {
      if (!jsonObject.isEmpty()) {
        success = jsonObject["success"].toBool();
        message = jsonObject["message"].toString();
      } else {
        success = true;
        message = "Şifre güncellendi";
      }
    } else {
      if (!jsonObject.isEmpty()) {
        message = jsonObject["message"].toString();
      } else {
        message = "Bağlantı Hatası: " + reply->errorString();
      }
    }

    callback(success, message);
    reply->deleteLater();
  });
}

// --- FİYAT ÇEKME ---
void NetworkManager::getPrice(
    const QString &productName, const QString &category,
    std::function<void(bool, QString, QString)> callback) {
  QUrl url(BASE_URL + "/get-price");
  QUrlQuery query;
  query.addQueryItem("productName", productName);
  query.addQueryItem("category", category);
  url.setQuery(query);

  QNetworkRequest request(url);
  QNetworkReply *reply = manager->get(request);

  connect(reply, &QNetworkReply::finished, [=]() {
    if (reply->error() == QNetworkReply::NoError) {
      QByteArray responseData = reply->readAll();
      QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
      QJsonObject jsonObj = jsonDoc.object();

      bool success = jsonObj["success"].toBool();
      QString price = jsonObj["price"].toString();
      QString source = jsonObj["source"].toString();

      callback(success, price, source);
    } else {
      callback(false, "Hata", "");
    }
    reply->deleteLater();
  });
}

// --- ÜRÜN ARAMA (Dropdown için) ---
void NetworkManager::searchProducts(
    const QString &query, const QString &category,
    std::function<void(bool success, QList<QVariantMap> results,
                       QString message)>
        callback) {
  QUrl url(BASE_URL + "/search-products");
  QUrlQuery q;
  q.addQueryItem("query", query);
  if (!category.isEmpty()) {
    q.addQueryItem("category", category);
  }
  url.setQuery(q);

  QNetworkRequest request(url);
  QNetworkReply *reply = manager->get(request);

  connect(reply, &QNetworkReply::finished, [=]() {
    if (reply->error() == QNetworkReply::NoError) {
      QByteArray responseData = reply->readAll();
      QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
      QJsonObject jsonObj = jsonDoc.object();

      bool success = jsonObj["success"].toBool();
      QString message = "";
      QList<QVariantMap> results;

      if (success) {
        QJsonArray jsonArray = jsonObj["results"].toArray();
        for (const QJsonValue &value : jsonArray) {
          results.append(value.toObject().toVariantMap());
        }
      } else {
        message = jsonObj["message"].toString();
      }

      callback(success, results, message);
    } else {
      callback(false, {}, "Bağlantı Hatası: " + reply->errorString());
    }
    reply->deleteLater();
  });
}

// --- ÖDEME BAŞLATMA ---
void NetworkManager::initializePayment(
    const QString &username, const QString &email, int price,
    const QString &productName, const QString &receiverUsername, int requestId,
    std::function<void(bool, QString, QString)> callback) {
  QUrl url(BASE_URL + "/payment/initialize");
  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

  QJsonObject json;
  json["price"] = price;
  json["productName"] = productName;
  json["receiverUsername"] = receiverUsername;
  json["requestId"] = requestId;

  QJsonObject userObj;
  userObj["name"] = username;
  userObj["email"] = email;
  json["user"] = userObj;

  QNetworkReply *reply = manager->post(request, QJsonDocument(json).toJson());

  connect(reply, &QNetworkReply::finished, [=]() {
    QByteArray responseData = reply->readAll();
    QJsonDocument jsonResponse = QJsonDocument::fromJson(responseData);
    QJsonObject jsonObject = jsonResponse.object();

    bool success = false;
    QString message = "Hata oluştu";
    QString paymentUrl = "";

    if (reply->error() == QNetworkReply::NoError) {
      if (!jsonObject.isEmpty()) {
        success = jsonObject["success"].toBool();
        if (success) {
          paymentUrl = jsonObject["paymentPageUrl"].toString();
          message = "Ödeme sayfası oluşturuldu";
        } else {
          message = jsonObject["message"].toString();
        }
      }
    } else {
      message = "Bağlantı Hatası: " + reply->errorString();
    }

    callback(success, paymentUrl, message);
    reply->deleteLater();
  });
}

// --- SUNUCUYU DURDURMA ---
void NetworkManager::stopServer() {
  QUrl url(BASE_URL + "/shutdown");
  QNetworkRequest request(url);
  manager->get(request);
}

// --- SUB-MERCHANT KONTROLÜ ---
void NetworkManager::checkSubMerchant(
    const QString &username,
    std::function<void(bool success, bool hasSubMerchant, QString message)>
        callback) {
  QUrl url(BASE_URL + "/check-submerchant");
  QUrlQuery query;
  query.addQueryItem("username", username);
  url.setQuery(query);

  QNetworkRequest request(url);
  QNetworkReply *reply = manager->get(request);

  connect(reply, &QNetworkReply::finished, [=]() {
    QByteArray responseData = reply->readAll();
    QJsonDocument jsonResponse = QJsonDocument::fromJson(responseData);
    QJsonObject jsonObject = jsonResponse.object();

    bool success = false;
    bool hasSubMerchant = false;
    QString message = "Kontrol hatası";

    if (reply->error() == QNetworkReply::NoError) {
      if (!jsonObject.isEmpty()) {
        success = jsonObject["success"].toBool();
        hasSubMerchant = jsonObject["hasSubMerchant"].toBool();
        message =
            success ? "Kontrol tamamlandı" : jsonObject["message"].toString();
      }
    } else {
      message = "Bağlantı Hatası: " + reply->errorString();
    }

    callback(success, hasSubMerchant, message);
    reply->deleteLater();
  });
}

// --- SUB-MERCHANT KAYDI ---
void NetworkManager::registerSubMerchant(
    const QString &username, const QString &name, const QString &surname,
    const QString &email, const QString &phone, const QString &identityNumber,
    const QString &iban, const QString &address,
    std::function<void(bool success, QString message)> callback) {
  QUrl url(BASE_URL + "/register-submerchant");
  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

  QJsonObject json;
  json["username"] = username;
  json["name"] = name;
  json["surname"] = surname;
  json["email"] = email;
  json["phone"] = phone;
  json["identityNumber"] = identityNumber;
  json["iban"] = iban;
  json["address"] = address;

  QNetworkReply *reply = manager->post(request, QJsonDocument(json).toJson());

  connect(reply, &QNetworkReply::finished, [=]() {
    QByteArray responseData = reply->readAll();
    QJsonDocument jsonResponse = QJsonDocument::fromJson(responseData);
    QJsonObject jsonObject = jsonResponse.object();

    bool success = false;
    QString message = "Kayıt hatası";

    if (reply->error() == QNetworkReply::NoError) {
      if (!jsonObject.isEmpty()) {
        success = jsonObject["success"].toBool();
        message = jsonObject["message"].toString();
      }
    } else {
      if (!jsonObject.isEmpty()) {
        message = jsonObject["message"].toString();
      } else {
        message = "Bağlantı Hatası: " + reply->errorString();
      }
    }

    callback(success, message);
    reply->deleteLater();
  });
}
