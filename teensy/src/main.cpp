/*
 * ============================================================================
 * Projet : Fraiseuse Semi-CNC
 * Fichier : main.cpp
 * Description : Lecture règle iGaging AbsoluteDRO Plus par POLLING
 *               Protocole 52 bits (13 nibbles)
 *
 * Câblage breakout → Teensy (vérifié avec points de test sur la tête) :
 *   GND du breakout → GND Teensy
 *   5V/VCC du breakout → 3.3V Teensy (VDD alimentation de la règle)
 *   D+ du breakout → Pin 2 Teensy (DATA de la tête)
 *   D- du breakout → Pin 3 Teensy (CLK de la tête)
 *   ID du breakout → GND Teensy (REQ tiré à masse = transmission continue)
 *
 * Méthode : polling (pas d'interruptions) — plus fiable pour ce protocole
 * ============================================================================
 */

#include <Arduino.h>

// ============================================================================
// Configuration des pins
// ============================================================================
const int CLK_PIN  = 3;   // D- du breakout = CLK de la tête iGaging
const int DATA_PIN = 2;   // D+ du breakout = DATA de la tête iGaging

// ============================================================================
// Lire un nibble (4 bits) en polling — méthode Mitutoyo/iGaging
// Attend le front descendant du clock puis lit le bit data
// ============================================================================
uint8_t read_nibble() {
    uint8_t nibble = 0;
    for (int bit = 0; bit < 4; bit++) {
        // Attendre que le clock monte (HIGH)
        while (digitalReadFast(CLK_PIN) == LOW) {}
        // Attendre que le clock redescende (LOW) = front descendant
        while (digitalReadFast(CLK_PIN) == HIGH) {}
        // Lire le bit data maintenant (après le front descendant)
        if (digitalReadFast(DATA_PIN)) {
            nibble |= (1 << bit);  // LSB first
        }
    }
    return nibble;
}

// ============================================================================
// Lire une trame complète de 13 nibbles (52 bits)
// ============================================================================
bool read_frame(uint8_t* nibbles) {
    // D'abord, attendre un gap (pause dans le clock) qui marque
    // le début d'une nouvelle trame
    // Attendre que le clock soit HIGH (état de repos)
    unsigned long timeout = micros();
    while (digitalReadFast(CLK_PIN) == LOW) {
        if (micros() - timeout > 100000) return false;  // timeout 100ms
    }

    // Attendre un gap : le clock reste HIGH pendant > 500µs
    // (entre les trames, le clock fait une pause)
    timeout = micros();
    while (digitalReadFast(CLK_PIN) == HIGH) {
        if (micros() - timeout > 100000) return false;  // timeout 100ms
    }

    // Le clock vient de descendre — c'est le début du premier bit
    // On a raté le premier front descendant, lire le premier bit maintenant
    nibbles[0] = 0;
    if (digitalReadFast(DATA_PIN)) {
        nibbles[0] |= (1 << 0);  // bit 0 du nibble 0
    }

    // Lire les 3 bits restants du nibble 0
    for (int bit = 1; bit < 4; bit++) {
        while (digitalReadFast(CLK_PIN) == LOW) {}
        while (digitalReadFast(CLK_PIN) == HIGH) {}
        if (digitalReadFast(DATA_PIN)) {
            nibbles[0] |= (1 << bit);
        }
    }

    // Lire les nibbles 1 à 12
    for (int i = 1; i < 13; i++) {
        nibbles[i] = read_nibble();
    }

    return true;
}

// ============================================================================
// Setup
// ============================================================================
void setup() {
    Serial.begin(115200);
    unsigned long start = millis();
    while (!Serial && (millis() - start < 3000)) {}
    delay(500);

    pinMode(CLK_PIN, INPUT);
    pinMode(DATA_PIN, INPUT);

    Serial.println("====================================");
    Serial.println("iGaging AbsoluteDRO Plus — Polling");
    Serial.println("====================================");
}

// ============================================================================
// Boucle principale
// ============================================================================
void loop() {
    uint8_t nibbles[13];

    if (!read_frame(nibbles)) {
        Serial.println("[ERREUR] Timeout — pas de signal clock");
        delay(1000);
        return;
    }

    // --- Vérification du header ---
    bool header_ok = (nibbles[0] == 0xF) && (nibbles[1] == 0xF) &&
                     (nibbles[2] == 0xF) && (nibbles[3] == 0xF);

    // --- Signe ---
    bool is_negative = (nibbles[4] == 0x8);

    // --- Position : nibbles 5-10, interprétation BINAIRE (24 bits) ---
    long pos_bin = 0;
    for (int i = 5; i <= 10; i++) {
        pos_bin |= ((long)nibbles[i] << ((i - 5) * 4));
    }
    if (is_negative) pos_bin = -pos_bin;
    float mm_bin = pos_bin / 100.0;

    // --- Position : nibbles 5-10, interprétation BCD ---
    // nibble 10 = MSD, nibble 5 = LSD
    long pos_bcd = 0;
    long mult = 1;
    for (int i = 5; i <= 10; i++) {
        pos_bcd += nibbles[i] * mult;
        mult *= 10;
    }
    if (is_negative) pos_bcd = -pos_bcd;
    float mm_bcd = pos_bcd / 100.0;

    // --- Affichage ---
    Serial.print("Nibbles: ");
    for (int i = 0; i < 13; i++) {
        Serial.print(nibbles[i], HEX);
        Serial.print(" ");
    }

    Serial.print(" | Hdr:");
    Serial.print(header_ok ? "OK" : "ERR");

    Serial.print(" | BIN:");
    Serial.print(mm_bin, 2);
    Serial.print("mm");

    Serial.print(" | BCD:");
    Serial.print(mm_bcd, 2);
    Serial.print("mm");

    Serial.print(" | Dec:");
    Serial.print(nibbles[11]);
    Serial.print(" U:");
    Serial.println(nibbles[12]);

    // Petite pause pour ne pas inonder le moniteur
    delay(200);
}
