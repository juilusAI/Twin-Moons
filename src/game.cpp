// Estado global do jogo, seções/checkpoints, câmera e o loop de atualização.
#include "common.h"

uint32_t rngState = 0x9E3779B9u;
float camX = 0, rcamX = 0, shakeY = 0, shakeT = 0, shakeMag = 0;
void shake(float mag, float t) { if (mag >= shakeMag || shakeT <= 0) { shakeMag = mag; shakeT = t; } }
Ent P, NZ;
bool nzActive = false, iteratingE = false;
std::vector<Ent> E, pendingE;
std::vector<Part> parts;
std::vector<Slash> slashes;
std::vector<Proj> projs;
std::vector<Petal> petals;
std::vector<Item> items;
std::vector<V3> dragon;
float dragonFade = 0;
bool dragonFire = false;
GameMode mode = GM_TITLE;
bool paused = false, selftest = false;
float breath = 50, nezCd = 0, dashCd = 0, comboT = 0, hitstop = 0, flashScreen = 0, hurtFlash = 0, akatoT = 0;
uint32_t flashColor = 0xFFFFFF;
int score = 0, combo = 0, maxCombo = 0, attackId = 0, kills = 0;
float modeT = 0, gameT = 0, dawn = 0, bannerT = 0, goT = 0, introT = 0, muzanT = 0;
std::wstring bannerJP, bannerPT;
bool bossEnraged = false, regenHint = false;
bool keyDown[256], keyPressed[256];
void addPart(float x, float z, float y, float vx, float vz, float vy, float life, uint32_t c, float size, int kind, float grav) {
    if (parts.size() > 2500) return;
    parts.push_back({ x, z, y, vx, vz, vy, life, life, size, grav, c, kind });
}
void addSlash(float x, float z, float y, int face, float dur, float r, float w, float a0, float a1, float squash, float tilt, int style) {
    slashes.push_back({ x, z, y, face, 0, dur, r, w, a0, a1, squash, tilt, style });
}
Proj& addProj(int kind, float x, float z, float y, float vx, float vz, float life, float dmg, int style) {
    projs.push_back({ x, z, y, vx, vz, life, 0, dmg, x, 0, kind, style, true, false, false });
    return projs.back();
}
void banner(const wchar_t* jp, const wchar_t* pt) { bannerJP = jp; bannerPT = pt; bannerT = 2.2f; }
void updateParts(float dt) {
    for (auto& p : parts) {
        p.life -= dt;
        p.x += p.vx * dt; p.z += p.vz * dt; p.y += p.vy * dt;
        p.vy -= p.grav * dt;
        if (p.kind == P_ASH) { p.vx += frange(-60, 60) * dt; p.vx *= 0.99f; }
        if (p.kind == P_SPARK || p.kind == P_WATER) { p.vx *= 0.92f; p.vy *= 0.92f; }
        if (p.kind == P_DOT && p.y < 0) { p.y = 0; p.vx = p.vy = p.vz = 0; p.grav = 0; }
    }
    parts.erase(std::remove_if(parts.begin(), parts.end(), [](const Part& p) { return p.life <= 0; }), parts.end());
    for (auto& s : slashes) {
        s.t += dt;
        if (s.style == SL_WATER && s.t < s.dur * 0.4f) {
            float u = std::min(1.f, s.t / (s.dur * 0.35f)), a = s.a0 + (s.a1 - s.a0) * u;
            float lx = cosf(a) * s.r, ly = sinf(a) * s.r * s.squash;
            addPart(s.x + lx * s.face, s.z + ly * 0.2f, s.y - ly, frange(-40, 40), 0, frange(-40, 60), frange(0.3f, 0.6f), RGBc(90, 170, 255), frange(1.5f, 3.f), P_WATER, 300);
        }
        if (s.style == SL_FIRE && s.t < s.dur * 0.5f && (rnd() & 1)) {
            float u = std::min(1.f, s.t / (s.dur * 0.35f)), a = s.a0 + (s.a1 - s.a0) * u;
            float lx = cosf(a) * s.r, ly = sinf(a) * s.r * s.squash;
            addPart(s.x + lx * s.face, s.z, s.y - ly, frange(-30, 30), 0, frange(30, 110), frange(0.3f, 0.6f), 0, frange(1.2f, 2.4f), P_FIRE);
        }
    }
    slashes.erase(std::remove_if(slashes.begin(), slashes.end(), [](const Slash& s) { return s.t >= s.dur; }), slashes.end());
    if (P.st != S_WATER10 && P.st != S_HINO_DRAGON && P.st != S_HINO_13) {
        dragonFade -= dt * 2.5f;
        if (dragonFade <= 0) dragon.clear();
    }
    for (auto& it : items) if (it.t >= 0) it.t += dt;
    items.erase(std::remove_if(items.begin(), items.end(), [](const Item& i) { return i.t < 0 || i.t > 14; }), items.end());
}

// ============================================================ fases
void resetGame() {
    P = Ent();
    P.type = ET_PLAYER; P.x = 140; P.z = 55; P.hp = P.maxhp = 100;
    E.clear(); parts.clear(); slashes.clear(); projs.clear(); dragon.clear(); items.clear();
    nzActive = false; breath = 50; nezCd = 3; dashCd = 0; akatoT = 0; score = 0; combo = 0; maxCombo = 0; kills = 0;
    hitstop = 0; flashScreen = 0; hurtFlash = 0; dawn = 0; bannerT = 0; goT = 0; bossEnraged = false; regenHint = false; muzanT = 0;
    secs = chapters[chapter].secs;
    curSec = 0; secLocked = true; spawnIdx = 0; spawnT = 1.6f; introT = 2.5f;
    camX = rcamX = 300; modeT = 0;
}

void jumpToSection(int k) {
    E.clear(); projs.clear(); items.clear();
    curSec = k; secLocked = true; spawnIdx = 0; spawnT = 0.6f; introT = 2.5f; bossEnraged = false; regenHint = false;
    P.x = secs[k].l + 170; P.z = 55;
    camX = rcamX = std::max(secs[k].l + 275, P.x);
}

void startChapter(int c, bool fromCheckpoint) {
    chapter = c;
    resetGame();
    if (fromCheckpoint && checkpoint > 0 && checkpoint < (int)secs.size()) { jumpToSection(checkpoint); breath = 100; }
    else checkpoint = 0;
    mode = GM_PLAY; paused = false;
}

void updateSections(float dt) {
    Section& s = secs[curSec];
    if (secLocked) {
        spawnT -= dt;
        int maxAlive = curSec >= 2 ? 5 : 4;
        if (spawnIdx < s.wave.size() && spawnT <= 0 && livingEnemies() < maxAlive) {
            EType t = s.wave[spawnIdx++];
            if (isBoss(t)) spawnBoss(t);
            else {
                float side = (curSec == 0 || (rnd() & 1)) ? 1.f : -1.f;
                float x = clampf(rcamX + side * frange(340, 400), s.l - 60, s.r + 60);
                if (fabsf(x - P.x) < 200) x = rcamX - side * 370;
                spawnEnemy(t, x, frange(8, 92));
            }
            spawnT = frange(0.5f, 1.1f);
        }
        if (spawnIdx >= s.wave.size() && livingEnemies() == 0) {
            secLocked = false;
            goT = 3.0f;
            P.hp = std::min(P.maxhp, P.hp + 25);
            if (curSec == (int)secs.size() - 1) {
                modeT = 0;
                unlocked = std::max(unlocked, std::min(chapter + 1, (int)chapters.size() - 1));
                writeSave();
                mode = chapter == (int)chapters.size() - 1 ? GM_WIN : GM_CLEAR;
            }
        }
    } else if (curSec + 1 < (int)secs.size() && P.x > secs[curSec + 1].l + 160) {
        curSec++; checkpoint = curSec; secLocked = true; spawnIdx = 0; spawnT = 0.6f; introT = 2.5f; bossEnraged = false; regenHint = false;
    }
    // Muzan: o amanhecer chega
    if (chapter == 4 && curSec == (int)secs.size() - 1) {
        Ent* m = findType(ET_MUZAN);
        if (m && m->st != S_B_INTRO) {
            muzanT += dt;
            if (muzanT >= MUZAN_LIMIT) {
                m->hp = 0; m->st = S_DEAD; m->t = 0; m->dieT = 0; m->vx = 0;
                banner(L"夜明けだ！", L"O sol nasceu! Muzan queima sob a luz do amanhecer!");
                flashScreen = 1; flashColor = RGBc(255, 230, 180); shake(8, 1); play(sfxDie);
                for (auto& e : E) if (e.alive && e.st != S_DEAD) { e.st = S_DEAD; e.t = 0; e.dieT = 0; }
            }
        }
        dawn = std::min(0.55f, muzanT / MUZAN_LIMIT * 0.55f);
    }
}

void boundsFor(float& l, float& r) {
    const Section& s = secs[curSec];
    l = s.l; r = s.r;
    if (!secLocked && curSec + 1 < (int)secs.size()) r = secs[curSec + 1].r;
}

void updateCamera(float dt) {
    float l, r; boundsFor(l, r);
    const float half = 275;
    float target = P.x + P.face * 30;
    float cl = l + half, cr = r - half;
    if (cl > cr) cl = cr = (l + r) * 0.5f;
    target = clampf(target, cl, cr);
    camX += (target - camX) * std::min(1.f, dt * 5);
    if (shakeT > 0) {
        shakeT -= dt;
        rcamX = camX + frange(-1, 1) * shakeMag;
        shakeY = frange(-1, 1) * shakeMag * 0.6f;
    } else { rcamX = camX; shakeY = 0; }
    float vis = (W * 0.5f) / S_NEAR - 18;
    P.x = clampf(P.x, std::max(l + 15, camX - vis), std::min(r - 15, camX + vis));
}

void updatePetals(float dt) {
    while (petals.size() < 45) petals.push_back({ frange(-20, W + 20), frange(-H, 0), frange(-25, -5), frange(15, 35), frange(0, 6.28f) });
    float wind = curBG() == BG_TRAIN ? -260.f : 0.f;
    for (auto& p : petals) {
        p.ph += dt * 2;
        p.x += (p.vx + wind + sinf(p.ph) * 15) * dt;
        p.y += p.vy * dt;
        if (p.y > H + 5 || p.x < -30) { p.x = frange(0, W + 60); p.y = frange(-40, H * (wind < 0 ? 0.6f : 0.f)); if (wind < 0) p.x = W + frange(0, 40); }
    }
}

void update(float dt) {
    gameT += dt;
    updatePetals(dt);
    if (bannerT > 0) bannerT -= dt;
    if (flashScreen > 0) flashScreen -= dt * 2.2f;
    if (hurtFlash > 0) hurtFlash -= dt;
    modeT += dt;

    if (mode == GM_TITLE) {
        camX += dt * 25; rcamX = camX;
        if (camX > 2400) camX = 300;
        if (kp(VK_LEFT) || kp('A')) titleSel = std::max(0, titleSel - 1);
        if (kp(VK_RIGHT) || kp('D')) titleSel = std::min(unlocked, titleSel + 1);
        chapter = titleSel;
        if (kp(VK_RETURN) || kp('J')) { mode = GM_STORY; modeT = 0; }
        if (kp(VK_ESCAPE)) PostQuitMessage(0);
        return;
    }
    if (mode == GM_STORY) {
        camX += dt * 25; rcamX = camX;
        if (modeT > 0.4f && (kp(VK_RETURN) || kp('J'))) startChapter(chapter);
        if (kp(VK_ESCAPE)) { mode = GM_TITLE; modeT = 0; }
        return;
    }
    if (mode == GM_PLAY && (kp('P') || kp(VK_ESCAPE))) paused = !paused;
    if (paused) {
        if (kp('Q')) { paused = false; mode = GM_TITLE; camX = 300; titleSel = chapter; }
        return;
    }
    if (mode == GM_OVER && modeT > 1.5f) {
        if (kp(VK_RETURN)) { startChapter(chapter, true); return; }
        if (kp(VK_ESCAPE)) { mode = GM_TITLE; titleSel = chapter; return; }
    }
    if (mode == GM_CLEAR && modeT > 1.5f && kp(VK_RETURN)) { chapter++; titleSel = chapter; mode = GM_STORY; modeT = 0; camX = 300; return; }
    if (mode == GM_WIN) {
        dawn = std::min(1.f, dawn + dt * 0.25f);
        if (modeT > 2.5f && kp(VK_RETURN)) { mode = GM_TITLE; titleSel = chapter; camX = 300; return; }
    }

    if (hitstop > 0) { hitstop -= dt; return; }
    if (introT > 0) introT -= dt;
    if (goT > 0) goT -= dt;
    if (comboT > 0) { comboT -= dt; if (comboT <= 0) combo = 0; }

    updatePlayer(dt);
    updateNezuko(dt);
    iteratingE = true;
    for (size_t i = 0; i < E.size(); i++) updateEnemy(E[i], dt);
    iteratingE = false;
    E.insert(E.end(), pendingE.begin(), pendingE.end());
    pendingE.clear();
    E.erase(std::remove_if(E.begin(), E.end(), [](const Ent& e) { return !e.alive; }), E.end());
    updateProjs(dt);
    updateParts(dt);
    if (mode == GM_PLAY) updateSections(dt);
    updateCamera(dt);
    if (mode == GM_PLAY && P.st == S_DEAD && P.t > 2.2f) { mode = GM_OVER; modeT = 0; }
}

