# Projet : Fraiseuse Semi-CNC

## Description
Motorisation d'une fraiseuse manuelle conventionnelle avec moteurs pas à pas,
contrôlée via une interface graphique PyQt6 sur Raspberry Pi ou Windows.

## Architecture générale

```
Raspberry Pi 5 (4GB)               Teensy 4.x
┌─────────────────────┐  USB/Série  ┌──────────────────────┐
│ PyQt6 GUI            │ ←────────→ │ 3× règles iGaging     │
│ Écran tactile 7"     │            │ 3× encodeurs quadrat. │
│ Mémo positions X/Y/Z │            │ 3× moteurs TMC5160    │
│ Logique haute niveau │            │ Joystick 2 axes       │
└─────────────────────┘            │ Boucle asservissement │
                                    └──────────────────────┘
```

## Stack technique

### Côté Teensy (C++)
- **IDE** : PlatformIO dans VS Code
- **Drivers moteur** : TMC5160 (SPI), drives StepperOnline
- **Librairie moteur** : TMCStepper
- **Encodeurs** : SameSky AMT10E2 (incrémentaux, interruptions hardware)
- **Règles DRO** : iGaging AbsoluteDRO Plus #35-806-A (protocole 52 bits BCD, 3.3V)
- **Boutons** : librairie Bounce2 (debouncing)
- **Servos** : PWM hardware Teensy

### Côté Raspberry Pi 5 (Python)
- **GUI** : PyQt6 + Qt Designer (fichiers .ui)
- **Écran** : tactile 7" officiel (DSI)
- **Communication série** : pyserial
- **Plateforme** : Raspberry Pi 5 (4 GB) — développement aussi possible sur Windows 11

## Ports série Teensy
- `Serial` (USB) → commandes/données vers Python
- `Serial1` → debug via Serial Monitor PlatformIO

## Protocole de communication série
- Format : **JSON newline-delimited** (décision finale)
- Baud : 115200, délimiteur `\n`, timeout ACK 200ms
- Voir `Claude memory/project_protocol.md` pour la structure complète

## Structure du projet
```
fraiseuse semi-cnc/
├── CLAUDE.md
├── JOURNAL.md
├── teensy/
│   ├── platformio.ini
│   └── src/main.cpp
└── python/
    ├── main.py
    ├── mainwindow.ui
    └── serial_comm.py
```

## Profil développeur
- 20+ ans VB6/VB.NET — pense en événements et composants visuels
- À l'aise avec Qt Designer (drag-and-drop, propriétés)
- Préfère exécuter les scripts lui-même
- Veut des backups avant modifications importantes
- Communication en français

## Fichiers mémoire Claude
Les fichiers mémoire sont dans `Claude memory/` (versionné dans Git).
Sur une nouvelle machine, copier le contenu de `Claude memory/` vers :
`C:\Users\<user>\.claude\projects\<slug-du-projet>\memory\`

## Règles iGaging AbsoluteDRO Plus
- 2 axes installés (X, Y), 3e (Z) à venir
- Connecteur USB Micro-B mais **ce n'est PAS du USB** — protocole propriétaire
- Câblage via breakout board USB Micro-B femelle vers GPIO Teensy
- Clock (D-), Data (D+), REQ (tirer à GND), alimentation 3.3V
- Objectif : DRO custom + boucle d'asservissement position (règle → Teensy → moteur)
- Voir `Claude memory/project_igaging.md` pour les détails complets

### Câblage vérifié (points de test sur la tête de lecture)
| Pin breakout | Signal tête | Pin Teensy |
|---|---|---|
| 5V/VCC | VDD | 3.3V |
| D+ | DATA | Pin 2 |
| D- | CLK | Pin 3 |
| ID | REQ | GND |
| GND | GND | GND |

### Protocole décodé et fonctionnel (2026-04-12)
- Encodage **BCD** confirmé (nibble 5=MSD → nibble 10=LSD, diviser par 100 pour mm)
- Méthode **interruptions FALLING** validée : ~10 Hz, 0% erreur, non-bloquant
- Synchro par détection gap inter-trame (clock HIGH > 2ms)
- Filtrage trames corrompues (Dec≠2 ou Unit≠0)
- Course mesurée : -0.79mm à 330.88mm (~331.67mm)
- Règles = 12" (pas 6" comme initialement noté)
- 3 axes en parallèle possible (~10 Hz chacun, interruptions indépendantes)

## Notes importantes
- Debouncing : hardware + software recommandé (environnement bruité fraiseuse)
- stallGuard TMC5160 : envisager pour détection fin de course sans capteur physique
- Protocole série doit être robuste — machine-outil, commande perdue = conséquences physiques
