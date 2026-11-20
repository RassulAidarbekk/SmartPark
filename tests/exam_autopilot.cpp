// Headless autopilot test for the AUTODROM EXAM (v0.3 tasks).
// Uses the game's own car physics, collisions, exam rules and task logic: main.cpp is included with its
// main() renamed, and no window is opened. An autopilot drives the whole exam (hill start, stop lines,
// reverse box, parallel park, slalom gates, final spot) and must pass with 0 penalty points; then a few
// rule checks run (final spot locked until all tasks are done, box must be entered in reverse, etc.).
//   ./smartpark_exam_test            run everything (CTest)
//   ./smartpark_exam_test checks     rule checks only
#define main smartpark_main
#include "../main.cpp"
#undef main

static float T = 0;
static int examIdx() { for (size_t i = 0; i < CAREER.size(); i++) if (CAREER[i].name == "AUTODROM EXAM") return (int)i; return -1; }
static bool verbose = true;
static Ctl ctl;
static float lastPrint = -1;

static void step(float dt = 1.f / 60.f) {
    T += dt; pClock += dt; pTime += dt;
    contacts.clear();
    int n = 2;
    for (int i = 0; i < n; i++) stepCar(car, dt / n, ctl);
    for (auto& c : contacts) { penalize(string("COLLISION: ") + KINDNAME[c.kind], 100); break; }
    if (pFailed) return;
    examChecks(dt);
    if (pFailed) return;
    updateTasks(dt);
    // parking test (copy of updatePlay's exam branch)
    OBB cb{car.p.x, car.p.y, car.L / 2, car.W / 2, car.a};
    V2 cs[4]; corners(cb, cs);
    bool all = true;
    for (auto& c : cs) { V2 l = rot(c - V2{wld.spot.x, wld.spot.y}, -rad(wld.spot.rot)); if (std::fabs(l.x) > wld.spot.w / 2 + 0.2f || std::fabs(l.y) > wld.spot.h / 2 + 0.2f) all = false; }
    pInSpot = all;
    if (!wld.tasksLeft() && all && std::fabs(car.vf) < 0.05f && car.gear == G_P) pDone = true;
}
static float hdg() { return deg(std::remainder(car.a, 2 * PI)); }
static float wrapDeg(float a) { return deg(std::remainder(rad(a), 2 * PI)); }
static void setWheel(float w) { hudWheelDrag = true; hudWheelTarget = clampf(w, -WHEEL_MAX, WHEEL_MAX); }
static bool dead() { return pFailed || T > 900; }
static void speedCtl(float target) {  // target signed m/s along the car
    float v = car.vf;
    ctl.gas = false; ctl.brake = false;
    if (target == 0) { ctl.brake = true; return; }
    if (std::fabs(v) < std::fabs(target) - 0.15f && v * target >= -0.05f) ctl.gas = true;
    else if (std::fabs(v) > std::fabs(target) + 0.3f || v * target < -0.05f) ctl.brake = true;
}
static void stopCar(float hold = 0.3f) {
    float t0 = -1;
    while (!dead()) {
        speedCtl(0); step();
        if (std::fabs(car.vf) < 0.01f) { if (t0 < 0) t0 = T; if (T - t0 >= hold) break; }
    }
}
static void shift(int g) { stopCar(0.1f); car.gear = g; }
// follow the line through P with heading thDeg; forward (dir=+1) or reverse (dir=-1) until cond() is true
template <class F> static void follow(V2 P, float thDeg, float speed, F cond) {
    while (!dead() && !cond()) {
        float th = rad(thDeg);
        V2 d = {std::cos(th), std::sin(th)}, n = {-std::sin(th), std::cos(th)};
        float e = dot(car.p - P, n);                       // + = car is right of the line
        float he = std::remainder(car.a - th, 2 * PI);
        (void)d;
        float w = speed >= 0 ? -(50.f * clampf(e, -3, 3) + 320.f * he)      // forward: steer back onto the line
                             : 420.f * (he - 0.2f * clampf(e, -2, 2));       // reverse: aim the rear at the line
        setWheel(w);
        speedCtl(speed); step();
    }
}
// hold a fixed steering wheel angle while driving at speed until cond() is true
template <class F> static void arcDrive(float wheel, float speed, F cond) {
    while (!dead() && !cond()) { setWheel(wheel); speedCtl(speed); step(); }
}
static void report(const char* what) {
    if (!verbose) return;
    printf("[%6.1fs] %-34s pos=(%6.2f,%6.2f) hdg=%7.1f task=%zu/%zu pen=%d %s\n", T, what, car.p.x, car.p.y, hdg(), wld.taskIdx,
           wld.tasks.size(), penTotal, pFailed ? "FAILED" : "");
    for (auto& p : pens) printf("            penalty +%d %s\n", p.pts, p.msg.c_str());
    pens.clear();
}

struct Params { float boxX = 109, boxLane = 88.6f, parY = 42.f, parLane = 143.4f, parAng = 52.f, parStart = 44.0f, finLane = 10.0f, finY = 47.5f; };
static bool runExam(const Params& P) {
    T = 0;
    startPlay(CAREER[examIdx()], PM_CAREER, examIdx(), "");
    belt = true; sigL = true;
    shift(G_D);
    // 1) hill start
    follow({0, 92}, 0, 3.0f, [] { return car.p.x >= 30.4f; });
    stopCar(2.0f); report("hill stop");
    follow({0, 92}, 0, 3.0f, [] { return car.p.x >= 54.8f; });
    stopCar(1.0f); report("crossing stop line");
    // 2) reverse box
    follow({0, P.boxLane}, 0, 3.0f, [&] { return car.p.x >= P.boxX; });
    shift(G_R);
    arcDrive(-WHEEL_MAX, -1.2f, [] { return hdg() >= 80.f; });
    follow({105, 0}, 90, -1.0f, [] { return car.p.y <= 83.15f; });
    stopCar(2.0f); report("reverse box");
    if (wld.taskIdx < 2) return false;
    shift(G_D);
    follow({105, 0}, 90, 1.2f, [] { return car.p.y >= 87.4f; });          // straight out of the box first
    arcDrive(-WHEEL_MAX, 1.5f, [] { return hdg() <= 10.f; });
    follow({0, 92}, 0, 3.5f, [] { return car.p.x >= 136.5f; });
    // left turn onto the right road (heading -90)
    arcDrive(-WHEEL_MAX * 0.6f, 2.0f, [] { return hdg() <= -80.f; });
    follow({142, 0}, -90, 3.0f, [] { return car.p.y <= 75.4f; });
    stopCar(1.0f); report("junction stop line");
    // 3) parallel park: pull alongside, reverse right lock, then left lock
    follow({P.parLane, 0}, -90, 3.0f, [&] { return car.p.y <= P.parY; });
    shift(G_R);
    follow({P.parLane, 0}, -90, -1.0f, [&] { return car.p.y >= P.parStart; });   // rear bumper level with the front car's
    float a1 = -90.f - P.parAng;
    arcDrive(WHEEL_MAX, -1.0f, [&] { return hdg() <= a1; });
    arcDrive(-WHEEL_MAX, -1.0f, [] { return hdg() >= -90.5f || car.p.y > 52.6f; });
    stopCar(0.2f);
    // straighten / centre if needed
    follow({146.3f, 0}, -90, 0.8f, [] { return car.p.y <= 50.3f; });
    stopCar(2.0f); report("parallel park");
    if (wld.taskIdx < 3) return false;
    // pull out: back up a little, then left lock forward
    shift(G_R); follow({146.3f, 0}, -90, -0.8f, [] { return car.p.y >= 51.4f; });
    shift(G_D);
    arcDrive(-WHEEL_MAX, 1.2f, [] { return car.p.x <= 144.0f || hdg() <= -125.f; });
    follow({142, 0}, -90, 3.0f, [] { return car.p.y <= 13.f; });
    arcDrive(-WHEEL_MAX * 0.6f, 2.0f, [] { return std::fabs(wrapDeg(hdg() - 180.f)) < 12.f; });
    // 4..7) slalom gates: weave on a cosine path
    while (!dead() && car.p.x > 50.f) {
        float la = 4.5f, tx = car.p.x - la;
        float ty = 8.f - 3.f * std::cos(PI * (116.f - tx) / 18.f);
        if (tx > 116.f) ty = 5.f;
        if (tx < 62.f) ty = 11.f;
        V2 to = V2{tx, ty} - car.p;
        float he = std::remainder(std::atan2(to.y, to.x) - car.a, 2 * PI);
        setWheel(deg(he) * 9.f);
        speedCtl(2.2f); step();
    }
    report("slalom");
    follow({0, 8}, 180, 3.0f, [] { return car.p.x <= 13.f; });
    arcDrive(-WHEEL_MAX * 0.6f, 2.0f, [] { return std::fabs(wrapDeg(hdg() - 90.f)) < 12.f; });
    follow({8, 0}, 90, 3.0f, [] { return car.p.y >= 22.6f; });
    stopCar(1.0f); report("crossing stop line (left road)");
    // final bay: left turn into the spot
    follow({P.finLane, 0}, 90, 2.0f, [&] { return car.p.y >= P.finY; });
    arcDrive(-WHEEL_MAX, 1.2f, [] { return hdg() <= 4.f; });
    follow({0, 50}, 0, 1.0f, [] { return car.p.x >= 18.2f; });
    stopCar(0.5f); car.gear = G_P;
    for (int i = 0; i < 30; i++) step();
    report("final spot");
    return pDone && !pFailed;
}

static void place(float x, float y, float hdgDeg) { car.p = {x, y}; car.a = rad(hdgDeg); car.v = {0, 0}; car.vf = 0; startPos = car.p; }
static int checks() {
    int bad = 0;
    auto expect = [&](bool c, const char* what) { printf("  [%s] %s\n", c ? "ok" : "FAIL", what); if (!c) bad++; };
    // 1) final spot is locked while tasks remain
    T = 0; startPlay(CAREER[examIdx()], PM_CAREER, examIdx(), ""); belt = true; sigL = true;
    place(18.4f, 50, 0); car.gear = G_P; for (int i = 0; i < 60; i++) { speedCtl(0); step(); }
    expect(pInSpot && !pDone, "parked in the final spot with tasks left -> NOT finished");
    wld.taskIdx = wld.tasks.size(); for (int i = 0; i < 10; i++) step();
    expect(pDone, "same spot after all tasks done -> finished");
    // 2) box entered nose-first does not count
    T = 0; startPlay(CAREER[examIdx()], PM_CAREER, examIdx(), ""); belt = true; sigL = true; wld.taskIdx = 1;
    place(105, 91, -90); car.gear = G_D;
    follow({105, 0}, -90, 1.0f, [] { return car.p.y <= 83.0f; }); stopCar(2.5f);
    expect(wld.taskIdx == 1 && wld.taskHint.find("REVERSE") != string::npos, "box entered forwards -> not done, hint says reverse");
    printf("        hint: %s | collisions/penalties: %d\n", wld.taskHint.c_str(), penTotal);
    // 3) out of order: driving through gate 4 while task 1 is current does nothing
    T = 0; startPlay(CAREER[examIdx()], PM_CAREER, examIdx(), ""); belt = true; sigL = true;
    place(66, 11, 180); car.gear = G_D;
    follow({0, 11}, 180, 2.0f, [] { return car.p.x <= 56.f; });
    expect(wld.taskIdx == 0, "gate 4 passed while on task 1 -> ignored");
    // 4) stop line still penalises a car that drives straight through
    T = 0; startPlay(CAREER[examIdx()], PM_CAREER, examIdx(), ""); belt = true; sigL = true;
    place(50, 92, 0); car.gear = G_D;
    follow({0, 92}, 0, 3.0f, [] { return car.p.x >= 70.f; });
    bool hit = false; for (auto& p : pens) if (p.msg == "DID NOT STOP AT STOP LINE") hit = true;
    expect(hit, "drove through the crossing stop line -> 25 points");
    printf("%s\n", bad ? "SOME CHECKS FAILED" : "ALL CHECKS PASSED");
    return bad;
}
int smartpark_test(int argc, char** argv) {
    buildCareer();
    cfg.exam = true; cfg.center = 1;
    Params P;
    if (examIdx() < 0) { printf("AUTODROM EXAM level not found\n"); return 1; }
    if (argc > 1 && string(argv[1]) == "checks") return checks();
    bool ok = runExam(P);
    printf("\nAUTOPILOT: %s  tasks %zu/%zu  penalty %d  time %.1fs\n\n", ok ? "EXAM PASSED" : "NOT PASSED", wld.taskIdx, wld.tasks.size(), penTotal, pTime);
    int bad = checks();
    return ok && bad == 0 ? 0 : 1;
}
int main(int argc, char** argv) { return smartpark_test(argc, argv); }
