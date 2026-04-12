/*
 * ============================================================================
 * Projet : Fraiseuse Semi-CNC
 * Fichier : main.cpp
 * Description : Lecture règle iGaging AbsoluteDRO Plus par POLLING v2
 *               Synchronisation de trame améliorée
 *
 * Câblage breakout → Teensy (vérifié avec points de test sur la tête) :
 *   5V/VCC breakout → 3.3V Teensy (VDD alimentation)
 *   D+     breakout → Pin 2 Teensy (DATA)
 *   D-     breakout → Pin 3 Teensy (CLK)
 *   ID     breakout → GND Teensy (REQ tiré à masse)
 *   GND    breakout → GND Teensy
 *
 * Pull-ups 82K sur DATA et CLK vers 3.3V
 * Condensateur 100nF entre 3.3V et GND
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
// Le gap entre trames doit être plus long que ça
const unsigned long FRAME_GAP_US = 2000;

// ============================================================================
// Attendre le début d'une nouvelle trame
// Cherche un moment où le clock reste stable (HIGH) pendant > FRAME_GAP_US
// puis attend le premier front descendant = bit 0
// ============================================================================
bool wait_for_frame_start() {
    unsigned long timeout = millis();

    // Étape 1 : attendre que le clock soit en train de bouger
    //           (on est peut-être déjà dans un gap, il faut d'abord
    //            voir au moins une transition pour savoir que ça tourne)

    // Étape 2 : chercher un gap — le clock reste HIGH pendant > FRAME_GAP_US
    while (true) {
        // Timeout global de 500ms
        if (millis() - timeout > 500) return false;

        // Attendre que le clock soit HIGH
        unsigned long wait_start = micros();
        while (digitalReadFast(CLK_PIN) == LOW) {
            if (micros() - wait_start > 100000) return false;
        }

        // Mesurer combien de temps le clock reste HIGH
        unsigned long high_start = micros();
        while (digitalReadFast(CLK_PIN) == HIGH) {
            // Si le clock reste HIGH assez longtemps = on est dans un gap
            if (micros() - high_start > FRAME_GAP_US) {
                // Trouvé un gap ! Maintenant attendre le premier front descendant
                // qui marque le début de la nouvelle trame
                unsigned long gap_wait = micros();
                while (digitalReadFast(CLK_PIN) == HIGH) {
                    if (micros() - gap_wait > 100000) return false;
                }
                // Le clock vient de descendre → c'est le début du bit 0
                return true;
            }
        }
        // Le clock est redescendu trop vite — c'était juste un bit normal
        // On continue à chercher le gap
    }
}

// ============================================================================
// Lire les 52 bits (13 nibbles) après synchronisation
// On est positionné juste après le premier front descendant (bit 0)
// ============================================================================
bool read_52_bits(uint8_t* nibbles) {
    // Le premier front descendant vient de se produire
    // Lire le bit 0 maintenant
    uint8_t current_nibble = 0;
    if (digitalReadFast(DATA_PIN)) {
        current_nibble |= 1;
    }

    // Lire les 51 bits restants
    for (int bit = 1; bit < 52; bit++) {
        // Attendre front montant (clock HIGH)
        unsigned long t = micros();
        while (digitalReadFast(CLK_PIN) == LOW) {
            if (micros() - t > 5000) return false;  // timeout 5ms par bit
        }
        // Attendre front descendant (clock LOW)
        t = micros();
        while (digitalReadFast(CLK_PIN) == HIGH) {
            if (micros() - t > 5000) return false;
        }
        // Lire le bit data
        int nibble_index = bit / 4;
        int bit_in_nibble = bit % 4;

        if (bit_in_nibble == 0) {
            current_nibble = 0;  // nouveau nibble
        }

        if (digitalReadFast(DATA_PIN)) {
            current_nibble |= (1 << bit_in_nibble);
        }

        // Stocker le nibble quand les 4 bits sont lus
        if (bit_in_nibble == 3) {
            nibbles[nibble_index] = current_nibble;
        }
    }

    // Le nibble 0 a été lu bit par bit mais jamais stocké via la boucle
    // (le premier bit était hors boucle). On doit le gérer.
    // En fait, le bit 0 met current_nibble, puis bits 1-3 complètent
    // et stockent nibbles[0] quand bit=3, bit_in_nibble=3. Vérifions :
    // bit=0 → hors boucle, current_nibble = bit0
    // bit=1 → nibble_index=0, bit_in_nibble=1 → continue
    // bit=2 → nibble_index=0, bit_in_nibble=2 → continue
    // bit=3 → nibble_index=0, bit_in_nibble=3 → nibbles[0] = current_nibble ✓

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
    Serial.println("iGaging AbsoluteDRO Plus — Polling v2");
    Serial.println("Synchro par détection de gap inter-trame");
    Serial.println("====================================");
}

// ============================================================================
// Boucle principale
// ============================================================================
void loop() {
    uint8_t nibbles[13] = {0};

    // Synchroniser sur le début d'une trame
    if (!wait_for_frame_start()) {
        Serial.println("[ERREUR] Timeout — pas de gap détecté");
        delay(1000);
        return;
    }

    // Lire les 52 bits
    if (!read_52_bits(nibbles)) {
        Serial.println("[ERREUR] Timeout pendant lecture des bits");
        delay(500);
        return;
    }

    // --- Vérification du header ---
    bool header_ok = (nibbles[0] == 0xF) && (nibbles[1] == 0xF) &&
                     (nibbles[2] == 0xF) && (nibbles[3] == 0xF);

    // Si le header est mauvais, ignorer cette trame (désynchronisé)
    if (!header_ok) {
        Serial.print("[SYNC] Header invalide: ");
        for (int i = 0; i < 4; i++) {
            Serial.print(nibbles[i], HEX);
            Serial.print(" ");
        }
        Serial.println("— trame ignorée");
        return;
    }

    // --- Filtrer les trames corrompues ---
    // Les trames valides ont toujours Dec=2 et Unit=0
    if (nibbles[11] != 2 || nibbles[12] != 0) {
        return;  // trame corrompue, ignorer silencieusement
    }

    // --- Signe ---
    bool is_negative = (nibbles[4] == 0x8);

    // --- Position : interprétation BINAIRE (24 bits) ---
    long pos_bin = 0;
    for (int i = 5; i <= 10; i++) {
        pos_bin |= ((long)nibbles[i] << ((i - 5) * 4));
    }
    if (is_negative) pos_bin = -pos_bin;
    float mm_bin = pos_bin / 100.0;

    // --- Position : interprétation BCD (nibble 5 = MSD, nibble 10 = LSD) ---
    // Lecture : d5 d6 d7 d8 d9 d10 forme le nombre décimal
    // Exemple : nibbles {0,3,3,1,1,3} → 033113 → /100 = 331.13 mm
    long pos_bcd = 0;
    for (int i = 5; i <= 10; i++) {
        pos_bcd = pos_bcd * 10 + nibbles[i];
    }
    if (is_negative) pos_bcd = -pos_bcd;
    float mm_bcd = pos_bcd / 100.0;

    // --- Affichage ---
    Serial.print("Nib: ");
    for (int i = 0; i < 13; i++) {
        Serial.print(nibbles[i], HEX);
        Serial.print(" ");
    }

    Serial.print("| BIN:");
    Serial.print(mm_bin, 2);
    Serial.print("mm");

    Serial.print(" | BCD:");
    Serial.print(mm_bcd, 2);
    Serial.print("mm");

    Serial.print(" | Dec:");
    Serial.print(nibbles[11]);
    Serial.print(" U:");
    Serial.println(nibbles[12]);

    // Pause pour lisibilité
    delay(250);
}
