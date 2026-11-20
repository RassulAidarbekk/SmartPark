// SmartPark - save_settings.h
// progress file + settings (keys, audio, gameplay)
// author: Zhalgas
// NOTE: not a standalone header - included only from main.cpp, in a fixed order.
#pragma once

// ----------------------------------------------------------------------------
//  Progress (stars + best time) persisted to a text file
// ----------------------------------------------------------------------------
static std::map<string, std::pair<int, float>> prog;
static const char* PROG_FILE = "smartpark_progress.txt";
static void loadProgress() {
    std::ifstream f(PROG_FILE);
    string k; int s; float t;
    while (f >> k >> s >> t) prog[k] = {s, t};
}
static void saveProgress() {
    std::ofstream f(PROG_FILE);
    for (auto& p : prog) f << p.first << " " << p.second.first << " " << p.second.second << "\n";
}
static bool cheatUnlock = false;
static bool careerUnlocked(int i) { return cheatUnlock || i == 0 || prog.count(fmt("career_%d", i - 1)); }

// ----------------------------------------------------------------------------
//  Settings: key bindings, audio, gameplay options (smartpark_settings.txt)
// ----------------------------------------------------------------------------
enum Act { A_GAS, A_BRAKE, A_LEFT, A_RIGHT, A_HANDBRAKE, A_DRIVE, A_REVERSE, A_NEUTRAL, A_PARK,
           A_SIGL, A_SIGR, A_BELT, A_HORN, A_CAMERA, A_ZOOMIN, A_ZOOMOUT, A_RESTART, A_COUNT };
static const char* ACTNAME[A_COUNT] = {"GAS PEDAL", "BRAKE PEDAL", "STEER LEFT", "STEER RIGHT", "HANDBRAKE",
                                       "GEAR D - DRIVE", "GEAR R - REVERSE", "GEAR N - NEUTRAL", "GEAR P - PARK",
                                       "LEFT SIGNAL", "RIGHT SIGNAL", "SEATBELT", "HORN", "CAMERA MODE", "ZOOM IN", "ZOOM OUT", "RESTART"};
static const char* ACTID[A_COUNT] = {"gas", "brake", "left", "right", "handbrake", "drive", "reverse", "neutral", "park",
                                     "sigl", "sigr", "belt", "horn", "camera", "zoomin", "zoomout", "restart"};
static const int DEFBIND[A_COUNT][2] = {
    {GLFW_KEY_W, GLFW_KEY_UP}, {GLFW_KEY_S, GLFW_KEY_DOWN}, {GLFW_KEY_A, GLFW_KEY_LEFT}, {GLFW_KEY_D, GLFW_KEY_RIGHT},
    {GLFW_KEY_SPACE, -1}, {GLFW_KEY_F, -1}, {GLFW_KEY_R, -1}, {GLFW_KEY_N, -1}, {GLFW_KEY_P, -1},
    {GLFW_KEY_Q, -1}, {GLFW_KEY_E, -1}, {GLFW_KEY_B, -1}, {GLFW_KEY_H, -1}, {GLFW_KEY_V, -1},
    {GLFW_KEY_EQUAL, GLFW_KEY_KP_ADD}, {GLFW_KEY_MINUS, GLFW_KEY_KP_SUBTRACT}, {GLFW_KEY_BACKSPACE, GLFW_KEY_F5}};
enum CamMode { CAM_FOLLOW, CAM_ROTATE, CAM_FIXED, CAM_COUNT };
static const char* CAMNAME[CAM_COUNT] = {"FOLLOW", "ROTATE WITH CAR", "FIXED (WHOLE MAP)"};
static const char* CENTERNAME[3] = {"OFF", "REALISTIC", "ALWAYS"};
struct Settings {
    int bind[A_COUNT][2];
    float master = 0.8f, engine = 0.7f, fx = 0.8f, ui = 0.6f;
    bool exam = true, minimap = true, guide = true;
    int cam = CAM_FOLLOW, center = 1;
} cfg;
static const char* CFG_FILE = "smartpark_settings.txt";
static void resetBindings() { for (int a = 0; a < A_COUNT; a++) { cfg.bind[a][0] = DEFBIND[a][0]; cfg.bind[a][1] = DEFBIND[a][1]; } }
static void applyAudio() { sfx::setVolumes(cfg.master, cfg.engine, cfg.fx, cfg.ui); }
static void saveSettings() {
    std::ofstream f(CFG_FILE);
    if (!f) return;
    f << "SPCFG1\n";
    for (int a = 0; a < A_COUNT; a++) f << "BIND " << ACTID[a] << " " << cfg.bind[a][0] << " " << cfg.bind[a][1] << "\n";
    f << "VOL " << cfg.master << " " << cfg.engine << " " << cfg.fx << " " << cfg.ui << "\n";
    f << "EXAM " << cfg.exam << "\nMINIMAP " << cfg.minimap << "\nGUIDE " << cfg.guide << "\nCAM " << cfg.cam << "\nCENTER " << cfg.center << "\n";
}
static void loadSettings() {
    resetBindings();
    std::ifstream f(CFG_FILE);
    string line, tag;
    if (!f || !std::getline(f, line)) return;
    while (std::getline(f, line)) {
        std::istringstream ss(line);
        tag.clear(); ss >> tag;
        if (tag == "BIND") {
            string id; int k0 = -1, k1 = -1; ss >> id >> k0 >> k1;
            for (int a = 0; a < A_COUNT; a++) if (id == ACTID[a]) { cfg.bind[a][0] = k0; cfg.bind[a][1] = k1; }
        } else if (tag == "VOL") ss >> cfg.master >> cfg.engine >> cfg.fx >> cfg.ui;
        else if (tag == "EXAM") ss >> cfg.exam;
        else if (tag == "MINIMAP") ss >> cfg.minimap;
        else if (tag == "GUIDE") ss >> cfg.guide;
        else if (tag == "CAM") ss >> cfg.cam;
        else if (tag == "CENTER") ss >> cfg.center;
    }
    cfg.master = clampf(cfg.master, 0, 1); cfg.engine = clampf(cfg.engine, 0, 1);
    cfg.fx = clampf(cfg.fx, 0, 1); cfg.ui = clampf(cfg.ui, 0, 1);
    cfg.cam = (int)clampf((float)cfg.cam, 0, CAM_COUNT - 1); cfg.center = (int)clampf((float)cfg.center, 0, 2);
}
static string keyName(int k) {
    if (k < 0) return "---";
    if ((k >= GLFW_KEY_A && k <= GLFW_KEY_Z) || (k >= GLFW_KEY_0 && k <= GLFW_KEY_9)) return string(1, (char)k);
    if (k >= GLFW_KEY_F1 && k <= GLFW_KEY_F25) return fmt("F%d", k - GLFW_KEY_F1 + 1);
    if (k >= GLFW_KEY_KP_0 && k <= GLFW_KEY_KP_9) return fmt("NUM%d", k - GLFW_KEY_KP_0);
    switch (k) {
        case GLFW_KEY_SPACE: return "SPACE"; case GLFW_KEY_UP: return "UP"; case GLFW_KEY_DOWN: return "DOWN";
        case GLFW_KEY_LEFT: return "LEFT"; case GLFW_KEY_RIGHT: return "RIGHT";
        case GLFW_KEY_LEFT_SHIFT: return "L.SHIFT"; case GLFW_KEY_RIGHT_SHIFT: return "R.SHIFT";
        case GLFW_KEY_LEFT_CONTROL: return "L.CTRL"; case GLFW_KEY_RIGHT_CONTROL: return "R.CTRL";
        case GLFW_KEY_LEFT_ALT: return "L.ALT"; case GLFW_KEY_RIGHT_ALT: return "R.ALT";
        case GLFW_KEY_TAB: return "TAB"; case GLFW_KEY_ENTER: return "ENTER"; case GLFW_KEY_BACKSPACE: return "BKSP";
        case GLFW_KEY_DELETE: return "DEL"; case GLFW_KEY_INSERT: return "INS"; case GLFW_KEY_HOME: return "HOME";
        case GLFW_KEY_END: return "END"; case GLFW_KEY_PAGE_UP: return "PGUP"; case GLFW_KEY_PAGE_DOWN: return "PGDN";
        case GLFW_KEY_CAPS_LOCK: return "CAPS"; case GLFW_KEY_KP_ADD: return "NUM+"; case GLFW_KEY_KP_SUBTRACT: return "NUM-";
        case GLFW_KEY_KP_MULTIPLY: return "NUM*"; case GLFW_KEY_KP_DIVIDE: return "NUM/"; case GLFW_KEY_KP_ENTER: return "NUMENTER";
        case GLFW_KEY_KP_DECIMAL: return "NUM."; case GLFW_KEY_EQUAL: return "="; case GLFW_KEY_MINUS: return "MINUS";
        case GLFW_KEY_COMMA: return ","; case GLFW_KEY_PERIOD: return "."; case GLFW_KEY_SLASH: return "/";
        case GLFW_KEY_SEMICOLON: return ";"; case GLFW_KEY_APOSTROPHE: return "'"; case GLFW_KEY_LEFT_BRACKET: return "[";
        case GLFW_KEY_RIGHT_BRACKET: return "]"; case GLFW_KEY_BACKSLASH: return "\\"; case GLFW_KEY_GRAVE_ACCENT: return "`";
    }
    return fmt("KEY%d", k);
}
static string bindLabel(int a) {
    string s = keyName(cfg.bind[a][0]);
    if (cfg.bind[a][1] >= 0) s += "/" + keyName(cfg.bind[a][1]);
    return s;
}
