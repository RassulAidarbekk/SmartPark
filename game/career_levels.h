// SmartPark - career_levels.h
// built-in career levels (incl. AUTODROM EXAM)
// author: Alijan
// NOTE: not a standalone header - included only from main.cpp, in a fixed order.
#pragma once

// ----------------------------------------------------------------------------
//  Built-in career levels (made by us, the devs)
// ----------------------------------------------------------------------------
static vector<Level> CAREER;
static void add(Level& L, int t, float x, float y, float w, float h, float r, int c = 0) { L.items.push_back({t, x, y, w, h, r, c}); }
static void wall(Level& L, float x1, float y1, float x2, float y2) { add(L, T_WALL, (x1 + x2) / 2, (y1 + y2) / 2, x2 - x1, y2 - y1, 0); }
static void pcar(Level& L, float x, float y, float r, int c) { add(L, T_PCAR, x, y, 4.2f, 1.9f, r, c); }
static void spot(Level& L, float x, float y, float r) { add(L, T_SPOT, x, y, 5.4f, 2.8f, r); }
static void startp(Level& L, float x, float y, float r) { add(L, T_START, x, y, 4.2f, 1.9f, r); }
static void cone(Level& L, float x, float y) { add(L, T_CONE, x, y, 0.8f, 0.8f, 0); }
static void tree(Level& L, float x, float y) { add(L, T_TREE, x, y, 2, 2, 0); }
static void zone(Level& L, int t, float x1, float y1, float x2, float y2, int c = 0) { add(L, t, (x1 + x2) / 2, (y1 + y2) / 2, x2 - x1, y2 - y1, 0, c); }
static void arc(Level& L, int t, float cx, float cy, float r, float a0, float sw, int c = 0) { add(L, t, cx, cy, r, sw, a0, c); }
static void ctrlH(Level& L, float x1, float x2, float y) { add(L, T_CTRLLINE, (x1 + x2) / 2, y, x2 - x1, LINE_T, 0); }
static void stopl(Level& L, float x, float y, float across, float dirDeg) { add(L, T_STOPLINE, x, y, 0.4f, across, dirDeg); }
static void coneArc(Level& L, float cx, float cy, float r, float a0, float sw, float spacing = 2.f);
static void task(Level& L, int kind, float x, float y, float len, float wid, float headingDeg) { add(L, T_TASK, x, y, len, wid, headingDeg, kind); }

static void buildCareer() {
    { Level L; L.name = "WARM UP"; L.W = 40; L.H = 24; L.par = 30;
      startp(L, 6, 12, 0); spot(L, 32, 12, 0);
      pcar(L, 32, 8.4f, 0, 0); pcar(L, 32, 15.6f, 0, 2);
      cone(L, 16, 8); cone(L, 16, 16); cone(L, 24, 12); tree(L, 20, 4); tree(L, 20, 20); tree(L, 8, 21);
      CAREER.push_back(L); }
    { Level L; L.name = "PERPENDICULAR"; L.par = 50;
      startp(L, 7, 17, 0); spot(L, 19.4f, 4.7f, 90);
      float xs[] = {9.8f, 13, 16.2f, 22.6f, 25.8f, 29, 32.2f};
      for (int i = 0; i < 7; i++) pcar(L, xs[i], 4.7f, 90, (i * 3) % 8);
      cone(L, 38, 20); cone(L, 38, 10); tree(L, 3, 4); tree(L, 3, 22); tree(L, 41, 15);
      CAREER.push_back(L); }
    { Level L; L.name = "PARALLEL"; L.par = 60;
      wall(L, 0, 22.8f, 44, 26); wall(L, 0, 0, 44, 2.2f);
      startp(L, 6, 13, 0); spot(L, 22, 21.2f, 0);
      float xs[] = {7.5f, 12.2f, 16.5f, 27.7f, 32.4f, 37};
      for (int i = 0; i < 6; i++) pcar(L, xs[i], 21.2f, 0, (i * 5 + 1) % 8);
      pcar(L, 12, 4.6f, 180, 4); pcar(L, 24, 4.6f, 180, 1); pcar(L, 36, 4.6f, 180, 5);
      cone(L, 10, 9); cone(L, 30, 12); tree(L, 41, 12);
      CAREER.push_back(L); }
    { Level L; L.name = "TIGHT GARAGE"; L.par = 60;
      wall(L, 32.2f, 7, 32.8f, 16); wall(L, 37.2f, 7, 37.8f, 16); wall(L, 32.2f, 15.4f, 37.8f, 16);
      wall(L, 27.2f, 7, 27.8f, 16); wall(L, 27.2f, 15.4f, 32.8f, 16); pcar(L, 30, 12.2f, 90, 0);
      wall(L, 41.2f, 7, 41.8f, 16); wall(L, 37.2f, 15.4f, 41.8f, 16); pcar(L, 39.5f, 12.2f, 90, 2);
      startp(L, 6, 4, 0); spot(L, 35, 12.2f, 90);
      add(L, T_BARRIER, 16, 12, 6.f, 0.7f, 90); cone(L, 22, 4); cone(L, 22, 18); tree(L, 8, 20); tree(L, 12, 22);
      CAREER.push_back(L); }
    { Level L; L.name = "SLALOM ON ICE"; L.par = 55;
      zone(L, T_ICE, 27, 3, 43, 23);
      startp(L, 5, 20, 0); spot(L, 38, 12, 0); pcar(L, 38, 8.4f, 0, 1); pcar(L, 38, 15.6f, 0, 6);
      cone(L, 11, 16); cone(L, 14, 10); cone(L, 17, 16); cone(L, 20, 10); cone(L, 23, 16);
      wall(L, 25, 3, 26, 9); tree(L, 8, 5); tree(L, 31, 21); tree(L, 41, 4);
      CAREER.push_back(L); }
    { Level L; L.name = "RUSH HOUR"; L.par = 70;
      for (int k = 0; k < 10; k++) {
          float x = 6.4f + 3.2f * k;
          pcar(L, x, 4.7f, 90, (k * 3 + 2) % 8);
          if (k == 6) spot(L, x, 21.3f, 90); else pcar(L, x, 21.3f, 90, (k * 5 + 1) % 8);
      }
      wall(L, 10, 11.4f, 34, 13.2f); zone(L, T_MUD, 18, 13.2f, 26, 19.2f);
      startp(L, 3.5f, 15, 0); cone(L, 30, 16); cone(L, 14, 9); cone(L, 40, 12); tree(L, 41, 4); tree(L, 41, 22);
      CAREER.push_back(L); }
    { Level L; L.name = "HILL START"; L.W = 56; L.H = 30; L.par = 60;
      zone(L, T_GRASS, 0, 0, 56, 7); zone(L, T_GRASS, 0, 23, 56, 30);
      ctrlH(L, 0, 56, 7.6f); ctrlH(L, 0, 56, 22.4f);
      zone(L, T_HILL, 16, 7.6f, 27, 22.4f, 0);      // uphill towards +x
      zone(L, T_HILL, 27, 7.6f, 33, 22.4f, 2);      // downhill side
      stopl(L, 25.2f, 15, 14.4f, 0);
      add(L, T_CROSSING, 39, 15, 14.4f, 4.f, 90);
      stopl(L, 35.8f, 15, 14.4f, 0);
      startp(L, 5, 15, 0); spot(L, 50, 15, 0); pcar(L, 50, 11.3f, 0, 3); pcar(L, 50, 18.7f, 0, 5);
      tree(L, 8, 3); tree(L, 20, 26); tree(L, 34, 3); tree(L, 46, 26);
      CAREER.push_back(L); }
    { Level L; L.name = "ROUNDABOUT"; L.W = 64; L.H = 44; L.par = 60;
      float cx = 32, cy = 22;
      add(L, T_ISLAND, cx, cy, 10, 10, 0);
      arc(L, T_ARCCURB, cx, cy, 14, 200, 140); arc(L, T_ARCCURB, cx, cy, 14, 20, 140);
      arc(L, T_ARCLINE, cx, cy, 9.5f, 0, 360, 1);
      coneArc(L, cx, cy, 17, 205, 130, 2.6f); coneArc(L, cx, cy, 17, 25, 130, 2.6f);
      startp(L, 5, cy, 0); spot(L, 56, cy, 0); pcar(L, 56, cy - 3.7f, 0, 1); pcar(L, 56, cy + 3.7f, 0, 4);
      ctrlH(L, 0, 15, cy - 4.5f); ctrlH(L, 0, 15, cy + 4.5f);
      tree(L, 4, 4); tree(L, 60, 4); tree(L, 4, 40); tree(L, 60, 40); tree(L, cx, cy);
      CAREER.push_back(L); }
    { Level L; L.name = "AUTODROM EXAM"; L.W = 150; L.H = 100; L.par = 260;
      // ---- layout: a 12 m wide ring road (counter-clockwise) around a grass field, 2 m grass verge outside.
      //      bottom road y 86..98 (drive +x), right road x 136..148 (drive up), top road y 2..14 (drive -x),
      //      left road x 2..14 (drive down). Two pads are cut out of the field: the reverse box and the final bay.
      zone(L, T_GRASS, 0, 0, 150, 2); zone(L, T_GRASS, 0, 98, 150, 100);
      zone(L, T_GRASS, 0, 2, 2, 98);  zone(L, T_GRASS, 148, 2, 150, 98);
      zone(L, T_GRASS, 14, 14, 136, 42);                                   // field, minus the final-bay pad (x14..22, y42..58)
      zone(L, T_GRASS, 22, 42, 136, 58);
      zone(L, T_GRASS, 14, 58, 136, 76);                                   // field, minus the box pad (x96..114, y76..86)
      zone(L, T_GRASS, 14, 76, 96, 86); zone(L, T_GRASS, 114, 76, 136, 86);
      // lane paint (decoration only)
      add(L, T_LINE, 19, 92, 10, 0.2f, 0, 1); add(L, T_LINE, 54, 92, 8, 0.2f, 0, 1); add(L, T_LINE, 82, 92, 24, 0.2f, 0, 1);
      add(L, T_LINE, 126, 92, 20, 0.2f, 0, 1);
      add(L, T_LINE, 141, 26, 24, 0.2f, 90, 1); add(L, T_LINE, 141, 79, 14, 0.2f, 90, 1);
      add(L, T_LINE, 8, 20, 12, 0.2f, 90, 1); add(L, T_LINE, 8, 62, 44, 0.2f, 90, 1);
      startp(L, 7, 92, 0);

      // ---- TASK 1: hill start (estakada). Stop on the uphill before the STOP line, move off without rolling back
      zone(L, T_HILL, 24, 86, 38, 98, 0); zone(L, T_HILL, 38, 86, 46, 98, 2);
      stopl(L, 36.6f, 92, 12, 0);
      task(L, TK_STOP, 31.0f, 92, 6.2f, 11.f, 0);
      // pedestrian crossing with its stop line
      add(L, T_CROSSING, 64, 92, 12, 4, 90); stopl(L, 60.6f, 92, 12, 0);

      // ---- TASK 2: reverse into a box (perpendicular bay 6.0 x 3.2, nose must point back at the road)
      for (int b = 0; b < 5; b++) add(L, T_BAY, 98.6f + 3.2f * b, 83, 6.f, 3.2f, 90);
      pcar(L, 98.6f, 83, 90, 3); pcar(L, 101.8f, 83, 90, 6); pcar(L, 108.2f, 83, 90, 0); pcar(L, 111.4f, 83, 90, 4);
      add(L, T_CURB, 105, 79.4f, 18, 0.5f, 0);
      task(L, TK_BOX, 105, 83, 6.0f, 3.2f, 90);

      // ---- right road: junction stop line, then TASK 3: parallel parking in reverse (pocket 7.2 x 2.8 on the right kerb)
      stopl(L, 142, 72, 12, 270);
      pcar(L, 146.3f, 44.f, 270, 1); pcar(L, 146.3f, 56.f, 270, 5);
      add(L, T_CTRLLINE, 147.85f, 50, 26, LINE_T, 90);                     // kerb line: wheel on it = 20 points
      add(L, T_LINE, 144.6f, 50, 22, 0.2f, 90, 1);                         // parking-lane edge paint
      task(L, TK_PARALLEL, 146.3f, 50, 7.2f, 2.8f, 270);

      // ---- TASKS 4-7: slalom gates on the top road (pass each gate in order, cones are the gate posts)
      const float gx[4] = {116, 98, 80, 62}, gy[4] = {5.0f, 11.0f, 5.0f, 11.0f};
      for (int g = 0; g < 4; g++) {
          cone(L, gx[g], 8.0f);                                             // post on the lane centre: pass right, left, right, left
          task(L, TK_GATE, gx[g], gy[g], 2.5f, 5.0f, 180);
      }

      // ---- left road: pedestrian crossing, then the final bay (TARGET SPOT, locked until every task is done)
      add(L, T_CROSSING, 8, 30, 12, 4, 0); stopl(L, 8, 26.4f, 12, 90);
      add(L, T_BAY, 18.4f, 47.2f, 5.4f, 2.8f, 0); add(L, T_BAY, 18.4f, 52.8f, 5.4f, 2.8f, 0);
      pcar(L, 18.4f, 47.2f, 0, 2); pcar(L, 18.4f, 52.8f, 0, 7);
      add(L, T_CURB, 21.6f, 50, 14, 0.5f, 90);
      spot(L, 18.4f, 50, 0);

      tree(L, 40, 30); tree(L, 70, 50); tree(L, 100, 30); tree(L, 122, 66); tree(L, 58, 70); tree(L, 32, 62);
      tree(L, 86, 22); tree(L, 120, 46); tree(L, 46, 48);
      CAREER.push_back(L); }
}
