// genere par tools/layout_from_manifest.py a partir de gui/manifest.json (design v018) - ne pas modifier a la main
// repere 617.5 x 900.0 unites = image 1235 x 1800 px / 2
#pragma once
#include <juce_graphics/juce_graphics.h>
namespace layout
{
    constexpr float frameW = 617.50f, frameH = 900.00f;
    // sprites des potards (ordre : gain, boost, level, treble, mids, bass) ; diametre visible = 95 % du sprite (190 / 200, 100 / 108)
    const juce::Rectangle<float> knobRect[6] = { { 104.50f, 82.50f, 100.00f, 100.00f }, { 282.00f, 105.50f, 54.00f, 54.00f }, { 413.00f, 82.50f, 100.00f, 100.00f }, { 104.50f, 275.00f, 100.00f, 100.00f }, { 259.00f, 275.00f, 100.00f, 100.00f }, { 413.00f, 275.00f, 100.00f, 100.00f } };
    const float knobDiamRatio[6] = { 0.95f, 0.9259f, 0.95f, 0.95f, 0.95f, 0.95f };
    const juce::Rectangle<float> boostGlowRect = { 239.00f, 62.50f, 140.00f, 140.00f };
    // inverseurs : sprites 174 x 210 px (meme cadrage pour les 3 positions), pivot du levier
    const juce::Rectangle<float> eqRect = { 120.00f, 451.00f, 87.00f, 105.50f }, modeRect = { 429.00f, 451.00f, 86.50f, 105.50f };
    const float switchPivotY = 0.4408f;   // hauteur relative du pivot dans le sprite
    const juce::Rectangle<float> ledRect = { 264.00f, 617.50f, 90.00f, 90.00f };
    const juce::Point<float> boostFsPos = { 177.50f, 782.50f }, bypassFsPos = { 440.00f, 782.50f };
    const float footR = 60.00f;
    // ecran de presets (cadre et fleches dans le fond) : le nom du preset est ecrit par l'editeur
    const juce::Rectangle<float> screenRect = { 154.38f, 585.25f, 308.75f, 32.50f };
    const juce::Point<float> arrowLeft = { 171.50f, 601.50f }, arrowRight = { 446.00f, 601.50f };
}
