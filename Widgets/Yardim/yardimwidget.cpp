#include "yardimwidget.h"
#include "ui_yardimwidget.h"
#include <QTreeWidgetItem> // TreeWidget öğeleri için gerekli
#include <QLabel>

YardimWidget::YardimWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::YardimWidget)
{
    ui->setupUi(this);

    // --- SORU VE CEVAPLARI BURAYA YAZ ---

    soruEkle("TechBench nedir ve ne işe yarar?",
             "TechBench, bilgisayarınızın donanım performansını (CPU, GPU, RAM) test eden ve \n"
             "diğer sistemlerle karşılaştırmanızı sağlayan gelişmiş bir araçtır.");

    soruEkle("Giriş yapmak önemli mi?",
             "Evet. Giriş yaparak test sonuçlarınızı buluta kaydedebilir, \n"
             "geçmiş testlerinizi görüntüleyebilir ve global sıralamada yer alabilirsiniz.");

    soruEkle("Benchmark sonuçlarım neden düşük?",
             "Arka planda çalışan uygulamalar, güncel olmayan sürücüler veya \n"
             "yetersiz soğutma performansı sonuçları etkileyebilir.");

    soruEkle("Sistemim uyumlu mu?",
             "TechBench, Windows 10 ve üzeri işletim sistemlerinde çalışacak şekilde tasarlanmıştır.\n"
             "OpenGL 3.0+ destekli bir ekran kartı önerilir.");

    soruEkle("Puanlar ne anlama geliyor?",
             "Puanlar donanımınızın ham işlem gücünü temsil eder. \n"
             "Daha yüksek puan, daha yüksek performans demektir.");

    soruEkle("Bağış Yap ne anlama geliyor?",
             "Bu özellik, kullanıcılar arasında doğrudan finansal etkileşimi sağlayan bir P2P (Eşten Eşe) \n"
             "mikro-ödeme mekanizmasıdır. iyzico güvenli ödeme altyapısı kullanılarak, \n"
             "kullanıcıların beğendikleri sistem sahiplerine veya topluluğa katkı sağlayanlara \n"
             "güvenli bir şekilde maddi destek (bağış) sağlamasına olanak tanınır.");
}

YardimWidget::~YardimWidget()
{
    delete ui;
}

// --- BU FONKSİYON SAYESİNDE ALT ALTA YAZI AÇILIR ---
void YardimWidget::soruEkle(QString soru, QString cevap)
{
    // 1. Ana Maddeyi (Soruyu) Oluştur
    QTreeWidgetItem *soruItem = new QTreeWidgetItem(ui->sss_listesi);
    soruItem->setText(0, " " + soru); // Başına soru işareti ekledim

    // 2. Alt Maddeyi (Cevabı) Oluştur
    QTreeWidgetItem *cevapItem = new QTreeWidgetItem(soruItem);

    // Cevap metni çok uzunsa düzgün görünmesi için Label kullanıyoruz
    QLabel *cevapLabel = new QLabel(cevap);
    cevapLabel->setWordWrap(true); // Yazı sığmazsa alt satıra geçsin
    cevapLabel->setStyleSheet("color: #bdc3c7; font-style: italic; padding: 10px;"); // Cevap stili

    // 3. Cevabı listeye widget olarak ekle
    ui->sss_listesi->setItemWidget(cevapItem, 0, cevapLabel);
}
