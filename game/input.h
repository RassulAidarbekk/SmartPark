// SmartPark - input.h
// keyboard / mouse input
// author: Zhalgas
// NOTE: not a standalone header - included only from main.cpp, in a fixed order.
#pragma once

// ----------------------------------------------------------------------------
//  Input
// ----------------------------------------------------------------------------
static GLFWwindow* win = nullptr;
struct Input {
    double mx = 0, my = 0, dmx = 0, dmy = 0;
    bool clickL = false, clickR = false, downL = false, downM = false, relL = false;
    float scroll = 0;
    bool kp[GLFW_KEY_LAST + 1] = {};
} in;
static string* textTarget = nullptr;   // when set, typed characters go here (level name field)
static int rebindAct = -1, rebindSlot = 0;
static vector<int> fakeHeld;            // --hold KEY (for automated screenshots)
static bool key(int k) {
    if (k < 0) return false;
    for (int f : fakeHeld) if (f == k) return true;
    return glfwGetKey(win, k) == GLFW_PRESS;
}
static bool actDown(int a) { return key(cfg.bind[a][0]) || key(cfg.bind[a][1]); }
static bool actHit(int a) {
    for (int s = 0; s < 2; s++) { int k = cfg.bind[a][s]; if (k >= 0 && k <= GLFW_KEY_LAST && in.kp[k]) return true; }
    return false;
}
static void onKey(GLFWwindow*, int k, int, int a, int) {
    if (k < 0 || k > GLFW_KEY_LAST) return;
    if (rebindAct >= 0 && a == GLFW_PRESS) {
        if (k == GLFW_KEY_ESCAPE) { rebindAct = -1; return; }
        if (k == GLFW_KEY_DELETE) { cfg.bind[rebindAct][rebindSlot] = -1; rebindAct = -1; saveSettings(); return; }
        for (int x = 0; x < A_COUNT; x++) for (int s = 0; s < 2; s++) if (cfg.bind[x][s] == k) cfg.bind[x][s] = -1;
        cfg.bind[rebindAct][rebindSlot] = k;
        rebindAct = -1; saveSettings(); sfx::play(sfx::UI_CLICK);
        return;
    }
    if (a == GLFW_PRESS || (a == GLFW_REPEAT && k == GLFW_KEY_BACKSPACE && textTarget)) in.kp[k] = true;
}
static void onChar(GLFWwindow*, unsigned cp) {
    if (!textTarget || cp < 32 || cp > 126) return;
    char ch = (char)std::toupper((int)cp);
    if ((std::isalnum((unsigned char)ch) || ch == ' ' || ch == '-' || ch == '_' || ch == '.') && textTarget->size() < 14)
        textTarget->push_back(ch);
}
static void onMouse(GLFWwindow*, int b, int a, int) {
    if (b == GLFW_MOUSE_BUTTON_LEFT) { if (a == GLFW_PRESS) { in.clickL = true; in.downL = true; } else { in.downL = false; in.relL = true; } }
    if (b == GLFW_MOUSE_BUTTON_RIGHT && a == GLFW_PRESS) in.clickR = true;
    if (b == GLFW_MOUSE_BUTTON_MIDDLE) in.downM = (a == GLFW_PRESS);
}
static void onScroll(GLFWwindow*, double, double y) { in.scroll += (float)y; }
