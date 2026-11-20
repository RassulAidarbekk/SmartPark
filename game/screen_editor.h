// SmartPark - screen_editor.h
// level editor screen
// author: Alijan
// NOTE: not a standalone header - included only from main.cpp, in a fixed order.
#pragma once

// ---- editor screen ----
static V2 snapv(V2 v) { if (!edSnap) return v; return {std::round(v.x * 2) / 2, std::round(v.y * 2) / 2}; }
static bool hasType(int t) { for (auto& i : ed.items) if (i.type == t) return true; return false; }
static bool showSafetyReport = false;
// tool palette: -1 entries are category headers
struct ToolEntry { int type; const char* header; };
static const ToolEntry TOOLS[] = {
    {-1, "BASICS"}, {T_START, 0}, {T_SPOT, 0}, {T_PCAR, 0}, {T_WALL, 0}, {T_BARRIER, 0}, {T_CURB, 0}, {T_CONE, 0}, {T_TREE, 0},
    {-1, "CURVES"}, {T_ARCCURB, 0}, {T_ARCLINE, 0}, {T_ISLAND, 0}, {T_CONEARC, 0},
    {-1, "MARKINGS"}, {T_CTRLLINE, 0}, {T_STOPLINE, 0}, {T_CROSSING, 0}, {T_LINE, 0}, {T_BAY, 0},
    {-1, "SURFACES"}, {T_GRASS, 0}, {T_HILL, 0}, {T_MUD, 0}, {T_ICE, 0},
    {-1, "EXAM TASKS"}, {T_TASK, 0},
    {-1, "URBAN LAB"}, {T_ACCESSIBLE, 0}, {T_ENTRANCE, 0},
};
static Item ghostItem(V2 mw) {
    Item g{edTool, mw.x, mw.y, DW[edTool], DH[edTool], edRot, edColor};
    if (isArcType(edTool)) { g.w = edArcR; g.h = edArcSweep; }
    if (edTool == T_ISLAND) { g.w = g.h = edArcR * 2; }
    if (edTool == T_HILL) g.color = ((int)std::round(edRot / 90.f)) & 3;
    if (edTool == T_ARCLINE || edTool == T_LINE) g.color = edColor & 1;
    if (edTool == T_TASK) { int k = edColor % TK_COUNT; g.color = k; g.w = TKW[k]; g.h = TKH[k]; }
    return g;
}
static void screenEdit(float dt) {
    glClearColor(0.07f, 0.08f, 0.1f, 1); glClear(GL_COLOR_BUFFER_BIT);
    bool typing = edNaming;
    if (typing) {
        if (in.kp[GLFW_KEY_BACKSPACE] && !ed.name.empty()) ed.name.pop_back();
        if (in.kp[GLFW_KEY_ENTER] || in.kp[GLFW_KEY_KP_ENTER] || in.kp[GLFW_KEY_ESCAPE]) endNaming();
    }
    float listTop = ED_TOP, listBot = fbh - 292.f;
    bool overList = in.mx < ED_PANEL && in.my > listTop && in.my < listBot;
    bool uiHot = in.mx < ED_PANEL || in.my < ED_TOP;
    // camera control
    float pan = 22.f / edZoom * 20.f * dt;
    if (!typing) {
        if (key(GLFW_KEY_LEFT) || key(GLFW_KEY_A)) edCam.x -= pan;
        if (key(GLFW_KEY_RIGHT) || key(GLFW_KEY_D)) edCam.x += pan;
        if (key(GLFW_KEY_UP) || key(GLFW_KEY_W)) edCam.y -= pan;
        if (key(GLFW_KEY_DOWN) || key(GLFW_KEY_S)) edCam.y += pan;
    }
    if (in.downM) { edCam.x -= (float)in.dmx / edZoom; edCam.y -= (float)in.dmy / edZoom; }
    auto zoomAt = [&](float factor, bool atMouse) {
        V2 before = atMouse ? toWorld(edCam, edZoom) : edCam;
        edZoom = clampf(edZoom * factor, edMinZoom(), 120);
        V2 after = atMouse ? toWorld(edCam, edZoom) : edCam;
        edCam = edCam + (before - after);
    };
    if (in.scroll != 0 && !uiHot) zoomAt(std::pow(1.12f, in.scroll), true);
    if (in.scroll != 0 && overList) edListScroll -= in.scroll * 40.f;
    bool ctrl = key(GLFW_KEY_LEFT_CONTROL) || key(GLFW_KEY_RIGHT_CONTROL);
    bool shift = key(GLFW_KEY_LEFT_SHIFT) || key(GLFW_KEY_RIGHT_SHIFT);
    if (!typing) {
        if (in.kp[GLFW_KEY_Q]) edRot -= 15;
        if (in.kp[GLFW_KEY_E]) edRot += 15;
        if (in.kp[GLFW_KEY_R]) edRot += 90;
        if (in.kp[GLFW_KEY_C]) edColor = (edColor + 1) % (edTool == T_TASK ? TK_COUNT : 8);
        if (in.kp[GLFW_KEY_G]) edSnap = !edSnap;
        if (in.kp[GLFW_KEY_Z] && !ed.items.empty()) ed.items.pop_back();
        if (in.kp[GLFW_KEY_LEFT_BRACKET]) edArcR = std::max(1.5f, edArcR - (shift ? 5.f : 0.5f));
        if (in.kp[GLFW_KEY_RIGHT_BRACKET]) edArcR = std::min(60.f, edArcR + (shift ? 5.f : 0.5f));
        if (in.kp[GLFW_KEY_COMMA]) edArcSweep = std::max(15.f, edArcSweep - 15.f);
        if (in.kp[GLFW_KEY_PERIOD]) edArcSweep = std::min(360.f, edArcSweep + 15.f);
        if (in.kp[GLFW_KEY_EQUAL] || in.kp[GLFW_KEY_KP_ADD]) zoomAt(1.25f, false);
        if (in.kp[GLFW_KEY_MINUS] || in.kp[GLFW_KEY_KP_SUBTRACT]) zoomAt(0.8f, false);
        if (in.kp[GLFW_KEY_F]) fitEditorCam();
        if (ctrl && in.kp[GLFW_KEY_S]) saveEditor();
    }
    edRot = std::fmod(edRot + 720.f, 360.f);
    auto testPlay = [&]() {
        if (!hasType(T_START) || !hasType(T_SPOT)) { showToast("NEED A START POINT AND A TARGET SPOT"); return; }
        endNaming();
        startPlay(ed, PM_TEST, 0, "");
    };
    if (!typing && in.kp[GLFW_KEY_P]) { testPlay(); if (screen == S_PLAY) return; }
    if (!typing && in.kp[GLFW_KEY_ESCAPE]) { saveEditor(); refreshCustoms(); screen = S_CUSTOM; return; }

    // world
    beginWorld(edCam, edZoom);
    drawLevel(ed, true, false, true);
    V2 mw = snapv(toWorld(edCam, edZoom));
    if (!uiHot) {
        if (edDrag) {
            float x1 = std::min(edDragStart.x, mw.x), x2 = std::max(edDragStart.x, mw.x), y1 = std::min(edDragStart.y, mw.y), y2 = std::max(edDragStart.y, mw.y);
            Item g{edTool, (x1 + x2) / 2, (y1 + y2) / 2, std::max(0.5f, x2 - x1), std::max(0.5f, y2 - y1), 0, edTool == T_HILL ? (((int)std::round(edRot / 90.f)) & 3) : 0};
            gAlpha = 0.6f; drawItem(g, true); gAlpha = 1;
            setc(hex(0xffffff)); text(fmt("%.1f X %.1f", x2 - x1, y2 - y1), x2 + 0.4f, y2 + 0.2f, 0.14f, hex(0xffffff));
        } else {
            Item g = ghostItem(mw);
            gAlpha = 0.6f;
            if (edTool == T_CONEARC) {
                vector<V2> ps; coneArcPositions(mw.x, mw.y, edArcR, edRot, edArcSweep, 2.f, ps);
                for (auto& p : ps) { Item c{T_CONE, p.x, p.y, 0.8f, 0.8f, 0, 0}; drawItem(c, true); }
            } else if (isDragTool(edTool)) { g.rot = 0; g.w = 1; g.h = 1; drawItem(g, true); }
            else drawItem(g, true);
            gAlpha = 1;
            if (isArcType(edTool) || edTool == T_ISLAND) { setc(hex(0xffc83d, 0.9f)); circle(mw.x, mw.y, 0.18f, 10); }
        }
    }
    glPopMatrix();

    // world interaction
    if (!uiHot) {
        if (in.clickL) {
            if (edNaming) endNaming();
            if (isDragTool(edTool)) { edDrag = true; edDragStart = mw; }
            else if (edTool == T_CONEARC) {
                vector<V2> ps; coneArcPositions(mw.x, mw.y, edArcR, edRot, edArcSweep, 2.f, ps);
                for (auto& p : ps) ed.items.push_back({T_CONE, p.x, p.y, 0.8f, 0.8f, 0, 0});
            } else {
                if (edTool == T_START || edTool == T_SPOT) ed.items.erase(std::remove_if(ed.items.begin(), ed.items.end(), [&](const Item& i) { return i.type == edTool; }), ed.items.end());
                ed.items.push_back(ghostItem(mw));
            }
        }
        if (in.clickR) {
            V2 p = toWorld(edCam, edZoom);
            for (int i = (int)ed.items.size() - 1; i >= 0; i--) if (itemHit(ed.items[i], p)) { ed.items.erase(ed.items.begin() + i); break; }
        }
    }
    if (edDrag && in.relL) {
        edDrag = false;
        float x1 = std::min(edDragStart.x, mw.x), x2 = std::max(edDragStart.x, mw.x), y1 = std::min(edDragStart.y, mw.y), y2 = std::max(edDragStart.y, mw.y);
        if (x2 - x1 < 0.5f) x2 = x1 + 0.5f;
        if (y2 - y1 < 0.5f) y2 = y1 + 0.5f;
        ed.items.push_back({edTool, (x1 + x2) / 2, (y1 + y2) / 2, x2 - x1, y2 - y1, 0, edTool == T_HILL ? (((int)std::round(edRot / 90.f)) & 3) : 0});
    }

    // ---- UI overlay: left panel with scrollable tool list
    panel(0, 0, ED_PANEL, (float)fbh, 0.95f);
    panel(ED_PANEL, 0, (float)fbw - ED_PANEL, ED_TOP, 0.95f);
    text("ITEMS", ED_PANEL / 2, 14, 3, hex(0xffc83d), 1);
    {
        int nT = (int)(sizeof TOOLS / sizeof TOOLS[0]);
        float contentH = 0; for (int i = 0; i < nT; i++) contentH += TOOLS[i].type < 0 ? 22.f : 30.f;
        float viewH = listBot - listTop - 6;
        edListScroll = clampf(edListScroll, 0, std::max(0.f, contentH - viewH));
        glEnable(GL_SCISSOR_TEST);
        glScissor(0, (int)(fbh - listBot), (int)ED_PANEL, (int)(listBot - listTop - 4));
        float y = listTop + 4 - edListScroll;
        for (int i = 0; i < nT; i++) {
            if (TOOLS[i].type < 0) { text(TOOLS[i].header, 12, y + 7, 1.6f, hex(0x8a94a6)); y += 22; continue; }
            int t = TOOLS[i].type;
            bool vis = y + 26 > listTop && y < listBot - 4;
            if (vis) {
                bool clickable = in.my > listTop && in.my < listBot - 4;
                bool saved = in.clickL; if (!clickable) in.clickL = false;
                if (button(TNAME[t], 10, y, ED_PANEL - 22, 26, edTool == t, 2)) edTool = t;
                if (!clickable) in.clickL = saved;
            }
            y += 30;
        }
        glDisable(GL_SCISSOR_TEST);
        if (contentH > viewH) {   // scrollbar
            float sh = viewH * viewH / contentH, sy = listTop + 2 + (viewH - sh) * (edListScroll / (contentH - viewH));
            setc(hex(0xffffff, 0.15f)); rectTL(ED_PANEL - 8, listTop + 2, 4, viewH);
            setc(hex(0xffc83d, 0.7f)); rectTL(ED_PANEL - 8, sy, 4, sh);
        }
        setc(hex(0x3a4452)); rectTL(6, listBot - 2, ED_PANEL - 12, 2);
    }
    // ---- fixed bottom section: map size, presets, par, name, curve params
    float uy = listBot + 6;
    float step = shift ? 20.f : 4.f;
    text("SIZE", 10, uy + 9, 2, hex(0x8a94a6));
    if (button("W-", 58, uy, 34, 26, false, 2)) ed.W = std::max(MAP_MIN_W, ed.W - step);
    if (button("W+", 96, uy, 34, 26, false, 2)) ed.W = std::min(MAP_MAX, ed.W + step);
    if (button("H-", 134, uy, 32, 26, false, 2)) ed.H = std::max(MAP_MIN_H, ed.H - step);
    if (button("H+", 170, uy, 32, 26, false, 2)) ed.H = std::min(MAP_MAX, ed.H + step);
    {
        struct Pre { const char* n; float w, h; } pre[] = {{"S", 44, 26}, {"M", 80, 48}, {"L", 140, 84}, {"XL", 220, 130}, {"XXL", 300, 180}};
        float px = 10;
        for (auto& p : pre) {
            float bw = p.n[1] == 0 ? 30.f : (p.n[2] == 0 ? 36.f : 44.f);
            if (button(p.n, px, uy + 32, bw, 24, ed.W == p.w && ed.H == p.h, 2)) { ed.W = p.w; ed.H = p.h; fitEditorCam(); }
            px += bw + 4;
        }
    }
    text(fmt("MAP %.0f X %.0f  (SHIFT=20)", ed.W, ed.H), ED_PANEL / 2, uy + 64, 1.5f, hex(0xffffff), 1);
    text("PAR", 10, uy + 87, 2, hex(0x8a94a6));
    if (button("-", 58, uy + 80, 34, 24, false, 2)) ed.par = std::max(5.f, ed.par - 5);
    if (button("+", 96, uy + 80, 34, 24, false, 2)) ed.par = std::min(600.f, ed.par + 5);
    text(fmt("%.0fS", ed.par), 140, uy + 87, 2, hex(0xffffff));
    {
        string label = ed.name + ((edNaming && std::fmod(gTime, 1.f) < 0.5f) ? "_" : "");
        text("LEVEL NAME", 10, uy + 114, 1.6f, hex(0x8a94a6));
        if (button(label, 10, uy + 126, ED_PANEL - 20, 26, edNaming, 2)) {
            edNaming = !edNaming;
            if (edNaming) textTarget = &ed.name; else endNaming();
        }
    }
    bool curveTool = isArcType(edTool) || edTool == T_ISLAND;
    if (curveTool) {
        text("RADIUS", 10, uy + 167, 2, hex(0x8a94a6));
        if (button("-", 90, uy + 160, 30, 24, false, 2)) edArcR = std::max(1.5f, edArcR - (shift ? 5.f : 0.5f));
        if (button("+", 124, uy + 160, 30, 24, false, 2)) edArcR = std::min(60.f, edArcR + (shift ? 5.f : 0.5f));
        text(fmt("%.1f", edArcR), 160, uy + 167, 2, hex(0xffffff));
        if (edTool != T_ISLAND) {
            text("SWEEP", 10, uy + 197, 2, hex(0x8a94a6));
            if (button("-", 90, uy + 190, 30, 24, false, 2)) edArcSweep = std::max(15.f, edArcSweep - 15.f);
            if (button("+", 124, uy + 190, 30, 24, false, 2)) edArcSweep = std::min(360.f, edArcSweep + 15.f);
            text(fmt("%.0f", edArcSweep), 160, uy + 197, 2, hex(0xffffff));
        }
        text("[ ] RADIUS  , . SWEEP  Q/E START", ED_PANEL / 2, uy + 224, 1.4f, hex(0x8a94a6), 1);
    } else if (edTool == T_HILL) {
        static const char* dn[4] = {"RIGHT", "DOWN", "LEFT", "UP"};
        text(fmt("UPHILL: %s (R TO TURN)", dn[((int)std::round(edRot / 90.f)) & 3]), ED_PANEL / 2, uy + 167, 1.6f, hex(0xffe08a), 1);
    } else if (edTool == T_TASK) {
        int k = edColor % TK_COUNT, nt = 0;
        for (auto& i : ed.items) if (i.type == T_TASK) nt++;
        text(fmt("KIND: %s", TKNAME[k]), ED_PANEL / 2, uy + 162, 1.6f, hex(0xffe08a), 1);
        text("C = CHANGE KIND", ED_PANEL / 2, uy + 180, 1.4f, hex(0x8a94a6), 1);
        text("ARROW = CAR NOSE", ED_PANEL / 2, uy + 196, 1.4f, hex(0x8a94a6), 1);
        text(fmt("ORDER = AS PLACED (%d)", nt), ED_PANEL / 2, uy + 212, 1.4f, hex(0x8a94a6), 1);
        text("SPOT LOCKED TILL DONE", ED_PANEL / 2, uy + 228, 1.4f, hex(0xffd59d), 1);
    } else if (edTool == T_LINE || edTool == T_ARCLINE) {
        text(edColor & 1 ? "DASHED (C TO TOGGLE)" : "SOLID (C TO TOGGLE)", ED_PANEL / 2, uy + 167, 1.6f, hex(0xffe08a), 1);
    }
    if (edTool == T_ARCLINE)
        text(edColor & 1 ? "DECORATION ONLY" : "SOLID = CONTROL LINE", ED_PANEL / 2, uy + 246, 1.4f, hex(0xffd59d), 1);
    text(fmt("ROT %.0f  SNAP %s  ZOOM %.0f", edRot, edSnap ? "ON" : "OFF", edZoom), ED_PANEL / 2, fbh - 16.f, 1.5f, hex(0xaab4c8), 1);

    // ---- top bar
    float bx = ED_PANEL + 10;
    if (button("TEST (P)", bx, 8, 104, 34, false, 2)) testPlay();
    if (screen == S_PLAY) return;
    bx += 112;
    if (button("SAVE", bx, 8, 76, 34, false, 2)) saveEditor();
    bx += 84;
    if (button("UNDO", bx, 8, 76, 34, false, 2) && !ed.items.empty()) ed.items.pop_back();
    bx += 84;
    if (button("CLEAR", bx, 8, 82, 34, false, 2)) { ed.items.clear(); showToast("LEVEL CLEARED"); }
    bx += 90;
    if (button("EXIT", bx, 8, 70, 34, false, 2)) { endNaming(); saveEditor(); refreshCustoms(); screen = S_CUSTOM; return; }
    bx += 78;
    if (button("ANALYZE", bx, 8, 104, 34, showSafetyReport, 2)) showSafetyReport = !showSafetyReport;
    bx += 120;
    text("ZOOM", bx, 21, 2, hex(0x8a94a6)); bx += 56;
    if (button("-", bx, 8, 38, 34)) zoomAt(0.8f, false);
    if (button("+", bx + 44, 8, 38, 34)) zoomAt(1.25f, false);
    if (button("FIT", bx + 88, 8, 56, 34, false, 2)) fitEditorCam();
    if (showSafetyReport) {
        auto result = analyzeLayout(ed);
        float x = std::max(ED_PANEL + 10, (float)fbw - 350.f);
        panel(x, 60, 335, 80.f + 27.f * std::min((int)result.findings.size(), 7), 0.97f);
        text("URBAN SAFETY LAB", x + 12, 72, 2.5f, hex(0xffc83d));
        text(fmt("LAYOUT SCORE: %d/100", result.score), x + 12, 98, 2.5f, result.score >= 75 ? hex(0x65d6a0) : hex(0xff927e));
        if (result.findings.empty()) text("NO BASIC ISSUES FOUND", x + 12, 128, 2, hex(0x65d6a0));
        for (int i = 0; i < (int)result.findings.size() && i < 7; i++)
            text(result.findings[i].message, x + 12, 128 + i * 27, 1.8f, hex(0xf0b3a5));
    }
    text("LMB PLACE  RMB DELETE  Q/E ROT15  R ROT90  C COLOR  G SNAP  WASD/MMB PAN  WHEEL/+/- ZOOM  F FIT  CTRL+S SAVE",
         ED_PANEL + 10, fbh - 18.f, 1.6f, hex(0x8a94a6));
    drawToast(dt);
}
