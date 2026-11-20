// SmartPark - ui.h
// immediate-mode UI (buttons, panels)
// author: Alijan
// NOTE: not a standalone header - included only from main.cpp, in a fixed order.
#pragma once

// ----------------------------------------------------------------------------
//  Immediate-mode UI
// ----------------------------------------------------------------------------
static bool hovered(float x, float y, float w, float h) { return in.mx >= x && in.mx <= x + w && in.my >= y && in.my <= y + h; }
static bool button(const string& label, float x, float y, float w, float h, bool selected = false, float px = 3) {
    bool hov = hovered(x, y, w, h);
    bool press = hov && in.downL;
    float oy = press ? 2.f : 0.f;
    setc(Col{0, 0, 0, 0.38f}); rectTL(x + 3, y + 5, w, h);
    Col top = selected ? hex(0x4d8dff) : (hov ? hex(0x4a5870) : hex(0x38414f));
    Col bot = selected ? hex(0x2358c9) : (hov ? hex(0x303b4b) : hex(0x232a34));
    gradV(x, y + oy, w, h, top, bot);
    setc(selected ? hex(0x9cc4ff) : (hov ? hex(0x8aa0c0) : hex(0x566275))); rectTL(x, y + oy, w, 2);
    setc(Col{0, 0, 0, 0.35f}); rectTL(x, y + oy + h - 2, w, 2);
    if (hov && !selected) { setc(hex(0xffc83d)); rectTL(x, y + oy, 4, h); }
    textS(label, x + w / 2, y + oy + (h - 7 * px) / 2, px, hex(0xffffff), 1);
    if (hov && in.clickL) { in.clickL = false; sfx::play(sfx::UI_CLICK); return true; }
    return false;
}
static void panel(float x, float y, float w, float h, float a = 0.8f) {
    setc(Col{0, 0, 0, 0.35f}); rectTL(x + 4, y + 5, w, h);
    gradV(x, y, w, h, hex(0x1d232c, a), hex(0x0e1116, a));
    setc(hex(0x3a4452)); rectTL(x, y, w, 2); rectTL(x, y + h - 2, w, 2); rectTL(x, y, 2, h); rectTL(x + w - 2, y, 2, h);
    setc(hex(0xffc83d)); rectTL(x, y, 26, 3);
}
static float* activeSlider = nullptr;
static bool slider(const string& label, float x, float y, float w, float& v) {
    float bx = x + 150, bw = w - 220, by = y + 6, bh = 12;
    text(label, x, y + 6, 2, hex(0xd6dce6));
    bool hov = hovered(bx - 6, y, bw + 12, 26);
    if (hov && in.clickL) { activeSlider = &v; in.clickL = false; }
    bool changed = false;
    if (activeSlider == &v) {
        if (in.downL) { float nv = clampf((float)(in.mx - bx) / bw, 0, 1); changed = nv != v; v = nv; }
        else { activeSlider = nullptr; sfx::play(sfx::UI_CLICK); }
    }
    setc(hex(0x2a313b)); rrect(bx + bw / 2, by + bh / 2, bw, bh, 5);
    gradH(bx, by + 2, bw * v, bh - 4, hex(0x2358c9), hex(0x4d8dff));
    setc(hov || activeSlider == &v ? hex(0xffe08a) : hex(0xffc83d)); circle(bx + bw * v, by + bh / 2, 10, 18);
    setc(hex(0x1d232c)); circle(bx + bw * v, by + bh / 2, 4, 12);
    text(fmt("%d%%", (int)std::round(v * 100)), x + w - 4, y + 6, 2, hex(0xffffff), 2);
    return changed;
}
static void drawToast(float dt) {
    if (toastT <= 0) return;
    toastT -= dt;
    float w = textW(toast, 3) + 40;
    panel(fbw / 2 - w / 2, fbh - 200, w, 44, 0.92f);
    text(toast, fbw / 2.f, fbh - 200 + 14, 3, hex(0xffe08a), 1);
}
static void vignette() {
    float s = 160.f, a = 0.5f;
    gradV(0, 0, (float)fbw, s, Col{0, 0, 0, a}, Col{0, 0, 0, 0});
    gradV(0, fbh - s, (float)fbw, s, Col{0, 0, 0, 0}, Col{0, 0, 0, a});
    gradH(0, 0, s, (float)fbh, Col{0, 0, 0, a}, Col{0, 0, 0, 0});
    gradH(fbw - s, 0, s, (float)fbh, Col{0, 0, 0, 0}, Col{0, 0, 0, a});
}
