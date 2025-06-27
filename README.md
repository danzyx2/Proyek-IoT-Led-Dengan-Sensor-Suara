# 💡 Proyek Cahaya Pintar: LED & Suara 🔊

Tempat di mana cahaya bertemu suara, dan ide-ide IoT berinteraksi dengan dunia nyata.
Tidak ada yang terlalu rumit di sini... atau mungkin justru terlalu menyenangkan?

---

🔗 **Tujuan Projek Ini**

* **⚡ Interaksi Menyenangkan**: Menciptakan cara baru dan seru untuk berinteraksi dengan pencahayaan di sekitar kita.
* **💡 Eksplorasi IoT**: Menjelajahi potensi ESP8266 sebagai otak di balik perangkat cerdas yang responsif.
* **🎨 Kreativitas Visual**: Bermain dengan berbagai efek cahaya yang memukau dan personalisasi.
* **🧪 Eksperimen Interaktif**: Menggabungkan input fisik (suara) dengan output visual (LED).

---

📦 **Isi Kotak Proyek Ini**

* **🧠 Otak Utama**: Sebuah NodeMCU ESP8266 yang berperan sebagai server dan pengontrol.
* **🌈 Kanvas Cahaya**: Kode untuk menggerakkan LED strip WS2812B dengan beragam animasi.
* **👂 Telinga Digital**: Implementasi deteksi suara untuk kontrol tepukan tangan.
* **📱 Remote Control Saku**: Antarmuka web responsif yang bisa diakses dari perangkat apapun.
* **📚 Dokumentasi Jelas**: Panduan langkah demi langkah untuk menyatukan semuanya.

---

✨ **Fitur-Fitur Kerennya**

* **Wi-Fi Sendiri!** 🌐
    NodeMCU Anda akan memancarkan hotspot Wi-Fi (`SSID: "LED Kontroler"`, `Password: "BUMIKITA"`). Cukup sambungkan ponsel atau laptop Anda ke jaringan ini, dan siap untuk mengontrol!

* **Panel Kontrol Web Modern** 💻
    Akses `http://192.168.4.1` dari browser Anda untuk mendapatkan antarmuka kontrol yang intuitif dan *real-time*:
    * Lihat status dan kecerahan LED secara langsung.
    * Intip kondisi memori NodeMCU Anda.
    * Pilih dari segudang mode efek LED dinamis.
    * Geser slider untuk mengatur kecerahan sesuai mood.
    * Atur detail kecil dari setiap mode (warna favorit, kecepatan pelangi, dll.).
    * Tinjau "Log Suara" untuk melihat kapan tepukan tangan terdeteksi.
    * Bersihkan log kapan saja.

* **Kontrol Dengan Tepukan Tangan!** 👏
    Sensor suara digital memungkinkan Anda berinteraksi tanpa menyentuh:
    * Satu tepukan saat LED mati? **LED akan menyala** ke mode terakhir Anda.
    * Satu tepukan saat LED menyala? **LED akan mati**.
    * Setiap interaksi tepukan tangan akan dicatat rapi di log web.

* **Kumpulan Efek LED FastLED yang Memukau** ✨
    Library FastLED yang powerful menghadirkan berbagai pesta cahaya di ujung jari Anda:
    * `MODE_OFF`: Total kegelapan.
    * `MODE_RAINBOW`: Warna-warni pelangi yang bergerak anggun.
    * `MODE_SOLID_COLOR`: Satu warna solid, mood Anda, pilihan Anda.
    * `MODE_FADE`: Transisi warna yang halus, seperti mimpi.
    * `MODE_TWO_COLOR`: Dua warna bergantian, menciptakan ritme visual.
    * `MODE_COMET_TRAIL`: Efek komet yang melesat dengan ekor bercahaya.
    * `MODE_METEOR_RAIN`: Hujan meteor mini di LED strip Anda.
    * `MODE_FIRE`: Simulasi api yang hidup dan berkedip.
    * `MODE_SPARKLE`: Percikan cahaya acak yang memberikan kesan magis.

---

🛠️ **Apa Yang Kamu Butuhkan (Hardware)**

* **NodeMCU ESP8266** (disarankan ESP-12E Module)
* **LED Strip WS2812B / NeoPixel** (sesuaikan `NUM_LEDS` dan `LED_DATA_PIN` di kode)
* **Modul Sensor Suara Digital** (dengan output DO)
* **Kabel Jumper**
* **Catu Daya 5V Eksternal** (Sangat direkomendasikan untuk lebih dari 8-10 LED!)

---

### 🔌 Hubungan Pin-Pin (Wiring Diagram)

Penting untuk menghubungkan setiap komponen dengan benar agar proyek ini dapat berfungsi. Perhatikan diagram koneksi dasar di bawah ini:

* **NodeMCU ESP8266**
    * **Ke LED Strip WS2812B / NeoPixel:**
        * **Pin D2 (GPIO 4)** pada NodeMCU $\rightarrow$ Pin **Data In** pada LED Strip.
        * Pin **GND** pada NodeMCU $\rightarrow$ Pin **GND** pada LED Strip.
        * Pin **5V (VIN)** pada NodeMCU $\rightarrow$ Pin **5V** pada LED Strip (Lihat **PERINGATAN PENTING UNTUK POWER LED** di bawah!).
    * **Ke Modul Sensor Suara Digital:**
        * Pin **D5 (GPIO 14)** pada NodeMCU $\rightarrow$ Pin **DO (Digital Out)** pada Modul Sensor Suara.
        * Pin **GND** pada NodeMCU $\rightarrow$ Pin **GND** pada Modul Sensor Suara.
        * Pin **3V3** pada NodeMCU $\rightarrow$ Pin **VCC** pada Modul Sensor Suara (Beberapa sensor suara bisa menggunakan 5V, periksa spesifikasi modul Anda. Namun, 3V3 lebih aman untuk pin ESP8266).

**⚠️ PERINGATAN PENTING UNTUK POWER LED:**
Jika LED strip Anda memiliki **lebih dari 8-10 LED**, **sangat penting** untuk menggunakan **catu daya 5V eksternal terpisah** yang cukup kuat untuk LED strip. Jangan pernah memberi daya banyak LED langsung dari pin NodeMCU, karena dapat merusak board! Pastikan **semua GND terhubung bersama** (NodeMCU, LED strip, dan Power Supply LED eksternal) untuk menciptakan ground bersama.

* **Contoh Koneksi Power Supply Eksternal untuk LED:**
    * Pin **5V** dari Power Supply Eksternal $\rightarrow$ Pin **5V** pada LED Strip.
    * Pin **GND** dari Power Supply Eksternal $\rightarrow$ Pin **GND** pada LED Strip **DAN** Pin **GND** pada NodeMCU.
    * **JANGAN** hubungkan 5V dari Power Supply LED eksternal ke pin 5V (VIN) NodeMCU jika NodeMCU sudah diberi daya dari USB, kecuali Anda yakin dengan konfigurasi power board Anda. Cukup sambungkan GND dari semua komponen.

---

🚀 **Langkah-Langkah Awal (Mulai Ngoding)**

### 1. Siapkan Dapur Coding (Arduino IDE)

1.  **Unduh Arduino IDE**: Pastikan Anda memiliki versi terbaru dari [situs resmi Arduino](https://www.arduino.cc/en/software).
2.  **Tambahkan Board ESP8266**:
    * Buka `File > Preferences`.
    * Tambahkan URL ini di "Additional Board Manager URLs":
        `http://arduino.esp8266.com/stable/package_esp8266com_index.json`
    * `OK`.
    * `Tools > Board > Board Manager...`. Cari "esp8266" dan instal paket "esp8266 by ESP8266 Community".
3.  **Pasang Library Esensial**:
    * `Sketch > Include Library > Manage Libraries...`.
    * Cari dan instal **FastLED**.
    * `ESP8266WebServer`, `ESP8266WiFi`, dan `FS (SPIFFS)` sudah jadi satu paket dengan core ESP8266.
4.  **Pilih Target & Port**:
    * `Tools > Board`: Pilih **"NodeMCU 1.0 (ESP-12E Module)"**.
    * `Tools > Port`: Pilih port serial tempat NodeMCU Anda terhubung.

### 2. Kirim Kode ke NodeMCU

1.  **Salin Kode**: Buka file `.ino` dari repositori ini, salin seluruh kodenya, dan tempelkan ke Arduino IDE Anda.
2.  **Verifikasi & Unggah**:
    * Klik ikon **Centang (Verify)** untuk *sanity check* kode Anda.
    * Setelah bebas error, klik ikon **Panah Kanan (Upload)**. Pastikan NodeMCU Anda terhubung ke komputer.
    * **Tips Unggah Anti-Gagal**: Jika upload macet ("Failed to connect"), coba tekan dan tahan tombol **`FLASH`** atau **`BOOT`** pada NodeMCU saat proses upload dimulai. Atau, tahan `BOOT`, tekan `RESET` sebentar, lalu lepaskan `BOOT`.

### 3. Saatnya Bersenang-senang! (Uji Coba)

1.  **Lihat Konsol Ajaib (Serial Monitor)**:
    * Buka `Tools > Serial Monitor` di Arduino IDE, set baud rate ke `115200`.
    * Lihat NodeMCU Anda menyala dan mencetak pesan penting: SSID hotspot (`LED Kontroler`) dan alamat IP (`192.168.4.1`).
2.  **Join ke Hotspot LED**:
    * Dari ponsel atau laptop Anda, cari dan sambungkan ke jaringan Wi-Fi bernama **"LED Kontroler"**.
    * Passwordnya: **"BUMIKITA"**.
3.  **Buka Remote Kontrol Web**:
    * Di browser web Anda, ketik alamat IP yang Anda lihat di Serial Monitor (biasanya `http://192.168.4.1`).
    * *Voila!* Panel kontrol LED Anda muncul di layar.
4.  **Mainkan Lampunya!** ✨
    * Klik tombol mode, geser slider kecerahan, pilih warna. Lihat bagaimana LED strip Anda berubah!
5.  **Coba Tepuk Tangan!** 👏
    * Tepuk tangan dengan jelas di dekat sensor suara. Lihat LED Anda merespons, dan log di web ikut berubah.

---

📄 **Lisensi**

Proyek ini dirilis di bawah [MIT License](LICENSE). Bebas untuk digunakan, dimodifikasi, dan disebarluaskan untuk keperluan apa pun.

---

👋 **Tertarik untuk Kolaborasi?**

Kami sangat terbuka untuk kontribusi! Jika Anda punya ide baru, menemukan *bug*, atau ingin menambahkan fitur, silakan:
* Buka `Issues` untuk melaporkan masalah atau menyarankan ide.
* Buat `Pull Request` jika Anda sudah punya kode yang siap digabungkan.

Mari ciptakan lebih banyak proyek keren bersama!
