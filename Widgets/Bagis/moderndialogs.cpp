
#include "moderndialogs.h"
#include <QHBoxLayout>
#include <QPainter>
#include <QStyleOption>

// ==========================================
// ModernInputDialog Implementation
// ==========================================

ModernInputDialog::ModernInputDialog(QWidget *parent) : QDialog(parent) {
  setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
  setAttribute(Qt::WA_TranslucentBackground);
}

int ModernInputDialog::getInt(QWidget *parent, const QString &title,
                              const QString &label, int value, int min, int max,
                              int step, bool *ok) {
  ModernInputDialog dialog(parent);
  dialog.setupUi(title, label, value, min, max, step);

  int ret = dialog.exec();
  if (ok)
    *ok = (ret == QDialog::Accepted);

  if (ret == QDialog::Accepted) {
    return dialog.spinBox->value();
  }
  return value;
}

void ModernInputDialog::setupUi(const QString &title, const QString &labelText,
                                int value, int min, int max, int step) {
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(0, 0, 0, 0);

  QFrame *frame = new QFrame(this);
  frame->setObjectName("mainFrame");
  frame->setStyleSheet(
      "#mainFrame { background-color: #2b2e38; border: 1px solid #3c404d; "
      "border-radius: 10px; }");
  mainLayout->addWidget(frame);

  QVBoxLayout *layout = new QVBoxLayout(frame);
  layout->setSpacing(15);
  layout->setContentsMargins(20, 20, 20, 20);

  // Title
  QLabel *titleLabel = new QLabel(title);
  titleLabel->setStyleSheet(
      "color: white; font-size: 16px; font-weight: bold;");
  layout->addWidget(titleLabel);

  // Label
  QLabel *msgLabel = new QLabel(labelText);
  msgLabel->setStyleSheet("color: #bec3cd; font-size: 13px;");
  msgLabel->setWordWrap(true);
  layout->addWidget(msgLabel);

  // SpinBox
  spinBox = new QSpinBox();
  spinBox->setRange(min, max);
  spinBox->setValue(value);
  spinBox->setSingleStep(step);
  spinBox->setStyleSheet(
      "QSpinBox { background-color: #1e1e1e; color: white; border: 1px solid "
      "#3c404d; border-radius: 5px; padding: 5px; font-size: 14px; }"
      "QSpinBox::up-button, QSpinBox::down-button { width: 0px; }"); // Hide
                                                                     // arrows
                                                                     // for
                                                                     // cleaner
                                                                     // look
  layout->addWidget(spinBox);

  // Buttons
  QHBoxLayout *btnLayout = new QHBoxLayout();
  btnLayout->addStretch();

  cancelButton = new QPushButton("İptal");
  cancelButton->setCursor(Qt::PointingHandCursor);
  cancelButton->setStyleSheet(
      "QPushButton { background-color: transparent; color: #bec3cd; border: "
      "none; font-weight: bold; padding: 8px 15px; }"
      "QPushButton:hover { color: white; }");
  connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
  btnLayout->addWidget(cancelButton);

  okButton = new QPushButton("Tamam");
  okButton->setCursor(Qt::PointingHandCursor);
  okButton->setStyleSheet(
      "QPushButton { background-color: #4facfe; color: white; border-radius: "
      "5px; font-weight: bold; padding: 8px 20px; border: none; }"
      "QPushButton:hover { background-color: #00f2fe; }");
  connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
  btnLayout->addWidget(okButton);

  layout->addLayout(btnLayout);
}

// ==========================================
// ModernMessageBox Implementation
// ==========================================

ModernMessageBox::ModernMessageBox(QWidget *parent) : QDialog(parent) {
  setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
  setAttribute(Qt::WA_TranslucentBackground);
}

void ModernMessageBox::information(QWidget *parent, const QString &title,
                                   const QString &text) {
  ModernMessageBox dialog(parent);
  dialog.setupUi(title, text, false, false);
  dialog.exec();
}

void ModernMessageBox::critical(QWidget *parent, const QString &title,
                                const QString &text) {
  ModernMessageBox dialog(parent);
  dialog.setupUi(title, text, true, false);
  dialog.exec();
}

bool ModernMessageBox::question(QWidget *parent, const QString &title,
                                const QString &text) {
  ModernMessageBox dialog(parent);
  dialog.setupUi(title, text, false, true);
  return dialog.exec() == QDialog::Accepted;
}

void ModernMessageBox::setupUi(const QString &title, const QString &text,
                               bool isError, bool isQuestion) {
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(0, 0, 0, 0);

  QFrame *frame = new QFrame(this);
  frame->setObjectName("mainFrame");
  frame->setStyleSheet(
      "#mainFrame { background-color: #2b2e38; border: 1px solid #3c404d; "
      "border-radius: 10px; }");
  mainLayout->addWidget(frame);

  QVBoxLayout *layout = new QVBoxLayout(frame);
  layout->setSpacing(15);
  layout->setContentsMargins(20, 20, 20, 20);

  // Title
  QLabel *titleLabel = new QLabel(title);
  QString titleColor =
      isError ? "#ff4d4d" : (isQuestion ? "#f1c40f" : "#4facfe");
  titleLabel->setStyleSheet(
      QString("color: %1; font-size: 16px; font-weight: bold;")
          .arg(titleColor));
  layout->addWidget(titleLabel);

  // Text
  QLabel *msgLabel = new QLabel(text);
  msgLabel->setStyleSheet("color: #bec3cd; font-size: 13px;");
  msgLabel->setWordWrap(true);
  layout->addWidget(msgLabel);

  // Buttons
  QHBoxLayout *btnLayout = new QHBoxLayout();
  btnLayout->addStretch();

  if (isQuestion) {
    QPushButton *cancelBtn = new QPushButton("Hayır");
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setStyleSheet(
        "QPushButton { background-color: transparent; color: #bec3cd; border: "
        "none; font-weight: bold; padding: 8px 15px; }"
        "QPushButton:hover { color: white; }");
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    btnLayout->addWidget(cancelBtn);

    QPushButton *okBtn = new QPushButton("Evet");
    okBtn->setCursor(Qt::PointingHandCursor);
    okBtn->setStyleSheet(
        QString(
            "QPushButton { background-color: %1; color: white; border-radius: "
            "5px; font-weight: bold; padding: 8px 20px; border: none; }"
            "QPushButton:hover { opacity: 0.8; }")
            .arg(titleColor));
    connect(okBtn, &QPushButton::clicked, this, &QDialog::accept);
    btnLayout->addWidget(okBtn);
  } else {
    QPushButton *okBtn = new QPushButton("Tamam");
    okBtn->setCursor(Qt::PointingHandCursor);
    okBtn->setStyleSheet(
        QString(
            "QPushButton { background-color: %1; color: white; border-radius: "
            "5px; font-weight: bold; padding: 8px 20px; border: none; }"
            "QPushButton:hover { opacity: 0.8; }")
            .arg(titleColor));
    connect(okBtn, &QPushButton::clicked, this, &QDialog::accept);
    btnLayout->addWidget(okBtn);
  }

  layout->addLayout(btnLayout);
}
