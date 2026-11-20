// SmartPark - render.h
// item and car rendering, task zones, level drawing
// author: Rasul
// NOTE: not a standalone header - included only from main.cpp, in a fixed order.
#pragma once

// ----------------------------------------------------------------------------
//  Item / car rendering
// ----------------------------------------------------------------------------
// blink bits: 1 = left indicators, 2 = right indicators
static void drawCar(Col body, float steer, bool brake, bool lights = false, int blink = 0, bool reverse = false) {
    if (lights) {
        beam(2.1f, -0.65f, 9.f, 2.0f, hex(0xfff3b0, 0.30f));
        beam(2.1f, 0.65f, 9.f, 2.0f, hex(0xfff3b0, 0.30f));
    }
    setc(Col{0, 0, 0, 0.28f}); rrect(0.12f, 0.18f, 4.2f, 1.9f, 0.5f);
    setc(hex(0x15171a));
    for (int sx = -1; sx <= 1; sx += 2) for (int sy = -1; sy <= 1; sy += 2) {
        glPushMatrix(); glTranslatef(sx * 1.3f, sy * 0.92f, 0);
        if (sx > 0) glRotatef(steer * 57.3f, 0, 0, 1);
        rect(0, 0, 0.75f, 0.32f); glPopMatrix();
    }
    setc(shade(body, 0.8f)); rect(0.75f, -1.05f, 0.3f, 0.2f); rect(0.75f, 1.05f, 0.3f, 0.2f);
    setc(shade(body, 0.85f)); rrect(0, 0, 4.2f, 1.9f, 0.6f);
    gradV(-2.1f + 0.05f, -0.95f + 0.05f, 4.1f, 0.9f, shade(body, 1.18f), shade(body, 1.0f));
    setc(body); rrect(0, 0.1f, 4.0f, 1.55f, 0.55f);
    setc(shade(body, 1.2f)); rrect(1.3f, 0, 1.0f, 1.0f, 0.3f);
    setc(shade(body, 0.72f)); rrect(-0.2f, 0, 2.0f, 1.6f, 0.4f);
    setc(hex(0x9fd0ee)); rrect(0.85f, 0, 0.6f, 1.4f, 0.15f);
    setc(hex(0xffffff, 0.35f)); rect(0.95f, -0.35f, 0.12f, 0.5f);
    setc(hex(0x4d6a85)); rrect(-1.2f, 0, 0.4f, 1.3f, 0.12f);
    setc(hex(0xfff3b0)); rect(2.0f, -0.65f, 0.2f, 0.4f); rect(2.0f, 0.65f, 0.2f, 0.4f);
    setc(brake ? hex(0xff2a2a) : hex(0x8a1c1c)); rect(-2.0f, -0.65f, 0.18f, 0.42f); rect(-2.0f, 0.65f, 0.18f, 0.42f);
    if (reverse) { setc(hex(0xffffff)); rect(-2.02f, -0.3f, 0.14f, 0.22f); rect(-2.02f, 0.3f, 0.14f, 0.22f);
                   glow(-2.1f, 0, 1.6f, hex(0xffffff, 0.35f)); }
    if (lights) { glow(2.1f, -0.65f, 1.0f, hex(0xfff3b0, 0.5f)); glow(2.1f, 0.65f, 1.0f, hex(0xfff3b0, 0.5f)); }
    if (brake) { glow(-2.1f, -0.65f, 1.5f, hex(0xff2a2a, 0.55f)); glow(-2.1f, 0.65f, 1.5f, hex(0xff2a2a, 0.55f)); }
    for (int side = 0; side < 2; side++) {
        if (!(blink & (1 << side))) continue;
        float y = side == 0 ? -0.86f : 0.86f;
        setc(hex(0xffb020)); rect(1.95f, y, 0.3f, 0.2f); rect(-1.95f, y, 0.3f, 0.2f);
        glow(2.0f, y, 1.1f, hex(0xffb020, 0.7f)); glow(-2.0f, y, 1.1f, hex(0xffb020, 0.7f));
    }
}
static void drawConeShape(float w, bool knocked) {
    if (!knocked) {
        setc(Col{0, 0, 0, 0.25f}); circle(0.1f, 0.12f, w * 0.52f, 12);
        setc(hex(0xc2410c)); rect(0, 0, w, w);
        setc(hex(0xff7a1a)); circle(0, 0, w * 0.45f);
        setc(hex(0xffffff)); circle(0, 0, w * 0.28f);
        setc(hex(0xff7a1a)); circle(0, 0, w * 0.13f);
    } else {   // lying on its side
        setc(Col{0, 0, 0, 0.25f}); rect(0.08f, 0.1f, w * 1.1f, w * 0.6f);
        setc(hex(0xc2410c)); rect(-w * 0.45f, 0, w * 0.2f, w * 0.75f);
        setc(hex(0xff7a1a)); tri(-w * 0.4f, -w * 0.32f, -w * 0.4f, w * 0.32f, w * 0.55f, 0);
        setc(hex(0xffffff)); rect(0, 0, w * 0.16f, w * 0.34f);
    }
}
static bool gSpotLocked = false;   // set while playing a level whose tasks are not finished yet
static void drawTaskZone(const Item& it, int ord, int state, float hold01 = 0);
static void drawItem(const Item& it, bool editor, bool activeSpot = false) {
    glPushMatrix();
    glTranslatef(it.x, it.y, 0); glRotatef(it.rot, 0, 0, 1);
    float w = it.w, h = it.h;
    switch (it.type) {
        case T_CROSSING:
            setc(hex(0x313c43)); rect(0, 0, w, h);
            setc(hex(0xe8e6d9));
            for (int k = 0; k < 5; k++) rect(-w / 2 + w * (k + 0.5f) / 5, 0, w / 9, h * 0.85f);
            break;
        case T_ACCESSIBLE:
            setc(hex(0x2563a5)); rect(0, 0, w, h);
            setc(hex(0xf7f7f7)); rect(0, 0, w * 0.76f, 0.13f);
            text("P", -0.35f, -0.45f, 0.18f, hex(0xffffff));
            break;
        case T_ENTRANCE:
            setc(hex(0x3e8f70)); rect(0, 0, w, h);
            setc(hex(0xf8f8f8)); rect(0, 0, w * 0.72f, 0.15f);
            break;
        case T_WALL:
            setc(Col{0, 0, 0, 0.3f}); rect(0.15f, 0.2f, w, h);
            setc(hex(0x3d434c)); rect(0, 0, w, h);
            setc(hex(0x6c7583)); rect(0, 0, std::max(0.1f, w - 0.25f), std::max(0.1f, h - 0.25f));
            setc(hex(0x8892a2, 0.55f)); rect(0, -h / 2 + 0.2f, std::max(0.1f, w - 0.3f), 0.08f);
            break;
        case T_BARRIER: {
            setc(Col{0, 0, 0, 0.3f}); rect(0.12f, 0.15f, w, h);
            int n = std::max(1, (int)std::ceil(w / 0.6f)); float sw = w / n;
            for (int i = 0; i < n; i++) { setc(i % 2 ? hex(0x1c1c1c) : hex(0xffc400)); rect(-w / 2 + sw * (i + 0.5f), 0, sw, h); }
            break; }
        case T_CURB: {
            setc(Col{0, 0, 0, 0.3f}); rect(0.08f, 0.12f, w, h);
            int n = std::max(1, (int)std::ceil(w / 1.0f)); float sw = w / n;
            for (int i = 0; i < n; i++) { setc(i % 2 ? hex(0xf1f1f1) : hex(0xd6342c)); rect(-w / 2 + sw * (i + 0.5f), 0, sw, h); }
            break; }
        case T_ARCCURB: {
            float r = w, sw = h, len = rad(sw) * r;
            setc(Col{0, 0, 0, 0.3f}); arcBand(0.1f, 0.14f, r - CURB_T / 2, r + CURB_T / 2, 0, sw);
            int n = std::max(1, (int)std::ceil(len / 1.0f));
            for (int i = 0; i < n; i++) {
                setc(i % 2 ? hex(0xf1f1f1) : hex(0xd6342c));
                arcBand(0, 0, r - CURB_T / 2, r + CURB_T / 2, sw * i / n, sw / n, 3);
            }
            break; }
        case T_ARCLINE: {
            float r = w, sw = h;
            setc(hex(0xffffff, 0.85f));
            if (it.color == 0) arcBand(0, 0, r - LINE_T / 2, r + LINE_T / 2, 0, sw);
            else {
                float len = rad(sw) * r; int n = std::max(1, (int)(len / 1.0f));
                for (int i = 0; i < n; i++) arcBand(0, 0, r - 0.1f, r + 0.1f, sw * i / n, sw / n * 0.6f, 3);
            }
            break; }
        case T_ISLAND: {
            float R = w / 2;
            setc(Col{0, 0, 0, 0.3f}); circle(0.2f, 0.25f, R, 40);
            setc(hex(0x3f7d3a)); circle(0, 0, R - CURB_T * 0.5f, 40);
            setc(hex(0x4f9446)); circle(-R * 0.1f, -R * 0.1f, (R - CURB_T) * 0.7f, 32);
            int n = std::max(8, (int)(2 * PI * R / 1.0f)); n += n % 2;
            for (int i = 0; i < n; i++) { setc(i % 2 ? hex(0xf1f1f1) : hex(0xd6342c)); arcBand(0, 0, R - CURB_T, R, 360.f * i / n, 360.f / n, 2); }
            if (R > 2.5f) { setc(hex(0xe8c547)); for (int i = 0; i < 6; i++) { float a = i * PI / 3; circle(std::cos(a) * R * 0.45f, std::sin(a) * R * 0.45f, 0.18f, 8); } }
            break; }
        case T_PCAR: glPushMatrix(); glScalef(w / 4.2f, h / 1.9f, 1); drawCar(hex(CARCOL[it.color & 7]), 0, false); glPopMatrix(); break;
        case T_CONE: drawConeShape(w, false); break;
        case T_TREE: {
            glRotatef(-it.rot, 0, 0, 1);
            float r = w * 0.5f;
            setc(Col{0, 0, 0, 0.28f}); circle(0.25f, 0.3f, r);
            setc(hex(0x2d7a3a)); circle(0, 0, r);
            setc(hex(0x3da34b)); circle(-r * 0.12f, -r * 0.12f, r * 0.72f);
            setc(hex(0x5cc468)); circle(-r * 0.22f, -r * 0.22f, r * 0.4f);
            break; }
        case T_SPOT: {
            if (gSpotLocked && !editor) {      // final spot is closed until all tasks are done
                setc(hex(0x5a6270, 0.35f)); rect(0, 0, w, h);
                setc(hex(0xff6b5a, 0.8f));
                rect(0, -h / 2 + 0.07f, w, 0.14f); rect(0, h / 2 - 0.07f, w, 0.14f);
                rect(-w / 2 + 0.07f, 0, 0.14f, h); rect(w / 2 - 0.07f, 0, 0.14f, h);
                glPushMatrix(); glRotatef(-it.rot, 0, 0, 1);
                setc(hex(0xff6b5a, 0.9f)); rect(0, 0.25f, 1.1f, 0.85f);                 // padlock body
                setc(hex(0xff6b5a, 0.9f)); arcBand(0, -0.18f, 0.3f, 0.45f, 180, 180, 10); // shackle
                setc(hex(0x2a1a1a)); circle(0, 0.25f, 0.12f, 8);
                glPopMatrix();
                break;
            }
            float pulse = 0.5f + 0.5f * std::sin(gTime * 4.f);
            setc(activeSpot ? hex(0x2ecc71, 0.5f) : hex(0x3b82f6, 0.22f + 0.1f * pulse));
            rect(0, 0, w, h);
            if (!activeSpot) { setc(hex(0x3b82f6, 0.10f * pulse)); rect(0, 0, w + 0.6f, h + 0.6f); }
            setc(activeSpot ? hex(0x7dffb0) : hex(0xffffff));
            rect(0, -h / 2 + 0.07f, w, 0.14f); rect(0, h / 2 - 0.07f, w, 0.14f);
            rect(-w / 2 + 0.07f, 0, 0.14f, h); rect(w / 2 - 0.07f, 0, 0.14f, h);
            glPushMatrix(); glRotatef(-it.rot, 0, 0, 1);
            text("P", -0.45f, -0.6f, 0.17f, hex(0xffffff, 0.85f));
            glPopMatrix();
            break; }
        case T_BAY:
            setc(hex(0xffffff, 0.8f));
            rect(0, -h / 2 + 0.05f, w, 0.1f); rect(0, h / 2 - 0.05f, w, 0.1f);
            rect(-w / 2 + 0.05f, 0, 0.1f, h); rect(w / 2 - 0.05f, 0, 0.1f, h);
            break;
        case T_START:
            if (editor) {
                gAlpha *= 0.55f; drawCar(hex(0x38bdf8), 0, false); gAlpha /= 0.55f;
                setc(hex(0xffffff)); tri(3.3f, 0, 2.5f, -0.5f, 2.5f, 0.5f);
                text("START", -1.4f, -2.2f, 0.14f, hex(0xffffff));
            }
            break;
        case T_MUD: {
            setc(hex(0x6b4423, 0.9f)); rect(0, 0, w, h);
            if (gLite) break;
            unsigned s = (unsigned)(it.x * 73 + it.y * 31 + 7);
            setc(hex(0x4a2f17, 0.8f));
            int n = std::min(600, (int)(w * h / 3) + 6);
            for (int i = 0; i < n; i++) {
                s = s * 1664525u + 1013904223u; float px = ((s >> 8) & 1023) / 1023.f - 0.5f;
                s = s * 1664525u + 1013904223u; float py = ((s >> 8) & 1023) / 1023.f - 0.5f;
                circle(px * (w - 0.6f), py * (h - 0.6f), 0.22f + 0.15f * ((i * 7) % 3), 10);
            }
            break; }
        case T_GRASS: {
            setc(hex(0x3c7a35)); rect(0, 0, w, h);
            if (gLite) break;
            setc(hex(0x2f6629, 0.9f));
            glBegin(GL_QUADS);
            int n = std::min(4000, (int)(w * h * 0.7f) + 4);
            unsigned s = (unsigned)(it.x * 91 + it.y * 17 + 3);
            for (int i = 0; i < n; i++) {
                s = s * 1664525u + 1013904223u; float px = (((s >> 8) & 4095) / 4095.f - 0.5f) * (w - 0.2f);
                s = s * 1664525u + 1013904223u; float py = (((s >> 8) & 4095) / 4095.f - 0.5f) * (h - 0.3f);
                glVertex2f(px, py); glVertex2f(px + 0.08f, py); glVertex2f(px + 0.12f, py + 0.3f); glVertex2f(px + 0.04f, py + 0.3f);
            }
            glEnd();
            setc(hex(0x2a5a25)); rect(0, -h / 2 + 0.06f, w, 0.12f); rect(0, h / 2 - 0.06f, w, 0.12f);
            rect(-w / 2 + 0.06f, 0, 0.12f, h); rect(w / 2 - 0.06f, 0, 0.12f, h);
            break; }
        case T_HILL: {
            int d = it.color & 3;
            Col lo = hex(0x4a525d), hi = hex(0x7a8494);
            if (d == 0) gradH(-w / 2, -h / 2, w, h, lo, hi);
            else if (d == 2) gradH(-w / 2, -h / 2, w, h, hi, lo);
            else if (d == 1) gradV(-w / 2, -h / 2, w, h, lo, hi);
            else gradV(-w / 2, -h / 2, w, h, hi, lo);
            setc(hex(0xffffff, 0.22f));
            for (float yy = -h / 2 + 1.5f; yy < h / 2 - 0.5f; yy += 3.f)
                for (float xx = -w / 2 + 1.5f; xx < w / 2 - 0.5f; xx += 3.f) {
                    glPushMatrix(); glTranslatef(xx, yy, 0); glRotatef(90.f * d, 0, 0, 1);
                    glBegin(GL_QUADS);
                    glVertex2f(-0.5f, -0.7f); glVertex2f(-0.1f, -0.7f); glVertex2f(0.5f, 0.f); glVertex2f(0.1f, 0.f);
                    glVertex2f(0.1f, 0.f); glVertex2f(0.5f, 0.f); glVertex2f(-0.1f, 0.7f); glVertex2f(-0.5f, 0.7f);
                    glEnd(); glPopMatrix();
                }
            setc(hex(0xffc400, 0.8f));
            rect(0, -h / 2 + 0.1f, w, 0.2f); rect(0, h / 2 - 0.1f, w, 0.2f);
            if (!gLite) text("16%", -w / 2 + 0.4f, -h / 2 + 0.45f, 0.14f, hex(0xffffff, 0.7f));
            break; }
        case T_ICE: {
            setc(hex(0xa6e3ff, 0.55f)); rect(0, 0, w, h);
            setc(hex(0xffffff, 0.45f));
            glBegin(GL_QUADS);
            for (float d = -h; d < w; d += 1.6f) {
                float ax = -w / 2 + d, ay = -h / 2, bx = ax + h, by = h / 2;
                if (ax < -w / 2) { ay += (-w / 2 - ax); ax = -w / 2; }
                if (bx > w / 2) { by -= (bx - w / 2); bx = w / 2; }
                if (ax >= bx - 0.05f) continue;
                glVertex2f(ax, ay); glVertex2f(ax + 0.18f, ay); glVertex2f(bx + 0.18f, by); glVertex2f(bx, by);
            }
            glEnd();
            break; }
        case T_LINE: {
            setc(hex(0xffffff, 0.85f));
            int n = std::max(1, (int)(w / 1.0f));
            for (int i = 0; i < n; i++) rect(-w / 2 + (i + 0.5f) * (w / n), 0, 0.6f * (w / n), h);
            break; }
        case T_CTRLLINE:
            setc(hex(0xffffff, 0.92f)); rect(0, 0, w, h);
            if (editor) { setc(hex(0xff5a5a, 0.45f)); rect(0, 0, w, h + 0.12f); }
            break;
        case T_STOPLINE:
            setc(hex(0xffffff, 0.95f)); rect(0, 0, w, h);
            if (!gLite) {
                glPushMatrix(); glTranslatef(-1.3f, 0, 0); glRotatef(90, 0, 0, 1);
                float px = std::min(0.22f, h / 26.f);
                text("STOP", 0, -3.5f * px, px, hex(0xffffff, 0.85f), 1);
                glPopMatrix();
                if (editor) { setc(hex(0xffc83d, 0.8f)); tri(1.4f, 0, 0.6f, -0.45f, 0.6f, 0.45f); }
            }
            break;
        default: break;
    }
    glPopMatrix();
    if (it.type == T_TASK) drawTaskZone(it, -1, 3);
}
// thick segment (world units)
static void seg(float ax, float ay, float bx, float by, float th) {
    float dx = bx - ax, dy = by - ay, l = std::sqrt(dx * dx + dy * dy);
    if (l < 1e-5f) return;
    float nx = -dy / l * th / 2, ny = dx / l * th / 2;
    glBegin(GL_QUADS);
    glVertex2f(ax + nx, ay + ny); glVertex2f(bx + nx, by + ny); glVertex2f(bx - nx, by - ny); glVertex2f(ax - nx, ay - ny);
    glEnd();
}
static void outlineRect(float w, float h, float th, bool dashed) {
    if (!dashed) {
        rect(0, -h / 2 + th / 2, w, th); rect(0, h / 2 - th / 2, w, th);
        rect(-w / 2 + th / 2, 0, th, h); rect(w / 2 - th / 2, 0, th, h);
        return;
    }
    const float d = 0.6f;
    for (float x = -w / 2; x < w / 2; x += 2 * d) { float e = std::min(x + d, w / 2); rect((x + e) / 2, -h / 2 + th / 2, e - x, th); rect((x + e) / 2, h / 2 - th / 2, e - x, th); }
    for (float y = -h / 2; y < h / 2; y += 2 * d) { float e = std::min(y + d, h / 2); rect(-w / 2 + th / 2, (y + e) / 2, th, e - y); rect(w / 2 - th / 2, (y + e) / 2, th, e - y); }
}
// nav-style chevron arrow pointing +x, centred at the origin, length ~ 2*s
static void chevron(float s) {
    glBegin(GL_TRIANGLES);
    glVertex2f(1.0f * s, 0); glVertex2f(-0.7f * s, -0.85f * s); glVertex2f(-0.25f * s, 0);
    glVertex2f(1.0f * s, 0); glVertex2f(-0.25f * s, 0); glVertex2f(-0.7f * s, 0.85f * s);
    glEnd();
}
// state: 0 = upcoming, 1 = current, 2 = done, 3 = editor.  ord = 0-based task number (-1 = no badge)
static void drawTaskZone(const Item& it, int ord, int state, float hold01) {
    int kind = taskKind(it);
    float w = it.w, h = it.h, pulse = 0.5f + 0.5f * std::sin(gTime * 5.f);
    Col c = state == 1 ? hex(0xffc83d) : state == 2 ? hex(0x3ce07a) : state == 3 ? hex(0xff9f43) : hex(0xffffff);
    float fillA = state == 1 ? 0.14f + 0.12f * pulse : state == 2 ? 0.10f : state == 3 ? 0.16f : 0.05f;
    float lineA = state == 1 ? 0.95f : state == 2 ? 0.55f : state == 3 ? 0.85f : 0.35f;
    glPushMatrix(); glTranslatef(it.x, it.y, 0); glRotatef(it.rot, 0, 0, 1);
    if (state == 1 && !gLite) { setc(Col{c.r, c.g, c.b, 0.10f * pulse}); rect(0, 0, w + 1.0f, h + 1.0f); }
    setc(Col{c.r, c.g, c.b, fillA}); rect(0, 0, w, h);
    setc(Col{c.r, c.g, c.b, lineA});
    if (gLite) { outlineRect(w, h, 0.5f, false); glPopMatrix(); return; }
    outlineRect(w, h, state == 1 ? 0.2f : 0.12f, state == 0);
    Col mk = Col{c.r, c.g, c.b, lineA * 0.85f};
    if (kind == TK_GATE) {
        setc(mk); rect(0, -h / 2, 0.5f, 0.5f); rect(0, h / 2, 0.5f, 0.5f);           // gate posts
        for (int i = -1; i <= 1; i++) { glPushMatrix(); glTranslatef(0, i * h * 0.28f, 0); chevron(0.45f); glPopMatrix(); }
    } else if (kind == TK_STOP) {
        setc(mk); rect(w / 2 - 0.2f, 0, 0.4f, h);                                     // "stop before here" bar
        for (int i = -1; i <= 1; i += 2) { glPushMatrix(); glTranslatef(-w * 0.2f, i * h * 0.28f, 0); chevron(0.5f); glPopMatrix(); }
    } else {
        // parking task: ghost of the car in its final position, nose = +x. "R" = enter in reverse.
        setc(Col{c.r, c.g, c.b, lineA * 0.5f});
        outlineRect(4.2f, 1.9f, 0.08f, false);
        setc(mk); glPushMatrix(); glTranslatef(w / 2 - 0.65f, 0, 0); chevron(0.42f); glPopMatrix();   // nose points this way
        glPushMatrix(); glTranslatef(-w / 2 + 0.75f, 0, 0); glRotatef(-it.rot, 0, 0, 1);
        text("R", 0, -0.35f * 3.5f, 0.35f, mk, 1); glPopMatrix();
    }
    if (state == 1 && hold01 > 0) {   // hold-still progress along the near edge
        setc(hex(0x2a313b, 0.8f)); rect(0, h / 2 + 0.45f, w, 0.35f);
        setc(hex(0x7dffb0)); rect(-w / 2 + w * hold01 / 2, h / 2 + 0.45f, w * hold01, 0.35f);
    }
    // number badge, always upright on screen (world-up)
    if (ord >= 0) {
        glRotatef(-it.rot, 0, 0, 1);
        float R = state == 1 ? 0.95f : 0.8f;
        setc(Col{0, 0, 0, 0.55f}); circle(0.08f, 0.1f, R + 0.12f, 20);
        setc(hex(0x161a20, 0.92f)); circle(0, 0, R, 20);
        setc(Col{c.r, c.g, c.b, state == 0 ? 0.6f : 1.f}); ring(0, 0, R - 0.14f, R, 24);
        if (state == 2) { setc(hex(0x3ce07a)); seg(-0.42f, 0.02f, -0.12f, 0.34f, 0.2f); seg(-0.12f, 0.34f, 0.45f, -0.32f, 0.2f); }
        else { float px = 0.2f; string n = fmt("%d", ord + 1); text(n, 0, -3.5f * px, px, state == 0 ? hex(0xffffff, 0.7f) : c, 1); }
    }
    glPopMatrix();
}
static void drawLevel(const Level& L, bool editor, bool spotActive, bool showGrid, bool trees = true, bool cones = true) {
    // view culling range (in world units)
    float vx0 = std::max(0.f, gView.x0), vy0 = std::max(0.f, gView.y0);
    float vx1 = std::min(L.W, gView.x1), vy1 = std::min(L.H, gView.y1);
    if (!gLite) { setc(Col{0, 0, 0, 0.35f}); rectTL(-0.2f, 0.4f, L.W + 0.8f, L.H + 0.8f); }
    setc(hex(0x394049)); rectTL(0, 0, L.W, L.H);
    if (!gLite) {
        setc(hex(0x3e454e));
        int tx0 = std::max(0, (int)(vx0 / 4) * 4), ty0 = std::max(0, (int)(vy0 / 4) * 4);
        for (int x = tx0; x < (int)std::ceil(vx1); x += 4) for (int y = ty0; y < (int)std::ceil(vy1); y += 4)
            if (((x + y) / 4) % 2) rectTL((float)x, (float)y, std::min(4.f, L.W - x), std::min(4.f, L.H - y));
        // asphalt speckle, hashed per 4x4 tile so huge maps only draw what is visible
        if (gZoom > 5.f) {
            setc(Col{0, 0, 0, 0.08f});
            glBegin(GL_QUADS);
            for (int x = tx0; x < (int)std::ceil(vx1); x += 4) for (int y = ty0; y < (int)std::ceil(vy1); y += 4) {
                unsigned s = hsh((unsigned)x * 73856093u ^ (unsigned)y * 19349663u);
                for (int i = 0; i < 13; i++) {
                    s = s * 1664525u + 1013904223u; float px = x + ((s >> 8) & 0xFFFF) / 65535.f * 4.f;
                    s = s * 1664525u + 1013904223u; float py = y + ((s >> 8) & 0xFFFF) / 65535.f * 4.f;
                    if (px > L.W - 0.15f || py > L.H - 0.15f) continue;
                    glVertex2f(px, py); glVertex2f(px + 0.12f, py); glVertex2f(px + 0.12f, py + 0.12f); glVertex2f(px, py + 0.12f);
                }
            }
            glEnd();
        }
    }
    if (showGrid) {
        int step = gZoom < 6 ? 5 : 1;
        glLineWidth(1); glBegin(GL_LINES);
        for (int x = (int)(vx0 / step) * step; x <= (int)vx1; x += step) { setc(x % 5 ? hex(0xffffff, 0.05f) : hex(0xffffff, 0.14f)); glVertex2f((float)x, 0); glVertex2f((float)x, L.H); }
        for (int y = (int)(vy0 / step) * step; y <= (int)vy1; y += step) { setc(y % 5 ? hex(0xffffff, 0.05f) : hex(0xffffff, 0.14f)); glVertex2f(0, (float)y); glVertex2f(L.W, (float)y); }
        glEnd();
    }
    // striped curb around the lot
    float t = 0.5f;
    for (float x = -t; x < L.W + t; x += 2) {
        float w = std::min(2.f, L.W + t - x);
        setc(((int)((x + t) / 2)) % 2 ? hex(0xe9edf3) : hex(0x9aa5b4));
        rectTL(x, -t, w, t); rectTL(x, L.H, w, t);
    }
    for (float y = 0; y < L.H; y += 2) {
        float h = std::min(2.f, L.H - y);
        setc(((int)(y / 2)) % 2 ? hex(0xe9edf3) : hex(0x9aa5b4));
        rectTL(-t, y, t, h); rectTL(L.W, y, t, h);
    }
    for (int pass = 0; pass < 4; pass++) {
        int ord = 0;
        for (auto& it : L.items) {
            int ty = it.type;
            if (ty == T_TASK) { if (pass == 1 && editor) drawTaskZone(it, ord, 3); ord++; continue; }
            bool zoneT = isZone(ty);
            bool flat = ty == T_LINE || ty == T_BAY || ty == T_SPOT || ty == T_CROSSING || ty == T_ACCESSIBLE || ty == T_ENTRANCE ||
                        ty == T_CTRLLINE || ty == T_STOPLINE || ty == T_ARCLINE;
            bool solid = ty == T_WALL || ty == T_BARRIER || ty == T_PCAR || (ty == T_CONE && cones) || ty == T_START ||
                         ty == T_CURB || ty == T_ARCCURB || ty == T_ISLAND;
            bool tr = ty == T_TREE && trees;
            if ((pass == 0 && zoneT) || (pass == 1 && flat) || (pass == 2 && solid) || (pass == 3 && tr))
                drawItem(it, editor, ty == T_SPOT && spotActive);
        }
    }
}
// world transform: screen centre -> zoom -> rotation -> camera. Also records the visible area.
static float gLift = 0;   // shifts the world centre up (pixels) so the bottom HUD does not cover the car
static void beginWorld(V2 cam, float zoom, V2 shake = {}, float angRad = 0) {
    glPushMatrix();
    glTranslatef(fbw / 2.f + shake.x, fbh / 2.f - gLift + shake.y, 0);
    glScalef(zoom, zoom, 1);
    if (angRad != 0) glRotatef(deg(angRad), 0, 0, 1);
    glTranslatef(-cam.x, -cam.y, 0);
    float hx = fbw / 2.f / zoom + 2, hy = (fbh / 2.f + gLift) / zoom + 2;
    if (angRad != 0) { float r = std::sqrt(hx * hx + hy * hy); hx = hy = r; }
    gView = {cam.x - hx, cam.y - hy, cam.x + hx, cam.y + hy};
    gZoom = zoom;
}
static V2 toWorld(V2 cam, float zoom) { return {(float)((in.mx - fbw / 2.0) / zoom + cam.x), (float)((in.my - fbh / 2.0) / zoom + cam.y)}; }
