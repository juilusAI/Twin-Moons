// Capítulos da campanha (seções, ondas, histórias) e save do progresso.
#include "common.h"

// ============================================================ capítulos
std::vector<Chapter> chapters;
std::vector<Section> secs;
int chapter = 0, unlocked = 0, titleSel = 0, checkpoint = 0;
int curSec = 0;
bool secLocked = false;
size_t spawnIdx = 0;
float spawnT = 0;

void buildChapters() {
    const EType G = ET_GRUNT, F = ET_FAST, B = ET_BIG;
    auto S = [](int i, std::vector<EType> w, const wchar_t* n) { return Section{ i * 720.f, (i + 1) * 720.f, w, n }; };
    chapters = {
        { L"那田蜘蛛山", L"Monte Natagumo", BG_FOREST,
          { L"Tanjiro, Zenitsu e Inosuke sobem o Monte Natagumo.",
            L"Uma família de demônios-aranha domina a floresta.",
            L"No topo espera Rui, a Lua Inferior Cinco." },
          L"Técnicas: Água 1ª Forma (U) · Água 10ª Forma (I) · Hinokami Kagura: Dança (O)",
          { S(0, { G, G, G, G }, L"Estrada da Montanha"), S(1, { G, F, G, F, G, G }, L"Teias da Floresta"),
            S(2, { G, B, F, G, F }, L"Santuário Abandonado"), S(3, { ET_RUI }, L"Covil da Família Aranha") } },
        { L"無限列車", L"Trem Infinito", BG_TRAIN,
          { L"A missão continua a bordo do Trem Infinito, ao lado de Rengoku.",
            L"Passageiros desaparecem em sonhos sem fim.",
            L"Enmu, a Lua Inferior Um, quer devorar todos — cuidado com o sono!" },
          L"Nova técnica: 碧羅の天 Céu Azul Claro (↑ + O)",
          { S(0, { G, G, F, G, F }, L"Vagões Traseiros"), S(1, { G, B, F, G, F, G }, L"Teto do Trem"),
            S(2, { ET_ENMU }, L"Locomotiva") } },
        { L"遊郭", L"Distrito do Entretenimento", BG_TOWN,
          { L"Em Yoshiwara, Tanjiro se disfarça para caçar uma Lua Superior.",
            L"Daki se esconde entre as cortesãs da Casa Kyogoku...",
            L"...e dentro dela dorme seu irmão Gyutaro. Os dois são a Lua Superior Seis." },
          L"Nova técnica: 烈日紅鏡 Espelho Solar Escarlate (→ ou ← + O)",
          { S(0, { G, F, G, F, G }, L"Ruas de Yoshiwara"), S(1, { ET_DAKI }, L"Casa Kyogoku"),
            S(2, { G, B, F, F, G }, L"Telhados"), S(3, { ET_GYUTARO }, L"Irmãos Demônios") } },
        { L"無限城", L"Castelo Infinito", BG_CASTLE,
          { L"Na Vila dos Ferreiros, Tanjiro recebeu a lâmina do Yoriichi Tipo Zero.",
            L"A marca do caçador desperta em sua testa.",
            L"No Castelo Infinito ele reencontra Akaza, a Lua Superior Três." },
          L"Lâmina Escarlate 赫刀 (Y): bloqueia a regeneração · Nova técnica: 日暈の龍 Dragão do Halo Solar (↓ + O)",
          { S(0, { F, G, F, G, F, G }, L"Corredores Sem Fim"), S(1, { B, G, F, B, G }, L"Salão Invertido"),
            S(2, { ET_AKAZA }, L"Duelo com Akaza") } },
        { L"夜明け", L"Até o Amanhecer", BG_CITY,
          { L"O castelo desaba e Muzan Kibutsuji surge na cidade.",
            L"Os Hashira lutam até o limite para segurá-lo.",
            L"Aguente até o sol nascer — ou derrote-o antes disso!" },
          L"Técnica final: 拾参ノ型 Décima Terceira Forma (H, respiração cheia)",
          { S(0, { G, F, B, G, F, G, F }, L"Ruínas da Cidade"), S(1, { ET_MUZAN }, L"Confronto Final") } },
    };
}
int curBG() { return chapters[chapter].bg; }

std::string savePath() {
    char buf[MAX_PATH];
    DWORD n = GetModuleFileNameA(NULL, buf, MAX_PATH);
    std::string p(buf, n);
    size_t k = p.find_last_of("\\/");
    return (k == std::string::npos ? std::string(".") : p.substr(0, k)) + "\\savegame.txt";
}
void loadSave() {
    FILE* f = fopen(savePath().c_str(), "r");
    if (!f) return;
    int v = 0;
    if (fscanf(f, "%d", &v) == 1) unlocked = std::max(0, std::min(v, 4));
    fclose(f);
}
void writeSave() {
    if (selftest) return;
    FILE* f = fopen(savePath().c_str(), "w");
    if (!f) return;
    fprintf(f, "%d\n", unlocked);
    fclose(f);
}
