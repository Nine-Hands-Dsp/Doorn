# Doorn (NINE HANDS | DSP) — VST3

Distorsion high gain au format pédale simple, reproduction de la
**Lichtlaerm Audio Thorn**, la pédale signature de Lennart Bossu (Amenra), elle-même rétro-conçue à partir de la
**Providence Stampede SDT-1**. *Doorn* veut dire « épine » en néerlandais.
Usage personnel. Windows 11 et macOS (binaire universel).

<p align="center">
  <a href="https://github.com/Nine-Hands-Dsp/Doorn/releases/latest/download/Doorn-VST3-Windows.zip"><img alt="Télécharger pour Windows" src="https://img.shields.io/badge/%E2%AC%87%20T%C3%A9l%C3%A9charger-Windows-0078D6?style=for-the-badge"></a>
  <a href="https://github.com/Nine-Hands-Dsp/Doorn/releases/latest/download/Doorn-VST3-macOS.zip"><img alt="Télécharger pour macOS" src="https://img.shields.io/badge/%E2%AC%87%20T%C3%A9l%C3%A9charger-macOS-111111?style=for-the-badge&logo=apple&logoColor=white"></a>
  <br>
  <a href="https://github.com/Nine-Hands-Dsp/Doorn/releases/latest/download/Doorn-Notice.pdf"><img alt="Télécharger la notice (PDF)" src="https://img.shields.io/badge/%F0%9F%93%96%20Notice-PDF-919496?style=for-the-badge"></a>
  <br>
  <a href="https://github.com/Nine-Hands-Dsp/Doorn/releases/latest"><img alt="Dernière version" src="https://img.shields.io/github/v/release/Nine-Hands-Dsp/Doorn?label=derni%C3%A8re%20version&color=b8a37a"></a>
</p>

## Installation

Les boutons ci-dessus téléchargent toujours la **dernière version** (Release GitHub publiée à chaque build de `main`).

- **Windows** : dézipper `Doorn-VST3-Windows.zip` et copier `Doorn.vst3` dans `C:\Program Files\Common Files\VST3\`
- **macOS** : dézipper `Doorn-VST3-macOS.zip` et copier `Doorn.vst3` dans `~/Library/Audio/Plug-Ins/VST3/`
  (premier lancement : `xattr -cr ~/Library/Audio/Plug-Ins/VST3/Doorn.vst3` si macOS le bloque)
- Relancer l'analyse des plugins. Le plugin apparaît comme **Doorn (NINE HANDS | DSP)** ; les projets existants le retrouvent avec tous leurs réglages.

## La pédale d'origine et le plugin

| Élément | Thorn / SDT-1 | Dans Doorn |
|---|---|---|
| GAIN | Distorsion dure, mur de son, passe en fuzz au-delà de 13 h | 20 à 72 dB répartis sur deux étages d'ampli op (alimentation ~18 V par pompe de charge : grande réserve), écrêtage par diodes à la masse, sur-échantillonnage x4 avec écrêteurs anti-repliement (ADAA) |
| LEVEL | Volume de sortie | Courbe audio, 0 = coupé, midi ≈ niveau de la guitare (−25 dB), maxi −10 dB (v2.15 : marge en sortie sur toute la course) |
| BOOST (petit potard + footswitch) | Boost réglable, **additif** : s'ajoute au gain, resserre l'attaque, ajoute du sustain | 0 à +20 dB avant l'écrêtage, passe-haut d'entrée remonté (grave plus serré), +1,5 dB de niveau au plus ; v2.11 : plus de poussée de médiums (mesure sur la vraie Thorn : le boost ne change presque pas la couleur, seulement un grave plus serré et un peu plus de grain en haut) |
| TREBLE / MIDS / BASS | EQ actif 3 bandes | Grave : plateau 90 Hz + bosse de punch vers 120 Hz −15 / +18 dB (après la distorsion, sans compression), cloche 750 Hz ±5 dB (v2.11), plateau aigu 4 kHz doux autour de midi, −5 / +16 dB aux butées (v2.11 : sur la vraie Thorn, 3 graduations de treble changent très peu le son) ; midi = plat, courses BASS / TREBLE douces et continues (fines autour de midi, plus fortes vers les butées, aucun saut). **v2.11 : voix calée sur la vidéo Dirty Demos « Michael Klein SPD100 + Lichtlaerm Audio Thorn »** (voir plus bas) : bas-médiums et médiums 400 Hz – 1,5 kHz allégés, présence vers 2,2 kHz, poussée 900 Hz retirée. Avant : voix stoner médium / médium-aigu (poussée 900 Hz et 2,3 kHz) sur un bas présent (grave vers 100 Hz, corps vers 220 / 320 Hz), grave serré (grave retiré avant l'écrêtage puis rendu après, liaison de 180 Hz entre étages, grave qui suit le jeu), mordant à 1,8 kHz avant l'écrêtage, définition sans sifflement (v2.8) : aigus limités avant l'écrêtage (2 pôles 5 / 7 kHz, pour ne pas amplifier le souffle et l'intermodulation des partiels aigus de la corde), clarté vers 3,2 kHz après l'écrêtage, là où ses harmoniques restent nettes, sortie refermée au-dessus de ~9 kHz, écrêtage franc, mordant intégré sans boost, sortie qui suit la dynamique du jeu (pas d'effet compressé), caractère metal / stoner : couche fuzz façon Big Muff mélangée légèrement sous la distorsion (v2.15 : environ 13 % du son, pour la texture ; avant ~40 %) et bosse de corps vers 220 Hz (gras) |
| EQ off / on | L'EQ off retire complètement le tone stack du circuit (Lennart dans un Ampeg V4, EQ off) | En OFF, l'EQ n'est plus dans le trajet du signal (fondu sans clic) |
| Mode SDT-1 | Le son de la Stampede d'origine | Silicium à la masse, médiums qui chantent, harmoniques paires |
| Mode SDT-2 | Plus compressé et saturé (Living Gate) | Étage de compression en plus, plus sombre, même volume que SDT-1 (v2.16) |
| Mode Laerm | Plus ouvert, plus direct | Seuil d'écrêtage plus haut (type LED), bande passante plus large, même volume que SDT-1 (v2.16) |
| Footswitch BYPASS | Relais « Smart Relay » | Clic = on / off ; **appui maintenu** (> 0,45 s) quand l'effet était coupé = fonctionnement momentané (désactivable, clic droit > Settings) |
| Footswitch BOOST | Boost on / off | Même comportement que BYPASS |
| Noise gate (ajout Doorn) | — (la Thorn est bruyante à fort gain) | Clic droit > *Noise gate* : Off / Light / Medium (défaut) / Strong. Détection sur l'entrée, en avance de 32 échantillons sur le signal traité (attaques intactes), hystérésis 6 dB, maintien 60 ms, fermeture douce 120 ms ; n'agit que sur l'effet. Souffle de −80 dBFS en entrée : −9 dBFS en sortie sans gate, silence avec |
| LED | Une seule LED entre les footswitchs ; le potard BOOST translucide s'allume avec le boost | v3.0 : LED blanche = effet actif ; potard BOOST en verre dépoli blanc qui s'allume (avec halo) quand le boost est enclenché. Halos réglables : clic droit > *Lights* (LED glow / Boost knob glow : Low, Medium, High) |

Sources : fiche Lichtlaerm Audio (lichtlaermaudio.com/shop/thorn), test Guitar Pedal X, Effects Database,
manuel Providence SDT-2, tests MusicRadar / Vintage Guitar de la Stampede DT, Premier Guitar (Rig Rundown Amenra,
Kurt Ballou), guide de montage GCI Brutalist Jr (clone de la SDT-1), photo du circuit imprimé de la Thorn
(LT1054 = pompe de charge, ATtiny85 + 2 relais Omron G6K = Smart Relay).

## Interface (v3.0 : design v017)

- **Visuel** : design fourni par l'utilisateur (`gui/`, `gui/manifest.json`, archive d'origine dans `design/v017/`), image **965 × 1800 px** plein cadre, fond transparent seulement autour des jacks. Calques : fond (textes, logo NINE HANDS | DSP, jacks, footswitchs, cadre de l'écran et ombres portées des commandes intégrés), halo du BOOST, potards (centre fixe, couronne + flèche qui tournent sur 270°, reflet fixe par-dessus), inverseurs (3 images chacun, image du milieu ~55 ms pendant une bascule haut ↔ bas), LED. Au repos, le rendu du plugin est identique à l'image de référence du design.
- **Fenêtre** : repère 482,5 × 900 unités (image / 2), ouverte à **493 px de haut** (« 60 % »), impossible de la réduire en dessous, proportions bloquées, jusqu'à 150 %. Clic droit > *Plugin size* : 60 % / 75 % / 100 % pour cette fenêtre, et *Default size (new windows)* pour la taille d'ouverture de toutes les nouvelles fenêtres (`NINE HANDS | DSP/Doorn/settings.xml` du dossier de données de l'utilisateur, comme Octopus).
- **Écran = barre de presets + ligne d'état au même endroit** (règle NINE HANDS | DSP, comme Echoflex v2), à la **taille de référence de Strong Pickle** (v3.2) : pastille 400 × 38 unités (800 × 76 px dans l'image), Poppins Medium 23 (46 px), dessinée par-dessus le petit écran du design avec sa matière (contour sombre, liseré gris clair, face noire) ; au repos, la barre de presets (◀ nom du preset ou *Edited* ▶ ; flèches = preset précédent / suivant, clic sur le nom = menu des presets). Dès qu'un réglage bouge (souris, molette, automation, MIDI, footswitch), la ligne d'état prend sa place dans la même pastille, même police Poppins, même taille (potards en %, inverseurs, footswitchs, messages du MIDI Learn) ; elle reste 2,5 s après le dernier mouvement puis la barre de presets revient. Fondu croisé ~120 ms à l'apparition, ~300 ms au retour ; les flèches s'effacent pendant la ligne d'état. Un clic sur l'écran pendant la ligne d'état fait revenir la barre de presets et agit.
- **Lumières** : clic droit > *Lights* : halo de la LED (lentille et halo séparés, seule le halo change) et halo du potard BOOST, chacun en Low / Medium (défaut) / High ; enregistré avec le projet, non automatisable.
- **Menus** : design des menus d'Octopus (`NineHandsMenu.h`, en-têtes avec pastille couleur sable), toujours ouverts là où l'on clique (le menu de presets ouvert depuis l'écran reste accroché à l'écran).
- **Footswitchs** : bascule à l'appui, bouton assombri tant qu'on appuie.
- **Anti-gel v2** : moteur de rendu logiciel forcé sous Windows (réappliqué à chaque nouvelle fenêtre hôte), fenêtre opaque, timer 30 Hz qui ne redessine que ce qui a changé (aucun redessin au repos), aucun calcul lourd dans le dessin (cache du fond reconstruit par le timer seulement), chien de garde qui relance le rendu si un redessin n'arrive pas en 1,5 s, jamais de redimensionnement forcé par la fenêtre ; côté audio, aucun appel à l'hôte ni allocation depuis le thread audio. JUCE 8.0.15.

## MIDI

v2.12.0 : MIDI Learn sur toutes les commandes (clic droit sur une commande ou menu MIDI), assignations sauvegardées avec le projet.
Aucun CC assigné par défaut : clic droit > MIDI > Learn. Clic droit > *MIDI* > *Clear all MIDI assignments* efface toutes les assignations.

| Commande apprise | Fonction |
|---|---|
| Gain / Boost (niveau) / Level | Continu (0-127 = toute la course) |
| Treble / Mids / Bass | Continu |
| CC n+32 | Poids faible 14 bits d'un potard appris sur un CC 0 à 31 (si la pédale l'envoie) |
| EQ | Bascule (≥ 64 on) |
| Mode | Sélecteur (0-42 SDT-2, 43-84 Laerm, 85-127 SDT-1) |
| Footswitch BOOST | Footswitch |
| Footswitch BYPASS | Footswitch |
| Program Change 1 à 13 | Presets d'usine |

Footswitchs MIDI à accrochage (127 = on / 0 = off, par défaut) ou momentanés (bascule à chaque appui) : clic droit > Settings.
Pédale d'expression sans saccades : rampe vers chaque nouvelle valeur sur l'intervalle mesuré entre deux CC,
course de la pédale apprise automatiquement (0 et 100 % exacts même si elle n'envoie que 2 à 116), zones mortes aux butées.
Tous les paramètres sont aussi automatisables dans le DAW.

## Presets d'usine

1. Signature Start (réglage de départ du test Guitar Pedal X) · 2. Wall of Thorns · 3. V4 EQ Bypass · 4. Living Gate Saturation ·
5. Laerm Open · 6. Laerm Lead Boost · 7. Stampede Crunch · 8. Hardcore Bite · 9. Sludge Sustain · 10. Thorn Reference · 11. Thorn Reference Boost · 12. Thorn Video Mids · 13. Thorn Video Mids Boost (réglages relevés dans la vidéo de référence, sur lesquels le son a été calé)

## Calage sur la vraie Thorn (v2.11)

Référence : vidéo YouTube *Michael Klein SPD100 + Lichtlaerm Audio Thorn* (chaîne Dirty Demos ; Carvin DC200 en B standard, baffle Celestion Redback, SM57 + M201).
L'audio de toute la vidéo a été capturé et mesuré en tiers d'octave ; les réglages ont été relevés image par image (le plan est fixe sur la pédale) :

| Passage | Gain | Level | Treble | Mids | Bass | Boost |
|---|---|---|---|---|---|---|
| 0:08 – 0:36 | ~10 h 30 | ~13 h | ~13 h 30 | ~12 h | ~15 h | off |
| 0:39 – 0:55 | idem | idem | idem | idem | idem | on |
| 5:16 – 8:19 | ~10 h | ~14 h 30 | ~10 h 30 | ~13 h 30 | ~13 h 30 | on |
| 9:34 – 11:36 | ~11 h | ~15 h | ~10 h | ~13 h 30 | ~13 h 30 | off |

Doorn passe par une vraie guitare DI transposée en B et deux réponses de baffle 4x12 + SM57 ; la voix fixe est ajustée par moindres carrés
(courbe lisse, pour ne pas recopier le baffle de la vidéo) et les courses MIDS / TREBLE / BOOST sont réglées sur les différences mesurées entre passages
(ce qui neutralise en grande partie ampli, baffle et micros). Écart moyen au spectre de la vraie Thorn : 4,9 → 3,0 dB (passage de référence), 6,7 → 3,5 dB et 7,8 → 3,9 dB (réglages « mids »),
soit l'ordre de grandeur de la différence entre les deux baffles utilisés : c'est la limite de ce que la méthode peut trancher.
Limites : l'ampli SPD100 colore aussi le son de la vidéo ; le niveau du boost et la position des inverseurs (EQ on, SDT-1 supposés) ne sont pas lisibles en 480p ;
YouTube normalise le volume (les niveaux relatifs ne sont pas mesurables). Données : `tools/video_reference.json`, comparaison : `tools/video_match.py`.

## Fichiers

- `DoornDSP.h` : moteur audio (sans JUCE, testé par `tests/test_dsp.cpp`, `tests/test_noise.cpp`, `tests/test_haze.cpp`).
- `PluginProcessor.*`, `PluginEditor.*`, `NineHandsMenu.h`, `WigaWindowSize.h` : plugin JUCE 8.0.15.
- `gui/` : visuel intégré (design v017) ; `Layout.h` est généré depuis `gui/manifest.json` par `tools/layout_from_manifest.py`.
- `tools/render/` : captures de la vraie fenêtre sous Linux (`DoornRender <dossier> NineHandsLogo.svg`) ; `tools/manual.py` : notice (français).
- `design/` : visuels fournis (design v017, ancien visuel), photo du circuit imprimé, photo des switchs.
- Polices : Poppins (écran, ligne d'état, menus), Barlow Condensed (couverture de la notice) (SIL Open Font License).

## Historique

- v2.12.1 : menu de taille de fenêtre : « 60 % (default) », 75 %, 100 % (plus de taille en pixels)
- v2.13.0 : clic droit > *Plugin size* > *Default size (new windows)* : taille par défaut des nouvelles fenêtres au choix (60 / 75 / 100 %), comme dans Octopus
- v2.13.1 : plus aucun CC MIDI assigné par défaut (tout passe par le MIDI Learn)
- v2.14.0 : fin du grésillement sifflant. L'écrêtage numérique repliait ses harmoniques en sifflement inharmonique qui suit la note (mesuré à −33 / −39 dB sous la note dans l'aigu). Les écrêteurs sont maintenant anti-repliement (méthode par primitive, ADAA) avec une courbe en tangente hyperbolique, au coude plus doux et plus naturel que l'ancien coude dur : parasite ramené à −52 / −68 dB. La voix est recalée sur la vidéo de référence avec ces écrêteurs, la bosse fixe de présence à 2,9 kHz est retirée et le plateau aigu de sortie baisse de 2,5 dB. Le paramètre d'automation de l'interrupteur s'appelle désormais « Footswitch » (au lieu de « Bypass Footswitch ») : Octopus affichait l'état de Doorn à l'envers. Même identifiant interne, les sessions, automations et MIDI Learn existants sont conservés.
- v2.15.0 : LEVEL 10 dB plus bas sur toute sa course : la sortie ne sature plus dans le DAW au-delà de midi (avec les presets d'usine, plus de saturation même LEVEL à fond). Couche fuzz réduite à environ 13 % du son (avant environ 45 %), juste pour la texture ; la couche diodes compense : même niveau, même équilibre tonal face à la vidéo de référence.
- v2.16.0 : les trois modes sortent au même volume (mesuré à travers un baffle : Laerm sortait ~4 dB plus fort que SDT-1 et SDT-2 ~2,7 dB moins fort ; écart ramené à ±0,5 dB dès 10 h de gain).
- v3.0.0 : nouveau design (v017) : boîtier noir aux fissures sable, potards noirs à 3 calques, potard BOOST en verre dépoli blanc qui s'allume avec le boost, LED blanche, écran de presets, ligne d'état en bulle ; halos réglables (clic droit > *Lights*) ; menus au design d'Octopus, ouverts là où l'on clique ; notice en français sur le gabarit NINE HANDS | DSP. Son inchangé.
- v3.1.0 : barre de presets et ligne d'état réunies dans l'écran (la ligne d'état remplace le preset le temps des réglages puis disparaît, fondu croisé, même police et même taille) ; plus de bulle par-dessus.
- v3.2.0 : barre de presets / ligne d'état agrandie à la taille de référence de Strong Pickle (400 × 38 unités, police 23 : environ 1,4 fois plus haute et 1,5 fois plus grand qu'avant), même matière que l'écran du design.
