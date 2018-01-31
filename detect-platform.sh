#!/bin/sh
platform="$(uname -s | tr '[:upper:]' '[:lower:]')"

# check for MUSL
if [ "${platform}" = "linux" ]; then
  if ldd /bin/sh | grep -i musl >/dev/null; then
    platform=alpine
  fi
fi

# mingw is Git-Bash
if echo "${platform}" | grep -i mingw >/dev/null; then
  platform=win
fi

# map "darwin" to "macos"
if [ "${platform}" = "darwin" ]; then
  platform=macos
fi

echo "${platform}"
