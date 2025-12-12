require('dotenv').config();
const express = require('express');
const mysql = require('mysql2');
const bodyParser = require('body-parser');
const cors = require('cors');
const bcrypt = require('bcrypt');
const nodemailer = require('nodemailer');
const axios = require('axios');
const cheerio = require('cheerio');
const fs = require('fs');
const path = require('path');

const logFile = path.join(__dirname, 'server_log.txt');

function logToFile(message) {
    const timestamp = new Date().toISOString();
    const logMessage = `[${timestamp}] ${message}\n`;
    fs.appendFileSync(logFile, logMessage);
    console.log(message);
}

const Iyzipay = require('iyzipay');

const iyzipay = new Iyzipay({
    apiKey: process.env.IYZICO_API_KEY,
    secretKey: process.env.IYZICO_SECRET_KEY,
    uri: process.env.IYZICO_BASE_URL
});

const app = express();
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: true }));
app.use(cors());

// --- MYSQL BAĞLANTISI ---
const db = mysql.createConnection({
    host: process.env.DB_HOST,
    user: process.env.DB_USER,
    password: process.env.DB_PASS,
    database: process.env.DB_NAME
});

db.connect((err) => {
    if (err) console.error('MySQL Bağlantı Hatası:', err);
    else {
        console.log('BAŞARILI: MySQL Veritabanına Bağlandı!');
        const alterTableQuery = "ALTER TABLE product_prices ADD COLUMN category VARCHAR(50) DEFAULT 'General' AFTER product_name";
        db.query(alterTableQuery, (errAlter) => {
            if (errAlter && errAlter.code !== 'ER_DUP_FIELDNAME') { }
            else if (!errAlter) { console.log("Tablo güncellendi: category sütunu eklendi."); }
        });

        const createDonationRequestsTableQuery = `
        CREATE TABLE IF NOT EXISTS donation_requests (
            id INT AUTO_INCREMENT PRIMARY KEY,
            username VARCHAR(255) NOT NULL,
            title VARCHAR(255) NOT NULL,
            category VARCHAR(50),
            price DECIMAL(10,2) NOT NULL,
            collected_amount DECIMAL(10,2) DEFAULT 0,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        )`;

        db.query(createDonationRequestsTableQuery, (err) => {
            if (err) console.error("Tablo oluşturma hatası (donation_requests):", err);
            else {
                const alterDonationQuery = "ALTER TABLE donation_requests ADD COLUMN collected_amount DECIMAL(10,2) DEFAULT 0";
                db.query(alterDonationQuery, (errAlter) => {
                    if (errAlter && errAlter.code !== 'ER_DUP_FIELDNAME') { }
                    else if (!errAlter) { console.log("Tablo güncellendi: collected_amount sütunu eklendi."); }
                });
            }
        });
    }
});

// --- TABLO OLUŞTURMA ---
const createPricesTableQuery = `
CREATE TABLE IF NOT EXISTS product_prices (
    id INT AUTO_INCREMENT PRIMARY KEY,
    product_name VARCHAR(255) NOT NULL,
    category VARCHAR(50) DEFAULT 'General',
    price VARCHAR(50) NOT NULL,
    source VARCHAR(50) DEFAULT 'DB',
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    UNIQUE KEY unique_product (product_name)
)`;

db.query(createPricesTableQuery, (err) => {
    if (err) console.error("Tablo oluşturma hatası (product_prices):", err);
    else console.log("Tablo kontrol edildi: product_prices");
});

const createHistoryTableQuery = `
CREATE TABLE IF NOT EXISTS test_history (
    id INT AUTO_INCREMENT PRIMARY KEY,
    username VARCHAR(255) NOT NULL,
    score INT NOT NULL,
    cpu VARCHAR(255),
    gpu VARCHAR(255),
    ram VARCHAR(255),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
)`;

db.query(createHistoryTableQuery, (err) => {
    if (err) console.error("Tablo olusturma hatasi (test_history):", err);
    else console.log("Tablo kontrol edildi: test_history");
});

// --- TRANSACTIONS TABLOSU OLUSTURMA ---
const createTransactionsTableQuery = `
CREATE TABLE IF NOT EXISTS transactions (
    id INT AUTO_INCREMENT PRIMARY KEY,
    sender_username VARCHAR(255) NOT NULL,
    receiver_username VARCHAR(255) NOT NULL,
    amount DECIMAL(10,2) NOT NULL,
    request_id INT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
)`;

db.query(createTransactionsTableQuery, (err) => {
    if (err) console.error("Tablo olusturma hatasi (transactions):", err);
    else console.log("Tablo kontrol edildi: transactions");
});

// --- KULLANICILAR TABLOSUNA GEREKLI SUTUNLARI EKLE ---
const alterColumns = [
    "ALTER TABLE kullanicilar ADD COLUMN balance DECIMAL(10,2) DEFAULT 0",
    "ALTER TABLE kullanicilar ADD COLUMN sub_merchant_key VARCHAR(255)",
    "ALTER TABLE kullanicilar ADD COLUMN iban VARCHAR(50)",
    "ALTER TABLE kullanicilar ADD COLUMN identity_number VARCHAR(20)",
    "ALTER TABLE kullanicilar ADD COLUMN phone VARCHAR(20)",
    "ALTER TABLE kullanicilar ADD COLUMN address TEXT",
    "ALTER TABLE kullanicilar ADD COLUMN cpu_score INT DEFAULT 0",
    "ALTER TABLE kullanicilar ADD COLUMN gpu_score INT DEFAULT 0",
    "ALTER TABLE kullanicilar ADD COLUMN ram_score INT DEFAULT 0"
];

alterColumns.forEach(sql => {
    db.query(sql, (err) => {
        if (err && err.code !== 'ER_DUP_FIELDNAME') {
            // Sutun zaten varsa hata verme
        }
    });
});

// --- NODEMAILER AYARLARI ---
const verificationCodes = {};

const transporter = nodemailer.createTransport({
    service: 'gmail',
    auth: {
        user: process.env.EMAIL_USER,
        pass: process.env.EMAIL_PASS
    }
});

// --- BAĞIŞ İSTEĞİ OLUŞTURMA API ---
app.post('/create-donation-request', (req, res) => {
    const { username, title, category, price } = req.body;
    if (!username || !title || !price) {
        return res.status(400).json({ success: false, message: "Eksik bilgi." });
    }

    const sql = "INSERT INTO donation_requests (username, title, category, price) VALUES (?, ?, ?, ?)";
    db.query(sql, [username, title, category, price], (err, result) => {
        if (err) {
            console.error("Bağış İsteği Kayıt Hatası:", err);
            res.status(500).json({ success: false, message: "Veritabanı hatası." });
        } else {
            console.log("Bağış isteği oluşturuldu:", title);
            res.status(200).json({ success: true, message: "Bağış isteği oluşturuldu." });
        }
    });
});

// --- BAĞIŞ İSTEKLERİNİ GETİRME API ---
app.get('/donation-requests', (req, res) => {
    const sql = "SELECT * FROM donation_requests ORDER BY created_at DESC";
    db.query(sql, (err, results) => {
        if (err) {
            console.error("Bağış İstekleri Getirme Hatası:", err);
            res.status(500).json({ success: false, message: "Veritabanı hatası." });
        } else {
            console.log("Bağış istekleri gönderiliyor:", results.length, "adet");
            res.status(200).json(results);
        }
    });
});

// --- KAYIT OLMA API ---
app.post('/register', (req, res) => {
    console.log("Kayıt İsteği Geldi:", req.body);
    const { username, password, email } = req.body;

    if (!username || !password || !email) {
        return res.status(400).json({ success: false, message: "Eksik veri." });
    }

    const saltRounds = 10;
    bcrypt.hash(password, saltRounds, (err, hash) => {
        if (err) {
            console.error("Hash Hatası:", err);
            return res.status(500).json({ success: false, message: "Sunucu hatası (Şifreleme)." });
        }

        const sql = "INSERT INTO kullanicilar (kullanici_adi, sifre_hash, email) VALUES (?, ?, ?)";
        db.query(sql, [username, hash, email], (err, result) => {
            if (err) {
                if (err.code === 'ER_DUP_ENTRY') {
                    res.status(409).json({ success: false, message: "Bu kullanıcı adı zaten alınmış." });
                } else {
                    console.error("SQL Hatası:", err);
                    res.status(500).json({ success: false, message: "Veritabanı hatası." });
                }
            } else {
                console.log("Kullanıcı oluşturuldu:", username);
                res.status(200).json({ success: true, message: "Kayıt Başarılı" });
            }
        });
    });
});

// --- GİRİŞ YAPMA API ---
app.post('/login', (req, res) => {
    console.log("Giriş İsteği Geldi:", req.body.username);
    const { username, password } = req.body;

    const sql = "SELECT * FROM kullanicilar WHERE kullanici_adi = ?";
    db.query(sql, [username], (err, results) => {
        if (err) {
            console.error("Giriş Hatası (SQL):", err);
            return res.status(500).json({ success: false, message: "Sunucu hatası" });
        }

        if (results.length === 0) {
            return res.status(401).json({ success: false, message: "Kullanıcı bulunamadı." });
        }

        const user = results[0];
        bcrypt.compare(password, user.sifre_hash, (err, isMatch) => {
            if (err) {
                console.error("Bcrypt Hatası:", err);
                return res.status(500).json({ success: false, message: "Sunucu hatası (Şifre Kontrolü)." });
            }

            if (isMatch) {
                console.log("Giriş Başarılı:", username);
                const { sifre_hash, ...userData } = user;
                res.status(200).json({ success: true, message: "Giriş Başarılı", user: userData });
            } else {
                console.log("Hatalı Şifre:", username);
                res.status(401).json({ success: false, message: "Hatalı şifre." });
            }
        });
    });
});

// --- HESAP SİLME API ---
app.post('/delete-account', (req, res) => {
    console.log("Hesap Silme İsteği:", req.body.username);
    const { username } = req.body;

    if (!username) return res.status(400).json({ success: false, message: "Kullanıcı adı eksik." });

    // 1. Test gecmisini sil
    const sqlDeleteHistory = "DELETE FROM test_history WHERE username = ?";
    db.query(sqlDeleteHistory, [username], (err1) => {
        if (err1) console.error("History silme hatasi:", err1);

        // 2. Bagis isteklerini sil
        const sqlDeleteRequests = "DELETE FROM donation_requests WHERE username = ?";
        db.query(sqlDeleteRequests, [username], (err2) => {
            if (err2) console.error("Requests silme hatasi:", err2);

            // 3. Islemleri (Transactions) sil - hem gonderen hem alan olarak
            const sqlDeleteTransactions = "DELETE FROM transactions WHERE sender_username = ? OR receiver_username = ?";
            db.query(sqlDeleteTransactions, [username, username], (err3) => {
                if (err3) console.error("Transactions silme hatasi:", err3);

                // 4. Kullaniciyi sil
                const sqlDeleteUser = "DELETE FROM kullanicilar WHERE kullanici_adi = ?";
                db.query(sqlDeleteUser, [username], (err, result) => {
                    if (err) {
                        console.error("Silme Hatası:", err);
                        res.status(500).json({ success: false, message: "Veritabanı hatası." });
                    } else if (result.affectedRows === 0) {
                        res.status(404).json({ success: false, message: "Kullanıcı bulunamadı." });
                    } else {
                        console.log("Kullanıcı ve tüm verileri silindi:", username);
                        res.status(200).json({ success: true, message: "Hesap ve tüm veriler başarıyla silindi." });
                    }
                });
            });
        });
    });
});

// --- SKOR KAYDETME API ---
app.post('/save-score', (req, res) => {
    console.log("Skor Kaydetme İsteği:", req.body.username);
    const { username, cpu, gpu, ram, score, cpu_score, gpu_score, ram_score } = req.body;

    const sqlUpdate = "UPDATE kullanicilar SET cpu = ?, gpu = ?, ram = ?, score = ?, cpu_score = ?, gpu_score = ?, ram_score = ? WHERE kullanici_adi = ?";
    db.query(sqlUpdate, [cpu, gpu, ram, score, cpu_score || 0, gpu_score || 0, ram_score || 0, username], (err, result) => {
        if (err) {
            console.error("Skor Kaydetme Hatası:", err);
            return res.status(500).json({ success: false, message: "Veritabanı hatası." });
        }
        const sqlInsertHistory = "INSERT INTO test_history (username, score, cpu, gpu, ram) VALUES (?, ?, ?, ?, ?)";
        db.query(sqlInsertHistory, [username, score, cpu, gpu, ram], (errHist, resultHist) => {
            if (errHist) {
                console.error("Geçmişe Ekleme Hatası:", errHist);
            } else {
                console.log("Geçmişe eklendi:", username);
            }
            res.status(200).json({ success: true, message: "Skor ve geçmiş kaydedildi." });
        });
    });
});

// --- SKOR GEÇMİŞİ API ---
app.get('/score-history', (req, res) => {
    const { username } = req.query;
    if (!username) return res.status(400).json({ success: false, message: "Kullanıcı adı gerekli." });

    const sql = "SELECT * FROM test_history WHERE username = ? ORDER BY created_at DESC LIMIT 20";
    db.query(sql, [username], (err, results) => {
        if (err) {
            console.error("Geçmiş Getirme Hatası:", err);
            res.status(500).json({ success: false, message: "Veritabanı hatası." });
        } else {
            res.status(200).json(results);
        }
    });
});

// --- TEST GEÇMİŞİNİ SİLME API ---
app.post('/delete-history', (req, res) => {
    console.log("Geçmiş Silme İsteği:", req.body.username);
    const { username } = req.body;

    if (!username) return res.status(400).json({ success: false, message: "Kullanıcı adı eksik." });

    const sqlDeleteHistory = "DELETE FROM test_history WHERE username = ?";
    db.query(sqlDeleteHistory, [username], (err, result) => {
        if (err) {
            console.error("Geçmiş Silme Hatası:", err);
            return res.status(500).json({ success: false, message: "Veritabanı hatası (History)." });
        }

        const sqlResetUser = "UPDATE kullanicilar SET score = 0, cpu_score = 0, gpu_score = 0, ram_score = 0, cpu = NULL, gpu = NULL, ram = NULL WHERE kullanici_adi = ?";
        db.query(sqlResetUser, [username], (errUser, resultUser) => {
            if (errUser) {
                console.error("Puan Sıfırlama Hatası:", errUser);
            } else {
                console.log("Kullanıcı puanı sıfırlandı:", username);
            }

            console.log("Geçmiş silindi:", username);
            res.status(200).json({ success: true, message: "Geçmiş ve puanlar başarıyla silindi." });
        });
    });
});

// --- ÜRÜN ARAMA API ---
app.get('/search-products', (req, res) => {
    const { query, category } = req.query;
    if (!query) {
        return res.status(400).json({ success: false, message: "Arama terimi gerekli." });
    }

    console.log(`Ürün aranıyor(Liste): ${query} [${category || 'Tümü'}]`);

    let sql = "SELECT * FROM product_prices WHERE product_name LIKE ?";
    let params = [`%${query}%`];

    if (category) {
        sql += " AND category = ?";
        params.push(category);
    }

    sql += " ORDER BY price ASC LIMIT 20";

    db.query(sql, params, (err, results) => {
        if (err) {
            console.error("Arama Hatası:", err);
            res.status(500).json({ success: false, message: "Veritabanı hatası." });
        } else {
            console.log(`Bulunan ürün sayısı: ${results.length}`);
            res.status(200).json({ success: true, results: results });
        }
    });
});

// --- FİYAT ÇEKME API ---
app.get('/get-price', async (req, res) => {
    const { productName, category } = req.query;
    if (!productName) {
        return res.status(400).json({ success: false, message: "Ürün adı gerekli." });
    }

    const cat = category || 'General';
    console.log(`Fiyat aranıyor: ${productName} (${cat})`);

    const sqlCheck = "SELECT * FROM product_prices WHERE product_name = ?";
    db.query(sqlCheck, [productName], async (err, results) => {
        if (err) {
            console.error("DB Fiyat Sorgu Hatası:", err);
            return res.status(500).json({ success: false, message: "Veritabanı hatası." });
        } else if (results.length > 0) {
            console.log("Fiyat DB'den bulundu:", results[0].price);
            return res.status(200).json({ success: true, price: results[0].price, source: "Veritabanı" });
        } else {
            console.log("DB'de bulunamadı.");
            return res.status(200).json({ success: false, message: "Fiyat bulunamadı.", price: "Bulunamadı", source: "Yok" });
        }
    });
});

// --- SUB-MERCHANT KAYIT API ---
app.post('/register-submerchant', (req, res) => {
    console.log("Sub-Merchant Kayit Istegi:", req.body.username);
    const { username, name, surname, email, iban, identityNumber, phone, address } = req.body;

    if (!username || !name || !surname || !email || !iban || !identityNumber) {
        return res.status(400).json({ success: false, message: "Eksik bilgi." });
    }

    // IBAN temizle
    let cleanIban = iban.replace(/\s/g, '').toUpperCase();
    if (!cleanIban.startsWith('TR')) {
        cleanIban = 'TR' + cleanIban;
    }
    if (cleanIban.length !== 26) {
        return res.status(400).json({ success: false, message: "IBAN 26 karakter olmali." });
    }

    const subMerchantExternalId = `SM_${username}_${Date.now()}`;
    const request = {
        locale: Iyzipay.LOCALE.TR,
        conversationId: String(Date.now()),
        subMerchantExternalId: subMerchantExternalId,
        subMerchantType: Iyzipay.SUB_MERCHANT_TYPE.PERSONAL,
        address: address || 'Istanbul, Turkey',
        contactName: name,
        contactSurname: surname,
        email: email,
        gsmNumber: phone || '+905350000000',
        name: name + ' ' + surname,
        iban: cleanIban,
        identityNumber: identityNumber,
        currency: Iyzipay.CURRENCY.TRY
    };

    iyzipay.subMerchant.create(request, function (err, result) {
        if (err) {
            console.error("Iyzico SubMerchant Hatasi:", err);
            return res.status(500).json({ success: false, message: "Iyzico hatasi." });
        }

        if (result.status === 'success') {
            const subMerchantKey = result.subMerchantKey;
            const sqlUpdate = `UPDATE kullanicilar SET sub_merchant_key = ?, iban = ?, identity_number = ?, phone = ?, address = ? WHERE kullanici_adi = ?`;
            db.query(sqlUpdate, [subMerchantKey, cleanIban, identityNumber, phone, address, username], (errDb) => {
                if (errDb) {
                    console.error("DB Guncelleme Hatasi:", errDb);
                    return res.status(500).json({ success: false, message: "Veritabani hatasi." });
                }
                console.log("Sub-merchant kaydedildi:", username, subMerchantKey);
                res.status(200).json({ success: true, message: "Sub-merchant olarak kaydedildiniz.", subMerchantKey: subMerchantKey });
            });
        } else {
            console.error("Iyzico API Hatasi:", result.errorMessage);
            res.status(400).json({ success: false, message: "Iyzico Hatasi: " + result.errorMessage });
        }
    });
});

// --- SUB-MERCHANT DURUM KONTROLU ---
app.get('/check-submerchant', (req, res) => {
    const { username } = req.query;
    if (!username) return res.status(400).json({ success: false, message: "Kullanici adi gerekli." });

    const sql = "SELECT sub_merchant_key FROM kullanicilar WHERE kullanici_adi = ?";
    db.query(sql, [username], (err, results) => {
        if (err) return res.status(500).json({ success: false, message: "Veritabani hatasi." });
        if (results.length === 0) return res.status(404).json({ success: false, message: "Kullanici bulunamadi." });

        const hasSubMerchant = results[0].sub_merchant_key ? true : false;
        res.status(200).json({ success: true, hasSubMerchant: hasSubMerchant });
    });
});

// --- SUB-MERCHANT SIFIRLAMA ---
app.post('/reset-submerchant', (req, res) => {
    const { username } = req.body;
    if (!username) return res.status(400).json({ success: false, message: "Kullanici adi eksik." });

    const sql = `UPDATE kullanicilar SET sub_merchant_key = NULL, iban = NULL, identity_number = NULL, phone = NULL, address = NULL WHERE kullanici_adi = ?`;
    db.query(sql, [username], (err, result) => {
        if (err) return res.status(500).json({ success: false, message: "Veritabani hatasi." });
        console.log("Sub-merchant kaydi sifirlandi:", username);
        res.status(200).json({ success: true, message: "Sub-merchant kaydi sifirlandi." });
    });
});

// --- IYZICO ODEME BASLATMA API ---
app.post('/payment/initialize', (req, res) => {
    console.log("Odeme Baslatma Istegi:", req.body);
    try {
        const { price, basketId, user, productName, receiverUsername, requestId } = req.body;

        if (!user) {
            return res.status(400).json({ success: false, message: "Kullanici bilgisi eksik." });
        }

        const senderUsername = user.name || 'UnknownSender';
        const receiver = receiverUsername || 'System';

        // Alicinin sub-merchant key'ini bul
        const sqlGetReceiver = "SELECT sub_merchant_key FROM kullanicilar WHERE kullanici_adi = ?";
        db.query(sqlGetReceiver, [receiver], (err, results) => {
            if (err) {
                console.error("Receiver sorgu hatasi:", err);
                return res.status(500).json({ success: false, message: "Veritabani hatasi." });
            }

            if (results.length === 0) {
                return res.status(404).json({ success: false, message: "Alici bulunamadi." });
            }

            const receiverSubMerchantKey = results[0].sub_merchant_key;

            if (!receiverSubMerchantKey) {
                return res.status(400).json({
                    success: false,
                    message: "Alici henuz sub-merchant olarak kayitli degil."
                });
            }

            // Iyzico komisyonu: %1.1 + 0.25 TL
            const priceNum = parseFloat(price) || 0;
            const priceStr = priceNum.toFixed(2);
            const iyzicoCommission = (priceNum * 0.011) + 0.25;
            const subMerchantPrice = (priceNum - iyzicoCommission).toFixed(2);

            console.log(`Odeme: ${priceStr} TL, Iyzico Kesintisi: ${iyzicoCommission.toFixed(2)} TL, Aliciya: ${subMerchantPrice} TL`);

            const reqId = requestId ? String(requestId) : '0';
            const conversationId = reqId.replace(/[^0-9]/g, '') || '0';
            const basketIdMeta = `DONATION:::${senderUsername}:::${receiver}:::${Date.now()}:::${reqId}`;

            const request = {
                locale: Iyzipay.LOCALE.TR,
                conversationId: conversationId,
                price: priceStr,
                paidPrice: priceStr,
                currency: Iyzipay.CURRENCY.TRY,
                basketId: basketIdMeta,
                paymentGroup: Iyzipay.PAYMENT_GROUP.PRODUCT,
                callbackUrl: 'http://localhost:3000/payment/callback',
                enabledInstallments: [2, 3, 6, 9],
                buyer: {
                    id: 'BUYER_' + senderUsername,
                    name: user.name || senderUsername,
                    surname: user.surname || user.name || senderUsername,
                    gsmNumber: '+905350000000',
                    email: user.email || 'email@email.com',
                    identityNumber: '74300864791',
                    lastLoginDate: new Date().toISOString().slice(0, 19).replace('T', ' '),
                    registrationDate: '2023-01-01 10:00:00',
                    registrationAddress: 'Istanbul, Turkey',
                    ip: req.ip || '127.0.0.1',
                    city: 'Istanbul',
                    country: 'Turkey',
                    zipCode: '34000'
                },
                shippingAddress: {
                    contactName: (user.name || senderUsername) + ' ' + (user.surname || user.name || senderUsername),
                    city: 'Istanbul',
                    country: 'Turkey',
                    address: 'Istanbul, Turkey',
                    zipCode: '34000'
                },
                billingAddress: {
                    contactName: (user.name || senderUsername) + ' ' + (user.surname || user.name || senderUsername),
                    city: 'Istanbul',
                    country: 'Turkey',
                    address: 'Istanbul, Turkey',
                    zipCode: '34000'
                },
                basketItems: [
                    {
                        id: 'BI_' + reqId,
                        name: productName || 'Bagis',
                        category1: 'Donation',
                        category2: 'Support',
                        itemType: Iyzipay.BASKET_ITEM_TYPE.PHYSICAL,
                        price: priceStr,
                        subMerchantKey: receiverSubMerchantKey,
                        subMerchantPrice: subMerchantPrice
                    }
                ]
            };

            iyzipay.checkoutFormInitialize.create(request, function (err, result) {
                if (err) {
                    console.error("Iyzico Hatasi:", err);
                    res.status(500).json({ success: false, message: "Odeme baslatilamadi." });
                } else {
                    if (result.status === 'success') {
                        res.status(200).json({ success: true, paymentPageUrl: result.paymentPageUrl, htmlContent: result.checkoutFormContent });
                    } else {
                        console.error("Iyzico API Hatasi:", result.errorMessage);
                        res.status(400).json({ success: false, message: "Iyzico Hatasi: " + result.errorMessage });
                    }
                }
            });
        });
    } catch (error) {
        console.error("Sunucu Ici Hata (/payment/initialize):", error);
        res.status(500).json({ success: false, message: "Sunucu hatasi: " + error.message });
    }
});

// --- ŞİFREMİ UNUTTUM API ---
app.post('/forgot-password', (req, res) => {
    const { email } = req.body;
    if (!email) return res.status(400).json({ success: false, message: "E-posta gerekli." });

    db.query("SELECT * FROM kullanicilar WHERE email = ?", [email], (err, results) => {
        if (err) return res.status(500).json({ success: false, message: "Veritabanı hatası." });
        if (results.length === 0) return res.status(404).json({ success: false, message: "Kullanıcı bulunamadı." });

        const code = Math.floor(100000 + Math.random() * 900000).toString();
        const expires = Date.now() + 5 * 60 * 1000;
        verificationCodes[email] = { code, expires };

        const mailOptions = {
            from: `Techbench App <${process.env.EMAIL_USER}>`,
            to: email,
            subject: 'Techbench Şifre Sıfırlama Kodu',
            text: `Doğrulama kodunuz: ${code}`
        };

        transporter.sendMail(mailOptions, (error, info) => {
            if (error) {
                console.log(`🔑 DOĞRULAMA KODU: ${code}`);
                return res.status(200).json({ success: true, message: "E-posta gönderilemedi (Simülasyon)." });
            }
            res.status(200).json({ success: true, message: "Doğrulama kodu gönderildi." });
        });
    });
});

// --- ŞİFRE SIFIRLAMA API ---
app.post('/reset-password', (req, res) => {
    const { email, code, newPassword } = req.body;
    if (!email || !code || !newPassword) return res.status(400).json({ success: false, message: "Eksik bilgi." });

    const record = verificationCodes[email];
    if (!record || Date.now() > record.expires || record.code !== code) {
        return res.status(400).json({ success: false, message: "Geçersiz veya süresi dolmuş kod." });
    }

    bcrypt.hash(newPassword, 10, (err, hash) => {
        if (err) return res.status(500).json({ success: false, message: "Şifreleme hatası." });
        db.query("UPDATE kullanicilar SET sifre_hash = ? WHERE email = ?", [hash, email], (err, result) => {
            if (err) return res.status(500).json({ success: false, message: "Veritabanı hatası." });
            delete verificationCodes[email];
            res.status(200).json({ success: true, message: "Şifre güncellendi." });
        });
    });
});

// --- IYZICO CALLBACK ---
app.post('/payment/callback', (req, res) => {
    logToFile("Callback received: " + JSON.stringify(req.body));
    const { token } = req.body;
    iyzipay.checkoutForm.retrieve({ locale: Iyzipay.LOCALE.TR, conversationId: '123456789', token: token }, function (err, result) {
        if (err) {
            logToFile("Iyzico Retrieve Error: " + JSON.stringify(err));
            return res.send("Odeme dogrulanamadi.");
        }

        logToFile("Iyzico Result Status: " + result.status + ", PaymentStatus: " + result.paymentStatus);

        if (result.status === 'success' && result.paymentStatus === 'SUCCESS') {
            logToFile("Odeme Basarili. BasketID: " + result.basketId);

            // BasketId formati: DONATION:::sender:::receiver:::timestamp:::requestId
            const basketId = result.basketId || "";
            if (basketId.startsWith('DONATION')) {
                const parts = basketId.split(':::');
                // parts[0] = 'DONATION', [1] = sender, [2] = receiver, [3] = timestamp, [4] = requestId
                const senderUsername = parts[1] || 'unknown';
                const receiverUsername = parts[2] || 'unknown';
                const requestId = parts[4] || '0';

                // Iyzico komisyonu: %1.1 + 0.25 TL
                const paidAmount = parseFloat(result.paidPrice) || 0;
                const iyzicoCommission = (paidAmount * 0.011) + 0.25;
                const netAmount = paidAmount - iyzicoCommission;

                logToFile(`Bagis alindi. Brut: ${paidAmount} TL, Komisyon: ${iyzicoCommission.toFixed(2)} TL, Net: ${netAmount.toFixed(2)} TL`);

                // 1. Alicinin bakiyesini guncelle
                db.query("UPDATE kullanicilar SET balance = COALESCE(balance, 0) + ? WHERE kullanici_adi = ?",
                    [netAmount, receiverUsername], (err1) => {
                        if (err1) logToFile("Balance guncelleme hatasi: " + err1.message);
                        else logToFile(`${receiverUsername} bakiyesi ${netAmount.toFixed(2)} TL artirildi`);
                    });

                // 2. Bagis isteginin collected_amount'unu guncelle
                if (requestId && requestId !== '0') {
                    db.query("UPDATE donation_requests SET collected_amount = COALESCE(collected_amount, 0) + ? WHERE id = ?",
                        [netAmount, requestId], (err2) => {
                            if (err2) logToFile("Collected amount guncelleme hatasi: " + err2.message);
                            else logToFile(`Request ${requestId} collected_amount ${netAmount.toFixed(2)} TL artirildi`);
                        });
                }

                // 3. Transaction kaydini olustur
                db.query("INSERT INTO transactions (sender_username, receiver_username, amount, request_id, created_at) VALUES (?, ?, ?, ?, NOW())",
                    [senderUsername, receiverUsername, netAmount, requestId], (err3) => {
                        if (err3) logToFile("Transaction kayit hatasi: " + err3.message);
                        else logToFile("Transaction kaydedildi");
                    });
            }
        } else {
            logToFile("Odeme basarisiz veya onaylanmadi: " + JSON.stringify(result));
        }
        res.send("Odeme Islemi Tamamlandi. Pencereyi kapatabilirsiniz.");
    });
});

// --- P2P TRANSFER API ---
app.post('/p2p-transfer', (req, res) => {
    const { sender_id, receiver_id, amount } = req.body;
    if (!sender_id || !receiver_id || !amount) return res.status(400).json({ success: false, message: "Eksik bilgi." });

    const sqlCheckUsers = "SELECT kullanici_adi FROM kullanicilar WHERE kullanici_adi IN (?, ?)";
    db.query(sqlCheckUsers, [sender_id, receiver_id], (err, results) => {
        if (err) return res.status(500).json({ success: false, message: "Veritabanı hatası." });

        const foundUsers = results.map(u => u.kullanici_adi);
        if (!foundUsers.includes(sender_id) || !foundUsers.includes(receiver_id)) {
            return res.status(404).json({ success: false, message: "Kullanıcı bulunamadı." });
        }

        const request = {
            locale: Iyzipay.LOCALE.TR,
            conversationId: String(Date.now()),
            price: amount.toString(),
            paidPrice: amount.toString(),
            currency: Iyzipay.CURRENCY.TRY,
            basketId: `BASKET${Date.now()}`,
            paymentGroup: Iyzipay.PAYMENT_GROUP.PRODUCT,
            callbackUrl: 'http://localhost:3000/payment/callback',
            enabledInstallments: [2, 3, 6, 9],
            buyer: {
                id: sender_id,
                name: sender_id,
                surname: 'User',
                gsmNumber: '+905350000000',
                email: 'email@email.com',
                identityNumber: '74300864791',
                lastLoginDate: '2015-10-05 12:43:35',
                registrationDate: '2013-04-21 15:12:09',
                registrationAddress: 'Istanbul',
                ip: '85.34.78.112',
                city: 'Istanbul',
                country: 'Turkey',
                zipCode: '34732'
            },
            shippingAddress: { contactName: sender_id, city: 'Istanbul', country: 'Turkey', address: 'Istanbul', zipCode: '34742' },
            billingAddress: { contactName: sender_id, city: 'Istanbul', country: 'Turkey', address: 'Istanbul', zipCode: '34742' },
            basketItems: [{ id: `TRANSFER${Date.now()}`, name: `Transfer to ${receiver_id}`, category1: 'Transfer', itemType: Iyzipay.BASKET_ITEM_TYPE.VIRTUAL, price: amount.toString() }]
        };

        iyzipay.checkoutFormInitialize.create(request, function (err, result) {
            if (err) res.status(500).json({ success: false, message: "Transfer başlatılamadı." });
            else if (result.status === 'success') res.status(200).json({ success: true, paymentPageUrl: result.paymentPageUrl, htmlContent: result.checkoutFormContent });
            else res.status(400).json({ success: false, message: "Iyzico Hatası: " + result.errorMessage });
        });
    });
});

// --- SUNUCUYU KAPATMA API ---
app.get('/shutdown', (req, res) => {
    res.send("Sunucu kapatılıyor...");
    setTimeout(() => process.exit(0), 1000);
});

// --- KULLANICI SIRALAMASI API ---
app.get('/ranking', (req, res) => {
    const { username } = req.query;
    if (!username) return res.status(400).json({ success: false, message: "Kullanıcı adı gerekli." });

    const sqlSimple = "SELECT kullanici_adi, score FROM kullanicilar WHERE score > 0 ORDER BY score DESC";

    db.query(sqlSimple, (err, results) => {
        if (err) {
            console.error("Sıralama Hatası:", err);
            return res.status(500).json({ success: false, message: "Veritabanı hatası." });
        }

        let rank = -1;
        for (let i = 0; i < results.length; i++) {
            if (results[i].kullanici_adi === username) {
                rank = i + 1;
                break;
            }
        }

        if (rank !== -1) {
            res.status(200).json({ success: true, ranking: rank });
        } else {
            res.status(200).json({ success: false, message: "Sıralamaya henüz girmediniz (Puan: 0)." });
        }
    });
});

// --- RAKİPLERİ GETİRME API ---
app.get('/rivals', (req, res) => {
    const sql = "SELECT kullanici_adi, score, cpu, gpu, ram FROM kullanicilar WHERE score > 0 AND kullanici_adi != 'admin' ORDER BY score DESC LIMIT 20";
    db.query(sql, (err, results) => {
        if (err) {
            console.error("Rakipleri Getirme Hatası:", err);
            res.status(500).json({ success: false, message: "Veritabanı hatası." });
        } else {
            res.status(200).json(results);
        }
    });
});

// --- LİDERLİK TABLOSU API ---
app.get('/leaderboard', (req, res) => {
    const sql = "SELECT kullanici_adi, score, cpu, gpu, ram, cpu_score, gpu_score, ram_score FROM kullanicilar WHERE score > 0 ORDER BY score DESC LIMIT 50";
    db.query(sql, (err, results) => {
        if (err) {
            console.error("Liderlik Tablosu Hatası:", err);
            res.status(500).json({ success: false, message: "Veritabanı hatası." });
        } else {
            res.status(200).json(results);
        }
    });
});

// --- DESTEKÇILER API (Top Supporters) ---
app.get('/top-supporters', (req, res) => {
    const { username } = req.query;
    if (!username) return res.status(400).json({ success: false, message: "Kullanıcı adı gerekli." });

    // amount'ları topla
    const sql = `
        SELECT sender_username, 
               SUM(amount) as total_amount, 
               COUNT(*) as donation_count
        FROM transactions
        WHERE receiver_username = ?
        GROUP BY sender_username
        ORDER BY SUM(amount) DESC
        LIMIT 10
    `;

    db.query(sql, [username], (err, results) => {
        if (err) {
            console.error("Destekçiler Getirme Hatası:", err);
            res.status(500).json({ success: false, message: "Veritabanı hatası: " + err.message });
        } else {
            // MySQL2 bazen DECIMAL'ı string döndürür, düzeltelim
            const formattedResults = results.map(row => ({
                sender_username: row.sender_username,
                total_amount: parseFloat(row.total_amount) || 0,
                donation_count: parseInt(row.donation_count) || 0
            }));

            console.log("Supporters formatted:", JSON.stringify(formattedResults));
            res.status(200).json({ success: true, supporters: formattedResults });
        }
    });
});

app.listen(3000, () => {
    console.log('Sunucu 3000 portunda çalışıyor...');
});
