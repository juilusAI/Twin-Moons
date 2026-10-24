// Visual dos personagens: aparência, poses, cabelo e desenho do corpo.
#include "common.h"

// ============================================================ desenho: personagens
struct V2 { float x, y; };
struct Pose { V2 hip, neck, head, fk, ff, bk, bf, fe, fh, be, bh; float sword; };
struct Look {
    uint32_t skin, hair, hairTip, top, pants, wraps, sleeve, eye;
    bool haori = false; uint32_t h1 = 0, h2 = 0; float cell = 4.5f;
    bool box = false, sword = false, claws = false, earrings = false, muzzle = false, longHair = false;
    bool redBlade = false, tattoo = false, sickles = false, obi = false, tentacles = false, palmEye = false, spots = false;
    int mark = 0; // 0 cicatriz, 1 marca de caçador, 2 marca brilhando
    int horns = 0, extraArms = 0, hairStyle = 0; // 0 tanjiro 1 bagunçado 2 rui 3 nezuko 4 enmu 5 daki 6 gyutaro 7 akaza 8 muzan
    uint32_t vest = 0;
    float eyeGlow = 0;
};

V2 bendJoint(V2 a, V2 b, float bend) {
    V2 m = { (a.x + b.x) * 0.5f, (a.y + b.y) * 0.5f };
    float dx = b.x - a.x, dy = b.y - a.y, L = sqrtf(dx * dx + dy * dy) + 1e-3f;
    return { m.x - dy / L * bend, m.y + dx / L * bend };
}

Pose poseStand(float ph, float amt, float crouch) {
    Pose q;
    float bob = amt > 0 ? -fabsf(sinf(ph)) * 1.5f : 0;
    q.hip = { 0, -29 + crouch + bob };
    q.neck = { 1.5f, -49 + crouch + bob };
    q.head = { 2.5f, -57.5f + crouch + bob };
    float sw = sinf(ph) * amt, cw = cosf(ph) * amt;
    if (amt > 0) { q.ff = { 3 + sw * 10, -std::max(0.f, cw) * 5 }; q.bf = { -2 - sw * 10, -std::max(0.f, -cw) * 5 }; }
    else { q.ff = { 7, 0 }; q.bf = { -6, 0 }; }
    V2 sh = { q.neck.x, q.neck.y + 3 };
    q.fh = { sh.x + 9, sh.y + 11 }; q.bh = { sh.x + 7, sh.y + 12 };
    q.sword = -0.8f;
    return q;
}
void finishPose(Pose& q) {
    q.fk = bendJoint(q.hip, q.ff, -3.5f);
    q.bk = bendJoint(q.hip, q.bf, -3.5f);
    V2 sh = { q.neck.x, q.neck.y + 3 };
    q.fe = bendJoint(sh, q.fh, 3.5f);
    q.be = bendJoint({ sh.x - 1.5f, sh.y }, q.bh, 3.5f);
}
void leanPose(Pose& q, float L) { q.neck.x += L; q.head.x += L * 1.3f; }

float swingU(float u) {
    if (u < 0.22f) return -0.08f * smooth01(u / 0.22f);
    if (u < 0.52f) return smooth01((u - 0.22f) / 0.30f);
    return 1.f;
}

Pose playerPose(const Ent& p) {
    Pose q = poseStand(p.anim * 11, p.st == S_WALK ? 1.f : 0.f, 0);
    if (p.st == S_IDLE) { q.neck.y += sinf(p.anim * 3) * 0.6f; q.head.y += sinf(p.anim * 3) * 0.6f; }
    auto swing = [&](float tt, float a0, float a1, float dur, float off, float lean) {
        float u = clampf(tt / dur, 0, 1);
        leanPose(q, lean * sinf(u * PI));
        q.ff = { 10, 0 }; q.bf = { -9, 0 }; q.hip.y += 2; q.neck.y += 2; q.head.y += 2;
        V2 sh = { q.neck.x, q.neck.y + 3 };
        float phi = a0 + (a1 - a0) * swingU(u);
        q.fh = { sh.x + cosf(phi) * 13, sh.y + sinf(phi) * 13 };
        q.bh = { q.fh.x - 2, q.fh.y + 1 };
        q.sword = phi + off;
    };
    auto spin = [&](float phi, float ry) {
        V2 sh = { q.neck.x, q.neck.y + 3 };
        q.fh = { sh.x + cosf(phi) * 13, sh.y + sinf(phi) * ry }; q.bh = { q.fh.x - 2, q.fh.y + 1 }; q.sword = phi - 0.3f;
        q.ff = { 9, 0 }; q.bf = { -9, 0 };
    };
    switch (p.st) {
    case S_ATK1: swing(p.t, -2.1f, 0.9f, 0.3f, -0.25f, 4); break;
    case S_ATK2: swing(p.t, 1.3f, -1.7f, 0.32f, 0.f, 3); break;
    case S_ATK3: swing(p.t, -2.7f, 1.2f, 0.46f, -0.2f, 6); break;
    case S_AIRATK: swing(p.t, -2.2f, 1.6f, 0.25f, 0.f, 3); q.ff = { 7, -9 }; q.bf = { -5, -6 }; break;
    case S_WATER1: swing(p.t, -2.9f, 0.6f, 0.55f, -0.3f, 7); q.ff = { 14, 0 }; q.bf = { -12, 0 }; break;
    case S_HINO_SUN: swing(p.t < 0.4f ? p.t : p.t - 0.4f, -2.9f, 0.7f, 0.35f, -0.3f, 7); q.ff = { 14, 0 }; q.bf = { -12, 0 }; break;
    case S_WATER10: case S_HINO_DRAGON: {
        leanPose(q, 7);
        float ph = p.anim * 22;
        q.ff = { 4 + sinf(ph) * 12, -std::max(0.f, cosf(ph)) * 6 }; q.bf = { -3 - sinf(ph) * 12, -std::max(0.f, -cosf(ph)) * 6 };
        V2 sh = { q.neck.x, q.neck.y + 3 };
        q.fh = { sh.x + 12, sh.y + 4 }; q.bh = { sh.x + 10, sh.y + 5 }; q.sword = 0.1f + sinf(p.t * 14) * 0.3f;
    } break;
    case S_HINO: case S_HINO_13: {
        spin(p.t * 15, 9);
        float endT = p.st == S_HINO ? 1.0f : 2.2f;
        if (p.t > endT) {
            V2 sh = { q.neck.x, q.neck.y + 3 };
            float u = swingU((p.t - endT) / 0.4f); float a = -2.6f + 3.8f * u;
            q.fh = { sh.x + cosf(a) * 13, sh.y + sinf(a) * 13 }; q.bh = { q.fh.x - 2, q.fh.y + 1 }; q.sword = a; leanPose(q, 6 * u);
        }
    } break;
    case S_HINO_SKY: spin(-PI / 2 + p.t * 16, 13); q.ff = { 6, -8 }; q.bf = { -5, -4 }; break;
    case S_JUMP: q.ff = { 6, -9 }; q.bf = { -5, -5 }; q.sword = -1.2f; break;
    case S_DASH: leanPose(q, 7); q.ff = { 12, -2 }; q.bf = { -12, -1 }; q.sword = 0.3f; { V2 sh = { q.neck.x, q.neck.y + 3 }; q.fh = { sh.x + 4, sh.y + 13 }; q.bh = { sh.x + 3, sh.y + 13 }; } break;
    case S_HURT: leanPose(q, -5); { V2 sh = { q.neck.x, q.neck.y + 3 }; q.fh = { sh.x - 4, sh.y + 12 }; q.bh = { sh.x + 8, sh.y + 4 }; q.sword = 0.9f; } break;
    case S_SLEEP: {
        q.hip.y += 3; q.neck.y += 4; q.head.y += 6; q.head.x -= 3; q.neck.x -= 1;
        V2 sh = { q.neck.x, q.neck.y + 3 }; q.fh = { sh.x + 3, sh.y + 15 }; q.bh = { sh.x + 1, sh.y + 15 }; q.sword = 1.3f;
        q.ff = { 6, 0 }; q.bf = { -5, 0 };
    } break;
    default: break;
    }
    finishPose(q);
    return q;
}

Pose enemyPose(const Ent& e) {
    bool walking = e.st == S_WALK || (e.type == ET_FAST && e.st == S_E_LUNGE);
    float crouch = e.type == ET_FAST ? 5.f : 0.f;
    Pose q = poseStand(e.anim * (e.type == ET_FAST ? 14 : 9), walking ? 1.f : 0.f, crouch);
    bool boss = isBoss(e.type);
    if (!boss || e.type == ET_GYUTARO) { leanPose(q, e.type == ET_GYUTARO ? 7.f : 5.f); q.head.y += 3; }
    V2 sh = { q.neck.x, q.neck.y + 3 };
    q.fh = { sh.x + 6 + sinf(e.anim * 4) * 2, sh.y + 14 }; q.bh = { sh.x + 3, sh.y + 15 };
    if (boss) {
        bool cast = e.st == S_B_CAST;
        float u = cast ? smooth01(e.t / 0.6f) : 0;
        switch (e.type) {
        case ET_RUI:
            q.ff = { 3, -1 + sinf(e.anim * 2) }; q.bf = { -3, 0 };
            q.fh = { sh.x + 5, sh.y + 13 }; q.bh = { sh.x - 4, sh.y + 13 };
            if (cast && e.pat == 0) { q.fh = { sh.x + 14 * u + 4, sh.y + 2 - 6 * u }; q.bh = { sh.x + 11 * u, sh.y + 4 - 4 * u }; }
            if (cast && e.pat == 1) { q.fh = { sh.x + 6, sh.y - 13 }; q.bh = { sh.x - 3, sh.y - 13 }; }
            break;
        case ET_ENMU:
            q.fh = { sh.x + 4, sh.y + 13 }; q.bh = { sh.x - 3, sh.y + 13 };
            if (cast) { q.fh = { sh.x + 14 * u + 2, sh.y + 4 - 10 * u }; }
            break;
        case ET_DAKI:
            q.fh = { sh.x + 2, sh.y + 10 }; q.bh = { sh.x - 6, sh.y + 8 };
            if (cast && e.pat != 2) { q.fh = { sh.x + 12 * u + 2, sh.y + 2 - 8 * u }; }
            if (cast && e.pat == 2 && e.t > 0.35f) { leanPose(q, -4); q.ff = { 22, -22 }; q.bf = { -6, 0 }; }
            break;
        case ET_GYUTARO:
            q.fh = { sh.x + 10, sh.y + 8 }; q.bh = { sh.x + 6, sh.y + 10 };
            if (cast) { float a = e.anim * 18; q.fh = { sh.x + cosf(a) * 13, sh.y + sinf(a) * 10 }; q.bh = { sh.x - cosf(a) * 11, sh.y - sinf(a) * 9 }; }
            break;
        case ET_AKAZA: {
            q.fh = { sh.x + 9, sh.y + 2 }; q.bh = { sh.x + 6, sh.y + 4 };
            q.ff = { 9, 0 }; q.bf = { -8, 0 }; q.hip.y += 2; q.neck.y += 2; q.head.y += 2;
            if (e.punchT > 0) { if ((e.cnt + (int)(e.t * 20)) & 1) q.fh = { sh.x + 17, sh.y + 1 }; else q.bh = { sh.x + 16, sh.y + 2 }; leanPose(q, 4); }
            if (cast && e.pat == 3 && e.t > 0.7f) { leanPose(q, 10); q.fh = { sh.x + 18, sh.y }; }
        } break;
        case ET_MUZAN:
            q.fh = { sh.x + 3, sh.y + 14 }; q.bh = { sh.x - 3, sh.y + 14 };
            if (cast) { q.fh = { sh.x + 16 * u, sh.y + 6 - 6 * u }; }
            break;
        default: break;
        }
        if (e.type == ET_DAKI || e.type == ET_MUZAN || e.type == ET_ENMU) { q.ff.x = 5; q.bf.x = -4; if (walking) { q.ff.x = 3 + sinf(e.anim * 9) * 8; q.bf.x = -2 - sinf(e.anim * 9) * 8; } }
        finishPose(q);
        return q;
    }
    if (e.st == S_E_WIND) {
        float u = smooth01(e.t / windDur(e));
        leanPose(q, -3 * u);
        q.fh = { sh.x - 6 * u, sh.y - 12 * u + 4 };
        if (e.type == ET_BIG) q.bh = { sh.x - 3, sh.y - 13 * u };
        if (e.type == ET_FAST) { q.hip.y += 4 * u; q.neck.y += 5 * u; q.head.y += 5 * u; }
    }
    if (e.st == S_E_STRIKE || (e.st == S_E_RECOVER && e.t < 0.15f)) {
        leanPose(q, 6);
        q.fh = { sh.x + 15, sh.y + 6 };
        if (e.type == ET_BIG) q.bh = { sh.x + 13, sh.y + 9 };
        q.ff = { 10, 0 }; q.bf = { -9, 0 };
    }
    if (e.st == S_E_LUNGE) { leanPose(q, 10); q.fh = { sh.x + 16, sh.y + 2 }; q.bh = { sh.x + 13, sh.y + 4 }; }
    if (e.st == S_HURT) leanPose(q, -6);
    finishPose(q);
    return q;
}

void drawSword(const Xf& x, V2 h, float a, bool red) {
    float c = cosf(a), s = sinf(a);
    Lcap(x, h.x - c * 6, h.y - s * 6, h.x + c * 1, h.y + s * 1, 1.5f, 1.5f, RGBc(30, 28, 34));
    Lcap(x, h.x - c * 6, h.y - s * 6, h.x - c * 6.1f, h.y - s * 6.1f, 1.6f, 1.6f, RGBc(170, 150, 90));
    if (red) {
        bool old = g_add; g_add = true;
        Lcap(x, h.x + c * 2.5f, h.y + s * 2.5f, h.x + c * 34, h.y + s * 34, 3.2f, 1.6f, RGBc(255, 40, 20), 120);
        g_add = old;
        Lcap(x, h.x + c * 2.5f, h.y + s * 2.5f, h.x + c * 33, h.y + s * 33, 1.35f, 0.5f, RGBc(220, 30, 20));
        Lcap(x, h.x + c * 3 - s * 0.7f, h.y + s * 3 + c * 0.7f, h.x + c * 32 - s * 0.4f, h.y + s * 32 + c * 0.4f, 0.45f, 0.3f, RGBc(255, 210, 160));
    } else {
        Lcap(x, h.x + c * 2.5f, h.y + s * 2.5f, h.x + c * 33, h.y + s * 33, 1.35f, 0.5f, RGBc(18, 18, 24));
        Lcap(x, h.x + c * 3 - s * 0.7f, h.y + s * 3 + c * 0.7f, h.x + c * 32 - s * 0.4f, h.y + s * 32 + c * 0.4f, 0.45f, 0.3f, RGBc(170, 180, 205));
    }
    Lcirc(x, h.x + c * 2, h.y + s * 2, 2.6f, RGBc(40, 36, 30));
}

void drawSpikes(const Xf& x, V2 hd, int n, float a0, float a1, float len, float cx, float cy, uint32_t c, uint32_t tip) {
    for (int i = 0; i < n; i++) {
        float a = a0 + (a1 - a0) * (n > 1 ? (float)i / (n - 1) : 0.5f);
        float bx0 = hd.x + cx + cosf(a - 0.3f) * 6, by0 = hd.y + cy + sinf(a - 0.3f) * 6;
        float bx1 = hd.x + cx + cosf(a + 0.3f) * 6, by1 = hd.y + cy + sinf(a + 0.3f) * 6;
        float tx = hd.x + cx + cosf(a) * len, ty = hd.y + cy + sinf(a) * len;
        Lpoly(x, { bx0, by0, tx, ty, bx1, by1 }, c);
        if (tip != c) {
            float mx = (bx0 + bx1) * 0.5f, my = (by0 + by1) * 0.5f;
            float kx = mx + (tx - mx) * 0.6f, ky = my + (ty - my) * 0.6f;
            Lpoly(x, { kx + (bx0 - mx) * 0.3f, ky + (by0 - my) * 0.3f, tx, ty, kx + (bx1 - mx) * 0.3f, ky + (by1 - my) * 0.3f }, tip);
        }
    }
}

void drawHair(const Xf& x, const Look& L, V2 hd) {
    switch (L.hairStyle) {
    case 0: {
        Lcirc(x, hd.x - 1.5f, hd.y - 2.5f, 7.8f, L.hair);
        const float sp[][6] = {
            { -6, -6, -16, -10, -7, -1 }, { -6, -1, -15, 3, -6, 4 }, { -2, -8, -6, -17, 3, -8 },
            { 2, -9, 9, -15, 7, -5 }, { 4, -8, 10, -2, 6, -4 }, { -5, -9, -12, -16, 0, -9 } };
        for (auto& s : sp) {
            Lpoly(x, { hd.x + s[0], hd.y + s[1], hd.x + s[2], hd.y + s[3], hd.x + s[4], hd.y + s[5] }, L.hair);
            float mx = (s[0] + s[4]) * 0.5f, my = (s[1] + s[5]) * 0.5f;
            float tx0 = mx + (s[2] - mx) * 0.55f, ty0 = my + (s[3] - my) * 0.55f;
            Lpoly(x, { hd.x + tx0 + (s[0] - mx) * 0.3f, hd.y + ty0 + (s[1] - my) * 0.3f, hd.x + s[2], hd.y + s[3], hd.x + tx0 + (s[4] - mx) * 0.3f, hd.y + ty0 + (s[5] - my) * 0.3f }, L.hairTip);
        }
    } break;
    case 1:
        Lcirc(x, hd.x - 1.5f, hd.y - 2.5f, 7.6f, L.hair);
        drawSpikes(x, hd, 5, -2.8f, -0.8f, 13, -1, -2, L.hair, L.hair);
        break;
    case 2: // Rui
        Lcirc(x, hd.x - 1, hd.y - 2.5f, 8.2f, L.hair);
        Lpoly(x, { hd.x - 9, hd.y - 3, hd.x - 8, hd.y + 7, hd.x - 3, hd.y + 2 }, L.hair);
        Lpoly(x, { hd.x + 2, hd.y - 8, hd.x + 9, hd.y - 2, hd.x + 7, hd.y + 1, hd.x + 3, hd.y - 4 }, L.hair);
        break;
    case 3: // Nezuko
        Lcirc(x, hd.x - 1.5f, hd.y - 2.5f, 8.f, L.hair);
        Lpoly(x, { hd.x + 3, hd.y - 8, hd.x + 9, hd.y - 3, hd.x + 7, hd.y + 2, hd.x + 3, hd.y - 3 }, L.hair);
        Lcap(x, hd.x - 6, hd.y - 6, hd.x - 4, hd.y - 9, 2.2f, 2.2f, RGBc(240, 120, 160));
        break;
    case 4: // Enmu
        Lcirc(x, hd.x - 1.5f, hd.y - 2.5f, 7.8f, L.hair);
        Lpoly(x, { hd.x + 1, hd.y - 9, hd.x + 9, hd.y - 4, hd.x + 7, hd.y + 4, hd.x + 3, hd.y - 2 }, L.hair);
        Lpoly(x, { hd.x - 8, hd.y - 3, hd.x - 10, hd.y + 9, hd.x - 4, hd.y + 2 }, L.hair);
        Lcap(x, hd.x - 4, hd.y - 8, hd.x + 4, hd.y - 9, 0.8f, 0.8f, L.hairTip);
        break;
    case 5: // Daki
        Lcirc(x, hd.x - 1.5f, hd.y - 2.5f, 7.8f, L.hair);
        Lcirc(x, hd.x - 4, hd.y - 11, 5, L.hair);
        Lcirc(x, hd.x + 1, hd.y - 13, 4, L.hair);
        Lcap(x, hd.x - 3, hd.y - 14, hd.x + 7, hd.y - 19, 0.6f, 0.4f, RGBc(240, 200, 90));
        Lcap(x, hd.x - 6, hd.y - 12, hd.x - 12, hd.y - 19, 0.6f, 0.4f, RGBc(240, 200, 90));
        Lpoly(x, { hd.x + 2, hd.y - 8, hd.x + 9, hd.y - 4, hd.x + 6, hd.y - 2 }, L.hair);
        break;
    case 6: // Gyutaro
        Lcirc(x, hd.x - 1.5f, hd.y - 2, 8.5f, L.hair);
        drawSpikes(x, hd, 8, -3.4f, -0.4f, 16, -1, -1, L.hair, L.hairTip);
        drawSpikes(x, hd, 3, 1.6f, 2.6f, 14, -2, 0, L.hair, L.hairTip);
        break;
    case 7: // Akaza
        Lcirc(x, hd.x - 1.5f, hd.y - 3, 7.6f, L.hair);
        drawSpikes(x, hd, 6, -3.0f, -0.6f, 12, -1, -2, L.hair, L.hairTip);
        Lpoly(x, { hd.x + 3, hd.y - 8, hd.x + 9, hd.y - 4, hd.x + 5, hd.y - 3 }, L.hair);
        break;
    case 8: // Muzan
        Lcirc(x, hd.x - 1.5f, hd.y - 2.5f, 8.f, L.hair);
        for (int k = 0; k < 9; k++) {
            float a = -3.4f + k * 0.38f;
            Lcirc(x, hd.x - 1.5f + cosf(a) * 8.5f, hd.y - 2.5f + sinf(a) * 8.5f, 3.3f, L.hair);
        }
        Lcirc(x, hd.x - 8, hd.y + 5, 3.5f, L.hair);
        break;
    }
}

void tattooBand(const Xf& x, V2 a, V2 b, float u, float w) {
    float px = a.x + (b.x - a.x) * u, py = a.y + (b.y - a.y) * u;
    float dx = b.x - a.x, dy = b.y - a.y, L = sqrtf(dx * dx + dy * dy) + 1e-3f;
    float nx = -dy / L * w, ny = dx / L * w;
    Lcap(x, px - nx, py - ny, px + nx, py + ny, 0.55f, 0.55f, RGBc(50, 80, 220));
}

void drawHumanoid(const Xf& x, const Look& L, const Pose& p, float t) {
    V2 sh = { p.neck.x, p.neck.y + 3 };
    uint32_t pantsD = mulC(L.pants, 0.7f), sleeveD = mulC(L.sleeve, 0.7f), skinD = mulC(L.skin, 0.75f);
    // tentáculos do Muzan (atrás de tudo)
    if (L.tentacles) {
        for (int k = 0; k < 5; k++) {
            float px = p.neck.x - 3, py = p.neck.y + 6, ang = -2.6f + k * 0.45f;
            for (int s = 0; s < 8; s++) {
                ang += sinf(t * 3 + k * 1.3f + s * 0.6f) * 0.25f;
                float nx = px + cosf(ang) * 6, ny = py + sinf(ang) * 6;
                Lcap(x, px, py, nx, ny, 3.2f - s * 0.3f, 3.f - s * 0.3f, s % 2 ? RGBc(110, 12, 28) : RGBc(90, 8, 22));
                if (s % 3 == 1) Lpoly(x, { nx, ny - 2, nx - 2, ny - 6, nx + 1, ny - 2 }, RGBc(60, 4, 12));
                px = nx; py = ny;
            }
        }
    }
    // faixas de obi da Daki
    if (L.obi) {
        for (int r = 0; r < 4; r++) {
            float px = p.hip.x - 4, py = p.hip.y - 6;
            for (int s = 0; s < 11; s++) {
                float nx = px - 4.2f, ny = py + sinf(t * 4 + s * 0.6f + r * 1.5f) * 2.2f - (r - 1.5f) * 0.9f;
                Lcap(x, px, py, nx, ny, 1.8f, 1.8f, ((s + r) & 1) ? RGBc(40, 150, 100) : RGBc(235, 120, 165));
                px = nx; py = ny;
            }
        }
    }
    // costas: caixa da Nezuko
    if (L.box) {
        float bx = p.neck.x - 4, by = p.neck.y;
        Lpoly(x, { bx - 15, by - 3, bx - 3, by - 3, bx - 3, by + 27, bx - 15, by + 27 }, RGBc(140, 96, 56));
        Lcap(x, bx - 15, by - 3, bx - 3, by - 3, 1.1f, 1.1f, RGBc(80, 50, 30));
        Lcap(x, bx - 15, by + 12, bx - 3, by + 12, 0.9f, 0.9f, RGBc(80, 50, 30));
        Lcap(x, bx - 15, by + 27, bx - 3, by + 27, 1.1f, 1.1f, RGBc(80, 50, 30));
        Lcap(x, bx - 15, by - 3, bx - 15, by + 27, 0.9f, 0.9f, RGBc(80, 50, 30));
    }
    if (L.longHair) {
        float sway = sinf(t * 3) * 1.5f;
        Lcap(x, p.head.x - 3, p.head.y - 2, p.head.x - 9 + sway, p.head.y + 22, 6.5f, 3.f, L.hair);
        Lcap(x, p.head.x - 9 + sway, p.head.y + 18, p.head.x - 10 + sway, p.head.y + 26, 3.5f, 2.f, L.hairTip);
    }
    // braço de trás
    if (L.haori) patOn(x, L.cell, mulC(L.h1, 0.7f), mulC(L.h2, 0.7f));
    Lcap(x, sh.x - 1.5f, sh.y, p.be.x, p.be.y, 3.3f, 2.9f, sleeveD);
    Lcap(x, p.be.x, p.be.y, p.bh.x, p.bh.y, 2.9f, 2.4f, sleeveD);
    patOff();
    Lcirc(x, p.bh.x, p.bh.y, 2.3f, skinD);
    if (L.sickles) {
        V2 h = p.bh;
        Lcap(x, h.x, h.y, h.x + 6, h.y - 6, 1.1f, 0.9f, RGBc(150, 10, 20));
        Lcap(x, h.x + 6, h.y - 6, h.x + 12, h.y - 2, 0.9f, 0.4f, RGBc(150, 10, 20));
    }
    // braços extras (demônio grande)
    for (int i = 0; i < L.extraArms; i++) {
        float a = -2.6f + i * 0.55f + sinf(t * 2.5f + i) * 0.25f;
        float ax = p.neck.x - 2, ay = p.neck.y + 6 + i * 3;
        float ex = ax + cosf(a) * 13, ey = ay + sinf(a) * 13;
        float hx = ex + cosf(a + 0.8f) * 11, hy = ey + sinf(a + 0.8f) * 11;
        Lcap(x, ax, ay, ex, ey, 2.8f, 2.4f, mulC(L.skin, 0.6f));
        Lcap(x, ex, ey, hx, hy, 2.4f, 2.f, mulC(L.skin, 0.6f));
        Lcirc(x, hx, hy, 2.6f, mulC(L.skin, 0.55f));
    }
    // perna de trás
    Lcap(x, p.hip.x - 1, p.hip.y, p.bk.x, p.bk.y, 4.6f, 3.8f, pantsD);
    Lcap(x, p.bk.x, p.bk.y, p.bf.x, p.bf.y - 1.5f, 3.8f, 2.8f, pantsD);
    if (L.wraps) Lcap(x, (p.bk.x + p.bf.x) * 0.5f, (p.bk.y + p.bf.y) * 0.5f, p.bf.x, p.bf.y - 1.5f, 3.f, 2.6f, mulC(L.wraps, 0.7f));
    Lcap(x, p.bf.x - 1, p.bf.y - 1, p.bf.x + 4, p.bf.y - 1, 1.8f, 1.6f, RGBc(30, 26, 26));
    // perna da frente
    Lcap(x, p.hip.x + 1, p.hip.y, p.fk.x, p.fk.y, 4.8f, 4.f, L.pants);
    Lcap(x, p.fk.x, p.fk.y, p.ff.x, p.ff.y - 1.5f, 4.f, 2.9f, L.pants);
    if (L.wraps) Lcap(x, (p.fk.x + p.ff.x) * 0.5f, (p.fk.y + p.ff.y) * 0.5f, p.ff.x, p.ff.y - 1.5f, 3.1f, 2.7f, L.wraps);
    Lcap(x, p.ff.x - 1, p.ff.y - 1, p.ff.x + 4.5f, p.ff.y - 1, 1.9f, 1.7f, RGBc(36, 30, 30));
    // tronco
    Lcap(x, p.hip.x, p.hip.y - 1, p.neck.x, p.neck.y + 2, 7.f, 6.4f, L.top);
    if (L.tattoo) {
        for (int k = 0; k < 3; k++) tattooBand(x, { p.hip.x, p.hip.y - 1 }, { p.neck.x, p.neck.y + 2 }, 0.25f + k * 0.22f, 6.5f);
    }
    if (L.vest) {
        Lpoly(x, { p.neck.x - 7, p.neck.y + 1, p.neck.x - 1, p.neck.y + 1, p.hip.x - 2, p.hip.y + 2, p.hip.x - 8, p.hip.y + 2 }, L.vest);
        Lcap(x, p.hip.x - 7, p.hip.y - 1, p.hip.x + 7, p.hip.y - 1, 1.8f, 1.8f, RGBc(60, 50, 90));
    }
    if (L.spots) {
        Lcirc(x, p.neck.x + 2, p.neck.y + 8, 1.2f, RGBc(70, 60, 70));
        Lcirc(x, p.hip.x + 3, p.hip.y - 6, 1.4f, RGBc(70, 60, 70));
    }
    if (L.haori) {
        patOn(x, L.cell, L.h1, L.h2);
        float sway = sinf(t * 4) * 1.5f;
        Lpoly(x, { p.neck.x - 7, p.neck.y + 1, p.neck.x + 5, p.neck.y + 1, p.hip.x + 8, p.hip.y + 13, p.hip.x - 12 - sway, p.hip.y + 14 }, L.h1);
        patOff();
        Lcap(x, p.neck.x + 3, p.neck.y + 2, p.hip.x + 6, p.hip.y + 1, 1.6f, 1.6f, L.top);
        Lcap(x, p.hip.x - 6, p.hip.y - 2, p.hip.x + 7, p.hip.y - 2, 1.5f, 1.5f, RGBc(225, 225, 215));
    }
    if (L.hairStyle == 8) Lpoly(x, { p.neck.x - 1, p.neck.y + 1, p.neck.x + 5, p.neck.y + 1, p.neck.x + 3, p.neck.y + 7 }, RGBc(230, 230, 235));
    // cabeça
    Lcap(x, p.neck.x, p.neck.y, p.head.x, p.head.y, 3.f, 3.f, L.skin);
    Lcirc(x, p.head.x, p.head.y, 7.2f, L.skin);
    drawHair(x, L, p.head);
    float ex = p.head.x + 4.6f, ey = p.head.y - 0.5f;
    if (L.eyeGlow > 0) {
        Lcirc(x, ex, ey, 1.4f, L.eye);
        glow(TX(x, ex, ey), TY(x, ex, ey), (4 + 6 * L.eyeGlow) * x.s, L.eye, (int)(120 + 120 * L.eyeGlow));
    } else Lcirc(x, ex, ey, 1.15f, L.eye);
    for (int i = 0; i < L.horns; i++) {
        float hx = p.head.x + 2 - i * 6;
        Lpoly(x, { hx - 2, p.head.y - 6, hx + 1.5f, p.head.y - 17 + i * 2, hx + 2, p.head.y - 6 }, RGBc(220, 210, 190));
    }
    if (L.horns > 0) Lpoly(x, { p.head.x + 5, p.head.y + 3, p.head.x + 7, p.head.y + 6, p.head.x + 7.5f, p.head.y + 3 }, RGBc(240, 240, 230));
    if (L.mark == 0 && L.earrings) Lcap(x, p.head.x + 3.5f, p.head.y - 6, p.head.x + 5.5f, p.head.y - 3.5f, 0.9f, 0.7f, RGBc(150, 50, 40));
    if (L.mark >= 1) {
        Lpoly(x, { p.head.x + 2, p.head.y - 3, p.head.x + 4, p.head.y - 9, p.head.x + 6, p.head.y - 6, p.head.x + 7, p.head.y - 9, p.head.x + 6.5f, p.head.y - 3 }, RGBc(190, 40, 30));
        if (L.mark == 2) glow(TX(x, p.head.x + 4.5f, p.head.y - 6), TY(x, p.head.x + 4.5f, p.head.y - 6), 9 * x.s, RGBc(255, 80, 30), 160);
    }
    if (L.earrings) {
        Lpoly(x, { p.head.x - 2.5f, p.head.y + 4, p.head.x + 0.5f, p.head.y + 4, p.head.x + 0.5f, p.head.y + 10, p.head.x - 2.5f, p.head.y + 10 }, RGBc(235, 230, 215));
        Lcirc(x, p.head.x - 1, p.head.y + 7, 1.1f, RGBc(200, 40, 30));
    }
    if (L.muzzle) Lcap(x, p.head.x + 1, p.head.y + 3, p.head.x + 8, p.head.y + 3, 1.6f, 1.6f, RGBc(110, 150, 60));
    if (L.hairStyle == 2) {
        Lcap(x, p.head.x + 3, p.head.y + 1, p.head.x + 6, p.head.y + 5, 0.5f, 0.5f, RGBc(200, 30, 40));
        Lcap(x, p.head.x + 1, p.head.y + 2, p.head.x + 3, p.head.y + 6, 0.5f, 0.5f, RGBc(200, 30, 40));
    }
    if (L.tattoo) {
        Lcap(x, ex, ey + 1.5f, ex - 1, ey + 6, 0.5f, 0.5f, RGBc(50, 80, 220));
        Lcap(x, p.head.x + 1, p.head.y - 6, p.head.x + 5, p.head.y - 7, 0.5f, 0.5f, RGBc(50, 80, 220));
    }
    if (L.spots) Lcirc(x, p.head.x + 2, p.head.y + 3, 1.f, RGBc(70, 60, 70));
    // espada + braço da frente
    if (L.sword) drawSword(x, p.fh, p.sword, L.redBlade);
    if (L.haori) patOn(x, L.cell, L.h1, L.h2);
    Lcap(x, sh.x, sh.y, p.fe.x, p.fe.y, 3.5f, 3.1f, L.sleeve);
    Lcap(x, p.fe.x, p.fe.y, p.fh.x, p.fh.y, 3.1f, 2.6f, L.sleeve);
    patOff();
    if (L.tattoo) { tattooBand(x, sh, p.fe, 0.5f, 3.4f); tattooBand(x, p.fe, p.fh, 0.4f, 3.f); tattooBand(x, p.fe, p.fh, 0.75f, 2.8f); }
    Lcirc(x, p.fh.x, p.fh.y, 2.5f, L.skin);
    if (L.palmEye) { Lcirc(x, p.fh.x + 1, p.fh.y, 1.6f, RGBc(240, 150, 210)); Lcirc(x, p.fh.x + 1.3f, p.fh.y, 0.7f, RGBc(60, 0, 40)); }
    if (L.sickles) {
        V2 h = p.fh;
        Lcap(x, h.x, h.y, h.x + 7, h.y - 7, 1.2f, 1.f, RGBc(200, 20, 30));
        Lcap(x, h.x + 7, h.y - 7, h.x + 14, h.y - 2, 1.f, 0.4f, RGBc(200, 20, 30));
        Lcap(x, h.x + 14, h.y - 2, h.x + 13, h.y + 4, 0.6f, 0.3f, RGBc(200, 20, 30));
    }
    if (L.claws) {
        float a = atan2f(p.fh.y - p.fe.y, p.fh.x - p.fe.x);
        for (int k = -1; k <= 1; k++) {
            float b = a + k * 0.35f;
            Lcap(x, p.fh.x, p.fh.y, p.fh.x + cosf(b) * 7, p.fh.y + sinf(b) * 7, 0.8f, 0.3f, RGBc(235, 230, 215));
        }
    }
}

Look lookFor(const Ent& e) {
    Look L{};
    static const uint32_t tint[5] = { RGBc(118, 128, 142), RGBc(120, 140, 120), RGBc(150, 110, 110), RGBc(130, 110, 150), RGBc(100, 100, 110) };
    switch (e.type) {
    case ET_PLAYER:
        L.skin = RGBc(245, 210, 180); L.hair = RGBc(48, 18, 18); L.hairTip = RGBc(160, 45, 35);
        L.top = RGBc(26, 28, 40); L.pants = RGBc(30, 32, 44); L.wraps = RGBc(228, 228, 218); L.sleeve = RGBc(26, 28, 40);
        L.eye = RGBc(120, 30, 30); L.haori = true; L.h1 = RGBc(20, 120, 88); L.h2 = RGBc(14, 14, 18); L.cell = 4.2f;
        L.box = chapter < 3; L.sword = true; L.earrings = true; L.hairStyle = 0;
        L.mark = chapter >= 3 ? (akatoT > 0 ? 2 : 1) : 0;
        L.redBlade = akatoT > 0;
        break;
    case ET_GRUNT: case ET_FAST: case ET_BIG:
        if (e.type == ET_GRUNT) {
            L.skin = tint[chapter % 5]; L.hair = RGBc(30, 44, 36); L.top = RGBc(70, 50, 40); L.pants = RGBc(55, 42, 36);
            L.eye = RGBc(255, 200, 60); L.horns = 1; L.eyeGlow = 0.2f;
        } else if (e.type == ET_FAST) {
            L.skin = RGBc(150, 105, 160); L.hair = RGBc(20, 20, 30); L.top = RGBc(40, 30, 50); L.pants = RGBc(36, 28, 44);
            L.eye = RGBc(255, 60, 200); L.horns = 2; L.eyeGlow = 0.3f;
        } else {
            L.skin = RGBc(86, 118, 74); L.hair = RGBc(40, 30, 20); L.top = RGBc(86, 118, 74); L.pants = RGBc(60, 50, 40);
            L.eye = RGBc(255, 80, 40); L.horns = 2; L.extraArms = 5; L.eyeGlow = 0.3f;
        }
        L.hairTip = L.hair; L.sleeve = L.skin; L.wraps = 0; L.claws = true; L.hairStyle = 1;
        break;
    case ET_RUI:
        L.skin = RGBc(238, 232, 236); L.hair = RGBc(238, 236, 240); L.hairTip = L.hair;
        L.top = RGBc(196, 190, 220); L.pants = RGBc(186, 180, 212); L.sleeve = RGBc(225, 225, 235); L.wraps = 0;
        L.eye = RGBc(255, 40, 60); L.hairStyle = 2; L.eyeGlow = 0.4f;
        L.haori = true; L.h1 = RGBc(236, 234, 244); L.h2 = RGBc(200, 196, 224); L.cell = 3.f;
        break;
    case ET_ENMU:
        L.skin = RGBc(235, 225, 232); L.hair = RGBc(30, 30, 52); L.hairTip = RGBc(120, 220, 210);
        L.top = RGBc(80, 100, 170); L.pants = RGBc(40, 40, 72); L.sleeve = RGBc(80, 100, 170); L.wraps = 0;
        L.eye = RGBc(200, 120, 255); L.hairStyle = 4; L.eyeGlow = 0.4f; L.palmEye = true;
        L.haori = true; L.h1 = RGBc(80, 100, 170); L.h2 = RGBc(60, 76, 140); L.cell = 6;
        break;
    case ET_DAKI:
        L.skin = RGBc(245, 222, 220); L.hair = e.assist ? RGBc(235, 235, 240) : RGBc(24, 18, 26); L.hairTip = RGBc(120, 220, 160);
        L.top = RGBc(22, 18, 26); L.pants = RGBc(30, 24, 34); L.sleeve = RGBc(245, 222, 220); L.wraps = 0;
        L.eye = RGBc(255, 210, 60); L.hairStyle = 5; L.eyeGlow = 0.4f; L.obi = true; L.longHair = true;
        break;
    case ET_GYUTARO:
        L.skin = RGBc(160, 150, 158); L.hair = RGBc(20, 26, 22); L.hairTip = RGBc(60, 130, 80);
        L.top = RGBc(44, 42, 34); L.pants = RGBc(54, 50, 42); L.sleeve = RGBc(160, 150, 158); L.wraps = 0;
        L.eye = RGBc(255, 220, 60); L.hairStyle = 6; L.eyeGlow = 0.5f; L.sickles = true; L.spots = true;
        break;
    case ET_AKAZA:
        L.skin = RGBc(240, 210, 200); L.hair = RGBc(240, 120, 160); L.hairTip = RGBc(255, 170, 200);
        L.top = RGBc(240, 210, 200); L.pants = RGBc(200, 196, 220); L.sleeve = RGBc(240, 210, 200); L.wraps = 0;
        L.eye = RGBc(255, 230, 90); L.hairStyle = 7; L.eyeGlow = 0.5f; L.tattoo = true; L.vest = RGBc(110, 70, 140);
        break;
    case ET_MUZAN:
        L.skin = RGBc(236, 226, 226); L.hair = RGBc(18, 16, 20); L.hairTip = L.hair;
        L.top = RGBc(26, 24, 30); L.pants = RGBc(22, 20, 26); L.sleeve = RGBc(26, 24, 30); L.wraps = 0;
        L.eye = RGBc(255, 20, 40); L.hairStyle = 8; L.eyeGlow = 0.7f; L.tentacles = true;
        break;
    case ET_NEZUKO:
        L.skin = RGBc(250, 220, 200); L.hair = RGBc(22, 16, 18); L.hairTip = RGBc(235, 110, 60);
        L.top = RGBc(230, 130, 160); L.pants = RGBc(250, 220, 200); L.sleeve = RGBc(230, 130, 160); L.wraps = 0;
        L.eye = RGBc(230, 80, 140); L.haori = true; L.h1 = RGBc(235, 140, 170); L.h2 = RGBc(250, 185, 200); L.cell = 3.f;
        L.muzzle = true; L.longHair = true; L.hairStyle = 3;
        break;
    }
    return L;
}

void drawShadow(float x, float z, float y, float scale) {
    float s = zScale(z);
    float k = std::max(0.3f, 1 - y / 220);
    fillEllipse(projX(x, z), groundY(z), 15 * s * scale * k, 4 * s * scale * k, 0x000000, (int)(110 * k));
}

float lyingRot(const Ent& e) {
    bool boss = isBoss(e.type);
    if (e.st == S_KNOCK) return -std::min(1.f, e.t * 3.5f) * PI * 0.5f;
    if (e.st == S_DOWN) return e.t > 0.6f && e.type != ET_PLAYER ? -PI * 0.5f * (1 - (e.t - 0.6f) / 0.3f) : -PI * 0.5f;
    if (e.st == S_DEAD) return boss ? -0.3f : (e.y > 0 ? -std::min(1.f, e.t * 3.5f) * PI * 0.5f : -PI * 0.5f);
    return 0;
}

void drawEnt(const Ent& e) {
    float s = zScale(e.z);
    float sx = projX(e.x, e.z), sy = groundY(e.z) - e.y * s;
    float rot = lyingRot(e);
    if (e.type == ET_PLAYER && e.st == S_DOWN && e.t > 0.55f) rot *= 1 - (e.t - 0.55f) / 0.25f;
    int face = e.face;
    if (e.type == ET_PLAYER && ((e.st == S_HINO && e.t < 1.0f) || (e.st == S_HINO_13 && e.t < 2.2f))) face = ((int)(e.t / 0.07f) & 1) ? e.face : -e.face;
    Xf xf = mkXf(sx, sy, s * e.scale, face, rot);
    g_fog = clampf(1 - e.z / DEPTH, 0, 1) * 0.3f;
    g_flash = e.flash > 0 ? 0.8f : 0;
    if (e.type == ET_PLAYER && e.inv > 0 && e.st != S_DOWN && ((int)(e.inv * 20) & 1)) g_flash = 0.35f;
    g_dis = e.st == S_DEAD && e.type != ET_PLAYER ? e.dieT / 1.2f : 0;
    Look L = lookFor(e);
    Pose q = e.type == ET_PLAYER ? playerPose(e) : enemyPose(e);
    if (e.type != ET_PLAYER && e.st == S_E_WIND) L.eyeGlow = 0.4f + smooth01(e.t / windDur(e)) * 1.2f;
    if (isBoss(e.type) && e.st == S_B_CAST) L.eyeGlow = 1.2f;
    if (e.type == ET_PLAYER) {
        if (e.st == S_WATER1 || e.st == S_WATER10) glow(sx, sy - 34 * s, 50 * s, RGBc(40, 110, 255), 60);
        if (e.st >= S_HINO && e.st <= S_HINO_13) glow(sx, sy - 34 * s, 70 * s, RGBc(255, 100, 20), 90);
        if (akatoT > 0) glow(sx, sy - 34 * s, 40 * s, RGBc(255, 40, 20), 40);
    }
    if (e.type == ET_AKAZA && e.st != S_DEAD) { // agulha da bússola sob os pés
        g_add = true;
        float gy = groundY(e.z);
        for (int k = 0; k < 6; k++) {
            float a = k / 6.f * PI + gameT * 0.3f;
            capsule(sx, gy, sx + cosf(a) * 30 * s, gy + sinf(a) * 30 * s * 0.32f, 0.6f, 0.6f, RGBc(90, 150, 255), 120);
            capsule(sx, gy, sx - cosf(a) * 30 * s, gy - sinf(a) * 30 * s * 0.32f, 0.6f, 0.6f, RGBc(90, 150, 255), 120);
        }
        ribbon(sx, gy, 30 * s, 1.2f, 0, 2 * PI, 0.32f, 0, 1, 1, RGBc(90, 150, 255), RGBc(90, 150, 255), 120, false);
        g_add = false;
    }
    if (e.assist) g_fog = std::max(g_fog, 0.25f);
    drawHumanoid(xf, L, q, e.anim);
    if (e.type == ET_RUI && e.st != S_DEAD) {
        g_add = true;
        float hx = TX(xf, q.fh.x, q.fh.y), hy = TY(xf, q.fh.x, q.fh.y);
        for (int i = 0; i < 4; i++) {
            float tx = hx + (i - 1.5f) * 40 + sinf(gameT + i) * 10;
            capsule(hx, hy, tx, -10, 0.4f, 0.4f, RGBc(255, 60, 90), 120);
        }
        if (e.st == S_B_CAST) glow(hx, hy, 18, RGBc(255, 40, 70), 160);
        g_add = false;
    }
    if (e.type == ET_ENMU && e.st == S_B_CAST) glow(TX(xf, q.fh.x, q.fh.y), TY(xf, q.fh.x, q.fh.y), 20, RGBc(190, 90, 255), 160);
    if (e.type == ET_PLAYER && e.st == S_SLEEP) {
        float hx = TX(xf, q.head.x, q.head.y), hy = TY(xf, q.head.x, q.head.y);
        for (int k = 0; k < 3; k++) {
            float u = fmodf(gameT * 0.8f + k / 3.f, 1.f);
            text(L"z", hx + 8 + u * 14, hy - 10 - u * 20, 6 + u * 6, RGBc(210, 180, 255), 1);
        }
    }
    g_fog = g_flash = g_dis = 0;
}

void drawNezuko() {
    if (!nzActive) return;
    float s = zScale(NZ.z);
    float sx = projX(NZ.x, NZ.z), sy = groundY(NZ.z) - NZ.y * s;
    Xf xf = mkXf(sx, sy, s * 0.9f, NZ.face, 0);
    Look L = lookFor(NZ);
    Pose q = poseStand(NZ.anim * 16, NZ.t > 0.2f ? 1.f : 0.f, 0);
    if (NZ.t > 0.2f && NZ.t < 0.85f) {
        leanPose(q, -5); q.ff = { 22, -24 }; q.bf = { -6, -2 };
        V2 sh = { q.neck.x, q.neck.y + 3 }; q.fh = { sh.x - 8, sh.y + 6 }; q.bh = { sh.x - 6, sh.y + 8 };
        glow(TX(xf, 20, -24), TY(xf, 20, -24), 20 * s, RGBc(255, 90, 140), 120);
    }
    finishPose(q);
    g_dis = NZ.t < 0.2f ? 1 - NZ.t / 0.2f : (NZ.t > 0.95f ? (NZ.t - 0.95f) / 0.3f : 0);
    drawHumanoid(xf, L, q, NZ.anim);
    g_dis = 0;
}

void drawItem(const Item& it) {
    float s = zScale(it.z);
    float sx = projX(it.x, it.z), gy = groundY(it.z);
    float bob = sinf(it.t * 4) * 2;
    fillEllipse(sx, gy, 6 * s, 2 * s, 0, 90);
    if (it.t > 11 && ((int)(it.t * 8) & 1)) return;
    float y = gy - (8 + bob) * s;
    float px[3] = { sx - 6 * s, sx + 6 * s, sx }, py[3] = { y + 5 * s, y + 5 * s, y - 6 * s };
    fillPoly(px, py, 3, RGBc(245, 245, 240));
    fillRect(sx - 3 * s, y + 1 * s, 6 * s, 4 * s, RGBc(20, 40, 30));
    glow(sx, y, 14 * s, RGBc(120, 255, 150), 50);
}

