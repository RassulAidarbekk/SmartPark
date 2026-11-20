// SmartPark - screens_menu.h
// menu, career and custom-level screens, editor state
// author: Alijan
// NOTE: not a standalone header - included only from main.cpp, in a fixed order.
#pragma once

// ----------------------------------------------------------------------------
//  Screens
// ----------------------------------------------------------------------------
static void drawBackdrop() {
    glClearColor(0.05f, 0.06f, 0.1f, 1); glClear(GL_COLOR_BUFFER_BIT);
    float bandH = 150.f, bandY = fbh - bandH;
    gradV(0, 0, (float)fbw, bandY, hex(0x0b1030), hex(0x4a3a7a));
    gradV(0, bandY * 0.55f, (float)fbw, bandY * 0.45f, hex(0x4a3a7a, 0.0f), hex(0xf08a5d, 0.55f));
    for (int i = 0; i < 40; i++) {
        unsigned h = hsh(i * 31u + 7u);
        float x = (h & 1023) / 1023.f * fbw, y = ((h >> 10) & 1023) / 1023.f * bandY * 0.5f;
        float tw = 0.4f + 0.6f * std::sin(gTime * (1.f + (h % 7) * 0.3f) + i);
        setc(hex(0xffffff, 0.25f + 0.4f * tw)); rect(x, y, 2, 2);
    }
    for (int layer = 0; layer < 2; layer++) {
        float sp = 6.f + layer * 14.f, bw = 70.f + layer * 20.f;
        float off = gTime * sp; int base = (int)(off / bw); float fr = std::fmod(off, bw);
        for (int k = -1; k < fbw / (int)bw + 3; k++) {
            int idx = base + k;
            unsigned h = hsh(idx * 7u + layer * 131u);
            float bh = 60.f + (h % 120) + layer * 20.f, x = k * bw - fr;
            setc(layer == 0 ? hex(0x1a1a3a) : hex(0x10122a));
            rectTL(x, bandY - bh, bw - 4, bh);
            for (int wy = 0; wy < (int)(bh / 16) - 1; wy++) for (int wx = 0; wx < 3; wx++) {
                unsigned wh = hsh(h + wy * 13u + wx * 7u);
                if (wh % 3 == 0) { setc(hex(layer == 0 ? 0xffd27a : 0xc9a85c, 0.55f)); rectTL(x + 8 + wx * 18.f * (bw / 70.f), bandY - bh + 10 + wy * 16.f, 7, 8); }
            }
        }
    }
    gradV(0, bandY, (float)fbw, bandH, hex(0x2f353f), hex(0x1f242b));
    setc(hex(0x4a5260)); rectTL(0, bandY, (float)fbw, 6);
    float z = 26.f;
    setc(hex(0xf1f1f1, 0.8f));
    for (float x = -((int)(gTime * 80) % 120); x < fbw; x += 120) rectTL(x, bandY + 72, 60, 5);
    for (int i = 0; i < 4; i++) {
        float speed = 90.f + i * 30.f; float span = fbw + 400.f;
        float x = std::fmod(gTime * speed + i * 330.f, span) - 200.f;
        float y = bandY + 38.f + (i % 2) * 74.f;
        glPushMatrix(); glTranslatef(x, y, 0); glScalef(z, z, 1);
        drawCar(hex(CARCOL[(i * 3 + 1) % 8]), 0, false, true);
        glPopMatrix();
    }
}
static void newEditor();
static void openSettings(Screen back) { settingsBack = back; rebindAct = -1; screen = S_SETTINGS; }
static void screenMenu() {
    drawBackdrop();
    float cx = fbw / 2.f;
    const string title = "SMARTPARK";
    float px = 14, tw = textW(title, px), x = cx - tw / 2;
    for (size_t i = 0; i < title.size(); i++) {
        float yy = 56 + std::sin(gTime * 2.2f + i * 0.55f) * 6.f;
        float k = 0.5f + 0.5f * std::sin(gTime * 1.5f + i * 0.4f);
        Col c = {1.f, 0.78f + 0.2f * k, 0.24f + 0.5f * k, 1.f};
        textS(string(1, title[i]), x + i * 6 * px, yy, px, c);
    }
    textS("PARK & BUILD", cx, 175, 5, hex(0xffffff), 1);
    float bw = 360, bx = cx - bw / 2, y = 222;
    if (button("CAREER", bx, y, bw, 50, false, 4)) screen = S_CAREER;
    if (button("CUSTOM LEVELS", bx, y + 62, bw, 50, false, 4)) screen = S_CUSTOM;
    if (button("LEVEL EDITOR", bx, y + 124, bw, 50, false, 4)) newEditor();
    if (button("SETTINGS", bx, y + 186, bw, 50, false, 4)) openSettings(S_MENU);
    if (button("QUIT", bx, y + 248, bw, 50, false, 4)) glfwSetWindowShouldClose(win, 1);
   /* text(cfg.exam ? "EXAM RULES: ON (KZ AUTODROM)" : "EXAM RULES: OFF (ARCADE)", cx, y + 312, 2, cfg.exam ? hex(0x9dffb8) : hex(0xffd59d), 1);*/
    text("TEAM GRAPHICAL GODDESS - SE-2411", cx, 14, 2, hex(0xaab4c8), 1);
    text("ALIZHAN  RASSUL  SAMANDAR  ZHALGAS", cx, 30, 2, hex(0x8a94a6), 1);
}

static void drawStars(float cx, float cy, float R, int n) {
    for (int i = 0; i < 3; i++) star(cx + (i - 1) * R * 2.3f, cy, R, i < n);
}
static void drawStarsAnim(float cx, float cy, float R, int n, float t) {
    for (int i = 0; i < 3; i++) {
        float x = cx + (i - 1) * R * 2.3f;
        if (i >= n) { star(x, cy, R, false); continue; }
        float s = clampf((t - 0.25f - i * 0.3f) / 0.3f, 0, 1);
        if (s <= 0) { star(x, cy, R, false); continue; }
        star(x, cy, R * s * (1.f + 0.35f * std::sin(s * PI)), true);
    }
}
static void screenCareer() {
    drawBackdrop();
    textS("CAREER", fbw / 2.f, 30, 8, hex(0xffffff), 1);
    if (in.kp[GLFW_KEY_F1]) { cheatUnlock = true; showToast("ALL LEVELS UNLOCKED (F1)"); }
    int n = (int)CAREER.size(), cols = 3;
    float cw = 300, ch = 124, gap = 16, x0 = fbw / 2.f - (cols * cw + (cols - 1) * gap) / 2, y0 = 96;
    for (int i = 0; i < n; i++) {
        float x = x0 + (i % cols) * (cw + gap), y = y0 + (i / cols) * (ch + gap);
        bool unl = careerUnlocked(i);
        string k = fmt("career_%d", i);
        bool hov = unl && hovered(x, y, cw, ch);
        if (hov) { setc(hex(0xffc83d, 0.18f)); rectTL(x - 6, y - 6, cw + 12, ch + 12); }
        panel(x, y, cw, ch, hov ? 0.98f : 0.88f);
        text(fmt("%d", i + 1), x + 16, y + 14, 8, unl ? hex(0xffc83d) : hex(0x59616f));
        text(CAREER[i].name, x + 16 + 60, y + 18, 2.5f, unl ? hex(0xffffff) : hex(0x59616f));
        text(fmt("PAR %.0fS  %.0fX%.0f", CAREER[i].par, CAREER[i].W, CAREER[i].H), x + 76, y + 42, 2, hex(0x8a94a6));
        if (unl) {
            int s = prog.count(k) ? prog[k].first : 0;
            drawStars(x + cw / 2, y + 88, 13, s);
            if (prog.count(k)) text(fmt("BEST %.1fS", prog[k].second), x + cw - 12, y + ch - 18, 2, hex(0x8a94a6), 2);
            if (hov && in.clickL) { in.clickL = false; sfx::play(sfx::UI_CLICK); startPlay(CAREER[i], PM_CAREER, i, k); return; }
        } else {
            setc(hex(0x59616f)); rrect(x + cw / 2, y + 92, 26, 20, 4);
            setc(hex(0x59616f)); rectTL(x + cw / 2 - 9, y + 68, 4, 14); rectTL(x + cw / 2 + 5, y + 68, 4, 14); rectTL(x + cw / 2 - 9, y + 66, 18, 4);
            setc(hex(0x1d232c)); circle(x + cw / 2, y + 92, 3.5f, 10);
        }
    }
    int rows = (n + cols - 1) / cols;
    text("F1 = UNLOCK ALL", fbw / 2.f, y0 + rows * (ch + gap) + 4, 2, hex(0xaab4c8), 1);
    if (button("BACK", 20, 20, 120, 40) || in.kp[GLFW_KEY_ESCAPE]) screen = S_MENU;
}

// ---- custom level list ----
struct CEntry { string path, name, key; };
static vector<CEntry> customs;
static int customScroll = 0, delConfirm = -1;
static void refreshCustoms() {
    customs.clear(); delConfirm = -1;
    std::error_code ec;
    if (!fs::exists("levels", ec)) return;
    vector<string> paths;
    for (auto& e : fs::directory_iterator("levels", ec)) if (e.path().extension() == ".lvl") paths.push_back(e.path().string());
    std::sort(paths.begin(), paths.end());
    for (auto& p : paths) { Level L; if (loadLevel(p, L)) customs.push_back({p, L.name, "c_" + fs::path(p).stem().string()}); }
}

// ---- editor state ----
static Level ed; static string edPath;
static int edTool = T_WALL, edColor = 0;
static float edRot = 0, edZoom = 20, edArcR = 8, edArcSweep = 90, edListScroll = 0;
static bool edSnap = true, edDrag = false, edNaming = false;
static V2 edCam, edDragStart;
static void endNaming() {
    edNaming = false; textTarget = nullptr;
    while (!ed.name.empty() && ed.name.back() == ' ') ed.name.pop_back();
    if (ed.name.empty()) ed.name = "UNTITLED";
}
static const float ED_PANEL = 210.f, ED_TOP = 50.f;
static float edFitZoom() { return std::min((fbw - ED_PANEL - 30.f) / ed.W, (fbh - ED_TOP - 40.f) / ed.H); }
static float edMinZoom() { return std::min(8.f, std::max(1.f, edFitZoom() * 0.5f)); }
static void fitEditorCam() {
    edZoom = edFitZoom() * 0.95f;
    edCam = {ed.W / 2 - (ED_PANEL / 2) / edZoom, ed.H / 2 - (ED_TOP / 2 - 10) / edZoom};
}
static void newEditor() {
    ed = Level(); edPath.clear();
    startp(ed, 6, ed.H / 2, 0); spot(ed, ed.W - 8, ed.H / 2, 0);
    edTool = T_WALL; edRot = 0; edDrag = false; edNaming = false; textTarget = nullptr;
    fitEditorCam(); screen = S_EDIT;
}
static void openEditor(const string& path) {
    if (!loadLevel(path, ed)) { showToast("COULD NOT LOAD LEVEL"); return; }
    edPath = path; edDrag = false; edNaming = false; textTarget = nullptr; fitEditorCam(); screen = S_EDIT;
}
static bool saveEditor() {
    std::error_code ec; fs::create_directories("levels", ec);
    if (edPath.empty()) {
        for (int i = 1; i < 1000; i++) {
            string p = fmt("levels/custom_%d.lvl", i);
            if (!fs::exists(p)) { edPath = p; if (ed.name == "UNTITLED") ed.name = fmt("CUSTOM %d", i); break; }
        }
    }
    bool ok = saveLevel(edPath, ed);
    showToast(ok ? "SAVED " + edPath : "SAVE FAILED");
    return ok;
}

static void screenCustom() {
    drawBackdrop();
    textS("CUSTOM LEVELS", fbw / 2.f, 30, 8, hex(0xffffff), 1);
    if (button("BACK", 20, 20, 120, 40) || in.kp[GLFW_KEY_ESCAPE]) { screen = S_MENU; return; }
    if (button("NEW LEVEL", fbw - 220.f, 20, 200, 40)) { newEditor(); return; }
    if (customs.empty()) text("NO LEVELS YET - MAKE ONE IN THE EDITOR!", fbw / 2.f, 260, 3, hex(0xaab4c8), 1);
    int vis = std::max(3, (int)((fbh - 260) / 66)); float rowH = 66, y0 = 110, x0 = fbw / 2.f - 420;
    customScroll = (int)clampf((float)customScroll - in.scroll, 0, (float)std::max(0, (int)customs.size() - vis));
    for (int i = customScroll; i < (int)customs.size() && i < customScroll + vis; i++) {
        float y = y0 + (i - customScroll) * rowH;
        panel(x0, y, 840, rowH - 8, 0.88f);
        text(customs[i].name, x0 + 18, y + 20, 3, hex(0xffffff));
        int s = prog.count(customs[i].key) ? prog[customs[i].key].first : 0;
        drawStars(x0 + 400, y + 26, 11, s);
        if (button("PLAY", x0 + 520, y + 8, 100, 40)) {
            Level L; if (loadLevel(customs[i].path, L)) {
                bool ok = false; for (auto& it : L.items) if (it.type == T_SPOT) ok = true;
                if (ok) { startPlay(L, PM_CUSTOM, i, customs[i].key); return; } else showToast("LEVEL HAS NO TARGET SPOT");
            }
        }
        if (button("EDIT", x0 + 630, y + 8, 90, 40)) { openEditor(customs[i].path); return; }
        if (button(delConfirm == i ? "SURE?" : "DELETE", x0 + 730, y + 8, 100, 40, delConfirm == i)) {
            if (delConfirm == i) { std::error_code ec; fs::remove(customs[i].path, ec); refreshCustoms(); return; }
            delConfirm = i;
        }
    }
}
