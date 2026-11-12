# Prompt – restore the Doorn boost knob (copy/paste)

**Role:** you are integrating a GUI asset into the Doorn pedal plugin of this repository. Read `README.md` and `boost_manifest.json` of the mini pack first.

**Goal:** the boost knob does not behave as expected after local modifications. Replace its whole GUI implementation with the layered-sprite configuration below and put it back in its **original configuration**: same position, size, parameter binding and behaviour. Do not touch any other control.

**What to do**
1. Locate the code that creates, lays out, draws and animates the boost knob (component / look-and-feel / skin / custom drawing, plus the glow or light logic tied to the boost footswitch). Revert or remove the custom changes that deviate from this spec. Keep the existing parameter IDs and attachments (boost amount parameter, boost footswitch/on-off parameter) - only the visuals and behaviour change.
2. Import the PNGs from `sprites/` as binary resources (full resolution, alpha preserved, no resampling).
3. Layout (GUI canvas 1235x1800 px): knob sprite top-left **(564, 211)**, size 108x108, pivot **(54, 54)** = centre **(617.5, 265)**. Glow sprite top-left **(478, 125)**, size 280x280. If the whole UI is scaled, scale all positions and sizes by the same factor.
4. Drawing order (normal alpha blending), bottom to top:
   a. `boost_glow_on.png` - only when boost is ON (never rotated);
   b. `boost_center_{off|on}_100px.png` - fixed;
   c. `boost_rotating_baked_{off|on}_100px.png` - rotated around the pivot. This sprite is intentionally OPAQUE (translucency baked in): do NOT apply extra opacity/alpha to it and do not swap it for the old translucent one;
   d. `boost_reflect_static_100px.png` - static, never rotated, always on top.
5. Rotation: angle = -135 deg + 270 deg x normalisedValue (clockwise positive; value 0.5 -> 0 deg, the white line points up). Only layer (c) rotates.
6. State: boost ON/OFF is driven by the **boost footswitch** (left footswitch, centre (355,1565), radius 120). OFF -> `*_off_*` sprites and no glow; ON -> `*_on_*` sprites + glow. Repaint when the state changes. The knob keeps rotating normally in both states.
7. The frosted blur and soft shadow under the knob are baked in `Doorn_Background.png`. If the repo background no longer has them, paste `optional/boost_background_patch_220.png` at (507, 155).

**Acceptance checklist**
- [ ] Knob centred on (617.5, 265), diameter 100 px, no scaling blur.
- [ ] Only the crown + white line rotate; centre disc and reflet stay still.
- [ ] Range -135 to +135 deg, value 0.5 = line pointing up.
- [ ] Boost footswitch toggles the OFF/ON sprites and the glow; OFF removes the glow completely.
- [ ] Parameter attachments and automation still work; no other control changed.
- [ ] Compare with `preview_off_on_rotated.png` (OFF, ON, ON rotated).

If something in the repository prevents this (for example no resource system for images), explain the blocker and propose the smallest change instead of rewriting unrelated code.

**Note on the crown turning white while rotating:** that was caused by the translucent rotating sprite losing its partial alpha in the rotation path. The baked opaque sprites fix it; the see-through background now rotates with the crown (accepted, very small). Check against `preview_baked_test.png`.
