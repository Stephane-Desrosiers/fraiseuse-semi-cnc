---
name: project_hardware
description: Choix matériel — Raspberry Pi 5 + Teensy 4.x, Arduino Uno/Nano écarté, liste d'achats
type: project
---

Architecture confirmée : **Raspberry Pi 5 (4 GB) + Teensy 4.x**
**Why:** Arduino Uno/Nano insuffisant — seulement 2 interrupts hardware (il en faut 6+ pour 3 encodeurs quadrature), 2 KB RAM trop juste, 16 MHz risque de perte d'impulsions. Pi 3B/3B+ possible mais lent (1 GB RAM) pour une utilisation quotidienne en atelier.
**How to apply:** Le Teensy gère tout le temps réel (scales, encodeurs, moteurs, joystick). Le Pi 5 gère la GUI PyQt6 sur écran tactile.

Raspberry Pi 3B/3B+ existants (3 unités) — gardés pour d'autres projets.

## Liste d'achats à prévoir
- Raspberry Pi 5 (4 GB)
- Écran tactile officiel 7" (DSI)
- Boîtier avec support écran
- Alimentation officielle Pi 5 (27W USB-C)
- Carte SD 32 GB (classe A2)
- 3× breakout boards USB Micro-B femelle (pour les 3 règles iGaging)

## I/O Teensy nécessaires (résumé)
- 3× règles iGaging : 2 GPIO/axe (Clock interrupt + Data) + 1 GPIO REQ
- 3× encodeurs quadrature : 2 GPIO interrupt/axe
- 3× TMC5160 : SPI bus partagé + 3 CS pins
- 1× joystick 2 axes : 2 entrées analogiques
