---
name: project_igaging
description: Règles iGaging AbsoluteDRO Plus — protocole 52 bits BCD, câblage, spécifications
type: project
---

Règles installées : iGaging AbsoluteDRO Plus, item #35-806-A (0-6")
- 2 axes installés (X, Y), 3e axe (Z) à venir
- Résolution : 0.0005" / 0.01mm
- Précision : 0.001"
- Connecteur : USB Micro-B (ce n'est PAS du USB — protocole propriétaire clock/data)

**Protocole 52 bits BCD :**
- 13 nibbles de 4 bits, LSB en premier
- La règle génère son propre clock (~2 kHz)
- Ligne REQ : tirer à GND pour déclencher la transmission continue
- Taux de rafraîchissement : jusqu'à 250 Hz
- Alimentation : 3.3V (natif Teensy, pas de diviseur de tension nécessaire)

**Câblage breakout USB Micro-B femelle → Teensy :**
| Pin breakout | Fonction | Pin Teensy |
|---|---|---|
| VCC (rouge) | Alimentation | 3.3V |
| D- (blanc) | Clock | GPIO (interrupt) |
| D+ (vert) | Data | GPIO |
| ID (noir) | GND | GND |

**Why:** Objectif = lire les positions des règles pour affichage DRO + boucle d'asservissement moteurs pas à pas.
**How to apply:** Utiliser des breakout boards USB Micro-B femelle (pas des adaptateurs USB-A). Prévoir 2 GPIO par axe (Clock + Data) + 1 GPIO REQ (partageable entre axes). Sources de référence : yuriystoys.com et touchdro.com.
