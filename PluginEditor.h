#pragma once

#include "PluginProcessor.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include "Layout.h"
#include "NineHandsMenu.h"

//==============================================================================
// Doorn v4 : interface du design v018 de l'utilisateur (gui/, manifest.json, 1235 x 1800 px).
// Potards en 3 calques (centre fixe, couronne + fleche qui tournent, reflet fixe par-dessus), au modele
// des potards NINE HANDS | DSP : flou de mouvement sur tout ce qui tourne, image prete calculee par le timer
// (anti-gel v2 : paint() ne fait que la poser). Potard BOOST en verre depoli blanc (mini pack v018) : calques « off » / « on » echanges net par le footswitch BOOST.
//==============================================================================
struct KnobImages
{
    juce::Image centre, rotating, reflect;     // potards noirs
    juce::Image bCentreOff, bRotOff, bCentreOn, bRotOn, bReflect;   // potard BOOST
};

class DoornKnob : public juce::Slider
{
public:
    bool isMini = false;
    float lit = 0.0f;                    // potard BOOST : 1 = boost enclenche (sprites ON), 0 = OFF
    float diamRatio = 0.95f;             // diametre visible / cote du sprite
    const KnobImages* img = nullptr;
    float startAngle = 0.0f, endAngle = 1.0f;
    bool hitTest (int x, int y) override;
    void paint (juce::Graphics&) override;
    void tick (float physScale);         // appele par le timer de l'editeur (30 Hz)

    // flou de mouvement (potards-nine-hands-dsp) : longueur = angle parcouru pendant l'image, retombee x0,6
    static constexpr float blurMax = 0.6981f, blurStep = 0.05236f, blurDecay = 0.6f;   // 40 deg, 3 deg

private:
    void rebuild (float sc);
    void render (float a);
    juce::Image cCentre, cRot, cReflect, cCentreOn, cRotOn, frame, scratch;
    float cacheSc = 0.0f;
    int cacheW = 0, cacheH = 0;
    float lastAngle = 1.0e9f, blurSpan = 0.0f, blurDir = 1.0f, drawnAngle = 1.0e9f, drawnSpan = -1.0f, drawnLit = -1.0f;
};

//==============================================================================
// Inverseur a levier : 3 images du design (haut, milieu, bas) ; l'image du milieu s'affiche ~55 ms
// pendant une bascule haut <-> bas (inverseur EQ a 2 positions, ou SDT-2 <-> SDT-1).
//==============================================================================
class LeverSwitch : public juce::Component
{
public:
    LeverSwitch (juce::RangedAudioParameter& p, int numPositions);
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) override;
    void tick (double nowMs);             // appele par le timer de l'editeur
    juce::Image frames[3];                 // haut, milieu, bas
    float pivotY = 0.44f;
    std::function<void()> onRightClick, onUserChange;

private:
    int frameFor (int position) const;
    void moveTo (int newPos);
    juce::ParameterAttachment attachment;
    int positions, pos = 0, shownFrame = 0, targetFrame = 0;
    double transitionUntil = 0.0;
};

//==============================================================================
// Footswitch (dans le fond) : fonctionnement d'Echoflex : bascule a l'appui, bouton assombri tant qu'on appuie.
// « Smart relay » : appui maintenu (> 0,45 s) quand l'effet etait coupe = fonctionnement momentane.
//==============================================================================
class FootSwitchArea : public juce::Component
{
public:
    FootSwitchArea (juce::RangedAudioParameter& p, std::atomic<float>* holdMomentary);
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    bool hitTest (int x, int y) override;
    std::function<void()> onRightClick, onUserChange;

private:
    bool value = false, pressed = false, turnedOnByPress = false;
    double downTime = 0.0;
    std::atomic<float>* hold = nullptr;
    juce::ParameterAttachment attachment;
};

//==============================================================================
struct RightClickForwarder : public juce::MouseListener
{
    std::function<void (juce::Component*)> callback;    // composant clique (potard) : menu MIDI Learn de cette commande
    void mouseDown (const juce::MouseEvent& e) override
    {
        if (e.mods.isPopupMenu() && callback) callback (e.eventComponent);
    }
};

//==============================================================================
class DoornAudioProcessorEditor : public juce::AudioProcessorEditor,
                                  private juce::Timer
{
public:
    explicit DoornAudioProcessorEditor (DoornAudioProcessor&);
    ~DoornAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void parentHierarchyChanged() override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseMove (const juce::MouseEvent&) override;
    void mouseExit (const juce::MouseEvent&) override;

    // Repere du fond (pedale plein cadre, image 1235 x 1800 px / 2). Hauteurs de fenetre des pedales simples :
    // taille par defaut = taille minimale 493 px (« 60 % »), 100 % = 958 px, jusqu'a 150 %.
    static constexpr float baseW = layout::frameW;
    static constexpr float baseH = layout::frameH;
    static constexpr int defaultH = 493;
    static constexpr int refH = 958;
    static constexpr int maxH = refH * 3 / 2;

    void setScaledHeight (int h);
    void forceRenderScale (float sc) { forcedScale = sc; rebuildCaches (sc); }   // captures (outil de rendu)
    static constexpr int numSizes = 3;          // 60 % (defaut d'origine) / 75 % / 100 %
    static int sizeHeight (int idx);
    void showStatus();
    void setGlowPercent (int pc);          // clic droit > Glow (aussi pour l'outil de rendu)

private:
    void timerCallback() override;
    void useSoftwareRenderer();
    void visibilityChanged() override;
    void askRepaint (juce::Rectangle<int> r);
    void showContextMenu (int learnIdx = -1);    // learnIdx >= 0 : clic droit sur une commande
    void handleMidiMenu (int r);
    void showPresetMenu();
    void stepPreset (int delta);
    void rebuildCaches (float scale = 0.0f);
    float physicalScale() const;
    float unitScale() const { return (float) getHeight() / baseH; }
    juce::Rectangle<int> scaled (juce::Rectangle<float> r) const;
    juce::String buildStatusLine() const;
    juce::String buildPresetLine() const;
    void drawLed (juce::Graphics&);
    void drawScreen (juce::Graphics&);

    DoornAudioProcessor& proc;
    KnobImages knobImg;
    RightClickForwarder rightClickFwd;

    juce::Image background, bgScaled, boostGlow, ledOff, ledOn, ledLens, ledHalo;   // LED : lentille et halo separes
    // clic droit > Glow : halo de la LED et du potard BOOST, 25 / 50 / 75 / 100 % (settings.xml, commun aux fenetres)
    int glowPc = 100;
    double lastGlowReadMs = 0.0;
    float glow() const { return (float) glowPc / 100.0f; }
    static juce::Rectangle<float> glowArea (juce::Rectangle<float> r) { return r.withSizeKeepingCentre (r.getWidth() * 1.35f, r.getHeight() * 1.35f); }
    int cacheW = 0, cacheH = 0;
    float cacheScale = 0.0f, forcedScale = 0.0f;
    int resizeSettle = 0;
    // anti-gel v2 : suivi de la fenetre hote, de l'echelle et des redessins
    void* lastPeer = nullptr;
    double lastPaintMs = 0.0, repaintAskedMs = 0.0, lastScaleCheckMs = 0.0, lastRendererCheckMs = 0.0;
    juce::Typeface::Ptr regularFace, labelFace, boldFace;
    NineHandsMenu::MenuLnf menuLnf;        // menus clic droit au design d'Octopus

    DoornKnob knobs[DoornAudioProcessor::numKnobs];
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> knobAtt[DoornAudioProcessor::numKnobs];
    LeverSwitch eqSwitch, modeSwitch;
    FootSwitchArea boostFs, bypassFs;

    // animation
    float ledLevel = 0.0f, boostLit = 0.0f, drawnLed = -1.0f, drawnBoost = -1.0f;
    int lastChanged = -1;               // commande affichee dans la ligne d'etat
    float statusAlpha = 0.0f, drawnStatus = -1.0f;
    double statusUntil = 0.0;
    juce::String lastLine1, lastPreset;
    juce::String learnLine;             // MIDI Learn : attente, puis « <Commande> = CC n » 2 s
    double learnLineUntil = 0.0;
    float lastValues[16];
    int lastPresetCounter = 0, lastMidiCount = 0, midiFlash = 0;
    int hoverZone = 0, drawnHover = -1;       // 1 = fleche gauche, 2 = nom, 3 = fleche droite

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DoornAudioProcessorEditor)
};
