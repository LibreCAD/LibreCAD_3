ARG RUNTIME_BASE=ubuntu:24.04
FROM ${RUNTIME_BASE}

# Runtime-only display/graphics libraries; no Python, Qt SDK or compiler.
RUN apt-get update && apt-get install -y --no-install-recommends \
    xvfb xauth fonts-dejavu-core libfontconfig1 libfreetype6 \
    libgl1 libegl1 libopengl0 libdbus-1-3 libxkbcommon-x11-0 \
    libxcb-cursor0 libxcb-icccm4 libxcb-keysyms1 libxcb-image0 \
    libxcb-render-util0 libxcb-xinerama0 \
    && rm -rf /var/lib/apt/lists/*

COPY --chmod=0755 smokeAppImageRuntime.sh /usr/local/bin/smokeAppImageRuntime
USER 65534:65534
WORKDIR /tmp
ENTRYPOINT ["/usr/local/bin/smokeAppImageRuntime"]
