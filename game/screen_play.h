// SmartPark - screen_play.h
// play screen drawing: HUD, minimap, task panel, nav arrow
// author: Rasul
// NOTE: not a standalone header - included only from main.cpp, in a fixed order.
#pragma once

// ---- HUD widgets ----
static void drawWheelHUD(float cx, float cy, float R, float wdeg) {
    setc(Col{0, 0, 0, 0.35f}); circle(cx + 4, cy + 6, R + 6, 40);
    glPushMatrix(); glTranslatef(cx, cy, 0); glRotatef(wdeg, 0, 0, 1);
    setc(hex(0x15181d)); ring(0, 0, R - 13, R, 48);
    setc(hex(0x2b313a)); ring(0, 0, R - 11, R - 3, 48);
    setc(hex(0x3a424e, 0.8f)); arcBand(0, 0, R - 9, R - 5, 200, 140, 20);
    // spokes
    setc(hex(0x23282f));
    glBegin(GL_QUADS);
    glVertex2f(-R + 10, -6); glVertex2f(-14, -8); glVertex2f(-14, 10); glVertex2f(-R + 10, 6);
    glVertex2f(R - 10, -6); glVertex2f(14, -8); glVertex2f(14, 10); glVertex2f(R - 10, 6);
    glVertex2f(-8, 14); glVertex2f(8, 14); glVertex2f(6, R - 10); glVertex2f(-6, R - 10);
    glEnd();
    setc(hex(0x2f3640)); circle(0, 2, 20, 28);
    setc(hex(0x15181d)); ring(0, 2, 17, 20, 28);
    setc(hex(0xffc83d)); rrect(0, 2, 14, 8, 3);
    setc(hex(0xffc83d)); rect(0, -R + 6, 7, 12);      // 12 o'clock marker
    glPopMatrix();
}
static void drawPedal(float x, float y, float w, float h, bool pressed, float amt, const string& label, Col accent) {
    float d = amt * 8.f;
    setc(Col{0, 0, 0, 0.4f}); rrect(x + w / 2 + 3, y + h / 2 + 6, w, h, 8);
    setc(hex(0x1a1e24)); rrect(x + w / 2, y + h / 2, w + 8, h + 8, 10);
    float sc = 1.f - amt * 0.06f;
    float pw = w * sc, ph = h * sc;
    gradV(x + (w - pw) / 2, y + (h - ph) / 2 + d, pw, ph, shade(hex(0x5b6573), pressed ? 0.8f : 1.f), hex(0x30363f));
    setc(hex(0x1f242b));
    for (int i = 1; i < 6; i++) rect(x + w / 2, y + (h - ph) / 2 + d + ph * i / 6.f, pw * 0.75f, 3);
    setc(accent); rect(x + w / 2, y + (h - ph) / 2 + d + 3, pw * 0.8f, 4);
    text(label, x + w / 2, y + h + 12, 2, pressed ? accent : hex(0xaab4c8), 1);
}
static void drawMinimap(float x, float y, float w, float h) {
    float s = std::min(w / cur.W, h / cur.H);
    float mw = cur.W * s, mh = cur.H * s;
    float ox = x + (w - mw) / 2, oy = y + (h - mh) / 2;
    panel(x - 8, y - 8, w + 16, h + 16, 0.85f);
    View saveV = gView; float saveZ = gZoom;
    gView = {0, 0, cur.W, cur.H}; gZoom = s; gLite = true;
    glPushMatrix(); glTranslatef(ox, oy, 0); glScalef(s, s, 1);
    gSpotLocked = wld.tasksLeft();
    drawLevel(cur, false, pInSpot, false, true, false);
    gSpotLocked = false;
    setc(hex(0xff7a1a)); for (auto& c : wld.cones) if (!c.knocked) rect(c.p.x, c.p.y, 0.9f, 0.9f);
    if (wld.tasksLeft()) drawTaskZone(wld.tasks[wld.taskIdx], -1, 1);
    float pulse = 0.5f + 0.5f * std::sin(gTime * 6.f);
    setc(hex(0x38bdf8, 0.35f + 0.3f * pulse)); circle(car.p.x, car.p.y, 3.5f / std::max(0.5f, s) * 1.5f, 16);
    glPushMatrix(); glTranslatef(car.p.x, car.p.y, 0); glRotatef(deg(car.a), 0, 0, 1);
    setc(hex(0x38bdf8)); rect(0, 0, 4.2f, 1.9f);
    setc(hex(0xffffff)); tri(2.6f, 0, 1.2f, -1.f, 1.2f, 1.f);
    glPopMatrix();
    glPopMatrix();
    gLite = false; gView = saveV; gZoom = saveZ;
    setc(hex(0xffffff, 0.35f));
    rectTL(ox, oy, mw, 1); rectTL(ox, oy + mh - 1, mw, 1); rectTL(ox, oy, 1, mh); rectTL(ox + mw - 1, oy, 1, mh);
}
static void drawSignalArrow(float x, float y, bool left, bool on) {
    Col c = on ? hex(0x3ce07a) : hex(0x2c343f);
    if (on) { setc(hex(0x3ce07a, 0.25f)); circle(x, y, 18, 18); }
    setc(c);
    float s = left ? -1.f : 1.f;
    tri(x + s * 14, y, x + s * 2, y - 11, x + s * 2, y + 11);
    rect(x - s * 5, y, 14, 9);
}

static string hintLine(int i) {
    auto k = [](int a) { return keyName(cfg.bind[a][0]); };
    if (i == 0) return bindLabel(A_GAS) + " GAS   " + bindLabel(A_BRAKE) + " BRAKE   " + k(A_LEFT) + "/" + k(A_RIGHT) + " STEER   " + k(A_HANDBRAKE) + " HANDBRAKE   MOUSE: WHEEL + PEDALS";
    return k(A_DRIVE) + " " + k(A_REVERSE) + " " + k(A_NEUTRAL) + " " + k(A_PARK) + " GEARS   " + k(A_SIGL) + "/" + k(A_SIGR) + " SIGNALS   " + k(A_BELT) + " BELT   " +
           k(A_CAMERA) + " CAMERA   " + k(A_ZOOMIN) + "/" + k(A_ZOOMOUT) + " ZOOM   ESC PAUSE";
}
static void screenPlay(float dt) {
    updatePlay(dt);
    if (!pPaused) updateFX(dt);
    if (pDone || pFailed) pDoneT += dt;
    if (!pPaused) pIntro += dt;
    glClearColor(0.07f, 0.08f, 0.1f, 1); glClear(GL_COLOR_BUFFER_BIT);

    // ---- camera
    float base = 22.f * (fbh / 720.f);
    float zfit = std::min(fbw / (cur.W + 6.f), fbh / (cur.H + 6.f));
    float zoom = base * pZoomMul, ang = 0;
    V2 target = car.p + car.v * 0.45f;
    if (cfg.cam == CAM_FIXED) { zoom = zfit; target = {cur.W / 2, cur.H / 2}; }
    gLift = cfg.cam == CAM_FIXED ? 0.f : 60.f;
    if (cfg.cam == CAM_FOLLOW) {
        // follow the car, but stop at the lot edges; centre an axis when the whole lot fits on it.
        // vertical range keeps the bottom HUD (~170 px) in mind.
        float hx = fbw / 2.f / zoom, cy0 = fbh / 2.f - gLift, hud = 170.f;
        float padR = wld.tasks.empty() ? 3.f : 3.f + 250.f / zoom;     // task levels: let the right edge scroll clear of the checklist
        float xmin = hx - 3, xmax = cur.W + padR - hx;
        float ymin = cy0 / zoom - 3, ymax = cur.H + 3 - (fbh - cy0 - hud) / zoom;
        target.x = xmin > xmax ? cur.W / 2 : clampf(target.x, xmin, xmax);
        target.y = ymin > ymax ? (ymin + ymax) / 2 : clampf(target.y, ymin, ymax);
    }
    pCam = pCam + (target - pCam) * std::min(1.f, dt * 5.f);
    if (cfg.cam == CAM_ROTATE) {
        float want = -PI / 2 - car.a, d = std::remainder(want - pCamA, 2 * PI);
        pCamA += d * std::min(1.f, dt * 4.f);
        ang = pCamA;
        pCam = pCam + ((car.p + V2{std::cos(car.a), std::sin(car.a)} * 3.f) - pCam) * std::min(1.f, dt * 6.f);
    } else pCamA = -PI / 2 - car.a;
    V2 sh = {(float)(std::sin(gTime * 90) * pShake * 14), (float)(std::cos(gTime * 77) * pShake * 14)};
    beginWorld(pCam, zoom, sh, ang);
    gSpotLocked = wld.tasksLeft();
    drawLevel(cur, false, pInSpot, false, false, false);
    // task zones: done = green tick, current = pulsing yellow, upcoming = faint dashed
    for (size_t i = 0; i < wld.tasks.size(); i++) {
        int st = i < wld.taskIdx ? 2 : (i == wld.taskIdx ? 1 : 0);
        drawTaskZone(wld.tasks[i], (int)i, st, st == 1 ? clampf(wld.taskHold / TASK_HOLD, 0, 1) : 0.f);
    }
    setc(hex(0x000000, 0.35f));
    for (auto& m : marks) { glPushMatrix(); glTranslatef(m.first.x, m.first.y, 0); glRotatef(deg(m.second), 0, 0, 1); rect(0, 0, 0.5f, 0.22f); glPopMatrix(); }
    for (auto& c : wld.cones) { glPushMatrix(); glTranslatef(c.p.x, c.p.y, 0); glRotatef(c.rot, 0, 0, 1); drawConeShape(0.8f, c.knocked); glPopMatrix(); }
    // navigation arrow orbiting the car: points at the current task, then at the final spot
    bool navTask = wld.tasksLeft();
    bool hasNav = (navTask || wld.hasSpot) && !pDone && !pFailed;
    V2 navT = navTask ? V2{wld.tasks[wld.taskIdx].x, wld.tasks[wld.taskIdx].y} : V2{wld.spot.x, wld.spot.y};
    if (hasNav && (cfg.guide || !wld.tasks.empty())) {
        V2 d = navT - car.p; float dl = vlen(d);
        if (dl > 6.f) {
            V2 dir = d * (1.f / dl); float pulse = 0.5f + 0.5f * std::sin(gTime * 5.f);
            Col ac = navTask ? hex(0xffc83d) : hex(0x38bdf8);
            glPushMatrix(); glTranslatef(car.p.x + dir.x * (4.6f + 0.3f * pulse), car.p.y + dir.y * (4.6f + 0.3f * pulse), 0);
            glRotatef(deg(std::atan2(dir.y, dir.x)), 0, 0, 1);
            setc(Col{0, 0, 0, 0.45f}); glPushMatrix(); glTranslatef(0.1f, 0.12f, 0); chevron(1.0f); glPopMatrix();
            setc(Col{ac.r, ac.g, ac.b, 0.70f + 0.30f * pulse}); chevron(0.95f);
            glPopMatrix();
        }
    }
    drawParts();
    glPushMatrix(); glTranslatef(car.p.x, car.p.y, 0); glRotatef(deg(car.a), 0, 0, 1);
    int blink = (sigPhase && (sigL || sigR)) ? (sigL ? 1 : 2) : 0;
    drawCar(hex(0x38bdf8), car.steer, car.braking || car.gear == G_P, true, blink, car.gear == G_R); glPopMatrix();
    for (auto& it : cur.items) if (it.type == T_TREE) drawItem(it, false);
    glPopMatrix();
    gLift = 0; gSpotLocked = false;

    vignette();

    // ---- HUD: top-left info + exam panel
    float kmh = std::fabs(car.vf) * 3.6f;
    panel(14, 14, 340, cfg.exam ? 118 : 92, 0.82f);
    textS(cur.name, 28, 26, 3, hex(0xffffff));
    float limit = cur.par * 2.f;
    text(cfg.exam ? fmt("TIME %.1f  PAR %.0f  LIMIT %.0f", pTime, cur.par, limit) : fmt("TIME %.1f / PAR %.0f", pTime, cur.par),
         28, 54, 2, pTime <= cur.par ? hex(0x9dffb8) : hex(0xffb09d));
    if (cfg.exam) {
        text(fmt("PENALTY %d / 100", penTotal), 28, 76, 2, penTotal == 0 ? hex(0x9dffb8) : (penTotal < 60 ? hex(0xffd59d) : hex(0xff8a7a)));
        setc(hex(0x2a313b)); rectTL(28, 94, 312, 10);
        gradH(28, 94, 312 * clampf(penTotal / 100.f, 0, 1), 10, hex(0xffb020), hex(0xff4a3a));
        text(belt ? "BELT ON" : "BELT OFF", 340, 76, 2, belt ? hex(0x9dffb8) : hex(0xff8a7a), 2);
    } else text(fmt("HITS %d", pHits), 28, 76, 2, pHits == 0 ? hex(0x9dffb8) : hex(0xffd59d));
    // penalty log
    {
        float y = cfg.exam ? 144.f : 118.f; int shown = 0;
        for (int i = (int)pens.size() - 1; i >= 0 && shown < 6; i--, shown++) {
            float age = gTime - pens[i].t, a = clampf(1.f - (age - 6.f) / 2.f, 0.25f, 1.f);
            string s = fmt("+%d  ", pens[i].pts) + pens[i].msg;
            setc(Col{0, 0, 0, 0.45f * a}); rectTL(14, y - 4, textW(s, 2) + 20, 22);
            text(s, 24, y, 2, Col{1.f, 0.62f, 0.52f, a});
            y += 24;
        }
    }
    // minimap (only useful when the map is larger than the view)
    bool bigMap = cur.W * zoom > fbw * 0.95f || cur.H * zoom > fbh * 0.95f;
    if (cfg.minimap && (bigMap || cfg.cam == CAM_ROTATE) && cfg.cam != CAM_FIXED) {
        float mw = 200, mh = std::min(150.f, 200.f * cur.H / cur.W + 4);
        drawMinimap(fbw - mw - 22, 22, mw, mh);
    }
    // ---- HUD: current task panel (top centre) with a compass arrow to the target
    if (!wld.tasks.empty() && !pDone && !pFailed) {
        float tw = std::min(640.f, fbw - 2 * 372.f), th = 70, tx = fbw / 2.f - tw / 2, ty = 12;
        auto fitPx = [](const string& str, float px, float avail) { return std::min(px, avail / std::max(1.f, textW(str, 1.f))); };
        float textAvail = tw - 16 - 84;   // leave room for the compass
        panel(tx, ty, tw, th, 0.9f);
        int nT = (int)wld.tasks.size();
        if (navTask) {
            const Item& t = wld.tasks[wld.taskIdx];
            string head = fmt("TASK %d/%d", (int)wld.taskIdx + 1, nT);
            textS(head, tx + 16, ty + 12, 2.6f, hex(0xffc83d));
            float hx = tx + 16 + textW(head, 2.6f) + 14;
            string ttl = taskTitle(t);
            textS(ttl, hx, ty + 12, fitPx(ttl, 2.6f, tx + 16 + textAvail - hx), hex(0xffffff));
            bool live = !wld.taskHint.empty();
            string sub = live ? wld.taskHint : taskHelp(t);
            text(sub, tx + 16, ty + 44, fitPx(sub, 1.8f, textAvail), live ? hex(0x9dffb8) : hex(0xaab4c8));
            if (wld.taskHold > 0) {
                setc(hex(0x2a313b)); rectTL(tx + 16, ty + th - 8, tw - 100, 4);
                setc(hex(0x7dffb0)); rectTL(tx + 16, ty + th - 8, (tw - 100) * clampf(wld.taskHold / TASK_HOLD, 0, 1), 4);
            }
        } else {
            textS("ALL TASKS DONE", tx + 16, ty + 12, 2.6f, hex(0x7dffb0));
            string sub = cfg.exam ? "PARK IN THE FINAL SPOT, STOP AND SHIFT TO P" : "PARK IN THE FINAL SPOT";
            text(sub, tx + 16, ty + 44, fitPx(sub, 1.8f, textAvail), hex(0xaab4c8));
        }
        // compass: direction to the target as seen on screen (includes the rotating camera)
        V2 d = navT - car.p; float dist = vlen(d);
        float ccx = tx + tw - 40, ccy = ty + th / 2;
        setc(hex(0x0b0e12)); circle(ccx, ccy, 25, 24);
        setc(navTask ? hex(0xffc83d, 0.8f) : hex(0x38bdf8, 0.8f)); ring(ccx, ccy, 23, 25, 24);
        glPushMatrix(); glTranslatef(ccx, ccy, 0); glRotatef(deg(std::atan2(d.y, d.x) + ang), 0, 0, 1);
        setc(navTask ? hex(0xffc83d) : hex(0x38bdf8)); chevron(15.f);
        glPopMatrix();
        text(fmt("%dM", (int)std::round(dist)), ccx, ty + th + 6, 2, hex(0xd6dce6), 1);
    }
    // ---- HUD: task checklist (right side, under the minimap)
    if (!wld.tasks.empty()) {
        bool mm = cfg.minimap && (bigMap || cfg.cam == CAM_ROTATE) && cfg.cam != CAM_FIXED;
        float lw = 216, lx = fbw - lw - 14, ly = mm ? 22 + std::min(150.f, 200.f * cur.H / cur.W + 4) + 22 : 14;
        int rows = (int)wld.tasks.size() + 1;
        float rh = 21, lh = 34 + rows * rh;
        if (ly + lh < fbh - 190) {
            panel(lx, ly, lw, lh, 0.82f);
            text("EXAM TASKS", lx + 12, ly + 10, 2, hex(0xffc83d));
            for (int i = 0; i < rows; i++) {
                float ry = ly + 30 + i * rh;
                bool fin = i == rows - 1;
                bool done = fin ? pDone : (size_t)i < wld.taskIdx;
                bool curT = fin ? (!wld.tasksLeft() && !pDone) : (size_t)i == wld.taskIdx;
                string lab = fin ? "FINAL SPOT" : fmt("%d ", i + 1) + taskShort(wld.tasks[i]);
                if (curT) { setc(hex(0xffc83d, 0.14f)); rectTL(lx + 6, ry - 3, lw - 12, rh - 2); }
                float ix = lx + 22, iy = ry + 6;
                if (done) { setc(hex(0x3ce07a)); seg(ix - 6, iy, ix - 2, iy + 5, 3); seg(ix - 2, iy + 5, ix + 6, iy - 5, 3); }
                else if (curT) { setc(hex(0xffc83d)); tri(ix + 6, iy, ix - 4, iy - 6, ix - 4, iy + 6); }
                else if (fin) { setc(hex(0xff6b5a, 0.8f)); rect(ix, iy + 2, 9, 7); ring(ix, iy - 2, 2.5f, 4.5f, 10); }
                else { setc(hex(0x59616f)); circle(ix, iy, 3.5f, 10); }
                text(lab, lx + 38, ry + 2, 1.8f, done ? hex(0x9dffb8) : curT ? hex(0xffffff) : hex(0x8a94a6));
            }
        }
    }
    // ---- HUD: bottom - gear selector, speed, signals
    float by = fbh - 150.f;
    panel(14, by - 6, 250, 140, 0.82f);
    for (int g = 0; g < 4; g++) {
        float gx = 30, gy = by + 6 + g * 31;
        bool sel = car.gear == g;
        Col gc = g == G_P ? hex(0xff6b5a) : g == G_R ? hex(0xffb020) : g == G_N ? hex(0xaab4c8) : hex(0x3ce07a);
        bool hov = hovered(gx - 4, gy - 2, 42, 29);
        if (sel) { setc(Col{gc.r, gc.g, gc.b, 0.25f}); rrect(gx + 17, gy + 12, 42, 28, 6); }
        setc(sel ? gc : (hov ? hex(0x4a5870) : hex(0x2c343f))); rrect(gx + 17, gy + 12, 34, 24, 5);
        text(string(1, GEARCH[g]), gx + 17, gy + 5, 2.4f, sel ? hex(0x10141a) : hex(0xd6dce6), 1);
        if (hov && in.clickL) { in.clickL = false; shiftTo(g); }
    }
    bool over = cfg.exam && kmh > 20.5f;
    textS(fmt("%d", (int)std::round(kmh)), 165, by + 24, 7, over ? hex(0xff6b5a) : hex(0xffffff), 1);
    text("KM/H", 165, by + 80, 2, hex(0x8a94a6), 1);
    if (cfg.exam) text("LIMIT 20", 165, by + 98, 2, over ? hex(0xff6b5a) : hex(0x59616f), 1);
    bool blinkOn = sigPhase;
    drawSignalArrow(96, by + 116, true, sigL && blinkOn);
    drawSignalArrow(234, by + 116, false, sigR && blinkOn);
    if (car.hb) text("HANDBRAKE", 165, by + 116, 2, hex(0xff6b5a), 1);
    // ---- steering wheel (drag it with the mouse)
    float wcx = fbw / 2.f, wcy = fbh - 92.f, wR = 74.f;
    {
        float mdx = (float)in.mx - wcx, mdy = (float)in.my - wcy, md = std::sqrt(mdx * mdx + mdy * mdy);
        if (!hudWheelDrag && in.clickL && md < wR + 6 && !pPaused && !pDone && !pFailed) {
            hudWheelDrag = true; in.clickL = false; hudWheelLastAng = std::atan2(mdy, mdx); hudWheelTarget = car.wheel;
        }
        if (hudWheelDrag) {
            if (!in.downL) hudWheelDrag = false;
            else if (md > 8) {
                float a = std::atan2(mdy, mdx);
                hudWheelTarget = clampf(hudWheelTarget + deg(std::remainder(a - hudWheelLastAng, 2 * PI)), -WHEEL_MAX, WHEEL_MAX);
                hudWheelLastAng = a;
            }
        }
        drawWheelHUD(wcx, wcy, wR, car.wheel);
        float wd = std::fabs(car.wheel);
        text(wd < 1 ? string("STEER 0") : fmt("STEER %d %s", (int)wd, car.wheel > 0 ? "R" : "L"), wcx, fbh - 14.f - 2, 2,
             hudWheelDrag ? hex(0xffe08a) : hex(0xaab4c8), 1);
        // tyre angle indicator
        text(fmt("TYRES %.0f", deg(std::fabs(car.steer))), wcx + wR + 16, wcy - 6, 2, hex(0x59616f));
    }
    // ---- pedals (click & hold with the mouse)
    {
        float px = fbw - 230.f, py = fbh - 150.f;
        bool live = !pPaused && !pDone && !pFailed;
        hudBrake = live && in.downL && hovered(px, py, 96, 96) && !hudWheelDrag;
        hudGas = live && in.downL && hovered(px + 130, py - 22, 64, 118) && !hudWheelDrag;
        if (hovered(px, py, 96, 96) || hovered(px + 130, py - 22, 64, 118)) in.clickL = false;
        drawPedal(px, py, 96, 96, car.brake > 0.05f, car.brake, "BRAKE", hex(0xff6b5a));
        drawPedal(px + 130, py - 22, 64, 118, car.gas > 0.05f, car.gas, "GAS", hex(0x3ce07a));
    }
    // ---- prompts
    if (pInSpot && !pDone && !pFailed) {
        bool still = std::fabs(car.vf) < 0.05f;
        string msg = wld.tasksLeft() ? fmt("FINAL SPOT LOCKED - FINISH TASK %d/%d FIRST", (int)wld.taskIdx + 1, (int)wld.tasks.size())
                   : !still ? "IN THE SPOT - STOP THE CAR" : (car.gear != G_P ? "STOPPED - SHIFT TO P (" + keyName(cfg.bind[A_PARK][0]) + ") TO FINISH" : "HOLD STILL...");
        textS(msg, fbw / 2.f, fbh - 236.f, 3, hex(0x9dffb8), 1);
        if (!cfg.exam && !wld.tasksLeft()) {
            setc(hex(0x2a313b)); rectTL(fbw / 2.f - 100, fbh - 208.f, 200, 10);
            gradH(fbw / 2.f - 100, fbh - 208.f, 200 * clampf(pPark / 0.9f, 0, 1), 10, hex(0x2ecc71), hex(0x9dffb8));
        }
    }
    if (!pDone && !pFailed && car.gear == G_P && !pInSpot && pIntro > 2.4f) {
        string m = cfg.exam && !belt ? "FASTEN BELT (" + keyName(cfg.bind[A_BELT][0]) + "), SIGNAL LEFT (" + keyName(cfg.bind[A_SIGL][0]) + "), SHIFT TO D (" + keyName(cfg.bind[A_DRIVE][0]) + ")"
                                     : "SHIFT TO D (" + keyName(cfg.bind[A_DRIVE][0]) + ") OR R (" + keyName(cfg.bind[A_REVERSE][0]) + ") TO DRIVE";
        textS(m, fbw / 2.f, wld.tasks.empty() ? 20.f : 106.f, 2.5f, hex(0xffe08a), 1);
    }
    if (pClock < 14.f && !pDone && !pFailed) {
        float a = clampf(14.f - pClock, 0, 1);
        float hw = std::max(textW(hintLine(0), 1.6f), textW(hintLine(1), 1.6f)) + 24;
        setc(Col{0, 0, 0, 0.5f * a}); rectTL(fbw / 2.f - hw / 2, fbh - 214.f, hw, 40);
        text(hintLine(0), fbw / 2.f, fbh - 207.f, 1.6f, hex(0xc4ccd8, 0.9f * a), 1);
        text(hintLine(1), fbw / 2.f, fbh - 191.f, 1.6f, hex(0xc4ccd8, 0.9f * a), 1);
    }

    float cx = fbw / 2.f, cy = fbh / 2.f;
    if (pIntro < 2.4f && !pDone && !pFailed) {
        float a = pIntro < 0.4f ? pIntro / 0.4f : (pIntro > 1.8f ? (2.4f - pIntro) / 0.6f : 1.f);
        float slide = (1.f - clampf(pIntro / 0.4f, 0, 1)) * 60.f;
        gAlpha = a;
        setc(Col{0, 0, 0, 0.55f}); rectTL(0, cy - 60 + slide, (float)fbw, 96);
        setc(hex(0xffc83d)); rectTL(0, cy - 60 + slide, (float)fbw, 3); rectTL(0, cy + 33 + slide, (float)fbw, 3);
        string lab = pmode == PM_CAREER ? fmt("LEVEL %d", careerIdx + 1) : (pmode == PM_TEST ? "TEST DRIVE" : "CUSTOM LEVEL");
        if (cfg.exam) lab += "  -  EXAM RULES";
        text(lab, cx, cy - 46 + slide, 3, hex(0xffc83d), 1);
        textS(cur.name, cx, cy - 18 + slide, 6, hex(0xffffff), 1);
        gAlpha = 1.f;
    }
    if (pDone || pFailed) {
        setc(hex(0x000000, 0.6f)); rectTL(0, 0, (float)fbw, (float)fbh);
        panel(cx - 300, cy - 230, 600, 460, 0.97f);
        if (pFailed) {
            textS("EXAM FAILED", cx, cy - 205, 8, hex(0xff6b5a), 1);
            text(fmt("PENALTY %d POINTS (100+ = FAIL)", penTotal), cx, cy - 140, 2.5f, hex(0xffd0c8), 1);
        } else {
            textS(cfg.exam ? "EXAM PASSED!" : "PARKED!", cx, cy - 205, 8, hex(0x7dffb0), 1);
            drawStarsAnim(cx, cy - 115, 24, pStars, pDoneT);
            text(cfg.exam ? fmt("TIME %.1fS   PENALTY %d   PAR %.0fS", pTime, penTotal, cur.par)
                          : fmt("TIME %.1fS   HITS %d   PAR %.0fS", pTime, pHits, cur.par), cx, cy - 72, 2.5f, hex(0xffffff), 1);
        }
        float ly = pFailed ? cy - 108 : cy - 44;
        if (pens.empty()) text(cfg.exam ? "NO MISTAKES - PERFECT DRIVE!" : (pHits == 0 ? "CLEAN RUN +1" : "NO HITS = +1 STAR"), cx, ly, 2, hex(0xaab4c8), 1);
        for (int i = 0; i < (int)pens.size() && i < 6; i++)
            text(fmt("+%d  ", pens[i].pts) + pens[i].msg, cx, ly + i * 20, 2, pens[i].pts >= 100 ? hex(0xff8a7a) : hex(0xffd59d), 1);
        float by2 = cy + 92;
        bool hasNext = !pFailed && pmode == PM_CAREER && careerIdx + 1 < (int)CAREER.size();
        if (hasNext && button("NEXT LEVEL", cx - 240, by2, 230, 46)) { startPlay(CAREER[careerIdx + 1], PM_CAREER, careerIdx + 1, fmt("career_%d", careerIdx + 1)); return; }
        if (!hasNext && !pFailed && pmode == PM_CAREER) text("CAREER COMPLETE!", cx - 125, by2 + 16, 3, hex(0xffc83d), 1);
        if (pFailed && button("RETRY", cx - 240, by2, 230, 46)) { restartPlay(); return; }
        if (!pFailed && button("RETRY", cx + 10, by2, 230, 46)) { restartPlay(); return; }
        if (pFailed && button(pmode == PM_TEST ? "BACK TO EDITOR" : "MENU", cx + 10, by2, 230, 46)) { leavePlay(); return; }
        if (!pFailed && button(pmode == PM_TEST ? "BACK TO EDITOR" : "MENU", cx - 115, by2 + 62, 230, 46)) { leavePlay(); return; }
        if (!pFailed) drawConfetti();
    } else if (pPaused) {
        setc(hex(0x000000, 0.55f)); rectTL(0, 0, (float)fbw, (float)fbh);
        panel(cx - 200, cy - 170, 400, 340, 0.97f);
        textS("PAUSED", cx, cy - 145, 8, hex(0xffffff), 1);
        if (button("RESUME", cx - 140, cy - 76, 280, 46)) pPaused = false;
        if (button("RESTART", cx - 140, cy - 20, 280, 46)) { restartPlay(); return; }
        if (button("SETTINGS", cx - 140, cy + 36, 280, 46)) { openSettings(S_PLAY); return; }
        if (button(pmode == PM_TEST ? "BACK TO EDITOR" : "QUIT TO MENU", cx - 140, cy + 92, 280, 46)) { leavePlay(); return; }
        text(hintLine(0), cx, cy + 196, 1.8f, hex(0xc4ccd8), 1);
        text(hintLine(1), cx, cy + 216, 1.8f, hex(0xc4ccd8), 1);
    }
}
