// SmartPark - physics.h
// collision world, car physics, play state, particles, penalty bookkeeping
// author: Zhalgas
// NOTE: not a standalone header - included only from main.cpp, in a fixed order.
#pragma once

// ----------------------------------------------------------------------------
//  Collision world + car physics
// ----------------------------------------------------------------------------
enum Kind { K_FENCE, K_WALL, K_BARRIER, K_CAR, K_CURB, K_TREE, K_CONE };
static const char* KINDNAME[] = {"BOUNDARY FENCE", "WALL", "BARRIER", "PARKED CAR", "CURB", "TREE", "CONE"};
struct OBB { float cx, cy, hx, hy, a; int kind = K_WALL; };
struct Circ { float x, y, r; int kind; };
struct ConeObj { V2 p, v; float rot = 0, vr = 0; bool knocked = false; };
struct LineState { bool inside = false; };
struct StopState { bool stopped = false, pastPen = false; float stillT = 0, pastT = 0, prevLx = -99; };
struct World {
    vector<OBB> boxes; vector<Circ> circs; vector<ConeObj> cones; vector<Item> zones;
    vector<Item> lines; vector<LineState> lineSt;          // solid control lines (straight + arc)
    vector<Item> stops; vector<StopState> stopSt;
    vector<Item> crossings; vector<float> crossStill; vector<bool> crossPen;
    Item start{}, spot{}; bool hasStart = false, hasSpot = false;
    // ordered tasks
    vector<Item> tasks; size_t taskIdx = 0;
    float taskHold = 0;          // seconds stood still in the current task
    int lastDir = 0;             // +1 last moved forward, -1 last moved backward
    int dirChanges = 0;          // forward/reverse switches near the current parking task
    bool corrPen = false;
    bool centreIn = false; int enteredDir = 0;   // direction of travel when the car's centre entered the current task zone
    string taskHint;             // live feedback for the current task ("MUST REVERSE IN" ...)
    bool tasksLeft() const { return taskIdx < tasks.size(); }
};
static void coneArcPositions(float cx, float cy, float r, float a0, float sw, float spacing, vector<V2>& out) {
    out.clear();
    float len = rad(std::fabs(sw)) * r;
    bool full = sw >= 359.9f;
    int n = std::max(1, (int)std::round(len / spacing));
    int cnt = full ? n : n + 1;
    for (int i = 0; i < cnt; i++) {
        float a = rad(a0 + sw * i / n);
        out.push_back({cx + std::cos(a) * r, cy + std::sin(a) * r});
    }
}
static void coneArc(Level& L, float cx, float cy, float r, float a0, float sw, float spacing) {
    vector<V2> ps; coneArcPositions(cx, cy, r, a0, sw, spacing, ps);
    for (auto& p : ps) cone(L, p.x, p.y);
}
static void addArcBoxes(World& w, const Item& it, float th, int kind) {
    float r = it.w, sw = it.h;
    float len = rad(sw) * r;
    int n = std::max(2, (int)std::ceil(len / 0.8f));
    for (int i = 0; i < n; i++) {
        float a0 = rad(it.rot + sw * i / n), a1 = rad(it.rot + sw * (i + 1) / n);
        V2 p0 = {it.x + std::cos(a0) * r, it.y + std::sin(a0) * r}, p1 = {it.x + std::cos(a1) * r, it.y + std::sin(a1) * r};
        V2 m = (p0 + p1) * 0.5f, d = p1 - p0;
        w.boxes.push_back({m.x, m.y, vlen(d) / 2 + 0.03f, th / 2, std::atan2(d.y, d.x), kind});
    }
}
static World buildWorld(const Level& L) {
    World w;
    float t = 6;
    w.boxes.push_back({-t / 2, L.H / 2, t / 2, L.H / 2 + t, 0, K_FENCE});
    w.boxes.push_back({L.W + t / 2, L.H / 2, t / 2, L.H / 2 + t, 0, K_FENCE});
    w.boxes.push_back({L.W / 2, -t / 2, L.W / 2 + t, t / 2, 0, K_FENCE});
    w.boxes.push_back({L.W / 2, L.H + t / 2, L.W / 2 + t, t / 2, 0, K_FENCE});
    for (auto& it : L.items) {
        switch (it.type) {
            case T_WALL: w.boxes.push_back({it.x, it.y, it.w / 2, it.h / 2, rad(it.rot), K_WALL}); break;
            case T_BARRIER: w.boxes.push_back({it.x, it.y, it.w / 2, it.h / 2, rad(it.rot), K_BARRIER}); break;
            case T_PCAR: w.boxes.push_back({it.x, it.y, it.w / 2, it.h / 2, rad(it.rot), K_CAR}); break;
            case T_CURB: w.boxes.push_back({it.x, it.y, it.w / 2, it.h / 2, rad(it.rot), K_CURB}); break;
            case T_ARCCURB: addArcBoxes(w, it, CURB_T, K_CURB); break;
            case T_ISLAND: w.circs.push_back({it.x, it.y, it.w * 0.5f, K_CURB}); break;
            case T_CONE: { ConeObj c; c.p = {it.x, it.y}; c.rot = it.rot; w.cones.push_back(c); break; }
            case T_TREE: w.circs.push_back({it.x, it.y, it.w * 0.45f, K_TREE}); break;
            case T_MUD: case T_ICE: case T_GRASS: case T_HILL: w.zones.push_back(it); break;
            case T_CTRLLINE: w.lines.push_back(it); break;
            case T_ARCLINE: if (it.color == 0) w.lines.push_back(it); break;
            case T_STOPLINE: w.stops.push_back(it); break;
            case T_CROSSING: w.crossings.push_back(it); break;
            case T_START: w.start = it; w.hasStart = true; break;
            case T_SPOT: w.spot = it; w.hasSpot = true; break;
            case T_TASK: w.tasks.push_back(it); break;
            default: break;
        }
    }
    w.lineSt.assign(w.lines.size(), {});
    w.stopSt.assign(w.stops.size(), {});
    w.crossStill.assign(w.crossings.size(), 0.f);
    w.crossPen.assign(w.crossings.size(), false);
    return w;
}
static void corners(const OBB& o, V2 out[4]) {
    V2 ax = {std::cos(o.a), std::sin(o.a)}, ay = {-std::sin(o.a), std::cos(o.a)}, c = {o.cx, o.cy};
    out[0] = c + ax * o.hx + ay * o.hy; out[1] = c - ax * o.hx + ay * o.hy;
    out[2] = c - ax * o.hx - ay * o.hy; out[3] = c + ax * o.hx - ay * o.hy;
}
// SAT; n points from b to a, d = penetration depth
static bool sat(const OBB& a, const OBB& b, V2& nOut, float& dOut) {
    V2 ca[4], cb[4];
    corners(a, ca); corners(b, cb);
    V2 axes[4] = {{std::cos(a.a), std::sin(a.a)}, {-std::sin(a.a), std::cos(a.a)},
                  {std::cos(b.a), std::sin(b.a)}, {-std::sin(b.a), std::cos(b.a)}};
    float best = 1e9f; V2 bn;
    for (auto& ax : axes) {
        float mnA = 1e9f, mxA = -1e9f, mnB = 1e9f, mxB = -1e9f;
        for (int i = 0; i < 4; i++) {
            float pa = dot(ca[i], ax), pb = dot(cb[i], ax);
            mnA = std::min(mnA, pa); mxA = std::max(mxA, pa); mnB = std::min(mnB, pb); mxB = std::max(mxB, pb);
        }
        float ov = std::min(mxA, mxB) - std::max(mnA, mnB);
        if (ov <= 0) return false;
        if (ov < best) { best = ov; bn = ax; }
    }
    if (dot(V2{a.cx - b.cx, a.cy - b.cy}, bn) < 0) bn = bn * -1.f;
    nOut = bn; dOut = best;
    return true;
}
static bool pointInItem(const Item& it, V2 p, float margin = 0) {
    V2 l = rot(p - V2{it.x, it.y}, -rad(it.rot));
    return std::fabs(l.x) <= it.w / 2 + margin && std::fabs(l.y) <= it.h / 2 + margin;
}
static bool onArc(const Item& it, V2 p, float halfT) {
    V2 d = p - V2{it.x, it.y};
    if (std::fabs(vlen(d) - it.w) > halfT) return false;
    return angleIn(deg(std::atan2(d.y, d.x)), it.rot, it.h);
}
// editor hit test (used by right-click delete)
static bool itemHit(const Item& it, V2 p) {
    if (it.type == T_ARCCURB || it.type == T_ARCLINE) return onArc(it, p, 0.6f);
    if (it.type == T_ISLAND || it.type == T_TREE) return vlen(p - V2{it.x, it.y}) <= it.w / 2;
    return pointInItem(it, p, it.h < 0.6f || it.w < 0.6f ? 0.3f : 0.f);
}

enum Gear { G_P, G_R, G_N, G_D };
static const char* GEARCH = "PRND";
struct Car {
    V2 p, v; float a = 0;
    float wheel = 0;     // steering-wheel angle, degrees (+ = right / clockwise)
    float steer = 0;     // road-wheel angle, radians
    float gas = 0, brake = 0;
    int gear = G_P;
    float vf = 0, slip = 0;
    float L = 4.2f, W = 1.9f;
    bool braking = false, hb = false;
};
static const float WHEEL_MAX = 540.f, STEER_RATIO = 15.4f, WHEELBASE = 2.6f, AXLE_OFF = 1.3f;

// ---- play state ----
enum Screen { S_MENU, S_CAREER, S_CUSTOM, S_PLAY, S_EDIT, S_SETTINGS };
enum PMode { PM_CAREER, PM_CUSTOM, PM_TEST };
static Screen screen = S_MENU, settingsBack = S_MENU;
static PMode pmode = PM_CAREER;
static int careerIdx = 0;
static string playKey;
static Level cur;
static World wld;
static Car car;
static float pTime = 0, pPark = 0, pHitCd = 0, pShake = 0, pDoneT = 0, pIntro = 0, pClock = 0;
static int pHits = 0, pStars = 0;
static bool pStarted = false, pDone = false, pPaused = false, pInSpot = false, pFailed = false;
static vector<std::pair<V2, float>> marks;
static V2 pCam; static float pCamA = 0, pZoomMul = 1.f;
static float gTime = 0;
static int fbw = 1280, fbh = 720;
struct View { float x0, y0, x1, y1; } gView{0, 0, 1e9f, 1e9f};
static float gZoom = 20; static bool gLite = false;

// exam state
struct Pen { string msg; int pts; float t; };
static vector<Pen> pens; static int penTotal = 0;
static bool sigL = false, sigR = false, sigArmed = false, belt = false;
static float sigT = 0; static bool sigPhase = false;
static bool exBeltPen, exStartChecked, exUsedLeft, exNoMovePen, exTimePen, exGrassPen;
static float exOverT, exNextOver, exWarnT, exRoll; static int exHill;
static V2 startPos;
// mouse-driven HUD controls
static bool hudWheelDrag = false; static float hudWheelLastAng = 0, hudWheelTarget = 0;
static bool hudGas = false, hudBrake = false;

// ---- particles (smoke / sparks in world space, confetti in screen space) ----
struct Part { V2 p, v; float life, max, size; Col c; bool spark; };
struct Conf { V2 p, v; float rot, vr, life; Col c; };
static vector<Part> parts;
static vector<Conf> confetti;
static void emitSmoke(V2 p, V2 v) {
    if (parts.size() > 600) return;
    parts.push_back({p, v + V2{frands() * 0.5f, frands() * 0.5f}, 0.9f, 0.9f, 0.3f, hex(0xdfe5ee, 0.5f), false});
}
static void emitSparks(V2 p, V2 dir, int n) {
    for (int i = 0; i < n && parts.size() < 600; i++) {
        V2 v = dir * (3.f + frand() * 6.f) + V2{frands() * 4.f, frands() * 4.f};
        parts.push_back({p, v, 0.35f + frand() * 0.2f, 0.5f, 0.09f, hex(0xffd25a), true});
    }
}
static void burstConfetti() {
    static const unsigned pal[] = {0xffc83d, 0x38bdf8, 0x7dffb0, 0xff6b8a, 0xffffff, 0xc084fc};
    for (int i = 0; i < 160; i++)
        confetti.push_back({{frand() * fbw, -20.f - frand() * 200.f}, {frands() * 120.f, 80.f + frand() * 220.f},
                            frand() * 360.f, frands() * 360.f, 4.f + frand() * 2.f, hex(pal[i % 6])});
}
static void updateFX(float dt) {
    for (auto& p : parts) {
        p.life -= dt; p.p = p.p + p.v * dt;
        p.v = p.v * (p.spark ? std::max(0.f, 1.f - 2.f * dt) : std::max(0.f, 1.f - 1.5f * dt));
    }
    parts.erase(std::remove_if(parts.begin(), parts.end(), [](const Part& p) { return p.life <= 0; }), parts.end());
    for (auto& c : confetti) {
        c.life -= dt; c.v.y += 90.f * dt; c.v.x *= (1.f - 0.4f * dt);
        c.p = c.p + c.v * dt; c.rot += c.vr * dt;
    }
    confetti.erase(std::remove_if(confetti.begin(), confetti.end(),
                                  [](const Conf& c) { return c.life <= 0 || c.p.y > fbh + 30; }), confetti.end());
    for (auto& c : wld.cones) if (c.knocked) {
        c.p = c.p + c.v * dt; c.rot += c.vr * dt;
        float k = std::exp(-3.2f * dt); c.v = c.v * k; c.vr *= k;
        c.p.x = clampf(c.p.x, 0.3f, cur.W - 0.3f); c.p.y = clampf(c.p.y, 0.3f, cur.H - 0.3f);
    }
}
static void drawParts() {
    for (auto& p : parts) {
        float k = clampf(p.life / p.max, 0, 1);
        if (p.spark) { setc(Col{p.c.r, p.c.g, p.c.b, k}); circle(p.p.x, p.p.y, p.size, 6); }
        else { setc(Col{p.c.r, p.c.g, p.c.b, p.c.a * k}); circle(p.p.x, p.p.y, p.size + (1 - k) * 0.9f, 12); }
    }
}
static void drawConfetti() {
    for (auto& c : confetti) {
        glPushMatrix(); glTranslatef(c.p.x, c.p.y, 0); glRotatef(c.rot, 0, 0, 1);
        setc(c.c); rect(0, 0, 9, 5);
        glPopMatrix();
    }
}
static string toast; static float toastT = 0;
static void showToast(const string& s) { toast = s; toastT = 2.5f; }

static void startPlay(const Level& L, PMode m, int idx, const string& key) {
    cur = L; wld = buildWorld(L); pmode = m; careerIdx = idx; playKey = key;
    car = Car();
    if (wld.hasStart) { car.p = {wld.start.x, wld.start.y}; car.a = rad(wld.start.rot); }
    else { car.p = {4, L.H / 2}; }
    car.gear = cfg.exam ? G_P : G_D;
    startPos = car.p;
    pTime = 0; pPark = 0; pHits = 0; pStars = 0; pHitCd = 0; pShake = 0; pDoneT = 0; pIntro = 0; pClock = 0;
    pStarted = pDone = pPaused = pInSpot = pFailed = false;
    pens.clear(); penTotal = 0;
    sigL = sigR = sigArmed = false; belt = !cfg.exam; sigT = 0; sigPhase = false;
    exBeltPen = exStartChecked = exUsedLeft = exNoMovePen = exTimePen = exGrassPen = false;
    exOverT = 0; exNextOver = 5; exWarnT = 0; exRoll = 0; exHill = -1;
    hudWheelDrag = hudGas = hudBrake = false;
    marks.clear(); parts.clear(); confetti.clear();
    pCam = car.p; pCamA = -PI / 2 - car.a;
    screen = S_PLAY;
}
static void restartPlay() { Level L = cur; startPlay(L, pmode, careerIdx, playKey); }

// ---- exam penalty bookkeeping ----
static void penalize(const string& msg, int pts) {
    if (pDone || pFailed) return;
    pens.push_back({msg, pts, gTime});
    penTotal += pts;
    if (penTotal >= 100) {
        pFailed = true; pDoneT = 0; sfx::play(sfx::FAIL);
        car.v = {0, 0}; car.vf = 0;
    } else sfx::play(sfx::PENALTY);
}

struct Contact { int kind; float speed; };
static vector<Contact> contacts;
static void impact(Car& c, V2 n, int kind) {  // n: from obstacle toward car
    float vn = dot(c.v, n);
    float sp = std::max(0.f, -vn);
    if (vn < 0) {
        c.v = c.v - n * (vn * 1.25f);
        c.v = c.v * 0.93f;
    }
    contacts.push_back({kind, sp});
    if (sp > 1.5f && pHitCd <= 0) emitSparks(c.p - n * 1.0f, n, 10);
}
static void collide(Car& c) {
    for (int iter = 0; iter < 3; iter++) {
        OBB cb{c.p.x, c.p.y, c.L / 2, c.W / 2, c.a};
        for (auto& b : wld.boxes) {
            V2 n; float d;
            if (sat(cb, b, n, d)) { c.p = c.p + n * d; cb.cx = c.p.x; cb.cy = c.p.y; impact(c, n, b.kind); }
        }
        for (auto& ci : wld.circs) {
            V2 loc = rot(V2{ci.x, ci.y} - c.p, -c.a);
            V2 cl = {clampf(loc.x, -c.L / 2, c.L / 2), clampf(loc.y, -c.W / 2, c.W / 2)};
            V2 df = loc - cl;
            float dist = vlen(df);
            if (dist < ci.r) {
                V2 nl = dist > 1e-4f ? df * (1.f / dist) : V2{1, 0};
                float depth = dist > 1e-4f ? ci.r - dist : ci.r + 0.5f;
                V2 nw = rot(nl, c.a);       // car -> circle
                c.p = c.p - nw * depth; cb.cx = c.p.x; cb.cy = c.p.y;
                impact(c, nw * -1.f, ci.kind);
            }
        }
    }
    // cones get knocked over instead of acting as walls
    for (auto& co : wld.cones) {
        if (co.knocked) continue;
        V2 loc = rot(co.p - c.p, -c.a);
        V2 cl = {clampf(loc.x, -c.L / 2, c.L / 2), clampf(loc.y, -c.W / 2, c.W / 2)};
        V2 df = loc - cl;
        float dist = vlen(df);
        if (dist < 0.42f) {
            V2 nl = dist > 1e-4f ? df * (1.f / dist) : V2{1, 0};
            V2 nw = rot(nl, c.a);
            float sp = vlen(c.v);
            co.knocked = true;
            co.v = nw * (1.2f + sp * 1.1f) + c.v * 0.5f;
            co.vr = frands() * 420.f;
            c.v = c.v * 0.96f;
            contacts.push_back({K_CONE, sp});
        }
    }
}
struct Ctl { bool gas, brake, left, right, hb; };
static void stepCar(Car& c, float dt, const Ctl& k) {
    // ---- surface
    float grip = 14.f, roll = 0.12f, brakeEff = 1.f, engMul = 1.f;
    V2 grav = {0, 0};
    for (auto& z : wld.zones) if (pointInItem(z, c.p)) {
        if (z.type == T_MUD) { roll = 2.0f; engMul = 0.6f; }
        if (z.type == T_ICE) { grip = 1.2f; brakeEff = 0.25f; }
        if (z.type == T_GRASS) { roll = 1.0f; grip = 7.f; engMul = 0.8f; }
        if (z.type == T_HILL) { float ua = rad(90.f * (z.color & 3)); grav = grav - V2{std::cos(ua), std::sin(ua)} * HILL_G; }
    }
    // ---- pedals (ramped, like pushing a real pedal)
    auto ramp = [&](float& v, bool on) { v += on ? std::min(1.f - v, 4.f * dt) : -std::min(v, 6.f * dt); };
    ramp(c.gas, k.gas); ramp(c.brake, k.brake);
    c.hb = k.hb;
    // ---- steering wheel
    V2 f = {std::cos(c.a), std::sin(c.a)}, r = {-std::sin(c.a), std::cos(c.a)};
    float vf = dot(c.v, f), vl = dot(c.v, r);
    if (hudWheelDrag) c.wheel = hudWheelTarget;
    else if (k.left != k.right) {
        float dir = k.right ? 1.f : -1.f;
        float rate = (c.wheel * dir < 0) ? 900.f : 620.f;      // unwinding is quicker
        c.wheel += dir * rate * dt;
    } else {
        float rate = cfg.center == 2 ? 520.f : (cfg.center == 1 ? std::min(620.f, 150.f * std::fabs(vf)) : 0.f);
        c.wheel -= clampf(c.wheel, -rate * dt, rate * dt);
    }
    c.wheel = clampf(c.wheel, -WHEEL_MAX, WHEEL_MAX);
    c.steer = rad(c.wheel / STEER_RATIO);
    // ---- longitudinal: automatic gearbox with creep
    int dirG = c.gear == G_D ? 1 : (c.gear == G_R ? -1 : 0);
    float acc = dot(grav, f);
    if (dirG != 0) {
        float vd = vf * dirG, maxV = dirG > 0 ? 14.f : 4.2f;
        // idle creep (~6 km/h on the flat, NOT enough to hold the car on a 16% hill)
        float creep = vd < 1.7f ? 1.25f * (1.f - std::max(0.f, vd) / 1.7f) : 0.f;
        float thr = c.gas * 3.4f * std::max(0.f, 1.f - std::max(0.f, vd) / maxV);
        acc += dirG * (creep + thr) * engMul;
    }
    float engineBrake = (dirG != 0 && c.gas < 0.05f && vf * dirG > 1.8f) ? 0.6f : 0.f;
    float resist = roll + 0.015f * vf * vf + engineBrake + c.brake * 8.f * brakeEff + (c.hb ? 6.f * brakeEff : 0.f) + (c.gear == G_P ? 30.f : 0.f);
    if (std::fabs(vf) < 0.02f) {
        if (std::fabs(acc) <= resist) vf = 0;
        else vf += (acc - sgn(acc) * resist) * dt;
    } else {
        vf += acc * dt;
        float d = resist * dt;
        vf = std::fabs(vf) <= d ? 0.f : vf - sgn(vf) * d;
    }
    c.braking = c.brake > 0.05f || c.hb;
    // ---- lateral grip + rear-axle pivot kinematics (the car turns about its rear axle)
    float g = c.hb ? std::min(grip, 2.5f) : grip;
    vl *= std::exp(-g * dt);
    float steerEff = c.steer / (1.f + 0.004f * vf * vf);   // speed-dependent understeer
    float omega = vf * std::tan(steerEff) / WHEELBASE;
    if (c.hb && std::fabs(vf) > 3.f) omega *= 1.35f;
    c.v = f * vf + r * vl;
    V2 rear = c.p - f * AXLE_OFF;
    rear = rear + c.v * dt;
    c.a += omega * dt;
    c.p = rear + V2{std::cos(c.a), std::sin(c.a)} * AXLE_OFF;
    c.vf = vf; c.slip = std::fabs(vl);
    collide(c);
}
static void wheelPts(const Car& c, V2 out[4]) {
    V2 f = {std::cos(c.a), std::sin(c.a)}, r = {-std::sin(c.a), std::cos(c.a)};
    out[0] = c.p + f * 1.3f + r * 0.92f; out[1] = c.p + f * 1.3f - r * 0.92f;
    out[2] = c.p - f * 1.3f + r * 0.92f; out[3] = c.p - f * 1.3f - r * 0.92f;
}
