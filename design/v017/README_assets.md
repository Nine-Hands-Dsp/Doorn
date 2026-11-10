# Doorn – VST GUI assets (v017)

Native resolution **965 × 1800 px** (origin = top-left). `manifest.json` lists every sprite (position, size, pivot, state files).
`preview.html` is an interactive demo: open it from inside this folder (drag knobs, click switches, click the two footswitches).
`Doorn_Reference_Full_v017.png` is the target look with everything at rest (arrows up, left switch down, right switch mid, LED off, boost off).

## Draw order
1. `Doorn_Background.png` – case, plate, labels, logos, jacks, preset-screen frame + its two arrows, the two footswitches, plus the **baked** cast shadows/occlusion of the knobs and LED and the frosted blur under the boost knob.
2. `sprites/boost_glow_on.png` – only when boost is ON (under the boost knob, never rotated).
3. Knob layers (see below).
4. Switch sprite (current position only – the cast shadow is included).
5. LED sprite.

## Standard knobs (gain, level, treble, mids, bass) – 200×200, knob diameter 190, pivot = sprite centre
| layer | file | behaviour |
|---|---|---|
| centre disc | `knob_center_fixed_190px.png` | fixed |
| crown + arrow | `knob_rotating_crown_and_arrow_190px.png` | **rotates** around the pivot |
| reflet | `knob_reflect_static_190px.png` | static, drawn on top (never rotated) |
(`knob_crown_only_190px.png` and `knob_arrow_only_190px.png` are provided separately if you prefer to rotate them independently.)
Rotation: value 0 → −135°, 0.5 → 0° (arrow up), 1 → +135° (clockwise positive). Full-resolution sources are in `sources_800/` (canvas 800, pivot 400,400).

## Boost knob (frosted white glass) – 108×108, diameter 100, pivot = centre
OFF: `boost_center_off_100px` (fixed) + `boost_rotating_off_100px` (crown + white line indicator, **rotates**) + `boost_reflect_static_100px` (static, on top).
ON (boost footswitch): `boost_center_on_100px` + `boost_rotating_on_100px` (rotates) + same reflect + `boost_glow_on.png` (280×280) behind.
`boost_crown_*` and `boost_indicator_*` are provided separately if you prefer to rotate them independently. In the ON state the white line has less contrast on the lit glass (it is pure white by design).

## Switches (both 3 positions)
`sprites/switch_<left|right>_<up|mid|down>.png` – same crop for the 3 states of a switch, drawn at the x/y in the manifest.
left: up = label "off", down = label "on". right: up = "sdt-2", mid = "laerm", down = "sdt-1".

## LED / footswitches / screen
* Bottom LED (482, 1325): `led_off.png` → `led_on.png` (vivid white + glow) when the **bypass footswitch** (right) engages the effect.
* Boost light: the **boost footswitch** (left) switches the boost knob and its glow to the ON layers.
* Footswitch hit zones: boost (220, 1565) r=120, bypass (745, 1565) r=120. They are static in the background (no pressed state).
* Preset screen: x=220, y=1175, 525×56, radius 28. Draw the preset name at runtime on top of the background.

## JUCE sketch
```cpp
// Rotary slider: fixed centre, rotating crown(+arrow), static reflet
void drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float, float, juce::Slider&) override
{
    const float angle = juce::degreesToRadians (-135.0f + 270.0f * pos);
    g.drawImageAt (centre, x, y);
    g.drawImageTransformed (rotating, juce::AffineTransform::rotation (angle, rotating.getWidth() * 0.5f, rotating.getHeight() * 0.5f).translated ((float) x, (float) y));
    g.drawImageAt (reflet, x, y);
}
```
Switch: keep 3 images, draw the one matching the parameter. LED and boost: swap images when the footswitch parameters change.

Note: made without access to the Doorn repository (not reachable from my environment) – adapt file names / positions to your engine as needed.
