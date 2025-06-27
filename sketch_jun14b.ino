#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FastLED.h>
#include <FS.h> // Untuk SPIFFS

// --- Definisi Hardware & Konstanta ---
#define SENSOR_SUARA_DO_PIN D5 // Pin digital untuk sensor suara
#define LED_DATA_PIN D2        // Pin data untuk LED strip
#define NUM_LEDS 8             // Jumlah LED pada strip
#define LED_TYPE WS2812B       // Tipe LED (misal: WS2812B, NEOPIXEL)
#define COLOR_ORDER GRB        // Urutan warna LED (misal: GRB, RGB)
#define BRIGHTNESS 180         // Kecerahan awal LED (0-255)

// --- Objek Global ---
CRGB leds[NUM_LEDS];         // Array untuk menyimpan data warna setiap LED
const char* ssid = "LED Kontroler";      // SSID untuk hotspot NodeMCU
const char* password = "BUMIKITA"; // Password untuk hotspot NodeMCU

ESP8266WebServer server(80); // Inisialisasi web server pada port 80

// --- Enumerasi Mode LED ---
enum LedMode {
  MODE_OFF,
  MODE_RAINBOW,
  MODE_SOLID_COLOR,
  MODE_FADE,
  MODE_TWO_COLOR,
  MODE_COMET_TRAIL, // <--- NAMA MODE DIUBAH: Dulu MOVING_LED, sekarang COMET_TRAIL
  MODE_METEOR_RAIN,
  MODE_FIRE,
  MODE_SPARKLE
};

// --- Variabel State Aplikasi ---
LedMode currentLedMode = MODE_OFF;        // Mode LED yang sedang aktif
LedMode lastActiveLedMode = MODE_RAINBOW; // Mode LED terakhir sebelum dimatikan

CRGB currentSolidColor = CRGB::Blue;  // Warna solid saat ini
uint8_t currentRainbowSpeed = 70;      // Kecepatan efek pelangi (nilai kecil = cepat)
int currentFadeSpeed = 500;             // Kecepatan efek fade (nilai kecil = cepat)
uint8_t fadeHue = 0;                  // Hue untuk efek fade
uint8_t currentBrightness = BRIGHTNESS; // Kecerahan LED saat ini

CRGB colorOne = CRGB::Red;            // Warna pertama untuk mode dua warna
CRGB colorTwo = CRGB::Green;          // Warna kedua untuk mode dua warna
unsigned long lastTwoColorToggleTime = 0; // Waktu terakhir mode dua warna berganti
const unsigned long TWO_COLOR_TOGGLE_DELAY = 1000; // Delay ganti warna mode dua warna (ms)
bool twoColorState = false;           // State untuk mode dua warna

// Variabel untuk MODE_COMET_TRAIL (dulu MOVING_LED)
int cometPosition = 0;                // Posisi awal kepala komet (akan terus bertambah)
CRGB cometColor = CRGB::White;        // Warna komet
int cometSpeed = 100;                 // Kecepatan pergerakan komet (ms per langkah, nilai kecil = cepat)
const int COMET_TRAIL_LENGTH = 4;     // Panjang jejak komet (Kepala + Ekor)

uint8_t gHue = 0; // Global hue untuk efek pelangi

// --- Variabel untuk Mode Lain ---
int meteorRainSpeed = 50;
int meteorRainTrailLength = 4;
CRGB meteorRainColor = CRGB::Blue;
int meteorRainPosition = 0;

int fireSpeed = 20;
uint8_t fireCooling = 55;
uint8_t fireSparking = 120;

int sparkleChance = 10;
int sparkleFadeSpeed = 15;
CRGB sparkleColor = CRGB::White;

bool lastSoundState = HIGH;             // State terakhir sensor suara (HIGH = tidak ada suara)
unsigned long lastSoundTime = 0;        // Waktu terakhir deteksi suara
const unsigned long DEBOUNCE_DELAY = 200; // Debounce untuk sensor suara (ms)

String soundDetectionLog = ""; // String untuk menyimpan log deteksi suara

// --- Variabel untuk Kontrol Update Web ---
bool webUpdateActive = false; // Flag untuk mengontrol apakah update web (AJAX) aktif
unsigned long lastClientCheckTime = 0;      // Waktu terakhir cek koneksi klien
const unsigned long CLIENT_CHECK_INTERVAL = 5000; // Interval cek koneksi klien (ms)

// --- Fungsi Helper ---

// Mengambil waktu saat ini dalam format HH:MM:SS
String getCurrentTime() {
  unsigned long currentMillis = millis();
  unsigned long totalSeconds = currentMillis / 1000;
  int hours = (totalSeconds / 3600) % 24;
  int minutes = (totalSeconds / 60) % 60;
  int secs = totalSeconds % 60;
  char timeStr[9];
  sprintf(timeStr, "%02d:%02d:%02d", hours, minutes, secs);
  return String(timeStr);
}

// --- Fungsi-fungsi Mode LED ---

// Efek pelangi
void rainbow() {
  EVERY_N_MILLISECONDS(currentRainbowSpeed) {
    fill_rainbow(leds, NUM_LEDS, gHue++, 20); // Mengisi strip dengan pelangi
  }
  FastLED.show();
}

// Efek dua warna bergantian
void twoColor() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastTwoColorToggleTime >= TWO_COLOR_TOGGLE_DELAY) {
    twoColorState = !twoColorState; // Ganti state
    lastTwoColorToggleTime = currentMillis;
  }
  if (twoColorState) {
    fill_solid(leds, NUM_LEDS, colorOne);
  } else {
    fill_solid(leds, NUM_LEDS, colorTwo);
  }
  FastLED.show();
}

// --- MODE BARU: KOMET (menggantikan MOVING_LED) ---
void cometTrail() {
  EVERY_N_MILLISECONDS(cometSpeed) {
    // 1. Perlahan-lahan pudarkan semua LED untuk menciptakan jejak
    // Nilai fadeToBlackBy yang kecil membuat jejak lebih panjang dan mulus
    fadeToBlackBy(leds, NUM_LEDS, 50); // Sesuaikan nilai ini (1-255) untuk panjang jejak

    // 2. Gambar kepala komet
    int headPos = cometPosition % NUM_LEDS; // Posisi kepala komet yang melingkar
    leds[headPos] = cometColor;             // Kepala komet dengan warna penuh

    // 3. Gambar ekor komet (COMET_TRAIL_LENGTH-1 LED di belakang kepala)
    // Ekor akan memudar secara bertahap, memberikan kesan "api" komet
    for (int i = 1; i < COMET_TRAIL_LENGTH; i++) {
      int tailPos = (cometPosition - i + NUM_LEDS * 2) % NUM_LEDS; // Pastikan posisi tidak negatif
      // Hitung kecerahan ekor: semakin jauh dari kepala, semakin pudar
      // Gunakan FastLED.ColorFromPalette untuk interpolasi warna yang lebih halus jika diinginkan,
      // atau tetap nscale8 untuk kontrol kecerahan langsung.
      // Di sini kita pakai nscale8 untuk pudar dari kepala.
      uint8_t brightness = 255 - (i * (255 / COMET_TRAIL_LENGTH)); // Kecerahan berkurang secara linier
      if (brightness < 0) brightness = 0; // Pastikan tidak negatif
      leds[tailPos] = cometColor;
      leds[tailPos].nscale8(brightness); // Atur kecerahan LED ekor
    }

    // 4. Gerakkan posisi komet maju
    cometPosition++;
    // Tidak perlu reset cometPosition ke 0. Biarkan terus bertambah.
    // Modulo pada headPos dan tailPos yang akan memastikan pergerakan melingkar.

    FastLED.show(); // Tampilkan perubahan pada LED
  }
}

// Mode Meteor Rain (Jejak 4 LED yang bergerak satu arah, tetap seperti sebelumnya)
void meteorRain() {
  EVERY_N_MILLISECONDS(meteorRainSpeed) {
    fadeToBlackBy(leds, NUM_LEDS, 10); // Semakin kecil nilai, semakin panjang jejaknya

    for (int i = 0; i < meteorRainTrailLength; i++) {
      int ledIndex = (meteorRainPosition - i + NUM_LEDS) % NUM_LEDS; // Perhitungan untuk jejak ke belakang
      leds[ledIndex] = meteorRainColor;
    }

    meteorRainPosition++;

    if (meteorRainPosition >= NUM_LEDS) {
      meteorRainPosition = 0;
    }
    FastLED.show();
  }
}

// Mode Fire
void fire() {
  EVERY_N_MILLISECONDS(fireSpeed) {
    for( int i = 0; i < NUM_LEDS; i++) {
      leds[i].nscale8(255 - fireCooling);
    }

    for( int k= NUM_LEDS - 1; k >= 2; k--) {
      leds[k] = (leds[k-1] + leds[k-2] + leds[k-2] ) / 3;
    }

    if( random8() < fireSparking ) {
      int y = random8(7);
      leds[y] = leds[y] + CHSV(0, 255, random8(160,255));
    }
    FastLED.show();
  }
}

// Mode Sparkle
void sparkle() {
  EVERY_N_MILLISECONDS(sparkleFadeSpeed) {
    fadeToBlackBy(leds, NUM_LEDS, 1);

    if (random8(100) < sparkleChance) {
      leds[random16(NUM_LEDS)] = sparkleColor;
    }
    FastLED.show();
  }
}

// --- Fungsi untuk Memeriksa Koneksi Klien (untuk update web) ---
void checkClientConnections() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastClientCheckTime >= CLIENT_CHECK_INTERVAL) {
    int connectedClients = WiFi.softAPgetStationNum();
    if (connectedClients > 0) {
      if (!webUpdateActive) {
        Serial.printf("Klien terhubung: %d. Mengaktifkan update web.\n", connectedClients);
        webUpdateActive = true;
      }
    } else {
      if (webUpdateActive) {
        Serial.println("Tidak ada klien terhubung. Menonaktifkan update web.");
        webUpdateActive = false;
      }
    }
    lastClientCheckTime = currentMillis;
  }
}

// --- Handler Permintaan HTTP (Deklarasi & Definisi di sini) ---

// Handler untuk halaman utama
void handleRoot() {
  String html = "<!DOCTYPE html><html><head><title>Monitor & Control</title>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>";
  html += "body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; text-align: center; margin: 0; padding-top: 20px; background-color: #1a1a1a; color: #e0e0e0; line-height: 1.6; }";
  html += "h1, h2 { color: #f0f0f0; margin-bottom: 10px; }";
  html += "p { font-size: 1.1em; margin-bottom: 5px; }";
  html += "b { color: " + String((currentLedMode == MODE_OFF) ? "#ff6b6b" : "#4caf50") + "; transition: color 0.3s ease; }";
  html += ".section-title { font-size: 1.5em; margin-top: 30px; margin-bottom: 15px; color: #c0c0c0; border-bottom: 2px solid #444; padding-bottom: 5px; }";
  html += ".button-container { margin: 20px 0; display: flex; flex-wrap: wrap; justify-content: center; align-items: center; gap: 10px; }";
  html += ".button { ";
  html += "  display: inline-block; padding: 12px 25px; ";
  html += "  background-color: #333; color: #f0f0f0; border: none; ";
  html += "  border-radius: 8px; cursor: pointer; font-size: 1.1em; ";
  html += "  text-decoration: none; transition: background-color 0.3s ease, transform 0.2s ease, box-shadow 0.2s ease; ";
  html += "  box-shadow: 0 4px 10px rgba(0,0,0,0.4);";
  html += "}";
  html += ".button:hover { background-color: #555; transform: translateY(-3px); box-shadow: 0 6px 12px rgba(0,0,0,0.6); }";
  html += ".button.active { background-color: #007bff; cursor: default; box-shadow: 0 4px 10px rgba(0,0,0,0.4); transform: translateY(0); }";
  html += ".button.active:hover { background-color: #007bff; }";
  html += "form { display: contents; }";
  html += "input[type='color'] { -webkit-appearance: none; -moz-appearance: none; appearance: none; border: none; width: 60px; height: 40px; cursor: pointer; border-radius: 8px; padding: 0; display: inline-block; vertical-align: middle; margin-right: 10px; background: transparent; box-shadow: inset 0 0 5px rgba(0,0,0,0.3); }";
  html += "input[type='color']::-webkit-color-swatch-wrapper { padding: 0; }";
  html += "input[type='color']::-webkit-color-swatch { border: 2px solid #555; border-radius: 8px; }";
  html += "input[type='range'] { width: calc(80% - 100px); margin: 0 10px; vertical-align: middle; background: #555; border-radius: 5px; height: 8px; cursor: pointer; outline: none; }";
  html += "input[type='range']::-webkit-slider-thumb { -webkit-appearance: none; appearance: none; width: 20px; height: 20px; border-radius: 50%; background: #007bff; cursor: pointer; transition: background-color 0.3s ease, transform 0.2s ease; box-shadow: 0 2px 5px rgba(0,0,0,0.3); }";
  html += "input[type='range']::-webkit-slider-thumb:hover { background: #0056b3; transform: scale(1.1); }";
  html += "output { font-weight: bold; color: #007bff; display: inline-block; width: 40px; text-align: left; vertical-align: middle; }";
  html += ".log-box, .memory-box { width: 90%; max-width: 700px; margin: 30px auto; border-collapse: collapse; box-shadow: 0 5px 15px rgba(0,0,0,0.5); background-color: #2a2a2a; border-radius: 10px; overflow: hidden; }";
  html += ".log-box, .memory-box { padding: 20px; text-align: left; background-color: #2a2a2a; border: 1px solid #3a3a3a; }";
  html += ".log-entry { margin-bottom: 8px; border-bottom: 1px dashed #444; padding-bottom: 5px; }";
  html += ".log-entry:last-child { border-bottom: none; }";
  html += ".clear-button { background-color: #dc3545; }";
  html += ".clear-button:hover { background-color: #c82333; }";
  html += "@keyframes fadeIn { from { opacity: 0; } to { opacity: 1; } }";
  html += ".fade-in { animation: fadeIn 0.5s ease-in-out; }";
  html += ".footer { margin-top: 50px; padding: 20px; font-size: 0.9em; color: #888; background-color: #222; }";
  html += "</style>";
  html += "</head><body>";
  html += "<div class='fade-in'>";
  html += "<h1>Status LED: <b id='ledStatusDisplay'>";
  if (currentLedMode == MODE_RAINBOW) {
    html += "PELANGI AKTIF";
  } else if (currentLedMode == MODE_SOLID_COLOR) {
    html += "WARNA SOLID";
  } else if (currentLedMode == MODE_FADE) {
    html += "FADE WARNA";
  } else if (currentLedMode == MODE_TWO_COLOR) {
    html += "DUA WARNA";
  } else if (currentLedMode == MODE_COMET_TRAIL) { // <--- NAMA MODE DIUBAH
    html += "KOMET";
  } else if (currentLedMode == MODE_METEOR_RAIN) {
    html += "HUJAN METEOR";
  } else if (currentLedMode == MODE_FIRE) {
    html += "EFEK API";
  } else if (currentLedMode == MODE_SPARKLE) {
    html += "KEDIP PERCIKAN";
  } else {
    html += "MATI";
  }
  html += "</b></h1>";
  html += "<p>Kecerahan LED: <b id='brightnessDisplay'>" + String(currentBrightness) + "</b></p>";
  html += "<p>IP Address NodeMCU: <b>" + WiFi.softAPIP().toString() + "</b></p>";
  html += "</div>";
  FSInfo fs_info;
  SPIFFS.info(fs_info);
  html += "<div class='section-title fade-in'>Status Memori:</div>";
  html += "<div class='memory-box fade-in'>";
  html += "<p>RAM Tersedia: <b>" + String(ESP.getFreeHeap() / 1024.0, 2) + " KB</b></p>";
  html += "<p>Flash Total: <b>" + String(fs_info.totalBytes / (1024.0 * 1024.0), 2) + " MB</b></p>";
  html += "<p>Flash Terpakai: <b>" + String(fs_info.usedBytes / (1024.0 * 1024.0), 2) + " MB</b></p>";
  html += "<p>Flash Tersedia: <b>" + String((fs_info.totalBytes - fs_info.usedBytes) / (1024.0 * 1024.0), 2) + " MB</b></p>";
  html += "</div>";
  html += "<div class='section-title fade-in'>Kontrol LED Strip:</div>";
  html += "<div class='button-container fade-in'>";
  html += "<a href='/setLedMode?mode=off' class='button " + String((currentLedMode == MODE_OFF) ? "active" : "") + "'>LED OFF</a>";
  html += "<a href='/setLedMode?mode=rainbow' class='button " + String((currentLedMode == MODE_RAINBOW) ? "active" : "") + "'>Pelangi</a>";
  html += "<a href='/setLedMode?mode=solid' class='button " + String((currentLedMode == MODE_SOLID_COLOR) ? "active" : "") + "'>Warna Solid</a>";
  html += "<a href='/setLedMode?mode=fade' class='button " + String((currentLedMode == MODE_FADE) ? "active" : "") + "'>Fade Warna</a>";
  html += "<a href='/setLedMode?mode=two_color' class='button " + String((currentLedMode == MODE_TWO_COLOR) ? "active" : "") + "'>Dua Warna</a>";
  html += "<a href='/setLedMode?mode=comet_trail' class='button " + String((currentLedMode == MODE_COMET_TRAIL) ? "active" : "") + "'>Komet</a>"; // <--- NAMA MODE DIUBAH
  html += "<a href='/setLedMode?mode=meteor_rain' class='button " + String((currentLedMode == MODE_METEOR_RAIN) ? "active" : "") + "'>Hujan Meteor</a>";
  html += "<a href='/setLedMode?mode=fire' class='button " + String((currentLedMode == MODE_FIRE) ? "active" : "") + "'>Api</a>";
  html += "<a href='/setLedMode?mode=sparkle' class='button " + String((currentLedMode == MODE_SPARKLE) ? "active" : "") + "'>Percikan</a>";
  html += "</div>";
  html += "<div class='section-title fade-in'>Pengaturan Kecerahan:</div>";
  html += "<div class='button-container fade-in'>";
  html += "<form action='/setBrightness' method='get'>";
  html += "<input type='range' name='brightness' min='0' max='255' value='" + String(currentBrightness) + "' oninput='this.nextElementSibling.value=this.value'>";
  html += "<output id='brightnessOutput'>" + String(currentBrightness) + "</output>";
  html += "<input type='submit' value='Set Kecerahan' class='button'>";
  html += "</form>";
  html += "</div>";
  if (currentLedMode == MODE_SOLID_COLOR) {
    html += "<div class='section-title fade-in'>Pilih Warna Solid:</div>";
    html += "<div class='button-container fade-in'>";
    html += "<form action='/setSolidColor' method='get'>";
    char hexColor[7];
    sprintf(hexColor, "%02X%02X%02X", currentSolidColor.r, currentSolidColor.g, currentSolidColor.b);
    html += "<input type='color' name='color' value='#" + String(hexColor) + "'>";
    html += "<input type='submit' value='Set Warna' class='button'>";
    html += "</form>";
    html += "</div>";
  }
  if (currentLedMode == MODE_RAINBOW) {
    html += "<div class='section-title fade-in'>Kecepatan Pelangi:</div>";
    html += "<div class='button-container fade-in'>";
    html += "<form action='/setRainbowSpeed' method='get'>";
    html += "<input type='range' name='speed' min='1' max='50' value='" + String(currentRainbowSpeed) + "' oninput='this.nextElementSibling.value=this.value'>";
    html += "<output>" + String(currentRainbowSpeed) + "</output>";
    html += "<input type='submit' value='Set Kecepatan' class='button'>";
    html += "</form>";
    html += "<p style='font-size:0.9em; color:#aaa;'>*Nilai lebih kecil = lebih cepat</p>";
    html += "</div>";
  }
  if (currentLedMode == MODE_FADE) {
    html += "<div class='section-title fade-in'>Kecepatan Fade:</div>";
    html += "<div class='button-container fade-in'>";
    html += "<form action='/setFadeSpeed' method='get'>";
    html += "<input type='range' name='speed' min='1' max='200' value='" + String(currentFadeSpeed) + "' oninput='this.nextElementSibling.value=this.value'>";
    html += "<output>" + String(currentFadeSpeed) + "</output>";
    html += "<input type='submit' value='Set Kecepatan' class='button'>";
    html += "</form>";
    html += "<p style='font-size:0.9em; color:#aaa;'>*Nilai lebih kecil = lebih cepat</p>";
    html += "</div>";
  }
  if (currentLedMode == MODE_TWO_COLOR) {
    html += "<div class='section-title fade-in'>Pengaturan Dua Warna:</div>";
    html += "<div class='button-container fade-in'>";
    html += "<form action='/setTwoColors' method='get'>";
    char hexColor1[7], hexColor2[7];
    sprintf(hexColor1, "%02X%02X%02X", colorOne.r, colorOne.g, colorOne.b);
    sprintf(hexColor2, "%02X%02X%02X", colorTwo.r, colorTwo.g, colorTwo.b);
    html += "<label for='color1'>Warna 1:</label><input type='color' id='color1' name='color1' value='#" + String(hexColor1) + "'>";
    html += "<label for='color2'>Warna 2:</label><input type='color' id='color2' name='color2' value='#" + String(hexColor2) + "'>";
    html += "<input type='submit' value='Set Warna' class='button'>";
    html += "</form>";
    html += "</div>";
  }
  if (currentLedMode == MODE_COMET_TRAIL) { // <--- NAMA MODE DIUBAH
    html += "<div class='section-title fade-in'>Pengaturan Komet:</div>"; // <--- TEKS DIUBAH
    html += "<div class='button-container fade-in'>";
    html += "<form action='/setCometTrail' method='get'>"; // <--- HANDLER DIUBAH
    html += "<label for='cometSpeed'>Kecepatan Gerak (ms):</label>"; // <--- TEKS DIUBAH
    html += "<input type='range' name='speed' min='10' max='500' value='" + String(cometSpeed) + "' oninput='this.nextElementSibling.value=this.value'>";
    html += "<output>" + String(cometSpeed) + "</output><br>";
    char hexCometColor[7];
    sprintf(hexCometColor, "%02X%02X%02X", cometColor.r, cometColor.g, cometColor.b);
    html += "<label for='cometColor'>Warna Komet:</label><input type='color' id='cometColor' name='color' value='#" + String(hexCometColor) + "'><br>"; // <--- TEKS DIUBAH
    html += "<input type='submit' value='Set Pengaturan' class='button'>";
    html += "</form>";
    html += "<p style='font-size:0.9em; color:#aaa;'>*Kecepatan Gerak: nilai lebih kecil = lebih cepat.</p>";
    html += "<p style='font-size:0.9em; color:#aaa;'>*Mode ini akan menampilkan 1 LED kepala dan 3 LED ekor memudar.</p>";
    html += "</div>";
  }
  if (currentLedMode == MODE_METEOR_RAIN) {
    html += "<div class='section-title fade-in'>Pengaturan Hujan Meteor:</div>";
    html += "<div class='button-container fade-in'>";
    html += "<form action='/setMeteorRain' method='get'>";
    html += "<label for='meteorRainSpeed'>Kecepatan Gerak (ms):</label>";
    html += "<input type='range' name='speed' min='10' max='500' value='" + String(meteorRainSpeed) + "' oninput='this.nextElementSibling.value=this.value'>";
    html += "<output>" + String(meteorRainSpeed) + "</output><br>";
    html += "<label for='meteorRainTrailLength'>Panjang Jejak:</label>";
    html += "<input type='range' name='length' min='1' max='" + String(NUM_LEDS) + "' value='" + String(meteorRainTrailLength) + "' oninput='this.nextElementSibling.value=this.value'>";
    html += "<output>" + String(meteorRainTrailLength) + "</output><br>";
    char hexMeteorColor[7];
    sprintf(hexMeteorColor, "%02X%02X%02X", meteorRainColor.r, meteorRainColor.g, meteorRainColor.b);
    html += "<label for='meteorRainColor'>Warna Meteor:</label><input type='color' id='meteorRainColor' name='color' value='#" + String(hexMeteorColor) + "'><br>";
    html += "<input type='submit' value='Set Pengaturan' class='button'>";
    html += "</form>";
    html += "<p style='font-size:0.9em; color:#aaa;'>*Kecepatan Gerak: nilai lebih kecil = lebih cepat.</p>";
    html += "</div>";
  }
  if (currentLedMode == MODE_FIRE) {
    html += "<div class='section-title fade-in'>Pengaturan Efek Api:</div>";
    html += "<div class='button-container fade-in'>";
    html += "<form action='/setFire' method='get'>";
    html += "<label for='fireSpeed'>Kecepatan Update (ms):</label>";
    html += "<input type='range' name='speed' min='1' max='100' value='" + String(fireSpeed) + "' oninput='this.nextElementSibling.value=this.value'>";
    html += "<output>" + String(fireSpeed) + "</output><br>";
    html += "<label for='fireCooling'>Pendinginan Api (0-255):</label>";
    html += "<input type='range' name='cooling' min='10' max='100' value='" + String(fireCooling) + "' oninput='this.nextElementSibling.value=this.value'>";
    html += "<output>" + String(fireCooling) + "</output><br>";
    html += "<label for='fireSparking'>Probabilitas Percikan (0-255):</label>";
    html += "<input type='range' name='sparking' min='50' max='250' value='" + String(fireSparking) + "' oninput='this.nextElementSibling.value=this.value'>";
    html += "<output>" + String(fireSparking) + "</output><br>";
    html += "<input type='submit' value='Set Pengaturan' class='button'>";
    html += "</form>";
    html += "<p style='font-size:0.9em; color:#aaa;'>*Kecepatan Update: nilai lebih kecil = lebih cepat.</p>";
    html += "</div>";
  }
  if (currentLedMode == MODE_SPARKLE) {
    html += "<div class='section-title fade-in'>Pengaturan Percikan:</div>";
    html += "<div class='button-container fade-in'>";
    html += "<form action='/setSparkle' method='get'>";
    html += "<label for='sparkleChance'>Peluang Percikan (0-100):</label>";
    html += "<input type='range' name='chance' min='1' max='100' value='" + String(sparkleChance) + "' oninput='this.nextElementSibling.value=this.value'>";
    html += "<output>" + String(sparkleChance) + "</output><br>";
    html += "<label for='sparkleFadeSpeed'>Kecepatan Pudar (ms):</label>";
    html += "<input type='range' name='fade' min='1' max='50' value='" + String(sparkleFadeSpeed) + "' oninput='this.nextElementSibling.value=this.value'>";
    html += "<output>" + String(sparkleFadeSpeed) + "</output><br>";
    char hexSparkleColor[7];
    sprintf(hexSparkleColor, "%02X%02X%02X", sparkleColor.r, sparkleColor.g, sparkleColor.b);
    html += "<label for='sparkleColor'>Warna Percikan:</label><input type='color' id='sparkleColor' name='color' value='#" + String(hexSparkleColor) + "'><br>";
    html += "<input type='submit' value='Set Pengaturan' class='button'>";
    html += "</form>";
    html += "<p style='font-size:0.9em; color:#aaa;'>*Kecepatan Pudar: nilai lebih kecil = lebih cepat pudar.</p>";
    html += "</div>";
  }
  html += "<div class='section-title fade-in'>Log Deteksi Suara (Tepuk Tangan):</div>";
  html += "<div class='log-box fade-in' id='soundLogBox'>";
  if (soundDetectionLog.isEmpty()) {
    html += "<p>Belum ada suara terdeteksi.</p>";
  } else {
    html += soundDetectionLog;
  }
  html += "</div>";
  html += "<div class='button-container fade-in'>";
  html += "<a href='/clearSoundLog' class='button clear-button'>Hapus Log Suara</a>";
  html += "</div>";
  html += R"rawliteral(
  <script>
    function updateLedStatus() {
      var xhr = new XMLHttpRequest();
      xhr.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          document.getElementById('ledStatusDisplay').innerHTML = this.responseText;
          var statusText = this.responseText;
          var ledStatusElement = document.getElementById('ledStatusDisplay');
          if (statusText.includes('MATI')) {
            ledStatusElement.style.color = '#ff6b6b';
          } else {
            ledStatusElement.style.color = '#4caf50';
          }
        }
      };
      xhr.open("GET", "/getLedStatus", true);
      xhr.send();
    }
    function updateBrightnessDisplay() {
      var xhr = new XMLHttpRequest();
      xhr.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          document.getElementById('brightnessDisplay').innerHTML = this.responseText;
          var outputElement = document.getElementById('brightnessOutput');
          if (outputElement) {
              outputElement.value = this.responseText;
          }
        }
      };
      xhr.open("GET", "/getBrightness", true);
      xhr.send();
    }
    function updateSoundLog() {
      var xhr = new XMLHttpRequest();
      xhr.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          document.getElementById('soundLogBox').innerHTML = this.responseText;
        }
      };
      xhr.open("GET", "/getSoundLog", true);
      xhr.send();
    }

    function checkIfWebUpdateActive() {
      var xhr = new XMLHttpRequest();
      xhr.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          var isActive = (this.responseText === '1');
          if (isActive) {
            if (!window.updateIntervalsSet) {
              setInterval(updateLedStatus, 1000);
              setInterval(updateBrightnessDisplay, 1000);
              setInterval(updateSoundLog, 5000);
              window.updateIntervalsSet = true;
              console.log("Update interval diaktifkan.");
            }
            updateLedStatus();
            updateBrightnessDisplay();
            updateSoundLog();
          } else {
            if (window.updateIntervalsSet) {
                console.log("Update interval dinonaktifkan (secara logis oleh ESP).");
                window.updateIntervalsSet = false;
            }
          }
        }
      };
      xhr.open("GET", "/getWebUpdateStatus", true);
      xhr.send();
    }

    setInterval(checkIfWebUpdateActive, 5000);
    checkIfWebUpdateActive();

  </script>
  )rawliteral";
  html += "<div class='footer fade-in'>";
  html += "<p>&copy; 2025 NodeMCU Kontrol. Dibuat dengan &hearts; untuk Pendidikan.</p>";
  html += "</div>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

// Handler untuk mendapatkan status update web (digunakan oleh JS)
void handleGetWebUpdateStatus() {
  server.send(200, "text/plain", webUpdateActive ? "1" : "0");
}

// Handler untuk mendapatkan status LED (untuk update AJAX)
void handleGetLedStatus() {
  if (!webUpdateActive) {
    server.send(200, "text/plain", "");
    return;
  }
  String statusText;
  if (currentLedMode == MODE_RAINBOW) {
    statusText = "PELANGI AKTIF";
  } else if (currentLedMode == MODE_SOLID_COLOR) {
    statusText = "WARNA SOLID";
  } else if (currentLedMode == MODE_FADE) {
    statusText = "FADE WARNA";
  } else if (currentLedMode == MODE_TWO_COLOR) {
    statusText = "DUA WARNA";
  } else if (currentLedMode == MODE_COMET_TRAIL) { // <--- NAMA MODE DIUBAH
    statusText = "KOMET";
  } else if (currentLedMode == MODE_METEOR_RAIN) {
    statusText = "HUJAN METEOR";
  } else if (currentLedMode == MODE_FIRE) {
    statusText = "EFEK API";
  } else if (currentLedMode == MODE_SPARKLE) {
    statusText = "KEDIP PERCIKAN";
  } else {
    statusText = "MATI";
  }
  server.send(200, "text/plain", statusText);
}

// Handler untuk mendapatkan nilai kecerahan (untuk update AJAX)
void handleGetBrightness() {
  if (!webUpdateActive) {
    server.send(200, "text/plain", "");
    return;
  }
  server.send(200, "text/plain", String(currentBrightness));
}

// Handler untuk mendapatkan log suara (untuk update AJAX)
void handleGetSoundLog() {
  if (!webUpdateActive) {
    server.send(200, "text/html", "");
    return;
  }
  String logToDisplay = soundDetectionLog;
  if (logToDisplay.isEmpty()) {
    logToDisplay = "<p>Belum ada suara terdeteksi.</p>";
  }
  server.send(200, "text/html", logToDisplay);
}

// Handler untuk menghapus log suara
void handleClearSoundLog() {
  soundDetectionLog = "";
  Serial.println("Log deteksi suara dihapus.");
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "");
}

// Handler untuk mengatur mode LED
void handleSetLedMode() {
  if (server.hasArg("mode")) {
    String mode = server.arg("mode");
    if (mode == "off") {
      if (currentLedMode != MODE_OFF) {
        lastActiveLedMode = currentLedMode;
      }
      currentLedMode = MODE_OFF;
      FastLED.clear();
      FastLED.show();
      Serial.println("LED Mode: OFF");
    } else if (mode == "rainbow") {
      currentLedMode = MODE_RAINBOW;
      Serial.println("LED Mode: Rainbow");
    } else if (mode == "solid") {
      currentLedMode = MODE_SOLID_COLOR;
      fill_solid(leds, NUM_LEDS, currentSolidColor);
      FastLED.show();
      Serial.println("LED Mode: Solid Color");
    } else if (mode == "fade") {
      currentLedMode = MODE_FADE;
      Serial.println("LED Mode: Fade");
    } else if (mode == "two_color") {
      currentLedMode = MODE_TWO_COLOR;
      lastTwoColorToggleTime = millis();
      Serial.println("LED Mode: Two Color");
    } else if (mode == "comet_trail") { // <--- NAMA MODE DIUBAH
      currentLedMode = MODE_COMET_TRAIL; // <--- MODE ENUM DIUBAH
      // cometPosition = 0; // Tidak perlu direset ke 0 saat ganti mode lagi
      Serial.println("LED Mode: Comet Trail"); // <--- TEKS DIUBAH
      FastLED.clear();
    } else if (mode == "meteor_rain") {
      currentLedMode = MODE_METEOR_RAIN;
      meteorRainPosition = 0;
      Serial.println("LED Mode: Meteor Rain (Jejak)");
      FastLED.clear();
    } else if (mode == "fire") {
      currentLedMode = MODE_FIRE;
      Serial.println("LED Mode: Fire");
      FastLED.clear();
    } else if (mode == "sparkle") {
      currentLedMode = MODE_SPARKLE;
      Serial.println("LED Mode: Sparkle");
      FastLED.clear();
    }
  }
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "");
}

// Handler untuk mengatur kecerahan LED
void handleSetBrightness() {
  if (server.hasArg("brightness")) {
    currentBrightness = server.arg("brightness").toInt();
    FastLED.setBrightness(currentBrightness);
    Serial.printf("Brightness set to: %d\n", currentBrightness);
  }
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "");
}

// Handler untuk mengatur warna solid
void handleSetSolidColor() {
  if (server.hasArg("color")) {
    String hexColor = server.arg("color");
    long number = (long) strtol( &hexColor[1], NULL, 16);
    currentSolidColor.r = (number >> 16) & 0xFF;
    currentSolidColor.g = (number >> 8) & 0xFF;
    currentSolidColor.b = number & 0xFF;
    if (currentLedMode == MODE_SOLID_COLOR) {
      fill_solid(leds, NUM_LEDS, currentSolidColor);
      FastLED.show();
    }
    Serial.printf("Solid Color set to: #%s (R:%d G:%d B:%d)\n", hexColor.c_str(), currentSolidColor.r, currentSolidColor.g, currentSolidColor.b);
  }
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "");
}

// Handler untuk mengatur kecepatan pelangi
void handleSetRainbowSpeed() {
  if (server.hasArg("speed")) {
    currentRainbowSpeed = server.arg("speed").toInt();
    if (currentRainbowSpeed < 1) currentRainbowSpeed = 1;
    if (currentRainbowSpeed > 50) currentRainbowSpeed = 50;
    Serial.printf("Rainbow Speed set to: %d\n", currentRainbowSpeed);
  }
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "");
}

// Handler untuk mengatur kecepatan fade
void handleSetFadeSpeed() {
  if (server.hasArg("speed")) {
    currentFadeSpeed = server.arg("speed").toInt();
    if (currentFadeSpeed < 1) currentFadeSpeed = 1;
    if (currentFadeSpeed > 200) currentFadeSpeed = 200;
    Serial.printf("Fade Speed set to: %d\n", currentFadeSpeed);
  }
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "");
}

// Handler untuk mengatur dua warna
void handleSetTwoColors() {
  if (server.hasArg("color1") && server.hasArg("color2")) {
    String hexColor1 = server.arg("color1");
    String hexColor2 = server.arg("color2");

    long number1 = (long) strtol( &hexColor1[1], NULL, 16);
    colorOne.r = (number1 >> 16) & 0xFF;
    colorOne.g = (number1 >> 8) & 0xFF;
    colorOne.b = number1 & 0xFF;

    long number2 = (long) strtol( &hexColor2[1], NULL, 16);
    colorTwo.r = (number2 >> 16) & 0xFF;
    colorTwo.g = (number2 >> 8) & 0xFF;
    colorTwo.b = number2 & 0xFF;

    Serial.printf("Two Colors set to: Color1 #%s, Color2 #%s\n", hexColor1.c_str(), hexColor2.c_str());
  }
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "");
}

// --- Handler untuk Mode Komet (menggantikan setMovingLed) ---
void handleSetCometTrail() {
  if (server.hasArg("speed")) {
    cometSpeed = server.arg("speed").toInt();
    if (cometSpeed < 10) cometSpeed = 10;
    if (cometSpeed > 500) cometSpeed = 500;
  }
  if (server.hasArg("color")) {
    String hexColor = server.arg("color");
    long number = (long) strtol( &hexColor[1], NULL, 16);
    cometColor.r = (number >> 16) & 0xFF;
    cometColor.g = (number >> 8) & 0xFF;
    cometColor.b = number & 0xFF;
  }
  Serial.printf("Set Comet Trail: Speed: %d, Color: R:%d G:%d B:%d\n",
                cometSpeed, cometColor.r, cometColor.g, cometColor.b);
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "");
}


// --- Handler untuk Mode Meteor Rain ---
void handleSetMeteorRain() {
  if (server.hasArg("speed")) {
    meteorRainSpeed = server.arg("speed").toInt();
    if (meteorRainSpeed < 10) meteorRainSpeed = 10;
    if (meteorRainSpeed > 500) meteorRainSpeed = 500;
  }
  if (server.hasArg("length")) {
    meteorRainTrailLength = server.arg("length").toInt();
    if (meteorRainTrailLength < 1) meteorRainTrailLength = 1;
    if (meteorRainTrailLength > NUM_LEDS) meteorRainTrailLength = NUM_LEDS;
  }
  if (server.hasArg("color")) {
    String hexColor = server.arg("color");
    long number = (long) strtol( &hexColor[1], NULL, 16);
    meteorRainColor.r = (number >> 16) & 0xFF;
    meteorRainColor.g = (number >> 8) & 0xFF;
    meteorRainColor.b = number & 0xFF;
  }
  Serial.printf("Set Meteor Rain: Speed: %d, Trail Length: %d, Color: R:%d G:%d B:%d\n",
                meteorRainSpeed, meteorRainTrailLength, meteorRainColor.r, meteorRainColor.g, meteorRainColor.b);
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "");
}

// --- Handler untuk Mode Fire ---
void handleSetFire() {
  if (server.hasArg("speed")) {
    fireSpeed = server.arg("speed").toInt();
    if (fireSpeed < 1) fireSpeed = 1;
    if (fireSpeed > 100) fireSpeed = 100;
  }
  if (server.hasArg("cooling")) {
    fireCooling = server.arg("cooling").toInt();
    if (fireCooling < 10) fireCooling = 10;
    if (fireCooling > 100) fireCooling = 100;
  }
  if (server.hasArg("sparking")) {
    fireSparking = server.arg("sparking").toInt();
    if (fireSparking < 50) fireSparking = 50;
    if (fireSparking > 250) fireSparking = 250;
  }
  Serial.printf("Set Fire: Speed: %d, Cooling: %d, Sparking: %d\n",
                fireSpeed, fireCooling, fireSparking);
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "");
}

// --- Handler untuk Mode Sparkle ---
void handleSetSparkle() {
  if (server.hasArg("chance")) {
    sparkleChance = server.arg("chance").toInt();
    if (sparkleChance < 1) sparkleChance = 1;
    if (sparkleChance > 100) sparkleChance = 100;
  }
  if (server.hasArg("fade")) {
    sparkleFadeSpeed = server.arg("fade").toInt();
    if (sparkleFadeSpeed < 1) sparkleFadeSpeed = 1;
    if (sparkleFadeSpeed > 50) sparkleFadeSpeed = 50;
  }
  if (server.hasArg("color")) {
    String hexColor = server.arg("color");
    long number = (long) strtol( &hexColor[1], NULL, 16);
    sparkleColor.r = (number >> 16) & 0xFF;
    sparkleColor.g = (number >> 8) & 0xFF;
    sparkleColor.b = number & 0xFF;
  }
  Serial.printf("Set Sparkle: Chance: %d, Fade Speed: %d, Color: R:%d G:%d B:%d\n",
                sparkleChance, sparkleFadeSpeed, sparkleColor.r, sparkleColor.g, sparkleColor.b);
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "");
}


// --- Fungsi Setup NodeMCU (Jalankan sekali saat boot) ---
void setup() {
  Serial.begin(115200);
  Serial.println("\nSound Detector & LED Control Started!");

  if (!SPIFFS.begin()) {
    Serial.println("Failed to initialize SPIFFS! (If not using, ignore)");
  } else {
    Serial.println("SPIFFS successfully initialized.");
  }

  // Konfigurasi LED strip
  FastLED.addLeds<LED_TYPE, LED_DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(currentBrightness);
  FastLED.clear();
  FastLED.show();

  // Konfigurasi pin sensor suara
  pinMode(SENSOR_SUARA_DO_PIN, INPUT_PULLUP);

  // Konfigurasi NodeMCU sebagai Access Point (Hotspot Wi-Fi)
  Serial.print("Starting NodeMCU as Access Point...");
  WiFi.softAP(ssid, password);
  Serial.print("Access Point Created with SSID: ");
  Serial.println(ssid);
  Serial.print("Hotspot IP Address: ");
  Serial.println(WiFi.softAPIP());

  // Daftarkan handler untuk setiap URL di web server
  server.on("/", handleRoot);
  server.on("/getLedStatus", handleGetLedStatus);
  server.on("/getBrightness", handleGetBrightness);
  server.on("/getSoundLog", handleGetSoundLog);
  server.on("/clearSoundLog", handleClearSoundLog);
  server.on("/setLedMode", handleSetLedMode);
  server.on("/setBrightness", handleSetBrightness);
  server.on("/setSolidColor", handleSetSolidColor);
  server.on("/setRainbowSpeed", handleSetRainbowSpeed);
  server.on("/setFadeSpeed", handleSetFadeSpeed);
  server.on("/setTwoColors", handleSetTwoColors);
  server.on("/setCometTrail", handleSetCometTrail); // <--- HANDLER BARU
  server.on("/getWebUpdateStatus", handleGetWebUpdateStatus);
  server.on("/setMeteorRain", handleSetMeteorRain);
  server.on("/setFire", handleSetFire);
  server.on("/setSparkle", handleSetSparkle);

  // Mulai server HTTP
  server.begin();
  Serial.println("HTTP server started");
}

// --- Fungsi Loop NodeMCU (Jalankan berulang kali) ---
void loop() {
  checkClientConnections();

  int currentSoundState = digitalRead(SENSOR_SUARA_DO_PIN);
  if (currentSoundState == LOW && lastSoundState == HIGH) {
    if (millis() - lastSoundTime > DEBOUNCE_DELAY) {
      Serial.println("Tepuk tangan terdeteksi!");
      if (currentLedMode == MODE_OFF) {
        currentLedMode = lastActiveLedMode;
        Serial.print("LED ON: Mengaktifkan mode sebelumnya: ");
        if(currentLedMode == MODE_RAINBOW) Serial.println("Pelangi.");
        else if(currentLedMode == MODE_SOLID_COLOR) {
          Serial.println("Warna Solid.");
          fill_solid(leds, NUM_LEDS, currentSolidColor);
          FastLED.show();
        }
        else if(currentLedMode == MODE_FADE) Serial.println("Fade Warna.");
        else if(currentLedMode == MODE_TWO_COLOR) Serial.println("Dua Warna.");
        else if(currentLedMode == MODE_COMET_TRAIL) Serial.println("Komet."); // <--- TEKS DIUBAH
        else if(currentLedMode == MODE_METEOR_RAIN) Serial.println("Hujan Meteor.");
        else if(currentLedMode == MODE_FIRE) Serial.println("Api.");
        else if(currentLedMode == MODE_SPARKLE) Serial.println("Percikan.");
      } else {
        lastActiveLedMode = currentLedMode;
        currentLedMode = MODE_OFF;
        FastLED.clear();
        FastLED.show();
        Serial.println("LED OFF: Mematikan LED.");
      }

      String newLogEntry = "<div class='log-entry'>" + getCurrentTime() + " - Tepuk Tangan Terdeteksi! LED ";
      newLogEntry += (currentLedMode == MODE_OFF) ? "MATI" : "NYALA";
      newLogEntry += "</div>";
      soundDetectionLog = newLogEntry + soundDetectionLog;

      int maxLogEntries = 20;
      int divCount = 0;
      int lastDivStart = -1;
      for (int i = 0; i < soundDetectionLog.length(); i++) {
          if (soundDetectionLog.substring(i, i+4) == "<div") {
              divCount++;
              if (divCount > maxLogEntries) {
                  lastDivStart = i;
                  break;
              }
          }
      }
      if (lastDivStart != -1) {
          soundDetectionLog = soundDetectionLog.substring(0, lastDivStart);
      }

      lastSoundTime = millis();
    }
  }
  lastSoundState = currentSoundState;

  // Jalankan efek LED sesuai mode yang aktif
  switch (currentLedMode) {
    case MODE_RAINBOW:
      rainbow();
      break;
    case MODE_SOLID_COLOR:
      break;
    case MODE_FADE:
      EVERY_N_MILLISECONDS(currentFadeSpeed) {
        fill_solid(leds, NUM_LEDS, CHSV(fadeHue++, 255, 255));
        FastLED.show();
      }
      break;
    case MODE_TWO_COLOR:
      twoColor();
      break;
    case MODE_COMET_TRAIL: // <--- NAMA MODE DIUBAH
      cometTrail();        // <--- FUNGSI DIUBAH
      break;
    case MODE_METEOR_RAIN:
      meteorRain();
      break;
    case MODE_FIRE:
      fire();
      break;
    case MODE_SPARKLE:
      sparkle();
      break;
    case MODE_OFF:
      break;
  }

  server.handleClient();
  yield();
}
