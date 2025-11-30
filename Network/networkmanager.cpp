#include "networkmanager.h"
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>

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
void NetworkManager::loginUser(const QString &username, const QString &password,
                               std::function<void(bool, QString)> callback) {
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

    if (reply->error() == QNetworkReply::NoError) {
      if (!jsonObject.isEmpty()) {
        success = jsonObject["success"].toBool();
        message = jsonObject["message"].toString();
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

    callback(success, message);
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
                               int score,
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
