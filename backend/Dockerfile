
FROM node:20-bookworm-slim

RUN apt-get update && apt-get install -y --no-install-recommends g++ \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY backend/package*.json ./backend/

WORKDIR /app/backend

RUN npm install --omit=dev

WORKDIR /app

COPY backend/ ./backend/
COPY frontend/ ./frontend/

RUN g++ \
    backend/cpp/main.cpp \
    backend/cpp/graph.cpp \
    backend/cpp/emergency.cpp \
    backend/cpp/resource.cpp \
    backend/cpp/dispatcher.cpp \
    backend/cpp/user.cpp \
    backend/cpp/file_handler.cpp \
    backend/cpp/sha1.cpp \
    -o backend/cpp/emergency

ENV NODE_ENV=production
ENV PORT=3000

EXPOSE 3000

WORKDIR /app/backend

CMD ["node", "sever.js"]
