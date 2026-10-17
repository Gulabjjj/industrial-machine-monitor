# Industrial Machine Monitoring System

C++17 / Qt 6 Widgets desktop app that simulates monitoring of industrial machines.

## Features
- Start/stop controls per machine (plus Start All / Stop All)
- Simulated temperature and speed readings (first-order dynamics + noise)
- Automated fault detection: overheat (≥ 95 °C), overspeed (> 115 % rated), warnings at 85 °C
- Manual fault injection and fault reset
- Dashboard: machine status, fleet statistics, per-machine uptime/starts/faults/peak temp
- Timestamped, colour-coded event log with CSV export

## Architecture (OOP + signals/slots)
| Class | Role |
|---|---|
| `Machine` | Simulation model; emits `stateChanged`, `readingsChanged`, `logEvent` |
| `EventLog` | Timestamped event store; emits `entryAdded`, `cleared` |
| `MachineWidget` | Dashboard card; observes one `Machine` |
| `MainWindow` | Owns machines and `QTimer`; wires everything together |

## Build locally
```bash
# Ubuntu: sudo apt install build-essential cmake qt6-base-dev
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/IndustrialMachineMonitor
```
(Windows/macOS: `-DCMAKE_PREFIX_PATH=<path to Qt 6>`.)

## Deploy on Render
Render hosts web services/containers, not desktop windows, so the Docker image runs the
real Qt app on a virtual X display (Xvfb) and streams it to the browser with x11vnc + noVNC.

1. Push this folder to a GitHub repo.
2. Render Dashboard → **New → Blueprint** → select the repo (uses `render.yaml`),
   or **New → Web Service → Docker**.
3. (Recommended) set env var `VNC_PASSWORD` so the UI isn't public.
4. Open the Render URL; the app loads in the browser.

Test locally: `docker build -t monitor . && docker run -p 10000:10000 monitor` → http://localhost:10000
