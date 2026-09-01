# Z80 benchmarking with `cap32 --benchmark`

This build of Caprice32 can measure the exact number of **Z80 T-states** (CPU clock
cycles, with the Gate Array wait states included, just like on real hardware) executed by
a program running inside the emulator. It is meant for comparing optimisation versions of
a CPC routine in a **deterministic, reproducible, scriptable** way: the same snapshot
always yields the same number, independent of the host machine and its load.

```sh
cap32 --benchmark engine.sna        # prints the T-state count to stdout, then exits
```

## How it works

`--benchmark` runs the emulator **headless at full speed** and counts T-states. The
emulated program tells the emulator where the measured region starts and ends by writing
to three I/O ports with `OUT`. The **command is the port** — the value written is ignored:

| Port     | Command | Effect                                                              |
|----------|---------|--------------------------------------------------------------------|
| `&FED0`  | START   | Capture the baseline T-state count.                                |
| `&FED1`  | STOP    | Print `current - baseline` (a single integer) to stdout and quit. |
| `&FED2`  | LAP     | Print a partial result to stderr and **keep running**.            |

- **stdout** carries only the integer, so it can be captured directly:
  `C=$(cap32 --benchmark engine.sna)`.
- **stderr** gets a human-readable summary, e.g.
  `[bench] 4180676 T-states = 1045169.0 us (52.26 frames @50Hz)`.
- If STOP is reached without a prior START, the count is measured from boot.

These three ports are **inert on real CPC hardware** (the Multiface 2 only answers `&FEE8`
/ `&FEEA`, and only when fitted), so a snapshot instrumented with these markers also runs
harmlessly on a real machine or an unpatched emulator — the markers simply do nothing
there.

## Z80: assembler macros

Use **inline** markers, not `CALL`/`RET` subroutines: a `CALL`+`RET` would add its own
cycles to the measured region.

```z80
;; z80_bench.asm  --  benchmark markers for caprice32 --benchmark
;; Syntax: RASM / Maxam. For other assemblers, inline the two instructions of each macro.

macro MEASURE_START
    ld bc,&FED0 : out (c),c        ;; capture baseline
mend

macro MEASURE_STOP
    ld bc,&FED1 : out (c),c        ;; print T-states to stdout and exit the emulator
mend

macro MEASURE_LAP
    ld bc,&FED2 : out (c),c        ;; print a partial result to stderr, keep running
mend
```

### Example

```z80
    di                  ; (optional) exclude the 50 Hz interrupt from the count
    MEASURE_START
    call draw_1000_lines ; the engine code under test
    MEASURE_STOP
    ei                  ; never reached: STOP quits the emulator
```

Build a `.sna` whose program counter sits at this code (or whose boot leads to it), then:

```sh
cyc_v1=$(cap32 --benchmark engine_v1.sna)
cyc_v2=$(cap32 --benchmark engine_v2.sna)
echo "v1=$cyc_v1  v2=$cyc_v2  saved=$((cyc_v1 - cyc_v2)) T-states"
```

### Multiple checkpoints with LAP

`MEASURE_LAP` reports progress without stopping. The partial counts go to stderr; the
final STOP still prints the full total to stdout.

```z80
    MEASURE_START
    call draw_top_half
    MEASURE_LAP          ; stderr: [bench] lap <n> T-states
    call draw_bottom_half
    MEASURE_STOP         ; stdout: full total
```

## BASIC: measuring a loop

The same ports work from BASIC, which is handy for a quick check without an assembler
(`OUT port,value` — the value is irrelevant, use `0`). Drive it with `--autocmd` so the
emulator types and runs the line automatically:

```sh
# Measure a 1000-iteration empty FOR/NEXT loop:
cap32 --benchmark --autocmd 'OUT &FED0,0:FOR I=1 TO 1000:NEXT:OUT &FED1,0'
# stdout -> e.g. 4180676   (interpreter overhead included; BASIC is slow)
```

You can wrap any BASIC fragment the same way:

```sh
cap32 --benchmark --autocmd 'OUT &FED0,0:CALL &8000:OUT &FED1,0'   # time a RSX / machine-code call
cap32 --benchmark --autocmd 'OUT &FED0,0:PLOT 0,0:DRAW 639,399:OUT &FED1,0'
```

Doubling the work roughly doubles the count, which is the easy sanity check that the
measurement tracks real work:

```sh
for n in 1000 2000 4000; do
  echo "$n: $(cap32 --benchmark --autocmd "OUT &FED0,0:FOR I=1 TO $n:NEXT:OUT &FED1,0")"
done
```

## Notes and caveats

- **Constant marker overhead.** The measured delta includes a small, *constant* overhead
  from the markers themselves (part of the START/STOP `OUT` plus the `ld bc,nn` setup). It
  cancels out when you compare two versions instrumented the same way — always compare
  like with like; don't read the absolute number as the pure cost of your routine.
- **Interrupts.** The 50 Hz interrupt service routine **is counted** if interrupts are
  enabled. Wrap the region in `DI` … `EI` to measure pure compute cost, or leave them on
  to measure the routine exactly as it runs in production.
- **LAP adds cycles.** Each `MEASURE_LAP` between START and STOP adds its own cycles to the
  final STOP total (unlike STOP, the emulator keeps running after a LAP).
- **No watchdog.** If STOP is never reached (a buggy snapshot), the emulator runs forever —
  wrap automated runs in a timeout, e.g. `timeout 40 cap32 --benchmark engine.sna`.
- **Determinism.** A given `.sna` produces the same count on every run and on every host.
- **T-state to time.** 1 T-state = 0.25 µs (4 MHz). One 50 Hz CPC frame is 80000 T-states.
