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
