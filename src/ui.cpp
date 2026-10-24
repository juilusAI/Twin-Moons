// Interface: HUD, faixas de técnica, título, história, pausa e telas de fim.
#include "common.h"

// ============================================================ HUD
void bar(float x, float y, float w, float h, float v, uint32_t c1, uint32_t c2) {
    fillRect(x - 2, y - 2, w + 4, h + 4, RGBc(10, 8, 14), 220);
    fillRect(x, y, w, h, RGBc(40, 30, 40));
    float fw = w * clampf(v, 0, 1);
    for (int i = 0; i < (int)h; i++) fillRect(x, y + i, fw, 1, lerpC(c1, c2, (float)i / h));
    fillRect(x, y, fw, 1, RGBc(255, 255, 255), 90);
}

void drawBannerBand(float y, float alpha) {
    for (int i = -14; i < 14; i++) {
        float j = (float)(hash2(i, 3) % 30);
        int a = (int)(alpha * 200 * (1 - fabsf(i / 14.f)));
        hspan((int)(y + i), 60 - j, W - 60 + j, RGBc(8, 6, 10), a);
    }
}

void skillIcon(float x, float y, const wchar_t* key, float cost, bool on, uint32_t col) {
    bool ok = on && breath >= cost;
    fillRect(x, y, 22, 12, ok ? col : RGBc(40, 36, 46), ok ? 220 : 200);
    fillRect(x, y, 22, 1, RGBc(255, 255, 255), ok ? 120 : 40);
    text(key, x + 11, y + 0.5f, 7.5f, ok ? 0xFFFFFF : RGBc(120, 110, 130), 1);
}

void drawHUD() {
    fillRect(8, 8, 196, 66, RGBc(10, 8, 18), 150);
    text(L"竈門 炭治郎", 14, 9, 12, 0xFFFFFF, 0, 1);
    text(L"TANJIRO", 96, 11, 9, RGBc(140, 220, 180), 0, 0);
    bar(14, 26, 180, 8, P.hp / P.maxhp, RGBc(255, 90, 80), RGBc(160, 20, 30));
    bar(14, 40, 180, 6, breath / 100, RGBc(130, 210, 255), RGBc(30, 90, 220));
    if (breath >= 100) glow(194, 43, 14, RGBc(255, 140, 40), (int)(140 + 100 * sinf(gameT * 8)));
    // técnicas disponíveis
    float ix = 14, iy = 54;
    skillIcon(ix, iy, L"U", 30, true, RGBc(40, 110, 220)); ix += 24;
    skillIcon(ix, iy, L"I", 50, true, RGBc(40, 110, 220)); ix += 24;
    skillIcon(ix, iy, L"O", 60, true, RGBc(220, 100, 20)); ix += 24;
    if (chapter >= 1) { skillIcon(ix, iy, L"↑O", 50, true, RGBc(220, 100, 20)); ix += 24; }
    if (chapter >= 2) { skillIcon(ix, iy, L"→O", 60, true, RGBc(220, 100, 20)); ix += 24; }
    if (chapter >= 3) { skillIcon(ix, iy, L"↓O", 70, true, RGBc(220, 100, 20)); ix += 24; skillIcon(ix, iy, L"Y", 30, akatoT <= 0, RGBc(200, 20, 20)); ix += 24; }
    if (chapter >= 4) { skillIcon(ix, iy, L"H", 100, true, RGBc(255, 170, 40)); ix += 24; }
    if (akatoT > 0) { bar(14, 70, 180 * akatoT / 10, 2, 1, RGBc(255, 80, 40), RGBc(200, 0, 0)); }
    // Nezuko
    float nx = 222, ny = 30;
    fillCircle(nx, ny, 13, RGBc(10, 8, 18), 180);
    fillCircle(nx, ny, 11, nezCd <= 0 ? RGBc(235, 130, 165) : RGBc(90, 60, 75));
    if (nezCd > 0) ribbon(nx, ny, 9, 4, -PI / 2, -PI / 2 + 2 * PI * (nezCd / 14), 1, 0, 1, 1, RGBc(30, 20, 30), RGBc(30, 20, 30), 200, false);
    text(L"N", nx, ny - 6, 9, 0xFFFFFF, 1);
    wchar_t buf[96];
    swprintf(buf, 96, L"%08d", score);
    text(buf, W - 12, 8, 14, RGBc(255, 230, 170), 2);
    swprintf(buf, 96, L"%ls · %ls", chapters[chapter].name, secs[curSec].name);
    text(buf, W - 12, 26, 8, RGBc(200, 190, 230), 2);
    if (combo >= 2) {
        float pop = 1 + std::max(0.f, comboT - 2.1f) * 2;
        swprintf(buf, 96, L"%d", combo);
        text(buf, W - 40, 60, 26 * pop, combo >= 20 ? RGBc(255, 140, 60) : RGBc(140, 210, 255), 2, 1);
        text(L"連撃", W - 12, 70, 11, 0xFFFFFF, 2, 1);
        text(L"HITS", W - 12, 84, 8, RGBc(200, 200, 220), 2);
    }
    // chefes
    float by = 334;
    for (auto& e : E) {
        if (!isBoss(e.type) || !e.alive || e.assist) continue;
        bar(120, by, 400, 7, e.hp / e.maxhp, RGBc(255, 70, 90), RGBc(130, 0, 30));
        text(bossName(e.type), 120, by - 16, 11, RGBc(255, 200, 210), 0, 1);
        if (e.regen > 0 && akatoT <= 0 && e.st != S_DEAD) text(L"再生中 · regenerando — use 赫刀 (Y)", 520, by - 14, 8, RGBc(255, 150, 150), 2);
        by -= 30;
    }
    if (chapter == 4 && curSec == (int)secs.size() - 1 && findType(ET_MUZAN)) {
        bar(220, 82, 200, 5, muzanT / MUZAN_LIMIT, RGBc(255, 220, 140), RGBc(255, 120, 40));
        text(L"夜明けまで · até o amanhecer", W / 2.f, 66, 9, RGBc(255, 220, 170), 1);
    }
    if (P.st == S_SLEEP) {
        fillRect(0, 0, W, H, RGBc(80, 30, 140), 70);
        text(L"眠り... Tanjiro adormeceu!  Aperte J repetidamente para acordar!", W / 2.f, 230, 11, RGBc(230, 200, 255), 1);
    }
    if (bannerT > 0) {
        float a = std::min(1.f, bannerT * 3) * std::min(1.f, (2.2f - bannerT) * 6);
        drawBannerBand(122, a);
        text(bannerJP, W / 2.f, 106, 20, RGBc(255, 255, 255), 1, 1);
        text(bannerPT, W / 2.f, 130, 9, RGBc(170, 210, 255), 1, 0);
    }
    if (introT > 0 && mode == GM_PLAY) {
        drawBannerBand(186, std::min(1.f, introT * 2));
        text(secs[curSec].name, W / 2.f, 174, 18, RGBc(255, 230, 200), 1, 1);
    }
    if (!secLocked && goT > 0 && curSec + 1 < (int)secs.size() && ((int)(goT * 3) & 1)) text(L"AVANCE  →", W - 30, 150, 18, RGBc(255, 220, 120), 2, 1);
}

void drawVignette() {
    static uint16_t vig[W * H];
    static bool init = false;
    if (!init) {
        init = true;
        for (int y = 0; y < H; y++)
            for (int x = 0; x < W; x++) {
                float dx = (x - W / 2.f) / (W / 2.f), dy = (y - H / 2.f) / (H / 2.f);
                float d = dx * dx * 0.6f + dy * dy * 0.8f;
                vig[y * W + x] = (uint16_t)(256 * clampf(1.1f - d * 0.45f, 0.45f, 1));
            }
    }
    float hf = clampf(hurtFlash / 0.35f, 0, 1);
    for (int i = 0; i < W * H; i++) {
        uint32_t c = fb[i];
        int v = vig[i];
        int r = ((c >> 16) & 255) * v >> 8, g = ((c >> 8) & 255) * v >> 8, b = (c & 255) * v >> 8;
        if (hf > 0 && v < 230) { int k = (int)((230 - v) * hf * 1.4f); r += k; g -= k / 3; b -= k / 3; }
        r = std::min(255, std::max(0, r)); g = std::min(255, std::max(0, g)); b = std::min(255, std::max(0, b));
        fb[i] = (uint32_t)((r << 16) | (g << 8) | b);
    }
}

void drawPetals() {
    int bg = curBG();
    uint32_t c = bg == BG_FOREST ? RGBc(205, 160, 245) : bg == BG_TOWN ? RGBc(255, 185, 205) : bg == BG_CASTLE ? RGBc(200, 150, 100) : bg == BG_CITY ? RGBc(130, 125, 135) : RGBc(255, 150, 80);
    for (auto& p : petals) {
        if (bg == BG_TRAIN) { fillRect(p.x, p.y, 6, 1, RGBc(200, 200, 230), 60); continue; }
        float w = 1.6f + sinf(p.ph) * 0.8f;
        fillEllipse(p.x, p.y, w, 1.4f, c, bg == BG_CASTLE ? 90 : 200);
    }
}

void drawControls(float y) {
    const wchar_t* lines[] = {
        L"Mover: WASD / Setas      Atacar: J      Pular: K / Espaço      Esquiva: L / Shift      Nezuko: N",
        L"U: Água 1ª Forma      I: Água 10ª Forma      O: Hinokami (↑O, →O, ↓O liberam outras formas)",
        L"Y: Lâmina Escarlate (cap. 4)      H: 13ª Forma (cap. 5)      P / Esc: Pausa      F11: Tela cheia",
    };
    for (int i = 0; i < 3; i++) text(lines[i], W / 2.f, y + i * 13, 8.5f, RGBc(210, 205, 230), 1);
}

void render() {
    texts.clear();
    drawBackground();
    drawFloor();
    if (mode != GM_TITLE && mode != GM_STORY) {
        for (auto& pr : projs) if (pr.kind == PR_RAIN || pr.kind == PR_SWEEP || pr.kind == PR_LINE || pr.kind == PR_RING) if (!pr.hit || pr.kind == PR_RAIN || pr.kind == PR_RING) drawProj(pr);
        for (auto& it : items) drawItem(it);
        drawShadow(P.x, P.z, P.y, 1);
        for (auto& e : E) if (e.alive && !(e.st == S_DEAD && e.dieT > 0.6f)) drawShadow(e.x, e.z, e.y, e.scale);
        if (nzActive) drawShadow(NZ.x, NZ.z, NZ.y, 0.9f);
        std::vector<std::pair<float, int>> order;
        order.push_back({ P.z, -1 });
        for (int i = 0; i < (int)E.size(); i++) order.push_back({ E[i].z, i });
        if (nzActive) order.push_back({ NZ.z, -2 });
        for (int i = 0; i < (int)projs.size(); i++) {
            int k = projs[i].kind;
            if (k == PR_THREAD || k == PR_OBI || k == PR_AIR || k == PR_SLEEP || k == PR_SICKLE || ((k == PR_SWEEP || k == PR_LINE) && projs[i].hit)) order.push_back({ projs[i].z, 1000 + i });
        }
        std::stable_sort(order.begin(), order.end(), [](auto& a, auto& b) { return a.first < b.first; });
        for (auto& o : order) {
            if (o.second == -1) drawEnt(P);
            else if (o.second == -2) drawNezuko();
            else if (o.second >= 1000) drawProj(projs[o.second - 1000]);
            else drawEnt(E[o.second]);
        }
        drawDragon();
        for (auto& sl : slashes) drawSlash(sl);
    }
    for (auto& p : parts) drawParticle(p);
    drawPetals();
    drawVignette();
    if (flashScreen > 0) fillRect(0, 0, W, H, flashColor, (int)(160 * std::min(1.f, flashScreen)));

    wchar_t buf[160];
    if (mode == GM_TITLE) {
        fillRect(0, 0, W, H, 0, 80);
        drawBannerBand(96, 1);
        text(L"鬼滅の刃", W / 2.f, 24, 34, RGBc(255, 255, 255), 1, 1);
        text(L"DEMON SLAYER", W / 2.f, 76, 22, RGBc(240, 90, 80), 1, 1);
        text(L"TWIN MOONS  ·  Noite das Duas Luas", W / 2.f, 104, 11, RGBc(200, 210, 255), 1, 0);
        const Chapter& c = chapters[titleSel];
        drawBannerBand(170, 0.8f);
        swprintf(buf, 160, L"%ls  第%d章  %ls  %ls", titleSel > 0 ? L"◀" : L"  ", titleSel + 1, c.jp, titleSel < unlocked ? L"▶" : L"  ");
        text(buf, W / 2.f, 148, 16, RGBc(255, 230, 200), 1, 1);
        swprintf(buf, 160, L"Capítulo %d: %ls", titleSel + 1, c.name);
        text(buf, W / 2.f, 172, 11, RGBc(230, 220, 255), 1, 0);
        if (unlocked < (int)chapters.size() - 1) {
            swprintf(buf, 160, L"Capítulos liberados: %d de %d", unlocked + 1, (int)chapters.size());
            text(buf, W / 2.f, 188, 8, RGBc(170, 160, 190), 1);
        }
        if ((int)(gameT * 2) & 1) text(L"←/→ escolhe o capítulo   ·   ENTER para começar", W / 2.f, 210, 11, RGBc(255, 230, 180), 1, 0);
        drawControls(262);
        text(L"Jogo de fã sem fins lucrativos", W / 2.f, 340, 7, RGBc(130, 120, 150), 1);
    } else if (mode == GM_STORY) {
        const Chapter& c = chapters[chapter];
        fillRect(0, 0, W, H, 0, 150);
        swprintf(buf, 160, L"第%d章", chapter + 1);
        text(buf, W / 2.f, 40, 14, RGBc(255, 200, 160), 1, 1);
        text(c.jp, W / 2.f, 60, 32, 0xFFFFFF, 1, 1);
        text(c.name, W / 2.f, 104, 13, RGBc(220, 210, 255), 1, 0);
        for (size_t i = 0; i < c.story.size(); i++) text(c.story[i], W / 2.f, 140 + i * 18.f, 10.5f, RGBc(235, 230, 240), 1, 0);
        drawBannerBand(222, 0.9f);
        text(c.unlock, W / 2.f, 215, 9, RGBc(255, 210, 120), 1, 0);
        if (modeT > 0.4f && ((int)(gameT * 2) & 1)) text(L"ENTER para começar   ·   ESC para voltar", W / 2.f, 290, 11, RGBc(255, 230, 180), 1);
    } else {
        drawHUD();
    }
    if (paused) {
        fillRect(0, 0, W, H, 0, 150);
        text(L"PAUSA", W / 2.f, 120, 26, 0xFFFFFF, 1, 1);
        text(L"P / Esc: continuar      Q: sair para o título", W / 2.f, 160, 10, RGBc(210, 205, 230), 1);
        drawControls(220);
    }
    if (mode == GM_OVER && modeT > 0.5f) {
        fillRect(0, 0, W, H, RGBc(30, 0, 0), (int)std::min(170.f, modeT * 200));
        text(L"敗北", W / 2.f, 100, 36, RGBc(255, 80, 80), 1, 1);
        text(L"Tanjiro caiu... mas a chama não se apaga.", W / 2.f, 150, 11, RGBc(230, 210, 210), 1);
        if (modeT > 1.5f) {
            swprintf(buf, 160, L"ENTER: tentar de novo (%ls)   ·   ESC: título", checkpoint > 0 ? L"do último ponto de controle" : L"do início do capítulo");
            text(buf, W / 2.f, 190, 11, RGBc(255, 230, 180), 1);
        }
    }
    if (mode == GM_CLEAR && modeT > 1.2f) {
        fillRect(0, 0, W, H, 0, (int)std::min(120.f, (modeT - 1.2f) * 200));
        text(L"章 完", W / 2.f, 80, 32, RGBc(255, 230, 150), 1, 1);
        swprintf(buf, 160, L"Capítulo %d concluído: %ls", chapter + 1, chapters[chapter].name);
        text(buf, W / 2.f, 128, 12, 0xFFFFFF, 1);
        swprintf(buf, 160, L"Pontos: %d      Demônios: %d      Maior combo: %d", score, kills, maxCombo);
        text(buf, W / 2.f, 150, 10, RGBc(230, 230, 240), 1);
        if (modeT > 1.5f) text(L"ENTER: próximo capítulo", W / 2.f, 190, 12, RGBc(255, 230, 180), 1);
    }
    if (mode == GM_WIN && modeT > 1.5f) {
        text(L"勝利", W / 2.f, 60, 36, RGBc(255, 230, 150), 1, 1);
        text(L"O sol nasce. Muzan virou cinzas e a longa noite acabou.", W / 2.f, 110, 11, RGBc(255, 245, 230), 1);
        text(L"Obrigado por jogar!", W / 2.f, 128, 10, RGBc(255, 245, 230), 1);
        swprintf(buf, 160, L"Pontos: %d      Demônios: %d      Maior combo: %d", score, kills, maxCombo);
        text(buf, W / 2.f, 150, 11, RGBc(255, 255, 255), 1);
        if (modeT > 2.5f) text(L"ENTER: voltar ao título", W / 2.f, 185, 12, RGBc(255, 230, 180), 1);
    }
}

