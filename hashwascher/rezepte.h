// ========================================
// HASHWASCHER - REZEPTE VERWALTUNG
// 10 Freie Plätze zur Bearbeitung
// ========================================

#ifndef REZEPTE_H
#define REZEPTE_H

// Rezept Struktur
struct Rezept {
  char name[20];           // Rezept Name (Max. 19 Zeichen)
  uint16_t waschzeit;      // Gesamtzeit in MINUTEN
  uint16_t intervallzeit;  // Drehzeit in SEKUNDEN
  uint16_t pausenzeit;     // Pausezeit in SEKUNDEN
  uint16_t drehzahl;       // Motor UPM (Umdrehungen/Minute)
};

// Array mit 10 Rezept-Plätzen (alle zur freien Bearbeitung)
Rezept rezepte[10] = {
  
  // ===== REZEPT 1 =====
  {
    "Rezept 1",      // Name (z.B. "Standard", "Schnell", etc.)
    10,              // Waschzeit in Minuten (z.B. 10 Min)
    30,              // Intervallzeit in Sekunden (z.B. 30 Sek Drehung)
    15,              // Pausezeit in Sekunden (z.B. 15 Sek Pause)
    200              // Drehzahl in UPM (z.B. 200 Umdrehungen/Min)
  },

  // ===== REZEPT 2 =====
  {
    "Rezept 2",      
    5,               // 5 Minuten Schnellprogramm
    20,              
    10,              
    150              // Niedrigere Drehzahl
  },

  // ===== REZEPT 3 =====
  {
    "Rezept 3",      
    30,              // 30 Minuten Langzeitprogramm
    45,              // Längere Drehzeiten
    20,              
    250              // Höhere Drehzahl
  },

  // ===== REZEPT 4 =====
  {
    "Rezept 4",      
    15,              
    35,              
    18,              
    180              
  },

  // ===== REZEPT 5 =====
  {
    "Rezept 5",      
    20,              
    40,              
    22,              
    220              
  },

  // ===== REZEPT 6 =====
  {
    "Rezept 6",      
    8,               
    25,              
    12,              
    160              
  },

  // ===== REZEPT 7 =====
  {
    "Rezept 7",      
    12,              
    38,              
    16,              
    190              
  },

  // ===== REZEPT 8 =====
  {
    "Rezept 8",      
    25,              
    42,              
    21,              
    240              
  },

  // ===== REZEPT 9 =====
  {
    "Rezept 9",      
    18,              
    33,              
    19,              
    175              
  },

  // ===== REZEPT 10 =====
  {
    "Rezept 10",     
    22,              
    36,              
    17,              
    210              
  }
};

// ========================================
// FUNKTION: Rezept laden
// ========================================
void ladenRezept(int platz) {
  if (platz < 0 || platz >= 10) return; // Sicherheit
  
  // Werte in Hauptprogramm kopieren
  waschzeit = rezepte[platz].waschzeit * 60;      // Min zu Sekunden
  intervallzeit = rezepte[platz].intervallzeit;
  pausenzeit = rezepte[platz].pausenzeit;
  drehzahl = rezepte[platz].drehzahl;
  
  // Motor Geschwindigkeit einstellen
  stepper.setMaxSpeed(drehzahl * 1.667);  // UPM zu Steps/Sec
  
  Serial.print("Rezept geladen: ");
  Serial.println(rezepte[platz].name);
}

// ========================================
// FUNKTION: Rezept speichern
// ========================================
void speichernRezept(int platz, char* name, uint16_t wz, uint16_t iz, uint16_t pz, uint16_t dz) {
  if (platz < 0 || platz >= 10) return; // Sicherheit
  
  strncpy(rezepte[platz].name, name, 19);
  rezepte[platz].waschzeit = wz;
  rezepte[platz].intervallzeit = iz;
  rezepte[platz].pausenzeit = pz;
  rezepte[platz].drehzahl = dz;
  
  // Optional: In EEPROM speichern für Persistierung
  // (Könnte implementiert werden)
  
  Serial.print("Rezept gespeichert: ");
  Serial.println(name);
}

// ========================================
// FUNKTION: Alle Rezepte auflisten
// ========================================
void listaalleRezepte() {
  Serial.println("\n=== ALLE REZEPTE ===");
  for (int i = 0; i < 10; i++) {
    Serial.print("Platz ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(rezepte[i].name);
    Serial.print(" | ");
    Serial.print(rezepte[i].waschzeit);
    Serial.print(" Min | ");
    Serial.print(rezepte[i].intervallzeit);
    Serial.print(" Sek Drehung | ");
    Serial.print(rezepte[i].pausenzeit);
    Serial.print(" Sek Pause | ");
    Serial.print(rezepte[i].drehzahl);
    Serial.println(" UPM");
  }
  Serial.println("=====================\n");
}

#endif // REZEPTE_H

/* ========================================
   ANLEITUNG ZUM BEARBEITEN:
   
   1. Rezept-Name ändern:
      "Rezept 1" → z.B. "Feinwäsche", "Standard", "Intensiv"
   
   2. Waschzeit (Minuten):
      5-60 Minuten empfohlen
      Bsp: 10 = 10 Minuten
   
   3. Intervallzeit (Sekunden):
      20-60 Sekunden empfohlen
      = wie lange der Motor pro Zyklus dreht
   
   4. Pausezeit (Sekunden):
      10-30 Sekunden empfohlen
      = wie lange Motor pausiert vor Richtungswechsel
   
   5. Drehzahl (UPM):
      100-300 UPM empfohlen
      (Umdrehungen pro Minute)
   
   BEISPIELE:
   
   Schnellprogramm: 5 Min, 20 Sek, 10 Sek, 150 UPM
   Standard:       10 Min, 30 Sek, 15 Sek, 200 UPM
   Langzeit:       30 Min, 45 Sek, 20 Sek, 250 UPM
   
   ======================================== */
