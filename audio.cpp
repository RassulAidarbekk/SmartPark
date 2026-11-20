// SmartPark procedural audio. All sounds are generated sample-by-sample in the
// miniaudio callback, so the game ships without any .wav/.ogg files.
#define MINIAUDIO_IMPLEMENTATION
#define MA_NO_ENCODING
#define NOMINMAX
#define MINIAUDIO_IMPLEMENTATION
#define MA_NO_ENCODING
#define MA_NO_DECODING
#define MA_NO_GENERATION
#define MA_NO_RESOURCE_MANAGER
#define MA_NO_NODE_GRAPH
#define MA_NO_ENGINE
#include "miniaudio.h"
#include "audio.h"
#define MA_NO_DECODING
#define MA_NO_GENERATION
#define MA_NO_RESOURCE_MANAGER
#define MA_NO_NODE_GRAPH
#define MA_NO_ENGINE

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>

namespace sfx {
namespace {
const float TAU = 6.28318530718f;
const int RATE = 48000;

std::atomic<float> aMaster{0.8f}, aEngineVol{0.7f}, aFxVol{0.8f}, aUiVol{0.7f};
std::atomic<float> aRpm{0.f}, aLoad{0.f}, aSkid{0.f};
std::atomic<bool> aRunning{false}, aHorn{false};

// single-producer / single-consumer event ring (game thread -> audio thread)
struct Msg { int ev; float s; };
const int QN = 64;
Msg queue[QN];
std::atomic<int> qHead{0}, qTail{0};

struct Voice { int ev = -1; float t = 0, s = 1, ph = 0, ph2 = 0; };
Voice voices[24];

uint32_t noiseState = 22222;
inline float noise() { noiseState = noiseState * 1664525u + 1013904223u; return ((noiseState >> 9) & 0xFFFF) / 32767.5f - 1.f; }

// smoothed engine/skid state (audio thread only)
float sRpm = 0, sLoad = 0, sSkid = 0, sRun = 0, sHorn = 0;
float engPh = 0, engPh2 = 0, engLp = 0, engLp2 = 0;
float skidLp = 0, skidHp = 0, skidPrev = 0, hornPh1 = 0, hornPh2 = 0, hornLp = 0;

ma_device device;
bool deviceOk = false;

float duration(int ev) {
    switch (ev) {
        case UI_CLICK: return 0.05f; case HIT: return 0.45f; case CONE: return 0.22f;
        case PENALTY: return 0.36f; case FAIL: return 0.9f; case SUCCESS: return 0.8f;
        case TICK: case TOCK: return 0.03f; case SHIFT: return 0.12f; case WARN: return 0.1f;
        case BELT: return 0.16f; case CHECK: return 0.34f;
    }
    return 0.1f;
}
float envExp(float t, float k) { return std::exp(-t * k); }
float sine(float& ph, float f) { ph += f / RATE; if (ph > 1) ph -= 1; return std::sin(ph * TAU); }

// one sample of a one-shot voice; returns false when finished. bus: 0 = fx, 1 = ui
bool voiceSample(Voice& v, float& out, int& bus) {
    float t = v.t, s = v.s;
    bus = 0;
    float o = 0;
    switch (v.ev) {
        case UI_CLICK: bus = 1; o = sine(v.ph, 1450.f) * envExp(t, 90.f) * 0.35f; break;
        case HIT: {
            float f = 95.f - 50.f * std::min(1.f, t * 4.f);
            o = sine(v.ph, f) * envExp(t, 9.f) * 0.9f + noise() * envExp(t, 35.f) * 0.5f;
            o *= 0.5f + 0.5f * s; break; }
        case CONE: o = (sine(v.ph, 520.f) * 0.6f + sine(v.ph2, 1180.f) * 0.3f) * envExp(t, 24.f) + noise() * envExp(t, 70.f) * 0.3f; break;
        case PENALTY: { float f = t < 0.14f ? 880.f : 620.f; float tt = t < 0.14f ? t : t - 0.16f;
            o = (t > 0.14f && t < 0.16f) ? 0 : (sine(v.ph, f) > 0 ? 0.25f : -0.25f) * envExp(tt, 7.f); bus = 1; break; }
        case FAIL: { float saw = std::fmod(t * 110.f, 1.f) * 2.f - 1.f; float saw2 = std::fmod(t * 116.f, 1.f) * 2.f - 1.f;
            o = (saw + saw2) * 0.22f * (0.75f + 0.25f * std::sin(t * TAU * 9.f)) * std::min(1.f, (0.9f - t) * 6.f); bus = 1; break; }
        case SUCCESS: { static const float notes[4] = {523.25f, 659.25f, 783.99f, 1046.5f};
            int k = std::min(3, (int)(t / 0.1f)); float tt = t - k * 0.1f;
            o = sine(v.ph, notes[k]) * envExp(tt, k == 3 ? 4.f : 10.f) * 0.35f; bus = 1; break; }
        case TICK: o = (noise() * 0.5f + sine(v.ph, 2600.f) * 0.5f) * envExp(t, 260.f) * 0.5f; bus = 1; break;
        case TOCK: o = (noise() * 0.4f + sine(v.ph, 1500.f) * 0.6f) * envExp(t, 230.f) * 0.5f; bus = 1; break;
        case SHIFT: o = sine(v.ph, 130.f) * envExp(t, 30.f) * 0.6f + noise() * envExp(t, 120.f) * 0.3f; break;
        case WARN: bus = 1; o = (sine(v.ph, 1250.f) > 0 ? 0.18f : -0.18f) * std::min(1.f, (0.1f - t) * 60.f); break;
        case BELT: { float tt = t < 0.07f ? t : t - 0.07f;
            o = (noise() * 0.6f + sine(v.ph, 1900.f) * 0.4f) * envExp(tt, 150.f) * 0.5f; bus = 1; break; }
        case CHECK: { float f = t < 0.1f ? 987.77f : 1318.5f; float tt = t < 0.1f ? t : t - 0.1f;   // two-note "ding-ding"
            o = (sine(v.ph, f) * 0.7f + sine(v.ph2, f * 2.f) * 0.15f) * envExp(tt, t < 0.1f ? 18.f : 9.f) * 0.4f; bus = 1; break; }
        default: return false;
    }
    v.t += 1.f / RATE;
    out = o;
    return v.t < duration(v.ev);
}

void callback(ma_device*, void* outV, const void*, ma_uint32 frames) {
    float* out = (float*)outV;
    // pull events
    int tail = qTail.load(std::memory_order_relaxed);
    while (tail != qHead.load(std::memory_order_acquire)) {
        Msg m = queue[tail];
        for (auto& v : voices) if (v.ev < 0) { v.ev = m.ev; v.t = 0; v.s = m.s; v.ph = v.ph2 = 0; break; }
        tail = (tail + 1) % QN;
    }
    qTail.store(tail, std::memory_order_release);

    float master = aMaster.load(), eVol = aEngineVol.load(), fxVol = aFxVol.load(), uiVol = aUiVol.load();
    float tRpm = aRpm.load(), tLoad = aLoad.load(), tSkid = aSkid.load();
    float tRun = aRunning.load() ? 1.f : 0.f, tHorn = aHorn.load() ? 1.f : 0.f;
    for (ma_uint32 i = 0; i < frames; i++) {
        sRpm += (tRpm - sRpm) * 0.0006f; sLoad += (tLoad - sLoad) * 0.001f;
        sSkid += (tSkid - sSkid) * 0.002f; sRun += (tRun - sRun) * 0.0004f; sHorn += (tHorn - sHorn) * 0.01f;
        float mix = 0;
        // ---- engine: firing pulses + saw, low-passed; cutoff opens with load
        if (sRun > 0.001f) {
            float f = 26.f + sRpm * 95.f;
            engPh += f / RATE; if (engPh > 1) engPh -= 1;
            engPh2 += f * 0.5f / RATE; if (engPh2 > 1) engPh2 -= 1;
            float pulse = engPh < 0.18f ? 1.f : -0.22f;
            float saw = engPh2 * 2.f - 1.f;
            float raw = pulse * 0.6f + saw * 0.4f + noise() * 0.05f;
            float cut = 0.03f + sLoad * 0.09f + sRpm * 0.04f;
            engLp += (raw - engLp) * cut; engLp2 += (engLp - engLp2) * cut;
            mix += engLp2 * (0.22f + sLoad * 0.22f) * sRun * eVol;
        }
        // ---- tyre squeal: band-passed noise with a wobble
        if (sSkid > 0.001f) {
            float n = noise();
            skidLp += (n - skidLp) * 0.35f;
            float hp = skidLp - skidPrev; skidPrev = skidLp; skidHp += (hp - skidHp) * 0.5f;
            float wob = 0.7f + 0.3f * std::sin(engPh * TAU * 3.f);
            mix += skidHp * sSkid * 0.55f * wob * fxVol;
        }
        // ---- horn: two detuned squares
        if (sHorn > 0.001f) {
            hornPh1 += 392.f / RATE; if (hornPh1 > 1) hornPh1 -= 1;
            hornPh2 += 494.f / RATE; if (hornPh2 > 1) hornPh2 -= 1;
            float raw = (hornPh1 < 0.5f ? 1.f : -1.f) + (hornPh2 < 0.5f ? 1.f : -1.f);
            hornLp += (raw - hornLp) * 0.12f;
            mix += hornLp * 0.12f * sHorn * fxVol;
        }
        for (auto& v : voices) {
            if (v.ev < 0) continue;
            float o = 0; int bus = 0;
            bool alive = voiceSample(v, o, bus);
            mix += o * (bus ? uiVol : fxVol);
            if (!alive) v.ev = -1;
        }
        mix *= master;
        mix = std::tanh(mix * 1.2f) * 0.9f;   // soft clip
        out[i * 2] = mix; out[i * 2 + 1] = mix;
    }
}
} // namespace

bool init() {
    ma_device_config cfg = ma_device_config_init(ma_device_type_playback);
    cfg.playback.format = ma_format_f32;
    cfg.playback.channels = 2;
    cfg.sampleRate = RATE;
    cfg.dataCallback = callback;
    if (ma_device_init(nullptr, &cfg, &device) != MA_SUCCESS) return false;
    if (ma_device_start(&device) != MA_SUCCESS) { ma_device_uninit(&device); return false; }
    deviceOk = true;
    return true;
}
void shutdown() { if (deviceOk) { ma_device_uninit(&device); deviceOk = false; } }
void setVolumes(float m, float e, float f, float u) { aMaster = m; aEngineVol = e; aFxVol = f; aUiVol = u; }
void setEngine(bool running, float rpm01, float load01) { aRunning = running; aRpm = rpm01; aLoad = load01; }
void setSkid(float a) { aSkid = a < 0 ? 0 : (a > 1 ? 1 : a); }
void setHorn(bool on) { aHorn = on; }
void play(Event e, float s) {
    if (!deviceOk) return;
    int head = qHead.load(std::memory_order_relaxed), next = (head + 1) % QN;
    if (next == qTail.load(std::memory_order_acquire)) return;   // queue full: drop
    queue[head] = {(int)e, s};
    qHead.store(next, std::memory_order_release);
}
} // namespace sfx

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif
