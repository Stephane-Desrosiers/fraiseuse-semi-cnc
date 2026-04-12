---
name: project_igaging
description: Règles iGaging AbsoluteDRO Plus — protocole 52 bits, câblage vérifié, état du décodage
type: project
originSessionId: 2526d917-5632-4725-870e-cff893d689a8
---
Règles installées : iGaging AbsoluteDRO Plus, item #35-806-A (0-6")
- 2 axes installés (X, Y), 3e axe (Z) à venir
- Résolution : 0.0005" / 0.01mm
- Précision : 0.001"
- Connecteur : USB Micro-B (ce n'est PAS du USB — protocole propriétaire clock/data)

**Protocole 52 bits (13 nibbles de 4 bits, LSB en premier) :**
- La règle génère son propre clock (~1.1 kHz mesuré)
- Ligne REQ : tirer à GND pour déclencher la transmission continue
- Nibbles 0-3 : Header (0xF)
- Nibble 4 : Signe (0=positif, 8=négatif)
- Nibbles 5-10 : Position (encodage binaire OU BCD — à confirmer, les deux sont testés)
- Nibble 11 : Position du point décimal (lit 2)
- Nibble 12 : Unités (lit 0)

**IMPORTANT — encodage binaire vs BCD non encore confirmé.** Le header se décode bien mais
les valeurs de position sont trop élevées en interprétation binaire. Le code actuel affiche
les deux interprétations en parallèle pour comparaison. Prochaine étape : valider avec
l'afficheur LCD iGaging.

**Câblage VÉRIFIÉ (points de test ouverts sur la tête de lecture) :**
| Pin breakout | Signal tête | Pin Teensy |
|---|---|---|
| 5V/VCC | VDD (alimentation) | 3.3V |
| D+ | DATA | Pin 2 (GPIO) |
| D- | CLK (clock) | Pin 3 (GPIO) |
| ID | REQ | GND (tiré à masse) |
| GND | GND | GND |

**Attention :** Le pinout USB Micro-B iGaging ne correspond PAS au pinout Mini-B ni au
pinout USB standard. Il faut vérifier sur les points de test de la tête de lecture.

Résistances pull-up 82K entre 3.3V et DATA (pin 2), entre 3.3V et CLK (pin 3).
Condensateur 100nF entre 3.3V et GND.

**Méthode de lecture :** Le polling (style Mitutoyo) est plus fiable que les interruptions.
Les interruptions causaient des crashs ou des lectures erratiques. Le code actuel utilise
le polling avec lecture au front descendant du clock.

**Why:** Objectif = lire les positions des règles pour affichage DRO + boucle d'asservissement.
**How to apply:** Utiliser des breakout boards USB Micro-B femelle. Prévoir 2 GPIO par axe
(CLK + DATA) + le REQ tiré à GND. Code dans teensy/src/main.cpp.
