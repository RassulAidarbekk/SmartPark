# SmartPark - Park & Build  (v0.3 "Exam tasks")
2D top-down parking game + level editor. C++17, OpenGL 2D, GLFW, miniaudio. Desktop only.
No asset files: graphics are OpenGL primitives, sounds are synthesised in code (`audio.cpp`).

## Build & run
    cmake -S . -B build
    cmake --build build --config Release
    # Linux/macOS:  ./build/smartpark
    # Windows:      build\Release\smartpark.exe   (or build\smartpark.exe with MinGW)

Linux prerequisites: `sudo apt install build-essential cmake libglfw3-dev libgl1-mesa-dev`
(or, to let CMake build GLFW itself: `sudo apt install xorg-dev libwayland-dev libxkbcommon-dev wayland-protocols`)

One-line alternative (Linux with GLFW installed):

    g++ -std=c++17 -O2 main.cpp audio.cpp -o smartpark -lglfw -lGL -ldl -lpthread -lm

macOS: replace `-lGL` with `-framework OpenGL -framework CoreFoundation -framework CoreAudio -framework AudioToolbox`.
`miniaudio.h` (v0.11.22, public domain / MIT-0) is included in this folder - nothing else to download.

## Driving (default keys - all rebindable in SETTINGS)
| Action | Keys | Mouse |
|---|---|---|
| Gas / brake pedal | W / S (or Up / Down) | hold the GAS / BRAKE pedals bottom-right |
| Steer | A / D (or Left / Right) | drag the steering wheel bottom-centre |
| Gears | F = D (drive), R = reverse, N = neutral, P = park | click P R N D bottom-left |
| Turn signals | Q left, E right (auto-cancel after the turn) | |
| Seatbelt / horn / handbrake | B / H / Space | |
| Camera mode / zoom | V (follow -> rotate with car -> fixed), = and - or mouse wheel | |
| Restart / pause | Backspace or F5 / Esc | |

How the car works now:
- Steering wheel turns up to 540 deg each way with a 15.4:1 ratio (about 35 deg at the tyres). It returns to centre
  only while the car is rolling (caster effect) - "WHEEL SELF-CENTER" in settings can make it OFF or ALWAYS.
- Pedals are progressive (they ramp in and out). Automatic gearbox with idle creep (about 6 km/h).
  Shifting between P/R/D needs the car to be stopped.
- The car pivots around its rear axle like a real car, so reversing into a bay feels different from driving in.
- Speed-dependent understeer, handbrake slides, ice / mud / grass grip, and HILLS: 16% grade - idle creep can't
  hold the car on a hill, so you must use the brake or gas or it rolls back.
- To finish: stop fully inside the target spot and shift to P.

## Exam rules (SETTINGS -> EXAM RULES, on by default)
Modelled on the category B autodrome exam in Kazakhstan (MVD RK order No. 862). 100+ penalty points = EXAM FAILED.

| Mistake | Points |
|---|---|
| Collision with a car, cone, wall, curb, tree or the fence | 100 (fail) |
| Leaving the roadway (wheel on grass) | 100 (fail) |
| Wheel touches a control line (solid white line) | 20 |
| Did not stop before a STOP line / stopped past it | 25 / 25 |
| Stopped on a pedestrian crossing | 25 |
| Rolled back more than 0.2 m on a hill | 20 |
| Over 20 km/h - for each 5 s of speeding | 5 |
| Seatbelt not fastened when moving off | 5 |
| No left signal when moving off | 5 |
| Did not move off within 30 s | 25 |
| Over the time limit (2 x par) | 50 |
| More than 3 forward/reverse changes in one parking task | 5 |

Stars: 1 for passing, +1 for zero penalty points, +1 for finishing within par.
With exam rules OFF the game plays as before (arcade: hits only cost a star, the car starts in D).

## Exam tasks (new in v0.3)
A level can contain ordered TASK ZONES. They must be done **in order**, and the final TARGET SPOT stays
**locked** (red padlock) until every task is done - parking there early does nothing.
- Navigation: a yellow arrow orbits the car and points at the current task (blue = final spot once all tasks
  are done). The top-centre panel shows the task, live hints ("YOU MUST REVERSE IN", "STRAIGHTEN UP - 15 DEG OFF",
  "HOLD STILL..."), the distance and a compass arrow. The checklist on the right ticks tasks off.
- On the map: current task = pulsing yellow zone with its number, upcoming = faint dashed, done = green tick.

| Kind | How to complete it |
|---|---|
| GATE | drive through the zone (in the arrow direction) |
| STOP ZONE | stop with the car's centre inside the zone and hold still 1.5 s (on a hill it is the hill start) |
| REVERSE BOX | back in: whole car inside the lines, nose pointing at the arrow (max 12 deg off), stopped 1.5 s |
| PARALLEL PARK | same as the box, for the parallel pocket at the kerb |

Parking tasks only count if the car **entered in reverse** - small forward corrections inside are fine.

## Level editor
LMB place, RMB delete, Q/E rotate 15, R rotate 90, C colour / solid-dashed, G snap, WASD/MMB pan,
wheel or +/- zoom, F or FIT = fit map, P test drive, Ctrl+S save, Z undo. Click the LEVEL NAME box to rename.
- Map size up to 300 x 300 m: W-/W+/H-/H+ (hold Shift for 20 m steps) or presets S / M / L / XL / XXL.
- Tool list is grouped and scrolls with the mouse wheel.
- Curve pieces for circular routes: ARC CURB (solid), ARC LINE (solid = control line, dashed = paint),
  ROUNDABOUT island, CONE ARC (places a ring of cones). `[` `]` change radius (Shift = 5 m), `,` `.` change
  sweep in 15 deg steps, Q/E set the start angle. A full 360 deg sweep makes a closed circle.
- New markings/surfaces: CONTROL LINE, STOP LINE (yellow arrow = travel direction), CURB, GRASS (off-road),
  HILL (R turns the uphill direction).
- EXAM TASKS -> TASK ZONE: C changes the kind (gate / stop zone / reverse box / parallel park), Q/E/R rotate
  it (the arrow = where the car's nose must point). Tasks are numbered in the order you place them.
- Levels live in ./levels/*.lvl (format SPARK3; SPARK1/SPARK2 files still load).

## Settings
Main menu or pause menu -> SETTINGS. Click a key slot and press a key (Esc cancels, Del clears).
Audio sliders: master, engine, effects, interface. Saved to `smartpark_settings.txt` next to the exe.

## Career
9 levels: the original 6 (pars adjusted for the new car), plus HILL START, ROUNDABOUT and AUTODROM EXAM.

AUTODROM EXAM (rebuilt in v0.3) is a 150 x 100 m ring road around a field, driven counter-clockwise, par 260 s:
1. HILL START - stop on the 16% uphill before the STOP line, move off without rolling back
   (then the pedestrian crossing with its own stop line)
2. REVERSE BOX - back into a 6.0 x 3.2 m bay between parked cars, nose out
3. PARALLEL PARK - after the junction stop line, reverse into a 7.2 x 2.8 m pocket on the right kerb
   (a control line marks the kerb)
4-7. SLALOM - four gates on the top road, alternate sides of the cones
Then the left road (crossing + stop line) to the FINAL SPOT, which unlocks only after tasks 1-7.
F1 on the Career screen unlocks everything.

## Test / screenshot flags
`--go career|custom|edit|settings|play:N|edit:N|file:levels/x.lvl`, `--frames N --shot out.ppm`,
`--gear D`, `--hold W` (pretend a key is held), `--arcade`, `--tool N`,
`--task N` (start at task N, 0-based), `--at X,Y,HEADING` (place the car).

## Tests
    ctest --test-dir build -C Release --output-on-failure
- `layout_safety` - Urban Safety Lab checks.
- `exam_autopilot` - drives the whole AUTODROM EXAM headlessly with the real physics and rules (must pass with
  0 penalty points), then checks the rules: final spot locked until all tasks are done, box entered forwards
  does not count, tasks out of order are ignored, a stop line still penalises driving straight through.
