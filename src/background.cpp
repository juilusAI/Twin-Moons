// Cenários de cada capítulo (céu, paralaxe, props) e o chão em perspectiva.
#include "common.h"

// ============================================================ desenho: cenário
void drawGradient(uint32_t top, uint32_t hor, int y1) {
    for (int y = 0; y < y1; y++) {
        uint32_t c = lerpC(top, hor, powf((float)y / y1, 1.4f));
        for (int x = 0; x < W; x++) fb[y * W + x] = c;
    }
}

void drawSky(uint32_t top, uint32_t hor) {
    drawGradient(lerpC(top, RGBc(70, 110, 190), dawn), lerpC(hor, RGBc(255, 170, 110), dawn), (int)FLOOR_TOP + 4);
    g_add = true;
    for (int i = 0; i < 140; i++) {
        uint32_t h = hash2(i, 77);
        float x = fmodf((h & 1023) * 0.7f - rcamX * 0.02f + 2000, (float)W), y = (float)((h >> 10) % 170);
        int a = (int)((90 + 90 * sinf(gameT * (1 + (h >> 20) % 4) + i)) * (1 - dawn));
        plot((int)x, (int)y, RGBc(220, 220, 255), a);
    }
    g_add = false;
    float mfade = 1 - dawn;
    // as duas luas
    float mx = 470 - rcamX * 0.03f, my = 78 + dawn * 120;
    glow(mx, my, 110, RGBc(110, 120, 200), (int)(70 * mfade));
    fillCircle(mx, my, 34, lerpC(RGBc(240, 238, 220), RGBc(200, 160, 160), dawn));
    fillCircle(mx - 10, my - 8, 6, RGBc(215, 212, 196), (int)(160 * mfade));
    fillCircle(mx + 9, my + 9, 9, RGBc(220, 216, 200), (int)(140 * mfade));
    fillCircle(mx + 14, my - 12, 4, RGBc(210, 206, 190), (int)(160 * mfade));
    float rx = 300 - rcamX * 0.012f, ry = 58 + dawn * 140;
    glow(rx, ry, 55, RGBc(220, 30, 40), (int)(90 * mfade));
    fillCircle(rx, ry, 13, RGBc(205, 55, 50), (int)(255 * mfade));
    fillCircle(rx + 3, ry - 2, 4, RGBc(170, 40, 40), (int)(200 * mfade));
    if (dawn > 0) {
        float sy = FLOOR_TOP + 30 - dawn * 110;
        glow(320, sy, 220, RGBc(255, 170, 80), (int)(160 * dawn));
        fillCircle(320, sy, 26, RGBc(255, 240, 200), (int)(255 * dawn));
    }
}

void drawRidges(float par, float shift, uint32_t col, float base, float a1, float f1, float a2, float f2, float a3, float f3, bool absMode) {
    for (int x = 0; x < W; x++) {
        float wx = x + rcamX * par + shift;
        float h = absMode ? base + a1 * sinf(wx * f1) + a2 * fabsf(sinf(wx * f2)) + a3 * fabsf(sinf(wx * f3 + 2))
                          : base + a1 * sinf(wx * f1) + a2 * sinf(wx * f2 + 1.3f) + a3 * sinf(wx * f3);
        for (int y = std::max(0, (int)(FLOOR_TOP - h)); y < (int)FLOOR_TOP; y++) fb[y * W + x] = col;
    }
}
void drawMist(uint32_t c, int alpha) {
    for (int y = (int)FLOOR_TOP - 40; y < (int)FLOOR_TOP; y++) {
        int a = (int)(alpha * (y - (FLOOR_TOP - 40)) / 40.f);
        hspan(y, 0, W, c, a);
    }
}

// chama f(i, sx) para cada objeto de cenário visível de uma camada com paralaxe
template <class F> static void propLoop(float par, float spacing, float shift, F f) {
    float base = rcamX * par + shift, step = spacing * par;
    int i0 = (int)floorf((base - W * 0.5f - 220) / step), i1 = (int)ceilf((base + W * 0.5f + 220) / step);
    for (int i = i0; i <= i1; i++) f(i, W * 0.5f + i * step - base);
}

void drawWisteria(float sx, uint32_t seed) {
    float by = FLOOR_TOP + 6;
    capsule(sx, by, sx + 6, by - 80, 6, 3, RGBc(26, 20, 28));
    capsule(sx + 4, by - 60, sx - 30, by - 92, 3, 1.5f, RGBc(26, 20, 28));
    capsule(sx + 5, by - 70, sx + 38, by - 96, 3, 1.5f, RGBc(26, 20, 28));
    for (int i = 0; i < 7; i++) {
        uint32_t h = hash2(seed, i);
        fillCircle(sx + (int)(h % 90) - 45.f, by - 88 - (h >> 8) % 8, 8 + (h >> 16) % 5, RGBc(46, 30, 66), 170);
    }
    for (int i = 0; i < 16; i++) {
        uint32_t h = hash2(seed, i + 50);
        float hx = sx + (int)(h % 100) - 50.f, hy = by - 90 + (float)((h >> 8) % 14);
        int len = 6 + (h >> 16) % 7;
        float sway = sinf(gameT * 1.5f + i) * 1.5f;
        for (int k = 0; k < len; k++) {
            float u = (float)k / len;
            fillCircle(hx + sway * u, hy + k * 3.2f, 3.2f * (1 - u * 0.6f), lerpC(RGBc(130, 80, 190), RGBc(225, 195, 255), u), 230);
        }
    }
    glow(sx, by - 80, 70, RGBc(120, 70, 170), 25);
}

void drawWeb(float cx, float cy, float r) {
    g_add = true;
    for (int k = 0; k < 8; k++) {
        float a = k / 8.f * 2 * PI;
        capsule(cx, cy, cx + cosf(a) * r, cy + sinf(a) * r, 0.4f, 0.4f, RGBc(200, 200, 230), 60);
    }
    for (int ring = 1; ring <= 4; ring++) ribbon(cx, cy, r * ring / 4.5f, 0.8f, 0, 2 * PI, 1, 0, 1, 1, RGBc(200, 200, 230), RGBc(200, 200, 230), 50, false);
    g_add = false;
}

void drawTorii(float sx) {
    float by = FLOOR_TOP + 4;
    uint32_t red = RGBc(150, 28, 30), dark = RGBc(30, 12, 14);
    fillRect(sx - 40, by - 95, 7, 95, red);
    fillRect(sx + 33, by - 95, 7, 95, red);
    fillRect(sx - 52, by - 104, 104, 7, dark);
    float px[4] = { sx - 60, sx + 60, sx + 52, sx - 52 }, py[4] = { by - 110, by - 110, by - 103, by - 103 };
    fillPoly(px, py, 4, dark);
    fillRect(sx - 46, by - 84, 92, 5, red);
    fillRect(sx - 4, by - 103, 8, 19, red);
}

void drawLantern(float sx) {
    float by = FLOOR_TOP + 5;
    uint32_t st = RGBc(60, 58, 70);
    fillRect(sx - 3, by - 22, 6, 22, st);
    fillRect(sx - 9, by - 26, 18, 4, st);
    fillRect(sx - 7, by - 38, 14, 12, st);
    float px[3] = { sx - 12, sx + 12, sx }, py[3] = { by - 38, by - 38, by - 48 };
    fillPoly(px, py, 3, st);
    fillRect(sx - 4, by - 35, 8, 6, RGBc(255, 190, 90));
    glow(sx, by - 32, 26, RGBc(255, 150, 60), (int)(70 + 15 * sinf(gameT * 7 + sx)));
}

void drawChochin(float x, float y) {
    capsule(x, y - 9, x, y - 14, 0.5f, 0.5f, RGBc(30, 20, 20));
    fillEllipse(x, y, 5, 7, RGBc(200, 40, 30));
    fillRect(x - 4, y - 7, 8, 1.5f, RGBc(40, 20, 20));
    fillRect(x - 4, y + 6, 8, 1.5f, RGBc(40, 20, 20));
    fillRect(x - 1, y - 3, 2, 5, RGBc(255, 210, 150), 200);
    glow(x, y, 22, RGBc(255, 90, 40), (int)(90 + 20 * sinf(gameT * 5 + x)));
}

void drawPine(float sx, float by, float h, uint32_t c) {
    for (int k = 0; k < 4; k++) {
        float w = h * (0.35f - k * 0.06f), y = by - h * k * 0.22f;
        float px[3] = { sx - w, sx + w, sx }, py[3] = { y, y, y - h * 0.4f };
        fillPoly(px, py, 3, c);
    }
}

void drawTownHouse(float sx, uint32_t seed) {
    float by = FLOOR_TOP + 4, w = 130, h1 = 52, h2 = 46;
    uint32_t wall = RGBc(48, 26, 20), roof = RGBc(26, 24, 34), lit = RGBc(255, 170, 90);
    fillRect(sx - w / 2, by - h1, w, h1, wall);
    // janelas de treliça acesas
    for (int k = 0; k < 4; k++) {
        float wx = sx - w / 2 + 8 + k * 31;
        bool on = (hash2(seed, k) & 3) != 0;
        fillRect(wx, by - h1 + 10, 24, 22, on ? lit : RGBc(60, 40, 30), on ? 170 : 255);
        for (int g = 1; g < 4; g++) fillRect(wx + g * 6, by - h1 + 10, 1, 22, RGBc(40, 20, 15));
        for (int g = 1; g < 3; g++) fillRect(wx, by - h1 + 10 + g * 7, 24, 1, RGBc(40, 20, 15));
    }
    float py1[4] = { by - h1 - 2, by - h1 - 2, by - h1 - 14, by - h1 - 14 }, px1[4] = { sx - w / 2 - 12, sx + w / 2 + 12, sx + w / 2 - 4, sx - w / 2 + 4 };
    fillPoly(px1, py1, 4, roof);
    fillRect(sx - w / 2 + 6, by - h1 - 14 - h2, w - 12, h2, mulC(wall, 0.85f));
    for (int k = 0; k < 3; k++) {
        float wx = sx - w / 2 + 16 + k * 36;
        bool on = (hash2(seed, k + 9) & 1) != 0;
        fillRect(wx, by - h1 - 14 - h2 + 10, 26, 20, on ? lit : RGBc(60, 40, 30), on ? 150 : 255);
        for (int g = 1; g < 4; g++) fillRect(wx + g * 6.5f, by - h1 - 14 - h2 + 10, 1, 20, RGBc(40, 20, 15));
    }
    float top = by - h1 - 14 - h2;
    float px2[4] = { sx - w / 2 - 8, sx + w / 2 + 8, sx + w / 2 - 14, sx - w / 2 + 14 }, py2[4] = { top, top, top - 18, top - 18 };
    fillPoly(px2, py2, 4, roof);
    glow(sx, by - h1 + 20, 60, RGBc(255, 140, 60), 30);
    drawChochin(sx - w / 2 + 12, by - h1 + 6);
    drawChochin(sx + w / 2 - 12, by - h1 + 6);
}

void drawCityBuilding(float sx, uint32_t seed, float nearLayer) {
    float by = FLOOR_TOP + 4;
    float w = 70 + (seed % 40), h = 80 + ((seed >> 8) % 70) * (nearLayer > 0.5f ? 1.f : 0.7f);
    uint32_t wall = nearLayer > 0.5f ? RGBc(60, 36, 34) : RGBc(26, 22, 36);
    fillRect(sx - w / 2, by - h, w, h, lerpC(wall, RGBc(150, 110, 110), dawn * 0.5f));
    // topo destruído
    float px[4] = { sx - w / 2, sx - w / 2 + w * 0.3f, sx - w / 2 + w * 0.5f, sx - w / 2 }, py[4] = { by - h, by - h, by - h + 16, by - h + 26 };
    if (seed & 1) fillPoly(px, py, 4, nearLayer > 0.5f ? RGBc(20, 16, 26) : RGBc(16, 14, 30));
    if (nearLayer > 0.5f) {
        for (int r = 0; r < (int)(h / 22); r++)
            for (int c = 0; c < (int)(w / 18); c++) {
                uint32_t hh = hash2(seed + c, r);
                bool on = (hh & 7) == 0;
                fillRect(sx - w / 2 + 6 + c * 18, by - h + 10 + r * 22, 9, 12, on ? RGBc(255, 190, 110) : RGBc(30, 20, 26), on ? 200 : 255);
            }
    }
}

void drawGasLamp(float sx) {
    float by = FLOOR_TOP + 6;
    fillRect(sx - 1.5f, by - 60, 3, 60, RGBc(30, 30, 36));
    fillRect(sx - 5, by - 70, 10, 10, RGBc(40, 40, 50));
    fillRect(sx - 3, by - 68, 6, 6, RGBc(255, 220, 150));
    glow(sx, by - 65, 40, RGBc(255, 180, 90), 80);
}

void drawBackground() {
    int bg = curBG();
    switch (bg) {
    case BG_FOREST:
        fogC = RGBc(22, 24, 48);
        drawSky(RGBc(6, 6, 20), RGBc(58, 34, 80));
        drawRidges(0.12f, 0, lerpC(RGBc(28, 22, 56), RGBc(130, 100, 130), dawn), 70, 28, 0.009f, 14, 0.023f, 6, 0.061f, false);
        drawRidges(0.32f, 0, lerpC(RGBc(16, 14, 34), RGBc(80, 60, 90), dawn), 34, 10, 0.02f, 7, 0.11f, 5, 0.31f, true);
        drawMist(lerpC(RGBc(90, 70, 130), RGBc(255, 200, 170), dawn), 70);
        propLoop(0.82f, 115, 0, [](int i, float sx) {
            uint32_t h = hash2(i, 9);
            int kind = (int)(h % 7);
            if (i % 9 == 4) drawTorii(sx);
            else if (kind < 4) { drawWisteria(sx, (uint32_t)i); if (kind == 1) drawWeb(sx + 40, FLOOR_TOP - 40, 28); }
            else if (kind == 4) drawLantern(sx);
            else if (kind == 5) drawWeb(sx, FLOOR_TOP - 50, 34);
        });
        break;
    case BG_TRAIN: {
        fogC = RGBc(20, 22, 44);
        float sc = gameT * 700;
        drawSky(RGBc(8, 8, 24), RGBc(50, 40, 80));
        drawRidges(0.12f, sc * 0.03f, RGBc(26, 24, 54), 64, 26, 0.008f, 12, 0.025f, 6, 0.06f, false);
        drawRidges(0.3f, sc * 0.12f, RGBc(16, 16, 34), 30, 9, 0.02f, 8, 0.09f, 4, 0.33f, true);
        drawMist(RGBc(90, 80, 140), 60);
        propLoop(0.6f, 60, sc * 0.6f, [](int i, float sx) { uint32_t h = hash2(i, 21); drawPine(sx, FLOOR_TOP + 4, 50 + h % 40, RGBc(12, 16, 26)); });
        propLoop(0.85f, 260, sc * 0.85f, [](int, float sx) {
            fillRect(sx - 2, FLOOR_TOP - 90, 4, 94, RGBc(30, 24, 22));
            fillRect(sx - 14, FLOOR_TOP - 84, 28, 3, RGBc(30, 24, 22));
        });
        // fumaça da locomotiva
        for (int k = 0; k < 14; k++) {
            float u = fmodf(gameT * 0.5f + k / 14.f, 1.f);
            float x = W + 40 - u * (W + 160), y = 60 + sinf(k * 1.7f) * 20 - u * 30;
            float r = 10 + u * 22;
            fillCircle(x, y, r, RGBc(70, 66, 84), (int)(35 * (1 - u)));
            fillCircle(x, y, r * 0.6f, RGBc(80, 76, 94), (int)(35 * (1 - u)));
        }
    } break;
    case BG_TOWN:
        fogC = RGBc(40, 18, 28);
        drawSky(RGBc(10, 6, 20), RGBc(110, 40, 50));
        drawRidges(0.15f, 0, RGBc(30, 16, 34), 50, 14, 0.012f, 8, 0.03f, 4, 0.08f, false);
        for (int x = 0; x < W; x++) { // silhueta de telhados distantes
            float wx = x + rcamX * 0.35f;
            float h = 40 + 12 * fabsf(sinf(wx * 0.025f)) + ((int)(wx / 50) % 3) * 8;
            for (int y = std::max(0, (int)(FLOOR_TOP - h)); y < (int)FLOOR_TOP; y++) fb[y * W + x] = RGBc(22, 12, 22);
        }
        drawMist(RGBc(160, 60, 60), 60);
        propLoop(0.82f, 150, 0, [](int i, float sx) { drawTownHouse(sx, (uint32_t)i * 7 + 3); });
        break;
    case BG_CASTLE: {
        fogC = RGBc(30, 16, 10);
        drawGradient(RGBc(14, 8, 8), RGBc(60, 34, 22), (int)FLOOR_TOP + 4);
        // paredes de shoji em vários andares (paralaxe lenta)
        for (int row = 0; row < 4; row++) {
            float par = 0.18f + row * 0.07f, y0 = 8 + row * 46.f;
            float off = fmodf(rcamX * par + row * 37, 46.f);
            for (float x = -off - 46; x < W + 46; x += 46) {
                uint32_t h = hash2((int)((x + rcamX * par) / 46), row);
                bool on = (h & 3) != 0;
                fillRect(x + 3, y0, 40, 38, on ? RGBc(200, 130, 70) : RGBc(50, 30, 20), on ? 120 : 255);
                fillRect(x + 22, y0, 2, 38, RGBc(40, 22, 14));
                fillRect(x + 3, y0 + 18, 40, 2, RGBc(40, 22, 14));
            }
            fillRect(0, y0 + 38, W, 6, RGBc(36, 20, 12));
        }
        propLoop(0.6f, 170, 0, [](int i, float sx) {
            uint32_t h = hash2(i, 33);
            fillRect(sx - 8, 0, 16, FLOOR_TOP + 4, RGBc(34, 18, 10));
            fillRect(sx - 10, 0, 20, 4, RGBc(20, 10, 6));
            if (h % 3 == 0) { // escadas flutuando de cabeça para baixo
                for (int k = 0; k < 6; k++) fillRect(sx + 20 + k * 10, 60 + k * 8 + (h % 40), 12, 4, RGBc(70, 40, 24));
            }
            if (h % 2 == 0) { capsule(sx + 40, 0, sx + 40, 40, 0.6f, 0.6f, RGBc(20, 10, 6)); drawChochin(sx + 40, 48); }
        });
        drawMist(RGBc(120, 60, 30), 50);
    } break;
    case BG_CITY:
        fogC = RGBc(22, 20, 36);
        drawSky(RGBc(10, 10, 30), RGBc(70, 40, 70));
        propLoop(0.3f, 80, 0, [](int i, float sx) { drawCityBuilding(sx, hash2(i, 55), 0.f); });
        for (int k = 0; k < 4; k++) glow(fmodf(120 + k * 190 - rcamX * 0.35f + 4000, (float)W), FLOOR_TOP - 10, 60, RGBc(255, 90, 30), 40);
        drawMist(lerpC(RGBc(90, 60, 90), RGBc(255, 190, 160), dawn), 70);
        propLoop(0.75f, 110, 0, [](int i, float sx) { drawCityBuilding(sx, hash2(i, 77), 1.f); });
        propLoop(0.85f, 230, 0, [](int, float sx) { drawGasLamp(sx); });
        break;
    }
}

void drawFloor() {
    int bg = curBG();
    for (int y = (int)FLOOR_TOP; y < H; y++) {
        float s = (y + 0.5f - shakeY - HORIZON) / KY;
        float z = (s - S_FAR) / (S_NEAR - S_FAR) * DEPTH;
        float fog = clampf(1 - z / DEPTH, 0, 1) * 0.55f;
        float moon = 0.85f + 0.25f * (z / DEPTH);
        for (int x = 0; x < W; x++) {
            float wx0 = rcamX + (x + 0.5f - W * 0.5f) / s;
            uint32_t c;
            switch (bg) {
            default: case BG_FOREST: {
                int tz = (int)floorf(z / 14);
                float fz = z / 14 - tz, wx = wx0 + (tz & 1) * 24;
                int tx = (int)floorf(wx / 48);
                float fx = wx / 48 - tx;
                uint32_t h = hash2(tx, tz);
                int v = (int)(h & 15) - 8;
                if (z > 18 && z < 88) {
                    c = RGBc(62 + v, 60 + v, 76 + v);
                    if (fx < 0.04f || fz < 0.08f) c = RGBc(30, 30, 40);
                    else if ((h >> 8) % 11 == 0) c = RGBc(50 + v, 70 + v, 58 + v);
                } else {
                    uint32_t g = hash2((int)(wx * 0.5f), y);
                    c = RGBc(24 + (int)(g & 7), 34 + (int)((g >> 4) & 9), 36 + (int)((g >> 8) & 7));
                }
            } break;
            case BG_TRAIN: {
                float fz = fmodf(z, 12) / 12;
                int v = (int)(hash2((int)floorf(wx0 / 90), (int)(z / 12)) & 7);
                c = RGBc(74 + v, 34 + v / 2, 34);
                if (fz < 0.1f) c = RGBc(110, 58, 52);
                if (fmodf(wx0 + 4000, 45) < 1.8f && fz > 0.4f && fz < 0.6f) c = RGBc(150, 120, 100);
                if (fmodf(wx0 + 4000, 360) < 3) c = RGBc(28, 14, 14);
                if (z < 5 || z > 95) c = RGBc(32, 16, 16);
            } break;
            case BG_TOWN: {
                uint32_t g = hash2((int)(wx0 * 0.7f), y);
                c = RGBc(66 + (int)(g & 7), 48 + (int)((g >> 3) & 7), 40 + (int)((g >> 6) & 5));
                if (z < 16) { c = RGBc(88, 58, 38); if (fmodf(wx0 + 4000, 12) < 1) c = RGBc(50, 30, 20); }
                else if (z < 18) c = RGBc(40, 24, 18);
                if (fabsf(fmodf(z, 33) - 16) < 1.5f && z > 20) c = mulC(c, 0.85f);
            } break;
            case BG_CASTLE: {
                int pz = (int)floorf(z / 7);
                float fz = z / 7 - pz;
                float off = (float)(hash2(pz, 5) % 120);
                int seg = (int)floorf((wx0 + off) / 120);
                uint32_t h = hash2(seg, pz);
                int v = (int)(h & 15) - 8;
                uint32_t grain = hash2((int)(wx0 / 3), pz) & 7;
                c = RGBc(96 + v + (int)grain, 60 + v / 2 + (int)grain / 2, 34 + v / 3);
                if (fz < 0.08f || fmodf(wx0 + off + 12000, 120) < 1.2f) c = RGBc(40, 22, 12);
                float refl = 1 + 0.2f * (1 - fabsf(z - 50) / 50);
                c = mulC(c, refl);
            } break;
            case BG_CITY: {
                int tz = (int)floorf(z / 8);
                float wx = wx0 + (tz & 1) * 13;
                int tx = (int)floorf(wx / 26);
                float fx = wx / 26 - tx, fz = z / 8 - tz;
                uint32_t h = hash2(tx, tz);
                int v = (int)(h & 15) - 8;
                c = RGBc(64 + v, 62 + v, 70 + v);
                if (fx < 0.07f || fz < 0.1f) c = RGBc(32, 30, 36);
                if ((h >> 6) % 23 == 0) c = RGBc(40, 36, 40);
            } break;
            }
            c = mulC(c, moon);
            c = lerpC(c, fogC, fog);
            if (dawn > 0) c = lerpC(c, mulC(c, 2.2f), dawn * 0.6f);
            fb[y * W + x] = c;
        }
    }
}

