#!/usr/bin/env python3
"""Doorn v3 : genere Layout.h a partir de gui/manifest.json (design v017 de l'utilisateur).

Repere du plugin = image 965 x 1800 px / 2 (482,5 x 900 unites), comme Screamer.
Usage : python3 tools/layout_from_manifest.py
"""
import json, os

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
m = json.load(open(os.path.join(ROOT, 'gui', 'manifest.json')))
U = 0.5   # unites du plugin par pixel de l'image


def r(x, y, w, h):
    return '{ %.2ff, %.2ff, %.2ff, %.2ff }' % (x * U, y * U, w * U, h * U)


def p(x, y):
    return '{ %.2ff, %.2ff }' % (x * U, y * U)


k = m['knobs']
order = ['gain', 'boost', 'level', 'treble', 'mids', 'bass']   # ordre des potards du processeur
knob_rects = []
for n in order:
    d = k[n]
    s = d['sprite_size']
    knob_rects.append(r(d['x'], d['y'], s, s))
sw = m['switches']
led = m['leds']['bypass']
fs = m['footswitches']
scr = m['screen']
glow = k['boost']['glow']

out = f"""// genere par tools/layout_from_manifest.py a partir de gui/manifest.json (design v017) - ne pas modifier a la main
// repere {m['canvas']['width'] * U:.1f} x {m['canvas']['height'] * U:.1f} unites = image {m['canvas']['width']} x {m['canvas']['height']} px / 2
#pragma once
#include <juce_graphics/juce_graphics.h>
namespace layout
{{
    constexpr float frameW = {m['canvas']['width'] * U:.2f}f, frameH = {m['canvas']['height'] * U:.2f}f;
    // sprites des potards (ordre : gain, boost, level, treble, mids, bass) ; diametre visible = 95 % du sprite (190 / 200, 100 / 108)
    const juce::Rectangle<float> knobRect[6] = {{ {', '.join(knob_rects)} }};
    const float knobDiamRatio[6] = {{ 0.95f, {k['boost']['diameter'] / k['boost']['sprite_size']:.4f}f, 0.95f, 0.95f, 0.95f, 0.95f }};
    const juce::Rectangle<float> boostGlowRect = {r(glow['x'], glow['y'], glow['size'], glow['size'])};
    // inverseurs : sprites 174 x 210 px (meme cadrage pour les 3 positions), pivot du levier
    const juce::Rectangle<float> eqRect = {r(sw['left']['x'], sw['left']['y'], sw['left']['w'], sw['left']['h'])}, modeRect = {r(sw['right']['x'], sw['right']['y'], sw['right']['w'], sw['right']['h'])};
    const float switchPivotY = {(sw['left']['center'][1] - sw['left']['y']) / sw['left']['h']:.4f}f;   // hauteur relative du pivot dans le sprite
    const juce::Rectangle<float> ledRect = {r(led['x'], led['y'], led['size'], led['size'])};
    const juce::Point<float> boostFsPos = {p(*fs['boost']['center'])}, bypassFsPos = {p(*fs['bypass']['center'])};
    const float footR = {fs['boost']['radius'] * U:.2f}f;
    // ecran de presets (cadre et fleches dans le fond) : le nom du preset est ecrit par l'editeur
    const juce::Rectangle<float> screenRect = {r(scr['x'], scr['y'], scr['w'], scr['h'])};
    const juce::Point<float> arrowLeft = {p(*scr['arrow_left_center'])}, arrowRight = {p(*scr['arrow_right_center'])};
}}
"""
open(os.path.join(ROOT, 'Layout.h'), 'w').write(out)
print(out)
