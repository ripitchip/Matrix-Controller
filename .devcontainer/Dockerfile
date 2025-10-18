# Base image
ARG VARIANT=debian
FROM mcr.microsoft.com/vscode/devcontainers/base:${VARIANT}

ENV DEBIAN_FRONTEND=noninteractive
ENV LC_ALL=C.UTF-8
ENV LANG=C.UTF-8

# Arguments
ARG CONTAINER_USER=vscode
ARG CONTAINER_GROUP=vscode
ARG GITHUB_TOKEN

# ------------------------------------------------------------------
# Install dependencies
# ------------------------------------------------------------------
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
    python3 python3-pip python3-venv \
    git curl udev usbutils \
    && apt-get clean -y && rm -rf /var/lib/apt/lists/* /tmp/library-scripts

# ------------------------------------------------------------------
# Set user (no dialout or sudo management)
# ------------------------------------------------------------------
# RUN adduser --disabled-password --gecos "" ${CONTAINER_USER}
# USER ${CONTAINER_USER}
WORKDIR /workspace

# ------------------------------------------------------------------
# Install PlatformIO Core
# ------------------------------------------------------------------
USER ${CONTAINER_USER}
WORKDIR /home/${CONTAINER_USER}

RUN curl -fsSL -o get-platformio.py https://raw.githubusercontent.com/platformio/platformio-core-installer/master/get-platformio.py && \
    python3 get-platformio.py && \
    rm get-platformio.py

# Automatically activate PlatformIO environment for bash sessions
RUN echo "source /home/${CONTAINER_USER}/.platformio/penv/bin/activate" >> /home/${CONTAINER_USER}/.bashrc

# ------------------------------------------------------------------
# Environment and workspace
# ------------------------------------------------------------------
ENV PATH=${PATH}:/home/${CONTAINER_USER}/.local/bin

RUN mkdir -p /workspace
WORKDIR /workspace

CMD ["/bin/bash"]
