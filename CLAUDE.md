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
*À définir — format JSON ou trames binaires structurées*
- Commandes : ex. `JOG X+0.1`, `HOME X`, `SET_FEEDRATE 500`
- Statuts retournés : position X/Y/Z, état fins de course, alertes

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

## Notes importantes
- Debouncing : hardware + software recommandé (environnement bruité fraiseuse)
- stallGuard TMC5160 : envisager pour détection fin de course sans capteur physique
- Protocole série doit être robuste — machine-outil, commande perdue = conséquences physiques
