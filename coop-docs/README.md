# Sonic 3 & Knuckles — Co-op local 2 jogadores

| | |
|---|---|
| **Engine** | Oxygen (Sonic 3 A.I.R. fork, GPLv3) |
| **Versões testadas** | Commit 87fed54 (A.I.R. main, out/2026) |
| **Projetos** | Este fork (`branch: coop`) |
| **QA** | none (em desenvolvimento) |
| **Última atualização** | 2026-10-23 |

## Estado atual
🔧 **Em construção.** Ambiente clonado; build original ainda não verificado nesta máquina.

## Onde as coisas estão
- Motor C++ (Oxygen): `Oxygen/oxygenengine/source/oxygen/`
- Código do jogo (Lemon scripts): `Oxygen/sonic3air/scripts/`
- Novo código co-op: `Oxygen/sonic3air/scripts/coop/`
- Modificações C++ do motor: ver etapas 1 e 9 no plano
- Documentação de engenharia: `coop-docs/NOTES.md`

## O que mudamos
_(preencher conforme cada etapa for concluída)_

## Como jogar
_(preencher após a Etapa 4 — primeira versão jogável)_

## Como compilar (Windows)
1. VS 2022 com workload "Desktop development with C++".
2. Ajustar `framework/external/get_msbuild_path.bat` para seu MSBuild.exe.
3. Rodar `framework/external/build_externals_windows.bat`.
4. Abrir `Oxygen/sonic3air/build/_vstudio/sonic3air.sln`.
5. Compilar `sonic3air` em Release.
6. Colocar a ROM em `Oxygen/sonic3air/Sonic_Knuckles_wSonic3.bin` (não incluída).

## Problemas conhecidos
_(preencher)_

## Próximos passos
- [ ] Verificar build sem modificações (Etapa 0)
- [ ] Adicionar suporte a planos A2/B2 no motor (Etapa 1)
- [ ] Seleção de personagem por jogador no Data Select (Etapa 2)
- [ ] Dois personagens quaisquer controláveis (Etapa 3)
- [ ] Tela dividida cima/baixo (Etapa 4)
- [ ] Mundo simulado para duas câmeras (Etapa 5)
- [ ] Itens compartilhados, morte, checkpoints (Etapa 6)
- [ ] Fim de fase exige os dois (Etapa 7)
- [ ] Estágios bônus em tela dividida (Etapa 8)
- [ ] Special Stage independente por jogador (Etapa 9)
