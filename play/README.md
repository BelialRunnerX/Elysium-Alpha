# Elysium — browser open world

High-fidelity WebGL client for the consolidated Elysium monorepo.

## Run

```bash
python3 -m http.server 8080
```

Open `http://localhost:8080`.

## Controls

| Input | Action |
|-------|--------|
| WASD | Move |
| Mouse | Look (click to lock) |
| Space | Jump |
| Shift | Sprint |
| LMB | Mine |
| RMB | Place selected block |
| 1–8 | Hotbar |
| F | Attack |
| E | Suit / ration cycle |
| T | Ship navigation (8 worlds) |
| M | Local chart |
| Esc | Pause |

## Systems

- Eight planet classes with unique sky, fog, weather particles, and hazards
- Day/night cinematic atmosphere (ACES tone mapping + custom sky shader)
- Mining / building / registry beacon claims
- Imperial hostiles that escalate with Suspicion
- Local save in `localStorage`
