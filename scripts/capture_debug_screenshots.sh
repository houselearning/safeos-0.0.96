#!/usr/bin/env bash
set -euo pipefail

REPO_ROOT="$PWD"
DEBUG_DIR="$REPO_ROOT/debug"
ISO="${1:-$REPO_ROOT/SafeOS-1.0.iso}"
SERIAL_LOG="$REPO_ROOT/serial_capture.log"
MON_SOCK="$REPO_ROOT/qemu-monitor.sock"
PPM_TMP="$DEBUG_DIR/safeos_screendump_$(date +%s).ppm"
rm -f "$PPM_TMP" || true
TIMEOUT="${2:-60}"

mkdir -p "$DEBUG_DIR"
: > "$SERIAL_LOG"

# Start QEMU if monitor socket not present
if [ ! -S "$MON_SOCK" ]; then
  qemu-system-i386 -cdrom "$ISO" -m 1024 \
    -monitor unix:$MON_SOCK,server,nowait \
    -serial file:$SERIAL_LOG \
    -vga std -display none -daemonize || { echo "Failed to start QEMU"; exit 1; }
  started_qemu=1
else
  started_qemu=0
fi

echo "Waiting up to $TIMEOUT seconds for 'DESKTOP OK' in $SERIAL_LOG..."
seen=0
for i in $(seq 1 "$TIMEOUT"); do
  if grep -q "DESKTOP OK" "$SERIAL_LOG" 2>/dev/null; then
    seen=1; break
  fi
  sleep 1
done
if [ "$seen" -ne 1 ]; then
  echo "Timeout waiting for DESKTOP OK - will attempt screendump anyway"
fi

send_monitor_cmd() {
  cmd="$1"
  if command -v socat >/dev/null 2>&1; then
    printf "%s\n" "$cmd" | socat - UNIX-CONNECT:"$MON_SOCK"
  elif command -v nc >/dev/null 2>&1 && nc -h 2>&1 | grep -q '\-U'; then
    printf "%s\n" "$cmd" | nc -U "$MON_SOCK"
  elif command -v python3 >/dev/null 2>&1; then
    # Fall back to a small Python writer if socat/nc are not available
    printf "%s\n" "$cmd" | python3 - "$MON_SOCK" <<'PY'
import sys, socket
cmd = sys.stdin.read()
sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
sock.connect(sys.argv[1])
sock.sendall(cmd.encode())
sock.close()
PY
  else
    echo "Require 'socat' or 'nc -U' to communicate with QEMU monitor" >&2
    return 1
  fi
}

send_monitor_cmd "screendump $PPM_TMP"

# Wait briefly for the PPM to be written by QEMU monitor
for i in {1..10}; do
  if [ -s "$PPM_TMP" ]; then break; fi
  sleep 0.2
done
if [ ! -s "$PPM_TMP" ]; then
  echo "Warning: screendump file is empty or missing: $PPM_TMP"
  # Try a direct Python monitor write as a fallback (more robust in this environment)
  if command -v python3 >/dev/null 2>&1; then
    python3 - "$MON_SOCK" "$PPM_TMP" <<'PY'
import sys, socket, time
mon = sys.argv[1]
ppm = sys.argv[2]
s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
s.connect(mon)
s.sendall(('screendump %s\n' % ppm).encode())
time.sleep(0.2)
s.close()
PY
    if [ -s "$PPM_TMP" ]; then
      echo "PPM written by python-monitor fallback"
    fi
  fi
fi

DATE_STR=$(date +"%Y%m%d-%H%M%S")
OUT_PNG="$DEBUG_DIR/image-${DATE_STR}.png"
OUT_MD="$DEBUG_DIR/image-${DATE_STR}.md"

python3 "$REPO_ROOT/scripts/ppm_to_png.py" "$PPM_TMP" "$OUT_PNG"

cat > "$OUT_MD" <<MD
# Screenshot $DATE_STR

Captured from QEMU using the monitor command 'screendump'.

- ISO: $ISO
- Serial log: $SERIAL_LOG (waited for 'DESKTOP OK')
- Monitor socket: $MON_SOCK

Conversion: PPM -> PNG using scripts/ppm_to_png.py and Pillow.
MD

echo "Saved $OUT_PNG and $OUT_MD"

exit 0
