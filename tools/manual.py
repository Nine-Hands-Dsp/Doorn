# DOORN (NINE HANDS | DSP) - notice PDF en francais, illustree avec le rendu reel du plugin (design v017).
# Usage : python3 tools/manual.py [<dossier des captures (tools/render)>] <fichier PDF de sortie>
#   avec captures : recompose les images et les copie dans tools/manual_assets/ ;
#   sans captures : reprend les images composees de tools/manual_assets/.
# Captures : DoornRender <dossier> NineHandsLogo.svg (exporte aussi nine_hands_logo_blanc.png pour la couverture).
import os, sys, shutil
import numpy as np
from PIL import Image, ImageDraw, ImageFont
from reportlab.lib.pagesizes import A4
from reportlab.lib.units import mm
from reportlab.lib import colors
from reportlab.lib.styles import ParagraphStyle
from reportlab.platypus import (BaseDocTemplate, PageTemplate, NextPageTemplate, Frame, Paragraph, Spacer, Image as RLImage,
                                Table, TableStyle, PageBreak, CondPageBreak, KeepTogether)
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SHOTS = sys.argv[1] if len(sys.argv) > 2 else None
OUT = sys.argv[-1]
VERSION = "3.1.0"
TMP = os.path.dirname(os.path.abspath(OUT))
FONTS = os.path.join(ROOT, "assets", "fonts")

for n, f in (("Pop", "Poppins-Regular"), ("PopL", "Poppins-Light"), ("PopM", "Poppins-Medium"), ("PopB", "Poppins-Bold"),
             ("Title", "BarlowCondensed-ExtraBold")):
    pdfmetrics.registerFont(TTFont(n, os.path.join(FONTS, f + ".ttf")))

ACC = colors.HexColor("#9C7F4E"); ACC_L = colors.HexColor("#CDB995")      # sable des fissures du visuel
BAND = colors.HexColor("#CDB995")
DARK = colors.HexColor("#161A26"); GREY = colors.HexColor("#646A78"); LIGHT = colors.HexColor("#F3EEE4")

ASSETS = os.path.join(ROOT, "tools", "manual_assets")
NAMES = ["doorn_annotated.png", "doorn_modes.png", "doorn_leds.png", "doorn_bar.png", "doorn_cover_cutout.png", "nine_hands_logo_blanc.png"]
if SHOTS:
    def shot(name): return Image.open(os.path.join(SHOTS, name)).convert("RGB")
    src = shot("doorn_status.png")
    k = src.height / 900.0                     # repere du plugin v3 : 482,5 x 900 unites (image 965 x 1800 / 2)

    # ---------- capture annotee (sans la bulle de la ligne d'etat) ----------
    base = shot("doorn_bypass.png")
    marks = [(1, 40, 95), (2, 280, 102), (3, 442, 95), (4, 40, 288), (5, 190, 288), (6, 442, 288),
             (7, 40, 470), (8, 442, 470), (9, 96, 601.5), (10, 200, 662), (11, 36, 735), (12, 448, 735), (13, 88, 40), (14, 395, 40)]
    ann = base.copy(); dr = ImageDraw.Draw(ann)
    fnt = ImageFont.truetype(os.path.join(FONTS, "Poppins-Bold.ttf"), 30)
    for n, x, y in marks:
        cx, cy, r = x * k, y * k, 24
        dr.ellipse((cx - r, cy - r, cx + r, cy + r), fill=(255, 255, 255), outline=(156, 127, 78), width=5)
        dr.text((cx, cy + 1), str(n), font=fnt, fill=(22, 26, 38), anchor="mm")
    annotated = os.path.join(TMP, "doorn_annotated.png"); ann.save(annotated)

    def crop_u(im, box): return im.crop(tuple(int(v * k) for v in box))
    gap = 18
    def strip(parts, path):
        s = Image.new("RGB", (sum(p.width for p in parts) + gap * (len(parts) - 1), max(p.height for p in parts)), (255, 255, 255))
        x = 0
        for p in parts: s.paste(p, (x, 0)); x += p.width + gap
        s.save(path); return path

    box_mode = (300, 440, 455, 562)            # inverseur de mode : SDT-2 (haut), Laerm (milieu), SDT-1 (bas)
    modes_img = strip([crop_u(shot(n), box_mode) for n in ("doorn_sdt2.png", "doorn_laerm.png", "doorn_bypass.png")], os.path.join(TMP, "doorn_modes.png"))
    box_led, box_bst = (186, 612, 296, 712), (176, 72, 306, 192)
    leds_img = strip([crop_u(base, box_led), crop_u(src, box_led), crop_u(src, box_bst), crop_u(shot("doorn_boost.png"), box_bst)],
                     os.path.join(TMP, "doorn_leds.png"))
    bar = Image.new("RGB", (int(290 * k), int(150 * k)), (255, 255, 255))
    bar.paste(crop_u(base, (96, 578, 386, 625)), (0, 0)); bar.paste(crop_u(src, (96, 578, 386, 625)), (0, int(56 * k)))
    bar = bar.crop((0, 0, bar.width, int(56 * k) + int(47 * k)))
    bar_img = os.path.join(TMP, "doorn_bar.png"); bar.save(bar_img)

    # ---------- couverture : pedale detouree avec le canal alpha du fond (boitier + jacks), reflet au sol, halo sable tres doux ----------
    ex = Image.open(os.path.join(SHOTS, "doorn_exact_on.png")).convert("RGB")
    bg_a = Image.open(os.path.join(ROOT, "gui", "Doorn_Background.png")).split()[3].resize(ex.size, Image.LANCZOS)
    cut = Image.new("RGBA", ex.size, (0, 0, 0, 0)); cut.paste(ex, (0, 0), bg_a)
    cut = cut.crop(cut.getbbox())
    cw, ch = cut.size
    pad = int(cw * 0.16)
    canvas = Image.new("RGBA", (cw + 2 * pad, int(ch * 1.30) + pad), (0, 0, 0, 255))
    yy, xx = np.mgrid[0:canvas.height, 0:canvas.width].astype(np.float64)
    gcx, gcy = canvas.width / 2, pad + ch * 0.45
    glow = np.exp(-(((xx - gcx) / (cw * 0.62)) ** 2 + ((yy - gcy) / (ch * 0.48)) ** 2)) * 26
    edge = np.clip(np.minimum.reduce([xx, canvas.width - 1 - xx, yy, canvas.height - 1 - yy]) / (pad * 0.9), 0, 1)
    glow *= edge ** 2
    halo = np.dstack([glow * 0.80, glow * 0.72, glow * 0.58, np.full_like(glow, 255)]).astype(np.uint8)
    canvas = Image.alpha_composite(canvas, Image.fromarray(halo, "RGBA"))
    refl = cut.transpose(Image.FLIP_TOP_BOTTOM)
    ra = np.array(refl).astype(np.float64)
    fade = np.clip(1 - np.arange(refl.height) / (refl.height * 0.22), 0, 1) ** 1.6 * 0.22
    ra[..., 3] *= fade[:, None]
    canvas.alpha_composite(Image.fromarray(ra.astype(np.uint8), "RGBA"), (pad, pad + ch + 6))
    canvas.alpha_composite(cut, (pad, pad))
    canvas = canvas.crop((0, 0, canvas.width, pad + int(ch * 1.20)))
    cover = os.path.join(TMP, "doorn_cover_cutout.png"); canvas.convert("RGB").save(cover)
    LOGO_PNG = os.path.join(TMP, "nine_hands_logo_blanc.png")
    shutil.copy(os.path.join(SHOTS, "nine_hands_logo_blanc.png"), LOGO_PNG)
    os.makedirs(ASSETS, exist_ok=True)
    for n in NAMES: shutil.copy(os.path.join(TMP, n), os.path.join(ASSETS, n))
else:
    annotated, modes_img, leds_img, bar_img, cover, LOGO_PNG = [os.path.join(ASSETS, n) for n in NAMES]

def img(path, width):
    im = Image.open(path); h = width * im.height / im.width
    return RLImage(path, width=width, height=h)

# ---------- styles ----------
st = {
    "h1": ParagraphStyle("h1", fontName="PopB", fontSize=20, leading=24, textColor=ACC, spaceAfter=8, spaceBefore=4, keepWithNext=1),
    "h2": ParagraphStyle("h2", fontName="PopM", fontSize=13, leading=17, textColor=DARK, spaceAfter=4, spaceBefore=10, keepWithNext=1),
    "p": ParagraphStyle("p", fontName="Pop", fontSize=9.6, leading=14, textColor=DARK, spaceAfter=5),
    "tip": ParagraphStyle("tip", fontName="Pop", fontSize=9.0, leading=12.4, textColor=DARK, spaceAfter=3),
    "small": ParagraphStyle("small", fontName="Pop", fontSize=8.2, leading=11.5, textColor=GREY),
    "cell": ParagraphStyle("cell", fontName="Pop", fontSize=8.4, leading=11, textColor=DARK),
    "cellb": ParagraphStyle("cellb", fontName="PopM", fontSize=8.4, leading=11, textColor=DARK),
    "cells": ParagraphStyle("cells", fontName="Pop", fontSize=7.6, leading=9.6, textColor=DARK),
    "cellbs": ParagraphStyle("cellbs", fontName="PopM", fontSize=7.6, leading=9.6, textColor=DARK),
}
def P(t, s="p"): return Paragraph(t, st[s])
def bullets(items, s="p"): return [P("&bull;&nbsp;&nbsp;" + i, s) for i in items]
def table(rows, widths, header=True, small=False):
    data = [[P(c, ("cellbs" if small else "cellb") if (header and r == 0) else ("cells" if small else "cell")) for c in row] for r, row in enumerate(rows)]
    t = Table(data, colWidths=widths, repeatRows=1 if header else 0)
    style = [("VALIGN", (0, 0), (-1, -1), "TOP"), ("LINEBELOW", (0, 0), (-1, -1), 0.3, colors.HexColor("#E2DACB")),
             ("TOPPADDING", (0, 0), (-1, -1), 2 if small else 3), ("BOTTOMPADDING", (0, 0), (-1, -1), 2 if small else 3)]
    if header: style += [("BACKGROUND", (0, 0), (-1, 0), LIGHT), ("LINEBELOW", (0, 0), (-1, 0), 0.8, ACC)]
    t.setStyle(TableStyle(style)); return t

# ---------- donnees (identiques au code livre : PluginProcessor.cpp) ----------
MODES = ["SDT-2", "Laerm", "SDT-1"]
FACTORY = [  # nom, gain, boost, level, treble, mids, bass, eq, mode, boostOn
    ("Signature Start",        7.5, 7.5, 7.5, 8.0, 3.75, 7.5, 1, 2, 0),
    ("Wall of Thorns",         8.5, 6.0, 7.0, 7.0, 3.5, 7.0, 1, 2, 0),
    ("V4 EQ Bypass",           7.5, 5.0, 7.0, 5.0, 5.0, 5.0, 0, 2, 0),
    ("Living Gate Saturation", 8.0, 5.0, 8.0, 7.0, 6.0, 6.0, 1, 0, 0),
    ("Laerm Open",             6.5, 5.0, 6.0, 7.0, 5.5, 6.0, 1, 1, 0),
    ("Laerm Lead Boost",       7.0, 8.0, 6.0, 6.5, 7.0, 5.0, 1, 1, 1),
    ("Stampede Crunch",        3.0, 5.0, 7.5, 6.0, 6.0, 5.0, 1, 2, 0),
    ("Hardcore Bite",          9.5, 4.0, 6.5, 7.5, 6.5, 4.5, 1, 2, 0),
    ("Sludge Sustain",         10,  5.0, 7.0, 5.0, 2.5, 9.0, 1, 0, 1),
    ("Thorn Reference",        3.8, 2.0, 6.0, 6.5, 5.0, 8.0, 1, 2, 0),
    ("Thorn Reference Boost",  3.8, 2.0, 6.0, 6.5, 5.0, 8.0, 1, 2, 1),
    ("Thorn Video Mids",       4.2, 2.0, 8.0, 3.3, 6.5, 6.5, 1, 2, 0),
    ("Thorn Video Mids Boost", 3.3, 2.0, 7.5, 3.5, 6.5, 6.5, 1, 2, 1)]
FACT_DESC = [
    "Le réglage de départ du test Guitar Pedal X : gain, boost et level à 15 h, aigus à 8, médiums à 10 h 30, graves à 15 h, EQ en marche.",
    "Plus de gain et des médiums plus creusés : le mur dense pour les riffs lents et lourds.",
    "EQ retiré du circuit, comme Lennart Bossu avec sa Thorn dans un Ampeg V4 : direct, brut, l'ampli fait le travail de tonalité.",
    "Voix SDT-2 : plus compressée et saturée, plus sombre, des accords qui tiennent.",
    "Voix Laerm à gain modéré : ouverte, directe, idéale pour des accords qui restent lisibles.",
    "Laerm avec le boost enclenché et les médiums poussés : un solo qui passe devant le mix.",
    "Peu de gain sur la SDT-1 : le côté crunch mordant de la Stampede (elle ne devient jamais vraiment claire).",
    "Gain presque au maximum, brillant et en avant dans les médiums : l'attaque hardcore mordante de la Stampede.",
    "Gain à fond, SDT-2, boost enclenché, graves lourds et médiums creusés : un sustain long et épais pour le sludge et le doom.",
    "Les réglages de la vraie Thorn dans la vidéo de référence (0:08 - 0:36), sur lesquels le son a été calé : gain à 10 h 30, level à 13 h, aigus à 13 h 30, médiums à midi, graves à 15 h, EQ on, SDT-1, boost coupé.",
    "Les mêmes réglages avec le boost enclenché, comme dans la vidéo à 0:39 - 0:55 : grave plus serré, un peu plus de grain, même couleur.",
    "Les réglages de la vidéo à 9:34 - 11:36 : gain à 11 h, level à 15 h, aigus à 10 h, médiums et graves à 13 h 30, boost coupé.",
    "Les réglages de la vidéo à 5:16 - 8:19 : gain à 10 h, level à 14 h 30, aigus à 10 h 30, médiums et graves à 13 h 30, boost enclenché."]
def pc(v): return f"{round(v * 10)} %"

# ---------- textes ----------
T = dict(
 lang="FRANÇAIS", sub="Distorsion high gain",
 intro_t="1. Présentation",
 intro=["<b>DOORN</b> (« épine » en néerlandais) est un plugin NINE HANDS | DSP inspiré de la <b>Lichtlaerm Audio Thorn</b>, la distorsion signature de <b>Lennart Bossu</b> (Amenra, Oathbreaker). La Thorn a elle-même été rétro-conçue à partir de la <b>Providence Stampede SDT-1</b> de Lennart, une distorsion dure japonaise rare, aussi utilisée par Kurt Ballou de Converge ; Lichtlaerm y a ajouté trois voix, un boost additif au pied et un EQ actif 3 bandes que l'on peut retirer du circuit.",
        "Le son est un mur de distorsion plutôt qu'un overdrive : un mordant agressif, des médiums qui chantent, de fortes harmoniques paires qui font ressortir les harmoniques pincées, et un grave énorme qui reste défini sur les accords. C'est proche d'une tête d'ampli poussée à fond, et cela ne devient jamais vraiment clair, même gain baissé.",
        "Dans le plugin, le circuit est reproduit étage par étage : buffer d'entrée et passe-haut (tenue du grave), pré-accentuation des médiums, deux étages de gain à ampli op alimentés sous environ 18 V (la Thorn utilise une pompe de charge interne, le LT1054 visible sur son circuit imprimé, pour une grande réserve de dynamique), diodes d'écrêtage à la masse, filtrage de l'ampli op, étage de sortie façon ampli, puis l'EQ bypassable et le volume. La distorsion est calculée avec un sur-échantillonnage x4 et des écrêteurs anti-repliement (méthode par primitive), avec une courbe de diode arrondie et naturelle : aucun sifflement ni grésillement inharmonique dans l'aigu.",
        "Depuis la version 3.0, Doorn a une nouvelle façade : boîtier noir aux fissures sable, potards noirs, potard BOOST en verre dépoli blanc qui s'éclaire quand le boost est enclenché, LED blanche et écran de presets."],
 inst_t="2. Installation",
 inst=[("Windows", "Sur la page GitHub <b>github.com/Nine-Hands-Dsp/Doorn</b>, le bouton <b>Télécharger Windows</b> donne toujours la dernière version (<b>Doorn-VST3-Windows.zip</b>). Décompressez-le et copiez le dossier <b>Doorn.vst3</b> dans <b>C:\\Program Files\\Common Files\\VST3</b>."),
       ("macOS", "Le bouton <b>Télécharger macOS</b> donne <b>Doorn-VST3-macOS.zip</b> : décompressez-le et copiez <b>Doorn.vst3</b> dans <b>~/Bibliothèque/Audio/Plug-Ins/VST3</b>. Si macOS le bloque, tapez <b>xattr -cr ~/Library/Audio/Plug-Ins/VST3/Doorn.vst3</b> dans le Terminal. Le binaire est universel (Apple Silicon et Intel)."),
       ("Reaper", "Options &gt; Préférences &gt; Plug-ins &gt; VST &gt; <b>Re-scan</b>. Le plugin apparaît sous le nom <b>Doorn (NINE HANDS | DSP)</b>. Pour le piloter avec vos footswitchs et pédales d'expression MIDI, la piste doit recevoir le MIDI du contrôleur (armement ou monitoring d'entrée, entrée MIDI réglée sur votre appareil, par exemple la M-Vave Chocolate Plus). Les projets faits avec les versions précédentes retrouvent Doorn et tous leurs réglages.")],
 ov_t="3. Vue d'ensemble de l'interface",
 ov_rows=[["#", "Commande", "Rôle"],
   ["1", "GAIN", "Quantité de distorsion. Glisser vers le haut / le bas, molette, double-clic = midi, clic droit = MIDI Learn (valable pour toutes les commandes)."],
   ["2", "BOOST (petit potard)", "Niveau du boost, de 0 à +20 dB. Ne s'entend que si le footswitch BOOST est enclenché : le potard en verre dépoli s'allume alors de l'intérieur."],
   ["3", "LEVEL", "Volume de sortie."],
   ["4", "TREBLE", "Plateau d'aigus à 4 kHz, -5 / +16 dB ; plat à midi."],
   ["5", "MIDS", "Cloche de médiums à 750 Hz, ±5 dB, plate à midi."],
   ["6", "BASS", "Plateau à 90 Hz et bosse de punch vers 120 Hz, -15 / +18 dB, plat à midi."],
   ["7", "Inverseur EQ", "En haut = off (EQ retiré du circuit), en bas = on."],
   ["8", "Inverseur de mode", "En haut = SDT-2, au centre = Laerm, en bas = SDT-1."],
   ["9", "Écran", "Barre de presets au repos (flèches = preset usine précédent / suivant, clic sur le nom = menu des presets) ; ligne d'état à la même place pendant les réglages."],
   ["10", "LED", "Blanche = effet actif, éteinte = bypass."],
   ["11", "Footswitch BOOST", "Boost on / off (appui maintenu = momentané)."],
   ["12", "Footswitch BYPASS", "Effet on / off (appui maintenu = momentané)."],
   ["13", "OUT", "Jack de sortie."],
   ["14", "IN", "Jack d'entrée."]],
 ctl_t="4. Les commandes en détail",
 ctl=[("GAIN", "D'environ 26 dB à 72 dB de gain, répartis sur deux étages d'ampli op. Le gain arrive tôt : le potard donne déjà un crunch dur et mordant en début de course, et le mur façon fuzz qui a fait la réputation de la Stampede est là dès 9 h environ ; plus haut, le grain s'épaissit et le sustain s'allonge. Quand le gain monte, la bande passante de l'ampli op se resserre un peu, ce qui évite que les aigus ne grésillent, exactement comme sur le vrai circuit. Le grave est filtré fortement avant l'écrêtage et suit votre jeu après : plein à l'attaque, il redescend quand la note s'éteint, les palm mutes restent nets. Une bosse large de bas-médiums vers 320 Hz, ajoutée après l'écrêtage, donne du corps aux accords. Une couche fuzz en parallèle (deux écrêtages en cascade légèrement asymétriques, comme une Big Muff) est mélangée légèrement sous la distorsion, à environ 13 % du son, pour la texture et le grain, et une bosse large vers 220 Hz après l'écrêtage donne le gras du son. La sortie suit la dynamique de votre jeu : les attaques ressortent et les notes retombent naturellement, le son n'est jamais écrasé."),
      ("BOOST", "Comme sur la Thorn, le boost est <b>additif</b> : ce n'est pas une pédale séparée placée devant, il ajoute son gain (0 à +20 dB, réglé avec le petit potard, plus jusqu'à +1,5 dB de niveau ; il ne change presque pas la couleur) au GAIN et remonte un peu le passe-haut d'entrée : l'attaque se resserre et le sustain s'allonge. Le petit potard règle la quantité ; le footswitch BOOST l'enclenche et le coupe. Comme sur la Thorn, le potard est translucide : quand le boost est enclenché, il s'allume de l'intérieur et sert de témoin."),
      ("LEVEL", "Courbe audio : 0 = coupé, midi environ −25 dB (à peu près le niveau de la guitare), 15 h environ −17 dB, à fond = −10 dB : la sortie garde de la marge dans le DAW sur toute la course. Les trois modes sortent au même volume : on peut changer de mode sans saut de niveau."),
      ("TREBLE · MIDS · BASS", "EQ actif 3 bandes après la distorsion : plateau de graves à 90 Hz avec une bosse de punch vers 120 Hz (-15 / +18 dB, après l'écrêtage : plus de corps sans plus de compression), cloche de médiums à <b>750 Hz</b> (±5 dB), plateau d'aigus à 4 kHz (-5 / +16 dB, avec de la brillance en plus vers 5,5 kHz quand on le monte). À midi, chaque bande est plate ; BASS et TREBLE suivent une courbe douce : fins autour de midi, plus forts vers les butées, jamais de changement brusque. La voix du circuit et la course des potards sont calées sur la vraie Thorn de la vidéo Dirty Demos (<i>Michael Klein SPD100 + Lichtlaerm Audio Thorn</i>)."),
      ("EQ off / on", "Inverseur en haut (off) : l'EQ est entièrement retiré du trajet du signal, comme sur la Thorn ; les trois potards d'EQ n'ont alors plus d'effet et la voix brute du circuit part directement dans l'ampli (le réglage de Lennart dans un Ampeg V4). Le changement se fait en fondu, sans aucun clic."),
      ("Modes", "<b>SDT-1</b> (en bas) : la Stampede d'origine, diodes silicium à la masse, médiums qui chantent. <b>Laerm</b> (au centre) : la voix propre à Lichtlaerm, seuil d'écrêtage plus haut (type LED), bande passante plus large, plus ouverte et plus directe. <b>SDT-2</b> (en haut) : un étage de compression en plus, plus saturée et plus sombre, les sons de Living Gate. Les trois modes sortent au même volume ; le changement se fait en fondu en quelques millisecondes. Un clic au-dessus du pivot de l'inverseur le monte d'un cran, en dessous le descend d'un cran (la molette fonctionne aussi).")],
 modes_cap="L'inverseur de mode sur SDT-2 (levier en haut), Laerm (au centre) et SDT-1 (levier en bas).",
 fs_t="5. Footswitchs, lumières et noise gate",
 fs=["<b>Footswitchs</b> : ils basculent dès l'appui et le bouton s'assombrit tant qu'on appuie. <b>Smart relay</b> (comme sur la Thorn) : si vous maintenez un footswitch plus de 0,45 s après avoir enclenché quelque chose, il fonctionne en momentané et se coupe quand vous relâchez. Clic droit &gt; <b>Settings</b> pour désactiver ce comportement.",
     "<b>LED</b> : blanche quand l'effet est actif, éteinte en bypass, avec allumage et extinction en fondu. <b>Potard BOOST</b> : en verre dépoli blanc ; boost coupé, il reste mat et gris clair ; boost enclenché, il s'allume de l'intérieur avec un halo autour.",
     "<b>Lumières</b> (clic droit &gt; <b>Lights</b>) : <b>LED glow</b> et <b>Boost knob glow</b> règlent séparément le halo de la LED et celui du potard BOOST, en <b>Low</b> (discret), <b>Medium</b> (par défaut) ou <b>High</b> (la lumière porte plus loin), selon votre écran ou la lumière de la pièce. La lentille de la LED reste toujours aussi blanche ; le réglage est enregistré avec le projet.",
     "<b>Bypass</b> : son direct (le signal sec passe, aligné dans le temps sur la latence du sur-échantillonnage annoncée au DAW).",
     "<b>Noise gate</b> (clic droit &gt; Noise gate) : coupe le souffle et le ronflement dès qu'on arrête de jouer, s'ouvre instantanément à la note suivante (aucune attaque coupée), laisse les notes s'éteindre en douceur. Off, Light, <b>Medium</b> (par défaut) ou Strong (micros bruyants). N'agit que sur l'effet."],
 led_cap="LED en bypass puis effet actif ; potard BOOST, boost coupé puis enclenché.",
 midi_t="6. MIDI",
 midi_intro=["Toutes les commandes de la façade se pilotent depuis un contrôleur MIDI (footswitchs, pédales d'expression, potards), par exemple un M-Vave Chocolate Plus. Doorn écoute tous les canaux MIDI. <b>Aucun CC n'est assigné par défaut</b> : chaque commande à piloter s'assigne avec le <b>MIDI Learn</b> (clic droit sur la commande, ou clic droit &gt; MIDI &gt; Learn: ...)."],
 midi_hdr=["Commande", "Type"],
 midi=[["Gain", "Continu (0-127 = toute la course)"], ["Boost (potard de niveau)", "Continu"], ["Level", "Continu"],
       ["Treble", "Continu"], ["Mids", "Continu"], ["Bass", "Continu"],
       ["EQ", "Bascule : 64 ou plus = on, moins de 64 = off"],
       ["Mode", "Sélecteur 3 positions : 0-42 SDT-2, 43-84 Laerm, 85-127 SDT-1"],
       ["Footswitch Boost", "Footswitch, à accrochage (127 = on / 0 = off) ou momentané (bascule à chaque appui)"],
       ["Bypass", "Footswitch, à accrochage ou momentané (même réglage)"]],
 learn_h="MIDI Learn",
 learn=["<b>Clic droit sur une commande</b> (potard, inverseur ou footswitch) : le menu s'ouvre là où vous cliquez, avec en en-tête le nom de la commande et son assignation (<i>MIDI: CC 64</i> ou <i>MIDI: not assigned</i>). Choisissez <b>MIDI Learn: &lt;nom&gt;</b>, puis bougez la pédale d'expression ou appuyez sur le bouton MIDI à utiliser. Pendant l'attente, l'écran affiche <b>MIDI LEARN · Gain</b> ; le premier CC reçu est affecté à la commande (il ne change pas le son) et l'écran le confirme pendant 2 secondes, par ex. <b>Gain = CC 64</b>.",
        "Si ce numéro de CC servait déjà à une autre commande, celle-ci est désassignée : un même CC ne pilote jamais deux commandes par erreur. <b>Clear MIDI: &lt;nom&gt;</b> retire l'assignation de la commande ; <b>Cancel MIDI learn</b> (affiché pendant l'attente) annule l'apprentissage sans rien changer.",
        "<b>Clic droit ailleurs &gt; MIDI</b> : la liste des dix commandes avec leur CC actuel (<b>Learn: Gain (not assigned)</b> tant qu'elle n'est pas assignée, <b>Learn: Gain (CC 64)</b> une fois apprise ; en choisir une lance son apprentissage), <b>Cancel MIDI learn</b> et <b>Clear all MIDI assignments</b>.",
        "Les assignations sont <b>sauvegardées avec le projet</b> (et avec le preset du DAW). Les CC 0 à 119 peuvent être appris (120 à 127 sont des messages de mode de canal)."],
 midi_more_h="Pédales d'expression, footswitchs et Program Change",
 midi_more=["<b>Pédales d'expression</b> : les mouvements ne sont jamais saccadés. Doorn mesure le temps entre deux messages de votre pédale et rejoint chaque nouvelle valeur par une rampe pendant ce temps ; la course réelle de votre pédale est apprise automatiquement : une pédale qui n'envoie que 2 à 116 atteint quand même exactement 0 et 100 %. Pour un potard appris sur un CC de 0 à 31, le CC n+32 est lu comme sa partie fine (14 bits) si la pédale l'envoie.",
            "<b>Footswitchs MIDI</b> : à accrochage (127 = on / 0 = off, par défaut) ou momentanés (bascule à chaque appui), clic droit &gt; Settings. Le réglage vaut pour les deux footswitchs, quel que soit le CC appris.",
            "<b>Program Change 1 à 13</b> : rappel des presets usine. Tous les paramètres sont aussi automatisables dans le DAW."],
 win_t="7. Fenêtre et écran",
 win=["Doorn utilise le format pédale simple de NINE HANDS | DSP : la fenêtre montre la pédale seule, s'ouvre à sa taille par défaut et ne peut jamais être réduite en dessous. Clic droit &gt; <b>Plugin size</b> : <b>60 %</b>, <b>75 %</b> ou <b>100 %</b> redimensionne cette fenêtre, <b>(default)</b> étant indiqué à côté de la taille par défaut. En bas du même menu, <b>Default size (new windows)</b> choisit la taille d'ouverture de toutes les nouvelles fenêtres de Doorn (réglage commun à toutes les instances, enregistré sur l'ordinateur) ; la fenêtre ouverte prend aussitôt cette taille. Vous pouvez aussi tirer le coin en bas à droite pour l'agrandir jusqu'à 150 % (les proportions sont conservées). Les menus clic droit s'ouvrent toujours là où vous cliquez.",
      "L'<b>écran</b> réunit la barre de presets et la ligne d'état. Au repos, il montre la <b>barre de presets</b> : le preset en cours (ou <i>Edited</i> dès qu'un réglage change) et l'état du bypass, entre les deux flèches. Dès qu'un réglage bouge (souris, molette, automation, MIDI, footswitch), la <b>ligne d'état</b> prend sa place, dans la même police et à la même taille : le réglage de la commande en direct (potards en %, par ex. <i>gain 76 %</i>, <i>eq off</i>, <i>laerm</i>, <i>boost on</i>) et les messages du MIDI Learn. Elle reste 2,5 s après le dernier mouvement, puis la barre de presets revient en fondu. Un clic sur l'écran pendant qu'elle est affichée fait revenir la barre de presets aussitôt (et le clic agit : flèche ou menu)."],
 bar_cap="L'écran : barre de presets au repos, puis ligne d'état pendant un réglage.",
 pre_t="8. Presets",
 pre=["Utilisez les flèches de l'écran, cliquez sur le nom du preset, faites un clic droit n'importe où, ou passez par le menu de presets de votre DAW. Clic droit &gt; <b>Save user preset</b> enregistre jusqu'à 9 presets personnels (<b>Load user preset</b> pour les rappeler)."],
 fac_hdr=["PC", "Preset", "Gain / Boost / Level", "Treble / Mids / Bass", "EQ · Mode", "Description"],
 tips_t="9. Conseils d'utilisation",
 tips=["Dans Reaper, placez Doorn <b>avant</b> votre simulateur d'ampli (par exemple avant Ampire, suivi d'Impulse pour le baffle) avec un ampli assez clair, comme avec la vraie pédale. Avec un ampli crunch, baissez le gain : la Stampede en a déjà beaucoup.",
       "Pour les accordages bas (Si standard comme Amenra, ou une 7 cordes en Sol et plus bas), gardez BASS entre midi et 13 h et servez-vous du BOOST : il resserre le grave sans perdre de poids.",
       "Les modes préférés du testeur d'origine sont SDT-1 et Laerm ; SDT-2 est parfait pour les longs accords saturés.",
       "S'il reste du souffle quand vous arrêtez de jouer, passez clic droit &gt; Noise gate sur Strong ; pour des arrêts très secs sur les riffs saccadés, NGATE de NINE HANDS | DSP peut toujours être ajouté après Doorn."],
 tech_t="10. Caractéristiques techniques",
 tech=[["Format", "VST3 (Windows 64 bits, macOS universel)"], ["Canaux", "Mono ou stéréo"],
       ["Latence", "32 échantillons (filtre de sur-échantillonnage), annoncée au DAW"], ["Gain", "environ 26 à 72 dB, + 0 à 20 dB de boost"],
       ["EQ", "graves 90 Hz + punch 120 Hz -15 / +18 dB, médiums 750 Hz ±5 dB, aigus 4 kHz -5 / +16 dB, bypassable"],
       ["Traitement", "sur-échantillonnage x4, écrêteurs anti-repliement (ADAA), paramètres lissés, pas de dénormaux"], ["Processeur", "environ 5 % d'un cœur à 48 kHz (stéréo)"],
       ["MIDI", "tous les canaux ; MIDI Learn sur les 10 commandes (aucun CC assigné par défaut), CC 14 bits, Program Change 1-13 ; assignations sauvegardées avec le projet"],
       ["Fenêtre", "60 % / 75 % / 100 % (Plugin size), taille par défaut des nouvelles fenêtres au choix, jusqu'à 150 %, jamais sous 60 % ; halos réglables (Lights) ; anti-gel v2 sous Windows (rendu logiciel, chien de garde, aucun appel à l'hôte depuis le thread audio)"]])

W = A4[0] - 36 * mm
def story():
    t = T; f = []
    f.append(NextPageTemplate("p"))
    f.append(Spacer(1, 1)); f.append(PageBreak())          # couverture dessinee a positions fixes (cover_bg)
    f.append(P(t["intro_t"], "h1")); f += [P(x) for x in t["intro"]]
    f.append(P(t["inst_t"], "h1"))
    for h, x in t["inst"]: f.append(P(h, "h2")); f.append(P(x))
    f.append(PageBreak())
    f.append(P(t["ov_t"], "h1"))
    ov = Table([[img(annotated, 88 * mm), table(t["ov_rows"], [8 * mm, 26 * mm, W - 92 * mm - 34 * mm])]], colWidths=[92 * mm, W - 92 * mm])
    ov.setStyle(TableStyle([("VALIGN", (0, 0), (-1, -1), "TOP"), ("LEFTPADDING", (0, 0), (-1, -1), 0)]))
    f.append(ov); f.append(PageBreak())
    f.append(P(t["ctl_t"], "h1"))
    for h, x in t["ctl"]: f.append(P(h, "h2")); f.append(P(x))
    f.append(KeepTogether([Spacer(1, 3 * mm), img(modes_img, W * 0.62), P(t["modes_cap"], "small")]))
    f.append(P(t["fs_t"], "h1")); f += bullets(t["fs"])
    f.append(KeepTogether([Spacer(1, 2 * mm), img(leds_img, W * 0.86), P(t["led_cap"], "small")]))
    f.append(P(t["midi_t"], "h1")); f += [P(x) for x in t["midi_intro"]]
    f.append(KeepTogether([table([t["midi_hdr"]] + t["midi"], [48 * mm, W - 48 * mm])]))
    f.append(P(t["learn_h"], "h2")); f += bullets(t["learn"])
    f.append(P(t["midi_more_h"], "h2")); f += bullets(t["midi_more"])
    f.append(P(t["win_t"], "h1")); f.append(P(t["win"][0]))
    wt = Table([[[P(t["win"][1])], [img(bar_img, 64 * mm), P(t["bar_cap"], "small")]]], colWidths=[W - 68 * mm, 68 * mm])
    wt.setStyle(TableStyle([("VALIGN", (0, 0), (-1, -1), "TOP"), ("LEFTPADDING", (0, 0), (0, 0), 0)]))
    f.append(wt)
    f.append(CondPageBreak(60 * mm))
    f.append(P(t["pre_t"], "h1")); f += [P(x) for x in t["pre"]]
    rows = [t["fac_hdr"]]
    for i, (n, g, b, l, tr, mi, ba, eq, mo, bo) in enumerate(FACTORY):
        em = ("on" if eq else "off") + " · " + MODES[mo] + (" + boost" if bo else "")
        rows.append([str(i + 1), "<b>" + n + "</b>", f"{pc(g)} / {pc(b)} / {pc(l)}", f"{pc(tr)} / {pc(mi)} / {pc(ba)}", em, FACT_DESC[i]])
    f.append(table(rows, [9 * mm, 28 * mm, 26 * mm, 26 * mm, 22 * mm, W - 111 * mm], small=True))
    f.append(P(t["tips_t"], "h1")); f += bullets(t["tips"], "tip")
    f.append(KeepTogether([P(t["tech_t"], "h1"), table(t["tech"], [26 * mm, W - 26 * mm], header=False, small=True)]))
    return f

def deco(c, doc):
    c.saveState()
    c.setFillColor(DARK); c.rect(0, A4[1] - 6 * mm, A4[0], 6 * mm, stroke=0, fill=1)
    c.setFillColor(ACC); c.rect(0, A4[1] - 6.8 * mm, A4[0], 0.8 * mm, stroke=0, fill=1)
    c.setFont("Pop", 7.5); c.setFillColor(GREY)
    c.drawString(18 * mm, 10 * mm, "Doorn (NINE HANDS | DSP) · v" + VERSION)
    c.drawRightString(A4[0] - 18 * mm, 10 * mm, str(doc.page))
    c.restoreState()

doc = BaseDocTemplate(OUT, pagesize=A4, leftMargin=18 * mm, rightMargin=18 * mm, topMargin=16 * mm, bottomMargin=18 * mm,
                      title="DOORN - Notice v" + VERSION, author="NINE HANDS | DSP")
# ---- Couverture « minimale » (modele 6, valide nov. 2026) : gabarit fixe commun a toutes les notices ----
# Mesures en mm depuis le HAUT de la page A4, sauf le logo et le bandeau (depuis le bas). Aucun autre element.
COVER_NAME = "DOORN"
PEDAL_H, PEDAL_MAXW, PEDAL_TOP = 82 * mm, 115 * mm, 84 * mm      # pedale (boitier + jacks, sans reflet) centree horizontalement
LANG_TOP = 18 * mm                                               # « FRANÇAIS » centre, Poppins Medium 8,5 pt, couleur d'accent claire
NAME_TOP, NAME_PT = 194 * mm, 30                                 # nom : Barlow Condensed ExtraBold 30 pt, approche 0,5 x corps
SUB_TOP, SUB_PT = 208 * mm, 10                                   # mention courte : Poppins 10 pt, approche 1,5 pt, gris clair
LOGO_W, LOGO_BOTTOM = 38 * mm, 22 * mm                           # logo blanc centre en bas
BAND_H = 3 * mm                                                  # bandeau couleur d'accent en BAS de page
def cover_bg(c, doc):
    c.saveState(); c.setFillColor(colors.black); c.rect(0, 0, A4[0], A4[1], stroke=0, fill=1)
    c.setFillColor(BAND); c.rect(0, 0, A4[0], BAND_H, stroke=0, fill=1)
    def line(text, top, font, size, cs, col):
        w = pdfmetrics.stringWidth(text, font, size) + cs * (len(text) - 1)
        t = c.beginText((A4[0] - w) / 2, A4[1] - top - size * 0.8); t.setFont(font, size); t.setCharSpace(cs)
        t.setFillColor(col); t.textOut(text); c.drawText(t)
    line(T["lang"], LANG_TOP, "PopM", 8.5, 0, ACC_L)
    ci = Image.open(cover); iw, ih = ci.size
    cw = iw / 1.32; ch = (ih - int(cw * 0.16)) / 1.2; pad = int(cw * 0.16)     # geometrie de doorn_cover_cutout.png
    k = min(PEDAL_H / ch, PEDAL_MAXW / cw)
    dw, dh = iw * k, ih * k
    y0 = A4[1] - (PEDAL_TOP + (PEDAL_H - ch * k) / 2) + pad * k            # haut de l'image (pedale centree dans sa boite)
    c.drawImage(cover, (A4[0] - dw) / 2, y0 - dh, dw, dh)
    line(COVER_NAME, NAME_TOP - NAME_PT * 0.02, "Title", NAME_PT, NAME_PT * 0.5, colors.white)
    line(T["sub"], SUB_TOP, "Pop", SUB_PT, 1.5, colors.HexColor("#BDC4B8"))
    lw, lh = Image.open(LOGO_PNG).size
    c.drawImage(LOGO_PNG, (A4[0] - LOGO_W) / 2, LOGO_BOTTOM, LOGO_W, LOGO_W * lh / lw, mask="auto")
    c.restoreState()
doc.addPageTemplates([PageTemplate(id="cover", frames=[Frame(doc.leftMargin, doc.bottomMargin, doc.width, doc.height, id="fc")], onPage=cover_bg),
                      PageTemplate(id="p", frames=[Frame(doc.leftMargin, doc.bottomMargin, doc.width, doc.height, id="f")], onPage=deco)])
doc.build(story())
print(OUT)
