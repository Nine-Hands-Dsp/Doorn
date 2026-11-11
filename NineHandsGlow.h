#pragma once
#include "WigaWindowSize.h"

// NINE HANDS | DSP : reglage « Glow » (halo des lumieres : LED, potard BOOST allume) du menu clic droit.
// Quatre valeurs seulement : 25 / 50 / 75 / 100 % (defaut). Commun a toutes les fenetres du plugin :
// attribut glowPercent du settings.xml du dossier de reglages (voir WigaWindowSize.h), relu par le timer.
namespace NineHandsGlow
{
    constexpr int values[4] = { 25, 50, 75, 100 };

    inline int sanitize (int pc)
    {
        int best = 100;
        for (int v : values) if (std::abs (v - pc) < std::abs (best - pc)) best = v;
        return best;
    }

    inline int percent (const juce::String& plugin)
    {
        const auto f = WigaWindowSize::file (plugin);
        if (f.existsAsFile())
            if (auto x = juce::XmlDocument::parse (f))
                return sanitize (x->getIntAttribute ("glowPercent", 100));
        return 100;
    }

    inline void setPercent (const juce::String& plugin, int pc)
    {
        const auto f = WigaWindowSize::file (plugin);
        juce::XmlElement x ("StudioWigaSettings");
        if (f.existsAsFile())
            if (auto old = juce::XmlDocument::parse (f))
                x = *old;
        x.setAttribute ("glowPercent", sanitize (pc));
        f.getParentDirectory().createDirectory();
        x.writeTo (f);
    }
}
