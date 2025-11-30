const express = require('express');
const mysql = require('mysql2');
const bodyParser = require('body-parser');
const cors = require('cors');
const bcrypt = require('bcrypt');
const nodemailer = require('nodemailer');

const app = express();
app.use(bodyParser.json());
app.use(cors());

// --- MYSQL BAĞLANTISI ---
const db = mysql.createConnection({
    host: 'localhost',
    user: 'root',
    password: 'Melekirem14.', // Senin şifren
    database: 'benchmark_db'  // Veritabanı adın
});

db.connect((err) => {
    if (err) console.error('MySQL Bağlantı Hatası:', err);
    else console.log('BAŞARILI: MySQL Veritabanına Bağlandı!');
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
        if (err) return res.status(500).json({ success: false, message: "Sunucu hatası" });
        if (results.length === 0) return res.status(401).json({ success: false, message: "Kullanıcı adı veya şifre hatalı" });

        const user = results[0];
        bcrypt.compare(password, user.sifre_hash, (err, isMatch) => {
            if (err) return res.status(500).json({ success: false, message: "Hata oluştu" });
            if (isMatch) {
                console.log("Giriş Başarılı:", username);
                res.status(200).json({ success: true, message: "Giriş Başarılı" });
            } else {
                console.log("Giriş Başarısız: Yanlış şifre.");
                res.status(401).json({ success: false, message: "Kullanıcı adı veya şifre hatalı" });
            }
        });
    });
});

// --- HESAP SİLME API ---
app.post('/delete-account', (req, res) => {
    console.log("Hesap Silme İsteği:", req.body.username);
    const { username } = req.body;

    if (!username) return res.status(400).json({ success: false, message: "Kullanıcı adı eksik." });

    const sql = "DELETE FROM kullanicilar WHERE kullanici_adi = ?";
    db.query(sql, [username], (err, result) => {
        if (err) {
            console.error("Silme Hatası:", err);
            res.status(500).json({ success: false, message: "Veritabanı hatası." });
        } else if (result.affectedRows === 0) {
            res.status(404).json({ success: false, message: "Kullanıcı bulunamadı." });
        } else {
            console.log("Kullanıcı silindi:", username);
            res.status(200).json({ success: true, message: "Hesap başarıyla silindi." });
        }
    });
});

// --- SKOR KAYDETME API ---
app.post('/save-score', (req, res) => {
    console.log("Skor Kaydetme İsteği:", req.body.username);
    const { username, cpu, gpu, ram, score } = req.body;

    const sql = "UPDATE kullanicilar SET cpu = ?, gpu = ?, ram = ?, score = ? WHERE kullanici_adi = ?";
    db.query(sql, [cpu, gpu, ram, score, username], (err, result) => {
        if (err) {
            console.error("Skor Kaydetme Hatası:", err);
            res.status(500).json({ success: false, message: "Veritabanı hatası." });
        } else {
            console.log("Skor güncellendi:", username);
            res.status(200).json({ success: true, message: "Skor başarıyla kaydedildi." });
        }
    });
});

// --- RAKİPLERİ GETİRME API ---
app.get('/rivals', (req, res) => {
    const sql = "SELECT kullanici_adi AS username, cpu, gpu, ram, score FROM kullanicilar ORDER BY score DESC LIMIT 50";
    db.query(sql, (err, results) => {
        if (err) {
            console.error("Rakipleri Getirme Hatası:", err);
            res.status(500).json({ success: false, message: "Veritabanı hatası." });
        } else {
            console.log("Rakipler gönderiliyor:", results.length, "kişi");
            res.status(200).json(results);
        }
    });
});

// --- NODEMAILER AYARLARI ---
const verificationCodes = {};

const transporter = nodemailer.createTransport({
    service: 'gmail',
    auth: {
        user: 'sadettinboylan80@gmail.com',
        pass: 'zaei jepx rppc mwuu' // Kullanıcının girdiği şifre (App Password değilse hata verebilir, simülasyon devrede)
    }
});

// --- ŞİFREMİ UNUTTUM API (KOD GÖNDERME) ---
app.post('/forgot-password', (req, res) => {
    const { email } = req.body;
    console.log("Şifre Sıfırlama İsteği:", email);

    if (!email) return res.status(400).json({ success: false, message: "E-posta gerekli." });

    db.query("SELECT * FROM kullanicilar WHERE email = ?", [email], (err, results) => {
        if (err) return res.status(500).json({ success: false, message: "Veritabanı hatası." });
        if (results.length === 0) return res.status(404).json({ success: false, message: "Bu e-posta ile kayıtlı kullanıcı bulunamadı." });

        const code = Math.floor(100000 + Math.random() * 900000).toString();
        const expires = Date.now() + 5 * 60 * 1000;

        verificationCodes[email] = { code, expires };

        const mailOptions = {
            from: 'Techbench App <sadettinboylan80@gmail.com>',
            to: email,
            subject: 'Techbench Şifre Sıfırlama Kodu',
            text: `Şifrenizi sıfırlamak için doğrulama kodunuz: ${code}\nBu kod 5 dakika süreyle geçerlidir.`
        };

        transporter.sendMail(mailOptions, (error, info) => {
            if (error) {
                console.error("E-posta Gönderme Hatası (Test Modu Aktif):", error.message);
                console.log("---------------------------------------------------");
                console.log("⚠️ SİMÜLASYON MODU: E-posta gönderilemediği için kod buraya yazılıyor.");
                console.log(`🔑 DOĞRULAMA KODU: ${code}`);
                console.log("---------------------------------------------------");
                // Hata olsa bile başarılı dönüyoruz ki arayüz test edilebilsin
                return res.status(200).json({ success: true, message: "E-posta gönderilemedi (Simülasyon). Kod backend konsolunda." });
            }
            console.log("Kod gönderildi:", email, code);
            res.status(200).json({ success: true, message: "Doğrulama kodu gönderildi." });
        });
    });
});

// --- ŞİFRE SIFIRLAMA API (KOD DOĞRULAMA VE YENİ ŞİFRE) ---
app.post('/reset-password', (req, res) => {
    const { email, code, newPassword } = req.body;
    console.log("Şifre Yenileme İsteği:", email);

    if (!email || !code || !newPassword) {
        return res.status(400).json({ success: false, message: "Eksik bilgi." });
    }

    const record = verificationCodes[email];

    if (!record) {
        return res.status(400).json({ success: false, message: "Kod bulunamadı veya süresi doldu." });
    }

    if (Date.now() > record.expires) {
        delete verificationCodes[email];
        return res.status(400).json({ success: false, message: "Kodun süresi dolmuş." });
    }

    if (record.code !== code) {
        return res.status(400).json({ success: false, message: "Geçersiz kod." });
    }

    const saltRounds = 10;
    bcrypt.hash(newPassword, saltRounds, (err, hash) => {
        if (err) return res.status(500).json({ success: false, message: "Şifreleme hatası." });

        db.query("UPDATE kullanicilar SET sifre_hash = ? WHERE email = ?", [hash, email], (err, result) => {
            if (err) return res.status(500).json({ success: false, message: "Veritabanı hatası." });

            delete verificationCodes[email];
            console.log("Şifre başarıyla güncellendi:", email);
            res.status(200).json({ success: true, message: "Şifreniz başarıyla güncellendi." });
        });
    });
});

app.listen(3000, () => {
    console.log('Sunucu 3000 portunda çalışıyor...');
});