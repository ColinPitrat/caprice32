# Caprice32 — fork pour corriger le clavier AZERTY

## Ce projet, et pourquoi il est séparé

Ce dossier est un **fork indépendant** de [Caprice32](https://github.com/ColinPitrat/caprice32),
un émulateur Amstrad CPC (GPL-2.0). Il n'a **aucun lien de code** avec
`~/projets/cpc-keeper` (le catalogueur de jeux CPC développé à côté, licence
non déclarée) — les deux projets restent volontairement séparés pour ne
jamais mélanger la GPL-2.0 de Caprice32 avec le code de CPC Keeper. CPC
Keeper se contente de lancer le binaire `cap32` compilé ici comme un
processus externe (voir `commands::launch::spawn_emulator` côté CPC Keeper),
exactement comme n'importe quel utilisateur lancerait Caprice32 depuis un
terminal.

**But de ce fork** : corriger un bug qui empêche de taper les caractères
accentués français (é/è/ç/à) dans l'émulateur avec un clavier hôte AZERTY.
Corrigé et vérifié (voir ci-dessous). Une fois la PR proposée et acceptée en
amont, ce fork n'aura plus besoin d'être maintenu.

## État du dépôt

- Remotes : `origin` = `https://github.com/sylvercis/caprice32` (ton fork),
  `upstream` = `https://github.com/ColinPitrat/caprice32` (le projet
  original).
- **Note** : ton fork GitHub contient une branche `aligned` avec des commits
  sans rapport (vue de pile dans le debugger, nettoyages) — ne pas y
  toucher, ne pas la fusionner par erreur.
- Le fix vit sur la branche `azerty-accents-fix`, créée depuis `master`.
- Binaire : compilé avec `make APP_PATH="$PWD"` (voir section Build) —
  nécessaire ici car `~/.local/bin/cap32` (utilisé par CPC Keeper) doit
  pointer vers `~/projets/caprice32/cap32`.

## Le bug corrigé : diagnostic réel (vérifié empiriquement)

**⚠️ Le diagnostic initial de ce document supposait que le mapping clavier
dans les fichiers `.map` était en cause (mauvais keycodes SDL utilisés pour
é/è/ç/à). C'était faux.** Il a été invalidé par des tests réels (clavier
physique + `xdotool` + logs verbeux `-v` de Caprice32), documentés ici pour
ne pas reproduire l'erreur.

### Cause réelle : IBus/XIM avale les événements clavier

Avec `system.keyboard=1` (clavier CPC français) et
`control.kbd_layout=keymap_fr_linux.map`, en appuyant sur é/è/ç/à, **aucun
événement `SDL_KEYDOWN` n'atteignait Caprice32** (vérifié : aucune ligne
`Keyboard: pressed: ...` dans le log verbeux pour ces touches, alors que
cette ligne est inconditionnelle pour tout `SDL_KEYDOWN` reçu — voir
`cap32.cpp` autour de `case SDL_KEYDOWN`).

Cause : SDL2 laisse le mode « text input » actif par défaut
(`SDL_StartTextInput()` implicite), ce qui donne la main à l'input method du
système (ici **IBus**, actif via XIM — `ibus-daemon --xim`,
`XMODIFIERS=@im=ibus`). IBus intercepte (`XFilterEvent`) les touches
« composables » comme les lettres accentuées Latin-1 pour son moteur de
compose, et ne laisse jamais l'événement brut atteindre SDL. Les touches
ASCII simples (chiffres, ponctuation courante) ne sont pas interceptées,
d'où le fonctionnement correct de Maj+chiffre et la confusion initiale.

**Le mapping `.map` lui-même (`CPC_FR_eACUTE SDLK_2`, etc.) était déjà
correct** : sur ce système, SDL verrouille les touches de la rangée des
chiffres (0-9) sur leur keycode chiffre `SDLK_0`..`SDLK_9`, quel que soit le
caractère réellement affiché sur la touche AZERTY — comportement volontaire
de SDL2 pour cette rangée spécifique (pas documenté nulle part de façon
évidente, découvert empiriquement). Shift est correctement reporté à part
via `keysym.mod`. C'est pour ça que `CPC_FR_eACUTE SDLK_2` (sans modificateur)
fonctionne : SDL rapporte bien `SDLK_2` + `mod=NONE` pour une pression non
shiftée de la touche é. Tenter d'utiliser le vrai codepoint Latin-1 (233 pour
é) — ce qui fonctionne pour `ù` (`SDLK_uGRAVE`=249, hors rangée chiffres,
non concernée par ce verrouillage) — ne fonctionne PAS pour les touches de la
rangée des chiffres : SDL ne le rapporte jamais.

### Le fix

Un seul changement fonctionnel, dans `src/cap32.cpp`, juste après
`SDL_Init()` :

```cpp
SDL_StopTextInput();
```

Les widgets GUI (`wg_editbox.cpp`, `wg_textbox.cpp`) gèrent déjà
`SDL_StartTextInput()`/`SDL_StopTextInput()` eux-mêmes quand un champ de
saisie a le focus — cet ajout fixe seulement l'état initial (aucun champ
actif au démarrage) et ne casse donc pas la saisie de texte dans les menus.

Tous les fichiers `.map` et `src/keyboard.cpp` restent **identiques à
l'original** (`git diff` ne montre que `src/cap32.cpp`).

### Vérifié avec

- `make unit_test` : 152/152 tests passent (aucun test à modifier, la
  logique de mapping n'a jamais été le problème).
- Clavier physique réel (AZERTY, XKB layout `fr`, variante `latin9,oss`) :
  é, è, ç, à, ù tapées et confirmées fonctionnelles à l'invite BASIC.

### Cas vérifiés et compris, non buggés

- **£ (Maj+$)** : ne fonctionne pas en clavier français (`system.keyboard=1`)
  — et c'est correct. La table `cpc_kbd` dans `keyboard.cpp` (section
  « French CPC keyboard ») donne `0xff` (invalide) pour `CPC_POUND`, alors
  que la section « original CPC keyboard » (anglais, `system.keyboard=0`) lui
  donne `0x30` (valide). Le vrai clavier Amstrad CPC français n'a jamais eu
  de touche £ physiquement (France = Franc, pas Livre Sterling). Rien à
  corriger.
- **Suppr (pavé numérique) + Shift = "."** : comportement voulu, reproduit le
  clavier CPC d'origine. Pas un bug.

## Hors scope (pas traité par ce fix)

- **°, ¨, µ, §** : aucune touche CPC n'existe pour ces caractères dans
  `keyboard.h`/`CPC_KEYS`. Il faudrait (a) vérifier que la ROM/police du CPC
  a un glyphe à ces positions avant d'ajouter quoi que ce soit, (b) ajouter
  une nouvelle entrée `CPC_KEYS` + les scancodes correspondants pour chaque
  variante de clavier CPC (anglais/français/espagnol) dans `cpc_kbd`. Pour
  celles situées sur des touches Shift+symbole (°, ¨), le host key à utiliser
  dans le `.map` est le keysym **non-shifté** de la touche (SDL rapporte
  toujours le niveau 0 pour les touches hors rangée chiffres, comme démontré
  pour `$`/£ ci-dessus) combiné à `MOD_PC_SHIFT`.
- **Pavé numérique** (`/ * - +`) : non mappé du tout, dans aucun fichier
  `.map`. Le chargeur de mapping (`InputMapper::load_layout` dans
  `keyboard.cpp`) n'autorise qu'**une seule touche hôte par touche CPC** (un
  `std::map`, la dernière ligne du fichier gagne silencieusement en cas de
  doublon) — donc lier ces touches aux équivalents déjà mappés sur la ligne
  principale les écraserait plutôt que de les compléter. Corriger ça
  proprement demanderait de passer à une structure autorisant plusieurs
  touches hôte par touche CPC (`std::multimap` ou équivalent).

## Build

```
make APP_PATH="$PWD"
```

`APP_PATH` doit être passé explicitement (voir `README.md`) : sans lui,
`chAppPath` est déterminé via `getcwd()` à l'exécution, ce qui casse dès que
le binaire n'est pas lancé depuis ce répertoire exact — et surtout, un
`obj/linux/src/cap32.o` déjà compilé avec un ancien `APP_PATH` (ex.
`~/apps/caprice32`, avant le déplacement de ce dossier) ne sera **pas
recompilé automatiquement** par `make` (le suivi de dépendances ne détecte
pas un changement de flag de compilation) — il faut `touch src/cap32.cpp
main.cpp` avant si ça arrive.

Binaire produit : `./cap32`.

## Tests

```
make unit_test
```

## Vérification manuelle

```
./cap32 -v -O system.keyboard=1 -O control.kbd_layout=keymap_fr_linux.map 2>&1 | grep --line-buffered "Keyboard: pressed"
```

Appuyer sur é, è, ç, à, ù. Chaque touche doit produire une ligne
`CPC key: CPC_FR_... - CPC scancode: <valeur> != 255`, et le caractère doit
s'afficher à l'invite BASIC dans la fenêtre de l'émulateur.

## Republier le lien symbolique

Pour que CPC Keeper utilise ce binaire :

```
ln -sf ~/projets/caprice32/cap32 ~/.local/bin/cap32
```

## Après le fix : proposer la PR upstream

Préparer une pull request vers `ColinPitrat/caprice32` (`master`) avec
uniquement l'ajout de `SDL_StopTextInput()` dans `src/cap32.cpp`, et une
description qui explique le mécanisme IBus/XIM (voir ci-dessus) avec les
logs à l'appui. C'est un fix minimal, à fort impact, qui affecte
probablement tout utilisateur Linux avec un IME actif (IBus/Fcitx), pas
seulement les claviers AZERTY.
