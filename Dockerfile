# ── Stage 1: Build backend (C++20 Linux binary) ──────────────────
FROM gcc:14 AS backend

WORKDIR /app

# Build dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

COPY slot/ slot/

# Compile with all necessary include paths
# Windows flags (-lws2_32, WIN32_LEAN_AND_MEAN, etc.) are omitted for Linux
RUN cd slot && \
    g++ -std=c++20 -O2 -flto \
    -I include \
    -I src \
    -I src/algorithms \
    -I src/analytics/BettingAdvisor \
    -I src/analytics/BettingEngine \
    -I src/analytics/FraudDetector \
    -I src/analytics/Leaderboard \
    -I src/analytics/ProbabilityAnalyzer \
    -I src/analytics/SearchEngine \
    -I src/api \
    -I src/core \
    -I src/core/ApplicationController \
    -I src/managers \
    -I src/models \
    -I src/storage \
    -I src/utils \
    src/main.cpp \
    src/utils/DateTime.cpp \
    src/storage/GameLogger.cpp \
    src/storage/CSVStorage.cpp \
    src/core/WeightedReel.cpp \
    src/core/SlotMachine.cpp \
    src/core/RNG.cpp \
    src/core/Reel.cpp \
    src/core/Payline.cpp \
    src/core/ApplicationController/ApplicationController.cpp \
    src/managers/PlayerManager.cpp \
    src/models/Bet.cpp \
    src/models/StrategyReport.cpp \
    src/models/SpinResult.cpp \
    src/models/Player.cpp \
    src/models/GameRecord.cpp \
    src/analytics/BettingAdvisor/BettingAdvisor.cpp \
    src/analytics/SearchEngine/SearchEngine.cpp \
    src/analytics/BettingEngine/BettingEngine.cpp \
    src/analytics/FraudDetector/RabinKarpDetector.cpp \
    src/analytics/FraudDetector/FraudDetector.cpp \
    src/analytics/Leaderboard/Leaderboard.cpp \
    src/analytics/ProbabilityAnalyzer/StrategyLab.cpp \
    src/analytics/ProbabilityAnalyzer/ProbabilityDP.cpp \
    src/analytics/ProbabilityAnalyzer/ProbabilityAnalyzer.cpp \
    src/analytics/ProbabilityAnalyzer/MarkovChain.cpp \
    src/analytics/ProbabilityAnalyzer/GamblerRuin.cpp \
    src/api/ApiServer.cpp \
    -o /app/slot-server -lpthread

# ── Stage 2: Build frontend (Vite + React) ───────────────────────
FROM node:22-alpine AS frontend

WORKDIR /build
COPY slot-ui/ slot-ui/

RUN cd slot-ui && \
    npm ci && \
    npm run build

# ── Stage 3: Runtime image ──────────────────────────────────────
FROM debian:bookworm-slim

WORKDIR /app

# Runtime libraries needed by the binary
RUN apt-get update && apt-get install -y --no-install-recommends \
    libstdc++6 \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

COPY --from=backend /app/slot-server .
COPY --from=frontend /build/slot-ui/dist slot-ui/dist

# Ensure data directory is writable
RUN mkdir -p data

EXPOSE 8080

CMD ["./slot-server"]
