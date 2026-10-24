// Desenho dos efeitos: cortes, dragões, partículas e ataques dos chefes.
#include "common.h"

// ============================================================ desenho: efeitos
void drawSlash(const Slash& sl) {
    float s = zScale(sl.z);
    float cx = projX(sl.x, sl.z), cy = groundY(sl.z) - sl.y * s;
    float p = sl.t / sl.dur;
    float head = std::min(1.f, p / 0.35f), tail = std::max(0.f, (p - 0.2f) / 0.8f);
    float a0 = sl.a0 + (sl.a1 - sl.a0) * tail, a1 = sl.a0 + (sl.a1 - sl.a0) * head;
    if (fabsf(a1 - a0) < 0.02f) return;
    int alpha = (int)(255 * (1 - p * p));
    g_add = true;
    switch (sl.style) {
    case SL_BLADE:
        if (akatoT > 0) ribbon(cx, cy, sl.r, sl.w, a0, a1, sl.squash, sl.tilt, sl.face, s, RGBc(200, 30, 10), RGBc(255, 220, 160), alpha, true);
        else if (chapter >= 3) ribbon(cx, cy, sl.r, sl.w, a0, a1, sl.squash, sl.tilt, sl.face, s, RGBc(200, 80, 20), RGBc(255, 240, 200), alpha, true);
        else ribbon(cx, cy, sl.r, sl.w, a0, a1, sl.squash, sl.tilt, sl.face, s, RGBc(60, 110, 200), RGBc(230, 245, 255), alpha, true);
        break;
    case SL_WATER:
        ribbon(cx, cy, sl.r, sl.w * 1.6f, a0, a1, sl.squash, sl.tilt, sl.face, s, RGBc(10, 50, 160), RGBc(40, 130, 255), alpha, true);
        ribbon(cx, cy, sl.r, sl.w, a0, a1, sl.squash, sl.tilt, sl.face, s, RGBc(30, 90, 220), RGBc(110, 200, 255), alpha, true);
        ribbon(cx, cy, sl.r + sl.w * 0.25f, sl.w * 0.3f, a0, a1, sl.squash, sl.tilt, sl.face, s, RGBc(120, 180, 255), RGBc(255, 255, 255), alpha, true);
        for (int k = 0; k < 4; k++) {
            float u = 0.3f + k * 0.18f;
            if (u > head) break;
            float a = sl.a0 + (sl.a1 - sl.a0) * u;
            float lx = cosf(a) * sl.r, ly = sinf(a) * sl.r * sl.squash;
            float px = cx + lx * s * sl.face, py = cy + ly * s;
            ribbon(px, py, sl.w * 0.55f, sl.w * 0.3f, a, a + 4.5f, 1, 0, sl.face, s, RGBc(60, 140, 255), RGBc(230, 245, 255), alpha / 2, true);
        }
        break;
    case SL_FIRE:
        ribbon(cx, cy, sl.r, sl.w * 1.7f, a0, a1, sl.squash, sl.tilt, sl.face, s, RGBc(120, 10, 0), RGBc(255, 80, 0), alpha, true);
        ribbon(cx, cy, sl.r, sl.w, a0, a1, sl.squash, sl.tilt, sl.face, s, RGBc(220, 60, 0), RGBc(255, 180, 40), alpha, true);
        ribbon(cx, cy, sl.r, sl.w * 0.35f, a0, a1, sl.squash, sl.tilt, sl.face, s, RGBc(255, 200, 80), RGBc(255, 255, 220), alpha, true);
        break;
    case SL_CLAW:
        for (int k = -1; k <= 1; k++)
            ribbon(cx, cy + k * 4 * s, sl.r + k * 3, 3, a0, a1, sl.squash, sl.tilt, sl.face, s, RGBc(120, 0, 20), RGBc(255, 80, 80), alpha, true);
        break;
    case SL_FIST:
        glow(cx, cy, 22 * s * (0.5f + p), RGBc(140, 190, 255), alpha / 2);
        ribbon(cx, cy, sl.r * (1 + p * 2), 2.5f, 0, 2 * PI, 1, 0, 1, s, RGBc(170, 210, 255), RGBc(170, 210, 255), alpha, false);
        break;
    }
    g_add = false;
}

void drawDragon() {
    if (dragon.size() < 2 || dragonFade <= 0) return;
    g_add = true;
    uint32_t cOut = dragonFire ? RGBc(170, 40, 0) : RGBc(10, 50, 170), cMid = dragonFire ? RGBc(255, 130, 20) : RGBc(40, 130, 255);
    uint32_t cCore = dragonFire ? RGBc(255, 240, 180) : RGBc(200, 235, 255), cFin = dragonFire ? RGBc(255, 180, 60) : RGBc(80, 170, 255);
    int n = (int)dragon.size();
    for (int i = 0; i < n; i++) {
        float u = (float)i / (n - 1);
        const V3& d = dragon[i];
        float s = zScale(d.z);
        float wob = sinf(i * 0.45f - gameT * 12) * 7 * u;
        float sx = projX(d.x, d.z), sy = groundY(d.z) - (d.y + wob) * s;
        float r = (3 + 11 * u) * s;
        int a = (int)(200 * dragonFade * (0.3f + 0.7f * u));
        fillCircle(sx, sy, r * 1.6f, cOut, a / 2);
        fillCircle(sx, sy, r, cMid, a);
        fillCircle(sx, sy - r * 0.3f, r * 0.45f, cCore, a);
        if (i % 4 == 0 && u > 0.2f) capsule(sx, sy - r, sx - P.face * r * 1.4f, sy - r * 2.4f, r * 0.35f, 0.5f, cFin, a);
    }
    const V3& h = dragon.back();
    const V3& h2 = dragon[n - 2];
    int dir = h.x >= h2.x ? 1 : -1;
    float s = zScale(h.z);
    float hx = projX(h.x, h.z) + dir * 10 * s, hy = groundY(h.z) - h.y * s;
    int a = (int)(230 * dragonFade);
    fillEllipse(hx, hy, 17 * s, 11 * s, cMid, a);
    fillEllipse(hx + dir * 12 * s, hy + 3 * s, 11 * s, 6 * s, cMid, a);
    capsule(hx - dir * 4 * s, hy - 8 * s, hx - dir * 20 * s, hy - 22 * s, 2 * s, 0.6f, cCore, a);
    capsule(hx + dir * 2 * s, hy - 9 * s, hx - dir * 10 * s, hy - 26 * s, 2 * s, 0.6f, cCore, a);
    fillCircle(hx + dir * 6 * s, hy - 3 * s, 2.5f * s, RGBc(255, 255, 255), a);
    glow(hx, hy, 50 * s, cMid, (int)(120 * dragonFade));
    g_add = false;
}

void drawParticle(const Part& p) {
    float s = zScale(p.z);
    float sx = projX(p.x, p.z), sy = groundY(p.z) - p.y * s;
    float u = p.life / p.max;
    switch (p.kind) {
    case P_SPARK:
        g_add = true;
        capsule(sx, sy, sx - p.vx * 0.03f * s, sy + p.vy * 0.03f * s, p.size * s, 0.3f, p.c, (int)(255 * u));
        g_add = false; break;
    case P_DOT:
        fillCircle(sx, sy, p.size * s * (p.y <= 0 ? 1.3f : 1.f), p.c, (int)(255 * std::min(1.f, u * 2)));
        break;
    case P_WATER: case P_HEAL:
        g_add = true;
        fillCircle(sx, sy, p.size * s * u + 0.5f, p.c, (int)(220 * u));
        fillCircle(sx, sy, p.size * s * u * 0.4f + 0.3f, RGBc(220, 240, 255), (int)(200 * u));
        g_add = false; break;
    case P_FIRE: {
        g_add = true;
        uint32_t c = p.c ? lerpC(RGBc(120, 20, 60), p.c, u) : lerpC(RGBc(170, 20, 0), RGBc(255, 220, 110), u);
        fillCircle(sx, sy, p.size * s * (0.4f + u), c, (int)(230 * u));
        g_add = false;
    } break;
    case P_ASH:
        if (p.c == RGBc(255, 120, 40)) { g_add = true; fillCircle(sx, sy, p.size * s * 0.7f, p.c, (int)(255 * u)); g_add = false; }
        else fillRect(sx, sy, p.size * s, p.size * s, p.c, (int)(220 * u));
        break;
    case P_RING: {
        g_add = true;
        float r = p.size * (1.6f - u) * s;
        ribbon(sx, sy, r, 3 * u + 1, 0, 2 * PI, p.y > 1 ? 1.f : 0.32f, 0, 1, 1, p.c, p.c, (int)(220 * u), false);
        g_add = false;
    } break;
    }
}

void floorBand(float x0, float x1, float z, float hz, uint32_t c, int a) {
    float px[4] = { projX(x0, z - hz), projX(x1, z - hz), projX(x1, z + hz), projX(x0, z + hz) };
    float py[4] = { groundY(z - hz), groundY(z - hz), groundY(z + hz), groundY(z + hz) };
    fillPoly(px, py, 4, c, a);
}

void drawProj(const Proj& pr) {
    float s = zScale(pr.z);
    float sx = projX(pr.x, pr.z), sy = groundY(pr.z) - pr.y * s, gy = groundY(pr.z);
    float pulse = 0.5f + 0.5f * sinf(pr.t * 30);
    switch (pr.kind) {
    case PR_THREAD: case PR_OBI: {
        float dx = pr.vx, dz = pr.vz, l = sqrtf(dx * dx + dz * dz);
        float len = pr.kind == PR_OBI ? 70.f : 55.f;
        float ex = projX(pr.x - dx / l * len, pr.z - dz / l * len), ey = groundY(pr.z - dz / l * len) - pr.y * s;
        fillEllipse(sx, gy, 8 * s, 2 * s, 0, 70);
        if (pr.kind == PR_OBI) {
            for (int k = 0; k < 6; k++) {
                float u0 = k / 6.f, u1 = (k + 1) / 6.f;
                float w0 = sinf(pr.t * 20 + k) * 3 * s;
                capsule(sx + (ex - sx) * u0, sy + (ey - sy) * u0 + w0, sx + (ex - sx) * u1, sy + (ey - sy) * u1 - w0, 2.6f * s, 2.6f * s, (k & 1) ? RGBc(40, 150, 100) : RGBc(235, 120, 165));
            }
        } else {
            g_add = true;
            capsule(sx, sy, ex, ey, 2.2f * s, 0.3f, RGBc(150, 0, 30), 200);
            capsule(sx, sy, ex, ey, 0.9f * s, 0.2f, RGBc(255, 160, 170), 255);
            glow(sx, sy, 12 * s, RGBc(255, 40, 70), 150);
            g_add = false;
        }
    } break;
    case PR_AIR:
        fillEllipse(sx, gy, 10 * s, 2.5f * s, 0, 60);
        g_add = true;
        for (int k = 0; k < 3; k++) {
            float r = (8 + k * 5) * s;
            ribbon(sx - signf(pr.vx) * k * 6 * s, sy, r, 2.5f, -1.2f, 1.2f, 1, 0, pr.vx > 0 ? 1 : -1, 1, RGBc(120, 170, 255), RGBc(240, 250, 255), 200 - k * 50, true);
        }
        glow(sx, sy, 18 * s, RGBc(120, 170, 255), 120);
        g_add = false;
        break;
    case PR_SLEEP:
        g_add = true;
        for (int k = 0; k < 3; k++) {
            float r = (10 + k * 7 + pulse * 3) * s;
            ribbon(sx, sy, r, 2, 0, 2 * PI, 1, 0, 1, 1, RGBc(170, 90, 255), RGBc(255, 150, 230), 160 - k * 40, false);
        }
        glow(sx, sy, 26 * s, RGBc(160, 80, 255), 140);
        ribbon(sx, gy, 22 * s, 3, 0, 2 * PI, 0.32f, 0, 1, 1, RGBc(160, 80, 255), RGBc(160, 80, 255), 90, false);
        g_add = false;
        break;
    case PR_SICKLE: {
        fillEllipse(sx, gy, 7 * s, 2 * s, 0, 70);
        g_add = true;
        float a = pr.t * 22;
        ribbon(sx, sy, 9 * s, 3.5f, a, a + 3.6f, 1, 0, 1, 1, RGBc(120, 0, 20), RGBc(255, 60, 80), 255, true);
        glow(sx, sy, 16 * s, RGBc(255, 30, 50), 110);
        g_add = false;
    } break;
    case PR_RAIN: {
        uint32_t c = pr.style == 0 ? RGBc(255, 30, 60) : (pr.style == 1 ? RGBc(255, 120, 170) : RGBc(200, 0, 30));
        g_add = true;
        if (!pr.hit) {
            float u = clampf(1 - (pr.life - 0.18f) / 1.0f, 0, 1);
            ribbon(sx, gy, 22 * s, 2.5f, 0, 2 * PI, 0.32f, 0, 1, 1, c, c, (int)(120 + 100 * pulse), false);
            fillEllipse(sx, gy, 22 * s * u, 7 * s * u, c, 80);
            if (pr.style == 0) capsule(sx, gy - 2, sx, 0, 0.5f, 0.5f, c, 60);
        } else {
            float k = pr.life / 0.18f;
            if (pr.style == 0) capsule(sx, gy, sx, 0, 3 * s, 1, RGBc(255, 80, 100), (int)(230 * k));
            g_add = false;
            if (pr.style == 1) { // tentáculo de carne
                for (int j = 0; j < 6; j++) capsule(sx + sinf(j + pr.t * 20) * 3 * s, gy - j * 8 * s, sx + sinf(j + 1 + pr.t * 20) * 3 * s, gy - (j + 1) * 8 * s, (6 - j) * s, (5 - j) * s + 0.5f, RGBc(220, 120, 140));
            }
            if (pr.style == 2) { // espinho do Muzan
                float px[3] = { sx - 7 * s, sx + 7 * s, sx }, py[3] = { gy, gy, gy - 55 * s };
                fillPoly(px, py, 3, RGBc(90, 5, 20));
                float px2[3] = { sx - 3 * s, sx + 3 * s, sx + 1 }, py2[3] = { gy, gy, gy - 50 * s };
                fillPoly(px2, py2, 3, RGBc(170, 20, 40));
            }
        }
        g_add = false;
    } break;
    case PR_SWEEP: case PR_LINE: {
        float hz = pr.kind == PR_SWEEP ? 11.f : 9.f;
        uint32_t c = pr.style == 0 ? RGBc(235, 120, 165) : (pr.style == 1 ? RGBc(220, 20, 40) : RGBc(120, 170, 255));
        if (!pr.hit) {
            floorBand(pr.x, pr.x2, pr.z, hz, c, (int)(40 + 50 * pulse));
            g_add = true;
            floorBand(pr.x, pr.x2, pr.z, 1, c, 160);
            g_add = false;
        } else if (pr.dmg > 0) {
            float k = pr.life / 0.18f;
            float x0 = projX(pr.x, pr.z), x1 = projX(pr.x2, pr.z), y = gy - 22 * s;
            if (pr.style == 0) {
                for (int j = 0; j < 10; j++) capsule(x0 + (x1 - x0) * j / 10, y + sinf(j + pr.t * 30) * 4, x0 + (x1 - x0) * (j + 1) / 10, y + sinf(j + 1 + pr.t * 30) * 4, 3.5f * s, 3.5f * s, (j & 1) ? RGBc(40, 150, 100) : RGBc(235, 120, 165), (int)(255 * k));
            } else {
                capsule(x0, y, x1, y, 5 * s, 2 * s, RGBc(80, 6, 20), (int)(255 * k));
                g_add = true;
                capsule(x0, y, x1, y, 2 * s, 1, RGBc(255, 60, 80), (int)(200 * k));
                g_add = false;
            }
        }
    } break;
    case PR_RING:
        g_add = true;
        if (!pr.hit) {
            float u = clampf(1 - (pr.life - 0.18f) / 0.72f, 0, 1);
            ribbon(sx, gy, pr.r * s, 2.5f, 0, 2 * PI, 0.45f, 0, 1, 1, RGBc(255, 30, 50), RGBc(255, 30, 50), (int)(120 + 100 * pulse), false);
            fillEllipse(sx, gy, pr.r * s * u, pr.r * 0.45f * s * u, RGBc(200, 0, 30), 70);
        } else {
            float k = pr.life / 0.18f;
            ribbon(sx, gy - 20 * s, pr.r * s, 12, 0, 2 * PI, 0.45f, 0, 1, 1, RGBc(150, 0, 20), RGBc(255, 80, 90), (int)(255 * k), false);
        }
        g_add = false;
        break;
    }
}

