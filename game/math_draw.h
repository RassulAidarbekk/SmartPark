// SmartPark - math_draw.h
// math helpers, colors, 2D drawing primitives, bitmap font
// author: Rasul
// NOTE: not a standalone header - included only from main.cpp, in a fixed order.
#pragma once

// ----------------------------------------------------------------------------
//  Math helpers
// ----------------------------------------------------------------------------
static const float PI = 3.14159265358979f;
static float rad(float d) { return d * PI / 180.f; }
static float deg(float r) { return r * 180.f / PI; }
struct V2 { float x = 0, y = 0; };
static V2 operator+(V2 a, V2 b) { return {a.x + b.x, a.y + b.y}; }
static V2 operator-(V2 a, V2 b) { return {a.x - b.x, a.y - b.y}; }
static V2 operator*(V2 a, float s) { return {a.x * s, a.y * s}; }
static float dot(V2 a, V2 b) { return a.x * b.x + a.y * b.y; }
static float vlen(V2 a) { return std::sqrt(dot(a, a)); }
static V2 rot(V2 a, float ang) {
    float c = std::cos(ang), s = std::sin(ang);
    return {a.x * c - a.y * s, a.x * s + a.y * c};
}
static float clampf(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }
static float sgn(float v) { return v > 0 ? 1.f : (v < 0 ? -1.f : 0.f); }
static string fmt(const char* f, ...) {
    char buf[256];
    va_list ap;
    va_start(ap, f);
    vsnprintf(buf, sizeof buf, f, ap);
    va_end(ap);
    return buf;
}
static unsigned hsh(unsigned x) {
    x ^= x >> 16; x *= 0x7feb352dU; x ^= x >> 15; x *= 0x846ca68bU; x ^= x >> 16;
    return x;
}
static unsigned rngS = 12345;
static float frand() { rngS = rngS * 1664525u + 1013904223u; return ((rngS >> 8) & 0xFFFF) / 65535.f; }
static float frands() { return frand() * 2.f - 1.f; }
// is angle (deg) inside the arc that starts at `start` and sweeps `sweep` degrees clockwise (screen space)
static bool angleIn(float a, float start, float sweep) {
    if (sweep >= 359.9f) return true;
    float d = std::fmod(a - start + 1440.f, 360.f);
    return d <= sweep;
}

// ----------------------------------------------------------------------------
//  Colors + 2D drawing primitives
// ----------------------------------------------------------------------------
struct Col { float r, g, b, a; };
static Col hex(unsigned h, float a = 1.f) {
    return {((h >> 16) & 255) / 255.f, ((h >> 8) & 255) / 255.f, (h & 255) / 255.f, a};
}
static Col shade(Col c, float k) { return {std::min(1.f, c.r * k), std::min(1.f, c.g * k), std::min(1.f, c.b * k), c.a}; }
static float gAlpha = 1.f;   // global alpha multiplier (used for editor ghosts)
static void setc(Col c) { glColor4f(c.r, c.g, c.b, c.a * gAlpha); }

static void rect(float cx, float cy, float w, float h) {
    glBegin(GL_QUADS);
    glVertex2f(cx - w / 2, cy - h / 2); glVertex2f(cx + w / 2, cy - h / 2);
    glVertex2f(cx + w / 2, cy + h / 2); glVertex2f(cx - w / 2, cy + h / 2);
    glEnd();
}
static void rectTL(float x, float y, float w, float h) { rect(x + w / 2, y + h / 2, w, h); }
static void gradV(float x, float y, float w, float h, Col top, Col bot) {
    glBegin(GL_QUADS);
    glColor4f(top.r, top.g, top.b, top.a * gAlpha); glVertex2f(x, y); glVertex2f(x + w, y);
    glColor4f(bot.r, bot.g, bot.b, bot.a * gAlpha); glVertex2f(x + w, y + h); glVertex2f(x, y + h);
    glEnd();
}
static void gradH(float x, float y, float w, float h, Col l, Col r) {
    glBegin(GL_QUADS);
    glColor4f(l.r, l.g, l.b, l.a * gAlpha); glVertex2f(x, y); glVertex2f(x, y + h);
    glColor4f(r.r, r.g, r.b, r.a * gAlpha); glVertex2f(x + w, y + h); glVertex2f(x + w, y);
    glEnd();
}
static void rrect(float cx, float cy, float w, float h, float r, int seg = 5) {
    r = std::min(r, std::min(w, h) / 2);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int k = 0; k < 4; k++) {
        float sx = (k == 0 || k == 3) ? 1.f : -1.f;
        float sy = (k == 0 || k == 1) ? 1.f : -1.f;
        float ox = cx + sx * (w / 2 - r), oy = cy + sy * (h / 2 - r);
        for (int i = 0; i <= seg; i++) {
            float a = rad(k * 90.f + 90.f * i / seg);
            glVertex2f(ox + std::cos(a) * r, oy + std::sin(a) * r);
        }
    }
    glVertex2f(cx + (w / 2), cy + (h / 2 - r));
    glEnd();
}
static void circle(float cx, float cy, float r, int seg = 28) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= seg; i++) {
        float a = 2 * PI * i / seg;
        glVertex2f(cx + std::cos(a) * r, cy + std::sin(a) * r);
    }
    glEnd();
}
// annulus sector: inner radius r0, outer r1, from angle a0 sweeping sw (degrees)
static void arcBand(float cx, float cy, float r0, float r1, float a0, float sw, int seg = 0) {
    if (seg <= 0) seg = std::max(4, (int)(std::fabs(sw) / 6.f));
    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i <= seg; i++) {
        float a = rad(a0 + sw * i / seg), c = std::cos(a), s = std::sin(a);
        glVertex2f(cx + c * r0, cy + s * r0); glVertex2f(cx + c * r1, cy + s * r1);
    }
    glEnd();
}
static void ring(float cx, float cy, float r0, float r1, int seg = 40) { arcBand(cx, cy, r0, r1, 0, 360, seg); }
// soft radial glow: opaque at the centre, transparent at the rim
static void glow(float cx, float cy, float r, Col c, int seg = 24) {
    glBegin(GL_TRIANGLE_FAN);
    glColor4f(c.r, c.g, c.b, c.a * gAlpha); glVertex2f(cx, cy);
    glColor4f(c.r, c.g, c.b, 0.f);
    for (int i = 0; i <= seg; i++) {
        float a = 2 * PI * i / seg;
        glVertex2f(cx + std::cos(a) * r, cy + std::sin(a) * r);
    }
    glEnd();
}
// light beam: bright at the lamp, fading out with distance
static void beam(float x, float y, float len, float spread, Col c) {
    glBegin(GL_TRIANGLES);
    glColor4f(c.r, c.g, c.b, c.a * gAlpha); glVertex2f(x, y);
    glColor4f(c.r, c.g, c.b, 0.f);
    glVertex2f(x + len, y - spread); glVertex2f(x + len, y + spread);
    glEnd();
}
static void tri(float x1, float y1, float x2, float y2, float x3, float y3) {
    glBegin(GL_TRIANGLES); glVertex2f(x1, y1); glVertex2f(x2, y2); glVertex2f(x3, y3); glEnd();
}
static void star(float cx, float cy, float R, bool filled) {
    if (filled) { setc(hex(0xffc83d, 0.25f)); circle(cx, cy, R * 1.35f, 20); }
    setc(filled ? hex(0xffc83d) : hex(0x4a5260));
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= 10; i++) {
        float r = (i % 2 == 0) ? R : R * 0.45f;
        float a = -PI / 2 + i * PI / 5;
        glVertex2f(cx + std::cos(a) * r, cy + std::sin(a) * r);
    }
    glEnd();
    if (filled) {
        setc(hex(0xfff0b0));
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(cx, cy);
        for (int i = 0; i <= 10; i++) {
            float r = ((i % 2 == 0) ? R : R * 0.45f) * 0.55f;
            float a = -PI / 2 + i * PI / 5;
            glVertex2f(cx + std::cos(a) * r, cy - R * 0.08f + std::sin(a) * r);
        }
        glEnd();
    }
}

// ----------------------------------------------------------------------------
//  Tiny built-in 5x7 bitmap font (uppercase only)
// ----------------------------------------------------------------------------
struct Glyph { char c; unsigned char r[7]; };
static const Glyph FONT[] = {
    {' ', {0, 0, 0, 0, 0, 0, 0}},
    {'0', {0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E}}, {'1', {0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E}},
    {'2', {0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F}}, {'3', {0x1E, 0x01, 0x01, 0x0E, 0x01, 0x01, 0x1E}},
    {'4', {0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02}}, {'5', {0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E}},
    {'6', {0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E}}, {'7', {0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08}},
    {'8', {0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E}}, {'9', {0x0E, 0x11, 0x11, 0x0F, 0x01, 0x02, 0x0C}},
    {'A', {0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11}}, {'B', {0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E}},
    {'C', {0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E}}, {'D', {0x1E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1E}},
    {'E', {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F}}, {'F', {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10}},
    {'G', {0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0F}}, {'H', {0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11}},
    {'I', {0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E}}, {'J', {0x07, 0x02, 0x02, 0x02, 0x02, 0x12, 0x0C}},
    {'K', {0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11}}, {'L', {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F}},
    {'M', {0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11}}, {'N', {0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11}},
    {'O', {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}}, {'P', {0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10}},
    {'Q', {0x0E, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0D}}, {'R', {0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11}},
    {'S', {0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E}}, {'T', {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04}},
    {'U', {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}}, {'V', {0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04}},
    {'W', {0x11, 0x11, 0x11, 0x15, 0x15, 0x1B, 0x11}}, {'X', {0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11}},
    {'Y', {0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04}}, {'Z', {0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F}},
    {'-', {0, 0, 0, 0x1F, 0, 0, 0}}, {'.', {0, 0, 0, 0, 0, 0x0C, 0x0C}}, {':', {0, 0x0C, 0x0C, 0, 0x0C, 0x0C, 0}},
    {'/', {0x01, 0x01, 0x02, 0x04, 0x08, 0x10, 0x10}}, {'!', {0x04, 0x04, 0x04, 0x04, 0x04, 0, 0x04}},
    {'?', {0x0E, 0x11, 0x01, 0x02, 0x04, 0, 0x04}}, {'+', {0, 0x04, 0x04, 0x1F, 0x04, 0x04, 0}},
    {'(', {0x02, 0x04, 0x08, 0x08, 0x08, 0x04, 0x02}}, {')', {0x08, 0x04, 0x02, 0x02, 0x02, 0x04, 0x08}},
    {'%', {0x18, 0x19, 0x02, 0x04, 0x08, 0x13, 0x03}}, {'=', {0, 0, 0x1F, 0, 0x1F, 0, 0}},
    {'>', {0x10, 0x08, 0x04, 0x02, 0x04, 0x08, 0x10}}, {'<', {0x02, 0x04, 0x08, 0x10, 0x08, 0x04, 0x02}},
    {',', {0, 0, 0, 0, 0x0C, 0x04, 0x08}}, {'_', {0, 0, 0, 0, 0, 0, 0x1F}},
    {'&', {0x0C, 0x12, 0x14, 0x08, 0x15, 0x12, 0x0D}},
    {'[', {0x0E, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0E}}, {']', {0x0E, 0x02, 0x02, 0x02, 0x02, 0x02, 0x0E}},
    {'\'', {0x04, 0x04, 0x08, 0, 0, 0, 0}}, {';', {0, 0x0C, 0x0C, 0, 0x0C, 0x04, 0x08}},
    {'\\', {0x10, 0x10, 0x08, 0x04, 0x02, 0x01, 0x01}}, {'`', {0x08, 0x04, 0, 0, 0, 0, 0}},
    {'*', {0, 0x04, 0x15, 0x0E, 0x15, 0x04, 0}}, {'#', {0x0A, 0x0A, 0x1F, 0x0A, 0x1F, 0x0A, 0x0A}},
};
static float textW(const string& s, float px) { return s.empty() ? 0 : s.size() * 6 * px - px; }
static void text(const string& s, float x, float y, float px, Col c, int align = 0) {
    if (align == 1) x -= textW(s, px) / 2;
    if (align == 2) x -= textW(s, px);
    setc(c);
    glBegin(GL_QUADS);
    for (char ch : s) {
        ch = (char)std::toupper((unsigned char)ch);
        for (const Glyph& g : FONT) {
            if (g.c != ch) continue;
            for (int row = 0; row < 7; row++)
                for (int col = 0; col < 5; col++)
                    if (g.r[row] & (1 << (4 - col))) {
                        float qx = x + col * px, qy = y + row * px;
                        glVertex2f(qx, qy); glVertex2f(qx + px, qy);
                        glVertex2f(qx + px, qy + px); glVertex2f(qx, qy + px);
                    }
            break;
        }
        x += 6 * px;
    }
    glEnd();
}
// text with a soft drop shadow
static void textS(const string& s, float x, float y, float px, Col c, int align = 0) {
    text(s, x + px * 0.6f, y + px * 0.8f, px, Col{0, 0, 0, 0.45f * c.a}, align);
    text(s, x, y, px, c, align);
}
