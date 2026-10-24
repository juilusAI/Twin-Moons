// Inimigos comuns, chefes (Luas Demoníacas e Muzan) e projéteis.
#include "common.h"

// ============================================================ jogo: inimigos
int tokensUsed() { int n = 0; for (auto& e : E) if (e.alive && e.token && e.st != S_DEAD) n++; return n; }
int livingEnemies() { int n = 0; for (auto& e : E) if (e.alive && e.st != S_DEAD && !e.assist) n++; return n; }
Ent* findType(EType t) { for (auto& e : E) if (e.alive && e.type == t && e.st != S_DEAD && !e.assist) return &e; return nullptr; }

void spawnEnemy(EType t, float x, float z) {
    Ent e;
    e.type = t; e.x = x; e.z = z; e.face = x < P.x ? 1 : -1;
    e.idx = (int)(rnd() % 7);
    e.strafe = frange(-35, 35);
    e.cd = frange(0.4f, 1.4f);
    float hpMul = 1 + chapter * 0.12f;
    switch (t) {
    case ET_GRUNT: e.hp = 45 * hpMul; break;
    case ET_FAST: e.hp = 32 * hpMul; e.scale = 0.95f; break;
    case ET_BIG: e.hp = 220 * hpMul; e.scale = 1.45f; e.armor = true; break;
    default: break;
    }
    e.maxhp = e.hp;
    // durante o loop de inimigos o vetor não pode realocar: guarda para depois
    if (iteratingE) pendingE.push_back(e);
    else E.push_back(e);
}

void spawnBoss(EType t, bool assist) {
    const Section& s = secs[curSec];
    Ent e;
    e.type = t; e.x = s.r - 150; e.z = 50; e.face = -1; e.armor = true;
    e.st = S_B_INTRO; e.aiT = 1.6f; e.y = 220; e.cd2 = 2;
    switch (t) {
    case ET_RUI: e.hp = 700; e.scale = 0.92f; e.floats = true; e.y = 260; break;
    case ET_ENMU: e.hp = 750; break;
    case ET_DAKI: e.hp = 480; break;
    case ET_GYUTARO: e.hp = 950; e.scale = 1.06f; break;
    case ET_AKAZA: e.hp = 1100; e.scale = 1.04f; e.regen = 7; break;
    case ET_MUZAN: e.hp = 1600; e.scale = 1.1f; e.regen = 5; break;
    default: break;
    }
    if (assist) { e.assist = true; e.x = s.r - 70; e.z = 10; e.hp = 9999; e.aiT = 4; }
    e.maxhp = e.hp;
    E.push_back(e);
    if (assist) return;
    switch (t) {
    case ET_RUI: banner(L"十二鬼月　下弦の伍　累", L"Rui — Lua Inferior Cinco"); break;
    case ET_ENMU: banner(L"十二鬼月　下弦の壱　魘夢", L"Enmu — Lua Inferior Um"); break;
    case ET_DAKI: banner(L"十二鬼月　上弦の陸　堕姫", L"Daki — Lua Superior Seis"); break;
    case ET_GYUTARO: banner(L"上弦の陸　妓夫太郎", L"Gyutaro desperta de dentro de Daki!"); spawnBoss(ET_DAKI, true); break;
    case ET_AKAZA: banner(L"十二鬼月　上弦の参　猗窩座", L"Akaza — Lua Superior Três"); break;
    case ET_MUZAN: banner(L"鬼舞辻無惨", L"Muzan Kibutsuji, o primeiro demônio"); muzanT = 0; break;
    default: break;
    }
    play(sfxThread);
}

const wchar_t* bossName(EType t) {
    switch (t) {
    case ET_RUI: return L"下弦の伍 · 累  (Rui)";
    case ET_ENMU: return L"下弦の壱 · 魘夢  (Enmu)";
    case ET_DAKI: return L"上弦の陸 · 堕姫  (Daki)";
    case ET_GYUTARO: return L"上弦の陸 · 妓夫太郎  (Gyutaro)";
    case ET_AKAZA: return L"上弦の参 · 猗窩座  (Akaza)";
    case ET_MUZAN: return L"鬼舞辻無惨  (Muzan)";
    default: return L"";
    }
}


void killFx(Ent& e) {
    for (int i = 0; i < 24; i++)
        addPart(e.x + frange(-10, 10), e.z, e.y + frange(5, 50) * e.scale, frange(-40, 40), frange(-10, 10), frange(20, 90), frange(0.8f, 1.6f),
                (rnd() & 3) ? RGBc(40, 36, 40) : RGBc(255, 120, 40), frange(1.f, 2.5f), P_ASH, -20);
}

void bossDefeated(Ent& e) {
    hitstop = 0.5f; flashScreen = 1; flashColor = 0xFFFFFF; shake(10, 0.8f);
    switch (e.type) {
    case ET_DAKI: banner(L"堕姫「お兄ちゃん...！」", L"Daki foge chorando... algo desperta dentro dela."); break;
    case ET_GYUTARO:
        banner(L"妓夫太郎「...」", L"As duas cabeças rolaram! A Lua Superior Seis caiu.");
        for (auto& o : E) if (o.assist && o.st != S_DEAD) { o.st = S_DEAD; o.t = 0; o.dieT = 0; o.vy = 0; }
        break;
    case ET_AKAZA: banner(L"猗窩座「...恋雪」", L"Akaza aceita a derrota e se desfaz."); break;
    case ET_MUZAN: banner(L"無惨「まだだ...！」", L"Muzan foi derrotado antes do amanhecer!"); break;
    case ET_ENMU: banner(L"魘夢「そんな...」", L"O pescoço do trem foi cortado!"); break;
    default: banner(L"累「...」", L"Rui se desfaz em cinzas."); break;
    }
}

void damageEnemy(Ent& e, float dmg, float kbx, bool launch, int fx) {
    if (e.assist) return;
    e.hp -= dmg;
    e.flash = 0.1f;
    hitstop = std::max(hitstop, launch ? 0.085f : 0.045f);
    shake(launch ? 5.f : 2.5f, 0.15f);
    combo++; comboT = 2.4f; maxCombo = std::max(maxCombo, combo);
    score += (int)(dmg * 10) * (1 + combo / 10);
    breath = std::min(100.f, breath + 2.5f);
    float hy = e.y + 34 * e.scale;
    uint32_t sc = fx == 2 ? RGBc(255, 160, 60) : (fx == 1 ? RGBc(120, 200, 255) : RGBc(230, 240, 255));
    if (akatoT > 0) sc = RGBc(255, 90, 60);
    for (int i = 0; i < 10; i++) {
        float a = frange(0, 2 * PI), sp = frange(120, 320);
        addPart(e.x, e.z + 1, hy, cosf(a) * sp, frange(-30, 30), sinf(a) * sp, frange(0.12f, 0.3f), sc, 1.2f, P_SPARK);
    }
    for (int i = 0; i < 6; i++)
        addPart(e.x, e.z + 1, hy, frange(-60, 60) + kbx * 0.3f, frange(-20, 20), frange(40, 180), frange(0.4f, 0.8f), RGBc(120, 10, 25), frange(1.2f, 2.2f), P_DOT, 600);
    addPart(e.x, e.z + 1, hy, 0, 0, 0, 0.18f, sc, 14, P_RING);
    play(launch ? sfxHeavy : sfxHit, launch ? 0.9f : 0.7f);
    if (e.hp <= 0) {
        e.hp = 0; e.st = S_DEAD; e.t = 0; e.dieT = 0; e.token = false;
        e.vx = kbx * (isBoss(e.type) ? 0.3f : 1.3f); e.vy = isBoss(e.type) ? 120.f : 260.f; e.y = std::max(e.y, 1.f);
        kills++;
        score += isBoss(e.type) ? 20000 : (e.type == ET_BIG ? 3000 : 800);
        play(sfxDie, 0.9f);
        if (isBoss(e.type)) bossDefeated(e);
        else {
            if (livingEnemies() == 0) hitstop = 0.18f;
            if (e.type == ET_BIG || frand() < 0.22f) items.push_back({ e.x, e.z, 0 });
        }
        return;
    }
    if (e.st == S_KNOCK) { e.vy = std::max(e.vy, 170.f); e.vx = kbx * 0.6f; return; }
    if (launch && !isBoss(e.type) && (!e.armor || dmg >= 30)) {
        e.st = S_KNOCK; e.t = 0; e.vy = 330; e.vx = kbx; e.y = std::max(e.y, 1.f); e.token = false;
    } else if (!e.armor) {
        e.st = S_HURT; e.t = 0; e.vx = kbx; e.token = false;
    } else if (!isBoss(e.type)) {
        e.vx += kbx * 0.15f;
    }
}

// golpe do jogador: área relativa à direção que ele encara
int playerHit(float r0, float r1, float zt, float yt, float dmg, float kb, bool launch, int fx, bool radial) {
    int hits = 0;
    if (akatoT > 0) dmg *= 1.4f;
    for (auto& e : E) {
        if (!e.alive || e.assist || e.st == S_DEAD || e.st == S_B_INTRO || e.lastHit == attackId) continue;
        float dx = (e.x - P.x) * P.face;
        float pad = 10 * e.scale;
        if (radial) dx = fabsf(e.x - P.x);
        if (dx < r0 - pad || dx > r1 + pad) continue;
        if (fabsf(e.z - P.z) > zt + 4 * e.scale) continue;
        if (fabsf(e.y - P.y) > yt + 20) continue;
        e.lastHit = attackId;
        float dir = radial ? signf(e.x - P.x) : (float)P.face;
        damageEnemy(e, dmg, dir * kb, launch, fx);
        hits++;
    }
    return hits;
}

// ============================================================ jogo: IA
void enemyPhysics(Ent& e, float dt) {
    e.x += e.vx * dt;
    e.z += e.vz * dt;
    if (e.floats && e.st != S_DEAD && e.st != S_B_INTRO) return;
    if (e.floats && e.st == S_B_INTRO) return;
    if (e.y > 0 || e.vy > 0) {
        e.vy -= 1200 * dt;
        e.y += e.vy * dt;
        if (e.y <= 0) {
            e.y = 0;
            if (e.st == S_KNOCK) {
                if (e.vy < -250) { e.vy = -e.vy * 0.3f; e.y = 0.1f; addPart(e.x, e.z, 0, 0, 0, 0, 0.3f, RGBc(160, 150, 180), 16, P_RING); shake(2, 0.1f); return; }
                e.st = S_DOWN; e.t = 0; e.vx *= 0.3f;
            }
            if (e.st == S_B_INTRO && e.vy < -200) {
                shake(6, 0.3f); play(sfxSlam, 0.8f);
                addPart(e.x, e.z, 0, 0, 0, 0, 0.5f, RGBc(200, 160, 160), 40, P_RING);
            }
            e.vy = 0;
        }
    }
}

float windDur(const Ent& e) { return e.type == ET_BIG ? 0.75f : (e.type == ET_FAST ? 0.45f : 0.42f); }

// ---------- chefes
void bossMove(Ent& e, float prefD, float spd) {
    const Section& s = secs[curSec];
    float dx = P.x - e.x;
    e.face = dx > 0 ? 1 : -1;
    float side = dx > 0 ? -1.f : 1.f;
    float tx = clampf(P.x + side * prefD, s.l + 40, s.r - 40);
    if (fabsf(tx - P.x) < prefD * 0.6f) tx = clampf(P.x - side * prefD, s.l + 40, s.r - 40);
    e.vx = clampf((tx - e.x) * 3, -spd, spd);
    e.vz = clampf((P.z - e.z) * 1.5f, -spd * 0.5f, spd * 0.5f);
    e.st = fabsf(e.vx) > 12 ? S_WALK : S_IDLE;
}
void bossTeleport(Ent& e) {
    const Section& s = secs[curSec];
    for (int i = 0; i < 16; i++) addPart(e.x, e.z, frange(0, 60), frange(-80, 80), 0, frange(-30, 60), 0.5f, RGBc(255, 60, 80), 1.2f, P_SPARK);
    float side = P.x > e.x ? -1.f : 1.f;
    e.x = clampf(P.x - side * 240, s.l + 40, s.r - 40);
    if (fabsf(e.x - P.x) < 120) e.x = clampf(P.x + side * 240, s.l + 40, s.r - 40);
    e.cd2 = 3.0f; play(sfxThread, 0.7f);
}
bool bossContact(Ent& e, float range, float dz, float dmg, bool knock) {
    if (e.hitDone) return false;
    float f = (P.x - e.x) * e.face;
    if (f > -10 && f < range && fabsf(P.z - e.z) < dz && P.y < 45) { e.hitDone = true; hurtPlayer(dmg, e.x, knock); return true; }
    return false;
}
void rainAround(int n, int style, float dmg, float spread) {
    for (int k = 0; k < n; k++) {
        float x = k == 0 ? P.x : P.x + frange(-spread, spread), z = k == 0 ? P.z : clampf(P.z + frange(-40, 40), 3, 97);
        addProj(PR_RAIN, x, z, 0, 0, 0, 1.0f + k * 0.07f, dmg, style);
    }
}
void addSweep(float z, float dmg, int style, float life) {
    Proj& p = addProj(PR_SWEEP, rcamX - 340, z, 0, 0, 0, life, dmg, style);
    p.x2 = rcamX + 340;
}
void addLine(float x0, float x1, float z, float dmg, int style, float life) {
    Proj& p = addProj(PR_LINE, std::min(x0, x1), z, 0, 0, 0, life, dmg, style);
    p.x2 = std::max(x0, x1);
}

void enrage(Ent& e) {
    bossEnraged = true;
    auto adds = [&](int n) { for (int i = 0; i < n; i++) spawnEnemy(ET_GRUNT, rcamX + (i % 2 ? 360.f : -360.f), frange(10, 90)); };
    switch (e.type) {
    case ET_RUI: banner(L"累「僕の糸は切れない」", L"Rui enfurece: seus fios ficam vermelhos de sangue!"); adds(3); break;
    case ET_ENMU: banner(L"魘夢「もっと深い眠りへ...」", L"Enmu se funde ao trem!"); adds(2); break;
    case ET_DAKI: banner(L"堕姫「醜い！醜い！」", L"Daki solta todas as faixas de obi!"); break;
    case ET_GYUTARO: banner(L"妓夫太郎「取り立てるぜ」", L"Gyutaro libera o Sangue Rodopiante!"); break;
    case ET_AKAZA: banner(L"猗窩座「破壊殺・滅式」", L"Akaza prepara a Destruição Letal: Aniquilação!"); break;
    case ET_MUZAN: banner(L"無惨「しつこい」", L"Muzan perde a paciência!"); adds(3); break;
    default: break;
    }
    flashScreen = 0.5f; flashColor = RGBc(200, 20, 40); play(sfxThread);
}

int choosePattern(Ent& e) {
    int r = (int)(rnd() % 100);
    switch (e.type) {
    case ET_RUI: return r < 65 ? 0 : 1;
    case ET_ENMU: return r < 45 ? 0 : (r < 85 || livingEnemies() > 3 ? 1 : 2);
    case ET_DAKI: return e.assist ? (r < 60 ? 0 : 1) : (r < 40 ? 0 : (r < 70 ? 1 : 2));
    case ET_GYUTARO: return r < 40 ? 0 : (r < 70 ? 1 : 2);
    case ET_AKAZA: return bossEnraged && r < 20 ? 3 : (r < 45 ? 0 : (r < 75 ? 1 : 2));
    case ET_MUZAN: return r < 40 ? 0 : (r < 75 ? 1 : 2);
    default: return 0;
    }
}

// executa o golpe atual; devolve true quando terminou
bool runPattern(Ent& e, float dt) {
    float side = (float)e.face;
    switch (e.type) {
    case ET_RUI:
        e.vx = e.vz = 0;
        if (e.pat == 0) {
            if (e.t > 0.7f && !e.fired) {
                e.fired = true;
                int n = bossEnraged ? 7 : 5;
                for (int k = 0; k < n; k++) addProj(PR_THREAD, e.x + side * 15, e.z, 32, side * 330.f, (k - (n - 1) * 0.5f) * 38.f, 2.6f, 10);
                play(sfxThread);
            }
            return e.t > 1.0f;
        }
        if (e.t > 0.5f && !e.fired) { e.fired = true; rainAround(bossEnraged ? 8 : 5, 0, 12, 110); play(sfxThread, 0.6f); }
        return e.t > 1.1f;
    case ET_ENMU:
        e.vx = e.vz = 0;
        if (e.pat == 0) {
            if (e.t > 0.6f && !e.fired) {
                e.fired = true;
                addProj(PR_SLEEP, e.x + side * 20, P.z, 25, side * 190.f, 0, 3.2f, 0);
                if (bossEnraged) addProj(PR_SLEEP, e.x + side * 20, clampf(P.z + (P.z > 50 ? -28.f : 28.f), 4, 96), 25, side * 170.f, 0, 3.4f, 0);
                play(sfxSleep, 0.8f);
            }
            return e.t > 0.95f;
        }
        if (e.pat == 1) {
            if (e.t > 0.4f && !e.fired) { e.fired = true; rainAround(bossEnraged ? 8 : 6, 1, 12, 120); play(sfxSlam, 0.4f); }
            return e.t > 1.0f;
        }
        if (e.t > 0.5f && !e.fired) { e.fired = true; for (int i = 0; i < 2; i++) spawnEnemy(ET_GRUNT, rcamX + (i ? 360.f : -360.f), frange(10, 90)); }
        return e.t > 0.9f;
    case ET_DAKI:
        if (e.pat == 0) {
            e.vx = e.vz = 0;
            if (e.t > 0.5f && !e.fired) {
                e.fired = true;
                int n = (bossEnraged || e.assist) ? 5 : 3;
                for (int k = 0; k < n; k++) addProj(PR_OBI, e.x + side * 15, e.z + (e.assist ? 0 : 0), 30, side * 360.f, (k - (n - 1) * 0.5f) * 30.f + (e.assist ? (P.z - e.z) * 1.2f : 0), 2.4f, 9);
                play(sfxWhip, 0.7f);
            }
            return e.t > 0.85f;
        }
        if (e.pat == 1) {
            e.vx = e.vz = 0;
            if (e.t > 0.3f && !e.fired) { e.fired = true; addSweep(P.z, 12, 0, 1.0f); if (bossEnraged && !e.assist) addSweep(clampf(P.z + 30, 5, 95), 12, 0, 1.25f); }
            return e.t > 0.7f;
        }
        // investida com chute
        if (e.t < 0.35f) { e.vx = e.vz = 0; e.face = P.x > e.x ? 1 : -1; e.hitDone = false; }
        else if (e.t < 0.75f) { e.vx = side * 430; e.vz = clampf((P.z - e.z) * 3, -80, 80); bossContact(e, 40, 14, 10, false); }
        else e.vx *= 0.8f;
        return e.t > 1.0f;
    case ET_GYUTARO:
        if (e.pat == 0) {
            e.vx = e.vz = 0;
            if (e.t > 0.45f && !e.fired) {
                e.fired = true;
                for (int k = -1; k <= 1; k += 2) addProj(PR_SICKLE, e.x + side * 15, e.z, 30, side * 330.f, k * (bossEnraged ? 40.f : 25.f), 4.f, 10);
                if (bossEnraged) addProj(PR_SICKLE, e.x + side * 15, e.z, 30, side * 360.f, 0, 4.f, 10);
                play(sfxWhip, 0.6f);
            }
            return e.t > 0.75f;
        }
        if (e.pat == 1) {
            e.vx = e.vz = 0;
            if (!e.fired) { e.fired = true; Proj& p = addProj(PR_RING, e.x, e.z, 0, 0, 0, 0.9f, 14); p.r = bossEnraged ? 100.f : 80.f; }
            return e.t > 1.0f;
        }
        if (e.t < 0.45f) {
            e.vx = e.vz = 0;
            if (!e.fired) { e.fired = true; e.face = P.x > e.x ? 1 : -1; e.z = clampf(e.z + (P.z - e.z) * 0.7f, 2, 98); addLine(e.x, e.x + e.face * 300, e.z, 0, 1, 0.45f); e.hitDone = false; }
        } else if (e.t < 0.8f) {
            e.vx = side * 650; bossContact(e, 40, 14, 12, false);
            if (rnd() & 1) addPart(e.x, e.z, 30, 0, 0, 0, 0.25f, RGBc(220, 30, 50), 2.5f, P_WATER);
        } else e.vx *= 0.7f;
        return e.t > 1.05f;
    case ET_AKAZA:
        if (e.pat == 0) { // investida + três socos
            if (e.cnt == 0) {
                e.face = P.x > e.x ? 1 : -1;
                float tx = P.x - e.face * 40;
                e.vx = clampf((tx - e.x) * 6, -320, 320); e.vz = clampf((P.z - e.z) * 6, -160, 160);
                if ((fabsf(P.x - e.x) < 54 && fabsf(P.z - e.z) < 9) || e.t > 1.0f) { e.cnt = 1; e.aux = 0; }
                return false;
            }
            e.vx = e.vz = 0; e.aux += dt;
            const float tm[3] = { 0.1f, 0.28f, 0.48f };
            if (e.cnt <= 3 && e.aux >= tm[e.cnt - 1]) {
                bool heavy = e.cnt == 3;
                e.hitDone = false; bossContact(e, 64, 15, heavy ? 13.f : 7.f, heavy);
                e.punchT = 0.12f;
                addSlash(e.x + e.face * 30, e.z, 36, e.face, 0.15f, 10, 5, 0, 2 * PI, 1, 0, SL_FIST);
                play(sfxPunch, 0.7f); shake(heavy ? 4.f : 2.f, 0.1f);
                e.cnt++;
            }
            return e.cnt > 3 && e.aux > 0.85f;
        }
        if (e.pat == 1) { // 空式: ondas de choque
            e.vx = e.vz = 0;
            const float tm[3] = { 0.35f, 0.5f, 0.65f };
            if (e.cnt < 3 && e.t > tm[e.cnt]) {
                float off = e.cnt == 0 ? 0.f : (e.cnt == 1 ? -14.f : 14.f);
                addProj(PR_AIR, e.x + side * 20, clampf(P.z + off, 3, 97), 32, side * 440.f, 0, 1.6f, 9);
                e.punchT = 0.12f; play(sfxPunch, 0.6f); e.cnt++;
            }
            return e.t > 1.0f;
        }
        if (e.pat == 2) { // 乱式: rajada em área
            e.vx = e.vz = 0;
            if (e.t > 0.55f && e.t < 1.15f) {
                e.punchT = 0.05f;
                float a = frange(0, 2 * PI), r = frange(20, 75);
                addPart(e.x + cosf(a) * r, e.z + sinf(a) * r * 0.4f, frange(10, 60), 0, 0, 0, 0.18f, RGBc(150, 200, 255), 8, P_RING);
                if (rnd() % 3 == 0) play(sfxPunch, 0.35f);
                float dx = P.x - e.x, dz = P.z - e.z;
                if (dx * dx + dz * dz * 4 < 80 * 80 && P.y < 50) hurtPlayer(11, e.x);
                shake(2, 0.05f);
            }
            return e.t > 1.5f;
        }
        // 滅式: aniquilação
        if (e.t < 0.7f) {
            e.vx = e.vz = 0;
            if (!e.fired) { e.fired = true; e.face = P.x > e.x ? 1 : -1; e.z = clampf(e.z + (P.z - e.z) * 0.8f, 2, 98); addLine(e.x, e.x + e.face * 320, e.z, 0, 2, 0.7f); e.hitDone = false; }
        } else if (e.t < 1.05f) {
            e.vx = side * 720; e.punchT = 0.1f; bossContact(e, 45, 16, 20, true);
            addPart(e.x, e.z, 34, 0, 0, 0, 0.25f, RGBc(130, 190, 255), 3, P_WATER);
        } else e.vx *= 0.7f;
        return e.t > 1.3f;
    case ET_MUZAN:
        e.vx = e.vz = 0;
        if (e.pat == 0) {
            if (e.t > 0.2f && !e.fired) {
                e.fired = true; addSweep(P.z, 14, 1, 0.9f);
                if (bossEnraged) addSweep(clampf(P.z + (P.z > 50 ? -30.f : 30.f), 5, 95), 14, 1, 1.15f);
            }
            return e.t > 0.6f;
        }
        if (e.pat == 1) {
            if (e.t > 0.3f && !e.fired) { e.fired = true; rainAround(bossEnraged ? 10 : 8, 2, 12, 130); }
            return e.t > 0.8f;
        }
        if (!e.fired) { e.fired = true; e.face = P.x > e.x ? 1 : -1; addLine(e.x, e.x + e.face * 330, P.z, 16, 1, 0.75f); }
        return e.t > 0.95f;
    default: return true;
    }
}

void updateBoss(Ent& e, float dt) {
    if (e.st == S_B_INTRO) {
        e.face = P.x > e.x ? 1 : -1;
        if (e.floats) e.y = std::max(14.f, e.y - 180 * dt);
        if ((e.floats ? e.y <= 14.f : e.y <= 0) && e.t > 1.4f) { e.st = S_IDLE; e.t = 0; }
        return;
    }
    if (e.floats) e.y = 14 + sinf(e.anim * 2.2f) * 5;
    e.aiT -= dt; e.cd2 -= dt;
    if (e.punchT > 0) e.punchT -= dt;
    // regeneração (bloqueada pela Lâmina Escarlate)
    if (e.regen > 0 && e.hp < e.maxhp) {
        if (akatoT <= 0) {
            e.hp = std::min(e.maxhp, e.hp + e.regen * dt);
            if (rnd() % 6 == 0) addPart(e.x + frange(-10, 10), e.z, frange(10, 60) * e.scale, 0, 0, 30, 0.5f, RGBc(200, 60, 80), 1.4f, P_HEAL);
            if (!regenHint && e.hp < e.maxhp * 0.9f) { regenHint = true; banner(L"再生", L"O demônio se regenera! Use a Lâmina Escarlate (Y)"); }
        }
    }
    if (!e.assist && !bossEnraged && e.hp < e.maxhp * 0.5f) enrage(e);
    if (e.st == S_B_CAST) {
        if (runPattern(e, dt)) { e.st = S_IDLE; e.t = 0; e.vx = e.vz = 0; }
        return;
    }
    if (e.assist) { // Daki ajudando o irmão: fica no fundo e ataca de vez em quando
        e.face = P.x > e.x ? 1 : -1;
        e.vx = clampf((P.x + 150 * (e.x > P.x ? 1 : -1) - e.x) * 1.5f, -70, 70); e.vz = clampf((8 - e.z) * 2, -40, 40);
        e.st = fabsf(e.vx) > 12 ? S_WALK : S_IDLE;
    } else {
        switch (e.type) {
        case ET_RUI: bossMove(e, 210, 110); if (fabsf(P.x - e.x) < 85 && e.cd2 <= 0) bossTeleport(e); break;
        case ET_ENMU: bossMove(e, 220, 90); if (fabsf(P.x - e.x) < 85 && e.cd2 <= 0) bossTeleport(e); break;
        case ET_DAKI: bossMove(e, 140, 130); break;
        case ET_GYUTARO: bossMove(e, 110, 150); break;
        case ET_AKAZA: bossMove(e, 70, 170); break;
        case ET_MUZAN: bossMove(e, 170, 80); break;
        default: break;
        }
    }
    if (e.aiT <= 0) {
        e.pat = choosePattern(e);
        e.st = S_B_CAST; e.t = 0; e.fired = false; e.cnt = 0; e.aux = 0; e.hitDone = false;
        e.vx = e.vz = 0;
        float cyc = 2.4f;
        switch (e.type) {
        case ET_ENMU: cyc = 2.3f; break;
        case ET_DAKI: cyc = e.assist ? 5.0f : 2.1f; break;
        case ET_GYUTARO: cyc = 1.9f; break;
        case ET_AKAZA: cyc = 1.7f; break;
        case ET_MUZAN: cyc = 1.9f; break;
        default: break;
        }
        e.aiT = cyc * (bossEnraged ? 0.75f : 1.f) + frange(0, 0.5f);
    }
}

void updateEnemy(Ent& e, float dt) {
    e.t += dt; e.anim += dt;
    if (e.flash > 0) e.flash -= dt;
    if (e.cd > 0) e.cd -= dt;
    float dx = P.x - e.x, dz = P.z - e.z, adx = fabsf(dx);
    bool pAlive = P.st != S_DEAD;
    bool boss = isBoss(e.type);

    switch (e.st) {
    case S_DEAD:
        if (e.y <= 0 || boss) {
            e.vx *= 0.9f; e.vz = 0;
            e.dieT += dt * (boss ? 0.45f : 1.f);
            if (rnd() % 2 == 0)
                addPart(e.x + frange(-14, 14) * e.scale, e.z, frange(0, 40) * e.scale, frange(-20, 20), 0, frange(30, 80), frange(0.6f, 1.4f),
                        (rnd() & 3) ? RGBc(50, 45, 50) : RGBc(255, 120, 40), frange(1.f, 2.2f), P_ASH, -15);
            if (e.dieT > 1.2f) { e.alive = false; killFx(e); }
        }
        break;
    case S_HURT:
        e.vx *= 0.85f; e.vz = 0;
        if (e.t > 0.35f) { e.st = S_IDLE; e.t = 0; }
        break;
    case S_KNOCK: break;
    case S_DOWN:
        e.vx *= 0.85f; e.vz = 0;
        if (e.t > 0.9f) { e.st = S_IDLE; e.t = 0; e.cd = 0.5f; }
        break;
    default:
        if (boss) { updateBoss(e, dt); break; }
        switch (e.st) {
        case S_E_WIND:
            e.vx = e.vz = 0;
            if (e.t > windDur(e)) {
                e.t = 0; e.hitDone = false;
                if (e.type == ET_FAST) { e.st = S_E_LUNGE; e.vx = e.face * 420.f; play(sfxDash, 0.5f); }
                else {
                    e.st = S_E_STRIKE; play(e.type == ET_BIG ? sfxSlam : sfxSwing, 0.6f);
                    if (e.type == ET_BIG) {
                        shake(6, 0.25f); addPart(e.x + e.face * 55, e.z, 0, 0, 0, 0, 0.4f, RGBc(170, 150, 120), 40, P_RING);
                        for (int i = 0; i < 14; i++) addPart(e.x + e.face * 55, e.z, 2, frange(-150, 150), frange(-40, 40), frange(60, 200), 0.6f, RGBc(110, 100, 90), 2, P_DOT, 700);
                    } else addSlash(e.x + e.face * 18, e.z, 34, e.face, 0.18f, 22, 6, -1.8f, 1.0f, 0.7f, 0.2f, SL_CLAW);
                }
            }
            break;
        case S_E_STRIKE:
            if (!e.hitDone) {
                e.hitDone = true;
                float f = (P.x - e.x) * e.face;
                if (e.type == ET_BIG) { if (f > -20 && f < 100 && fabsf(dz) < 28 && P.y < 40) hurtPlayer(18, e.x, true); }
                else if (f > -5 && f < 56 * e.scale && fabsf(dz) < 14 && P.y < 35) hurtPlayer(8, e.x);
            }
            if (e.t > 0.15f) { e.st = S_E_RECOVER; e.t = 0; }
            break;
        case S_E_LUNGE:
            if (rnd() & 1) addPart(e.x, e.z, frange(10, 40), 0, 0, 0, 0.2f, RGBc(170, 80, 200), 2, P_WATER);
            if (!e.hitDone && fabsf(P.x - e.x) < 26 && fabsf(dz) < 13 && P.y < 30) { e.hitDone = true; hurtPlayer(10, e.x); }
            if (e.t > 0.33f) { e.vx = 0; e.st = S_E_RECOVER; e.t = 0; }
            break;
        case S_E_RECOVER:
            e.vx *= 0.8f;
            if (e.t > (e.type == ET_BIG ? 0.8f : 0.55f)) { e.st = S_IDLE; e.t = 0; e.token = false; e.cd = frange(0.7f, 1.7f); }
            break;
        case S_IDLE: case S_WALK: {
            e.face = dx > 0 ? 1 : -1;
            e.aiT -= dt;
            if (e.aiT <= 0) { e.aiT = frange(1.5f, 3.f); e.strafe = frange(-38, 38); }
            if (!pAlive) { e.vx = -e.face * 30.f; e.vz = 0; e.st = S_WALK; break; }
            int maxTok = bossEnraged ? 3 : 2;
            if (!e.token && e.cd <= 0 && tokensUsed() < maxTok) e.token = true;
            float range = e.type == ET_BIG ? 62.f : (e.type == ET_FAST ? 130.f : 42.f);
            float side = dx > 0 ? -1.f : 1.f;
            float tx, tz;
            if (e.token) { tx = P.x + side * range * 0.9f; tz = P.z; }
            else { tx = P.x + side * (135 + e.idx * 14.f); tz = clampf(P.z + e.strafe, 4, 96); }
            float spd = e.type == ET_FAST ? 145.f : (e.type == ET_BIG ? 62.f : 88.f);
            float ex = tx - e.x, ez = tz - e.z, el = sqrtf(ex * ex + ez * ez);
            if (el > 6) { e.vx = ex / el * spd; e.vz = ez / el * spd * 0.75f; e.st = S_WALK; }
            else { e.vx = e.vz = 0; e.st = S_IDLE; }
            bool inRange = e.type == ET_FAST ? (adx < range + 25 && adx > 50 && fabsf(dz) < 9) : (fabsf(adx - range * 0.9f) < 14 && fabsf(dz) < 9);
            if (e.token && inRange && e.cd <= 0) { e.st = S_E_WIND; e.t = 0; e.vx = e.vz = 0; }
        } break;
        default: break;
        }
        break;
    }

    // separação entre inimigos
    if (!e.assist)
        for (auto& o : E) {
            if (&o == &e || !o.alive || o.st == S_DEAD || o.assist) continue;
            float ox = e.x - o.x, oz = (e.z - o.z) * 1.6f, d = sqrtf(ox * ox + oz * oz);
            float minD = 22 * (e.scale + o.scale) * 0.5f;
            if (d < minD && d > 0.01f) { e.x += ox / d * (minD - d) * 0.3f; e.z += oz / d * (minD - d) * 0.15f; }
        }
    enemyPhysics(e, dt);
    e.z = clampf(e.z, 2, DEPTH - 2);
}

void strikeFx(float x, float z, uint32_t c) {
    for (int i = 0; i < 10; i++) addPart(x, z, 2, frange(-90, 90), frange(-20, 20), frange(40, 160), 0.35f, c, 1.2f, P_SPARK);
    addPart(x, z, 0, 0, 0, 0, 0.3f, c, 24, P_RING);
}

void updateProjs(float dt) {
    for (size_t i = 0; i < projs.size(); i++) {
        Proj& pr = projs[i];
        if (!pr.alive) continue;
        pr.t += dt; pr.life -= dt;
        bool canHit = P.st != S_DEAD && P.inv <= 0;
        switch (pr.kind) {
        case PR_THREAD: case PR_OBI: case PR_AIR: case PR_SLEEP: {
            pr.x += pr.vx * dt; pr.z += pr.vz * dt;
            if (pr.z < 0 || pr.z > DEPTH) pr.alive = false;
            float hx = pr.kind == PR_SLEEP ? 22.f : 22.f, hz = pr.kind == PR_SLEEP ? 20.f : (pr.kind == PR_AIR ? 10.f : 8.f);
            if (canHit && fabsf(P.x - pr.x) < hx && fabsf(P.z - pr.z) < hz && P.y < 45) {
                if (pr.kind == PR_SLEEP) { sleepPlayer(); pr.alive = false; }
                else if (!(P.st == S_DASH || isSkill(P.st))) { hurtPlayer(pr.dmg, pr.x - pr.vx, pr.kind == PR_AIR && pr.dmg > 10); pr.alive = false; }
            }
            if (pr.kind == PR_AIR && (rnd() & 1)) addPart(pr.x, pr.z, pr.y, 0, 0, 0, 0.2f, RGBc(150, 200, 255), 2, P_WATER);
        } break;
        case PR_SICKLE: {
            Ent* g = findType(ET_GYUTARO);
            if (!pr.ret) {
                pr.x += pr.vx * dt; pr.z += pr.vz * dt;
                pr.z = clampf(pr.z, 2, 98);
                if (pr.t > 0.8f) { pr.ret = true; pr.hit = false; }
            } else if (g) {
                float dx = g->x - pr.x, dz = g->z - pr.z, d = sqrtf(dx * dx + dz * dz);
                if (d < 18) pr.alive = false;
                else { pr.x += dx / d * 400 * dt; pr.z += dz / d * 400 * dt; }
            } else pr.alive = false;
            if (!pr.hit && canHit && fabsf(P.x - pr.x) < 20 && fabsf(P.z - pr.z) < 10 && P.y < 45) { pr.hit = true; hurtPlayer(pr.dmg, pr.x - pr.vx); }
        } break;
        default: // ataques com aviso no chão: golpeiam quando o tempo acaba
            if (!pr.hit && pr.life <= 0.18f) {
                pr.hit = true;
                switch (pr.kind) {
                case PR_RAIN: {
                    uint32_t c = pr.style == 0 ? RGBc(255, 70, 90) : (pr.style == 1 ? RGBc(255, 140, 170) : RGBc(200, 20, 40));
                    strikeFx(pr.x, pr.z, c);
                    if (canHit && fabsf(P.x - pr.x) < 24 && fabsf(P.z - pr.z) < 12 && P.y < 60) hurtPlayer(pr.dmg, pr.x);
                    shake(2, 0.1f);
                } break;
                case PR_SWEEP: case PR_LINE:
                    if (pr.dmg > 0) {
                        play(sfxWhip, 0.8f); shake(4, 0.15f);
                        float hz = pr.kind == PR_SWEEP ? 12.f : 10.f;
                        if (canHit && fabsf(P.z - pr.z) < hz && P.x > pr.x - 10 && P.x < pr.x2 + 10 && P.y < 50) hurtPlayer(pr.dmg, P.x - 1, pr.dmg >= 15);
                    }
                    break;
                case PR_RING: {
                    play(sfxWhip, 0.8f); shake(4, 0.15f);
                    float dx = (P.x - pr.x) / pr.r, dz = (P.z - pr.z) / (pr.r * 0.45f);
                    if (canHit && dx * dx + dz * dz < 1 && P.y < 50) hurtPlayer(pr.dmg, pr.x);
                    for (int k = 0; k < 16; k++) { float a = k / 16.f * 2 * PI; addPart(pr.x + cosf(a) * pr.r, pr.z + sinf(a) * pr.r * 0.45f, 20, cosf(a) * 80, 0, 40, 0.4f, RGBc(230, 30, 50), 1.4f, P_SPARK); }
                } break;
                }
            }
            break;
        }
        if (pr.life <= 0 && pr.kind != PR_SICKLE) pr.alive = false;
        if (pr.kind == PR_SICKLE && pr.t > 5) pr.alive = false;
    }
    projs.erase(std::remove_if(projs.begin(), projs.end(), [](const Proj& p) { return !p.alive; }), projs.end());
}

