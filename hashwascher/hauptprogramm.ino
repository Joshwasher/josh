// ========================================
// HASHWASCHER - HAUPTPROGRAMM
// ESP32 + Elecrow 7" Display + NEMA Stepper
// Mit Eingabefeldern für alle Zahleneingaben
// ========================================

#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <AccelStepper.h>
#include "konfiguration.h"
#include "rezepte.h"

// ========================================
// GLOBALE OBJEKTE
// ========================================

TFT_eSPI tft = TFT_eSPI();
XPT2046_Touchscreen ts(TOUCH_CS, TOUCH_IRQ);
AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);

// ========================================
// GLOBALE VARIABLEN
// ========================================

uint16_t waschzeit = 600;        // Sekunden (10 Min default)
uint16_t intervallzeit = 30;     // Sekunden
uint16_t pausenzeit = 15;        // Sekunden
uint16_t drehzahl = 200;         // UPM

bool running = false;
bool paused = false;
uint32_t startzeit = 0;
uint32_t pausezeit_start = 0;
uint32_t verbrauchte_zeit = 0;

int currentPage = 0;             // 0=Hauptseite, 1=Rezepte, 2=Einstellungen
int selectedRezept = -1;         // Welches Rezept ist aktiv?

// Eingabefeld Struktur
struct InputField {
  int x, y, width, height;
  uint16_t value;
  char label[30];
  bool active;
  int maxLength;
};

// Eingabefelder für Hauptseite
InputField waschzeit_field = {50, 100, 150, 40, 10, "Waschzeit (Min)", false, 3};
InputField intervall_field = {50, 160, 150, 40, 30, "Intervall (Sek)", false, 3};
InputField pause_field = {50, 220, 150, 40, 15, "Pause (Sek)", false, 3};
InputField drehzahl_field = {50, 280, 150, 40, 200, "Drehzahl (UPM)", false, 3};

// ========================================
// SETUP
// ========================================

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n========== HASHWASCHER BOOT ==========");
  
  // TFT Initialize
  tft.init();
  tft.setRotation(1);  // 1024x600 Landscape
  tft.fillScreen(TFT_BLACK);
  
  // Touch Initialize
  ts.begin();
  ts.setRotation(1);
  
  // Stepper Initialize
  stepper.setMaxSpeed(drehzahl * 1.667);
  stepper.setAcceleration(500);
  pinMode(ENABLE_PIN, OUTPUT);
  digitalWrite(ENABLE_PIN, HIGH);  // Stepper aus
  
  // Logo Animation
  showLogo();
  
  // Erste Seite
  drawMainPage();
  
  Serial.println("========== READY ==========\n");
}

// ========================================
// HAUPTSCHLEIFE
// ========================================

void loop() {
  handleTouch();
  updateMotor();
  updateDisplay();
}

// ========================================
// DISPLAY FUNKTIONEN
// ========================================

void showLogo() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(3);
  tft.setTextColor(TFT_CYAN);
  tft.drawString("HASHWASCHER", 512, 250);
  tft.setTextSize(2);
  tft.setTextColor(TFT_MAGENTA);
  tft.drawString("ESP32 Edition", 512, 320);
  delay(5000);
  tft.fillScreen(TFT_BLACK);
}

void drawMainPage() {
  tft.fillScreen(TFT_DARK_GREY);
  
  // Header
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(3);
  tft.setTextColor(TFT_CYAN);
  tft.drawString("HAUPTSEITE", 512, 30);
  
  // Eingabefelder zeichnen
  drawInputField(waschzeit_field, waschzeit);
  drawInputField(intervall_field, intervallzeit);
  drawInputField(pause_field, pausenzeit);
  drawInputField(drehzahl_field, drehzahl);
  
  // Restzeit anzeigen
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);
  tft.drawString("Restzeit:", 100, 340);
  drawTimeDisplay(waschzeit);
  
  // Buttons
  drawButton(100, 450, 200, 50, "START", TFT_GREEN);
  drawButton(350, 450, 200, 50, "PAUSE", TFT_ORANGE);
  drawButton(600, 450, 200, 50, "STOP", TFT_RED);
  
  drawButton(100, 530, 200, 50, "Rezepte", TFT_CYAN);
  drawButton(350, 530, 200, 50, "Settings", TFT_MAGENTA);
}

void drawRecipesPage() {
  tft.fillScreen(TFT_DARK_GREY);
  
  // Header
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(3);
  tft.setTextColor(TFT_CYAN);
  tft.drawString("REZEPTE", 512, 30);
  
  // Rezepte auflisten
  int y_pos = 80;
  for (int i = 0; i < 10; i++) {
    tft.setTextSize(2);
    tft.setTextColor(selectedRezept == i ? TFT_YELLOW : TFT_WHITE);
    
    char rezeptText[80];
    sprintf(rezeptText, "%d. %s (%dMin, %dUPM)", 
            i+1, rezepte[i].name, rezepte[i].waschzeit, rezepte[i].drehzahl);
    
    tft.drawString(rezeptText, 50, y_pos);
    y_pos += 50;
  }
  
  // Buttons
  drawButton(100, 550, 150, 40, "LADEN", TFT_GREEN);
  drawButton(350, 550, 150, 40, "EDIT", TFT_CYAN);
  drawButton(600, 550, 150, 40, "ZURÜCK", TFT_RED);
}

void drawSettingsPage() {
  tft.fillScreen(TFT_DARK_GREY);
  
  // Header
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(3);
  tft.setTextColor(TFT_CYAN);
  tft.drawString("EINSTELLUNGEN", 512, 30);
  
  // Helligkeit Slider
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);
  tft.drawString("Helligkeit:", 100, 150);
  drawSlider(100, 200, 300, 20, 80);  // 80% Helligkeit
  
  // Lautstärke Slider
  tft.drawString("Lautstärke:", 100, 300);
  drawSlider(100, 350, 300, 20, 60);  // 60% Lautstärke
  
  // Sprache
  tft.drawString("Sprache:", 100, 450);
  drawButton(300, 440, 150, 40, "DEUTSCH", TFT_CYAN);
  drawButton(550, 440, 150, 40, "ENGLISH", TFT_ORANGE);
  
  // Zurück Button
  drawButton(400, 550, 200, 40, "ZURÜCK", TFT_RED);
}

void drawInputField(InputField &field, uint16_t value) {
  // Rahmen
  uint16_t color = field.active ? TFT_YELLOW : TFT_CYAN;
  tft.drawRect(field.x, field.y, field.width, field.height, color);
  
  // Label
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE);
  tft.drawString(field.label, field.x, field.y - 20);
  
  // Wert anzeigen
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);
  char valueStr[10];
  sprintf(valueStr, "%d", value);
  tft.drawString(valueStr, field.x + field.width/2, field.y + field.height/2);
}

void drawButton(int x, int y, int w, int h, const char* label, uint16_t color) {
  tft.fillRect(x, y, w, h, color);
  tft.drawRect(x, y, w, h, TFT_WHITE);
  
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(2);
  tft.setTextColor(TFT_BLACK);
  tft.drawString(label, x + w/2, y + h/2);
}

void drawSlider(int x, int y, int w, int h, int percent) {
  // Rahmen
  tft.drawRect(x, y, w, h, TFT_WHITE);
  
  // Gefüllter Teil
  int filledWidth = (w * percent) / 100;
  tft.fillRect(x, y, filledWidth, h, TFT_GREEN);
  
  // Prozent Text
  char percentStr[10];
  sprintf(percentStr, "%d%%", percent);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE);
  tft.drawString(percentStr, x + w + 20, y);
}

void drawTimeDisplay(uint16_t seconds) {
  int hours = seconds / 3600;
  int minutes = (seconds % 3600) / 60;
  int secs = seconds % 60;
  
  char timeStr[20];
  sprintf(timeStr, "%02d:%02d:%02d", hours, minutes, secs);
  
  tft.setTextSize(3);
  tft.setTextColor(TFT_YELLOW);
  tft.drawString(timeStr, 400, 360);
}

void updateDisplay() {
  if (running) {
    uint32_t elapsed = millis() - startzeit;
    if (paused) elapsed -= (millis() - pausezeit_start);
    
    uint16_t remaining = (waschzeit * 1000 - elapsed) / 1000;
    
    // Update Restzeit wenn sich geändert
    static uint16_t lastRemaining = 0;
    if (remaining != lastRemaining) {
      tft.fillRect(350, 320, 200, 60, TFT_DARK_GREY);
      drawTimeDisplay(remaining);
      lastRemaining = remaining;
      
      if (remaining == 0) {
        stopWashing();
      }
    }
  }
}

// ========================================
// TOUCH / INPUT HANDLING
// ========================================

void handleTouch() {
  if (!ts.touched()) return;
  
  TS_Point p = ts.getPoint();
  
  // Touch-Koordinaten normalisieren
  int touch_x = p.x;
  int touch_y = p.y;
  
  Serial.print("Touch: ");
  Serial.print(touch_x);
  Serial.print(", ");
  Serial.println(touch_y);
  
  // Seite spezifische Handler
  if (currentPage == 0) {
    handleMainPageTouch(touch_x, touch_y);
  } else if (currentPage == 1) {
    handleRecipesPageTouch(touch_x, touch_y);
  } else if (currentPage == 2) {
    handleSettingsPageTouch(touch_x, touch_y);
  }
  
  delay(200);  // Debounce
}

void handleMainPageTouch(int x, int y) {
  // Eingabefelder prüfen
  if (isInRect(x, y, waschzeit_field.x, waschzeit_field.y, waschzeit_field.width, waschzeit_field.height)) {
    numpadInput(waschzeit, "Waschzeit (Min)");
    drawMainPage();
    return;
  }
  
  if (isInRect(x, y, intervall_field.x, intervall_field.y, intervall_field.width, intervall_field.height)) {
    numpadInput(intervallzeit, "Intervall (Sek)");
    drawMainPage();
    return;
  }
  
  if (isInRect(x, y, pause_field.x, pause_field.y, pause_field.width, pause_field.height)) {
    numpadInput(pausenzeit, "Pause (Sek)");
    drawMainPage();
    return;
  }
  
  if (isInRect(x, y, drehzahl_field.x, drehzahl_field.y, drehzahl_field.width, drehzahl_field.height)) {
    numpadInput(drehzahl, "Drehzahl (UPM)");
    drawMainPage();
    return;
  }
  
  // START Button
  if (isInRect(x, y, 100, 450, 200, 50)) {
    startWashing();
    return;
  }
  
  // PAUSE Button
  if (isInRect(x, y, 350, 450, 200, 50)) {
    togglePause();
    return;
  }
  
  // STOP Button
  if (isInRect(x, y, 600, 450, 200, 50)) {
    stopWashing();
    return;
  }
  
  // Rezepte Button
  if (isInRect(x, y, 100, 530, 200, 50)) {
    currentPage = 1;
    drawRecipesPage();
    return;
  }
  
  // Settings Button
  if (isInRect(x, y, 350, 530, 200, 50)) {
    currentPage = 2;
    drawSettingsPage();
    return;
  }
}

void handleRecipesPageTouch(int x, int y) {
  // Rezepte selektieren (y: 80-580)
  int clickedRezept = (y - 80) / 50;
  if (clickedRezept >= 0 && clickedRezept < 10) {
    selectedRezept = clickedRezept;
    drawRecipesPage();
    return;
  }
  
  // LADEN Button
  if (isInRect(x, y, 100, 550, 150, 40) && selectedRezept >= 0) {
    ladenRezept(selectedRezept);
    currentPage = 0;
    drawMainPage();
    return;
  }
  
  // EDIT Button
  if (isInRect(x, y, 350, 550, 150, 40) && selectedRezept >= 0) {
    editRezept(selectedRezept);
    drawRecipesPage();
    return;
  }
  
  // ZURÜCK Button
  if (isInRect(x, y, 600, 550, 150, 40)) {
    currentPage = 0;
    drawMainPage();
    return;
  }
}

void handleSettingsPageTouch(int x, int y) {
  // DEUTSCH Button
  if (isInRect(x, y, 300, 440, 150, 40)) {
    Serial.println("Sprache: Deutsch");
    return;
  }
  
  // ENGLISH Button
  if (isInRect(x, y, 550, 440, 150, 40)) {
    Serial.println("Language: English");
    return;
  }
  
  // ZURÜCK Button
  if (isInRect(x, y, 400, 550, 200, 40)) {
    currentPage = 0;
    drawMainPage();
    return;
  }
}

bool isInRect(int x, int y, int rx, int ry, int rw, int rh) {
  return (x >= rx && x <= rx + rw && y >= ry && y <= ry + rh);
}

// ========================================
// NUMMERNPAD / TEXTINPUT
// ========================================

void numpadInput(uint16_t &value, const char* label) {
  tft.fillScreen(TFT_DARK_GREY);
  
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(3);
  tft.setTextColor(TFT_CYAN);
  tft.drawString(label, 512, 50);
  
  // Eingabefeld oben
  tft.fillRect(200, 150, 600, 80, TFT_BLACK);
  tft.drawRect(200, 150, 600, 80, TFT_CYAN);
  
  char inputStr[20] = "";
  int inputLen = 0;
  
  // Nummernpad zeichnen
  drawNumpad();
  
  bool inputFinished = false;
  
  while (!inputFinished) {
    // Eingabefeld Text anzeigen
    tft.fillRect(210, 160, 580, 60, TFT_BLACK);
    tft.setTextSize(4);
    tft.setTextColor(TFT_WHITE);
    tft.drawString(inputStr, 500, 190);
    
    if (!ts.touched()) continue;
    
    TS_Point p = ts.getPoint();
    int x = p.x;
    int y = p.y;
    
    // Nummernpad prüfen (0-9)
    for (int i = 0; i < 10; i++) {
      int btnX = 150 + (i % 5) * 150;
      int btnY = 300 + (i / 5) * 100;
      
      if (isInRect(x, y, btnX, btnY, 130, 80)) {
        inputStr[inputLen++] = '0' + i;
        inputStr[inputLen] = '\0';
        Serial.print("Eingabe: ");
        Serial.println(inputStr);
      }
    }
    
    // Delete Button
    if (isInRect(x, y, 150 + 5*150, 300, 130, 80) && inputLen > 0) {
      inputStr[--inputLen] = '\0';
    }
    
    // OK Button
    if (isInRect(x, y, 350, 520, 300, 60)) {
      if (inputLen > 0) {
        value = atoi(inputStr);
        inputFinished = true;
        Serial.print("Wert gespeichert: ");
        Serial.println(value);
      }
    }
    
    delay(200);
  }
}

void drawNumpad() {
  // Nummernpad 0-9
  for (int i = 0; i < 10; i++) {
    int x = 150 + (i % 5) * 150;
    int y = 300 + (i / 5) * 100;
    
    drawButton(x, y, 130, 80, String(i).c_str(), TFT_BLUE);
  }
  
  // Delete Button
  drawButton(150 + 5*150, 300, 130, 80, "DEL", TFT_RED);
  
  // OK Button
  drawButton(350, 520, 300, 60, "OK", TFT_GREEN);
}

void editRezept(int platz) {
  tft.fillScreen(TFT_DARK_GREY);
  
  tft.setTextSize(3);
  tft.setTextColor(TFT_CYAN);
  tft.drawString("REZEPT EDITIEREN", 512, 30);
  
  // Name Input
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);
  tft.drawString("Name:", 50, 120);
  
  // Werte Input
  tft.drawString("Waschzeit (Min):", 50, 180);
  InputField edit_waschzeit = {400, 165, 150, 40, rezepte[platz].waschzeit, "", false, 3};
  drawInputField(edit_waschzeit, rezepte[platz].waschzeit);
  
  tft.drawString("Intervall (Sek):", 50, 240);
  InputField edit_intervall = {400, 225, 150, 40, rezepte[platz].intervallzeit, "", false, 3};
  drawInputField(edit_intervall, rezepte[platz].intervallzeit);
  
  tft.drawString("Pause (Sek):", 50, 300);
  InputField edit_pause = {400, 285, 150, 40, rezepte[platz].pausenzeit, "", false, 3};
  drawInputField(edit_pause, rezepte[platz].pausenzeit);
  
  tft.drawString("Drehzahl (UPM):", 50, 360);
  InputField edit_drehzahl = {400, 345, 150, 40, rezepte[platz].drehzahl, "", false, 3};
  drawInputField(edit_drehzahl, rezepte[platz].drehzahl);
  
  // Buttons
  drawButton(150, 450, 200, 50, "SPEICHERN", TFT_GREEN);
  drawButton(550, 450, 200, 50, "ZURÜCK", TFT_RED);
  
  bool editFinished = false;
  
  while (!editFinished) {
    if (!ts.touched()) continue;
    
    TS_Point p = ts.getPoint();
    
    // SPEICHERN
    if (isInRect(p.x, p.y, 150, 450, 200, 50)) {
      speichernRezept(platz, rezepte[platz].name, 
                     rezepte[platz].waschzeit,
                     rezepte[platz].intervallzeit,
                     rezepte[platz].pausenzeit,
                     rezepte[platz].drehzahl);
      editFinished = true;
    }
    
    // ZURÜCK
    if (isInRect(p.x, p.y, 550, 450, 200, 50)) {
      editFinished = true;
    }
    
    delay(200);
  }
}

// ========================================
// MOTOR FUNKTIONEN
// ========================================

void startWashing() {
  if (running) return;
  
  running = true;
  paused = false;
  startzeit = millis();
  
  digitalWrite(ENABLE_PIN, LOW);  // Motor an
  
  Serial.println("=== WASCHEN GESTARTET ===");
  Serial.print("Waschzeit: ");
  Serial.print(waschzeit);
  Serial.println(" Sekunden");
}

void togglePause() {
  if (!running) return;
  
  if (!paused) {
    paused = true;
    pausezeit_start = millis();
    digitalWrite(ENABLE_PIN, HIGH);  // Motor aus
    Serial.println("PAUSE");
  } else {
    paused = false;
    startzeit += (millis() - pausezeit_start);  // Zeit kompensieren
    digitalWrite(ENABLE_PIN, LOW);   // Motor an
    Serial.println("FORTGESETZT");
  }
}

void stopWashing() {
  running = false;
  paused = false;
  digitalWrite(ENABLE_PIN, HIGH);  // Motor aus
  stepper.stop();
  
  Serial.println("=== STOPP ===");
  drawMainPage();
}

void updateMotor() {
  if (!running || paused) {
    stepper.run();
    return;
  }
  
  // Intervall/Pause Zyklus
  static uint32_t cycleStart = 0;
  static bool inIntervall = true;
  
  if (cycleStart == 0) cycleStart = millis();
  
  uint32_t cycleTime = millis() - cycleStart;
  uint32_t cycleDuration = (intervallzeit + pausenzeit) * 1000;
  
  if (cycleTime > cycleDuration) {
    cycleStart = millis();
    inIntervall = true;
  }
  
  if (inIntervall && cycleTime > intervallzeit * 1000) {
    inIntervall = false;
  }
  
  if (inIntervall) {
    // Motor dreht
    stepper.setMaxSpeed(drehzahl * 1.667);
    stepper.moveTo(stepper.currentPosition() + 200);  // 200 Steps
  } else {
    // Motor pausiert
    stepper.setMaxSpeed(0);
  }
  
  stepper.run();
}

// ========================================
// SETUP COMPLETE
// ========================================
