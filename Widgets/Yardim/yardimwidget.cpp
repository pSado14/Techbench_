#include "yardimwidget.h"
#include "ui_yardimwidget.h"
#include <QEasingCurve>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QLabel>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

// --- FAQItem Implementation ---

FAQItem::FAQItem(const QString &question, const QString &answer,
                 QWidget *parent)
    : QWidget(parent), m_isExpanded(false) {

  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(0, 5, 0, 5);
  mainLayout->setSpacing(0);

  // 1. Toggle Button (Soru)
  m_toggleBtn = new QPushButton(question, this);
  m_toggleBtn->setCursor(Qt::PointingHandCursor);
  m_toggleBtn->setStyleSheet("QPushButton {"
                             "  text-align: left;"
                             "  padding: 15px;"
                             "  background-color: #363a45;"
                             "  color: #ffffff;"
                             "  border: none;"
                             "  border-radius: 8px;"
                             "  font-weight: bold;"
                             "  font-size: 14px;"
                             "}"
                             "QPushButton:hover {"
                             "  background-color: #404452;"
                             "}");

  connect(m_toggleBtn, &QPushButton::clicked, this, &FAQItem::onToggle);
  mainLayout->addWidget(m_toggleBtn);

  // 2. Answer Frame (Cevap Alanı)
  m_answerFrame = new QFrame(this);
  m_answerFrame->setStyleSheet("background-color: transparent;");
  m_answerFrame->setFrameShape(QFrame::NoFrame);
  m_answerFrame->setMaximumHeight(0); // Başlangıçta kapalı
  // m_answerFrame->setClipsChildren(true); // QWidget'ta bu metod yok,
  // varsayılan olarak clip'lenir.

  QVBoxLayout *frameLayout = new QVBoxLayout(m_answerFrame);
  frameLayout->setContentsMargins(15, 10, 15, 10);

  m_answerLabel = new QLabel(answer, m_answerFrame);
  m_answerLabel->setWordWrap(true);
  m_answerLabel->setStyleSheet(
      "color: #bdc3c7; font-size: 13px; line-height: 1.4;");

  frameLayout->addWidget(m_answerLabel);
  mainLayout->addWidget(m_answerFrame);

  // Animasyon
  m_animation = new QPropertyAnimation(m_answerFrame, "maximumHeight", this);
  m_animation->setDuration(300);
  m_animation->setEasingCurve(QEasingCurve::OutQuad);
}

void FAQItem::onToggle() {
  m_animation->stop();

  if (m_isExpanded) {
    // Kapat
    m_animation->setStartValue(m_answerFrame->height());
    m_animation->setEndValue(0);
    m_toggleBtn->setStyleSheet("QPushButton {"
                               "  text-align: left;"
                               "  padding: 15px;"
                               "  background-color: #363a45;"
                               "  color: #ffffff;"
                               "  border: none;"
                               "  border-radius: 8px;"
                               "  font-weight: bold;"
                               "  font-size: 14px;"
                               "}"
                               "QPushButton:hover {"
                               "  background-color: #404452;"
                               "}");
  } else {
    // Aç
    // İçerik yüksekliğini hesapla - Layout'un sizeHint'ini kullanmak daha doğru
    int contentHeight = m_answerFrame->layout()->sizeHint().height();
    m_animation->setStartValue(m_answerFrame->height());
    m_animation->setEndValue(contentHeight);

    // Aktif buton stili
    m_toggleBtn->setStyleSheet("QPushButton {"
                               "  text-align: left;"
                               "  padding: 15px;"
                               "  background-color: #3498db;"
                               "  color: #ffffff;"
                               "  border: none;"
                               "  border-radius: 8px;"
                               "  font-weight: bold;"
                               "  font-size: 14px;"
                               "}"
                               "QPushButton:hover {"
                               "  background-color: #2980b9;"
                               "}");
  }

  m_animation->start();
  m_isExpanded = !m_isExpanded;
}

// --- YardimWidget Implementation ---

YardimWidget::YardimWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::YardimWidget) {
  ui->setupUi(this);

  setupModernUI();

  // --- KATEGORİLER VE SORULAR ---

  // 1. Kategori: Genel Sorular
  kategoriEkle("Genel Sorular");
  soruEkle(
      "TechBench nedir ve ne işe yarar?",
      "TechBench, bilgisayarınızın donanım performansını (CPU, GPU, RAM) test "
      "eden ve diğer sistemlerle karşılaştırmanızı sağlayan gelişmiş bir "
      "araçtır.");
  soruEkle(
      "Sistemim uyumlu mu?",
      "TechBench, Windows 10 ve üzeri işletim sistemlerinde çalışacak "
      "şekilde tasarlanmıştır. OpenGL 3.0+ destekli bir ekran kartı önerilir.");
  soruEkle("Verilerim güvende mi?",
           "Evet. Kişisel verileriniz şifrelenerek saklanır ve üçüncü "
           "şahıslarla paylaşılmaz. "
           "Ödeme işlemleri ise iyzico güvencesiyle gerçekleşir.");

  // 2. Kategori: Hesap İşlemleri
  kategoriEkle("Hesap İşlemleri");
  soruEkle("Giriş yapmak önemli mi?",
           "Evet. Giriş yaparak test sonuçlarınızı buluta kaydedebilir, "
           "geçmiş testlerinizi görüntüleyebilir ve global sıralamada yer "
           "alabilirsiniz.");
  soruEkle("Hesabımı silebilir miyim?",
           "Evet. Anasayfa menüsünden hesabınızı ve tüm verilerinizi "
           "kalıcı olarak silebilirsiniz.");
  soruEkle(
      "Şifremi unuttum, ne yapmalıyım?",
      "Giriş ekranındaki 'Şifremi Unuttum' bağlantısını kullanarak e-posta "
      "adresinize sıfırlama bağlantısı gönderebilirsiniz.");

  // 3. Kategori: Benchmark ve Performans
  kategoriEkle("Benchmark ve Performans");
  soruEkle("Puanlar ne anlama geliyor?",
           "Puanlar donanımınızın ham işlem gücünü temsil eder. "
           "Daha yüksek puan, daha yüksek performans demektir.");
  soruEkle("Benchmark sonuçlarım neden düşük?",
           "Arka planda çalışan uygulamalar, güncel olmayan sürücüler veya "
           "yetersiz soğutma performansı sonuçları etkileyebilir.");
  soruEkle("Testler ne kadar sürer?",
           "Sisteminizin hızına bağlı olarak testler genellikle 30 saniye ile "
           "2 dakika arasında tamamlanır.");
  soruEkle(
      "Sürücülerimi güncellemeli miyim?",
      "Kesinlikle. Özellikle ekran kartı (GPU) sürücülerinizin güncel olması, "
      "test sonuçlarınızın doğruluğu ve performansı için kritiktir.");

  // 4. Kategori: Bağış Sistemi
  kategoriEkle("Bağış Sistemi");
  soruEkle(
      "Bağış Yap ne anlama geliyor?",
      "Bu özellik, kullanıcılar arasında doğrudan finansal etkileşimi sağlayan "
      "bir P2P (Eşten Eşe) mikro-ödeme mekanizmasıdır. iyzico güvenli ödeme "
      "altyapısı "
      "kullanılarak, kullanıcıların beğendikleri sistem sahiplerine veya "
      "topluluğa katkı "
      "sağlayanlara güvenli bir şekilde maddi destek (bağış) sağlamasına "
      "olanak tanınır.");
  soruEkle("Ben de bağış alabilir miyim?",
           "Evet. "
           "sistemlerinizi beğenen diğer kullanıcılardan güvenli bir şekilde "
           "bağış alabilirsiniz.");

  // En sona esnek boşluk ekle
  m_scrollLayout->addStretch();
}

YardimWidget::~YardimWidget() { delete ui; }

void YardimWidget::setupModernUI() {
  // 1. Ana Widget Layout'u (Sayfayı tam kaplaması için)
  if (!this->layout()) {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10); // Kenarlardan biraz boşluk
    mainLayout->addWidget(ui->sikca_sorulan_frame);
  }

  // Eski QTreeWidget'ı gizle/kaldır
  if (ui->sss_listesi) {
    ui->sss_listesi->setVisible(false);
  }

  // Ana layout'u bul (sikca_sorulan_frame içindeki layout)
  QVBoxLayout *frameLayout =
      qobject_cast<QVBoxLayout *>(ui->sikca_sorulan_frame->layout());
  if (!frameLayout) {
    frameLayout = new QVBoxLayout(ui->sikca_sorulan_frame);
  }

  // Scroll Area Oluştur
  m_scrollArea = new QScrollArea(this);
  m_scrollArea->setWidgetResizable(true);
  m_scrollArea->setFrameShape(QFrame::NoFrame);

  // Scrollbar politikasını AlwaysOn yapıyoruz ki içerik genişliği sürekli
  // değişip titreme yapmasın
  m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

  m_scrollArea->setStyleSheet(
      "QScrollArea { background: transparent; border: none; }"
      "QScrollBar:vertical {"
      "    border: none;"
      "    background-color: #2b2e38;"
      "    width: 12px;" /* Biraz daha geniş ve tutulabilir */
      "    margin: 0px;"
      "    border-radius: 6px;"
      "}"
      "QScrollBar::handle:vertical {"
      "    background-color: #505563;" /* Daha belirgin bir gri */
      "    min-height: 30px;"
      "    border-radius: 6px;"
      "    margin: 2px;" /* Kenarlardan boşluk bırakarak 'yüzen' hissi ver */
      "}"
      "QScrollBar::handle:vertical:hover { background: #4facfe; }" /* Hover
                                                                      rengi */
      "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: "
      "0px; }"
      "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { "
      "background: none; }");

  // Scroll Content Widget
  m_scrollContent = new QWidget();
  m_scrollContent->setStyleSheet("background: transparent;");

  m_scrollLayout = new QVBoxLayout(m_scrollContent);
  m_scrollLayout->setSpacing(10);
  m_scrollLayout->setContentsMargins(10, 10, 20,
                                     10); // Sağ tarafta scrollbar için boşluk

  m_scrollArea->setWidget(m_scrollContent);

  // Frame Layout'a ekle
  frameLayout->addWidget(m_scrollArea);
}

void YardimWidget::kategoriEkle(const QString &baslik) {
  QLabel *label = new QLabel(baslik, m_scrollContent);
  label->setStyleSheet("color: #4facfe;"
                       "font-size: 16px;"
                       "font-weight: bold;"
                       "margin-top: 15px;"
                       "margin-bottom: 5px;");
  m_scrollLayout->addWidget(label);
}

void YardimWidget::soruEkle(const QString &soru, const QString &cevap) {
  FAQItem *item = new FAQItem(soru, cevap, m_scrollContent);
  m_scrollLayout->addWidget(item);
}
