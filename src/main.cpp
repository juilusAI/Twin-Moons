// Janela Win32, apresentação na tela, entrada, autoteste e WinMain.
#include "common.h"

// ============================================================ janela / apresentação
HWND hwnd;
HDC backDC = 0;
HBITMAP backBmp = 0;
int backW = 0, backH = 0;
std::vector<std::pair<int, HFONT>> fontCache;
bool fullscreen = false;
WINDOWPLACEMENT prevPlace = { sizeof(prevPlace) };

HFONT getFont(int px, int face) {
    int key = px * 4 + face;
    for (auto& f : fontCache) if (f.first == key) return f.second;
    HFONT h = CreateFontW(-px, 0, 0, 0, face == 1 ? FW_HEAVY : FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                          CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, face == 1 ? L"Yu Mincho" : L"Segoe UI");
    fontCache.push_back({ key, h });
    return h;
}

void present() {
    HDC hdc = GetDC(hwnd);
    RECT rc; GetClientRect(hwnd, &rc);
    int cw = std::max(1, (int)rc.right), ch = std::max(1, (int)rc.bottom);
    static uint32_t* backPx = nullptr;
    static std::vector<int> xmap;
    float sc = std::min(cw / (float)W, ch / (float)H);
    int dw = (int)(W * sc), dh = (int)(H * sc), ox = (cw - dw) / 2, oy = (ch - dh) / 2;
    if (cw != backW || ch != backH) {
        if (backBmp) DeleteObject(backBmp);
        if (!backDC) backDC = CreateCompatibleDC(hdc);
        BITMAPINFO bi = {};
        bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bi.bmiHeader.biWidth = cw; bi.bmiHeader.biHeight = -ch;
        bi.bmiHeader.biPlanes = 1; bi.bmiHeader.biBitCount = 32; bi.bmiHeader.biCompression = BI_RGB;
        backBmp = CreateDIBSection(backDC, &bi, DIB_RGB_COLORS, (void**)&backPx, NULL, 0);
        SelectObject(backDC, backBmp);
        backW = cw; backH = ch;
        memset(backPx, 0, (size_t)cw * ch * 4);
        xmap.resize(dw);
        for (int x = 0; x < dw; x++) xmap[x] = std::min(W - 1, (int)(x / sc));
    }
    GdiFlush();
    int prevSy = -1;
    for (int y = 0; y < dh; y++) {
        int sy = std::min(H - 1, (int)(y / sc));
        uint32_t* dst = backPx + (size_t)(oy + y) * cw + ox;
        if (sy == prevSy) { memcpy(dst, dst - cw, dw * 4); continue; }
        const uint32_t* src = fb + sy * W;
        for (int x = 0; x < dw; x++) dst[x] = src[xmap[x]];
        prevSy = sy;
    }
    SetBkMode(backDC, TRANSPARENT);
    for (auto& t : texts) {
        int px = std::max(6, (int)(t.size * sc));
        SelectObject(backDC, getFont(px, t.font));
        UINT al = TA_TOP | (t.align == 1 ? TA_CENTER : (t.align == 2 ? TA_RIGHT : TA_LEFT));
        SetTextAlign(backDC, al);
        int x = ox + (int)(t.x * sc), y = oy + (int)(t.y * sc);
        int o = std::max(1, px / 12);
        SetTextColor(backDC, RGB(8, 6, 12));
        if (t.size < 10) TextOutW(backDC, x + o, y + o, t.s.c_str(), (int)t.s.size()); // texto pequeno: só sombra
        else
            for (int k = 0; k < 4; k++)
                TextOutW(backDC, x + ((k & 1) ? o : -o), y + ((k & 2) ? o : -o), t.s.c_str(), (int)t.s.size());
        SetTextColor(backDC, RGB((t.col >> 16) & 255, (t.col >> 8) & 255, t.col & 255));
        TextOutW(backDC, x, y, t.s.c_str(), (int)t.s.size());
    }
    BitBlt(hdc, 0, 0, cw, ch, backDC, 0, 0, SRCCOPY);
    ReleaseDC(hwnd, hdc);
}

void toggleFullscreen() {
    DWORD style = GetWindowLong(hwnd, GWL_STYLE);
    if (!fullscreen) {
        MONITORINFO mi = { sizeof(mi) };
        if (GetWindowPlacement(hwnd, &prevPlace) && GetMonitorInfo(MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY), &mi)) {
            SetWindowLong(hwnd, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(hwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right - mi.rcMonitor.left,
                         mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    } else {
        SetWindowLong(hwnd, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(hwnd, &prevPlace);
        SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
    fullscreen = !fullscreen;
}

LRESULT CALLBACK WndProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    switch (m) {
    case WM_DESTROY: PostQuitMessage(0); return 0;
    case WM_KEYDOWN: case WM_SYSKEYDOWN:
        if (w < 256) { if (!(l & (1 << 30))) keyPressed[w] = true; keyDown[w] = true; }
        if (w == VK_F11 && !(l & (1 << 30))) toggleFullscreen();
        if (m == WM_SYSKEYDOWN) break;
        return 0;
    case WM_KEYUP: case WM_SYSKEYUP:
        if (w < 256) keyDown[w] = false;
        if (m == WM_SYSKEYUP) break;
        return 0;
    case WM_KILLFOCUS: memset(keyDown, 0, sizeof(keyDown)); return 0;
    case WM_PAINT: { PAINTSTRUCT ps; BeginPaint(h, &ps); EndPaint(h, &ps); return 0; }
    case WM_ERASEBKGND: return 1;
    }
    return DefWindowProcW(h, m, w, l);
}

// ---------- autoteste: entradas roteirizadas + capturas de tela (DemonSlayer.exe --selftest <pasta>)
void saveShot(const std::string& path) {
    BITMAPINFO bi = {};
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = backW; bi.bmiHeader.biHeight = -backH;
    bi.bmiHeader.biPlanes = 1; bi.bmiHeader.biBitCount = 32; bi.bmiHeader.biCompression = BI_RGB;
    std::vector<uint32_t> px((size_t)backW * backH);
    GetDIBits(backDC, backBmp, 0, backH, px.data(), &bi, DIB_RGB_COLORS);
    BITMAPFILEHEADER fh = {};
    fh.bfType = 0x4D42; fh.bfOffBits = sizeof(fh) + sizeof(BITMAPINFOHEADER);
    fh.bfSize = fh.bfOffBits + (DWORD)(px.size() * 4);
    FILE* f = fopen(path.c_str(), "wb");
    if (!f) return;
    fwrite(&fh, sizeof(fh), 1, f); fwrite(&bi.bmiHeader, sizeof(BITMAPINFOHEADER), 1, f);
    fwrite(px.data(), 4, px.size(), f); fclose(f);
}

void autoFight(int fr) {
    Ent* target = nullptr;
    for (auto& e : E) if (e.alive && !e.assist && e.st != S_DEAD) { target = &e; break; }
    if (target) {
        float dx = target->x - P.x;
        keyDown[(int)(dx > 0 ? 'D' : 'A')] = fabsf(dx) > 48;
        if (fabsf(target->z - P.z) > 6) keyDown[(int)(target->z > P.z ? 'S' : 'W')] = true;
        if (fabsf(dx) < 70 && P.face != (dx > 0 ? 1 : -1)) keyDown[(int)(dx > 0 ? 'D' : 'A')] = true;
    } else keyDown[(int)'D'] = true;
    if (fr % 9 == 0) keyPressed[(int)'J'] = true;
    if (P.st == S_SLEEP && fr % 4 == 0) keyPressed[(int)'J'] = true;
}

void selftestInput(int fr, std::string& shot) {
    auto press = [](int k) { keyPressed[k] = true; };
    memset(keyDown, 0, sizeof(keyDown));
    char name[64];
    if (fr == 20) shot = "00_title";
    if (fr == 30) { unlocked = 4; titleSel = 2; }
    if (fr == 40) shot = "00b_title_select";
    if (fr == 50) { titleSel = 0; mode = GM_STORY; modeT = 1; chapter = 1; }
    if (fr == 70) shot = "00c_story";
    // (capítulo, seção do chefe) — o capítulo 3 é testado duas vezes: Daki sozinha e depois Gyutaro
    const int runCh[6] = { 0, 1, 2, 2, 3, 4 }, runSec[6] = { 3, 2, 1, 3, 2, 1 };
    const int base0 = 100, per = 760;
    int run = (fr - base0) / per, lf = (fr - base0) % per;
    if (fr < base0 || run > 5) { if (fr == base0 + 6 * per + 10) PostQuitMessage(0); return; }
    int c = runCh[run];
    if (P.hp < 50) P.hp = 50; // mantém vivo durante o teste
    if (lf == 0) { startChapter(c); }
    if (lf == 50) { sprintf(name, "r%d_c%d_a_wave", run, c); shot = name; }
    if (lf > 10 && lf < 120) autoFight(fr);
    if (lf == 120) jumpToSection(runSec[run]);
    if (lf > 130 && lf < 640) autoFight(fr);
    if (lf == 230) { sprintf(name, "r%d_c%d_b_boss_intro", run, c); shot = name; }
    // técnicas liberadas em cada capítulo
    const int skillFrame[5] = { 330, 330, 330, 330, 330 };
    if (lf == skillFrame[c]) {
        breath = 100;
        if (c == 0) press('O');
        if (c == 1) { keyDown[(int)'W'] = true; press('O'); }
        if (c == 2) { keyDown[(int)'D'] = true; press('O'); }
        if (c == 3) { press('Y'); }
        if (c == 4) { press('H'); }
    }
    if (c == 3 && lf == 345) { breath = 100; keyDown[(int)'S'] = true; press('O'); }
    if (lf == (c == 4 ? 380 : 352)) { sprintf(name, "r%d_c%d_c_skill", run, c); shot = name; }
    if (lf == 470) { sprintf(name, "r%d_c%d_d_boss_fight", run, c); shot = name; }
    if (lf == 560) { sprintf(name, "r%d_c%d_e_boss_fight2", run, c); shot = name; }
    if (lf == 640) { for (auto& e : E) if (isBoss(e.type) && !e.assist && e.st != S_DEAD) { attackId++; damageEnemy(e, e.hp + 1, 100, true, 1); } for (auto& e : E) if (!isBoss(e.type)) e.alive = false; }
    if (lf == 740) { sprintf(name, "r%d_c%d_f_clear", run, c); shot = name; }
}

int WINAPI WinMain(HINSTANCE hi, HINSTANCE, LPSTR cmd, int show) {
    std::string args = cmd ? cmd : "";
    selftest = args.rfind("--selftest", 0) == 0;
    std::string shotDir = selftest && args.size() > 11 ? args.substr(11) : ".";
    int frameNo = 0;
    SetProcessDPIAware();
    WNDCLASSW wc = {};
    wc.lpfnWndProc = WndProc; wc.hInstance = hi; wc.lpszClassName = L"DemonSlayerTwinMoons";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW); wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    RegisterClassW(&wc);
    RECT r = { 0, 0, W * 2, H * 2 };
    AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW, FALSE);
    hwnd = CreateWindowW(wc.lpszClassName, L"Demon Slayer: Twin Moons", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                         r.right - r.left, r.bottom - r.top, NULL, NULL, hi, NULL);
    ShowWindow(hwnd, show);
    timeBeginPeriod(1);
    rngState ^= (uint32_t)GetTickCount();
    genSfx();
    audioInit();
    buildChapters();
    if (!selftest) loadSave();
    titleSel = unlocked;
    chapter = titleSel;
    resetGame();
    mode = GM_TITLE;

    LARGE_INTEGER freq, last, now;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&last);
    double acc = 0;
    const double step = 1.0 / 60.0;
    bool running = true;
    while (running) {
        MSG msg;
        while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) running = false;
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        if (!running) break;
        QueryPerformanceCounter(&now);
        double dt = (double)(now.QuadPart - last.QuadPart) / freq.QuadPart;
        last = now;
        acc += std::min(dt, 0.1);
        std::string shot;
        if (selftest) { acc = step; selftestInput(frameNo++, shot); }
        while (acc >= step) {
            update((float)step);
            memset(keyPressed, 0, sizeof(keyPressed));
            acc -= step;
        }
        LARGE_INTEGER t0, t1, t2;
        QueryPerformanceCounter(&t0);
        render();
        QueryPerformanceCounter(&t1);
        present();
        QueryPerformanceCounter(&t2);
        if (selftest) {
            static double sumR = 0, sumP = 0, maxR = 0;
            double rr = (double)(t1.QuadPart - t0.QuadPart) / freq.QuadPart * 1000, pp = (double)(t2.QuadPart - t1.QuadPart) / freq.QuadPart * 1000;
            sumR += rr; sumP += pp; maxR = std::max(maxR, rr);
            if (frameNo % 500 == 0) {
                FILE* f = fopen((shotDir + "\\perf.txt").c_str(), "a");
                if (f) { fprintf(f, "frame %d (cap %d): render avg %.2f ms (max %.2f), present avg %.2f ms, parts %zu\n", frameNo, chapter + 1, sumR / 500, maxR, sumP / 500, parts.size()); fclose(f); }
                sumR = sumP = maxR = 0;
            }
        }
        if (!shot.empty()) saveShot(shotDir + "\\" + shot + ".bmp");
        audioPump();
        QueryPerformanceCounter(&now);
        double used = (double)(now.QuadPart - last.QuadPart) / freq.QuadPart;
        int ms = (int)((step - used) * 1000) - 1;
        if (ms > 0 && !selftest) Sleep((DWORD)ms);
    }
    audioShutdown();
    timeEndPeriod(1);
    return 0;
}
