---
name: user_profile
description: Profil utilisateur - Steph, développeur VB6/VB.NET 20 ans, projets motion control Teensy/Python
type: user
---

- Prénom : Steph (sdesrosiers)
- Environnement : Windows 11 Pro, Python 3.13
- Langue : français (Québec)
- Niveau technique : développeur de longue date (20+ ans VB6 / VB.NET), très à l'aise techniquement
- Pense naturellement en événements, composants visuels et propriétés (héritage VB)
- Préfère lancer les scripts lui-même plutôt que de les faire exécuter par Claude
- Veut des backups avant les modifications importantes

## Stack habituel
- Teensy (préféré à Arduino pour puissance, I/O, ports série multiples)
- PlatformIO dans VS Code pour le code Teensy (C++)
- PyQt6 + Qt Designer pour les interfaces graphiques (Windows et Raspberry Pi)
- Qt Designer déjà maîtrisé (drag-and-drop, propriétés visuelles)
- pyserial pour la communication USB série Teensy <-> Python
- Tkinter connu mais évité dès que le projet se complexifie

## Architecture série typique
- Serial (USB) : commandes/données vers Python
- Serial1 : debug via Serial Monitor PlatformIO

## Domaines de projets
- Contrôle de moteurs pas à pas : drives StepperOnline, modules TMC5160 (SPI)
- Motorisation fraiseuse manuelle conventionnelle (ce projet)
- Robot avec servos RC en contrôle de vitesse constante
- Gestion de boutons avec debouncing (Bounce2)
- Encodeurs incrémentaux (SameSky AMT10E2 et équivalents)
