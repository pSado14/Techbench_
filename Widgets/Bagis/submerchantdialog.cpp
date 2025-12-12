#include "submerchantdialog.h"
#include "moderndialogs.h"
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPushButton>
#include <QRegularExpression>
#include <QVBoxLayout>

SubMerchantDialog::SubMerchantDialog(QWidget *parent) : QDialog(parent) {
  setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
  setAttribute(Qt::WA_TranslucentBackground);
  resize(500, 650);
  setupUi();
}

SubMerchantDialog::~SubMerchantDialog() {}

void SubMerchantDialog::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    m_dragPosition = event->globalPos() - frameGeometry().topLeft();
    event->accept();
  }
}

void SubMerchantDialog::mouseMoveEvent(QMouseEvent *event) {
  if (event->buttons() & Qt::LeftButton) {
    move(event->globalPos() - m_dragPosition);
    event->accept();
  }
}

void SubMerchantDialog::setupUi() {
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(15, 15, 15, 15);

  // Main Frame (Rounded & Dark with gradient)
  QFrame *frame = new QFrame(this);
  frame->setObjectName("mainFrame");
  frame->setStyleSheet("#mainFrame { "
                       "   background-color: #2b2e38; "
                       "   border: 1px solid #3c404d; "
                       "   border-radius: 15px; "
                       "}");

  // Shadow effect
  QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
  shadow->setBlurRadius(30);
  shadow->setColor(QColor(0, 0, 0, 150));
  shadow->setOffset(0, 5);
  frame->setGraphicsEffect(shadow);

  mainLayout->addWidget(frame);

  QVBoxLayout *layout = new QVBoxLayout(frame);
  layout->setSpacing(15);
  layout->setContentsMargins(25, 25, 25, 25);

  // --- Title Bar ---
  QHBoxLayout *titleLayout = new QHBoxLayout();

  QLabel *titleLabel = new QLabel("Ödeme Bilgileri");
  titleLabel->setStyleSheet(
      "color: #ffffff; font-size: 20px; font-weight: bold; "
      "background: transparent;");

  QPushButton *closeBtn = new QPushButton("X");
  closeBtn->setFixedSize(30, 30);
  closeBtn->setCursor(Qt::PointingHandCursor);
  closeBtn->setStyleSheet("QPushButton { "
                          "   background-color: transparent; "
                          "   color: #bec3cd; "
                          "   border: none; "
                          "   font-weight: bold; "
                          "   font-size: 16px; "
                          "}"
                          "QPushButton:hover { "
                          "   color: #ff4d4d; "
                          "}");
  connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);

  titleLayout->addWidget(titleLabel);
  titleLayout->addStretch();
  titleLayout->addWidget(closeBtn);
  layout->addLayout(titleLayout);

  // --- Subtitle ---
  QLabel *subtitleLabel =
      new QLabel("Bağış alabilmek için aşağıdaki bilgileri doldurun:");
  subtitleLabel->setStyleSheet(
      "color: #8f94a8; font-size: 12px; background: transparent;");
  layout->addWidget(subtitleLabel);

  // --- Common Input Style ---
  QString inputStyle = "QLineEdit, QTextEdit { "
                       "   background-color: #1e2029; "
                       "   border: 1px solid #3c404d; "
                       "   border-radius: 8px; "
                       "   color: #e0e0e0; "
                       "   padding: 10px; "
                       "   font-size: 13px; "
                       "}"
                       "QLineEdit:focus, QTextEdit:focus { "
                       "   border: 1px solid #4facfe; "
                       "}";

  QString labelStyle =
      "color: #bec3cd; font-size: 12px; font-weight: bold; background: "
      "transparent;";

  // --- Name & Surname Row ---
  QHBoxLayout *nameRow = new QHBoxLayout();

  QVBoxLayout *nameCol = new QVBoxLayout();
  QLabel *nameLabel = new QLabel("Ad");
  nameLabel->setStyleSheet(labelStyle);
  nameEdit = new QLineEdit();
  nameEdit->setPlaceholderText("Adınız");
  nameEdit->setStyleSheet(inputStyle);
  nameCol->addWidget(nameLabel);
  nameCol->addWidget(nameEdit);

  QVBoxLayout *surnameCol = new QVBoxLayout();
  QLabel *surnameLabel = new QLabel("Soyad");
  surnameLabel->setStyleSheet(labelStyle);
  surnameEdit = new QLineEdit();
  surnameEdit->setPlaceholderText("Soyadınız");
  surnameEdit->setStyleSheet(inputStyle);
  surnameCol->addWidget(surnameLabel);
  surnameCol->addWidget(surnameEdit);

  nameRow->addLayout(nameCol);
  nameRow->addLayout(surnameCol);
  layout->addLayout(nameRow);

  // --- Email ---
  QLabel *emailLabel = new QLabel("E-posta");
  emailLabel->setStyleSheet(labelStyle);
  emailEdit = new QLineEdit();
  emailEdit->setPlaceholderText("ornek@email.com");
  emailEdit->setStyleSheet(inputStyle);
  layout->addWidget(emailLabel);
  layout->addWidget(emailEdit);

  // --- Phone ---
  QLabel *phoneLabel = new QLabel("Telefon Numarası");
  phoneLabel->setStyleSheet(labelStyle);
  phoneEdit = new QLineEdit();
  phoneEdit->setPlaceholderText("+90 5XX XXX XX XX");
  phoneEdit->setStyleSheet(inputStyle);
  layout->addWidget(phoneLabel);
  layout->addWidget(phoneEdit);

  // --- Identity Number ---
  QLabel *identityLabel = new QLabel("TC Kimlik Numarası");
  identityLabel->setStyleSheet(labelStyle);
  identityEdit = new QLineEdit();
  identityEdit->setPlaceholderText("11 haneli TC kimlik numarası");
  identityEdit->setMaxLength(11);
  identityEdit->setStyleSheet(inputStyle);
  layout->addWidget(identityLabel);
  layout->addWidget(identityEdit);

  // --- IBAN ---
  QLabel *ibanLabel = new QLabel("IBAN");
  ibanLabel->setStyleSheet(labelStyle);
  ibanEdit = new QLineEdit();
  ibanEdit->setPlaceholderText("TR00 0000 0000 0000 0000 0000 00");
  ibanEdit->setStyleSheet(inputStyle);
  layout->addWidget(ibanLabel);
  layout->addWidget(ibanEdit);

  // --- Address ---
  QLabel *addressLabel = new QLabel("Adres");
  addressLabel->setStyleSheet(labelStyle);
  addressEdit = new QTextEdit();
  addressEdit->setPlaceholderText("Açık adresiniz (İl, İlçe, Mahalle, Sokak, "
                                  "No)");
  addressEdit->setFixedHeight(80);
  addressEdit->setStyleSheet(inputStyle);
  layout->addWidget(addressLabel);
  layout->addWidget(addressEdit);

  // --- Info Text ---
  QLabel *infoText =
      new QLabel("Bu bilgiler Iyzico ödeme sistemi için gereklidir ve "
                 "güvenli şekilde saklanır.");
  infoText->setWordWrap(true);
  infoText->setAlignment(Qt::AlignCenter);
  infoText->setStyleSheet("color: #4facfe; font-size: 11px; background: "
                          "transparent; margin-top: 5px;");
  layout->addWidget(infoText);

  // --- Buttons ---
  QHBoxLayout *btnLayout = new QHBoxLayout();
  btnLayout->addStretch();

  QPushButton *cancelBtn = new QPushButton("İptal", this);
  cancelBtn->setCursor(Qt::PointingHandCursor);
  cancelBtn->setStyleSheet("QPushButton { "
                           "   background-color: transparent; "
                           "   color: #bec3cd; "
                           "   border: 1px solid #3c404d; "
                           "   border-radius: 8px; "
                           "   padding: 10px 25px; "
                           "   font-weight: bold; "
                           "   font-size: 13px; "
                           "}"
                           "QPushButton:hover { "
                           "   color: white; "
                           "   border-color: #4facfe; "
                           "   background-color: #3c404d; "
                           "}");
  connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

  QPushButton *saveBtn = new QPushButton("Kaydet ve Devam Et", this);
  saveBtn->setCursor(Qt::PointingHandCursor);
  saveBtn->setStyleSheet(
      "QPushButton { "
      "   background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, "
      "       stop:0 #4facfe, stop:1 #00f2fe); "
      "   color: white; "
      "   border-radius: 8px; "
      "   padding: 10px 30px; "
      "   font-weight: bold; "
      "   font-size: 13px; "
      "   border: none; "
      "}"
      "QPushButton:hover { "
      "   background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, "
      "       stop:0 #00f2fe, stop:1 #4facfe); "
      "}");
  connect(saveBtn, &QPushButton::clicked, this,
          &SubMerchantDialog::onSaveClicked);

  btnLayout->addWidget(cancelBtn);
  btnLayout->addSpacing(10);
  btnLayout->addWidget(saveBtn);
  layout->addLayout(btnLayout);
}

void SubMerchantDialog::setEmail(const QString &email) {
  emailEdit->setText(email);
}

bool SubMerchantDialog::validateIban(const QString &iban) {
  // Basit IBAN doğrulama - TR ile başlamalı ve 26 karakter olmalı
  QString cleanIban = iban.simplified().replace(" ", "");
  if (cleanIban.length() != 26)
    return false;
  if (!cleanIban.startsWith("TR"))
    return false;
  return true;
}

bool SubMerchantDialog::validateIdentityNumber(const QString &tc) {
  // TC Kimlik numarası 11 haneli olmalı ve 0 ile başlamamalı
  if (tc.length() != 11)
    return false;
  if (tc.startsWith("0"))
    return false;

  // Sadece rakamlardan oluşmalı
  for (const QChar &c : tc) {
    if (!c.isDigit())
      return false;
  }
  return true;
}

void SubMerchantDialog::validateInputs() {
  // Real-time validation can be added here
}

void SubMerchantDialog::onSaveClicked() {
  // Validate all fields
  QString name = nameEdit->text().trimmed();
  QString surname = surnameEdit->text().trimmed();
  QString email = emailEdit->text().trimmed();
  QString phone = phoneEdit->text().trimmed();
  QString identity = identityEdit->text().trimmed();
  QString iban = ibanEdit->text().trimmed();
  QString address = addressEdit->toPlainText().trimmed();

  // Check required fields
  if (name.isEmpty()) {
    ModernMessageBox::warning(this, "Eksik Bilgi", "Lütfen adınızı giriniz.");
    nameEdit->setFocus();
    return;
  }

  if (surname.isEmpty()) {
    ModernMessageBox::warning(this, "Eksik Bilgi",
                              "Lütfen soyadınızı giriniz.");
    surnameEdit->setFocus();
    return;
  }

  if (email.isEmpty()) {
    ModernMessageBox::warning(this, "Eksik Bilgi",
                              "Lütfen e-posta adresinizi giriniz.");
    emailEdit->setFocus();
    return;
  }

  if (!validateIdentityNumber(identity)) {
    ModernMessageBox::warning(this, "Geçersiz TC",
                              "TC Kimlik numarası 11 haneli olmalıdır ve 0 ile "
                              "başlamamalıdır.");
    identityEdit->setFocus();
    return;
  }

  if (!validateIban(iban)) {
    ModernMessageBox::warning(
        this, "Geçersiz IBAN",
        "IBAN 'TR' ile başlamalı ve 26 karakter olmalıdır.");
    ibanEdit->setFocus();
    return;
  }

  if (address.isEmpty()) {
    ModernMessageBox::warning(this, "Eksik Bilgi",
                              "Lütfen adresinizi giriniz.");
    addressEdit->setFocus();
    return;
  }

  // Save info
  info.name = name;
  info.surname = surname;
  info.email = email;
  info.phone = phone;
  info.identityNumber = identity;
  info.iban = iban.simplified().replace(" ", ""); // Remove spaces
  info.address = address;

  accept();
}

SubMerchantDialog::SubMerchantInfo
SubMerchantDialog::getSubMerchantInfo() const {
  return info;
}
