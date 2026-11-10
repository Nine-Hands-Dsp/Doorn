// Captures PNG de la fenetre reelle de Doorn (meme code que le plugin), pour l'apercu et la notice.
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "BinaryData.h"
using namespace juce;

static void save (const Image& img, const String& path)
{
    File f (path); f.deleteFile();
    FileOutputStream os (f); PNGImageFormat().writeImageToStream (img, os);
    std::printf ("%s (%d x %d)\n", f.getFullPathName().toRawUTF8(), img.getWidth(), img.getHeight());
}

int main (int argc, char** argv)
{
    ScopedJuceInitialiser_GUI init;
    const String out = argc > 1 ? argv[1] : ".";
    DoornAudioProcessor proc;
    proc.prepareToPlay (48000.0, 256);
    std::unique_ptr<AudioProcessorEditor> ed (proc.createEditor());
    auto* ded = static_cast<DoornAudioProcessorEditor*> (ed.get());
    std::printf ("taille par defaut : %d x %d\n", ed->getWidth(), ed->getHeight());
    auto pump = [] (int n) { for (int i = 0; i < n; ++i) MessageManager::getInstance()->runDispatchLoopUntil (40); };

    auto shot = [&] (const String& name, int preset, bool active, bool boost, bool status, int height, float scale)
    {
        ded->setScaledHeight (height);
        ded->forceRenderScale (scale);
        proc.applyFactoryPreset (preset);
        proc.setParamValue ("active", active ? 1.f : 0.f);
        proc.setParamValue ("boostOn", boost ? 1.f : 0.f);
        AudioBuffer<float> b (2, 256); MidiBuffer m;
        for (int i = 0; i < 20; ++i) { b.clear(); proc.processBlock (b, m); }
        pump (30);
        if (status) proc.setParamValue ("gain", 7.6f);
        else pump (100);
        pump (12);
        save (ed->createComponentSnapshot (ed->getLocalBounds(), true, scale), out + "/" + name + ".png");
    };
    // fenetre par defaut (60 %) a l'echelle d'un ecran Retina, puis 100 %
    shot ("doorn_default", 0, true,  false, false, DoornAudioProcessorEditor::defaultH, 2.0f);
    shot ("doorn_status",  0, true,  false, true,  DoornAudioProcessorEditor::refH, 2.0f);
    shot ("doorn_boost",   5, true,  true,  false, DoornAudioProcessorEditor::refH, 2.0f);
    shot ("doorn_bypass",  2, false, false, false, DoornAudioProcessorEditor::refH, 2.0f);
    shot ("doorn_sdt2",    3, true,  false, false, DoornAudioProcessorEditor::refH, 2.0f);
    shot ("doorn_laerm",   4, true,  false, false, DoornAudioProcessorEditor::refH, 2.0f);
    // rendu exact de l'image du design (965 x 1800) : comparaison avec Doorn_Reference_Full_v017.png
    {
        ded->setScaledHeight (900); ded->forceRenderScale (2.0f);
        proc.applyFactoryPreset (0);
        for (auto* id : { "gain", "boost", "level", "treble", "mids", "bass" }) proc.setParamValue (id, 5.0f);
        proc.setParamValue ("eq", 1.0f); proc.setParamValue ("mode", 1.0f);      // levier gauche en bas (on), droit au milieu
        proc.setParamValue ("active", 0.0f); proc.setParamValue ("boostOn", 0.0f);
        pump (140);
        save (ed->createComponentSnapshot (ed->getLocalBounds(), true, 2.0f), out + "/doorn_exact_rest.png");
        proc.setParamValue ("active", 1.0f); proc.setParamValue ("boostOn", 1.0f);
        proc.setParamValue ("gain", 7.5f); proc.setParamValue ("boost", 8.0f); proc.setParamValue ("mode", 2.0f); proc.setParamValue ("eq", 0.0f);
        pump (140);
        save (ed->createComponentSnapshot (ed->getLocalBounds(), true, 2.0f), out + "/doorn_exact_on.png");
    }

    // clic droit > Lights : halos de la LED et du potard BOOST en Low / Medium / High (effet + boost actifs)
    for (int lvl = 0; lvl < 3; ++lvl)
    {
        ded->setScaledHeight (900); ded->forceRenderScale (2.0f);
        proc.setParamValue ("active", 1.0f); proc.setParamValue ("boostOn", 1.0f);
        proc.setParamValue ("ledGlow", (float) lvl); proc.setParamValue ("boostGlow", (float) lvl);
        pump (60);
        save (ed->createComponentSnapshot (ed->getLocalBounds(), true, 2.0f), out + "/doorn_lights_" + String (lvl) + ".png");
    }
    proc.setParamValue ("ledGlow", 1.0f); proc.setParamValue ("boostGlow", 1.0f);

    // menus clic droit (design Octopus, NineHandsMenu.h) : rendu hors ecran avec le meme LookAndFeel que le plugin
    {
        NineHandsMenu::MenuLnf lnf;
        lnf.f = { Typeface::createSystemTypefaceFor (BinaryData::PoppinsRegular_ttf, BinaryData::PoppinsRegular_ttfSize),
                  Typeface::createSystemTypefaceFor (BinaryData::PoppinsMedium_ttf, BinaryData::PoppinsMedium_ttfSize),
                  Typeface::createSystemTypefaceFor (BinaryData::PoppinsBold_ttf, BinaryData::PoppinsBold_ttfSize) };
        struct It { String text; bool sep, sub, ticked, active; };
        auto render = [&] (const String& name, const String& title, const String& subtitle, int ico, const std::vector<It>& items, int hiIndex)
        {
            NineHandsMenu::Header hd (lnf.f, ico, Colour (0xffcdb995), title, subtitle);
            int w = 0, h = 0; hd.getIdealSize (w, h);
            const int border = lnf.getPopupMenuBorderSize();
            int totalH = border * 2 + h + 11, maxW = w;
            for (auto& it : items) { int iw, ih; lnf.getIdealPopupMenuItemSize (it.text, it.sep, 0, iw, ih); totalH += ih; maxW = jmax (maxW, iw + (it.sub ? 20 : 0)); }
            maxW += border * 2;
            Image img (Image::ARGB, maxW * 2, totalH * 2, true);
            Graphics g (img); g.addTransform (AffineTransform::scale (2.0f));
            lnf.drawPopupMenuBackground (g, maxW, totalH);
            int y = border;
            hd.setSize (maxW - border * 2, h);
            { Graphics::ScopedSaveState ss (g); g.setOrigin (border, y); hd.paint (g); }
            y += h;
            lnf.drawPopupMenuItem (g, { border, y, maxW - border * 2, 11 }, true, true, false, false, false, {}, {}, nullptr, nullptr); y += 11;
            for (size_t i = 0; i < items.size(); ++i)
            {
                const auto& it = items[i]; int iw, ih; lnf.getIdealPopupMenuItemSize (it.text, it.sep, 0, iw, ih);
                lnf.drawPopupMenuItem (g, { border, y, maxW - border * 2, ih }, it.sep, it.active, (int) i == hiIndex, it.ticked, it.sub, it.text, {}, nullptr, nullptr);
                y += ih;
            }
            save (img, out + "/" + name + ".png");
        };
        const String v = JucePlugin_VersionString;
        render ("doorn_menu", "Doorn", "Distortion  v" + v, NineHandsMenu::icoPedal,
                { { "Plugin size", false, true, false, true }, { "", true }, { "Factory presets", false, true, false, true },
                  { "Load user preset", false, true, false, true }, { "Save user preset", false, true, false, true }, { "", true },
                  { "Noise gate", false, true, false, true }, { "Settings", false, true, false, true }, { "MIDI", false, true, false, true },
                  { "", true }, { "Doorn v" + v + "  NINE HANDS | DSP", false, false, false, false } }, 2);
        render ("doorn_menu_knob", "Gain", "MIDI: not assigned", NineHandsMenu::icoKnob,
                { { "MIDI Learn: Gain", false, false, false, true }, { "Clear MIDI: Gain", false, false, false, false }, { "", true },
                  { "Plugin size", false, true, false, true }, { "", true }, { "Factory presets", false, true, false, true } }, 0);
    }

    // test pedale d'expression : course 2..116 sur CC 20 (gain) ; aucun CC par defaut -> CC 20 assigne au gain comme par un MIDI Learn
    {
        proc.ccMap[0].store (20);
        auto sendCC = [&] (int v)
        {
            AudioBuffer<float> b (2, 256); MidiBuffer m; m.addEvent (MidiMessage::controllerEvent (1, 20, v), 0);
            proc.processBlock (b, m);
            for (int k = 0; k < 20; ++k) { MidiBuffer e; b.clear(); proc.processBlock (b, e); }
            return proc.raw ("gain");
        };
        const float a = sendCC (2), c = sendCC (116), mid = sendCC (59);
        std::printf ("pedale 2..116 -> gain %.2f / %.2f, mi-course %.2f\n", a, c, mid);
    }
    // logo NINE HANDS | DSP blanc en PNG (fond transparent) depuis le SVG officiel, pour la couverture de la notice
    if (argc > 2)
    {
        if (auto d = Drawable::createFromImageFile (File (argv[2])))
        {
            const auto b = d->getDrawableBounds();
            const int w = 1200, h = roundToInt (1200.0f * b.getHeight() / b.getWidth());
            Image img (Image::ARGB, w, h, true);
            Graphics g (img);
            d->drawWithin (g, Rectangle<float> ((float) w, (float) h), RectanglePlacement::stretchToFit, 1.0f);
            save (img, out + "/nine_hands_logo_blanc.png");
        }
    }
    ed.reset();
    return 0;
}
