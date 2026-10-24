# Sonic 3 & Knuckles — Co-op local 2 jogadores

| | |
|---|---|
| **Engine** | Oxygen (Sonic 3 A.I.R. fork, GPLv3) |
| **Base** | Sonic 3 A.I.R. commit `87fed54` (branch `coop`) |
| **ROM** | `Sonic_Knuckles_wSonic3.bin` — SHA-256 `fba0677f…` — **não incluída, não commitar** |
| **QA** | Código completo; compilação e teste em jogo **pendentes** |
| **Última atualização** | 2026-10-23 |

---

## Estado atual

Todas as Etapas 0–9 estão completas — camada C++ do motor e todos os scripts Lemon.
O projeto está pronto para compilação e teste em jogo.

**O que está pronto:**

| Etapa | Arquivo(s) | Descrição |
|---|---|---|
| 0 | — | Ambiente, clone, `.gitignore`, SHA-256 |
| 1 | `PlaneManager`, `OpenGLRenderResources`, `SoftwareRenderer`, `VideoOut`, `RendererBindings` (C++) | Planos COOP_B/A no motor; `Renderer.fillCoopPlane` binding |
| 2–3 | `coop_charselect.lemon`, `coop_init.lemon`, `coop_players.lemon` | Seleção de personagem; dois personagens controláveis |
| 4 | `coop_render.lemon` | Tela dividida cima/baixo, câmera independente P2 |
| 5 | `coop_world.lemon` | Despawn/bordas por câmera, teletransporte de jogador atrasado |
| 6 | `coop_items.lemon` | Itens compartilhados, morte individual, checkpoints |
| 7 | `coop_finish.lemon` | Fim de fase só quando os dois chegaram |
| 8 | `coop_bonus.lemon` | Bonus stages (GS, SM) em co-op |
| 9 | `coop_specialstage.lemon` | Blue Spheres — P2 controla Tails com pad2 |
| — | `coop_defines.lemon` | Constantes e globals globais do mod |
| — | `coop_intro.lemon` | Vídeo de abertura customizado |

---

## Como compilar (Windows)

1. **Visual Studio 2022** com workload *"Desktop development with C++"*.
2. Ajustar `framework/external/get_msbuild_path.bat` para o caminho do seu `MSBuild.exe`.
3. Rodar `framework/external/build_externals_windows.bat` (baixa e compila SDL2, etc.).
4. Abrir `Oxygen/sonic3air/build/_vstudio/sonic3air.sln`.
5. Selecionar configuração **Release** e compilar o projeto `sonic3air`.
6. O executável sai em `Oxygen/sonic3air/build/bin/`.
7. Colocar a ROM em `Oxygen/sonic3air/Sonic_Knuckles_wSonic3.bin` (**não commitar**).

### Rodar com o launcher
```
Iniciar Sonic 3 Coop.bat
```
Abre o vídeo de intro (`Misc/lordmacintosh_abertura.mp4`) via `ffplay` ou VLC,
depois inicia o jogo automaticamente.

---

## Como jogar

### Controles
Configurar em **Options → Controls** no menu do A.I.R.:
- **P1** — teclado ou controle 1 (`Input.getController(0)`)
- **P2** — teclado ou controle 2 (`Input.getController(1)`)

Qualquer combinação funciona (dois teclados, dois controles, ou um de cada).

### Selecionar personagens
Na tela de Data Select, ao iniciar um novo jogo:
- P1 escolhe seu personagem com o controle 1.
- P2 escolhe com o controle 2.
- Qualquer par é permitido, incluindo dois personagens iguais.

### Regras co-op
| Recurso | Comportamento |
|---|---|
| **Argolas** | Compartilhadas — qualquer jogador coleta, ambos perdem ao levar dano |
| **Vidas** | Compartilhadas — chega a 0 → Game Over para os dois |
| **Monitores** | O efeito (escudo, invencibilidade, tênis, 1-up) aplica-se aos dois |
| **Morte** | Só o jogador que morreu respawna (queda do céu); o outro continua |
| **Checkpoint** | Qualquer jogador ativa o starpost |
| **Fim de ato** | A placa/cápsula só dispara os resultados quando os dois chegaram |
| **Jogador atrasado** | Teletransportado para perto do líder (P1) em eventos de câmera travada |

### Estágios bônus
| Bônus | Comportamento |
|---|---|
| **Glowing Spheres** | O estágio só termina quando os dois saíram pelo topo do feixe |
| **Slot Machine** | Bola P2 extra spawna; o estágio aguarda as duas bolas terminarem |
| **Gumball Machine** | Exit guiado pelo P1 *(limitação conhecida)* |

### Special Stage (Blue Spheres)
P2 controla o Tails com o controle 2, imediatamente e sem delay.
P2 pode pular e desviar de esferas azuis de forma independente.
A câmera e a grade de esferas são compartilhadas (ver Limitações abaixo).

---

## Limitações conhecidas

### Gerais
- **Spawn de objetos**: objetos fora da janela do P1 não são criados (só o despawn é
  dual-camera). Se P2 está muito à frente, itens podem não aparecer. Deferred.
- **Parallax do P2**: o plano de fundo da câmera do P2 usa o mesmo scrolling do P1
  (fundo paralaxe não é calculado por câmera). Visualmente aceitável na maioria das zonas.

### Estágios bônus
- **Gumball Machine**: o retorno ao nível é disparado quando P1 termina. P2 coleta
  gumballs de forma independente mas o timing do exit é do P1.
- **Slot Machine — sprite do P2**: a bola P2 renderiza com o banco VRAM do P1
  (aparência visual = personagem do P1, não do P2).

### Special Stage (Blue Spheres)
- **Sem tela dividida**: ambos veem a mesma câmera 3D (P1 dirige a visão).
  Split-screen real exigiria duplicar todo o bloco de estado (`bluespheres.*`) e
  rodar `SpecialStage()` duas vezes por frame — reescrita completa (~400 linhas).
- **Colisão de esferas do P2**: `fn009402` usa as coordenadas da grade do P1.
  P2 não tem grade independente; esfera coletada = na posição atual do P1.
- **Esfera vermelha**: `fn00972e` só é chamada para P1. Quando P1 bate numa esfera
  vermelha, o estágio encerra para os dois (comportamento original preservado).
- **Sprite do P2 no BS**: sempre aparece como Tails, independentemente do personagem
  escolhido para P2 (só os padrões VRAM do personagem P1 + Tails são carregados).

### Motor C++ — nível de água por viewport (deferred)
- O split de paleta (`PaletteManager::mSplitPositionY`) é global — uma linha de água
  para toda a tela. Em zonas com água (HCZ, LBZ), se P1 está submerso e P2 não (ou
  vice-versa), a linha de água afetará ambas as metades. Pode causar cores erradas no
  viewport do jogador que está na superfície. Corrigível com um array de split por
  viewport no PaletteManager e nos shaders — deferred.

---

## Onde as coisas estão

```
Oxygen/sonic3air/scripts/coop/
  coop_defines.lemon       — globals e constantes
  coop_init.lemon          — ativação do modo co-op
  coop_charselect.lemon    — seleção de personagem por jogador
  coop_players.lemon       — dois personagens controláveis
  coop_render.lemon        — tela dividida e câmera P2
  coop_world.lemon         — despawn, bordas, teletransporte
  coop_items.lemon         — itens, morte individual, checkpoints
  coop_finish.lemon        — fim de ato exige os dois
  coop_intro.lemon         — vídeo de abertura
  coop_bonus.lemon         — bonus stages (GS, SM)
  coop_specialstage.lemon  — Blue Spheres

coop-docs/
  README.md   — este arquivo
  NOTES.md    — diário de engenharia por etapa

Oxygen/sonic3air/Sonic_Knuckles_wSonic3.bin  — ROM (não commitada)
Iniciar Sonic 3 Coop.bat                      — launcher
```

---

## Referências
- Motor base: [github.com/Eukaryot/sonic3air](https://github.com/Eukaryot/sonic3air) (GPLv3)
- Desmontagem S3K: [github.com/sonicretro/skdisasm](https://github.com/sonicretro/skdisasm)
- Processo de documentação: [github.com/cranebeaker/coop-anything](https://github.com/cranebeaker/coop-anything) (AGENTS.md)
- Modo Competition (referência de split-screen): `Oxygen/sonic3air/scripts/level/competition/`
- Câmera P2: `camera.position.x/y.player2` em `0xffffee60/ee64`
