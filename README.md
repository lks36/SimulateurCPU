# Simulateur de CPU et Gestionnaire de Mémoire

## Présentation
Ce projet universitaire consiste en la réalisation d'un simulateur d'unité centrale de traitement (CPU) capable d'exécuter un langage pseudo-assembleur. Il reproduit les mécanismes fondamentaux d'un processeur réel, de la gestion des registres à l'allocation dynamique de la mémoire.

## Fonctionnalités Clés
- **Interpréteur de Code** : Analyseur syntaxique (Parser) capable de lire des sections `.DATA` et `.CODE` et de résoudre les étiquettes (labels).
- **Gestionnaire de Mémoire (Memory Manager)** : 
  - Division de la mémoire en segments (Data, Code, Stack, Extra).
  - Algorithmes d'allocation : *First Fit*, *Best Fit* et *Worst Fit*.
  - Fusion de segments libres pour limiter la fragmentation.
- **Modes d'Adressage** : Implémentation de 5 modes (Immédiat, Registre, Direct, Indirect, et Segment Override) via des expressions régulières (Regex).
- **Cycle d'Exécution** : Simulation complète du cycle *Fetch-Decode-Execute* avec gestion des drapeaux (Zero Flag, Sign Flag).

## Architecture Technique
- **Langage** : C (Norme C99/C11)
- **Structures de Données** : 
  - Table de hachage générique (gestion des collisions par sondage linéaire et "Tombstones").
  - Listes chaînées pour la gestion des blocs de mémoire libres.
  - Pile (Stack) pour les opérations PUSH/POP.
- **Outils** : POSIX Regex pour le parsing des instructions.

## Instructions Implémentées
- `MOV`, `ADD`, `CMP` : Manipulation de données et calculs.
- `JMP`, `JZ`, `JNZ` : Contrôle du flux d'exécution.
- `PUSH`, `POP` : Gestion de la pile.
- `ALLOC`, `FREE` : Gestion dynamique de l'Extra Segment (ES).

## 🛠️ Compilation et Exécution

Le projet est structuré de manière modulaire pour faciliter la maintenance et les tests. Un `Makefile` est fourni pour automatiser la gestion des dépendances.

### Prérequis
- Compilateur `gcc`
- Bibliothèque standard `libc` (avec support `regex.h`)

### Commandes
- **Compiler l'application principale** :
  ```bash
  make main
