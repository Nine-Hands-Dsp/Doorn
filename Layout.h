// genere par tools/layout_from_manifest.py a partir de gui/manifest.json (design v017) - ne pas modifier a la main
// repere 482.5 x 900.0 unites = image 965 x 1800 px / 2
#pragma once
#include <juce_graphics/juce_graphics.h>
namespace layout
{
    constexpr float frameW = 482.50f, frameH = 900.00f;
    // sprites des potards (ordre : gain, boost, level, treble, mids, bass) ; diametre visible = 95 % du sprite (190 / 200, 100 / 108)
    const juce::Rectangle<float> knobRect[6] = { { 42.50f, 82.00f, 100.00f, 100.00f }, { 214.00f, 105.00f, 54.00f, 54.00f }, { 340.00f, 82.00f, 100.00f, 100.00f }, { 42.50f, 275.00f, 100.00f, 100.00f }, { 191.00f, 275.00f, 100.00f, 100.00f }, { 340.00f, 275.00f, 100.00f, 100.00f } };
    const float knobDiamRatio[6] = { 0.95f, 0.9259f, 0.95f, 0.95f, 0.95f, 0.95f };
    const juce::Rectangle<float> boostGlowRect = { 171.00f, 62.00f, 140.00f, 140.00f };
    // inverseurs : sprites 174 x 210 px (meme cadrage pour les 3 positions), pivot du levier
    const juce::Rectangle<float> eqRect = { 58.00f, 451.00f, 87.00f, 105.00f }, modeRect = { 355.50f, 451.00f, 87.00f, 105.00f };
    const float switchPivotY = 0.4405f;   // hauteur relative du pivot dans le sprite
    const juce::Rectangle<float> ledRect = { 196.00f, 617.50f, 90.00f, 90.00f };
    const juce::Point<float> boostFsPos = { 110.00f, 782.50f }, bypassFsPos = { 372.50f, 782.50f };
    const float footR = 60.00f;
    // ecran de presets (cadre et fleches dans le fond) : le nom du preset est ecrit par l'editeur
    const juce::Rectangle<float> screenRect = { 110.00f, 587.50f, 262.50f, 28.00f };
    const juce::Point<float> arrowLeft = { 125.00f, 601.50f }, arrowRight = { 357.00f, 601.50f };
}
