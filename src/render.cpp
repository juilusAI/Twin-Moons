// Renderizador por software: pixels, formas, fitas em arco e transformação local dos personagens.
#include "common.h"

uint32_t fb[W * H];
// ============================================================ renderer
struct Pattern { bool on = false; float ox, oy, s, c, sn; int f; float cell; uint32_t c1, c2; };
Pattern g_pat;
bool g_add = false;   // blend aditivo
float g_dis = 0;      // dissolução (cinzas) 0..1
float g_flash = 0;    // clarão branco ao tomar dano
float g_fog = 0;      // névoa de profundidade
uint32_t fogC = 0x161830;

void plot(int x, int y, uint32_t c, int a) {
    if ((unsigned)x >= (unsigned)W || (unsigned)y >= (unsigned)H || a <= 0) return;
    if (g_dis > 0) {
        int h = (int)(hash2(x, y) & 255), th = (int)(g_dis * 300) - 20;
        if (h < th) return;
        if (h < th + 28) c = 0xFF7A30;
    }
    if (g_fog > 0) c = lerpC(c, fogC, g_fog);
    if (g_flash > 0) c = lerpC(c, 0xFFFFFF, g_flash);
    uint32_t& d = fb[y * W + x];
    int sr = (c >> 16) & 255, sg = (c >> 8) & 255, sb = c & 255;
    int dr = (d >> 16) & 255, dg = (d >> 8) & 255, db = d & 255;
    if (a > 255) a = 255;
    if (g_add) {
        dr += sr * a >> 8; dg += sg * a >> 8; db += sb * a >> 8;
        dr = std::min(dr, 255); dg = std::min(dg, 255); db = std::min(db, 255);
    } else if (a >= 255) { d = c; return; }
    else { dr += (sr - dr) * a >> 8; dg += (sg - dg) * a >> 8; db += (sb - db) * a >> 8; }
    d = (uint32_t)((dr << 16) | (dg << 8) | db);
}

void hspan(int y, float xa, float xb, uint32_t c, int a) {
    if (y < 0 || y >= H) return;
    int x0 = (int)ceilf(xa - 0.5f), x1 = (int)floorf(xb - 0.5f);
    if (x0 < 0) x0 = 0;
    if (x1 > W - 1) x1 = W - 1;
    if (g_pat.on) {
        for (int x = x0; x <= x1; x++) {
            float lx = ((x + 0.5f) - g_pat.ox) / (g_pat.s * g_pat.f), ly = ((y + 0.5f) - g_pat.oy) / g_pat.s;
            float ux = lx * g_pat.c + ly * g_pat.sn, uy = -lx * g_pat.sn + ly * g_pat.c;
            int cx = (int)floorf(ux / g_pat.cell), cy = (int)floorf(uy / g_pat.cell);
            plot(x, y, ((cx + cy) & 1) ? g_pat.c1 : g_pat.c2, a);
        }
    } else {
        for (int x = x0; x <= x1; x++) plot(x, y, c, a);
    }
}

void fillRect(float x, float y, float w, float h, uint32_t c, int a) {
    int y0 = (int)floorf(y), y1 = (int)ceilf(y + h) - 1;
    for (int yy = y0; yy <= y1; yy++) hspan(yy, x, x + w, c, a);
}

void fillPoly(const float* px, const float* py, int n, uint32_t c, int a) {
    float miny = 1e9f, maxy = -1e9f;
    for (int i = 0; i < n; i++) { miny = std::min(miny, py[i]); maxy = std::max(maxy, py[i]); }
    int y0 = std::max(0, (int)ceilf(miny - 0.5f)), y1 = std::min(H - 1, (int)floorf(maxy - 0.5f));
    float xs[32];
    for (int y = y0; y <= y1; y++) {
        float sy = y + 0.5f;
        int k = 0;
        for (int i = 0, j = n - 1; i < n; j = i++) {
            float yi = py[i], yj = py[j];
            if ((yi <= sy && yj > sy) || (yj <= sy && yi > sy)) {
                float t = (sy - yi) / (yj - yi);
                if (k < 32) xs[k++] = px[i] + t * (px[j] - px[i]);
            }
        }
        std::sort(xs, xs + k);
        for (int i = 0; i + 1 < k; i += 2) hspan(y, xs[i], xs[i + 1], c, a);
    }
}

void fillEllipse(float cx, float cy, float rx, float ry, uint32_t c, int a) {
    if (rx < 0.5f) rx = 0.5f;
    if (ry < 0.5f) ry = 0.5f;
    int y0 = (int)floorf(cy - ry), y1 = (int)ceilf(cy + ry);
    for (int y = y0; y <= y1; y++) {
        float dy = (y + 0.5f - cy) / ry, d = 1 - dy * dy;
        if (d < 0) continue;
        float hw = rx * sqrtf(d);
        hspan(y, cx - hw, cx + hw, c, a);
    }
}
void fillCircle(float cx, float cy, float r, uint32_t c, int a) { fillEllipse(cx, cy, r, r, c, a); }

void capsule(float x0, float y0, float x1, float y1, float r0, float r1, uint32_t c, int a) {
    float dx = x1 - x0, dy = y1 - y0, L = sqrtf(dx * dx + dy * dy);
    if (L > 0.01f) {
        float nx = -dy / L, ny = dx / L;
        float px[4] = { x0 + nx * r0, x1 + nx * r1, x1 - nx * r1, x0 - nx * r0 };
        float py[4] = { y0 + ny * r0, y1 + ny * r1, y1 - ny * r1, y0 - ny * r0 };
        fillPoly(px, py, 4, c, a);
    }
    fillCircle(x0, y0, r0, c, a);
    fillCircle(x1, y1, r1, c, a);
}

// brilho radial aditivo
void glow(float cx, float cy, float r, uint32_t c, int a) {
    if (r < 1) return;
    bool old = g_add; g_add = true;
    int x0 = std::max(0, (int)(cx - r)), x1 = std::min(W - 1, (int)(cx + r));
    int y0 = std::max(0, (int)(cy - r)), y1 = std::min(H - 1, (int)(cy + r));
    float inv = 1.f / r;
    for (int y = y0; y <= y1; y++)
        for (int x = x0; x <= x1; x++) {
            float dx = (x + 0.5f - cx) * inv, dy = (y + 0.5f - cy) * inv, d = dx * dx + dy * dy;
            if (d >= 1) continue;
            float f = 1 - sqrtf(d);
            plot(x, y, c, (int)(a * f * f));
        }
    g_add = old;
}

// fita em arco (cortes de espada, anéis)
void ribbon(float cx, float cy, float r, float w, float a0, float a1, float squash, float tilt, int face, float s,
                   uint32_t cA, uint32_t cB, int alpha, bool headHeavy) {
    float span = fabsf(a1 - a0);
    int N = std::max(6, std::min(80, (int)(span * r * s / 3)));
    float ct = cosf(tilt), st = sinf(tilt);
    auto pt = [&](float ang, float rad, float& ox, float& oy) {
        float lx = cosf(ang) * rad, ly = sinf(ang) * rad * squash;
        float rx = lx * ct - ly * st, ry = lx * st + ly * ct;
        ox = cx + rx * s * face; oy = cy + ry * s;
    };
    for (int i = 0; i < N; i++) {
        float u0 = (float)i / N, u1 = (float)(i + 1) / N, um = (u0 + u1) * 0.5f;
        float tap = headHeavy ? powf(um, 0.8f) : sinf(um * PI);
        if (!headHeavy && squash >= 0) tap = 1;
        float g0 = a0 + (a1 - a0) * u0, g1 = a0 + (a1 - a0) * u1;
        float hw = w * tap * 0.5f;
        float px[4], py[4];
        pt(g0, r + hw, px[0], py[0]); pt(g1, r + hw, px[1], py[1]);
        pt(g1, r - hw, px[2], py[2]); pt(g0, r - hw, px[3], py[3]);
        fillPoly(px, py, 4, lerpC(cA, cB, um), (int)(alpha * (0.25f + 0.75f * tap)));
    }
}

// ---------- transformação local (personagens)
Xf mkXf(float ox, float oy, float s, int f, float rot) { return { ox, oy, s, cosf(rot), sinf(rot), f }; }
void Lcap(const Xf& x, float x0, float y0, float x1, float y1, float r0, float r1, uint32_t c, int a) {
    capsule(TX(x, x0, y0), TY(x, x0, y0), TX(x, x1, y1), TY(x, x1, y1), r0 * x.s, r1 * x.s, c, a);
}
void Lcirc(const Xf& x, float lx, float ly, float r, uint32_t c, int a) { fillCircle(TX(x, lx, ly), TY(x, lx, ly), r * x.s, c, a); }
void Lpoly(const Xf& x, std::initializer_list<float> pts, uint32_t c, int a) {
    float px[16], py[16];
    int n = 0;
    for (auto it = pts.begin(); it != pts.end() && n < 16; it += 2, n++) {
        float lx = *it, ly = *(it + 1);
        px[n] = TX(x, lx, ly); py[n] = TY(x, lx, ly);
    }
    fillPoly(px, py, n, c, a);
}
void patOn(const Xf& x, float cell, uint32_t c1, uint32_t c2) { g_pat = { true, x.ox, x.oy, x.s, x.c, x.sn, x.f, cell, c1, c2 }; }
void patOff() { g_pat.on = false; }

// ============================================================ texto (GDI, desenhado sobre a imagem escalada)
std::vector<TextCmd> texts;
void text(const std::wstring& s, float x, float y, float size, uint32_t col, int align, int font) {
    texts.push_back({ s, x, y, size, col, align, font });
}

