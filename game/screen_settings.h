// SmartPark - screen_settings.h
// settings screen
// author: Alijan
// NOTE: not a standalone header - included only from main.cpp, in a fixed order.
#pragma once

// ---- settings screen ----
static float setScroll = 0;
static void screenSettings() {
    drawBackdrop();
    setc(hex(0x000000, 0.45f)); rectTL(0, 0, (float)fbw, (float)fbh);
    textS("SETTINGS", fbw / 2.f, 24, 7, hex(0xffffff), 1);
    auto goBack = [&]() { saveSettings(); rebindAct = -1; screen = settingsBack; };
    if (button("BACK", 20, 20, 120, 40)) { goBack(); return; }
    if (in.kp[GLFW_KEY_ESCAPE] && rebindAct < 0) { goBack(); return; }
    float cx = fbw / 2.f;
    float lw = 560, lx = cx - lw - 10, top = 86;
    float rowH = 26, listH = std::min(A_COUNT * rowH + 88, fbh - top - 20.f);
    panel(lx, top, lw, listH, 0.9f);
    text("CONTROLS", lx + 16, top + 14, 3, hex(0xffc83d));
    text("PRIMARY", lx + 290, top + 18, 1.8f, hex(0x8a94a6), 1);
    text("SECONDARY", lx + 460, top + 18, 1.8f, hex(0x8a94a6), 1);
    float rowsTop = top + 40, rowsH = listH - 80;
    float contentH = A_COUNT * rowH;
    if (hovered(lx, rowsTop, lw, rowsH)) setScroll -= in.scroll * 30.f;
    setScroll = clampf(setScroll, 0, std::max(0.f, contentH - rowsH));
    glEnable(GL_SCISSOR_TEST);
    glScissor((int)lx, (int)(fbh - (rowsTop + rowsH)), (int)lw, (int)rowsH);
    for (int a = 0; a < A_COUNT; a++) {
        float y = rowsTop + a * rowH - setScroll;
        if (y + rowH < rowsTop || y > rowsTop + rowsH) continue;
        if (a % 2) { setc(hex(0xffffff, 0.03f)); rectTL(lx + 6, y, lw - 12, rowH); }
        text(ACTNAME[a], lx + 18, y + 8, 2, hex(0xd6dce6));
        for (int s = 0; s < 2; s++) {
            bool waiting = rebindAct == a && rebindSlot == s;
            string lab = waiting ? (std::fmod(gTime, 0.8f) < 0.5f ? "PRESS KEY" : "") : keyName(cfg.bind[a][s]);
            bool inRows = in.my > rowsTop && in.my < rowsTop + rowsH;
            bool saved = in.clickL; if (!inRows) in.clickL = false;
            if (button(lab, lx + 220 + s * 170, y + 1, 150, rowH - 3, waiting, 2)) { rebindAct = a; rebindSlot = s; }
            if (!inRows) in.clickL = saved;
        }
    }
    glDisable(GL_SCISSOR_TEST);
    float fy = top + listH - 32;
    if (button("RESET KEYS", lx + 16, fy, 150, 26, false, 2)) { resetBindings(); saveSettings(); showToast("KEYS RESET TO DEFAULT"); }
    text(rebindAct >= 0 ? "PRESS A KEY - ESC CANCEL - DEL CLEAR" : "CLICK A SLOT TO REBIND", lx + 180, fy + 9, 1.8f,
         rebindAct >= 0 ? hex(0xffe08a) : hex(0x8a94a6));

    // right column: audio + gameplay
    float rx = cx + 10, rw = 520;
    panel(rx, top, rw, 190, 0.9f);
    text("AUDIO", rx + 16, top + 14, 3, hex(0xffc83d));
    bool ch = false;
    ch |= slider("MASTER", rx + 18, top + 46, rw - 36, cfg.master);
    ch |= slider("ENGINE", rx + 18, top + 80, rw - 36, cfg.engine);
    ch |= slider("EFFECTS", rx + 18, top + 114, rw - 36, cfg.fx);
    ch |= slider("INTERFACE", rx + 18, top + 148, rw - 36, cfg.ui);
    if (ch) applyAudio();
    float gy = top + 206;
    panel(rx, gy, rw, 262, 0.9f);
    text("GAMEPLAY", rx + 16, gy + 14, 3, hex(0xffc83d));
    auto row = [&](float y, const string& label, const string& val, const string& note) {
        text(label, rx + 18, y + 9, 2, hex(0xd6dce6));
        bool r = button(val, rx + 250, y, 250, 28, false, 2);
        if (!note.empty()) text(note, rx + 18, y + 31, 1.4f, hex(0x8a94a6));
        return r;
    };
    if (row(gy + 44, "EXAM RULES", cfg.exam ? "ON - KZ AUTODROM" : "OFF - ARCADE", cfg.exam ? "COLLISION = 100 PTS, 100+ PTS = FAIL" : "HITS ONLY COST A STAR")) { cfg.exam = !cfg.exam; saveSettings(); }
    if (row(gy + 92, "CAMERA", CAMNAME[cfg.cam], "")) { cfg.cam = (cfg.cam + 1) % CAM_COUNT; saveSettings(); }
    if (row(gy + 128, "WHEEL SELF-CENTER", CENTERNAME[cfg.center], cfg.center == 1 ? "RETURNS WHILE ROLLING, LIKE A REAL CAR" : "")) { cfg.center = (cfg.center + 1) % 3; saveSettings(); }
    if (row(gy + 176, "MINIMAP", cfg.minimap ? "ON" : "OFF", "")) { cfg.minimap = !cfg.minimap; saveSettings(); }
    if (row(gy + 212, "GUIDE ARROW", cfg.guide ? "ON" : "OFF", "")) { cfg.guide = !cfg.guide; saveSettings(); }
}
