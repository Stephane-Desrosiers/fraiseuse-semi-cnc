# Journal — Discussion initiale de mise en place
**Date :** 2026-03-22
**Workspace :** reservateur de campings (à migrer vers fraiseuse semi-cnc)

---

## Sujet de départ
Steph souhaite travailler sur des projets intégrant Arduino, Teensy et Raspberry Pi.
Question initiale : **quel est le meilleur environnement de développement à mettre en place?**

---

## Environnement recommandé

### Éditeur principal : VS Code

Extensions clés :

| Extension | Utilité |
|---|---|
| **Arduino** (Microsoft) | Support `.ino`, upload vers Arduino/Teensy |
| **PlatformIO IDE** | Alternative puissante à l'IDE Arduino, gère aussi Teensy |
| **C/C++** (Microsoft) | IntelliSense pour le code embarqué |
| **Python** (Microsoft) | Pour les scripts côté Raspberry Pi |
| **Remote - SSH** | Pour éditer du code directement sur le Raspberry Pi |

---

## arduino-cli vs PlatformIO

| | **arduino-cli** | **PlatformIO** |
|---|---|---|
| Officiel Arduino | Oui | Non (tiers) |
| Support Teensy | Limité (via Teensyduino) | Excellent natif |
| Intégration VS Code | Manuelle | Native (extension dédiée) |
| Gestion des librairies | `arduino-cli lib install` | Automatique via `platformio.ini` |
| Scripting / automatisation | Excellent (CLI pur) | Bien mais plus lourd |
| Courbe d'apprentissage | Faible | Moyenne |

**Conclusion :** PlatformIO recommandé pour la combinaison Teensy + Raspberry Pi.
arduino-cli reste pertinent pour automatiser des builds ponctuels.

---

## Profil et stack de Steph

- **Teensy** préféré à Arduino : plus de puissance, plus d'I/O, plusieurs ports série
- **Architecture typique :**
  - Port série 1 (`Serial`) → commandes/données vers Python
  - Port série 2 (`Serial1`) → debug via Serial Monitor PlatformIO
- **Python + PyQt6** pour l'interface graphique (Windows et Raspberry Pi)
- **Qt Designer** pour le design visuel des interfaces (déjà utilisé et apprécié)
- **tkinter** connu mais évité dès que le projet se complexifie
- **20+ ans de développement VB6/VB.NET** → pense naturellement en événements, composants visuels et propriétés

### Qt Designer — équivalent du Form Designer VB
```python
# Charger un .ui directement dans PyQt6
from PyQt6 import uic

class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        uic.loadUi("mainwindow.ui", self)  # comme LoadForm en VB
        self.btnEnvoyer.clicked.connect(self.envoyer_commande)
```

---

## Domaines de projets identifiés

- **Moteurs pas à pas** : drives StepperOnline, modules TMC5160
- **Fraiseuse manuelle conventionnelle** à motoriser (projet principal)
- **Robot** avec servos RC en contrôle de vitesse constante
- **Boutons** avec debouncing
- **Encodeurs incrémentaux** : SameSky AMT10E2

---

## Architecture retenue pour la fraiseuse

### Évolution du concept
Steph avait commencé avec un Teensy + petit écran LCD.
**Problème :** la complexité de programmation de l'interface LCD était trop grande.
**Décision :** passer à une interface PyQt6 sur Raspberry Pi.

### Architecture finale

```
Raspberry Pi (ou Windows)          Teensy 4.x
┌─────────────────────┐  USB/Série  ┌──────────────────────┐
│ PyQt6 GUI            │ ←────────→ │ Contrôle moteurs      │
│ Écran tactile        │            │ Lecture encodeurs     │
│ Logique haute niveau │            │ Debouncing boutons    │
│                      │            │ Temps réel            │
└─────────────────────┘            └──────────────────────┘
```

**Principe :** séparer les responsabilités
- Le **Raspberry** fait ce qu'il fait bien : affichage, interface, logique applicative
- Le **Teensy** fait ce qu'il fait bien : temps réel, PWM précis, interruptions, protocole moteur

### Avantages
- Interface comme une vraie CNC commerciale (pendant, DRO, etc.)
- Modification de l'interface sans toucher au code Teensy
- Widgets PyQt6 adaptés : `QLCDNumber`, `QDial`, jogs virtuels

---

## Points techniques importants

### TMC5160
- Communication **SPI** (pas seulement STEP/DIR)
- **stallGuard** : détection de fin de course sans capteur physique — très utile sur fraiseuse
- Librairie de référence : **TMCStepper**

### Encodeurs AMT10E2
- Le Teensy a des **pins d'interruption matérielle** sur presque toutes ses broches
- Essentiel pour compter les impulsions sans en perdre à haute vitesse
- C'est là que l'Arduino montre ses limites

### Servos en vitesse constante
- Teensy : timers hardware dédiés → beaucoup moins de jitter qu'un Arduino Uno

### Debouncing
- Recommandation : **hardware + software** sur boutons critiques (environnement bruité)
- Librairie : **Bounce2**

### Protocole série — point critique
> Sur une machine-outil, une commande perdue ou corrompue peut avoir des conséquences physiques.

Format à définir (JSON ou trames binaires) :
- Commandes : `JOG X+0.1`, `HOME X`, `SET_FEEDRATE 500`
- Statuts retournés : position X/Y/Z, état fins de course, alertes stallGuard

---

## Setup complet recommandé

| Outil | Rôle |
|---|---|
| **VS Code + PlatformIO** | Code Teensy (C++) |
| **Qt Designer** (`pyqt6-tools`) | Design de l'interface visuellement |
| **VS Code + Python** | Code Python / logique applicative |
| **pyserial** | Communication USB série |
| **Git** | Versionnement |
| **Remote - SSH** (extension VS Code) | Édition directe sur Raspberry Pi |

```bash
pip install pyqt6-tools
pyqt6-tools designer   # pour lancer Qt Designer
```

---

## Structure de projet recommandée

```
fraiseuse semi-cnc/
├── CLAUDE.md          ← contexte pour Claude (lu automatiquement)
├── JOURNAL.md         ← décisions d'architecture
├── teensy/            ← projet PlatformIO
│   ├── platformio.ini
│   └── src/main.cpp
└── python/            ← application PyQt6
    ├── main.py
    ├── mainwindow.ui
    └── serial_comm.py
```

### Le fichier `CLAUDE.md`
Fichier clé pour l'efficacité de Claude d'une conversation à l'autre. Doit contenir :
- La plateforme Teensy utilisée (4.1, 4.0, etc.)
- Les drivers moteur
- Le pinout
- Le format du protocole série
- Les librairies utilisées

---

## Gestion de la mémoire et des workspaces

- Claude conserve des fichiers mémoire `.md` par projet dans `~/.claude/projects/`
- La mémoire est rattachée au **répertoire ouvert dans VS Code**
- **Chaque projet devrait avoir son propre dossier ouvert** pour isoler la mémoire
- Le fichier `CLAUDE.md` est lu automatiquement à chaque conversation — c'est le mécanisme principal de continuité

### Workspace créé
`U:\Claude\fraiseuse semi-cnc` — répertoire dédié à ce projet.

---

## Prochaines étapes
- [ ] Ouvrir `U:\Claude\fraiseuse semi-cnc` dans VS Code comme workspace principal
- [ ] Définir le protocole de communication série Teensy ↔ Python
- [ ] Créer la structure de projet (dossiers `teensy/` et `python/`)
- [ ] Initialiser le projet PlatformIO pour Teensy
- [ ] Démarrer le template PyQt6 de base
