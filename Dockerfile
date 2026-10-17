# ---- Stage 1: build ----
FROM ubuntu:24.04 AS build
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential cmake ninja-build qt6-base-dev \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /src
COPY CMakeLists.txt .
COPY src ./src
RUN cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build --parallel

# ---- Stage 2: runtime (virtual display + browser streaming) ----
FROM ubuntu:24.04
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
        libqt6widgets6 qt6-qpa-plugins fonts-dejavu-core \
        xvfb x11vnc novnc websockify procps \
    && rm -rf /var/lib/apt/lists/*
COPY --from=build /src/build/IndustrialMachineMonitor /usr/local/bin/IndustrialMachineMonitor
COPY docker/start.sh /start.sh
COPY docker/index.html /usr/share/novnc/index.html
RUN chmod +x /start.sh

ENV QT_QPA_PLATFORM=xcb \
    MONITOR_KIOSK=1 \
    DISPLAY=:0 \
    SCREEN_RES=1280x800x24
# Render injects $PORT; default for local runs
ENV PORT=10000
EXPOSE 10000
CMD ["/start.sh"]
