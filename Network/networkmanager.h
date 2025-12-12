#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QVariantMap>
#include <functional>

class NetworkManager : public QObject {
  Q_OBJECT
public:
  static NetworkManager *instance();
  explicit NetworkManager(QObject *parent = nullptr);

  // Kayıt Ol İsteği
  void
  registerUser(const QString &username, const QString &password,
               const QString &email,
               std::function<void(bool success, QString message)> callback);

  // Giriş Yap İsteği
  void loginUser(
      const QString &username, const QString &password,
      std::function<void(bool success, QString message, QVariantMap userData)>
          callback);

  // --- YENİ EKLENEN: Hesap Silme İsteği ---
  void
  deleteAccount(const QString &username,
                std::function<void(bool success, QString message)> callback);

  // Skor Kaydetme İsteği
  // Skor Kaydetme İsteği
  void saveScore(const QString &username, const QString &cpu,
                 const QString &gpu, const QString &ram, int score,
                 int cpuScore, int gpuScore, int ramScore,
                 std::function<void(bool success, QString message)> callback);

  // Rakip Sistemleri Getirme İsteği
  void getRivals(std::function<void(bool success, QList<QVariantMap> rivals,
                                    QString message)>
                     callback);

  // --- YENİ: Kullanıcı Sıralamasını Getirme ---
  void getUserRanking(
      const QString &username,
      std::function<void(bool success, int ranking, QString message)> callback);

  // --- YENİ: Bağış İsteği Oluşturma ---
  void createDonationRequest(
      const QString &username, const QString &title, const QString &category,
      int price, std::function<void(bool success, QString message)> callback);

  // --- YENİ: Bağış İsteklerini Getirme ---
  void getDonationRequests(
      std::function<void(bool success, QList<QVariantMap> requests,
                         QString message)>
          callback);

  // --- YENİ: Test Geçmişini Getirme ---
  void
  getScoreHistory(const QString &username,
                  std::function<void(bool success, QList<QVariantMap> history,
                                     QString message)>
                      callback);

  // --- YENİ: Test Geçmişini Silme ---
  void deleteScoreHistory(
      const QString &username,
      std::function<void(bool success, QString message)> callback);

  // Şifremi Unuttum - Kod Gönderme
  void sendForgotPasswordCode(
      const QString &email,
      std::function<void(bool success, QString message)> callback);

  // Şifre Sıfırlama - Yeni Şifre Belirleme
  void
  resetPassword(const QString &email, const QString &code,
                const QString &newPassword,
                std::function<void(bool success, QString message)> callback);

  // --- YENİ: Parça Fiyatı Getirme ---
  void getPrice(const QString &productName, const QString &category,
                std::function<void(bool success, QString price, QString source)>
                    callback);

  // --- YENİ: Ürün Arama (Dropdown için) ---
  void
  searchProducts(const QString &query, const QString &category,
                 std::function<void(bool success, QList<QVariantMap> results,
                                    QString message)>
                     callback);

  // --- YENİ: Ödeme Başlatma ---
  void initializePayment(
      const QString &username, const QString &email, int price,
      const QString &productName, const QString &receiverUsername,
      int requestId,
      std::function<void(bool success, QString paymentUrl, QString message)>
          callback);

  // --- YENİ: Sunucuyu Durdurma ---
  void stopServer();

  // --- YENİ: Sub-Merchant Kontrolü ---
  void checkSubMerchant(
      const QString &username,
      std::function<void(bool success, bool hasSubMerchant, QString message)>
          callback);

  // --- YENİ: Sub-Merchant Kaydı ---
  void registerSubMerchant(
      const QString &username, const QString &name, const QString &surname,
      const QString &email, const QString &phone, const QString &identityNumber,
      const QString &iban, const QString &address,
      std::function<void(bool success, QString message)> callback);

  // --- YENİ: Liderlik Tablosunu Getirme ---
  void getLeaderboard(
      std::function<void(bool success, QList<QVariantMap> leaderboard,
                         QString message)>
          callback);

  // --- YENİ: Destekçileri Getirme ---
  void getTopSupporters(
      const QString &username,
      std::function<void(bool success, QList<QVariantMap> supporters,
                         QString message)>
          callback);

private:
  QNetworkAccessManager *manager;
  const QString BASE_URL = "http://127.0.0.1:3000"; // Node.js sunucu adresiniz
  static NetworkManager *m_instance;
};

#endif // NETWORKMANAGER_H
