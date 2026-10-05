/*
 * =================================================================================
 *                    UKŁAD STEROWANIA - Aplikacja USB HID / OTG
 * =================================================================================
 * Urządzenie: Arduino Leonardo / Pro Micro (ATmega32u4)
 * Zasilanie obwodu: +5V (VCC) / GND
 * Polączenie z hostem: USB OTG (PC / Android)
 *
 * --- MAPA KANAŁÓW ANALOGOWYCH I PRZYCISKÓW ---
 *
 * [ KANAŁ A1 ] -> Poziom (Lewo / Prawo)
 *   • Przycisk 1 : LEWO   | Rezystancja: za 1. rezystorem 330R | U: ~2.919V | Klawisz: STRZAŁKA W LEWO
 *   • Przycisk 2 : PRAWO  | Rezystancja: za 2. rezystorem 330R | U: ~2.864V | Klawisz: STRZAŁKA W PRAWO
 *
 * [ KANAŁ A2 ] -> Pion (Góra / Dół)
 *   • Przycisk 3 : GÓRA   | Rezystancja: za 1. rezystorem 330R | U: ~2.919V | Klawisz: STRZAŁKA W GÓRĘ
 *   • Przycisk 4 : DÓŁ    | Rezystancja: za 2. rezystorem 330R | U: ~2.864V | Klawisz: STRZAŁKA W DÓŁ
 *
 * [ KANAŁ A3 ] -> Akcje (Strzał / Wybór)
 *   • Przycisk 5 : STRZAŁ | Rezystancja: za 1. rezystorem 330R | U: ~2.919V | Klawisz: SPACJA
 *   • Przycisk 6 : WYBÓR  | Rezystancja: za 2. rezystorem 330R | U: ~2.864V | Klawisz: ENTER
 *
 * --- SCHEMAT ELEKTRYCZNY JEDNEGO KANAŁU ---
 *
 *    +5V (VCC) --- [6.8 kΩ] --- [330 Ω] ---+--- [Przycisk nieparzysty] ---+
 *                                          |                             |
 *                                       [330 Ω]                          +---> Pin Analogowy (A1/A2/A3)
 *                                          |                             |     (podciągnięty 10k do GND)
 *                                          +--- [Przycisk parzysty] -----+
 *
 * =================================================================================
 */

#include <Arduino.h>
#include <Keyboard.h> // Biblioteka obsługi klawiatury USB HID dla Arduino Leonardo / Micro

const float voltagePerStep = 5.0 / 1023.0; // Napięcie na krok konwersji ADC

// Napięcia wzorcowe dla dzielnika (6.8k + 330R + 330R z podciągnięciem 10k do GND)
const float vButton1 = 2.919; // Przycisk nieparzysty (za 1-szym 330R) -> ~597 ADC
const float vButton2 = 2.864; // Przycisk parzysty    (za 2-gim 330R) -> ~586 ADC
const float voltageTolerance = 0.020; // Tolerancja ±0.020 V (~4 kroki ADC)

const int channelCount = 3;
const int analogPins[channelCount] = {A1, A2, A3};

// Kody klawiszy USB przypisane do przycisków (1-6)
const uint8_t buttonKeys[6] = {
  KEY_LEFT_ARROW,  // Przycisk 1 (A1) - LEWO
  KEY_RIGHT_ARROW, // Przycisk 2 (A1) - PRAWO
  KEY_UP_ARROW,    // Przycisk 3 (A2) - GÓRA
  KEY_DOWN_ARROW,  // Przycisk 4 (A2) - DÓŁ
  ' ',             // Przycisk 5 (A3) - STRZAŁ (Spacja)
  KEY_RETURN       // Przycisk 6 (A3) - WYBÓR (Enter)
};

// Pamięć stanu przycisków w poprzedniej pętli
bool previousButtonState[6] = {false, false, false, false, false, false};

void setup() {
  // Inicjalizacja emulacji klawiatury USB HID
  Keyboard.begin();
}

void loop() {
  // 1. Weryfikacja połączenia USB z komputerem/telefonem
  if (!USBDevice.configured()) {
    // W przypadku odłączenia USB rozłącz powiązane klawisze
    Keyboard.releaseAll();
    delay(100);
    return;
  }

  // 2. Tymczasowa tablica na odczyt bieżącego stanu
  bool currentButtonState[6] = {false, false, false, false, false, false};

  // 3. Odczyt z 3 kanałów analogowych
  for (int ch = 0; ch < channelCount; ch++) {
    int sensorValue = analogRead(analogPins[ch]);
    float voltage = sensorValue * voltagePerStep;

    int btn1Index = ch * 2;       // Przyciski nieparzyste: 0 (A1), 2 (A2), 4 (A3)
    int btn2Index = (ch * 2) + 1;   // Przyciski parzyste:    1 (A1), 3 (A2), 5 (A3)

    // Detekcja przycisku nieparzystego
    if (voltage >= (vButton1 - voltageTolerance) && voltage <= (vButton1 + voltageTolerance)) {
      currentButtonState[btn1Index] = true;
    } 
    // Detekcja przycisku parzystego
    else if (voltage >= (vButton2 - voltageTolerance) && voltage <= (vButton2 + voltageTolerance)) {
      currentButtonState[btn2Index] = true;
    }
  }

  // 4. Obsługa zdarzeń wciśnięcia i puszczenia dla USB HID
  for (int i = 0; i < 6; i++) {
    // Przycisk został właśnie NACIŚNIĘTY
    if (currentButtonState[i] && !previousButtonState[i]) {
      Keyboard.press(buttonKeys[i]);
    }
    // Przycisk został właśnie PUSZCZONY
    else if (!currentButtonState[i] && previousButtonState[i]) {
      Keyboard.release(buttonKeys[i]);
    }

    // Zapamiętanie stanu do kolejnej pętli
    previousButtonState[i] = currentButtonState[i];
  }

  delay(10); // Krótkie opóźnienie filtrujące (debouncing dla USB)
}


