# Demon Slayer: Twin Moons

Beat 'em up 2.5D (estilo *Streets of Rage*) inspirado em Kimetsu no Yaiba, escrito em C++ puro para Windows.
Não usa nenhuma biblioteca externa: os gráficos são desenhados pixel a pixel por um renderizador por software
(640×360, escalado para a janela), o som é sintetizado em tempo real com `waveOut`, e o texto usa GDI.

Jogo de fã, sem fins lucrativos. mas se puder se inscrever no meu canal do youtube( juliusAI736 ) ou so deixar o like nos video ja ajudaria demais tipo vc nao tem ideia do quanto :3 

## Como compilar

**MinGW (g++):**

```bash
build.bat
```

**Visual Studio / CMake:**

```bash
cmake -B build && cmake --build build --config Release
```

Gera `DemonSlayer.exe`. O progresso (capítulos liberados) fica em `savegame.txt`, ao lado do executável.

## Campanha

| Cap. | Cenário | Chefe | Tanjiro aprende |
|---|---|---|---|
| 1 | 那田蜘蛛山 Monte Natagumo | Rui, Lua Inferior 5: rajadas e chuva de fios, teletransporte | Água 1ª e 10ª Forma, Hinokami Kagura: Dança |
| 2 | 無限列車 Trem Infinito | Enmu, Lua Inferior 1: ondas de sono, tentáculos de carne, reforços | 碧羅の天 Céu Azul Claro |
| 3 | 遊郭 Distrito do Entretenimento | Daki (mini-chefe), depois Gyutaro com a Daki ajudando: Lua Superior 6, foices de sangue que voltam, anel de sangue, investida | 烈日紅鏡 Espelho Solar Escarlate |
| 4 | 無限城 Castelo Infinito | Akaza, Lua Superior 3: socos, 空式 ondas de choque, 乱式 rajada, 滅式 Aniquilação; **regenera** | Lâmina do Yoriichi + 赫刀 Lâmina Escarlate, 日暈の龍 Dragão do Halo Solar |
| 5 | 夜明け Até o Amanhecer | Muzan: chicotes, espinhos, estocadas; **regenera**. Aguente até o sol nascer ou derrote-o antes | 拾参ノ型 13ª Forma |

No título, ←/→ escolhe entre os capítulos já liberados.

**Para não ficar difícil demais:**
- Ao morrer, você volta do último ponto de controle (início da seção).
- Cada seção limpa recupera vida.
- Inimigos derrubam oniguiri, que cura.
- No capítulo 5, o amanhecer derrota o Muzan sozinho se você sobreviver.

## Controles

| Tecla | Ação |
|---|---|
| WASD / Setas | Mover (esquerda/direita e profundidade) |
| J | Atacar (combo de 3 golpes; no ar, golpe aéreo) |
| K / Espaço | Pular |
| L / Shift | Esquiva (invencível durante o dash) |
| U | Água · 1ª Forma: Corte da Superfície da Água (30) |
| I | Água · 10ª Forma: Mudança Constante, o dragão d'água (50) |
| O | Hinokami Kagura: Dança (60) |
| ↑ + O | 碧羅の天 Céu Azul Claro: giro vertical subindo (50), a partir do cap. 2 |
| → / ← + O | 烈日紅鏡 Espelho Solar Escarlate: corte para frente e para trás (60), a partir do cap. 3 |
| ↓ + O | 日暈の龍 Dragão do Halo Solar: investida com dragão de fogo (70), a partir do cap. 4 |
| Y | 赫刀 Lâmina Escarlate: 10 s de lâmina vermelha, mais dano e bloqueia a regeneração (30), a partir do cap. 4 |
| H | 拾参ノ型 13ª Forma: todas as danças encadeadas (respiração cheia), cap. 5 |
| N | Chamar a Nezuko (chute, recarga de 14 s) |
| P / Esc | Pausa (Q na pausa volta ao título) |
| F11 | Tela cheia |

Os números entre parênteses são o custo de respiração. As técnicas podem cancelar o combo normal.
A respiração recarrega com o tempo e a cada golpe acertado.

Enmu solta ondas roxas de sono. Se o Tanjiro dormir, aperte J repetidamente para acordar.

## Organização do código

Todos os arquivos ficam em `src/` e incluem `common.h`, que tem os tipos, constantes, variáveis globais
compartilhadas e as funções que cada arquivo oferece aos outros.

| Arquivo | O que tem | Mexa aqui para... |
|---|---|---|
| `common.h` | tipos (`Ent`, `Proj`, `Chapter`...), enums, constantes do mundo, protótipos | adicionar um novo tipo de inimigo/estado ou uma função usada em vários arquivos |
| `chapters.cpp` | capítulos, seções, ondas de inimigos, textos da história, save | mudar a ordem/quantidade de inimigos, criar capítulos |
| `player.cpp` | movimento do Tanjiro, combo, técnicas de respiração, Lâmina Escarlate, Nezuko | ajustar dano, custo de respiração, criar técnicas |
| `enemies.cpp` | IA dos inimigos, padrões de ataque dos chefes, projéteis | deixar chefes mais fáceis/difíceis, novos ataques |
| `game.cpp` | estado global, seções e checkpoints, câmera, loop de atualização | regras gerais (cura por seção, amanhecer do Muzan) |
| `characters.cpp` | aparência (`lookFor`), poses e desenho do corpo, cabelos | cores e visual dos personagens |
| `effects.cpp` | desenho de cortes, dragões, partículas e ataques dos chefes | visual das técnicas |
| `background.cpp` | cenários de cada capítulo e o chão | cenários |
| `ui.cpp` | HUD, faixas de técnica, título, história, pausa, fim de jogo | textos e telas |
| `render.cpp` | renderizador: pixels, formas, fitas em arco, fila de texto | raramente |
| `audio.cpp` | efeitos sonoros sintetizados e mixer | sons |
| `main.cpp` | janela, tela cheia, teclado, autoteste, `WinMain` | raramente |

Ao criar uma função num arquivo que precise ser usada em outro, declare-a em `common.h`.

## Como funciona o 2.5D

O mundo tem coordenadas `x` (horizontal), `z` (profundidade) e `y` (altura). A escala de cada objeto
cresce conforme `z` se aproxima da câmera, e a posição na tela segue uma projeção em perspectiva
(o chão converge para um horizonte acima da tela). Personagens, projéteis e efeitos são ordenados por `z`
antes do desenho. Cada capítulo tem cenário próprio com camadas de paralaxe. No trem, o fundo corre
sozinho para dar a sensação de velocidade.

## Autoteste

`DemonSlayer.exe --selftest <pasta>` joga sozinho todos os chefes com entradas roteirizadas, salva capturas `.bmp`
e o tempo médio de cada quadro em `perf.txt` dentro da pasta.
