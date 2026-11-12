#include "PluginEditor.h"
#include "BinaryData.h"
#include "WigaWindowSize.h"
#include "NineHandsGlow.h"

using namespace juce;

namespace
{
    // Repere : image du design v018 (1235 x 1800 px) / 2, positions generees dans Layout.h depuis gui/manifest.json
    // Barre de presets / ligne d'etat de reference NINE HANDS | DSP (celle de Strong Pickle v1.5.3) :
    // pastille 400 x 38 unites (800 x 76 px dans l'image de 1800 px), Poppins Medium 23 (46 px), centree sur l'ecran du design
    const float barH = 38.0f, barW = 400.0f, barFont = 23.0f, arrowSize = 5.5f;
    const Rectangle<float> screenRect = Rectangle<float> (barW, barH).withCentre (layout::screenRect.getCentre());
    // barre de presets et ligne d'etat au meme endroit (regle NINE HANDS | DSP, validee sur Echoflex v2) :
    // une seule pastille, l'ecran du design ; la ligne d'etat y remplace le preset le temps des reglages
    const float arrowZone = 34.0f;     // zones cliquables des fleches de l'ecran (largeur en unites)

    // rotation des potards (manifest) : valeur 0 -> -135 degres, 0,5 -> fleche en haut, 1 -> +135 degres
    const float rotStart = MathConstants<float>::pi * 1.25f;
    const float rotEnd   = MathConstants<float>::pi * 2.75f;

    const Colour inkWhite (0xffe9e7e2);          // texte de l'ecran
    const Colour screenFace (0xff1c1c1d);       // face de l'ecran du design (mesuree dans le fond)
    const Colour menuAccent (0xffcdb995);        // pastille des en-tetes de menu (sable des fissures)

    const char* const valueIds[] = { "gain", "boost", "level", "treble", "mids", "bass", "eq", "mode", "boostOn", "active", "gate" };
    constexpr int numValueIds = 11;

    Image png (const void* data, int size) { return ImageCache::getFromMemory (data, size); }

    void drawImageIn (Graphics& g, const Image& im, Rectangle<float> r, float opacity = 1.0f)
    {
        if (! im.isValid() || opacity <= 0.0f) return;
        g.setOpacity (opacity);
        g.drawImage (im, r);
        g.setOpacity (1.0f);
    }
}

//==============================================================================
// POTARDS
//==============================================================================
bool DoornKnob::hitTest (int x, int y)
{
    const auto c = getLocalBounds().toFloat().getCentre();
    return c.getDistanceFrom ({ (float) x, (float) y }) <= (float) getWidth() * 0.5f * diamRatio;
}

void DoornKnob::paint (Graphics& g)
{
    // anti-gel v2 : l'image est prete (calculee par tick), on la pose
    if (frame.isValid())
    {
        g.setImageResamplingQuality (Graphics::mediumResamplingQuality);
        g.drawImage (frame, getLocalBounds().toFloat());
    }
}

void DoornKnob::rebuild (float sc)
{
    cacheSc = sc; cacheW = getWidth(); cacheH = getHeight();
    const int px = roundToInt ((float) jmin (getWidth(), getHeight()) * sc);
    if (px <= 0 || img == nullptr) return;
    // calques a la taille physique, une fois (jamais d'agrandissement : 200 px -> taille affichee, toujours plus petite)
    auto fit = [px] (const Image& src) -> Image
    {
        if (! src.isValid()) return {};
        return src.getWidth() == px ? src.createCopy() : src.rescaled (px, px, Graphics::highResamplingQuality);
    };
    if (isMini)
    {
        cCentre = fit (img->bCentreOff); cRot = fit (img->bRotOff); cReflect = fit (img->bReflect);
        cCentreOn = fit (img->bCentreOn); cRotOn = fit (img->bRotOn);
    }
    else
    {
        cCentre = fit (img->centre); cRot = fit (img->rotating); cReflect = fit (img->reflect);
    }
    frame   = Image (Image::ARGB, px, px, true);
    scratch = Image (Image::ARGB, px, px, true);
    drawnAngle = 1.0e9f;
}

void DoornKnob::tick (float sc)
{
    if (img == nullptr) return;
    if (std::abs (sc - cacheSc) > 0.01f || getWidth() != cacheW || getHeight() != cacheH) rebuild (sc);
    if (! frame.isValid()) return;

    const float a = startAngle + (float) valueToProportionOfLength (getValue()) * (endAngle - startAngle);
    const float v = lastAngle > 1.0e8f ? 0.0f : a - lastAngle;
    lastAngle = a;
    if (std::abs (v) > 1.0e-5f) blurDir = v > 0.0f ? 1.0f : -1.0f;
    blurSpan = jmax (jmin (std::abs (v), blurMax), blurSpan * blurDecay);
    if (blurSpan < 0.002f) blurSpan = 0.0f;

    if (std::abs (a - drawnAngle) > 1.0e-4f || blurSpan != drawnSpan || lit != drawnLit)
    {
        render (a);
        drawnAngle = a; drawnSpan = blurSpan; drawnLit = lit;
        repaint();
    }
}

void DoornKnob::render (float a)
{
    const float px = (float) frame.getWidth();
    // BOOST : sprites ON ou OFF selon l'etat (pas de fondu) ; centre fixe, seule la couronne + ligne tourne
    const bool on = isMini && lit >= 0.5f;
    const Image* centre = on ? &cCentreOn : &cCentre;
    const Image* rot    = on ? &cRotOn    : &cRot;

    frame.clear (frame.getBounds());
    Graphics g (frame);
    g.setImageResamplingQuality (Graphics::highResamplingQuality);
    if (centre->isValid()) g.drawImageAt (*centre, 0, 0);                // centre fixe
    auto at = [px] (float ang) { return AffineTransform::rotation (ang, px * 0.5f, px * 0.5f); };
    if (rot->isValid())
    {
        if (blurSpan <= 0.0f) g.drawImageTransformed (*rot, at (a));    // couronne + fleche, nettes a l'arret
        else
        {
            // flou : positions espacees de 3 deg au plus, MOYENNE EXACTE des pixels (premultiplies).
            // (v4.1.1 : l'ancienne moyenne « progressive » par superposition rendait opaque une couronne
            //  translucide pendant la rotation : la couronne de verre du BOOST devenait blanche en tournant.)
            const int n = jlimit (4, 16, (int) std::ceil (blurSpan / blurStep) + 1);
            const int w = scratch.getWidth(), h = scratch.getHeight();
            accum.assign ((size_t) w * (size_t) h * 4, 0u);
            for (int k = 0; k < n; ++k)
            {
                const float t = (float) k / (float) (n - 1);
                scratch.clear (scratch.getBounds());
                {
                    Graphics gs (scratch);
                    gs.setImageResamplingQuality (Graphics::highResamplingQuality);
                    gs.drawImageTransformed (*rot, at (a - blurDir * blurSpan * t));
                }
                const Image::BitmapData bd (scratch, Image::BitmapData::readOnly);
                for (int y = 0; y < h; ++y)
                {
                    uint32* acc = accum.data() + (size_t) y * (size_t) w * 4;
                    for (int x = 0; x < w; ++x, acc += 4)
                    {
                        const auto* px4 = reinterpret_cast<const PixelARGB*> (bd.getPixelPointer (x, y));
                        acc[0] += px4->getAlpha(); acc[1] += px4->getRed(); acc[2] += px4->getGreen(); acc[3] += px4->getBlue();
                    }
                }
            }
            {
                const uint32 half = (uint32) n / 2;
                Image::BitmapData bd (scratch, Image::BitmapData::writeOnly);
                for (int y = 0; y < h; ++y)
                {
                    const uint32* acc = accum.data() + (size_t) y * (size_t) w * 4;
                    for (int x = 0; x < w; ++x, acc += 4)
                    {
                        auto* px4 = reinterpret_cast<PixelARGB*> (bd.getPixelPointer (x, y));
                        // valeurs premultipliees : la moyenne reste valide (r, g, b <= alpha)
                        px4->setARGB ((uint8) ((acc[0] + half) / (uint32) n), (uint8) ((acc[1] + half) / (uint32) n),
                                      (uint8) ((acc[2] + half) / (uint32) n), (uint8) ((acc[3] + half) / (uint32) n));
                    }
                }
            }
            g.drawImageAt (scratch, 0, 0);
        }
    }
    if (cReflect.isValid()) g.drawImageAt (cReflect, 0, 0);              // reflet fixe par-dessus
}

//==============================================================================
// INVERSEUR
//==============================================================================
LeverSwitch::LeverSwitch (RangedAudioParameter& p, int numPositions)
    : attachment (p, [this] (float v)
                  {
                      const int np = jlimit (0, positions - 1, roundToInt (v));
                      if (np != pos)
                      {
                          const int from = frameFor (pos);
                          pos = np;
                          targetFrame = frameFor (pos);
                          // haut <-> bas : image du milieu ~55 ms pendant la bascule
                          shownFrame = std::abs (from - targetFrame) == 2 ? 1 : targetFrame;
                          transitionUntil = Time::getMillisecondCounterHiRes() + 55.0;
                          repaint();
                      }
                  }),
      positions (numPositions)
{
    attachment.sendInitialUpdate();
    shownFrame = targetFrame = frameFor (pos);
    setMouseCursor (MouseCursor::PointingHandCursor);
}

int LeverSwitch::frameFor (int p) const
{
    if (positions == 2) return p == 0 ? 0 : 2;     // EQ : off = levier en haut, on = levier en bas
    return jlimit (0, 2, p);                        // mode : SDT-2 haut, Laerm milieu, SDT-1 bas
}

void LeverSwitch::tick (double now)
{
    if (shownFrame != targetFrame && now >= transitionUntil)
    {
        shownFrame = targetFrame;
        repaint();
    }
}

void LeverSwitch::paint (Graphics& g)
{
    const auto& im = frames[jlimit (0, 2, shownFrame)];
    if (! im.isValid()) return;
    g.setImageResamplingQuality (Graphics::highResamplingQuality);
    g.drawImage (im, getLocalBounds().toFloat());
}

void LeverSwitch::moveTo (int np)
{
    np = jlimit (0, positions - 1, np);
    if (np == pos) return;
    attachment.setValueAsCompleteGesture ((float) np);
    if (onUserChange) onUserChange();
}

void LeverSwitch::mouseDown (const MouseEvent& e)
{
    if (e.mods.isPopupMenu()) { if (onRightClick) onRightClick(); return; }
    if (positions == 2) { moveTo (1 - pos); return; }
    // inverseur 3 positions : clic au-dessus du pivot = levier vers le haut, en dessous = vers le bas
    const bool upper = e.position.y < (float) getHeight() * pivotY;
    if (upper) moveTo (pos > 0 ? pos - 1 : pos);
    else       moveTo (pos < positions - 1 ? pos + 1 : pos);
}

void LeverSwitch::mouseWheelMove (const MouseEvent&, const MouseWheelDetails& w)
{
    if (std::abs (w.deltaY) > 0.0f) moveTo (pos + (w.deltaY > 0 ? -1 : 1));
}

//==============================================================================
// FOOTSWITCH
//==============================================================================
FootSwitchArea::FootSwitchArea (RangedAudioParameter& p, std::atomic<float>* holdMomentary)
    : hold (holdMomentary),
      attachment (p, [this] (float v) { value = v > 0.5f; })
{
    attachment.sendInitialUpdate();
    setMouseCursor (MouseCursor::PointingHandCursor);
}

bool FootSwitchArea::hitTest (int x, int y)
{
    const auto c = getLocalBounds().toFloat().getCentre();
    return c.getDistanceFrom ({ (float) x, (float) y }) <= (float) getWidth() * 0.5f;
}

void FootSwitchArea::paint (Graphics& g)
{
    if (pressed)      // bouton assombri tant qu'on appuie (Echoflex)
    {
        g.setColour (Colours::black.withAlpha (0.25f));
        g.fillEllipse (getLocalBounds().toFloat().reduced ((float) getWidth() * 0.23f));
    }
}

void FootSwitchArea::mouseDown (const MouseEvent& e)
{
    if (e.mods.isPopupMenu()) { if (onRightClick) onRightClick(); return; }
    pressed = true;
    downTime = Time::getMillisecondCounterHiRes();
    turnedOnByPress = ! value;
    repaint();
    attachment.setValueAsCompleteGesture (value ? 0.0f : 1.0f);    // bascule des l'appui (comme Echoflex)
    if (onUserChange) onUserChange();
}

void FootSwitchArea::mouseUp (const MouseEvent& e)
{
    if (e.mods.isPopupMenu()) return;
    // « Smart relay » : appui maintenu = momentane -> on relache, ca se coupe
    const bool momentary = hold != nullptr && hold->load() > 0.5f;
    if (pressed && momentary && turnedOnByPress && Time::getMillisecondCounterHiRes() - downTime > 450.0 && value)
    {
        attachment.setValueAsCompleteGesture (0.0f);
        if (onUserChange) onUserChange();
    }
    pressed = false;
    repaint();
}

//==============================================================================
// EDITEUR
//==============================================================================
DoornAudioProcessorEditor::DoornAudioProcessorEditor (DoornAudioProcessor& p)
    : AudioProcessorEditor (&p), proc (p),
      eqSwitch   (*p.apvts.getParameter ("eq"), 2),
      modeSwitch (*p.apvts.getParameter ("mode"), 3),
      boostFs  (*p.apvts.getParameter ("boostOn"), nullptr),   // « hold = momentary » retire (v4.1.0)
      bypassFs (*p.apvts.getParameter ("active"),  nullptr)
{
    using namespace BinaryData;
    background = png (Doorn_Background_png, Doorn_Background_pngSize);
    boostGlow  = png (boost_glow_on_png, boost_glow_on_pngSize);
    ledOff     = png (led_off_png, led_off_pngSize);
    ledOn      = png (led_on_png, led_on_pngSize);
    {
        // LED : lentille (opaque jusqu'a ~30 px sur 180) et halo separes : clic droit > Glow ne dose que le halo
        const int w = ledOn.getWidth(), h = ledOn.getHeight();
        ledLens = Image (Image::ARGB, w, h, true);
        ledHalo = Image (Image::ARGB, w, h, true);
        const Image::BitmapData src (ledOn, Image::BitmapData::readOnly);
        Image::BitmapData lens (ledLens, Image::BitmapData::writeOnly), halo (ledHalo, Image::BitmapData::writeOnly);
        const float c = (float) (w - 1) * 0.5f, r0 = 27.0f * (float) w / 180.0f, r1 = 31.0f * (float) w / 180.0f;
        for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x)
            {
                const float d = std::hypot ((float) x - c, (float) y - c);
                const float t = jlimit (0.0f, 1.0f, (d - r0) / (r1 - r0));
                const float m = 1.0f - t * t * (3.0f - 2.0f * t);              // 1 dans la lentille, 0 dans le halo
                const auto px = src.getPixelColour (x, y);
                lens.setPixelColour (x, y, px.withMultipliedAlpha (m));
                halo.setPixelColour (x, y, px.withMultipliedAlpha (1.0f - m));
            }
    }
    knobImg.centre     = png (knob_center_fixed_190px_png, knob_center_fixed_190px_pngSize);
    knobImg.rotating   = png (knob_rotating_crown_and_arrow_190px_png, knob_rotating_crown_and_arrow_190px_pngSize);
    knobImg.reflect    = png (knob_reflect_static_190px_png, knob_reflect_static_190px_pngSize);
    knobImg.bCentreOff = png (boost_center_off_100px_png, boost_center_off_100px_pngSize);
    knobImg.bRotOff    = png (boost_rotating_off_100px_png, boost_rotating_off_100px_pngSize);
    knobImg.bCentreOn  = png (boost_center_on_100px_png, boost_center_on_100px_pngSize);
    knobImg.bRotOn     = png (boost_rotating_on_100px_png, boost_rotating_on_100px_pngSize);
    knobImg.bReflect   = png (boost_reflect_static_100px_png, boost_reflect_static_100px_pngSize);

    eqSwitch.frames[0]   = png (switch_left_up_png, switch_left_up_pngSize);
    eqSwitch.frames[1]   = png (switch_left_mid_png, switch_left_mid_pngSize);
    eqSwitch.frames[2]   = png (switch_left_down_png, switch_left_down_pngSize);
    modeSwitch.frames[0] = png (switch_right_up_png, switch_right_up_pngSize);
    modeSwitch.frames[1] = png (switch_right_mid_png, switch_right_mid_pngSize);
    modeSwitch.frames[2] = png (switch_right_down_png, switch_right_down_pngSize);
    eqSwitch.pivotY = modeSwitch.pivotY = layout::switchPivotY;

    // Poppins embarquee : ecran de presets, ligne d'etat et menus (design d'Octopus)
    regularFace = Typeface::createSystemTypefaceFor (PoppinsRegular_ttf, PoppinsRegular_ttfSize);
    labelFace   = Typeface::createSystemTypefaceFor (PoppinsMedium_ttf, PoppinsMedium_ttfSize);
    boldFace    = Typeface::createSystemTypefaceFor (PoppinsBold_ttf, PoppinsBold_ttfSize);
    menuLnf.f = { regularFace, labelFace, boldFace };

    rightClickFwd.callback = [this] (Component* c)
    {
        int idx = -1;
        for (int i = 0; i < DoornAudioProcessor::numKnobs; ++i)
            if (c == &knobs[i]) idx = i;
        showContextMenu (idx);
    };

    for (int i = 0; i < DoornAudioProcessor::numKnobs; ++i)
    {
        auto& s = knobs[i];
        s.isMini = (i == 1);
        s.diamRatio = layout::knobDiamRatio[i];
        s.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle (Slider::NoTextBox, false, 0, 0);
        s.setRotaryParameters (rotStart, rotEnd, true);
        s.img = &knobImg;
        s.startAngle = rotStart; s.endAngle = rotEnd;
        s.setMouseDragSensitivity (220);
        s.setDoubleClickReturnValue (true, 5.0);
        s.setMouseCursor (MouseCursor::PointingHandCursor);
        s.addMouseListener (&rightClickFwd, false);
        s.onDragStart = [this] { proc.currentPreset = -1; };
        addAndMakeVisible (s);
        knobAtt[i] = std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, DoornAudioProcessor::knobId (i), s);
    }

    eqSwitch.onRightClick   = [this] { showContextMenu (6); };
    modeSwitch.onRightClick = [this] { showContextMenu (7); };
    for (auto* l : { &eqSwitch, &modeSwitch })
    {
        l->onUserChange = [this] { proc.currentPreset = -1; };
        addAndMakeVisible (*l);
    }
    boostFs.onRightClick  = [this] { showContextMenu (8); };
    bypassFs.onRightClick = [this] { showContextMenu (9); };
    boostFs.onUserChange  = [this] { proc.currentPreset = -1; };
    addAndMakeVisible (boostFs);
    addAndMakeVisible (bypassFs);

    for (int i = 0; i < numValueIds; ++i)
        lastValues[i] = proc.raw (valueIds[i]);
    lastPresetCounter = proc.presetCounter.load();
    lastMidiCount = proc.midiCounter.load();
    ledLevel = proc.raw ("active") > 0.5f ? 1.0f : 0.0f;
    glowPc = NineHandsGlow::percent (JucePlugin_Name);
    boostLit = proc.raw ("boostOn") > 0.5f ? 1.0f : 0.0f;
    knobs[1].lit = boostLit;

    // Taille par defaut = taille minimale (impossible de reduire en dessous), proportions bloquees
    const int defaultW = roundToInt ((float) defaultH * baseW / baseH);
    setResizable (true, true);
    const int maxW = roundToInt ((float) maxH * baseW / baseH);
    setResizeLimits (defaultW, defaultH, maxW, maxH);
    if (auto* cons = getConstrainer())
        cons->setFixedAspectRatio ((double) baseW / (double) baseH);
    hostLock.setLimits (defaultW, defaultH, maxW, maxH);
    setOpaque (true);                      // anti-gel : l'hote ne redessine jamais ce qui est derriere la fenetre
    setSize (defaultW, defaultH);
    // taille choisie au menu pour ce projet, sinon celle de « Default size (new windows) »
    setScaledHeight (sizeHeight (NineHandsWindow::openIndex (JucePlugin_Name, proc.uiSizeIdx.load(), numSizes)));
    rebuildCaches();
    startTimerHz (30);
}

DoornAudioProcessorEditor::~DoornAudioProcessorEditor()
{
    stopTimer();
    PopupMenu::dismissAllActiveMenus();
    for (auto& s : knobs)
    {
        s.removeMouseListener (&rightClickFwd);
    }
}

//==============================================================================
int DoornAudioProcessorEditor::sizeHeight (int idx)
{
    const int h[numSizes] = { (int) defaultH, roundToInt (refH * 0.75f), (int) refH };
    return h[jlimit (0, numSizes - 1, idx)];
}

void DoornAudioProcessorEditor::setScaledHeight (int h)
{
    h = jlimit ((int) defaultH, (int) maxH, h);
    setSize (roundToInt ((float) h * baseW / baseH), h);
    hostLock.enforce();
}

Rectangle<int> DoornAudioProcessorEditor::scaled (Rectangle<float> r) const
{
    return r.transformedBy (AffineTransform::scale (unitScale())).getSmallestIntegerContainer();
}

float DoornAudioProcessorEditor::physicalScale() const
{
    if (forcedScale > 0.0f) return forcedScale;
    return jlimit (1.0f, 4.0f, (float) Component::getApproximateScaleFactorForComponent (this));
}

void DoornAudioProcessorEditor::showStatus()
{
    statusUntil = Time::getMillisecondCounterHiRes() + 2500.0;
}

//==============================================================================
// Cache d'affichage : fond deja a la taille physique de la fenetre (net sur Retina / 4K)
//==============================================================================
void DoornAudioProcessorEditor::rebuildCaches (float sc)
{
    if (sc <= 0.0f) sc = cacheScale > 0.0f ? cacheScale : physicalScale();
    const int w = getWidth(), h = getHeight();
    if (w <= 0 || h <= 0) return;
    if (w == cacheW && h == cacheH && std::abs (sc - cacheScale) < 0.01f && bgScaled.isValid()) return;

    cacheW = w; cacheH = h; cacheScale = sc;
    const int pw = roundToInt ((float) w * sc), ph = roundToInt ((float) h * sc);
    bgScaled = (ph >= background.getHeight()) ? background : background.rescaled (pw, ph, Graphics::highResamplingQuality);
    repaint();
}

void DoornAudioProcessorEditor::resized()
{
    // anti-gel : jamais de setSize ici (les limites du constrainer suffisent)
    for (int i = 0; i < DoornAudioProcessor::numKnobs; ++i)
        knobs[i].setBounds (scaled (layout::knobRect[i]));
    eqSwitch.setBounds (scaled (layout::eqRect));
    modeSwitch.setBounds (scaled (layout::modeRect));
    auto fs = [] (Point<float> c) { return Rectangle<float> (layout::footR * 2.0f, layout::footR * 2.0f).withCentre (c); };
    boostFs.setBounds  (scaled (fs (layout::boostFsPos)));
    bypassFs.setBounds (scaled (fs (layout::bypassFsPos)));
    resizeSettle = 4;
}

//==============================================================================
// Anti-gel v2 (NINE HANDS | DSP) :
//  1. moteur de rendu logiciel force sur Windows (Direct2D peut figer la fenetre dans certains hotes),
//     applique a chaque nouvelle fenetre hote (peer) et reverifie toutes les secondes, jamais en boucle ;
//  2. aucun calcul lourd dans paint() : le cache du fond est reconstruit par le timer seulement ;
//  3. chien de garde : si un redessin demande n'est pas arrive au bout de 1,5 s alors que la fenetre est
//     visible, le moteur de rendu est reapplique et toute la fenetre est redessinee ;
//  4. cote audio : aucun appel a l'hote ni allocation depuis le thread audio (voir PluginProcessor).
void DoornAudioProcessorEditor::useSoftwareRenderer()
{
    auto* peer = getPeer();
    if (peer != lastPeer) { lastPeer = peer; repaint(); }
   #if JUCE_WINDOWS
    if (peer != nullptr)
    {
        int sw = peer->getAvailableRenderingEngines().indexOf ("Software Renderer");
        if (sw < 0) sw = 0;
        if (peer->getCurrentRenderingEngine() != sw)
            peer->setCurrentRenderingEngine (sw);
    }
   #endif
}

void DoornAudioProcessorEditor::parentHierarchyChanged() { useSoftwareRenderer(); hostLock.track (*this); }
void DoornAudioProcessorEditor::visibilityChanged()      { useSoftwareRenderer(); if (isShowing()) repaint(); }

void DoornAudioProcessorEditor::askRepaint (Rectangle<int> r)
{
    if (repaintAskedMs <= 0.0) repaintAskedMs = Time::getMillisecondCounterHiRes();
    repaint (r);
}

//==============================================================================
String DoornAudioProcessorEditor::buildStatusLine() const
{
    // seulement le reglage de la commande qui vient de bouger
    const int i = lastChanged;
    if (i < 0) return {};
    const String id = valueIds[i];
    const float v = proc.raw (valueIds[i]);
    if (i < 6) return id + "  " + String (roundToInt (v * 10.0f)) + " %";
    if (id == "eq")      return v > 0.5f ? "eq on" : "eq off";
    if (id == "mode")    return DoornAudioProcessor::modeNames()[jlimit (0, 2, roundToInt (v))].toLowerCase();
    if (id == "boostOn") return v > 0.5f ? "boost on" : "boost off";
    if (id == "gate")    return "noise gate " + DoornAudioProcessor::gateNames()[jlimit (0, 3, roundToInt (v))].toLowerCase();
    return v > 0.5f ? "effect on" : "bypass";
}

String DoornAudioProcessorEditor::buildPresetLine() const
{
    const int cp = proc.currentPreset.load();
    String s;
    if (cp >= 0 && cp < DoornAudioProcessor::getNumFactoryPresets())
        s << (cp + 1) << ". " << DoornAudioProcessor::getFactoryPreset (cp).name;
    else if (cp > 100) s << "User " << (cp - 100);
    else               s << "Edited";
    if (proc.raw ("active") < 0.5f) s << "  (bypass)";
    return s;
}

//==============================================================================
void DoornAudioProcessorEditor::timerCallback()
{
    const double now = Time::getMillisecondCounterHiRes();
    if (getPeer() != lastPeer || now - lastRendererCheckMs > 1000.0) { lastRendererCheckMs = now; useSoftwareRenderer(); }
    hostLock.track (*this);

    // cache : reconstruit ici seulement (jamais dans paint)
    if (resizeSettle > 0 && --resizeSettle == 0) rebuildCaches (forcedScale > 0.0f ? forcedScale : 0.0f);
    else if (resizeSettle == 0 && forcedScale <= 0.0f && now - lastScaleCheckMs > 250.0)
    {
        lastScaleCheckMs = now;
        const float sc = physicalScale();
        if (std::abs (sc - cacheScale) > 0.01f) rebuildCaches (sc);
    }

    // chien de garde : un redessin demande n'arrive pas -> on relance le moteur de rendu et on redessine tout
    if (repaintAskedMs > 0.0 && isShowing() && now - repaintAskedMs > 1500.0 && lastPaintMs < repaintAskedMs)
    {
        repaintAskedMs = 0.0;
        lastPeer = nullptr;
        useSoftwareRenderer();
        repaint();
    }

    // potards : image prete recalculee ici (rotation, flou de mouvement, fondu du BOOST)
    for (auto& k : knobs) k.tick (cacheScale > 0.0f ? cacheScale : physicalScale());

    eqSwitch.tick (now);
    modeSwitch.tick (now);

    // la ligne d'etat apparait quand un reglage bouge, reste 2,5 s puis s'efface
    int changed = -1, numChanged = 0;
    for (int i = 0; i < numValueIds; ++i)
    {
        const float v = proc.raw (valueIds[i]);
        if (std::abs (v - lastValues[i]) > 1.0e-4f) { lastValues[i] = v; changed = i; ++numChanged; }
    }
    const int pcount = proc.presetCounter.load();
    if (pcount != lastPresetCounter) { lastPresetCounter = pcount; numChanged = 0; }   // preset : affiche dans l'ecran
    if (numChanged == 1) { lastChanged = changed; showStatus(); learnLineUntil = 0.0; }
    for (int i = 0; i < DoornAudioProcessor::numKnobs; ++i)
        if (knobs[i].isMouseButtonDown()) { lastChanged = i; showStatus(); learnLineUntil = 0.0; }
    // MIDI Learn : attente puis confirmation 2 s dans la ligne d'etat
    const int lt = proc.learnTarget.load();
    const int learned = proc.lastLearned.exchange (-1);
    if (learned >= 0)
    {
        const int cc = proc.ccMap[learned].load();
        learnLine = String (DoornAudioProcessor::learnDef (learned).name) + " = CC " + String (cc);
        learnLineUntil = now + 2000.0;
        statusUntil = jmax (statusUntil, learnLineUntil);
    }
    else if (lt >= 0)
    {
        learnLine = String (CharPointer_UTF8 ("MIDI LEARN \xc2\xb7 ")) + DoornAudioProcessor::learnDef (lt).name;
        learnLineUntil = now + 200.0;
        statusUntil = jmax (statusUntil, now + 200.0);
    }

    const int mc = proc.midiCounter.load();
    if (mc != lastMidiCount) { lastMidiCount = mc; midiFlash = 8; }
    if (midiFlash > 0) --midiFlash;

    // fondu croise court : ~120 ms a l'apparition de la ligne d'etat, ~300 ms au retour de la barre de presets
    const float target = now < statusUntil ? 1.0f : 0.0f;
    if (target > statusAlpha) statusAlpha = jmin (1.0f, statusAlpha + 0.28f);
    else if (target < statusAlpha) statusAlpha = jmax (0.0f, statusAlpha - 0.11f);

    // clic droit > Glow : reglage commun a toutes les fenetres, relu toutes les 3 s (jamais dans paint)
    if (now - lastGlowReadMs > 3000.0)
    {
        lastGlowReadMs = now;
        const int pc = NineHandsGlow::percent (JucePlugin_Name);
        if (pc != glowPc) setGlowPercent (pc);
    }

    // LED blanche = effet actif ; fondu (0,45 a l'allumage / 0,28 a l'extinction)
    const bool on = proc.raw ("active") > 0.5f;
    ledLevel += ((on ? 1.0f : 0.0f) - ledLevel) * (on ? 0.45f : 0.28f);
    if (ledLevel < 0.003f) ledLevel = 0.0f;
    if (ledLevel > 0.997f) ledLevel = 1.0f;
    if (std::abs (ledLevel - drawnLed) > 0.004f)
    {
        drawnLed = ledLevel;
        askRepaint (scaled (glowArea (layout::ledRect)));
    }

    // potard BOOST (mini pack v018) : etat pilote par le footswitch BOOST seul, bascule nette OFF / ON
    // (sprites *_off_* sans halo, sprites *_on_* + halo), redessin au changement d'etat
    boostLit = proc.raw ("boostOn") > 0.5f ? 1.0f : 0.0f;
    if (boostLit != drawnBoost)
    {
        drawnBoost = boostLit;
        knobs[1].lit = boostLit;
        askRepaint (scaled (glowArea (layout::boostGlowRect)));
    }

    // (le message de learn reste pendant le fondu de sortie)
    const auto l1 = (learnLineUntil > 0.0 && now < learnLineUntil + 800.0 ? learnLine : buildStatusLine()), pl = buildPresetLine();
    const bool textChanged = l1 != lastLine1;
    lastLine1 = l1;
    if (std::abs (statusAlpha - drawnStatus) > 0.003f || (statusAlpha > 0.0f && textChanged))
    {
        drawnStatus = statusAlpha;
        askRepaint (scaled (screenRect.expanded (3.0f)));
    }
    if (pl != lastPreset || hoverZone != drawnHover || midiFlash > 0)
    {
        lastPreset = pl; drawnHover = hoverZone;
        askRepaint (scaled (screenRect.expanded (3.0f)));
    }
}

//==============================================================================
// clic droit > Glow : 25 / 50 / 75 / 100 % ; seul le halo est dose, la lentille de la LED et le verre du BOOST restent pleins
void DoornAudioProcessorEditor::setGlowPercent (int pc)
{
    glowPc = NineHandsGlow::sanitize (pc);
    askRepaint (scaled (glowArea (layout::ledRect)));
    askRepaint (scaled (glowArea (layout::boostGlowRect)));
}

void DoornAudioProcessorEditor::drawLed (Graphics& g)
{
    g.setImageResamplingQuality (Graphics::highResamplingQuality);
    drawImageIn (g, ledOff, layout::ledRect);
    drawImageIn (g, ledHalo, layout::ledRect, ledLevel * glow());   // halo (clic droit > Glow)
    drawImageIn (g, ledLens, layout::ledRect, ledLevel);            // lentille blanc vif quand l'effet est actif
}

void DoornAudioProcessorEditor::drawScreen (Graphics& g)
{
    // une seule pastille, a la taille de reference (Strong Pickle), avec la matiere de l'ecran du design :
    // fin contour sombre, liseré gris clair de 1 px, face noire ; barre de presets au repos, ligne d'etat pendant
    // les reglages, en fondu croise, meme police et meme taille
    const auto r = screenRect;
    const float rad = r.getHeight() * 0.5f;
    g.setColour (Colour (0xff151516));
    g.fillRoundedRectangle (r.expanded (0.6f), rad + 0.6f);
    g.setColour (Colour (0xff6f6f71).withAlpha (hoverZone != 0 && statusAlpha <= 0.0f ? 1.0f : 0.85f));
    g.fillRoundedRectangle (r, rad);
    g.setColour (screenFace);
    g.fillRoundedRectangle (r.reduced (0.6f), rad - 0.6f);

    const float a = statusAlpha, pa = 1.0f - a;
    g.setFont (Font (FontOptions (labelFace).withHeight (barFont)));
    if (pa > 0.0f)
    {
        auto arrow = [&] (Rectangle<float> ar, bool left, bool hot)
        {
            Path p; const auto c = ar.getCentre(); const float sz = arrowSize;
            if (left) p.addTriangle (c.x + sz * 0.6f, c.y - sz, c.x + sz * 0.6f, c.y + sz, c.x - sz * 0.8f, c.y);
            else      p.addTriangle (c.x - sz * 0.6f, c.y - sz, c.x - sz * 0.6f, c.y + sz, c.x + sz * 0.8f, c.y);
            g.setColour (inkWhite.withAlpha ((hot ? 1.0f : 0.6f) * pa));
            g.fillPath (p);
        };
        arrow (r.withWidth (arrowZone), true, hoverZone == 1);
        arrow (r.withTrimmedLeft (r.getWidth() - arrowZone), false, hoverZone == 3);
        g.setColour ((midiFlash > 0 ? Colour (0xffffd9a0) : inkWhite).withAlpha ((hoverZone == 2 ? 1.0f : 0.92f) * pa));
        g.drawFittedText (lastPreset, r.reduced (arrowZone, 0.0f).toNearestInt(), Justification::centred, 1, 0.7f);
    }
    if (a > 0.0f)
    {
        g.setColour (inkWhite.withAlpha (a));
        g.drawFittedText (lastLine1, r.reduced (rad, 0.0f).toNearestInt(), Justification::centred, 1, 0.6f);
    }
}

void DoornAudioProcessorEditor::paint (Graphics& g)
{
    lastPaintMs = Time::getMillisecondCounterHiRes();
    repaintAskedMs = 0.0;
    g.fillAll (Colour (0xff111112));          // autour des jacks : le fond de l'image est transparent
    // anti-gel : aucun rescale ici, on dessine le cache existant (ou l'original en attendant le timer)
    const Image& bg = bgScaled.isValid() ? bgScaled : background;
    g.setImageResamplingQuality (&bg == &background ? Graphics::highResamplingQuality : Graphics::lowResamplingQuality);
    if (bg.isValid())
        g.drawImage (bg, getLocalBounds().toFloat());

    g.addTransform (AffineTransform::scale (unitScale()));
    // halo du potard BOOST (sous le potard, seulement quand le boost est enclenche)
    g.setImageResamplingQuality (Graphics::highResamplingQuality);
    drawImageIn (g, boostGlow, layout::boostGlowRect, boostLit * glow());
    drawLed (g);
    drawScreen (g);
}

//==============================================================================
// Souris : ecran de presets (fleches = precedent / suivant, nom = menu deroulant), clic droit = menu
//==============================================================================
static int presetZoneAt (Point<float> p)
{
    if (! screenRect.contains (p)) return 0;
    if (p.x < screenRect.getX() + arrowZone) return 1;
    if (p.x > screenRect.getRight() - arrowZone) return 3;
    return 2;
}

void DoornAudioProcessorEditor::mouseMove (const MouseEvent& e)
{
    hoverZone = presetZoneAt (e.position / unitScale());
    setMouseCursor (hoverZone != 0 ? MouseCursor::PointingHandCursor : MouseCursor::NormalCursor);
}

void DoornAudioProcessorEditor::mouseExit (const MouseEvent&) { hoverZone = 0; }

void DoornAudioProcessorEditor::mouseDown (const MouseEvent& e)
{
    if (e.mods.isPopupMenu()) { showContextMenu(); return; }
    const int zone = presetZoneAt (e.position / unitScale());
    if (zone != 0 && statusAlpha > 0.0f)
    {
        // la ligne d'etat ne bloque pas les clics : la barre de presets revient et le clic agit
        statusUntil = 0.0; learnLineUntil = 0.0; statusAlpha = 0.0f;
        askRepaint (scaled (screenRect.expanded (3.0f)));
    }
    switch (zone)
    {
        case 1: stepPreset (-1); break;
        case 2: showPresetMenu(); break;
        case 3: stepPreset (1); break;
        default: break;
    }
}

void DoornAudioProcessorEditor::stepPreset (int delta)
{
    const int n = DoornAudioProcessor::getNumFactoryPresets();
    int cp = proc.currentPreset.load();
    if (cp < 0 || cp >= n) cp = delta > 0 ? -1 : 0;
    proc.applyFactoryPreset ((cp + delta + n) % n);
}

void DoornAudioProcessorEditor::showPresetMenu()
{
    PopupMenu m;
    m.setLookAndFeel (&menuLnf);
    NineHandsMenu::addHeader (m, menuLnf.f, NineHandsMenu::icoPreset, menuAccent, "Presets", buildPresetLine());
    m.addSectionHeader ("Factory presets");
    for (int i = 0; i < DoornAudioProcessor::getNumFactoryPresets(); ++i)
        m.addItem (100 + i, String (i + 1) + ". " + DoornAudioProcessor::getFactoryPreset (i).name, true, proc.currentPreset.load() == i);
    m.addSeparator();
    PopupMenu load, save;
    for (int slot = 1; slot <= 9; ++slot)
    {
        const bool exists = proc.hasUserPreset (slot);
        load.addItem (200 + slot, "User " + String (slot) + (exists ? "" : "  (empty)"), exists, proc.currentPreset.load() == 100 + slot);
        save.addItem (300 + slot, "User " + String (slot) + (exists ? "  (overwrite)" : ""));
    }
    m.addSubMenu ("Load user preset", load);
    m.addSubMenu ("Save user preset", save);

    // menu de presets ouvert d'un clic gauche sur l'ecran : accroche a l'ecran
    m.showMenuAsync (PopupMenu::Options().withTargetScreenArea (localAreaToGlobal (scaled (screenRect))),
        [safe = Component::SafePointer<DoornAudioProcessorEditor> (this)] (int r)
        {
            if (safe == nullptr || r == 0) return;
            auto& pr = safe->proc;
            if      (r >= 100 && r < 200) pr.applyFactoryPreset (r - 100);
            else if (r > 200 && r < 300)  pr.loadUserPreset (r - 200);
            else if (r > 300 && r < 400)  pr.saveUserPreset (r - 300);
        });
}

void DoornAudioProcessorEditor::handleMidiMenu (int r)
{
    if (r >= 1000 && r < 1000 + DoornAudioProcessor::numLearn)      proc.learnTarget.store (r - 1000);
    else if (r >= 1100 && r < 1100 + DoornAudioProcessor::numLearn) { proc.ccMap[r - 1100].store (-1); }
    else if (r == 1200) proc.learnTarget.store (-1);
    else if (r == 1201) { proc.learnTarget.store (-1); proc.clearMidiMap(); }
}

PopupMenu DoornAudioProcessorEditor::buildContextMenu (int learnIdx)
{
    using namespace NineHandsMenu;
    PopupMenu menu;
    menu.setLookAndFeel (&menuLnf);
    auto ccText = [this] (int i)
    {
        const int cc = proc.ccMap[i].load();
        return cc >= 0 ? "CC " + String (cc) : String ("not assigned");
    };
    const int lt = proc.learnTarget.load();

    if (learnIdx >= 0)
    {
        // menu d'une commande (tronc commun) : en-tete, MIDI Learn, Clear MIDI, Cancel MIDI learn
        const String name = DoornAudioProcessor::learnDef (learnIdx).name;
        const int ico = learnIdx < 6 ? icoKnob : (learnIdx < 8 ? icoSwitch : icoPedal);
        addHeader (menu, menuLnf.f, ico, menuAccent, name, "MIDI: " + ccText (learnIdx));
        menu.addItem (1000 + learnIdx, "MIDI Learn: " + name, true, lt == learnIdx);
        menu.addItem (1100 + learnIdx, "Clear MIDI: " + name, proc.ccMap[learnIdx].load() >= 0);
        menu.addItem (1200, "Cancel MIDI learn", lt >= 0);
    }
    else
    {
        // menu de la pedale (tronc commun) : presets, reglages propres, MIDI, Plugin size, Glow intensity
        addHeader (menu, menuLnf.f, icoPedal, menuAccent, "Doorn", "Distortion  v" + String (JucePlugin_VersionString));

        PopupMenu factory;
        for (int i = 0; i < DoornAudioProcessor::getNumFactoryPresets(); ++i)
            factory.addItem (100 + i, String (i + 1) + ". " + DoornAudioProcessor::getFactoryPreset (i).name, true, proc.currentPreset.load() == i);
        menu.addSubMenu ("Factory presets", factory);
        PopupMenu load, save;
        for (int slot = 1; slot <= 9; ++slot)
        {
            const bool exists = proc.hasUserPreset (slot);
            load.addItem (200 + slot, "User " + String (slot) + (exists ? "" : "  (empty)"), exists, proc.currentPreset.load() == 100 + slot);
            save.addItem (300 + slot, "User " + String (slot) + (exists ? "  (overwrite)" : ""));
        }
        menu.addSubMenu ("Load user preset", load);
        menu.addSubMenu ("Save user preset", save);
        menu.addSeparator();

        PopupMenu gate;
        const int gNow = jlimit (0, 3, roundToInt (proc.raw ("gate")));
        const char* gateHelp[4] = { "Off", "Light (very quiet setup)", "Medium (default)", "Strong (noisy pickups / hum)" };
        for (int i = 0; i < 4; ++i) gate.addItem (50 + i, gateHelp[i], true, gNow == i);
        menu.addSubMenu ("Noise gate", gate);
        menu.addSeparator();

        PopupMenu midi;
        midi.addSectionHeader ("MIDI Learn");
        for (int i = 0; i < DoornAudioProcessor::numLearn; ++i)
            midi.addItem (1000 + i, "Learn: " + String (DoornAudioProcessor::learnDef (i).name) + "  (" + ccText (i) + ")", true, lt == i);
        midi.addSeparator();
        midi.addItem (1200, "Cancel MIDI learn", lt >= 0);
        midi.addItem (1201, "Clear all MIDI assignments");
        midi.addSeparator();
        NineHandsWindow::addMidiFootswitchMode (midi, proc.raw ("midiFsMode") < 0.5f);
        midi.addSeparator();
        NineHandsWindow::addProgramChangeInfo (midi);
        menu.addSubMenu ("MIDI", midi);
        menu.addSeparator();

        const char* names[numSizes] = { "60 %", "75 %", "100 %" };
        const int heights[numSizes] = { sizeHeight (0), sizeHeight (1), sizeHeight (2) };
        menu.addSubMenu ("Plugin size", NineHandsWindow::sizeMenu (JucePlugin_Name, numSizes, names, heights, getHeight()));
        menu.addSubMenu ("Glow intensity", NineHandsWindow::glowMenu (glowPc));
    }
    return menu;
}

void DoornAudioProcessorEditor::showContextMenu (int learnIdx)
{
    auto menu = buildContextMenu (learnIdx);
    menu.showMenuAsync (PopupMenu::Options().withMousePosition(),       // menu clic droit : toujours la ou l'on clique
        [safe = Component::SafePointer<DoornAudioProcessorEditor> (this)] (int r)
        {
            if (safe == nullptr || r == 0) return;
            auto& ed = *safe;
            auto& pr = ed.proc;
            int inst = pr.uiSizeIdx.load();
            const int sz = NineHandsWindow::handleSize (r, JucePlugin_Name, numSizes, inst);
            if (sz >= 0) { pr.uiSizeIdx = inst; ed.setScaledHeight (ed.sizeHeight (sz)); return; }
            if (const int pc = NineHandsWindow::handleGlow (r, JucePlugin_Name))
            {
                ed.setGlowPercent (pc);
                ed.learnLine = "Glow  " + String (pc) + " %";        // ligne d'etat, brievement
                ed.learnLineUntil = Time::getMillisecondCounterHiRes() + 1500.0;
                ed.statusUntil = jmax (ed.statusUntil, ed.learnLineUntil);
                return;
            }
            if      (r == NineHandsWindow::idLatch)     pr.setParamValue ("midiFsMode", 0.0f);
            else if (r == NineHandsWindow::idMomentary) pr.setParamValue ("midiFsMode", 1.0f);
            else if (r >= 50 && r <= 53) pr.setParamValue ("gate", (float) (r - 50));
            else if (r >= 100 && r < 200) pr.applyFactoryPreset (r - 100);
            else if (r > 200 && r < 300)  pr.loadUserPreset (r - 200);
            else if (r > 300 && r < 400)  pr.saveUserPreset (r - 300);
            else if (r >= 1000) { ed.handleMidiMenu (r); return; }
            ed.showStatus();
        });
}
