#!/bin/sh
# Run SIMH with unbuffered stdio and watch its live output for an ordered
# sequence of fixed strings.  No expect/pexpect or shell extensions required.

set -u

usage()
{
        echo "usage: $0 [-f first-seconds] [-t phase-seconds] simh ini simh-log stdout-log pattern ..." >&2
        exit 2
}

first_timeout=5
phase_timeout=2
stdbuf_cmd=${STDBUF:-stdbuf}

if ! command -v "$stdbuf_cmd" >/dev/null 2>&1; then
        echo "$0: stdbuf utility not found: $stdbuf_cmd" >&2
        exit 127
fi

while getopts 'f:t:' opt; do
        case "$opt" in
        f) first_timeout=$OPTARG ;;
        t) phase_timeout=$OPTARG ;;
        *) usage ;;
        esac
done
shift $((OPTIND - 1))

test $# -ge 5 || usage

simh=$1
ini=$2
simh_log=$3
stdout_log=$4
shift 4

pid=

cleanup()
{
        if test -n "$pid"; then
                kill -TERM "$pid" 2>/dev/null || :
                wait "$pid" 2>/dev/null || :
        fi
}

trap cleanup 0 HUP INT TERM

rm -f "$simh_log" "$stdout_log"
"$stdbuf_cmd" -o0 -e0 "$simh" "$ini" </dev/null >"$stdout_log" 2>&1 &
pid=$!

fail_phase()
{
        pattern=$1
        reason=$2

        printf '%-30s %s\n' "$pattern" "$reason" >&2

        kill -TERM "$pid" 2>/dev/null || :
        wait "$pid" 2>/dev/null || :
        pid=

        echo "--- simulator output ---" >&2
        if test -f "$stdout_log"; then
                cat "$stdout_log" >&2
        fi
        echo "--- SIMH log ---" >&2
        if test -f "$simh_log"; then
                cat "$simh_log" >&2
        fi
        exit 1
}

wait_for_phase()
{
        pattern=$1
        seconds=$2

        while test "$seconds" -gt 0; do
                if test -f "$stdout_log" && grep -F "$pattern" "$stdout_log" >/dev/null 2>&1; then
                        printf '%s\n' "$pattern"
                        return 0
                fi

                if ! kill -0 "$pid" 2>/dev/null; then
                        fail_phase "$pattern" EOF
                fi

                sleep 1
                seconds=$((seconds - 1))
        done

        if test -f "$stdout_log" && grep -F "$pattern" "$stdout_log" >/dev/null 2>&1; then
                printf '%s\n' "$pattern"
                return 0
        fi

        fail_phase "$pattern" TIMEOUT
}

timeout=$first_timeout
for pattern in "$@"; do
        wait_for_phase "$pattern" "$timeout"
        timeout=$phase_timeout
done

# The final checkpoint has been observed.  Do not wait for any later prompt
# processing or a broad simulator timeout.
kill -TERM "$pid" 2>/dev/null || :
wait "$pid" 2>/dev/null || :
pid=

exit 0
