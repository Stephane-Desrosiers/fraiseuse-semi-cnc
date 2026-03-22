---
name: project_protocol
description: État de la conception du protocole série — en attente du cahier des charges
type: project
---

Protocole série choisi : JSON newline-delimited (décision 2026-03-22)
**Why:** Lisibilité pour le débogage, simplicité de parsing des deux côtés, extensibilité. Binaire écarté — pas nécessaire pour les vitesses d'une fraiseuse.
**How to apply:** Utiliser ArduinoJson côté Teensy, json.loads() côté Python.

Structure de base définie :
- PC → Teensy : {"cmd": "JOG", "axis": "X", "dist": 0.1, "feed": 500}
- Teensy → PC (ACK) : {"ack": "JOG", "ok": true}
- Teensy → PC (statut ~10Hz) : {"type": "STATUS", "x": 0.0, "y": 0.0, "z": 0.0, "state": "IDLE", "limits": [0,0,0], "stall": false}
- Teensy → PC (alerte) : {"type": "ALERT", "code": "STALL_X", "msg": "..."}
- Délimiteur : \n | Baud : 115200 | Timeout ACK Python : 200ms
- STOP traité en priorité avant le reste du buffer

Questions ouvertes — à répondre via le cahier des charges :
1. Nombre d'axes (X/Y/Z + axe rotatif ?)
2. Unités : mm côté Python, conversion mm→steps dans le Teensy
3. Fins de course : capteurs physiques, stallGuard, ou les deux ?
4. JOG : mode incrémental seulement ou aussi mode continu (bouton maintenu) ?

Prochaine étape : Steph envoie le cahier des charges en PDF pour finaliser l'architecture.
