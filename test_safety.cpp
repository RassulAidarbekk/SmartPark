#include "safety_analysis.h"
#include <cassert>
#include <iostream>
int main() {
    using namespace safety;
    std::vector<Object> objects;
    auto empty=analyze(objects);
    assert(empty.score==40);
    objects={{Entrance,10,10,4,2},{AccessibleBay,12,10,5,3},{Crossing,18,10,5,2}};
    assert(analyze(objects).score==100);
    objects.push_back({ParkedCar,18,10,4,2});
    assert(analyze(objects).score==80);
    objects[1].x=40;
    assert(analyze(objects).score==65);
    std::cout << "Safety checks passed\n";
}
