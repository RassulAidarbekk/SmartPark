// ============================================================================
//  SmartPark - "Park & Build"   (Team Graphical Goddess, SE-2411)
//  2D top-down car parking game + level editor, C++17 / OpenGL / GLFW
//  Rendering uses only OpenGL 2D primitives (quads, fans, lines) + a built-in
//  bitmap font, so there are no external assets and no extra loader needed.
//
//  v0.2 "Autodrom" update
//   - realistic driving: steering wheel (15.4:1 ratio, caster self-centering),
//     gas/brake pedals, automatic gearbox P/R/N/D with idle creep, rear-axle
//     pivot kinematics, hills with gravity
//   - exam rules modelled on the Kazakhstan category B autodrome
//     (MVD RK order No.862): collisions = 100 points = fail, control lines,
//     stop lines, pedestrian crossings, hill roll-back, 20 km/h limit, belt,
//     turn signal at start, time limit. 100+ points = exam failed
//   - follow / rotating / fixed camera with zoom + minimap
//   - level editor: maps up to 300x300, zoom buttons, curve pieces
//     (arc curb, arc line, roundabout island, cone arc), grass, hills, lines
//   - settings: key rebinding + audio sliders + gameplay options
//
//  v0.3 "Exam tasks"
//   - TASK ZONE items: ordered tasks (gate, stop zone, reverse box, parallel park); the target spot is
//     locked until all tasks are done. Navigation arrow, task panel with compass, checklist, live hints
//   - AUTODROM EXAM rebuilt: hill start, crossing, reverse box, parallel park, slalom gates, final bay
//   - fix: stop lines no longer fire when the car joins the road from the side past the line
// ============================================================================
#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include "audio.h"
#include "safety_analysis.h"

using std::string;
using std::vector;
namespace fs = std::filesystem;

// ----------------------------------------------------------------------------
//  Game parts (one file per area, order matters: each part uses the ones above)
// ----------------------------------------------------------------------------
#include "game/math_draw.h"   // Rasul
#include "game/level_data.h"   // Alijan
#include "game/career_levels.h"   // Alijan
#include "game/save_settings.h"   // Zhalgas
#include "game/physics.h"   // Zhalgas
#include "game/input.h"   // Zhalgas
#include "game/render.h"   // Rasul
#include "game/ui.h"   // Alijan
#include "game/screens_menu.h"   // Alijan
#include "game/game_rules.h"   // Samandar
#include "game/screen_play.h"   // Rasul
#include "game/screen_editor.h"   // Alijan
#include "game/screen_settings.h"   // Alijan

// ----------------------------------------------------------------------------
//  main
// ----------------------------------------------------------------------------
int main(int argc, char** argv) {
    int maxFrames = -1; string shot, go; int forceGear = -1; float startWheel = 0; bool forceArcade = false;
    int jumpTask = -1; float atX = -1, atY = -1, atH = 0;
    for (int i = 1; i < argc; i++) {
        string a = argv[i];
        if (a == "--frames" && i + 1 < argc) maxFrames = atoi(argv[++i]);
        else if (a == "--shot" && i + 1 < argc) shot = argv[++i];
        else if (a == "--go" && i + 1 < argc) go = argv[++i];
        else if (a == "--hold" && i + 1 < argc) { string k = argv[++i]; fakeHeld.push_back(k.size() == 1 ? std::toupper(k[0]) : (k == "UP" ? GLFW_KEY_UP : (k == "SPACE" ? GLFW_KEY_SPACE : -1))); }
        else if (a == "--gear" && i + 1 < argc) { const char* g = std::strchr(GEARCH, std::toupper(argv[++i][0])); if (g) forceGear = (int)(g - GEARCH); }
        else if (a == "--wheel" && i + 1 < argc) startWheel = (float)atof(argv[++i]);
        else if (a == "--arcade") forceArcade = true;
        else if (a == "--tool" && i + 1 < argc) edTool = atoi(argv[++i]) % T_COUNT;
        else if (a == "--task" && i + 1 < argc) jumpTask = atoi(argv[++i]);                 // start at task N (0-based)
        else if (a == "--at" && i + 1 < argc) sscanf(argv[++i], "%f,%f,%f", &atX, &atY, &atH); // place the car x,y,heading
    }
    if (!glfwInit()) { fprintf(stderr, "GLFW init failed\n"); return 1; }
    glfwWindowHint(GLFW_SAMPLES, 4);
    win = glfwCreateWindow(1280, 720, "SmartPark - Park & Build", nullptr, nullptr);
    if (!win) { fprintf(stderr, "Window creation failed\n"); glfwTerminate(); return 1; }
    glfwMakeContextCurrent(win);
    glfwSwapInterval(1);
    glfwSetWindowSizeLimits(win, 1100, 640, GLFW_DONT_CARE, GLFW_DONT_CARE);
    glfwSetKeyCallback(win, onKey); glfwSetMouseButtonCallback(win, onMouse);
    glfwSetScrollCallback(win, onScroll); glfwSetCharCallback(win, onChar);
    loadSettings();
    if (forceArcade) cfg.exam = false;
    if (!sfx::init()) fprintf(stderr, "Audio device not available - running silent\n");
    applyAudio();
    buildCareer(); loadProgress();
    std::error_code ec; fs::create_directories("levels", ec);
    refreshCustoms();
    if (go == "career") { cheatUnlock = true; screen = S_CAREER; }
    else if (go == "custom") screen = S_CUSTOM;
    else if (go == "edit") newEditor();
    else if (go == "settings") openSettings(S_MENU);
    else if (go.rfind("edit:", 0) == 0) { int k = atoi(go.c_str() + 5) % (int)CAREER.size(); int t = edTool; ed = CAREER[k]; edPath.clear(); fitEditorCam(); edTool = t; screen = S_EDIT; }
    else if (go.rfind("file:", 0) == 0) {
        Level L; if (loadLevel(go.substr(5), L)) { startPlay(L, PM_CUSTOM, 0, ""); if (forceGear >= 0) car.gear = forceGear; belt = true; sigL = true; }
    }
    else if (go.rfind("play:", 0) == 0) {
        int k = atoi(go.c_str() + 5); k %= (int)CAREER.size(); startPlay(CAREER[k], PM_CAREER, k, "");
        if (forceGear >= 0) car.gear = forceGear;
        car.wheel = startWheel; belt = true; sigL = true;
    }
    if (screen == S_PLAY) {
        if (jumpTask >= 0) wld.taskIdx = std::min((size_t)jumpTask, wld.tasks.size());
        if (atX >= 0) { car.p = {atX, atY}; car.a = rad(atH); startPos = car.p; pCam = car.p; exStartChecked = true; }
    }

    double last = glfwGetTime(); double px = 0, py = 0; int frame = 0;
    float fadeA = 1.f; Screen prevScr = screen;
    while (!glfwWindowShouldClose(win)) {
        glfwPollEvents();
        double now = glfwGetTime(); float dt = (float)std::min(0.05, now - last); last = now;
        if (maxFrames > 0) dt = 1.f / 60.f;     // deterministic for automated captures
        gTime += dt;
        int ww, wh; glfwGetWindowSize(win, &ww, &wh); glfwGetFramebufferSize(win, &fbw, &fbh);
        double mx, my; glfwGetCursorPos(win, &mx, &my);
        double sx = ww ? (double)fbw / ww : 1, sy = wh ? (double)fbh / wh : 1;
        in.mx = mx * sx; in.my = my * sy; in.dmx = in.mx - px; in.dmy = in.my - py; px = in.mx; py = in.my;

        glViewport(0, 0, fbw, fbh);
        glMatrixMode(GL_PROJECTION); glLoadIdentity(); glOrtho(0, fbw, fbh, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW); glLoadIdentity();
        glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_DEPTH_TEST);
        gView = {-1e9f, -1e9f, 1e9f, 1e9f};

        if (screen != S_PLAY) { sfx::setEngine(false, 0, 0); sfx::setSkid(0); sfx::setHorn(false); }
        switch (screen) {
            case S_MENU: screenMenu(); break;
            case S_CAREER: screenCareer(); break;
            case S_CUSTOM: screenCustom(); break;
            case S_PLAY: screenPlay(dt); break;
            case S_EDIT: screenEdit(dt); break;
            case S_SETTINGS: screenSettings(); break;
        }
        if (screen == S_MENU || screen == S_CAREER || screen == S_CUSTOM || screen == S_SETTINGS) vignette();
        if (screen != S_EDIT) drawToast(dt);
        if (screen != prevScr) { fadeA = prevScr == S_SETTINGS || screen == S_SETTINGS ? 0.5f : 1.f; prevScr = screen; }
        if (fadeA > 0.f) { setc(Col{0.04f, 0.05f, 0.08f, fadeA}); rectTL(0, 0, (float)fbw, (float)fbh); fadeA -= dt * 3.f; }
        if (screen == S_MENU && in.kp[GLFW_KEY_ESCAPE]) glfwSetWindowShouldClose(win, 1);

        in.clickL = in.clickR = in.relL = false; in.scroll = 0;
        std::memset(in.kp, 0, sizeof in.kp);

        if (maxFrames > 0 && ++frame >= maxFrames) {
            if (!shot.empty()) {
                vector<unsigned char> px3((size_t)fbw * fbh * 3);
                glReadPixels(0, 0, fbw, fbh, GL_RGB, GL_UNSIGNED_BYTE, px3.data());
                FILE* f = fopen(shot.c_str(), "wb");
                if (f) { fprintf(f, "P6\n%d %d\n255\n", fbw, fbh); for (int y = fbh - 1; y >= 0; y--) fwrite(&px3[(size_t)y * fbw * 3], 1, (size_t)fbw * 3, f); fclose(f); }
            }
            break;
        }
        glfwSwapBuffers(win);
    }
    sfx::shutdown();
    glfwDestroyWindow(win); glfwTerminate();
    return 0;
}
