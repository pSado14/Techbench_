#include "bagiswidget.h"
#include "ui_bagiswidget.h"

BagisWidget::BagisWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::BagisWidget) {
  ui->setupUi(this);

  // Modern Blue Gradient for "Bağışı Tamamla"
  ui->bagis_yap_butonu_sayfa_2->setStyleSheet(
      "QPushButton { "
      "   background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, "
      "y2:0, stop:0 #4facfe, stop:1 #00f2fe); "
      "   border-radius: 8px; "
      "   color: white; "
      "   padding: 10px 20px; "
      "   font-weight: bold; "
      "   font-size: 14px; "
      "   border: none; "
      "}"
      "QPushButton:hover { background-color: qlineargradient(spread:pad, "
      "x1:0, y1:0, x2:1, y2:0, stop:0 #00f2fe, stop:1 #4facfe); }");

  // "Bağışı Tamamla" (bagis_yap_butonu_sayfa_2) butonu için
  // ödeme ağ geçidi (payment gateway) mantığı buraya bağlanacak.
}

BagisWidget::~BagisWidget() { delete ui; }
