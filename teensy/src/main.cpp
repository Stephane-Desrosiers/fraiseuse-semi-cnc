/*
 * ============================================================================
 * Projet : Fraiseuse Semi-CNC
 * Fichier : main.cpp
 * Description : Test #2 — Lecture iGaging par INTERRUPTIONS
 *               Avec le câblage correct et décodage BCD confirmé
 *
 * Câblage breakout → Teensy (vérifié) :
 *   5V/VCC breakout → 3.3V Teensy (VDD alimentation)
 *   D+     breakout → Pin 2 Teensy (DATA)
 *   D-     breakout → Pin 3 Teensy (CLK)
 *   ID     breakout → GND Teensy (REQ tiré à masse)
 *   GND    breakout → GND Teensy
 * ============================================================================
 */

#include <Arduino.h>

// ============================================================================
// Configuration des pins
// ============================================================================
const int CLK_PIN  = 3;   // D- du breakout = CLK de la tête iGaging
const int DATA_PIN = 2;   // D+ du breakout = DATA de la tête iGaging

// Seuil pour détecter le gap inter-trame (en µs)
// Le clock tourne à ~1.1 kHz → ~900 µs entre bits
// Gap entre trames > 2000 µs
const unsigned long FRAME_GAP_US = 2000;

// ============================================================================
// Variables volatiles partagées entre ISR et loop()
// ============================================================================

// Buffer de collecte dans l'ISR
volatile uint8_t isr_nibbles[13];   // nibbles en cours de collecte
volatile int isr_bit_count = 0;     // bits reçus dans la trame courante
volatile uint8_t isr_current_nibble = 0;  // nibble en construction
volatile unsigned long isr_last_clock_us = 0;  // timestamp du dernier clock

// Résultat : dernière trame complète et valide
volatile float position_mm = 0.0;   // position décodée en mm
volatile bool new_position = false;  // flag : nouvelle position disponible
volatile unsigned long frame_count = 0;  // compteur de trames valides
volatile unsigned long error_count = 0;  // compteur de trames rejetées

// ============================================================================
// Routine d'interruption — front descendant du Clock
// ============================================================================
void clock_isr() {
    unsigned long now = micros();

    // --- Détecter le gap inter-trame ---
    if (now - isr_last_clock_us > FRAME_GAP_US) {
        // Gap détecté : la trame précédente est-elle complète (52 bits) ?
        if (isr_bit_count == 52) {
            // Vérifier le header (nibbles 0-3 = 0xF)
            if (isr_nibbles[0] == 0xF && isr_nibbles[1] == 0xF &&
                isr_nibbles[2] == 0xF && isr_nibbles[3] == 0xF) {
                // Vérifier Dec=2 et Unit=0 (filtrage trames corrompues)
                if (isr_nibbles[11] == 2 && isr_nibbles[12] == 0) {
                    // --- Décodage BCD (nibble 5=MSD → nibble 10=LSD) ---
                    long pos = 0;
                    for (int i = 5; i <= 10; i++) {
                        pos = pos * 10 + isr_nibbles[i];
                    }
                    // Appliquer le signe (nibble 4 : 0=positif, 8=négatif)
                    if (isr_nibbles[4] == 0x8) {
                        pos = -pos;
                    }
                    // Stocker la position en mm (diviser par 100)
                    position_mm = pos / 100.0;
                    new_position = true;
                    frame_count++;
                } else {
                    error_count++;
                }
            } else {
                error_count++;
            }
        }
        // Réinitialiser pour la nouvelle trame
        isr_bit_count = 0;
        isr_current_nibble = 0;
    }

    // --- Lire le bit data au front descendant du clock ---
    if (isr_bit_count < 52) {
        int bit_in_nibble = isr_bit_count % 4;
        int nibble_index = isr_bit_count / 4;

        // Début d'un nouveau nibble
        if (bit_in_nibble == 0) {
            isr_current_nibble = 0;
        }

        // Lire le bit (LSB first dans chaque nibble)
        if (digitalReadFast(DATA_PIN)) {
            isr_current_nibble |= (1 << bit_in_nibble);
        }

        // Stocker le nibble quand les 4 bits sont lus
        if (bit_in_nibble == 3) {
            isr_nibbles[nibble_index] = isr_current_nibble;
        }

        isr_bit_count++;
    }

    isr_last_clock_us = now;
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
    Serial.println("iGaging — Test INTERRUPTIONS v2");
    Serial.println("Front descendant, décodage BCD");
    Serial.println("====================================");

    // Attacher l'interruption APRÈS les messages de démarrage
    attachInterrupt(digitalPinToInterrupt(CLK_PIN), clock_isr, FALLING);
    Serial.println("Interruption activée sur pin 3 (FALLING).");
    Serial.println();
}

// ============================================================================
// Boucle principale — complètement non-bloquante
// ============================================================================
unsigned long last_print_ms = 0;
float last_displayed_mm = -99999;

void loop() {
    // Afficher la position toutes les 250ms OU quand elle change
    if (millis() - last_print_ms > 250) {
        last_print_ms = millis();

        // Lire la position de façon atomique
        noInterrupts();
        float mm = position_mm;
        bool fresh = new_position;
        new_position = false;
        unsigned long frames = frame_count;
        unsigned long errors = error_count;
        interrupts();

        if (fresh) {
            Serial.print("Position: ");
            Serial.print(mm, 2);
            Serial.print(" mm | Trames OK: ");
            Serial.print(frames);
            Serial.print(" | Erreurs: ");
            Serial.print(errors);

            // Taux de réussite
            unsigned long total = frames + errors;
            if (total > 0) {
                Serial.print(" | Taux: ");
                Serial.print((frames * 100) / total);
                Serial.print("%");
            }

            // Fréquence estimée (trames par seconde)
            Serial.print(" | ~");
            Serial.print(frames / max(1UL, millis() / 1000));
            Serial.print(" Hz");

            Serial.println();
        } else {
            Serial.println("[ATTENTE] Pas de nouvelle trame...");
        }
    }
}
