// Tanjiro: movimento, combos, técnicas de respiração, Lâmina Escarlate e Nezuko.
#include "common.h"

// ============================================================ jogo: jogador
void startAttack(St s) {
    P.st = s; P.t = 0; P.queued = false; P.fired = false; P.tick = 0; P.cnt = 0; attackId++;
    play(sfxSwing, 0.6f);
}

void callNezuko() {
    nzActive = true;
    NZ = Ent();
    NZ.type = ET_NEZUKO; NZ.face = P.face; NZ.x = P.x - P.face * 70; NZ.z = std::min(DEPTH - 2, P.z + 8); NZ.t = 0;
    NZ.lastNz = ++attackId;
    nezCd = 14;
    banner(L"禰豆子！", L"Nezuko entra na luta!");
    play(sfxNezuko);
    for (int i = 0; i < 20; i++)
        addPart(NZ.x, NZ.z, frange(0, 50), frange(-50, 50), 0, frange(0, 60), frange(0.4f, 0.9f), RGBc(255, 130, 170), 1.6f, P_FIRE);
}

void updateNezuko(float dt) {
    if (!nzActive) return;
    NZ.t += dt; NZ.anim += dt;
    if (NZ.t > 0.2f && NZ.t < 0.85f) {
        NZ.x += NZ.face * 430 * dt;
        for (auto& e : E) {
            if (!e.alive || e.assist || e.st == S_DEAD || e.lastNz == NZ.lastNz || e.st == S_B_INTRO) continue;
            if (fabsf(e.x - NZ.x) < 34 * e.scale && fabsf(e.z - NZ.z) < 24) {
                e.lastNz = NZ.lastNz;
                damageEnemy(e, 32, NZ.face * 300.f, true, 2);
            }
        }
        if (rnd() & 1) addPart(NZ.x, NZ.z, frange(5, 40), -NZ.face * 60.f, 0, 20, 0.4f, RGBc(255, 110, 150), 1.5f, P_FIRE);
    }
    if (NZ.t > 1.25f) nzActive = false;
}

void sleepPlayer() {
    if (P.st == S_DEAD || P.st == S_SLEEP || P.inv > 0 || isSkill(P.st) || P.st == S_DASH || P.st == S_DOWN || P.st == S_KNOCK) return;
    P.st = S_SLEEP; P.t = 0; P.vx = P.vz = 0;
    play(sfxSleep);
}

void playerPhysics(float dt) {
    Ent& p = P;
    p.x += p.vx * dt;
    p.z += p.vz * dt;
    if (p.y > 0 || p.vy > 0) {
        p.vy -= 1300 * dt;
        p.y += p.vy * dt;
        if (p.y <= 0) {
            p.y = 0; p.vy = 0;
            if (p.st == S_JUMP || p.st == S_AIRATK) { p.st = S_IDLE; p.t = 0; addPart(p.x, p.z, 0, 0, 0, 0, 0.25f, RGBc(150, 140, 170), 10, P_RING); }
            else if (p.st == S_KNOCK) { p.st = S_DOWN; p.t = 0; p.vx = 0; shake(3, 0.15f); }
            else if (p.st == S_DEAD) { p.vx = 0; }
        }
    }
    p.z = clampf(p.z, 2, DEPTH - 2);
}

// técnicas de respiração (também cancelam o combo normal) e chamada da Nezuko
bool trySpecial(float mx, float mz) {
    if (kp('N') && nezCd <= 0 && !nzActive) callNezuko();
    auto need = [](float c) { if (breath < c) { banner(L"...", L"Respiração insuficiente!"); return false; } breath -= c; return true; };
    if (kp('Y') && chapter >= 3) {
        if (akatoT > 0) return false;
        if (!need(30)) return false;
        akatoT = 10; play(sfxAkato); flashScreen = 0.5f; flashColor = RGBc(255, 40, 30);
        banner(L"赫刀", L"Lâmina Escarlate! A regeneração dos demônios foi bloqueada");
        return false;
    }
    if (kp('U')) {
        if (!need(30)) return false;
        startAttack(S_WATER1); play(sfxWater);
        banner(L"水の呼吸　壱ノ型　水面斬り", L"Respiração da Água · Primeira Forma: Corte da Superfície da Água");
        return true;
    }
    if (kp('I')) {
        if (!need(50)) return false;
        startAttack(S_WATER10); play(sfxWater); dragon.clear(); dragonFade = 1; dragonFire = false;
        banner(L"水の呼吸　拾ノ型　生生流転", L"Respiração da Água · Décima Forma: Mudança Constante");
        return true;
    }
    if (kp('H') && chapter >= 4) {
        if (breath < 100) { banner(L"...", L"A Décima Terceira Forma exige a respiração no máximo!"); return false; }
        breath = 0; startAttack(S_HINO_13); play(sfxFire); play(sfxAkato);
        banner(L"日の呼吸　拾参ノ型", L"Respiração do Sol · Décima Terceira Forma: as doze danças em ciclo infinito");
        flashScreen = 0.8f; flashColor = RGBc(255, 160, 50);
        return true;
    }
    if (kp('O')) {
        if (mz < 0 && chapter >= 1) {
            if (!need(50)) return false;
            startAttack(S_HINO_SKY); play(sfxFire); P.vy = 330; P.y = std::max(P.y, 1.f);
            banner(L"ヒノカミ神楽　碧羅の天", L"Hinokami Kagura · Céu Azul Claro");
        } else if (mz > 0 && chapter >= 3) {
            if (!need(70)) return false;
            startAttack(S_HINO_DRAGON); play(sfxFire); dragon.clear(); dragonFade = 1; dragonFire = true;
            banner(L"ヒノカミ神楽　日暈の龍　頭舞い", L"Hinokami Kagura · Dragão do Halo Solar");
        } else if (mx != 0 && chapter >= 2) {
            if (!need(60)) return false;
            P.face = mx > 0 ? 1 : -1;
            startAttack(S_HINO_SUN); play(sfxFire);
            banner(L"ヒノカミ神楽　烈日紅鏡", L"Hinokami Kagura · Espelho Solar Escarlate");
        } else {
            if (!need(60)) return false;
            startAttack(S_HINO); play(sfxFire);
            banner(L"ヒノカミ神楽　円舞", L"Hinokami Kagura · Dança Circular");
        }
        flashScreen = 0.4f; flashColor = RGBc(255, 140, 40);
        return true;
    }
    return false;
}

void fireBurst(float n) {
    for (int i = 0; i < (int)n; i++) {
        float a = frange(0, 2 * PI), r = frange(20, 70);
        addPart(P.x + cosf(a) * r, P.z + sinf(a) * r * 0.35f, P.y + frange(0, 40), 0, 0, frange(40, 140), frange(0.3f, 0.7f), 0, frange(1.2f, 2.4f), P_FIRE);
    }
}

void updatePlayer(float dt) {
    Ent& p = P;
    p.t += dt; p.anim += dt;
    if (p.inv > 0) p.inv -= dt;
    dashCd -= dt; nezCd -= dt;
    if (akatoT > 0) akatoT -= dt;
    if (p.st != S_DEAD) breath = std::min(100.f, breath + dt * 4.5f);

    float mx = (float)((kd(VK_RIGHT) || kd('D')) - (kd(VK_LEFT) || kd('A')));
    float mz = (float)((kd(VK_DOWN) || kd('S')) - (kd(VK_UP) || kd('W')));
    bool atk = kp('J'), jump = kp('K') || kp(VK_SPACE), dash = kp('L') || kp(VK_SHIFT);

    switch (p.st) {
    case S_IDLE: case S_WALK: {
        float len = sqrtf(mx * mx + mz * mz), nx = mx, nz = mz;
        if (len > 1) { nx /= len; nz /= len; }
        p.vx = nx * 165; p.vz = nz * 115;
        if (mx != 0) p.face = mx > 0 ? 1 : -1;
        p.st = len > 0 ? S_WALK : S_IDLE;
        if (atk) startAttack(S_ATK1);
        else if (jump) { p.st = S_JUMP; p.t = 0; p.vy = 440; p.airUsed = false; play(sfxJump, 0.6f); }
        else if (dash && dashCd <= 0) {
            p.st = S_DASH; p.t = 0; dashCd = 0.45f;
            float dx = mx != 0 ? mx : (float)p.face, dz = mz;
            float l = sqrtf(dx * dx + dz * dz); p.vx = dx / l * 430; p.vz = dz / l * 300;
            play(sfxDash, 0.7f);
        } else trySpecial(mx, mz);
    } break;
    case S_JUMP:
        p.vx = mx * 175; p.vz = mz * 110;
        if (mx != 0) p.face = mx > 0 ? 1 : -1;
        if (atk && !p.airUsed) { p.airUsed = true; startAttack(S_AIRATK); }
        break;
    case S_AIRATK:
        p.vx = mx * 120;
        if (p.t > 0.06f && p.t < 0.2f) playerHit(-10, 62, 18, 45, 14, 90, false, 0);
        if (p.t > 0.08f && !p.fired) { p.fired = true; addSlash(p.x + p.face * 8, p.z, p.y + 34, p.face, 0.22f, 30, 8, -2.0f, 1.7f, 1.0f, 0, SL_BLADE); }
        break;
    case S_ATK1: case S_ATK2: case S_ATK3: {
        float dur = p.st == S_ATK1 ? 0.3f : (p.st == S_ATK2 ? 0.32f : 0.46f);
        p.vz = 0;
        p.vx = (p.t < 0.12f) ? p.face * (p.st == S_ATK3 ? 170.f : 70.f) : p.vx * 0.8f;
        if (atk && p.t > 0.05f) p.queued = true;
        float h0 = dur * 0.25f, h1 = dur * 0.55f;
        if (p.t > h0 && !p.fired) {
            p.fired = true;
            if (p.st == S_ATK1) addSlash(p.x + p.face * 10, p.z, 36, p.face, 0.2f, 30, 9, -2.2f, 0.8f, 0.55f, 0.2f, SL_BLADE);
            if (p.st == S_ATK2) addSlash(p.x + p.face * 10, p.z, 34, p.face, 0.2f, 30, 9, 1.2f, -1.7f, 0.7f, -0.3f, SL_BLADE);
            if (p.st == S_ATK3) addSlash(p.x + p.face * 14, p.z, 34, p.face, 0.28f, 38, 13, -2.7f, 1.2f, 0.9f, 0.1f, chapter >= 3 ? SL_FIRE : SL_WATER);
        }
        if (p.t > h0 && p.t < h1) {
            if (p.st == S_ATK1) playerHit(-5, 62, 16, 40, 10, 70, false, 0);
            if (p.st == S_ATK2) playerHit(-5, 62, 16, 40, 12, 80, false, 0);
            if (p.st == S_ATK3) playerHit(-5, 72, 20, 40, 22, 240, true, chapter >= 3 ? 2 : 1);
        }
        if (trySpecial(mx, mz)) break;
        if (p.t > h1 && dash && dashCd <= 0) {
            p.st = S_DASH; p.t = 0; dashCd = 0.45f;
            float dx = mx != 0 ? mx : (float)p.face;
            float l = sqrtf(dx * dx + mz * mz); p.vx = dx / l * 430; p.vz = mz / l * 300;
            play(sfxDash, 0.7f);
            break;
        }
        if (p.queued && p.t > dur * 0.68f && p.st != S_ATK3) startAttack(p.st == S_ATK1 ? S_ATK2 : S_ATK3);
        else if (p.t > dur) { p.st = S_IDLE; p.t = 0; }
    } break;
    case S_DASH:
        if (rnd() % 2 == 0) addPart(p.x, p.z, frange(5, 50), 0, 0, 0, 0.25f, chapter >= 3 ? RGBc(255, 120, 50) : RGBc(80, 160, 255), 2.5f, P_WATER);
        if (p.t > 0.22f) { p.st = S_IDLE; p.t = 0; p.vx *= 0.3f; p.vz *= 0.3f; }
        break;
    case S_WATER1:
        p.vz = 0;
        p.vx = (p.t > 0.08f && p.t < 0.3f) ? p.face * 160.f : p.vx * 0.85f;
        if (p.t > 0.14f && !p.fired) {
            p.fired = true;
            addSlash(p.x + p.face * 26, p.z, 30, p.face, 0.5f, 70, 18, 2.9f, 7.0f, 0.32f, 0.05f, SL_WATER);
            addSlash(p.x + p.face * 26, p.z, 30, p.face, 0.5f, 54, 10, 3.2f, 6.8f, 0.30f, 0.1f, SL_WATER);
            shake(4, 0.2f);
        }
        if (p.t > 0.14f && p.t < 0.32f) playerHit(-30, 130, 34, 55, 38, 270, true, 1);
        if (p.t > 0.62f) { p.st = S_IDLE; p.t = 0; }
        break;
    case S_WATER10: case S_HINO_DRAGON: {
        bool fire = p.st == S_HINO_DRAGON;
        p.vz = mz * 60;
        p.vx = p.t < 0.8f ? p.face * (fire ? 380.f : 340.f) : p.vx * 0.85f;
        dragon.push_back({ p.x + p.face * 12, p.z, 34 + sinf(p.t * 14) * 14 });
        if (dragon.size() > 46) dragon.erase(dragon.begin());
        p.tick -= dt;
        if (p.t < 0.8f && p.tick <= 0) { p.tick = fire ? 0.08f : 0.09f; attackId++; playerHit(-45, 60, 30, 55, fire ? 11.f : 8.f, 340, false, fire ? 2 : 1); }
        if (p.t > 0.84f && !p.fired) {
            p.fired = true; attackId++;
            playerHit(-60, 90, 36, 60, fire ? 40.f : 26.f, 320, true, fire ? 2 : 1);
            addSlash(p.x + p.face * 20, p.z, 34, p.face, 0.35f, 46, 16, -2.5f, 1.6f, 1.0f, 0, fire ? SL_FIRE : SL_WATER);
            shake(5, 0.25f);
        }
        for (int i = 0; i < 2; i++) {
            if (fire) addPart(p.x, p.z + frange(-4, 4), frange(10, 60), -p.face * frange(20, 120), 0, frange(20, 90), 0.5f, 0, 2.2f, P_FIRE);
            else addPart(p.x, p.z + frange(-4, 4), frange(10, 60), -p.face * frange(20, 120), 0, frange(-20, 60), 0.5f, RGBc(70, 150, 255), 2.4f, P_WATER, 200);
        }
        if (p.t > 1.0f) { p.st = S_IDLE; p.t = 0; }
    } break;
    case S_HINO: {
        p.vz = mz * 50;
        p.vx = p.face * 70.f * (p.t < 1.0f);
        p.tick -= dt;
        if (p.t < 1.0f && p.tick <= 0) {
            p.tick = 0.11f; attackId++;
            playerHit(0, 100, 38, 60, 8, 110, false, 2, true);
            float a0 = frange(0, 2 * PI);
            addSlash(p.x, p.z, 30, (rnd() & 1) ? 1 : -1, 0.3f, 58, 14, a0, a0 + 3.8f, 0.38f, frange(-0.3f, 0.3f), SL_FIRE);
        }
        fireBurst(4);
        if (p.t > 1.08f && !p.fired) {
            p.fired = true; attackId++;
            playerHit(0, 125, 44, 90, 48, 330, true, 2, true);
            addSlash(p.x + p.face * 10, p.z, 40, p.face, 0.5f, 85, 26, -2.4f, 1.9f, 1.0f, 0, SL_FIRE);
            addSlash(p.x, p.z, 25, p.face, 0.5f, 95, 18, 0, 2 * PI, 0.32f, 0, SL_FIRE);
            flashScreen = 0.7f; flashColor = RGBc(255, 150, 50); shake(9, 0.4f); play(sfxSlam);
        }
        if (p.t > 1.45f) { p.st = S_IDLE; p.t = 0; }
    } break;
    case S_HINO_SKY: {
        // Céu Azul Claro: giro vertical subindo, bom contra grupos e no ar
        p.vx = mx * 90; p.vz = mz * 60;
        float marks[3] = { 0.06f, 0.26f, 0.46f };
        if (p.cnt < 3 && p.t > marks[p.cnt]) {
            attackId++;
            bool last = p.cnt == 2;
            playerHit(0, 88, 34, 90, last ? 26.f : 14.f, last ? 260.f : 120.f, last, 2, true);
            addSlash(p.x, p.z, p.y + 34, p.face, 0.35f, 44 + p.cnt * 6, 12, -PI / 2, -PI / 2 + 2 * PI, 1.0f, 0, SL_FIRE);
            if (last) addSlash(p.x, p.z, p.y + 34, -p.face, 0.4f, 60, 10, -PI / 2, -PI / 2 + 2 * PI, 1.0f, 0.3f, SL_FIRE);
            shake(3, 0.15f);
            p.cnt++;
        }
        fireBurst(3);
        if (p.t > 0.8f && p.y <= 0) { p.st = S_IDLE; p.t = 0; }
    } break;
    case S_HINO_SUN: {
        // Espelho Solar Escarlate: corte largo para frente e depois para trás
        p.vz = 0; p.vx = p.face * 40.f;
        if (p.cnt == 0 && p.t > 0.12f) {
            p.cnt = 1; attackId++;
            playerHit(-20, 135, 36, 60, 32, 280, true, 2);
            addSlash(p.x + p.face * 22, p.z, 30, p.face, 0.45f, 74, 20, 2.9f, 7.0f, 0.3f, 0.05f, SL_FIRE);
            shake(5, 0.2f);
        }
        if (p.cnt == 1 && p.t > 0.4f) {
            p.cnt = 2; p.face = -p.face; attackId++;
            playerHit(-20, 135, 36, 60, 32, 280, true, 2);
            addSlash(p.x + p.face * 22, p.z, 30, p.face, 0.45f, 74, 20, 2.9f, 7.0f, 0.3f, -0.05f, SL_FIRE);
            shake(6, 0.25f); play(sfxHeavy, 0.7f);
        }
        fireBurst(2);
        if (p.t > 0.8f) { p.st = S_IDLE; p.t = 0; }
    } break;
    case S_HINO_13: {
        // Décima Terceira Forma: todas as danças encadeadas
        p.vx = mx * 90; p.vz = mz * 60;
        p.tick -= dt;
        if (p.t < 2.2f && p.tick <= 0) {
            p.tick = 0.1f; attackId++;
            playerHit(0, 115, 40, 80, 10, 120, false, 2, true);
            float a0 = frange(0, 2 * PI);
            bool vert = rnd() & 1;
            addSlash(p.x + frange(-15, 15), p.z, 32, (rnd() & 1) ? 1 : -1, 0.32f, frange(45, 80), 14, a0, a0 + 3.6f, vert ? 1.f : 0.36f, frange(-0.4f, 0.4f), SL_FIRE);
            if (p.cnt++ % 4 == 0) play(sfxSwing, 0.5f);
        }
        if (p.t < 2.2f) {
            float a = p.t * 7;
            dragon.push_back({ p.x + cosf(a) * 60, clampf(p.z + sinf(a) * 25, 0, DEPTH), 30 + sinf(a * 2) * 15 });
            if (dragon.size() > 40) dragon.erase(dragon.begin());
            dragonFade = 1; dragonFire = true;
        }
        fireBurst(5);
        if (p.t > 2.25f && !p.fired) {
            p.fired = true; attackId++;
            playerHit(0, 150, 50, 100, 75, 360, true, 2, true);
            addSlash(p.x, p.z, 40, p.face, 0.6f, 100, 30, -2.6f, 2.2f, 1.0f, 0, SL_FIRE);
            addSlash(p.x, p.z, 20, p.face, 0.6f, 120, 20, 0, 2 * PI, 0.32f, 0, SL_FIRE);
            flashScreen = 1; flashColor = RGBc(255, 200, 120); shake(12, 0.5f); play(sfxSlam);
        }
        if (p.t > 2.7f) { p.st = S_IDLE; p.t = 0; }
    } break;
    case S_SLEEP:
        p.vx = p.vz = 0;
        if (atk || jump || dash || kp('U') || kp('I') || kp('O')) p.t += 0.35f;
        if (p.t > 2.3f) { p.st = S_IDLE; p.t = 0; p.inv = 0.6f; banner(L"目覚めろ！", L"Tanjiro acordou do sonho!"); }
        break;
    case S_HURT:
        p.vx *= 0.88f; p.vz = 0;
        if (p.t > 0.35f) { p.st = S_IDLE; p.t = 0; }
        break;
    case S_KNOCK: break;
    case S_DOWN:
        p.vx = 0; p.vz = 0;
        if (p.t > 0.8f) { p.st = S_IDLE; p.t = 0; p.inv = 1.0f; }
        break;
    case S_DEAD:
        p.vz = 0; if (p.y <= 0) p.vx *= 0.9f;
        break;
    default: break;
    }
    playerPhysics(dt);
    // oniguiri
    for (auto& it : items) {
        if (it.t < 0) continue;
        if (fabsf(it.x - p.x) < 20 && fabsf(it.z - p.z) < 10 && p.st != S_DEAD) {
            it.t = -1; p.hp = std::min(p.maxhp, p.hp + 22); play(sfxHeal);
            for (int i = 0; i < 12; i++) addPart(p.x + frange(-10, 10), p.z, frange(5, 50), 0, 0, frange(30, 80), 0.7f, RGBc(120, 255, 150), 1.5f, P_HEAL);
        }
    }
}

void hurtPlayer(float dmg, float srcX, bool knock) {
    Ent& p = P;
    if (p.st == S_DEAD || p.inv > 0) return;
    if (p.st == S_DASH || isSkill(p.st) || p.st == S_DOWN) return;
    if (mode != GM_PLAY) return;
    p.hp -= dmg;
    combo = 0;
    shake(4, 0.2f); hurtFlash = 0.35f;
    play(sfxHurt, 0.8f);
    float sx = signf(p.x - srcX);
    for (int i = 0; i < 8; i++)
        addPart(p.x, p.z + 1, 35, sx * frange(20, 120), frange(-20, 20), frange(40, 160), 0.6f, RGBc(150, 10, 20), 1.8f, P_DOT, 600);
    if (p.hp <= 0) {
        p.hp = 0; p.st = S_DEAD; p.t = 0; p.vy = 260; p.vx = sx * 130; p.y = std::max(p.y, 1.f); modeT = 0;
    } else if (knock || dmg >= 15) {
        p.st = S_KNOCK; p.t = 0; p.vy = 300; p.vx = sx * 170; p.y = std::max(p.y, 1.f); p.inv = 1.4f;
    } else {
        p.st = S_HURT; p.t = 0; p.vx = sx * 150; p.inv = 0.6f;
    }
}

