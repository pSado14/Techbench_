#ifndef YARDIMWIDGET_H
#define YARDIMWIDGET_H

#include <QWidget>

class QVBoxLayout;
class QScrollArea;
class QPushButton;
class QLabel;
class QPropertyAnimation;
class QFrame;

namespace Ui {
class YardimWidget;
}

class FAQItem : public QWidget {
  Q_OBJECT
public:
  explicit FAQItem(const QString &question, const QString &answer,
                   QWidget *parent = nullptr);

private slots:
  void onToggle();

private:
  QPushButton *m_toggleBtn;
  QFrame *m_answerFrame;
  QLabel *m_answerLabel;
  QPropertyAnimation *m_animation;
  bool m_isExpanded;
};

class YardimWidget : public QWidget {
  Q_OBJECT

public:
  explicit YardimWidget(QWidget *parent = nullptr);
  ~YardimWidget();

private:
  Ui::YardimWidget *ui;

  // Yeni yapı için scroll area ve layout
  QScrollArea *m_scrollArea;
  QWidget *m_scrollContent;
  QVBoxLayout *m_scrollLayout;

  // Yardımcı fonksiyonlar
  void setupModernUI();
  void kategoriEkle(const QString &baslik);
  void soruEkle(const QString &soru, const QString &cevap);
};

#endif // YARDIMWIDGET_H
