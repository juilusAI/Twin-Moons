// Áudio: efeitos sintetizados em tempo real e mixer próprio sobre waveOut.
#include "common.h"

// ============================================================ áudio (mixer próprio sobre waveOut)
const int SR = 22050;
Sfx sfxSwing, sfxHit, sfxHeavy, sfxWater, sfxFire, sfxHurt, sfxDie, sfxJump, sfxThread, sfxNezuko, sfxSlam, sfxDash;
Sfx sfxPunch, sfxWhip, sfxSleep, sfxAkato, sfxHeal;
struct Voice { const Sfx* s; size_t pos; float vol; };
Voice voices[24];
HWAVEOUT hwo = 0;
const int ABUF = 4, ALEN = 512;
WAVEHDR ahdr[ABUF];
int16_t abuf[ABUF][ALEN];

void play(const Sfx& s, float vol) {
    for (auto& v : voices) if (!v.s) { v = { &s, 0, vol }; return; }
    voices[rnd() % 24] = { &s, 0, vol };
}
void audioInit() {
    WAVEFORMATEX wf = {};
    wf.wFormatTag = WAVE_FORMAT_PCM; wf.nChannels = 1; wf.nSamplesPerSec = SR; wf.wBitsPerSample = 16;
    wf.nBlockAlign = 2; wf.nAvgBytesPerSec = SR * 2;
    if (waveOutOpen(&hwo, WAVE_MAPPER, &wf, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR) { hwo = 0; return; }
    for (int i = 0; i < ABUF; i++) {
        memset(&ahdr[i], 0, sizeof(WAVEHDR));
        ahdr[i].lpData = (LPSTR)abuf[i]; ahdr[i].dwBufferLength = ALEN * 2;
        waveOutPrepareHeader(hwo, &ahdr[i], sizeof(WAVEHDR));
    }
}
void audioPump() {
    if (!hwo) return;
    for (int b = 0; b < ABUF; b++) {
        if (ahdr[b].dwFlags & WHDR_INQUEUE) continue;
        for (int i = 0; i < ALEN; i++) {
            float m = 0;
            for (auto& v : voices) {
                if (!v.s) continue;
                m += v.s->d[v.pos++] * v.vol;
                if (v.pos >= v.s->d.size()) v.s = nullptr;
            }
            m = tanhf(m * 0.8f);
            abuf[b][i] = (int16_t)(m * 30000);
        }
        waveOutWrite(hwo, &ahdr[b], sizeof(WAVEHDR));
    }
}
float noise1() { return frand() * 2 - 1; }
void genSfx() {
    auto make = [](Sfx& s, float dur) { s.d.assign((size_t)(SR * dur), 0.f); return (int)s.d.size(); };
    int n;
    n = make(sfxSwing, 0.17f);
    { float lp = 0; for (int i = 0; i < n; i++) { float u = (float)i / n; float k = 0.05f + 0.5f * sinf(u * PI); lp += (noise1() - lp) * k; float e = sinf(u * PI); sfxSwing.d[i] = lp * e * e * 0.9f; } }
    n = make(sfxHit, 0.16f);
    { float ph = 0; for (int i = 0; i < n; i++) { float t = (float)i / SR; ph += 2 * PI * (170 - 300 * t) / SR; sfxHit.d[i] = sinf(ph) * expf(-22 * t) * 0.8f + noise1() * expf(-55 * t) * 0.6f; } }
    n = make(sfxHeavy, 0.35f);
    { float ph = 0, lp = 0; for (int i = 0; i < n; i++) { float t = (float)i / SR; ph += 2 * PI * (95 - 120 * t) / SR; lp += (noise1() - lp) * 0.3f; sfxHeavy.d[i] = sinf(ph) * expf(-9 * t) + lp * expf(-14 * t) * 0.9f; } }
    n = make(sfxWater, 0.75f);
    { float lp = 0, ph = 0; for (int i = 0; i < n; i++) { float t = (float)i / SR, u = (float)i / n; float k = 0.08f + 0.07f * sinf(t * 40); lp += (noise1() - lp) * k; ph += 2 * PI * (900 + 500 * sinf(t * 25) + 600 * u) / SR; float e = sinf(u * PI); sfxWater.d[i] = (lp * 1.4f + sinf(ph) * 0.12f) * e; } }
    n = make(sfxFire, 0.9f);
    { float lp = 0, ph = 0; for (int i = 0; i < n; i++) { float u = (float)i / n; lp += (noise1() - lp) * 0.18f; float crack = (frand() < 0.004f) ? noise1() * 2 : 0; ph += 2 * PI * (60 + 30 * u) / SR; float e = sinf(u * PI); sfxFire.d[i] = (lp * 0.9f + crack + sinf(ph) * 0.5f) * e; } }
    n = make(sfxHurt, 0.22f);
    { float ph = 0; for (int i = 0; i < n; i++) { float t = (float)i / SR; ph += 2 * PI * (240 - 500 * t) / SR; float sq = sinf(ph) > 0 ? 1.f : -1.f; sfxHurt.d[i] = (sq * 0.35f + noise1() * 0.3f) * expf(-10 * t); } }
    n = make(sfxDie, 1.0f);
    { float lp = 0, hp = 0, prev = 0; for (int i = 0; i < n; i++) { float t = (float)i / SR; float x = noise1(); hp = 0.9f * (hp + x - prev); prev = x; lp += (hp - lp) * 0.5f; sfxDie.d[i] = lp * expf(-3.5f * t) * 0.5f + sinf(t * 2 * PI * 70) * expf(-12 * t) * 0.6f; } }
    n = make(sfxJump, 0.12f);
    { float ph = 0; for (int i = 0; i < n; i++) { float u = (float)i / n; ph += 2 * PI * (300 + 500 * u) / SR; sfxJump.d[i] = sinf(ph) * (1 - u) * 0.35f + noise1() * (1 - u) * 0.15f; } }
    n = make(sfxThread, 0.3f);
    { float ph = 0; for (int i = 0; i < n; i++) { float t = (float)i / SR, u = (float)i / n; ph += 2 * PI * (2200 - 900 * u) / SR; sfxThread.d[i] = sinf(ph) * (0.5f + 0.5f * sinf(t * 120)) * (1 - u) * 0.3f; } }
    n = make(sfxNezuko, 0.35f);
    { float lp = 0, ph = 0; for (int i = 0; i < n; i++) { float t = (float)i / SR, u = (float)i / n; lp += (noise1() - lp) * 0.25f; ph += 2 * PI * (420 + 80 * sinf(t * 30)) / SR; sfxNezuko.d[i] = (lp * 0.6f + sinf(ph) * 0.25f * (1 - u)) * sinf(u * PI); } }
    n = make(sfxSlam, 0.5f);
    { float ph = 0, lp = 0; for (int i = 0; i < n; i++) { float t = (float)i / SR; ph += 2 * PI * (55 - 30 * t) / SR; lp += (noise1() - lp) * 0.1f; sfxSlam.d[i] = sinf(ph) * expf(-6 * t) * 1.1f + lp * expf(-8 * t) * 1.2f; } }
    n = make(sfxDash, 0.15f);
    { float lp = 0; for (int i = 0; i < n; i++) { float u = (float)i / n; lp += (noise1() - lp) * (0.4f - 0.3f * u); sfxDash.d[i] = lp * (1 - u) * 0.6f; } }
    n = make(sfxPunch, 0.14f);
    { float ph = 0; for (int i = 0; i < n; i++) { float t = (float)i / SR; ph += 2 * PI * (130 - 500 * t) / SR; sfxPunch.d[i] = sinf(ph) * expf(-18 * t) + noise1() * expf(-70 * t) * 0.7f; } }
    n = make(sfxWhip, 0.3f);
    { float lp = 0, hp = 0, prev = 0; for (int i = 0; i < n; i++) { float t = (float)i / SR; float x = noise1(); hp = 0.95f * (hp + x - prev); prev = x; lp += (hp - lp) * 0.6f; float e = t < 0.12f ? t / 0.12f * 0.3f : expf(-25 * (t - 0.12f)) * 1.2f; sfxWhip.d[i] = lp * e; } }
    n = make(sfxSleep, 0.9f);
    { for (int i = 0; i < n; i++) { float t = (float)i / SR; float v = sinf(2 * PI * 880 * t + sinf(t * 30) * 2) + 0.6f * sinf(2 * PI * 1320 * t) + 0.4f * sinf(2 * PI * 660 * t * (1 + 0.01f * sinf(t * 8))); sfxSleep.d[i] = v * expf(-3 * t) * 0.18f; } }
    n = make(sfxAkato, 0.7f);
    { float ph = 0, lp = 0; for (int i = 0; i < n; i++) { float u = (float)i / n; ph += 2 * PI * (180 + 700 * u * u) / SR; lp += (noise1() - lp) * 0.2f; sfxAkato.d[i] = (sinf(ph) * 0.4f + lp * 0.6f) * sinf(u * PI); } }
    n = make(sfxHeal, 0.35f);
    { float ph = 0; for (int i = 0; i < n; i++) { float u = (float)i / n; ph += 2 * PI * (u < 0.5f ? 660 : 990) / SR; sfxHeal.d[i] = sinf(ph) * (1 - u) * 0.3f; } }
}


void audioShutdown() {
    if (!hwo) return;
    waveOutReset(hwo);
    for (int i = 0; i < ABUF; i++) waveOutUnprepareHeader(hwo, &ahdr[i], sizeof(WAVEHDR));
    waveOutClose(hwo);
    hwo = 0;
}
