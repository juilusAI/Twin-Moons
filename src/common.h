// Demon Slayer: Twin Moons — beat 'em up 2.5D em C++ puro (Win32 + renderizador por software)
// Cabeçalho compartilhado: tipos, constantes, globais e as funções que cada arquivo oferece aos outros.
//
//   render.cpp      pixels, formas, fitas em arco, transformação local, fila de texto
//   audio.cpp       efeitos sonoros sintetizados e mixer
//   chapters.cpp    dados da campanha e save
//   game.cpp        estado global, seções, câmera, loop de atualização
//   player.cpp      Tanjiro, técnicas e Nezuko
//   enemies.cpp     inimigos, chefes e projéteis
//   background.cpp  cenários e chão
//   characters.cpp  aparência e poses dos personagens
//   effects.cpp     desenho de cortes, dragões, partículas e ataques
//   ui.cpp          HUD, menus e a composição do quadro (render)
//   main.cpp        janela, entrada, autoteste e WinMain
#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif
#include <windows.h>
#include <mmsystem.h>
#include <cstdint>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <string>
#include <algorithm>
#include <initializer_list>

static const float PI = 3.14159265f;
static const int W = 640, H = 360;   // resolução interna, escalada para a janela
extern uint32_t fb[W * H];           // imagem do quadro atual (0x00RRGGBB)

// ============================================================ util
extern uint32_t rngState;
static inline uint32_t rnd() { rngState ^= rngState << 13; rngState ^= rngState >> 17; rngState ^= rngState << 5; return rngState; }
static inline float frand() { return (rnd() & 0xFFFFFF) / 16777216.f; }
static inline float frange(float a, float b) { return a + (b - a) * frand(); }
static inline uint32_t hash2(int x, int y) {
    uint32_t h = (uint32_t)x * 374761393u + (uint32_t)y * 668265263u;
    h = (h ^ (h >> 13)) * 1274126177u;
    return h ^ (h >> 16);
}
static inline float clampf(float v, float a, float b) { return v < a ? a : (v > b ? b : v); }
static inline float smooth01(float t) { t = clampf(t, 0, 1); return t * t * (3 - 2 * t); }
static inline float signf(float v) { return v < 0 ? -1.f : 1.f; }
static inline uint32_t RGBc(int r, int g, int b) { return (uint32_t)((r << 16) | (g << 8) | b); }
static inline uint32_t lerpC(uint32_t a, uint32_t b, float t) {
    t = clampf(t, 0, 1);
    int ar = (a >> 16) & 255, ag = (a >> 8) & 255, ab = a & 255;
    int br = (b >> 16) & 255, bg = (b >> 8) & 255, bb = b & 255;
    return RGBc(ar + (int)((br - ar) * t), ag + (int)((bg - ag) * t), ab + (int)((bb - ab) * t));
}
static inline uint32_t mulC(uint32_t c, float k) {
    int r = (int)(((c >> 16) & 255) * k), g = (int)(((c >> 8) & 255) * k), b = (int)((c & 255) * k);
    return RGBc(std::min(r, 255), std::min(g, 255), std::min(b, 255));
}

// ============================================================ render.cpp
extern bool g_add;      // blend aditivo
extern float g_dis;     // dissolução (cinzas) 0..1
extern float g_flash;   // clarão branco ao tomar dano
extern float g_fog;     // névoa de profundidade
extern uint32_t fogC;   // cor da névoa do cenário atual

// transformação local: personagens são desenhados em coordenadas próprias (pés na origem, y para cima negativo)
struct Xf { float ox, oy, s, c, sn; int f; };
static inline float TX(const Xf& x, float lx, float ly) { return x.ox + (lx * x.c - ly * x.sn) * x.s * x.f; }
static inline float TY(const Xf& x, float lx, float ly) { return x.oy + (lx * x.sn + ly * x.c) * x.s; }

struct TextCmd { std::wstring s; float x, y, size; uint32_t col; int align, font; };
extern std::vector<TextCmd> texts;

void plot(int x, int y, uint32_t c, int a);
void hspan(int y, float xa, float xb, uint32_t c, int a);
void fillRect(float x, float y, float w, float h, uint32_t c, int a = 255);
void fillPoly(const float* px, const float* py, int n, uint32_t c, int a = 255);
void fillEllipse(float cx, float cy, float rx, float ry, uint32_t c, int a = 255);
void fillCircle(float cx, float cy, float r, uint32_t c, int a = 255);
void capsule(float x0, float y0, float x1, float y1, float r0, float r1, uint32_t c, int a = 255);
void glow(float cx, float cy, float r, uint32_t c, int a);
void ribbon(float cx, float cy, float r, float w, float a0, float a1, float squash, float tilt, int face, float s,
            uint32_t cA, uint32_t cB, int alpha, bool headHeavy);
Xf mkXf(float ox, float oy, float s, int f, float rot);
void Lcap(const Xf& x, float x0, float y0, float x1, float y1, float r0, float r1, uint32_t c, int a = 255);
void Lcirc(const Xf& x, float lx, float ly, float r, uint32_t c, int a = 255);
void Lpoly(const Xf& x, std::initializer_list<float> pts, uint32_t c, int a = 255);
void patOn(const Xf& x, float cell, uint32_t c1, uint32_t c2);
void patOff();
void text(const std::wstring& s, float x, float y, float size, uint32_t col, int align = 0, int font = 0);

// ============================================================ audio.cpp
struct Sfx { std::vector<float> d; };
extern Sfx sfxSwing, sfxHit, sfxHeavy, sfxWater, sfxFire, sfxHurt, sfxDie, sfxJump, sfxThread, sfxNezuko, sfxSlam, sfxDash;
extern Sfx sfxPunch, sfxWhip, sfxSleep, sfxAkato, sfxHeal;
void play(const Sfx& s, float vol = 1.f);
void audioInit();
void audioPump();
void audioShutdown();
void genSfx();

// ============================================================ mundo / projeção 2.5D
// x = horizontal, z = profundidade (0 fundo .. 100 perto da câmera), y = altura
static const float DEPTH = 100.f;
static const float S_FAR = 0.80f, S_NEAR = 1.16f;
static const float FLOOR_TOP = 200.f, FLOOR_BOT = 352.f;
static const float KY = (FLOOR_BOT - FLOOR_TOP) / (S_NEAR - S_FAR);
static const float HORIZON = FLOOR_TOP - S_FAR * KY;
extern float camX, rcamX, shakeY, shakeT, shakeMag;
static inline float zScale(float z) { return S_FAR + (S_NEAR - S_FAR) * (z / DEPTH); }
static inline float groundY(float z) { return HORIZON + KY * zScale(z) + shakeY; }
static inline float projX(float x, float z) { return W * 0.5f + (x - rcamX) * zScale(z); }

// ============================================================ entidades
enum EType { ET_PLAYER, ET_GRUNT, ET_FAST, ET_BIG, ET_NEZUKO, ET_RUI, ET_ENMU, ET_DAKI, ET_GYUTARO, ET_AKAZA, ET_MUZAN };
static inline bool isBoss(EType t) { return t >= ET_RUI; }
enum St {
    S_IDLE, S_WALK, S_ATK1, S_ATK2, S_ATK3, S_AIRATK, S_JUMP, S_DASH,
    S_WATER1, S_WATER10, S_HINO, S_HINO_SKY, S_HINO_SUN, S_HINO_DRAGON, S_HINO_13, S_SLEEP,
    S_HURT, S_KNOCK, S_DOWN, S_DEAD, S_E_WIND, S_E_STRIKE, S_E_RECOVER, S_E_LUNGE, S_B_CAST, S_B_INTRO
};
static inline bool isSkill(St s) { return s >= S_WATER1 && s <= S_HINO_13; }
struct Ent {
    EType type = ET_GRUNT;
    float x = 0, z = 50, y = 0, vx = 0, vz = 0, vy = 0;
    int face = 1;
    float hp = 100, maxhp = 100;
    St st = S_IDLE;
    float t = 0, anim = 0, inv = 0, flash = 0, cd = 0, cd2 = 0, aiT = 0, strafe = 0, dieT = 0, tick = 0, scale = 1, aux = 0, regen = 0, punchT = 0;
    int lastHit = -1, lastNz = -1, idx = 0, pat = 0, cnt = 0;
    bool alive = true, token = false, armor = false, hitDone = false, airUsed = false, queued = false, fired = false;
    bool assist = false, floats = false;
};

struct Part { float x, z, y, vx, vz, vy, life, max, size, grav; uint32_t c; int kind; };
enum { P_SPARK, P_DOT, P_WATER, P_FIRE, P_ASH, P_RING, P_HEAL };
struct Slash { float x, z, y; int face; float t, dur, r, w, a0, a1, squash, tilt; int style; };
enum { SL_BLADE, SL_WATER, SL_FIRE, SL_CLAW, SL_BLOOD, SL_FIST };
struct Proj { float x, z, y, vx, vz, life, t, dmg, x2, r; int kind, style; bool alive, hit, ret; };
enum { PR_THREAD, PR_RAIN, PR_SLEEP, PR_OBI, PR_SWEEP, PR_SICKLE, PR_AIR, PR_RING, PR_LINE };
struct Petal { float x, y, vx, vy, ph; };
struct Item { float x, z, t; };
struct V3 { float x, z, y; };

// ============================================================ game.cpp (estado global)
enum GameMode { GM_TITLE, GM_STORY, GM_PLAY, GM_OVER, GM_CLEAR, GM_WIN };
static const float MUZAN_LIMIT = 110.f;   // segundos até o amanhecer na luta final

extern Ent P, NZ;
extern bool nzActive, iteratingE;
extern std::vector<Ent> E, pendingE;
extern std::vector<Part> parts;
extern std::vector<Slash> slashes;
extern std::vector<Proj> projs;
extern std::vector<Petal> petals;
extern std::vector<Item> items;
extern std::vector<V3> dragon;
extern float dragonFade;
extern bool dragonFire;
extern GameMode mode;
extern bool paused, selftest;
extern float breath, nezCd, dashCd, comboT, hitstop, flashScreen, hurtFlash, akatoT;
extern uint32_t flashColor;
extern int score, combo, maxCombo, attackId, kills;
extern float modeT, gameT, dawn, bannerT, goT, introT, muzanT;
extern std::wstring bannerJP, bannerPT;
extern bool bossEnraged, regenHint;

extern bool keyDown[256], keyPressed[256];
static inline bool kd(int k) { return keyDown[k]; }
static inline bool kp(int k) { return keyPressed[k]; }

void shake(float mag, float t);
void addPart(float x, float z, float y, float vx, float vz, float vy, float life, uint32_t c, float size, int kind, float grav = 0);
void addSlash(float x, float z, float y, int face, float dur, float r, float w, float a0, float a1, float squash, float tilt, int style);
Proj& addProj(int kind, float x, float z, float y, float vx, float vz, float life, float dmg, int style = 0);
void banner(const wchar_t* jp, const wchar_t* pt);
void updateParts(float dt);
void resetGame();
void jumpToSection(int k);
void startChapter(int c, bool fromCheckpoint = false);
void updateSections(float dt);
void boundsFor(float& l, float& r);
void updateCamera(float dt);
void updatePetals(float dt);
void update(float dt);

// ============================================================ chapters.cpp
enum { BG_FOREST, BG_TRAIN, BG_TOWN, BG_CASTLE, BG_CITY };
struct Section { float l, r; std::vector<EType> wave; const wchar_t* name; };
struct Chapter { const wchar_t* jp; const wchar_t* name; int bg; std::vector<const wchar_t*> story; const wchar_t* unlock; std::vector<Section> secs; };
extern std::vector<Chapter> chapters;
extern std::vector<Section> secs;
extern int chapter, unlocked, titleSel, checkpoint;
extern int curSec;
extern bool secLocked;
extern size_t spawnIdx;
extern float spawnT;
void buildChapters();
int curBG();
void loadSave();
void writeSave();

// ============================================================ player.cpp
void startAttack(St s);
void callNezuko();
void updateNezuko(float dt);
void sleepPlayer();
bool trySpecial(float mx, float mz);
void updatePlayer(float dt);
void hurtPlayer(float dmg, float srcX, bool knock = false);

// ============================================================ enemies.cpp
int tokensUsed();
int livingEnemies();
Ent* findType(EType t);
void spawnEnemy(EType t, float x, float z);
void spawnBoss(EType t, bool assist = false);
const wchar_t* bossName(EType t);
void damageEnemy(Ent& e, float dmg, float kbx, bool launch, int fx);
int playerHit(float r0, float r1, float zt, float yt, float dmg, float kb, bool launch, int fx, bool radial = false);
float windDur(const Ent& e);
void updateEnemy(Ent& e, float dt);
void updateProjs(float dt);

// ============================================================ background.cpp
void drawBackground();
void drawFloor();
void drawChochin(float x, float y);

// ============================================================ characters.cpp
void drawShadow(float x, float z, float y, float scale);
void drawEnt(const Ent& e);
void drawNezuko();
void drawItem(const Item& it);

// ============================================================ effects.cpp
void drawSlash(const Slash& sl);
void drawDragon();
void drawParticle(const Part& p);
void drawProj(const Proj& pr);

// ============================================================ ui.cpp
void render();
