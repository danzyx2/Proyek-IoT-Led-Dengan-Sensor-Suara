# Proyek-IoT-Led-Dengan-Sensor-Suara



💡 Smart LED Strip Controller dengan NodeMCU & Deteksi Suara 🔊
Kontrol LED strip WS2812B Anda dengan mudah melalui antarmuka web intuitif yang di-host langsung dari NodeMCU ESP8266 Anda! Proyek ini juga dilengkapi fitur deteksi suara (tepukan tangan) untuk kontrol tanpa sentuhan.
(Ganti URL ini dengan gambar/gif/video demo proyek Anda untuk tampilan yang lebih menarik!)
✨ Fitur Unggulan
 * Hotspot Wi-Fi Terintegrasi: NodeMCU bertindak sebagai Access Point (SSID: "LED Kontroler", Password: "BUMIKITA"), memungkinkan Anda terhubung langsung dari ponsel atau laptop Anda tanpa perlu router eksternal.
 * Antarmuka Web Lokal: Akses halaman kontrol LED yang responsif dan modern langsung dari browser Anda (biasanya di http://192.168.4.1). Dari sana, Anda dapat:
   * Melihat status dan kecerahan LED secara real-time.
   * Memantau penggunaan memori (RAM & Flash) NodeMCU.
   * Mengganti dan mengelola berbagai mode efek LED dinamis.
   * Menyesuaikan kecerahan LED dengan slider interaktif.
   * Mengatur parameter spesifik untuk setiap mode (misalnya, warna solid, kecepatan pelangi, warna komet).
   * Melihat log deteksi suara (tepukan tangan) yang terjadi.
   * Menghapus log suara untuk tampilan yang bersih.
 * Kontrol Suara Inovatif: Gunakan sensor suara digital untuk menyalakan/mematikan LED atau mengaktifkan mode terakhir hanya dengan tepukan tangan!
 * Beragam Efek LED FastLED: Ditenagai oleh library FastLED yang canggih, proyek ini menghadirkan berbagai efek visual menakjubkan:
   * MODE_OFF: Matikan semua LED.
   * MODE_RAINBOW: Animasi pelangi klasik yang bergerak.
   * MODE_SOLID_COLOR: Tampilkan warna solid pilihan Anda.
   * MODE_FADE: Transisi warna yang halus dan memudar.
   * MODE_TWO_COLOR: Dua warna yang bergantian secara dinamis.
   * MODE_COMET_TRAIL: (Sebelumnya MOVING_LED) Efek komet yang bergerak dengan kepala terang dan ekor memudar.
   * MODE_METEOR_RAIN: Efek hujan meteor dengan jejak yang memukau.
   * MODE_FIRE: Simulasi efek api yang realistis dan berkedip.
   * MODE_SPARKLE: Percikan cahaya acak yang indah.
 * Sistem File SPIFFS: Terintegrasi dengan SPIFFS untuk potensi penyimpanan konfigurasi atau halaman web yang lebih kompleks di masa depan.
🛠️ Persyaratan Hardware
 * Mikrokontroler: NodeMCU ESP8266 (ESP-12E Module)
 * LED Strip: WS2812B Addressable LED Strip (sesuaikan NUM_LEDS dan LED_DATA_PIN di kode). Contoh ini menggunakan 8 LED.
 * Sensor Suara: Modul sensor suara digital dengan output DO (Digital Output).
 * Kabel Jumper: Untuk koneksi antar komponen.
 * Catu Daya 5V Eksternal (Sangat Disarankan): Untuk LED strip dengan lebih dari 8-10 LED atau jika Anda melihat perilaku LED yang tidak stabil, gunakan catu daya 5V terpisah yang memadai.
🔌 Skema Koneksi (Wiring)
Berikut adalah panduan koneksi dasar untuk proyek ini:
+----------------+          +-----------------+          +-----------------+
|    NodeMCU     |          |   LED Strip     |          |   Sensor Suara  |
|                |          | (WS2812B)       |          |    (Digital)    |
|       D2 (GPIO4) --------> Data In           |          |                 |
|                |          |                 |          |                 |
|       GND --------------> GND -------------+-----------> GND             |
|                |          |                 |          |                 |
|       VIN (5V) ----------> 5V (Jika LED sedikit) |          |                 |
| (Untuk Power LED)        |                 |          |                 |
|                |          |                 |          |                 |
|       D5 (GPIO14) <------ DO                |          |                 |
|                |          |                 |          |                 |
|       3V3 ---------------> VCC              |          |                 |
+----------------+          +-----------------+          +-----------------+

Catatan Penting untuk Power Supply LED:
 * Jika LED strip Anda memiliki lebih dari 8-10 LED, sangat disarankan untuk menggunakan catu daya 5V eksternal yang terpisah untuk LED strip.
 * Hubungkan GND dari catu daya eksternal LED ke GND NodeMCU dan GND LED strip.
 * Hubungkan +5V dari catu daya eksternal LED ke +5V pada LED strip.
 * JANGAN hubungkan langsung +5V dari catu daya eksternal LED ke pin VIN/+5V NodeMCU Anda kecuali Anda tahu NodeMCU Anda dapat menangani input voltage tersebut. Hubungkan hanya pin GND secara bersamaan untuk memastikan referensi tegangan yang sama.
🚀 Panduan Memulai Cepat
Ikuti langkah-langkah ini untuk menjalankan proyek di NodeMCU Anda:
1. Persiapan Lingkungan Pengembangan (Arduino IDE)
 * Unduh Arduino IDE: Dapatkan versi terbaru dari situs resmi Arduino.
 * Tambahkan Board ESP8266:
   * Buka Arduino IDE, navigasi ke File > Preferences.
   * Di kolom "Additional Board Manager URLs", tambahkan URL berikut:
     http://arduino.esp8266.com/stable/package_esp8266com_index.json
   * Klik OK.
   * Buka Tools > Board > Board Manager.... Cari "esp8266" dan instal paket "esp8266 by ESP8266 Community".
 * Instal Library yang Diperlukan:
   * Buka Sketch > Include Library > Manage Libraries....
   * Cari dan instal FastLED.
   * Library ESP8266WebServer, ESP8266WiFi, dan FS (SPIFFS) seharusnya sudah otomatis terinstal dengan core ESP8266.
 * Pilih Board dan Port:
   * Pergi ke Tools > Board, lalu pilih "NodeMCU 1.0 (ESP-12E Module)".
   * Pergi ke Tools > Port, dan pilih port serial yang terhubung dengan NodeMCU Anda (misalnya, COM3 di Windows atau /dev/cu.usbserial-XXXX di macOS/Linux).
2. Unggah Kode ke NodeMCU
 * Salin Kode: Salin seluruh kode Arduino C++ (.ino) dari repositori ini ke Arduino IDE Anda.
 * Verifikasi & Unggah:
   * Klik ikon Centang (Verify) di pojok kiri atas Arduino IDE untuk memeriksa sintaks kode.
   * Setelah verifikasi berhasil, klik ikon Panah Kanan (Upload) untuk mengunggah kode ke NodeMCU Anda.
   * Tips Unggah: Jika Anda mengalami masalah seperti "Failed to connect to ESP8266", coba tekan dan tahan tombol FLASH atau BOOT pada NodeMCU Anda saat proses unggah dimulai, atau tahan BOOT lalu tekan RESET sebentar, kemudian lepaskan BOOT.
3. Pengujian Sistem
 * Monitor Serial: Buka Tools > Serial Monitor di Arduino IDE dan atur baud rate ke 115200. Anda akan melihat pesan inisialisasi, termasuk SSID hotspot (LED Kontroler) dan Alamat IP (192.168.4.1).
 * Koneksi Wi-Fi: Dari ponsel atau komputer Anda, cari dan sambungkan ke jaringan Wi-Fi bernama "LED Kontroler". Masukkan kata sandi "BUMIKITA".
 * Akses Antarmuka Web: Buka browser web Anda dan ketik alamat IP yang ditampilkan di Serial Monitor (default: http://192.168.4.1). Antarmuka kontrol LED akan muncul.
 * Kontrol LED: Eksplorasi berbagai tombol mode, geser slider kecerahan, dan sesuaikan parameter untuk setiap efek. Perhatikan bagaimana LED strip Anda merespons!
 * Uji Deteksi Suara: Tepuk tangan dengan jelas di dekat sensor suara. Anda akan melihat LED berganti status (ON/OFF) dan log deteksi suara akan diperbarui secara otomatis di antarmuka web.
📄 Lisensi
Proyek ini dilisensikan di bawah MIT License. Anda bebas untuk menggunakan, memodifikasi, dan mendistribusikan kode ini untuk tujuan pribadi atau komersial.
🤝 Kontribusi
Kontribusi dalam bentuk bug reports, feature requests, atau pull requests sangat kami hargai! Mari buat proyek ini lebih baik bersama.
❓ Ada Pertanyaan?
Jika Anda memiliki pertanyaan atau mengalami masalah, jangan ragu untuk membuka Issues di repositori GitHub ini. Kami akan dengan senang hati membantu!
