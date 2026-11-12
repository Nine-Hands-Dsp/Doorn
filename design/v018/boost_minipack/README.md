# Doorn – boost knob mini pack (design v018)

Only the boost knob, exactly as in the current design (frosted white glass, white line indicator, lit from inside when boost is ON).
Canvas of the full GUI: **1235 × 1800 px**. Boost knob centre: **(617.5, 265)**, diameter **100 px**.

## Files
| file | role | state |
|---|---|---|
| `sprites/boost_center_off_100px.png` / `_on_` | centre disc – **fixed** | OFF / ON |
| `sprites/boost_rotating_off_100px.png` / `_on_` | crown + white line indicator – **rotates** around the pivot | OFF / ON |
| `sprites/boost_reflect_static_100px.png` | reflet – **static**, always drawn on top, same for both states | both |
| `sprites/boost_glow_on.png` | white halo (280×280) – drawn **under** the knob, only when ON | ON |
| `sprites/boost_crown_*` / `boost_indicator_*` | crown and line separately (only to rotate them independently) | |
| `sources_800/` | original 800 px layers | |
| `optional/boost_background_patch_220.png` | piece of the background (baked shadow + frosted blur under the knob) to paste at (507,155) if the repo background lost it | |
| `boost_manifest.json` | positions / pivot / states | |
| `INTEGRATION_PROMPT.md` | prompt to hand to the person (or AI assistant) who integrates it | |
| `preview_boost.html` | tiny demo: drag = rotate, click = ON/OFF | |

Sprites are 108×108 with the pivot at the centre (54,54). Top-left on the GUI: (564, 211). Glow (280×280) top-left: (478, 125).
Draw order: glow (if ON) → centre → rotating layer → reflet. Draw at integer pixel positions, no scaling.
Rotation: value 0 → −135°, 0.5 → 0° (line points up), 1 → +135°, clockwise positive.
`preview_off_on_rotated.png` shows OFF, ON and ON rotated.
