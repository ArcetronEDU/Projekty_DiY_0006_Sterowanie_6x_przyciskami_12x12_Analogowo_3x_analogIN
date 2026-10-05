#include <Arduino.h>

const float voltagePerStep = 5.0 / 1023.0; // Napięcie na krok konwersji ADC

// Napięcia wzorcowe dla dzielnika (6.8k + 330R + 330R z podciągnięciem 10k do GND)
const float vButton1 = 2.919; // Przycisk nieparzysty (za 1-szym 330R) -> ~597 ADC
const float vButton2 = 2.864; // Przycisk parzysty    (za 2-gim 330R) -> ~586 ADC
const float voltageTolerance = 0.020; // Tolerancja ±0.020 V (~4 kroki ADC)

const int channelCount = 3;
const int analogPins[channelCount] = {A1, A2, A3};

// Nazwy funkcji przypisane do przycisków (1-6)
const char* buttonNames[6] = {
  "LEWO",   // Przycisk 1 (A1)
  "PRAWO",  // Przycisk 2 (A1)
  "GÓRA",   // Przycisk 3 (A2)
  "DÓŁ",    // Przycisk 4 (A2)
  "STRZAŁ", // Przycisk 5 (A3)
  "WYBÓR"   // Przycisk 6 (A3)
};

// Tablica przechowująca aktualny stan chwilowy dla każdego z 6 przycisków
bool buttonState[6] = {false, false, false, false, false, false};

void setup() {
  Serial.begin(9600);
  Serial.println("==================================================");
  Serial.println("  Kontroler Gier - Tryb Chwilowy (Real-Time)      ");
  Serial.println("==================================================");
}

void loop() {
  // 1. Zresetuj stany przycisków przed nowym odczytem
  for (int i = 0; i < 6; i++) {
    buttonState[i] = false;
  }

  // 2. Odczytaj niezależnie każdy z 3 kanałów analogowych
  for (int ch = 0; ch < channelCount; ch++) {
    int sensorValue = analogRead(analogPins[ch]);
    float voltage = sensorValue * voltagePerStep;

    int btn1Index = ch * 2;     // Przycisk nieparzysty: 0 (A1), 2 (A2), 4 (A3)
    int btn2Index = (ch * 2) + 1; // Przycisk parzysty:    1 (A1), 3 (A2), 5 (A3)

    // Detekcja przycisku nieparzystego na danym kanale
    if (voltage >= (vButton1 - voltageTolerance) && voltage <= (vButton1 + voltageTolerance)) {
      buttonState[btn1Index] = true;
    } 
    // Detekcja przycisku parzystego na danym kanale
    else if (voltage >= (vButton2 - voltageTolerance) && voltage <= (vButton2 + voltageTolerance)) {
      buttonState[btn2Index] = true;
    }
  }

  // 3. Wypisz aktywne przyciski (stan chwilowy)
  bool anyButtonPressed = false;
  
  for (int i = 0; i < 6; i++) {
    if (buttonState[i]) {
      if (!anyButtonPressed) {
        Serial.print("AKTYWNE: ");
        anyButtonPressed = true;
      }
      Serial.print("[");
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(buttonNames[i]);
      Serial.print("] ");
    }
  }

  if (anyButtonPressed) {
    Serial.println(); // Nowa linia po wypisaniu wszystkich wciśniętych w tej pętli
  }

  delay(20); // Krótkie opóźnienie dla stabilizacji odczytów
}

