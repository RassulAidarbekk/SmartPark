// SmartPark - game_rules.h
// play controls, exam checks, ordered exam tasks, updatePlay
// author: Samandar
// NOTE: not a standalone header - included only from main.cpp, in a fixed order.
#pragma once

// ---- play screen ----
static void leavePlay() {
    if (pmode == PM_TEST) screen = S_EDIT;
    else if (pmode == PM_CUSTOM) { refreshCustoms(); screen = S_CUSTOM; }
    else screen = S_CAREER;
}
static void shiftTo(int g) {
    if (g == car.gear) return;
    if (g != G_N && std::fabs(car.vf) > 0.3f) { showToast("STOP THE CAR BEFORE SHIFTING"); return; }
    car.gear = g; sfx::play(sfx::SHIFT);
}
static void toggleSignal(bool left) {
    if (left) { sigL = !sigL; sigR = false; } else { sigR = !sigR; sigL = false; }
    sigArmed = false; sigT = 0; sigPhase = true; sfx::play(sfx::TICK);
}
static bool carOverlapsItem(const Item& it) {
    OBB a{car.p.x, car.p.y, car.L / 2, car.W / 2, car.a}, b{it.x, it.y, it.w / 2, it.h / 2, rad(it.rot)};
    V2 n; float d;
    return sat(a, b, n, d);
}
// all exam checks for one frame (only when exam rules are on)
static void examChecks(float dt) {
    float spd = std::fabs(car.vf), kmh = spd * 3.6f;
    float moved = vlen(car.p - startPos);
    bool still = spd < 0.05f;
    // seatbelt + start procedure (exercise 1 "start")
    if (moved > 0.3f && !belt && !exBeltPen) { exBeltPen = true; penalize("SEATBELT NOT FASTENED", 5); }
    if (!exStartChecked) {
        if (sigL) exUsedLeft = true;
        if (moved > 1.5f) { exStartChecked = true; if (!exUsedLeft) penalize("NO LEFT SIGNAL WHEN MOVING OFF", 5); }
        else if (pClock > 30.f && !exNoMovePen) { exNoMovePen = true; penalize("DID NOT MOVE OFF WITHIN 30 S", 25); }
    }
    // speed limit 20 km/h: 5 points for each 5 s of speeding
    if (kmh > 20.5f) {
        exOverT += dt; exWarnT -= dt;
        if (exWarnT <= 0) { sfx::play(sfx::WARN); exWarnT = 1.f; }
        if (exOverT >= exNextOver) { exNextOver += 5.f; penalize("SPEED OVER 20 KM/H FOR 5 S", 5); }
    } else exWarnT = 0;
    // total time limit
    if (!exTimePen && pTime > cur.par * 2.f) { exTimePen = true; penalize(fmt("TIME LIMIT %.0f S EXCEEDED", cur.par * 2.f), 50); }
    V2 wp[4]; wheelPts(car, wp);
    // grass = left the roadway
    if (!exGrassPen) for (auto& z : wld.zones) if (z.type == T_GRASS) {
        for (auto& p : wp) if (pointInItem(z, p)) { exGrassPen = true; break; }
        if (exGrassPen) { penalize("LEFT THE ROADWAY", 100); return; }
    }
    // control lines: 20 points when a wheel touches; re-armed only after the whole car has left the line
    V2 cc[4]; { OBB cb{car.p.x, car.p.y, car.L / 2, car.W / 2, car.a}; corners(cb, cc); }
    for (size_t i = 0; i < wld.lines.size(); i++) {
        const Item& ln = wld.lines[i];
        bool wheelOn = false, bodyOn = false;
        if (ln.type == T_ARCLINE) {
            for (auto& p : wp) if (onArc(ln, p, LINE_T / 2 + 0.14f)) wheelOn = true;
            float mn = 1e9f, mx = -1e9f; bool ang = false;
            for (auto& p : cc) {
                V2 d = p - V2{ln.x, ln.y}; float sd = vlen(d) - ln.w;
                mn = std::min(mn, sd); mx = std::max(mx, sd);
                if (angleIn(deg(std::atan2(d.y, d.x)), ln.rot, ln.h)) ang = true;
            }
            bodyOn = ang && mn < 0.3f && mx > -0.3f;
        } else {
            for (auto& p : wp) if (pointInItem(ln, p, 0.14f)) wheelOn = true;
            Item fat = ln; fat.h += 0.6f; fat.w += 0.6f;
            bodyOn = carOverlapsItem(fat);
        }
        if (wheelOn && !wld.lineSt[i].inside) { wld.lineSt[i].inside = true; penalize("WHEEL ON CONTROL LINE", 20); }
        if (!bodyOn && !wheelOn) wld.lineSt[i].inside = false;
    }
    // pedestrian crossing: no stopping on the zebra
    for (size_t i = 0; i < wld.crossings.size(); i++) {
        bool ov = carOverlapsItem(wld.crossings[i]);
        if (ov && still) wld.crossStill[i] += dt; else wld.crossStill[i] = 0;
        if (!ov) wld.crossPen[i] = false;
        if (wld.crossStill[i] > 0.6f && !wld.crossPen[i]) { wld.crossPen[i] = true; penalize("STOPPED ON PEDESTRIAN CROSSING", 25); }
    }
    // stop lines: full stop before the line, don't stop past it
    V2 fwd = {std::cos(car.a), std::sin(car.a)};
    V2 front = car.p + fwd * (car.L / 2);
    for (size_t i = 0; i < wld.stops.size(); i++) {
        const Item& s = wld.stops[i]; StopState& st = wld.stopSt[i];
        V2 l = rot(front - V2{s.x, s.y}, -rad(s.rot));
        bool across = std::fabs(l.y) <= s.h / 2 + 0.6f;
        float edge = -s.w / 2;
        if (across && l.x > -7.f && l.x < edge + 0.05f) {
            if (still) st.stillT += dt; else st.stillT = 0;
            if (st.stillT > 0.4f) st.stopped = true;
        }
        // crossing the line this frame (prevLx must be just before it - entering the road from the side doesn't count)
        if (across && st.prevLx < edge && st.prevLx > edge - 3.f && l.x >= edge && car.vf > 0) {
            if (!st.stopped) penalize("DID NOT STOP AT STOP LINE", 25);
            st.stopped = false;
        }
        if (across && l.x >= edge && l.x < 2.5f && still) st.pastT += dt; else st.pastT = 0;
        if (st.pastT > 0.5f && !st.pastPen) { st.pastPen = true; penalize("STOPPED PAST THE STOP LINE", 25); }
        if (!across || l.x > 4.f || l.x < edge - 1.f) st.pastPen = false;
        st.prevLx = across ? l.x : -99.f;
    }
    // hill: roll-back more than 0.2 m
    int hill = -1;
    for (size_t i = 0; i < wld.zones.size(); i++) if (wld.zones[i].type == T_HILL && pointInItem(wld.zones[i], car.p)) hill = (int)i;
    if (hill != exHill) { exHill = hill; exRoll = 0; }
    if (hill >= 0 && car.gear == G_D) {
        if (car.vf < 0) exRoll += -car.vf * dt;
        else if (car.vf > 0.05f) exRoll = 0;
        if (exRoll > 0.2f) { exRoll = -1e9f; penalize("ROLLED BACK ON HILL > 0.2 M", 20); }
    }
}
// ---- ordered tasks ----
static bool taskOnHill(const Item& t) {
    for (auto& z : wld.zones) if (z.type == T_HILL && pointInItem(z, {t.x, t.y})) return true;
    return false;
}
static string taskShort(const Item& t) {
    switch (taskKind(t)) {
        case TK_GATE: return "GATE";
        case TK_STOP: return taskOnHill(t) ? "HILL START" : "STOP ZONE";
        case TK_BOX: return "REVERSE BOX";
        default: return "PARALLEL PARK";
    }
}
static string taskTitle(const Item& t) {
    switch (taskKind(t)) {
        case TK_GATE: return "DRIVE THROUGH THE GATE";
        case TK_STOP: return taskOnHill(t) ? "STOP ON THE HILL, THEN MOVE OFF" : "STOP IN THE ZONE";
        case TK_BOX: return "REVERSE INTO THE BOX";
        default: return "PARALLEL PARK IN REVERSE";
    }
}
static string taskHelp(const Item& t) {
    switch (taskKind(t)) {
        case TK_GATE: return "PASS BETWEEN THE CONE AND THE EDGE OF THE ROAD";
        case TK_STOP: return taskOnHill(t) ? "STOP BEFORE THE STOP LINE - NO ROLLING BACK > 0.2 M" : "STOP INSIDE THE ZONE AND HOLD STILL";
        case TK_BOX: return "BACK IN, NOSE TO THE ARROW, STOP INSIDE THE LINES";
        default: return "BACK INTO THE POCKET, STRAIGHT, INSIDE THE LINES";
    }
}
static void taskComplete() {
    const Item& t = wld.tasks[wld.taskIdx];
    showToast(fmt("TASK %d DONE - ", (int)wld.taskIdx + 1) + taskShort(t));
    wld.taskIdx++; wld.taskHold = 0; wld.dirChanges = 0; wld.corrPen = false; wld.taskHint.clear();
    wld.centreIn = false; wld.enteredDir = 0;
    sfx::play(sfx::CHECK);
    if (!wld.tasksLeft()) showToast("ALL TASKS DONE - GO TO THE FINAL SPOT");
}
static void updateTasks(float dt) {
    // remember which way the car last moved (to tell "reversed in" from "drove in")
    if (std::fabs(car.vf) > 0.3f) {
        int d = car.vf > 0 ? 1 : -1;
        if (wld.lastDir != 0 && d != wld.lastDir && wld.tasksLeft()) {
            const Item& t = wld.tasks[wld.taskIdx];
            if (isParkTask(taskKind(t)) && vlen(car.p - V2{t.x, t.y}) < 14.f) wld.dirChanges++;
        }
        wld.lastDir = d;
    }
    wld.taskHint.clear();
    if (!wld.tasksLeft()) return;
    const Item& t = wld.tasks[wld.taskIdx];
    int kind = taskKind(t);
    float hd = std::fabs(deg(std::remainder(car.a - rad(t.rot), 2 * PI)));
    bool still = std::fabs(car.vf) < 0.05f;
    // parking tasks: more than 3 changes of direction near the task costs 5 points (exam rules)
    if (cfg.exam && isParkTask(kind) && wld.dirChanges > 3 && !wld.corrPen) { wld.corrPen = true; penalize("MORE THAN 3 CORRECTIONS", 5); }
    if (kind == TK_GATE) {
        if (pointInItem(t, car.p) && hd < 75.f) taskComplete();
        return;
    }
    if (kind == TK_STOP) {
        if (pointInItem(t, car.p) && hd < 35.f) {
            if (still) { wld.taskHold += dt; wld.taskHint = "HOLD STILL..."; }
            else { wld.taskHold = 0; wld.taskHint = "STOP INSIDE THE ZONE"; }
            if (wld.taskHold >= TASK_HOLD) taskComplete();
        } else wld.taskHold = 0;
        return;
    }
    // parking: every corner of the car inside the zone, stopped, straight, and the car came in backwards
    bool cin = pointInItem(t, car.p);
    if (cin && !wld.centreIn) wld.enteredDir = car.vf < 0 ? -1 : 1;
    wld.centreIn = cin;
    OBB cb{car.p.x, car.p.y, car.L / 2, car.W / 2, car.a};
    V2 cs[4]; corners(cb, cs);
    bool all = true;
    for (auto& c : cs) if (!pointInItem(t, c, 0.15f)) all = false;
    if (!all) {
        wld.taskHold = 0;
        if (vlen(car.p - V2{t.x, t.y}) < 6.f && still) wld.taskHint = "NOT FULLY INSIDE THE LINES";
        return;
    }
    if (!still) { wld.taskHold = 0; wld.taskHint = "STOP HERE"; return; }
    if (wld.enteredDir > 0) { wld.taskHold = 0; wld.taskHint = "YOU MUST REVERSE IN - PULL OUT AND TRY AGAIN"; return; }
    if (hd > TASK_ANG_PARK) { wld.taskHold = 0; wld.taskHint = fmt("STRAIGHTEN UP - %d DEG OFF", (int)std::round(hd)); return; }
    wld.taskHold += dt; wld.taskHint = "PERFECT - HOLD STILL...";
    if (wld.taskHold >= TASK_HOLD) taskComplete();
}
static void updatePlay(float dt) {
    if (actHit(A_RESTART)) { restartPlay(); return; }
    if (in.kp[GLFW_KEY_ESCAPE] && !pDone && !pFailed) pPaused = !pPaused;
    bool frozen = pPaused || pDone || pFailed;
    sfx::setHorn(!frozen && actDown(A_HORN));
    if (frozen) {
        car.gas = 0; sfx::setSkid(0);
        sfx::setEngine(!pFailed, 0.12f, 0);
        return;
    }
    pClock += dt;
    // camera + zoom
    if (actHit(A_CAMERA)) { cfg.cam = (cfg.cam + 1) % CAM_COUNT; saveSettings(); showToast(string("CAMERA: ") + CAMNAME[cfg.cam]); }
    if (actHit(A_ZOOMIN)) pZoomMul = clampf(pZoomMul * 1.15f, 0.35f, 3.f);
    if (actHit(A_ZOOMOUT)) pZoomMul = clampf(pZoomMul / 1.15f, 0.35f, 3.f);
    if (in.scroll != 0 && !hudWheelDrag) pZoomMul = clampf(pZoomMul * std::pow(1.12f, in.scroll), 0.35f, 3.f);
    // gears, signals, belt
    if (actHit(A_DRIVE)) shiftTo(G_D);
    if (actHit(A_REVERSE)) shiftTo(G_R);
    if (actHit(A_NEUTRAL)) shiftTo(G_N);
    if (actHit(A_PARK)) shiftTo(G_P);
    if (actHit(A_SIGL)) toggleSignal(true);
    if (actHit(A_SIGR)) toggleSignal(false);
    if (actHit(A_BELT)) { belt = !belt; sfx::play(sfx::BELT); showToast(belt ? "SEATBELT FASTENED" : "SEATBELT OFF"); }
    if (sigL || sigR) {
        sigT += dt; bool ph = std::fmod(sigT, 0.7f) < 0.35f;
        if (ph != sigPhase) { sigPhase = ph; sfx::play(ph ? sfx::TICK : sfx::TOCK); }
        float w = sigR ? car.wheel : -car.wheel;          // auto-cancel after the turn is unwound
        if (w > 240.f) sigArmed = true;
        if (sigArmed && w < 60.f) { sigL = sigR = false; sigArmed = false; }
    }
    Ctl k{actDown(A_GAS) || hudGas, actDown(A_BRAKE) || hudBrake, actDown(A_LEFT), actDown(A_RIGHT), actDown(A_HANDBRAKE)};
    if (k.gas || k.brake || car.gear != (cfg.exam ? G_P : G_D)) pStarted = true;
    if (pStarted) pTime += dt;
    pHitCd -= dt; pShake = std::max(0.f, pShake - dt);
    contacts.clear();
    int n = std::max(1, (int)std::ceil(dt / (1.f / 120.f)));
    for (int i = 0; i < n; i++) stepCar(car, dt / n, k);
    // collisions
    for (auto& c : contacts) {
        if (c.kind == K_CONE) sfx::play(sfx::CONE);
        else if (c.speed > 0.3f) { sfx::play(sfx::HIT, clampf(c.speed / 5.f, 0.2f, 1.f)); pShake = std::max(pShake, std::min(0.45f, c.speed * 0.05f)); }
        if (cfg.exam) { penalize(string("COLLISION: ") + KINDNAME[c.kind], 100); break; }
        if ((c.speed > 1.5f || c.kind == K_CONE) && pHitCd <= 0) { pHits++; pHitCd = 0.5f; }
    }
    if (pFailed) return;
    if (cfg.exam) { examChecks(dt); if (pFailed) return; }
    updateTasks(dt);
    if (pFailed) return;
    // engine + tyre audio
    {
        int dirG = car.gear == G_D || car.gear == G_R;
        float rpm = dirG ? 0.12f + car.gas * 0.3f + std::fabs(car.vf) / 14.f * 0.6f : 0.12f + car.gas * 0.85f;
        sfx::setEngine(true, clampf(rpm, 0, 1), car.gas);
        sfx::setSkid(clampf((car.slip - 1.6f) / 4.f, 0, 1) + (car.hb && std::fabs(car.vf) > 2.f ? 0.5f : 0.f));
    }
    float spd = vlen(car.v);
    if ((car.slip > 2.2f || (car.hb && spd > 2.f)) && spd > 1.5f) {
        V2 f = {std::cos(car.a), std::sin(car.a)}, r = {-std::sin(car.a), std::cos(car.a)};
        V2 w1 = car.p - f * 1.3f + r * 0.85f, w2 = car.p - f * 1.3f - r * 0.85f;
        marks.push_back({w1, car.a}); marks.push_back({w2, car.a});
        emitSmoke(w1, car.v * 0.1f); emitSmoke(w2, car.v * 0.1f);
        if (marks.size() > 2400) marks.erase(marks.begin(), marks.begin() + 200);
    }
    // parking test
    pInSpot = false;
    if (wld.hasSpot) {
        OBB cb{car.p.x, car.p.y, car.L / 2, car.W / 2, car.a};
        V2 cs[4]; corners(cb, cs);
        bool all = true;
        for (auto& c : cs) {
            V2 l = rot(c - V2{wld.spot.x, wld.spot.y}, -rad(wld.spot.rot));
            if (std::fabs(l.x) > wld.spot.w / 2 + 0.2f || std::fabs(l.y) > wld.spot.h / 2 + 0.2f) all = false;
        }
        pInSpot = all;
        bool still = std::fabs(car.vf) < 0.05f;
        bool finish = false;
        if (wld.tasksLeft()) { finish = false; pPark = 0; }        // final spot is locked until every task is done
        else if (cfg.exam) finish = all && still && car.gear == G_P;
        else {
            if (all && still) pPark += dt; else pPark = std::max(0.f, pPark - dt * 2);
            finish = pPark >= 0.9f || (all && still && car.gear == G_P);
        }
        if (finish) {
            pDone = true; pDoneT = 0;
            bool clean = cfg.exam ? penTotal == 0 : pHits == 0;
            pStars = 1 + (clean ? 1 : 0) + (pTime <= cur.par ? 1 : 0);
            burstConfetti(); sfx::play(sfx::SUCCESS); sfx::setSkid(0);
            if (!playKey.empty()) {
                auto it = prog.find(playKey);
                if (it == prog.end()) prog[playKey] = {pStars, pTime};
                else { it->second.first = std::max(it->second.first, pStars); it->second.second = std::min(it->second.second, pTime); }
                saveProgress();
            }
        }
    }
}
