#!/usr/bin/env bash
set -euo pipefail

# Build the Docker image and run tests
IMAGE_NAME=shawncoin-test
CONTEXT_DIR=$(cd "$(dirname "$0")/.." && pwd)

docker build -t ${IMAGE_NAME} ${CONTEXT_DIR}/docker

docker run --rm ${IMAGE_NAME}
