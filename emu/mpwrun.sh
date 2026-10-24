#!/bin/bash
# Run a compile job in the emulated Mac and bring the results back.
# Usage (in WSL): emu/mpwrun.sh <job folder>
# The job folder holds build.mpw (an MPW script) plus its sources. Text files (.c .h .mpw) are converted to
# MacRoman with CR line endings on the way in; log.txt comes back as UTF-8. Every file the job made is copied
# back into the job folder. Basilisk II exits on its own when the job's MPW script ends (Shutdown -n).
set -e
EMU="$(cd "$(dirname "$0")" && pwd)"
JOB="$(cd "$1" && pwd)"
SHARE="$EMU/share/job"

rm -rf "$SHARE" && mkdir -p "$SHARE/.finf" "$SHARE/.rsrc"
for f in "$JOB"/*; do
    case "$f" in
        *.c|*.h|*.mpw)
            iconv -f UTF-8 -t MACINTOSH "$f" | tr '\n' '\r' > "$SHARE/$(basename "$f")"
            # Basilisk II keeps each file's Finder info in .finf/<name>. MPW only runs scripts of type TEXT,
            # so mark these as TEXT files made by the MPW Shell ('MPS ').
            { printf 'TEXTMPS '; head -c 24 /dev/zero; } > "$SHARE/.finf/$(basename "$f")" ;;
        */log.txt|*/status.txt|*/runner.txt) ;;
        *) cp "$f" "$SHARE/" ;;
    esac
    # An empty resource fork (.rsrc/<name>): without one, MPW's Duplicate fails with "file not found".
    : > "$SHARE/.rsrc/$(basename "$f")"
done

# A run that had to be killed leaves the Mac disk "not shut down properly", which stops the next boot at a
# dialog. So: mark the disk dirty while running, and if a mark is left over, start from the clean copy.
if [ -f "$EMU/.dirty" ] && [ -f "$EMU/pristine/System753.dsk" ]; then
    cp "$EMU/pristine/System753.dsk" "$EMU/System753.dsk"
fi
touch "$EMU/.dirty"
# Basilisk II's settings: the template plus this machine's paths (the ROM and disk are your own files).
{ echo "rom $EMU/Quadra630.ROM"; echo "disk $EMU/System753.dsk"; echo "extfs $EMU/share"
  cat "$EMU/basilisk_prefs.template"; } > "$EMU/basilisk_prefs"
# Basilisk II sometimes crashes right at launch; try again a few times when it dies within seconds.
for try in 1 2 3; do
    start=$(date +%s)
    timeout -s KILL "${MPW_TIMEOUT:-600}" ~/foe_tools/macemu/BasiliskII/src/Unix/BasiliskII --config "$EMU/basilisk_prefs" \
        > "$EMU/basilisk.log" 2>&1 || true
    [ $(( $(date +%s) - start )) -gt 5 ] && break
done
[ -f "$SHARE/status.txt" ] && rm -f "$EMU/.dirty"

for f in "$SHARE"/*; do
    case "$f" in
        *.c|*.h|*.mpw) ;;
        */log.txt|*/status.txt|*/runner.txt) tr '\r' '\n' < "$f" | iconv -f MACINTOSH -t UTF-8 > "$JOB/$(basename "$f")" ;;
        *) cp "$f" "$JOB/" ;;
    esac
done
[ -f "$JOB/status.txt" ] || { echo "No status.txt: the job did not run (see emu/basilisk.log)"; exit 1; }
cat "$JOB/log.txt"
echo "MPW status: $(cat "$JOB/status.txt")"
