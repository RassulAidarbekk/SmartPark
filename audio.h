#pragma once
// ============================================================================
//  SmartPark audio - every sound is synthesised in real time (no asset files).
//  Backend: miniaudio (single header, public domain / MIT-0).
// ============================================================================
namespace sfx {
enum Event {
    UI_CLICK, HIT, CONE, PENALTY, FAIL, SUCCESS, TICK, TOCK, SHIFT, WARN, BELT, CHECK, EV_COUNT
};
bool init();                 // returns false if no audio device (game keeps running silently)
void shutdown();
void setVolumes(float master, float engine, float effects, float ui);   // 0..1 each
void setEngine(bool running, float rpm01, float load01);              // continuous engine hum
void setSkid(float amount01);                                          // tyre squeal
void setHorn(bool on);
void play(Event e, float strength = 1.f);
}
