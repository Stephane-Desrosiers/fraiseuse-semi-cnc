# Projet : Fraiseuse Semi-CNC

## Description
Motorisation d'une fraiseuse manuelle conventionnelle avec moteurs pas à pas,
contrôlée via une interface graphique PyQt6 sur Raspberry Pi ou Windows.

## Architecture générale

```
Raspberry Pi (ou Windows)          Teensy 4.x
┌─────────────────────┐  USB/Série  ┌──────────────────────┐
│ PyQt6 GUI            │ ←────────→ │ Contrôle moteurs      │
│ Écran tactile        │            │ Lecture encodeurs     │
│ Logique haute niveau │            │ Debouncing boutons    │
│                      │            │ Temps réel            │
└─────────────────────┘            └──────────────────────┘
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

### Côté Python
- **GUI** : PyQt6 + Qt Designer (fichiers .ui)
- **Communication série** : pyserial
- **Plateforme** : Windows 11 ou Raspberry Pi

## Ports série Teensy
- `Serial` (USB) → commandes/données vers Python
- `Serial1` → debug via Serial Monitor PlatformIO

## Protocole de communication série
- Format : **JSON newline-delimited** (décision finale)
- Baud : 115200, délimiteur `\n`, timeout ACK 200ms
- Voir `memory/project_protocol.md` pour la structure complète

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
Les fichiers mémoire sont dans `memory/` (versionné dans Git).
Sur une nouvelle machine, copier le contenu de `memory/` vers :
`C:\Users\<user>\.claude\projects\<slug-du-projet>\memory\`

## Règles iGaging AbsoluteDRO Plus
- 2 axes installés (X, Y), 3e (Z) à venir
- Connecteur USB Micro-B mais **ce n'est PAS du USB** — protocole propriétaire
- Câblage via breakout board USB Micro-B femelle vers GPIO Teensy
- Clock (D-), Data (D+), REQ (tirer à GND), alimentation 3.3V
- Objectif : DRO custom + boucle d'asservissement position (règle → Teensy → moteur)
- Voir `memory/project_igaging.md` pour les détails complets

## Notes importantes
- Debouncing : hardware + software recommandé (environnement bruité fraiseuse)
- stallGuard TMC5160 : envisager pour détection fin de course sans capteur physique
- Protocole série doit être robuste — machine-outil, commande perdue = conséquences physiques
