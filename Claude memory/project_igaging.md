---
name: project_igaging
description: Règles iGaging AbsoluteDRO Plus — protocole 52 bits BCD décodé et fonctionnel
type: project
originSessionId: 2526d917-5632-4725-870e-cff893d689a8
---
Règles installées : iGaging AbsoluteDRO Plus (12", pas 6")
- 2 axes installés (X, Y), 3e axe (Z) à venir
- Résolution : 0.0005" / 0.01mm
- Précision : 0.001"
- Connecteur : USB Micro-B (ce n'est PAS du USB — protocole propriétaire clock/data)
- Course mesurée : -0.79mm à 330.88mm (~331.67mm ≈ 13.06")

**Protocole 52 bits — DÉCODÉ ET FONCTIONNEL (2026-04-12) :**
- 13 nibbles de 4 bits, LSB en premier dans chaque nibble
- Clock généré par la règle (~1.1 kHz mesuré)
- Ligne REQ tirée à GND = transmission continue
- Lecture par **polling** au front descendant du clock (pas interruptions — instables)
- Gap inter-trame détecté quand clock reste HIGH > 2ms

**Format des nibbles :**
| Nibbles | Contenu |
|---|---|
| 0-3 | Header (toujours 0xF) |
| 4 | Signe (0=positif, 8=négatif) |
| 5-10 | Position **BCD**, nibble 5=MSD, nibble 10=LSD |
| 11 | Décimales (toujours 2 → diviser par 100 pour mm) |
| 12 | Unités (toujours 0 → mm) |

**Décodage BCD :** lire nibbles 5→10 comme chiffres décimaux de gauche à droite
(nibble 5 = chiffre le plus significatif). Diviser par 100 pour obtenir mm.
Filtrer les trames où Dec≠2 ou Unit≠0 (trames corrompues).

**Câblage VÉRIFIÉ (points de test sur la tête de lecture) :**
| Pin breakout | Signal tête | Pin Teensy |
|---|---|---|
| 5V/VCC | VDD (alimentation) | 3.3V |
| D+ | DATA | Pin 2 (GPIO) |
| D- | CLK (clock) | Pin 3 (GPIO) |
| ID | REQ | GND (tiré à masse) |
| GND | GND | GND |

Résistances pull-up 82K entre 3.3V et DATA/CLK.
Condensateur 100nF entre 3.3V et GND.

**ATTENTION pinout :** Le pinout Micro-B iGaging est NON STANDARD et différent du Mini-B.
Ne PAS se fier aux labels du breakout board — vérifier sur les points de test de la tête.

**Why:** Objectif = DRO custom + boucle d'asservissement position (règle → Teensy → moteur).
**How to apply:** Code dans teensy/src/main.cpp. Utiliser polling, pas interruptions.
