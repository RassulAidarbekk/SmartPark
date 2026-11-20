#pragma once
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

// Small, deterministic layout checks. Units are game-world units, not metres.
namespace safety {
struct Object { int kind; float x, y, w, h; };
enum Kind { Wall=0, Barrier=1, ParkedCar=2, Cone=3, Tree=4,
            Target=5, Start=6, Mud=7, Ice=8, RoadLine=9,
            Bay=10, Crossing=11, AccessibleBay=12, Entrance=13 };
struct Finding { std::string message; int penalty; };
struct Result { int score=100; std::vector<Finding> findings; };
inline bool overlaps(const Object& a, const Object& b, float margin=0) {
    return std::fabs(a.x-b.x) < (a.w+b.w)*0.5f+margin &&
           std::fabs(a.y-b.y) < (a.h+b.h)*0.5f+margin;
}
inline float distance(const Object& a, const Object& b) {
    return std::hypot(a.x-b.x, a.y-b.y);
}
inline Result analyze(const std::vector<Object>& objects) {
    Result result;
    auto report=[&](const std::string& message, int points) {
        result.findings.push_back({message, points});
        result.score = std::max(0, result.score-points);
    };
    const Object* entrance=nullptr;
    const Object* accessible=nullptr;
    int crossings=0;
    for (const auto& o:objects) {
        if(o.kind==Entrance) entrance=&o;
        if(o.kind==AccessibleBay) accessible=&o;
        if(o.kind==Crossing) ++crossings;
    }
    if(!entrance) report("NO BUILDING ENTRANCE", 20);
    if(!accessible) report("NO ACCESSIBLE PARKING BAY", 25);
    if(!crossings) report("NO PEDESTRIAN CROSSING", 15);
    if(entrance && accessible && distance(*entrance,*accessible)>15.f)
        report("ACCESSIBLE BAY FAR FROM ENTRANCE", 15);
    for(const auto& o:objects) {
        if(o.kind!=Crossing && o.kind!=AccessibleBay) continue;
        for(const auto& other:objects) {
            if(&o==&other) continue;
            if(other.kind!=Wall && other.kind!=Barrier && other.kind!=ParkedCar && other.kind!=Tree) continue;
            if(overlaps(o,other)) {
                report(o.kind==Crossing ? "CROSSING BLOCKED" : "ACCESSIBLE BAY BLOCKED", 20);
                break;
            }
        }
    }
    return result;
}
}
