// SmartPark - level_data.h
// level data structures, item types, task kinds
// author: Alijan
// NOTE: not a standalone header - included only from main.cpp, in a fixed order.
#pragma once

// ----------------------------------------------------------------------------
//  Level data
// ----------------------------------------------------------------------------
enum IType {
    T_WALL, T_BARRIER, T_PCAR, T_CONE, T_TREE, T_SPOT, T_START, T_MUD, T_ICE, T_LINE, T_BAY, T_CROSSING, T_ACCESSIBLE, T_ENTRANCE,
    // v0.2 additions (appended so old .lvl files still load)
    T_GRASS, T_HILL, T_CTRLLINE, T_STOPLINE, T_CURB, T_ARCCURB, T_ARCLINE, T_ISLAND, T_CONEARC,
    // v0.3: ordered exam tasks (color = task kind, order = order in the file)
    T_TASK, T_COUNT
};
static const char* TNAME[T_COUNT] = {"WALL (DRAG)", "BARRIER", "PARKED CAR", "CONE", "TREE", "TARGET SPOT",
                                     "START POINT", "MUD (DRAG)", "ICE (DRAG)", "ROAD LINE", "EMPTY BAY", "CROSSING", "ACCESS BAY", "ENTRANCE",
                                     "GRASS (DRAG)", "HILL (DRAG)", "CONTROL LINE", "STOP LINE", "CURB", "ARC CURB", "ARC LINE", "ROUNDABOUT", "CONE ARC",
                                     "TASK ZONE"};
static const float DW[T_COUNT] = {2, 3.2f, 4.2f, 0.8f, 2.f, 5.4f, 4.2f, 6, 6, 3.f, 5.4f, 5.f, 5.4f, 4.f,
                                  6, 8, 6.f, 0.4f, 4.f, 8, 8, 10, 8, 6.f};
static const float DH[T_COUNT] = {2, 0.7f, 1.9f, 0.8f, 2.f, 2.8f, 1.9f, 6, 6, 0.2f, 2.8f, 2.f, 3.5f, 1.5f,
                                  6, 6, 0.15f, 3.5f, 0.5f, 90, 90, 10, 180, 3.2f};
static bool isDragTool(int t) { return t == T_WALL || t == T_MUD || t == T_ICE || t == T_GRASS || t == T_HILL; }
static bool isArcType(int t) { return t == T_ARCCURB || t == T_ARCLINE || t == T_CONEARC; }
static bool isZone(int t) { return t == T_MUD || t == T_ICE || t == T_GRASS || t == T_HILL; }
static const unsigned CARCOL[8] = {0xe03c3c, 0x3b82f6, 0xf2f2f2, 0x2b2f36, 0xf5b301, 0x22b573, 0x9b59d0, 0xa8b0bd};
static const float CURB_T = 0.5f, LINE_T = 0.15f, HILL_G = 1.57f;   // HILL_G: 16% grade * 9.8 m/s^2 (order allows 8-16%)
static const float MAP_MIN_W = 20, MAP_MIN_H = 14, MAP_MAX = 300;

// ---- exam tasks (TASK ZONE items) ----
// A task is a zone the player must complete IN ORDER before the final TARGET SPOT accepts the car.
// Item fields: x,y = centre, w = length along the required heading, h = width, rot = required heading
// (direction the car's FRONT must point when the task is done), color = kind.
enum TaskKind { TK_GATE, TK_STOP, TK_BOX, TK_PARALLEL, TK_COUNT };
static const char* TKNAME[TK_COUNT] = {"GATE", "STOP ZONE", "REVERSE BOX", "PARALLEL PARK"};
static const float TKW[TK_COUNT] = {2.5f, 6.5f, 6.0f, 7.2f}, TKH[TK_COUNT] = {5.0f, 6.0f, 3.2f, 2.8f};
static const float TASK_HOLD = 1.5f;      // seconds the car must stand still to complete a stop / parking task
static const float TASK_ANG_PARK = 12.f;  // max heading error (deg) for parking tasks
static bool isParkTask(int k) { return k == TK_BOX || k == TK_PARALLEL; }

// Item notes:  arcs (ARC CURB / ARC LINE): x,y = centre, w = radius, h = sweep (deg), rot = start angle.
//              ROUNDABOUT: w = h = diameter.  HILL: color = uphill direction (0..3 = 0/90/180/270 deg).
//              ARC LINE / ROAD LINE: color 0 = solid (control line), 1 = dashed (decoration).
struct Item { int type; float x, y, w, h, rot; int color; };
static int taskKind(const Item& it) { return ((it.color % TK_COUNT) + TK_COUNT) % TK_COUNT; }
struct Level {
    string name = "UNTITLED";
    float W = 44, H = 26, par = 40;
    vector<Item> items;
};

static safety::Result analyzeLayout(const Level& L) {
    std::vector<safety::Object> objects;
    for (const auto& it : L.items) objects.push_back({it.type, it.x, it.y, it.w, it.h});
    return safety::analyze(objects);
}

static bool saveLevel(const string& path, const Level& L) {
    std::ofstream f(path);
    if (!f) return false;
    f << "SPARK3\nNAME " << L.name << "\nSIZE " << L.W << " " << L.H << "\nPAR " << L.par << "\n";
    for (auto& it : L.items)
        f << "I " << it.type << " " << it.x << " " << it.y << " " << it.w << " " << it.h << " " << it.rot << " " << it.color << "\n";
    return true;
}
static bool loadLevel(const string& path, Level& L) {
    std::ifstream f(path);
    if (!f) return false;
    string line, tag;
    L = Level();
    if (!std::getline(f, line)) return false;
    if (!line.empty() && line.back() == '\r') line.pop_back();
    if (line != "SPARK1" && line != "SPARK2" && line != "SPARK3") return false;
    while (std::getline(f, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        std::istringstream ss(line);
        tag.clear();
        ss >> tag;
        if (tag == "NAME") { std::getline(ss, L.name); if (!L.name.empty() && L.name[0] == ' ') L.name.erase(0, 1); }
        else if (tag == "SIZE") ss >> L.W >> L.H;
        else if (tag == "PAR") ss >> L.par;
        else if (tag == "I") {
            Item it{};
            ss >> it.type >> it.x >> it.y >> it.w >> it.h >> it.rot >> it.color;
            if (it.type >= 0 && it.type < T_COUNT && it.type != T_CONEARC) L.items.push_back(it);
        }
    }
    L.W = clampf(L.W, MAP_MIN_W, MAP_MAX); L.H = clampf(L.H, MAP_MIN_H, MAP_MAX);
    return true;
}
