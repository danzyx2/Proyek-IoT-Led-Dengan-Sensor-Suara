# Proyek-IoT-Led-Dengan-Sensor-Suara


Kode ini adalah sebuah program Arduino C++ yang dirancang untuk mikrokontroler ESP8266 (misalnya NodeMCU). Fungsinya adalah untuk mengontrol LED strip jenis WS2812B (atau NeoPixel) melalui antarmuka web yang di-host langsung oleh NodeMCU itu sendiri. Selain itu, ada fitur tambahan untuk mendeteksi suara (misalnya tepukan tangan) menggunakan sensor suara digital untuk menyalakan/mematikan LED atau mengganti mode.

Secara garis besar, kode ini memiliki fitur:
 * Hotspot Wi-Fi (Access Point): NodeMCU akan membuat jaringannya sendiri dengan SSID dan password yang ditentukan (LED Kontroler / BUMIKITA). Anda bisa terhubung ke jaringan ini dari ponsel atau komputer Anda.
 * Web Server Lokal: Setelah terhubung ke hotspot NodeMCU, Anda bisa mengakses antarmuka web (halaman HTML) melalui browser. Antarmuka ini memungkinkan Anda:
   * Melihat status LED (ON/OFF, mode aktif).
   * Melihat kecerahan LED saat ini.
   * Melihat status memori NodeMCU (RAM dan Flash).
   * Mengontrol mode efek LED (mati, pelangi, warna solid, fade, dua warna, komet, hujan meteor, api, percikan).
   * Mengatur kecerahan LED menggunakan slider.
   * Menyesuaikan parameter spesifik untuk setiap mode LED (misal: warna solid, kecepatan pelangi, warna komet, dll.).
   * Melihat log deteksi suara (kapan suara terdeteksi).
   * Menghapus log suara.
 * Deteksi Suara (Tepukan Tangan): Menggunakan sensor suara digital, NodeMCU dapat mendeteksi suara keras (seperti tepukan tangan). Ketika terdeteksi:
   * Jika LED dalam keadaan mati, ia akan menyala kembali ke mode terakhir yang aktif.
   * Jika LED dalam keadaan menyala, ia akan mati.
   * Setiap deteksi akan dicatat ke dalam log yang bisa dilihat di antarmuka web.
 * Berbagai Efek LED FastLED: Kode ini memanfaatkan library FastLED yang kuat untuk menciptakan berbagai efek pencahayaan dinamis pada LED strip:
   * MODE_OFF: Mematikan semua LED.
   * MODE_RAINBOW: Efek pelangi yang bergerak.
   * MODE_SOLID_COLOR: Menampilkan satu warna solid.
   * MODE_FADE: Warna-warna memudar secara bertahap.
   * MODE_TWO_COLOR: Dua warna bergantian.
   * MODE_COMET_TRAIL: (Sebelumnya MOVING_LED) Efek komet dengan kepala terang dan ekor memudar.
   * MODE_METEOR_RAIN: Efek hujan meteor dengan jejak.
   * MODE_FIRE: Efek api yang realistis.
   * MODE_SPARKLE: Efek percikan acak.
 * SPIFFS (Flash File System): Digunakan untuk mengelola file sistem pada memori flash NodeMCU (meskipun dalam kode ini tidak ada file eksternal yang disimpan, ini adalah persiapan yang baik jika Anda ingin menyimpan konfigurasi atau halaman HTML di masa depan).
 * Responsive Web Interface (HTML, CSS, JavaScript): Antarmuka web dibuat dengan HTML dan styling CSS agar terlihat modern dan responsif di berbagai ukuran layar. JavaScript digunakan untuk pembaruan status secara real-time (melalui AJAX) tanpa perlu me-refresh halaman.
Komponen Utama Kode
 * Inklusi Library: Memanggil library yang diperlukan seperti ESP8266WiFi, ESP8266WebServer, FastLED, dan FS.
 * Definisi Hardware: Menentukan pin-pin yang digunakan untuk sensor suara dan LED strip, serta jumlah LED dan tipenya.
 * Variabel Global: Menyimpan status aktuall aplikasi seperti mode LED, warna, kecepatan, kecerahan, status sensor suara, dan log.
 * Fungsi Mode LED: Setiap fungsi (rainbow, twoColor, cometTrail, dll.) mengimplementasikan logika visual untuk efek LED tertentu. Fungsi EVERY_N_MILLISECONDS dari FastLED digunakan untuk menjalankan kode efek pada interval waktu tertentu, memastikan animasi yang halus.
 * Web Server Handlers: Fungsi-fungsi yang menangani permintaan HTTP dari browser (misalnya, handleRoot untuk halaman utama, handleSetLedMode untuk mengubah mode, dll.). Ini adalah "otak" di balik antarmuka web.
 * setup(): Fungsi ini dijalankan sekali saat NodeMCU dihidupkan. Ini menginisialisasi serial monitor, FastLED, sensor suara, mengonfigurasi NodeMCU sebagai Access Point, dan mendaftarkan semua handler web server.
 * loop(): Fungsi ini berjalan berulang kali tanpa henti. Di sini, server menangani permintaan klien, membaca status sensor suara, dan menjalankan fungsi efek LED yang sesuai dengan mode yang sedang aktif.

   
Cara Membuatnya Berhasil (Langkah Demi Langkah)
Untuk membuat kode ini berhasil berjalan di NodeMCU Anda, ikuti langkah-langkah berikut:
1. Persiapan Hardware
Anda akan membutuhkan:
 * NodeMCU ESP8266 (misal ESP-12E)
 * LED Strip WS2812B / NeoPixel: Perhatikan jumlah LED (NUM_LEDS) dan pin data (LED_DATA_PIN) yang Anda definisikan di kode. Untuk tutorial ini, kita asumsikan 8 LED dan pin D2.
 * Sensor Suara Digital: Modul sensor suara dengan output digital (DO).
 * Kabel Jumper: Untuk menghubungkan komponen.
 * Power Supply Eksternal (Opsional tapi Disarankan): Jika Anda menggunakan LED strip dengan banyak LED (lebih dari 8-10), atau jika Anda melihat LED berkedip/berperilaku aneh, Anda memerlukan catu daya 5V yang terpisah dan cukup kuat untuk LED strip. Jangan memberi daya LED strip langsung dari NodeMCU kecuali hanya beberapa LED.
Skema Koneksi Dasar:
 * NodeMCU ke LED Strip:
   * NodeMCU D2 (GPIO 4) ke Data In LED Strip
   * NodeMCU GND ke GND LED Strip
   * NodeMCU 5V (pin VIN) ke 5V LED Strip (HANYA untuk sedikit LED, disarankan Power Supply Eksternal 5V untuk LED)
     * Jika pakai Power Supply Eksternal untuk LED: Hubungkan GND dari Power Supply ke GND NodeMCU dan GND LED Strip. Hubungkan 5V dari Power Supply ke 5V LED Strip. JANGAN hubungkan 5V Power Supply LED ke 5V NodeMCU, kecuali Anda yakin NodeMCU Anda memiliki regulator yang sesuai. Lebih aman: hubungkan hanya GND secara bersamaan.
 * NodeMCU ke Sensor Suara:
   * Sensor Suara DO ke NodeMCU D5 (GPIO 14)
   * Sensor Suara GND ke NodeMCU GND
   * Sensor Suara VCC ke NodeMCU 3V3 (atau 5V, tergantung sensor Anda, tapi 3V3 lebih aman untuk pin ESP8266)
2. Persiapan Software (Arduino IDE)
 * Unduh dan Instal Arduino IDE: Jika Anda belum punya, unduh dari situs web resmi Arduino.
 * Tambahkan Board ESP8266:
   * Buka Arduino IDE.
   * Pergi ke File > Preferences.
   * Di kolom "Additional Board Manager URLs", tambahkan URL ini:
     http://arduino.esp8266.com/stable/package_esp8266com_index.json
   * Klik OK.
   * Pergi ke Tools > Board > Board Manager....
   * Cari "esp8266" dan instal paket "esp8266 by ESP8266 Community".
 * Instal Library yang Diperlukan:
   * FastLED: Pergi ke Sketch > Include Library > Manage Libraries.... Cari "FastLED" dan instal versi terbaru.
   * ESP8266WebServer dan ESP8266WiFi: Ini seharusnya sudah terinstal otomatis saat Anda menambahkan board ESP8266.
   * FS (SPIFFS): Ini juga bagian dari core ESP8266.
 * Pilih Board dan Port:
   * Pergi ke Tools > Board. Pilih "NodeMCU 1.0 (ESP-12E Module)".
   * Pergi ke Tools > Port. Pilih port serial yang terhubung dengan NodeMCU Anda (misal: COM3, /dev/cu.usbserial-XXXX).
3. Upload Kode ke NodeMCU
 * Salin Kode: Salin seluruh kode C++ yang Anda berikan ke Arduino IDE.
 * Verifikasi Kode: Klik tombol centang (Verify) di pojok kiri atas Arduino IDE untuk memeriksa apakah ada kesalahan sintaks.
 * Upload Kode: Setelah verifikasi berhasil, klik tombol panah kanan (Upload) untuk mengunggah kode ke NodeMCU Anda. Pastikan NodeMCU terhubung dengan benar ke komputer Anda.
   * Tips Upload: Jika Anda mengalami masalah upload (misalnya, "Failed to connect to ESP8266"), coba tekan tombol FLASH atau BOOT pada NodeMCU saat proses upload dimulai atau tahan BOOT lalu tekan RESET sebentar, kemudian lepaskan BOOT.
4. Menguji Sistem
 * Monitor Serial: Buka Serial Monitor (Tools > Serial Monitor) di Arduino IDE. Pastikan baud rate diatur ke 115200. Anda akan melihat pesan inisialisasi, termasuk SSID hotspot dan alamat IP.
 * Koneksi Hotspot: Dari ponsel atau komputer Anda, cari jaringan Wi-Fi baru dengan nama "LED Kontroler". Masukkan password "BUMIKITA" untuk terhubung.
 * Akses Web Interface: Setelah terhubung ke hotspot, buka browser web Anda dan ketik alamat IP yang ditampilkan di Serial Monitor (biasanya 192.168.4.1). Anda akan melihat antarmuka kontrol LED.
 * Kontrol LED:
   * Coba klik tombol mode LED yang berbeda.
   * Geser slider kecerahan.
   * Pilih warna untuk mode solid, dll.
   * Perhatikan bagaimana LED strip merespons.
 * Uji Sensor Suara: Tepuk tangan di dekat sensor suara. Anda akan melihat LED berganti status (mati/menyala) dan log deteksi suara akan muncul di antarmuka web.
