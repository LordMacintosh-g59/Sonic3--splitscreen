# NOTES — Sonic 3 & Knuckles Co-op

## Identidade do jogo
| Campo | Valor |
|---|---|
| ROM | Sonic_Knuckles_wSonic3.bin |
| SHA-256 | fba0677fde9f76df93f3e98d6310d8af68b9847bde16e253d73cd4dd8134ed23 |
| Base | Sonic 3 A.I.R. (github.com/Eukaryot/sonic3air) |
| Licença base | GPLv3 — uso não comercial |
| Commit base | 87fed54 |
| Branch | coop |

## Diário

### 2026-10-24 — Special Stage (esferas azuis) em co-op com tela dividida
- **Regras:** cada jogador corre sozinho pelo mesmo planeta (posição, direção, velocidade e pulo próprios). O mapa de esferas e o contador de azuis são compartilhados. Esfera vermelha elimina só quem bateu, e a metade dele mostra "AGUARDANDO O PARCEIRO". A esmeralda aparece na frente de quem ainda está vivo e qualquer um pode pegá-la. Com os dois fora, a fase termina como o game over original.
- **Estado por jogador:** o estado de movimento (`bluespheres.*` de 0xffffe420+, mais `scrolloffset.y.planeB`) do jogador inativo fica em globais `coop.bs.other.*`. `coop.bs.useState(p)` troca os valores. O P2 (objeto 0xffffb04a, update 0x00927a) vira um corredor completo, com a mesma lógica do P1 (0x00903e) e o controle 2 no lugar do 1.
- **Animação de coleta:** os slots de animação (0xffffa400) checam a posição do corredor. Cada slot guarda no byte 1 quem coletou, e `fn009e34` usa a posição dessa pessoa.
- **Tela dividida:** cada metade mostra as linhas 58..169 da visão original de 224 linhas, de modo que os pés do personagem ficam na borda de baixo. Os itens de cada metade usam bandas de render queue: 0x1001..0x4fff em cima, 0x5001..0x8fff embaixo, e 0x9000+ para o que é compartilhado (HUD e textos). O céu (plano B) usa os scroll sets 0 e 2.
- **Cores por metade:** o xadrez do chão depende da posição do corredor, então a metade de baixo usa a paleta secundária do motor (a da água), a partir da linha 112. A nova binding `Renderer.setSecondaryPaletteColor` grava cores com alfa. A mesma paleta corrige as cores da linha 0 quando P1 e P2 são Sonic e Knuckles.
- **Largada:** o texto "Get Blue Spheres" dá a largada automática nos dois jogadores.
- **Personagens:** o A.I.R. desenha os personagens do estágio a partir da ROM (`Standalone.onWriteToSpriteTable`, que usa `getSecondCharacter()` para o P2), então a VRAM não precisa ser alterada.
- **Verificado com atalho temporário para o estágio, já removido:**
  - tela dividida com visões independentes;
  - P2 virando sozinho;
  - eliminação de um jogador, com o outro seguindo sozinho;
  - os dois fora encerram a fase;
  - coleta pelo P1 com `CHEAT_ONE_BLUE_SPHERE` levando à esmeralda.
- **Não verificado:** a esmeralda pega pelo P2 (usa o mesmo código, com o estado do P2 ativo) e a jogabilidade em estágios longos com dois humanos.
- **Observação:** sem comando, em alguns estágios o corredor bate numa esfera vermelha logo na largada. Isso também acontece no modo de 1 jogador.

### 2026-10-24 — Hooks de endereço ignoravam overrides com scripts do cache
- **Sintoma:** com Knuckles (ou Sonic) no P2, o controle do P1 movia os dois. Era intermitente: funcionava logo depois de editar um `.lemon` e falhava nas execuções seguintes.
- **Causa:** quando os scripts vêm de `scripts.bin` (cache em `saves/` ou `gamedata.bin` do jogo instalado), o módulo desserializado não é registrado no `GlobalsLookup`. Por isso `collectHooksFromFunctions` não achava o override mais profundo, e o hook `0x01656c` chamava a `Character.UpdateNormalState.Knuckles` original, que lê `control.pad1`. Medido com contadores: 0 chamadas ao override vindo do cache, cerca de 900 com o script recém-compilado. O Tails só escapava porque a base dele já lê `control.tails`.
- **Correção (`LemonScriptProgram.cpp`):** `getDeepestFunctionOverload` passa a usar como fallback a última definição de cada nome+assinatura em `mProgram.getScriptFunctions()`.
- **Verificação:** P2 Knuckles com scripts compilados, P2 Knuckles com cache e P2 Sonic com cache. Nos três, o P2 fica parado enquanto o P1 corre.

### 2026-10-24 — Controle num jogador e teclado no outro; tela dividida após a intro
- **Tela dividida atrasada após a intro da AIZ:** depois da cena, o objeto do P2 ficava vários segundos sem ser executado, e a checagem de espera só existia em `Character.BaseUpdate.*`. Agora o override de `UpdateGameObjects` (`coop_world.lemon`) também chama `coop.updatePlayer2Waiting()` com `A0 = 0xffffb04a` enquanto o P2 espera. Verificado: a divisão aparece em 0:04.
- **Dispositivos por jogador:** `InputManager::getPlayerDeviceFlags` (0x01 = teclado, 0x02 = controle) é exposto como `Game.getCoopPlayerDevices(player)`, que respeita a troca de controles. O Data Select mostra "P1: CONTROLE / P2: TECLADO".
- **Troca com L** (tecla E / LB) no Data Select: chama `Game.swapCoopPlayerDevices()` (`ControlsIn::switchGamepads`). A borda é detectada por frame de simulação no bit 0x0400, com o latch `coop.deviceSwapLatch`, porque `Input.buttonPressed` se repete entre frames de simulação e, depois da troca, o outro dispositivo ainda está segurando L. A troca é desfeita ao voltar ao menu principal (`GameApp::openMainMenu`).
- **IA do Tails no P2:** `Character.UpdateNormalState.Tails` agora alimenta `control.tails = control.pad2` e roda a base com `player2.control_override = -1`. Isso pula a cópia do pad e a IA, que imitava o input do P1.
- **Verificação:** só teclado (Keyboard2 mapeado temporariamente), o P2 fica parado quando só o P1 se move. Depois da troca, o teclado move só o P1. **Não testado com controle físico:** não foi possível simular um gamepad.

### 2026-10-23 — Instalador
- `installer\gerar-instalador.ps1`: `Sonic3AIR.exe -pack` (gera `gamedata/audiodata/enginedata.bin` na raiz
  de `Oxygen\sonic3air`; o `scripts.bin` compilado vai dentro do `gamedata.bin`), monta `dist\Sonic3Coop`
  e compila `installer\sonic3coop.iss` (Inno Setup 6) → `installer\output\Instalar-Sonic3-Coop.exe`.
- Sem ROM e sem áudio remasterizado (o jogo oferece o download). O exe não depende do runtime do VC++.
- Cuidado: restaurar um `.lemon` de backup com `Copy-Item` mantém a data antiga, e o cache
  `saves\scripts.bin` passa por "atualizado". Ao restaurar, atualize a data do arquivo.

### 2026-10-23 — P2 só entra após a cena inicial; teleporte, vidas e ultrapassagem
- P2 com personagem diferente do Tails morria em loop na abertura do AIZ1 (a cena só conhece o P1
  e o Tails original). Agora o P2 espera (`coop.p2Waiting`, checado em `Character.BaseUpdate.*`):
  invisível e sem física até `global.level_started != 0` e o P1 sem `control_override`. Enquanto
  espera, a tela fica cheia (renderização/câmera normais).
- `coop.teleportLaggingPlayer` usava `u16(p1x - p2x)`: com o P2 à frente o valor dava a volta e ele
  era teleportado para trás do P1 a cada quadro (trava, pisca, não ultrapassa). Agora é com sinal.
- Teleporte/renascimento gravavam invencibilidade em `A0 + 0x68` (fora do objeto de 0x4a bytes,
  corrompendo o objeto de game over em 0xffffb094). Agora usam `char.invuln.countdown` (+0x34).
- Game over em co-op não tirava o personagem do estado "morto": as vidas desciam a cada quadro e
  davam a volta (99). Agora vai para `INACTIVE` como no jogo original, e as vidas não ficam negativas.
- Durante a atualização do P2, `control.player1` recebe sempre o controle 2 (antes, com o P1 em
  cutscene, o P2 Sonic/Knuckles lia a entrada da cutscene do P1).

### 2026-10-23 — HUD, argolas da fase e teclado para o P2
- O renderizador co-op (cópia do Competition) não chamava `RenderHUD` nem `RenderStaticRings`.
  A HUD (compartilhada) é desenhada uma vez em espaço de tela; as argolas fixas são desenhadas por
  metade (`coop.renderStaticRings`), com a câmera e o recorte de cada metade. Só existem as argolas
  da janela que o jogo mantém em volta do P1.
- Teclado: o teclado 1 é sempre do P1 e o teclado 2 do P2. Em co-op (`Game::isCoopMode()`), se o P1
  tem controle e o P2 não, `InputManager::setKeyboardToPlayerWithoutGamepad` passa o teclado 1
  para o P2. Ao abrir o menu principal o co-op é desligado e o teclado volta ao P1.

### 2026-10-23 — Controle independente, seleção por jogador, intro com som
- **Intro sem som:** o registro `coop_intro` no `audio_default.json` tinha se perdido (instabilidade do
  drive F:). Refeito e conferido no git. `StartPhase` agora é `1`: vídeo → SEGA → tela título → menu.
- **Controle 1 movia os dois:** o P2 nunca teve entrada própria. A física de Sonic/Knuckles lê
  `control.player1` e o Tails do P2 rodava a IA, que imita o P1. Agora, durante
  `Character.UpdateNormalState.*` do 2º slot, o controle 2 assume o lugar do controle 1, e a IA do
  Tails não roda para o P2. Não sobrescrever `Character.BaseUpdate.*`: o renderizador reconhece os
  personagens pelos endereços originais (0x010a94 / 0x01365c / 0x016444), e `addressof` de uma
  função sobrescrita deixa de devolvê-los.
- **Escolha ignorada:** só era gravada em "No Save"/slot novo. Agora fica em `coop.character.p1/p2`,
  escolhida no Data Select (cima/baixo no controle de cada jogador) e válida para qualquer slot.
- **Knuckles azul / sprite embaralhado:** `renderNextCompModeObject` (Competition) não chama
  `Standalone.onWriteToSpriteTable`, então os personagens saíam como sprites VDP crus (paleta da
  linha 0, tiles de VRAM compartilhados). O co-op usa `coop.renderNextObject` com o gancho, e
  `Standalone.getCharacterRenderQueue` mantém cada personagem na faixa de fila da sua metade.
- Limitação: rotas e cenas exclusivas do Knuckles seguem `global.characters` (Sonic & Tails).

### 2026-10-23 — Scripts nunca recarregavam; tela dividida, menu Co-op e intro refeitos
- **Causa de "nada muda":** sem Dev Mode e sem `data/scripts.bin`, o motor carregava o cache
  `Oxygen/sonic3air/saves/scripts.bin` (gerado às 18:01) e ignorava os `.lemon` editados depois.
  Todo teste feito após 18:01 rodou scripts antigos. Correção em `LemonScriptProgram.cpp`:
  recompila do código-fonte quando algum `.lemon` é mais novo que esse cache.
- **Plano co-op do P2:** `fillBufferByAbstraction` lia uma tabela de chunks da ROM (`0x00f02a`) e
  usava a camada de frente para os dois planos. Substituída por `fillCoopBufferFromLevelLayout`,
  que lê o formato do S3K em RAM (layout `0xFFFF8000`, chunks `0xFFFF0000`, blocos `0xFFFF9000`)
  e grava nas mesmas posições de anel que a VDP (rolagem = posição da câmera).
- Metade de baixo: frente = `PLANE_COOP_A` com a câmera do P2; fundo = cópia do fundo do P1.
- Cada metade mostra ~110 das 224 linhas da câmera; um "view offset" por jogador escolhe quais
  linhas mostrar para o personagem ficar dentro da sua metade (planos e sprites).
- Menu principal: nova entrada "CO-OP GAME"; os scripts usam `coop.isActive()` → `Game.isCoopMode()`.
  `LoadLevel` no `config.json` (atalho de dev) também liga o co-op.
- Intro: o MP4 virou 240 quadros JPG (`data/images/intro`) + `coop_intro.ogg`, tocados pelo
  `GameApp` antes do disclaimer/menu (qualquer botão pula). A tentativa antiga em Lemon
  (gancho na tela da SEGA, que o A.I.R. não usa) foi removida.

### 2026-10-23 — Etapa 1 verificada — Motor C++ já implementado
- Auditoria confirmou que todas as modificações C++ necessárias já estavam no branch:
  - `PlaneManager.h/.cpp`: `PLANE_COOP_B=4`, `PLANE_COOP_A=5`, `PLANE_COUNT=6`,
    `fillCoopPlane()` usando `fillBufferByAbstraction()` com screenSize(400, 112).
    Buffer `mPlanePatternsBuffer[PLANE_COUNT]` já dimensionado para 6 planos.
  - `OpenGLRenderResources.h/.cpp`: `mPlanePatternsTexture[PLANE_COUNT]` (6 texturas);
    loop de upload iterando de 0 a `PLANE_COUNT`; co-op planes marcados para upload
    incondicional (scripts preenchem todo frame).
  - `SoftwareRenderer.cpp`: trata `PLANE_COOP_B/A` via `getPlaneDataInVRAM()` que
    já retorna `mPlanePatternsBuffer[planeType]` para planos co-op. `MAX_BUFFER_PLANE_DATA=8`
    é suficiente (máximo 5 planos em uso simultâneo no co-op).
  - `VideoOut.cpp`: `collectGeometries` usa `customPlane.mSourcePlane & 0x07`
    (não `& 0x03`) — planos 4 e 5 passam corretamente.
  - `RenderPlaneShader.cpp`: usa `resources.getPlanePatternsTexture(geometry.mPlaneIndex)`
    diretamente — funciona para qualquer índice 0–5.
  - `RendererBindings.cpp`: `Renderer_fillCoopPlane()` registrado como
    `Renderer.fillCoopPlane(planeIndex, cameraX, cameraY)`.
  - `cpp_core_functions.lemon`: `declare function void Renderer.fillCoopPlane(...)` presente.
- PENDÊNCIA: split de paleta por viewport (nível de água diferente por metade).
  `PaletteManager::mSplitPositionY` é global; cada jogador em água diferente pode
  ter cores erradas. Deferred — requer array de splitY por viewport + shader.

### 2026-10-23 — Etapa 10 concluída — Documentação final
- `coop-docs/README.md` reescrito com:
  - Tabela de estado por etapa (o que está pronto em Lemon vs. pendente em C++)
  - Instruções completas de compilação (VS 2022, build_externals, solução)
  - Instruções de jogo: controles, regras co-op, bonus stages, Special Stage
  - Seção de Limitações Conhecidas organizada por área (gerais, bônus, BS, motor)
  - Mapa de arquivos do projeto
  - Links de referência
- QA: nenhum teste de compilação ou em-jogo realizado nesta sessão. Todos os
  scripts foram escritos e revisados por leitura cruzada com os originais do A.I.R.,
  mas o status é "pendente de compilação" para todas as etapas de Lemon, e
  "pendente de implementação" para a Etapa 1 (motor C++) que desbloqueia o split-screen.

### 2026-10-23 — Etapa 9 concluída (scripts, pendente compilação)
- `coop_specialstage.lemon` criado com:
  - `fn009212()` override: reseta `coop.bs.p1.eliminated` e `coop.bs.p2.eliminated` a cada
    nova visita ao estágio, antes de chamar o init base do objeto Tails.
  - `fn00937c()` override: no co-op, P2 sempre usa `control.pad2` diretamente, sem o
    sistema de buffer/delay do original. O resultado (em D0.u16) é lido por `fn00927a`
    para controle de direção e pulo do Tails.
- Dois globals novos em `coop_defines.lemon`:
    `coop.bs.p1.eliminated` e `coop.bs.p2.eliminated`.
- LIMITAÇÃO CONHECIDA — tela dividida: o BS stage é monolítico (loop `while(true)` em
  `SpecialStage()`, 0x0081c6, ~400 linhas). Todo estado de mundo (posição na grade,
  direção, velocidade) fica em variáveis globais únicas. Dois viewports independentes
  exigiriam duplicar todo esse bloco de estado e rodar a simulação duas vezes por frame
  — reescrita completa. Deferred para iteração futura.
- LIMITAÇÃO CONHECIDA — colisão de esferas do P2: `fn009402()` lê as coordenadas do P1
  (`bluespheres.position.x/y`). Não há grade independente para P2. P2 só pode coletar
  esferas que estão na posição atual do P1 na grade.
- LIMITAÇÃO CONHECIDA — esfera vermelha: `fn00972e()` só é chamada para P1. Quando P1
  bate numa esfera vermelha, o estágio encerra para ambos (comportamento original
  preservado). Eliminação independente do P2 só será útil após o state-swap da grade.
- LIMITAÇÃO CONHECIDA — sprite do P2 no BS: Tails é sempre renderizado com
  `SpecialStage.Tails.targetInVRAM` independentemente de `coop.character.p2`, pois o
  BS stage só carrega padrões para o personagem do P1 e para o Tails.
- O que funciona bem: P2 controla o Tails com o controle 2 sem delay; pode pular e
  desviar de esferas azuis independentemente de P1; os dois jogadores percorrem o mesmo
  corredor 3D, mas P2 tem agência real sobre seus próprios pulos.

### 2026-10-23 — Etapa 8 concluída (scripts, pendente compilação)
- `coop_bonus.lemon` criado com:
  - `fn049f20()` override: reseta `coop.bonus.p1exited / p2exited` quando o feixe de energia
    do Glowing Spheres é inicializado (nova visita ao bônus).
  - `fn049fe4()` override (GS exit): rastreia P1 e P2 individualmente; o estágio só retorna
    à fase normal (`global.stage_type = 0`) quando **ambos** saíram pelo topo do feixe.
  - `fn04b9ce()` override (SM init): após o init do P1, spawna um objeto "bola P2" dinâmico
    com flag `u8[A0 + 0x4c] = 1`, `base_state = 0x02` e tipo de personagem de `coop.character.p2`.
    Também reseta os flags de exit da SM.
  - `fn04b97c()` override (SM render): quando `A0` é a bola P2 (`u8[A0+0x4c] == 1`), usa
    `A1 = 0xffffb04a` para sprite patterns e chama `fn04bbf4()` (também overrido).
  - `fn04bbf4()` override (SM câmera): quando `A0` é a bola P2, escreve em `0xffffee60/ee64`
    (câmera P2) em vez da câmera principal do P1.
  - `fn04bc54()` override (SM fadeout): P2 ball gira e espera, marca `coop.bonus.p2exited`
    quando o countdown chega a zero e desativa o objeto. P1 ball congela em countdown=1 até
    P2 concluir, depois executa o exit normal via `base.fn04bc54()`.
  - `BonusSM.P2.ballUpdate()`: loop de update da bola P2 — troca temporariamente
    `control.player1 = control.pad2` para que a física da SM use o input do P2, depois
    restaura e chama `fn04b97c()` para render.
- Dois globals novos em `coop_defines.lemon`:
    `coop.bonus.p1exited` e `coop.bonus.p2exited`.
- LIMITAÇÃO CONHECIDA — Gumball Machine: não tem override de exit; o nível retorna quando
  o P1 sai. P2 coleta gumballs independentemente mas o timing do exit é guiado pelo P1.
- LIMITAÇÃO CONHECIDA — SM sprite: a bola P2 usa o banco VRAM de `Char.Player1.targetInVRAM`
  porque apenas esse banco é carregado para a SM. Aparência visual da bola P2 no SM é a do
  personagem do P1. Correto nos próximos Etapas se necessário.

### 2026-10-23 — Etapa 7 concluída (scripts, pendente compilação)
- `coop_finish.lemon` criado com:
  - `fn0838c2()` override: remove o `player2.control_override = 0xff` no co-op para que
    o P2 ainda possa se mover e chegar ao fim da fase.
  - `fn083912()` override (signpost): só aciona win-pose e level-results quando P1 está no
    chão E `u16[0xffffb04a + 0x10] >= signpostX` (P2 chegou à posição da placa).
  - `fn0868f8()` override (cápsula padrão): aguarda P1 no chão+vivo E P2 vivo+no chão.
  - `fn086984()` override (cápsula voadora — MGZ2): mesma lógica adaptada.
  - `coop.triggerWinSequence()`: chama `StartCharacterWinPose()` para P1 e P2, aloca
    `LevelResults.Update` e chama `Game.onZoneActCompleted()`.
  - `coop.finished.p1/p2` marcados como 1 quando o fim é acionado (já declarados em
    coop_defines.lemon desde Etapa 0).
- `coop_items.lemon` atualizado — `coop.respawnPlayer()` melhorado:
  - Personagem posicionado 224px acima do topo do viewport (`camY - 0xe0`) para efeito
    de "descida do céu" visível ao jogador.
  - Sonic cai livremente (gravidade); jogador pode pular e controlar durante a queda.
  - Tails spawna com `tails.fly_countdown = 0x78` e `double_jump_state = 1` (hélice já
    girando); jogador segura jump para flutuar a descida.
  - Knuckles cai; jogador pressiona jump no ar para planar como de costume.
  - Invencibilidade de 300 frames (5 s) cobre toda a queda + margem de segurança.
  - Controle imediato — sem lock de input: o jogador guia a descida desde o primeiro frame.

### 2026-10-23 — Etapa 6 concluída (scripts, pendente compilação)
- `coop_items.lemon` criado com:
  - `ItemEffect.ApplyExtraRings()` override: no co-op, P2 quebrando monitor de argolas
    e atingindo threshold de 1-up ganha uma vida extra (não leva Robotnik como antes).
  - `ItemEffect.ApplyShieldFire/Lightning/Bubble()` overrides: qualquer jogador quebrando
    monitor de escudo aplica o mesmo escudo ao outro jogador também.
  - `ItemEffect.ApplyInvincibility()` override: invencibilidade aplicada aos dois jogadores
    com estrelas de invencibilidade nos dois slots de objeto (0xffffcd7c e 0xffffcea4).
  - `ItemEffect.ApplySpeedUp()` override: flag SPEED_UP e countdown aplicados aos dois.
  - `Character.CheckDiedStateEnd()` override: morte individual no co-op — vidas compartilhadas
    decrementadas, jogador morto renasce sem reiniciar a fase. P2 renasce ao lado de P1,
    P1 renasce no último checkpoint. Game over quando vidas chegam a 0.
  - `fn02d012()` override: qualquer jogador pode ativar um starpost — P2 é verificado
    por proximidade com o checkpoint após P1 usando o mesmo teste de AABB.
- Controles: nenhuma alteração necessária. O A.I.R. já suporta teclado e gamepad para
  ambos os jogadores via Options → Controls → P1/P2. `Input.getController(0)` = P1,
  `Input.getController(1)` = P2. Funciona com qualquer combinação de teclado/controle.
- LIMITAÇÃO CONHECIDA: Giant Ring (anel gigante) ainda só detecta P1 por colisão.
  Fica para iteração futura (Etapa 7 ou posterior).

### 2026-10-23 — Etapa 5 concluída (scripts, pendente compilação)
- `coop_world.lemon` criado com:
  - `shouldUnloadObjectAtPosition` override: objeto NÃO é despawnado se ainda está dentro
    da janela de câmera do P2 (check de `camera.position.x.u16.player2` = 0xffffee60).
    Objeto só sai se estiver fora das duas câmeras simultaneamente.
  - `Character.CheckAgainstMoveBorders` override: quando A0=0xffffb04a (P2) usa
    `move_area.left.player2` / `move_area.right.player2` / `move_area.bottom.current.player2`
    em vez das variáveis do P1. Mantém Y-border e bottomless-pit corretos para P2.
  - `coop.syncP2MoveArea()`: sincroniza as variáveis de move_area do P2 com as do P1
    a cada frame (eles percorrem o mesmo nível, só em ritmos diferentes).
  - `coop.teleportLaggingPlayer()`: se P2 ficou para trás do limite esquerdo avançando
    (arenas de chefe, transições de ato) ou está > 1536px à esquerda de P1, teletransporta
    para ao lado do P1 com 180 frames de invencibilidade.
  - `UpdateGameObjects()` override: chama base + sync + teleport a cada frame de gameplay
    normal (guarda `global.stage_type != 0` para pular bônus e especial).
- LIMITAÇÃO CONHECIDA: DynamicObjectsLoading.UpdateDefault continua centrado na câmera
  do P1 para SPAWN. Objetos fora da janela do P1 não serão criados mesmo que P2 os veja.
  Fixado apenas o lado do DESPAWN. Spawn dual-camera fica para iteração futura.

### 2026-10-23 — Intro video "lordmacintosh_abertura" integrada
- Sprite sheet gerado:
    `data/sprites/intro/lordmacintosh_frames.png` — 4000×2240px (10×10 grid, 400×224/frame)
    `data/sprites/intro/lordmacintosh_frames.json` — 100 entradas "lordmacintosh_frame_000"…"_099"
- Origem: `Misc/lordmacintosh_abertura.mp4` (10s, 24fps, 1280×720, H.264/AAC)
    Extraído para 10fps, 400×224 com `ffmpeg -vf "fps=10,scale=400:224,tile=10x10"`
- `scripts/coop/coop_intro.lemon`: override de `fn00076e` (SEGA logo per-frame)
    Mostra a intro quando `coop.active && !coop.intro.done`
    6 ticks/frame (60fps jogo / 10fps vídeo) → 600 ticks = 10s
    Qualquer botão no pad1 ou pad2 pula.
- `Iniciar Sonic 3 Coop.bat`: launcher imediato
    Usa `ffplay -fs -autoexit` (parte do ffmpeg, já instalado e testado — exit 0)
    Fallback: VLC → player padrão do Windows
    Após o vídeo, busca `Sonic3AIR.exe` em `Oxygen/sonic3air/build/bin/**`

### 2026-10-23 — Etapa 4 concluída (scripts, pendente compilação)
- `coop_render.lemon` criado com:
  - Override de `UpdateCamera()`: atualiza câmera P2 via `UpdateCameraPositionX_CompMode` +
    `UpdateCameraPositionY` quando `coop.active && !competition_mode.active`.
    Sincroniza `camera.foreground.*.player2` (equivalente de `fn23a03c`).
  - `coop.setupSplitScreen()`: configura tela dividida cima/baixo exatamente como
    `fn23a04a` mas a metade inferior usa source flags `0x04/0x05/0x14/0x15` (COOP_PLANE_B/A)
    e chama `Renderer.fillCoopPlane` para preencher os buffers C++ com o mapa no ponto de
    câmera do P2. Sub-tile scroll via `camPos & 7` (u16, sem sinal negativo).
  - Override de `RenderSprites()` → redireciona para `RenderSprites_CoopMode()`.
  - `RenderSprites_CoopMode()`: cópia de `RenderSprites_CompetitionMode` mas com
    `A3 = 0xffffee60` para P2 (câmera co-op, não 0xffffee68 do competition).
- Pendência: ao compilar em VS 2022, verificar se:
  1. `Renderer.setScrollOffsetH(2, 0, p2FracX)` alinha corretamente (pode precisar de
     `(8 - p2FracX) & 7` dependendo da convenção do renderer).
  2. Sub-tile V-scroll: se a imagem aparecer deslocada verticalmente ±8px, ajustar sinal.
  3. Background parallax do P2 não é tratado ainda — ambos planos B e A recebem o mesmo
     `fillCoopPlane` (foreground). Parallax real fica para iteração posterior.

### 2026-10-23 — Etapa 0 concluída
- Clonado sonic3air com `--depth 1`.
- Branch `coop` criada a partir do commit `87fed54`.
- ROM posicionada em `Oxygen/sonic3air/Sonic_Knuckles_wSonic3.bin` e adicionada ao `.gitignore`.
- SHA-256 verificado: confere.
- Pastas `coop-docs/` e `Oxygen/sonic3air/scripts/coop/` criadas.
- Próximo: compilar a base sem modificações (VS 2022).

## Referências
- Playbook: https://github.com/cranebeaker/coop-anything (AGENTS.md)
- Desmontagem S3K: https://github.com/sonicretro/skdisasm (RAM map, offsets)
- Modo Competition (tela dividida existente): `Oxygen/sonic3air/scripts/level/competition/level_competition.lemon`
- Câmera P2: `camera.position.x.player2 = 0xffffee60` (`scripts/_defines/defines_global.lemon`)
