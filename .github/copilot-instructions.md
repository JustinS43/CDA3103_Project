## Purpose

Short, concrete guidance for an AI agent working on this repository (student MIPS/SPIM simulator project).

## Quick summary / big picture

- This repo implements a simplified SPIM-like CPU simulator. Two code roles:
  - `spimcore.c` + `spimcore.h` — runtime, main loop, memory/register layout, interactive shell, and helpers.
  - `project.c` — the student/assignments file: key datapath functions (ALU, instruction parsing, decode, memory ops, register write, PC update) are implemented here (many are currently stubs).
- Dataflow: `main` in `spimcore.c` loads a text memory image into a word-addressed memory starting at PCINIT (0x4000), then enters an interactive loop which calls `Step()` to run the datapath. `Step()` calls the functions implemented in `project.c` in this order: fetch -> partition -> decode -> read registers -> sign extend -> ALU ops -> memory access -> write register -> PC update.

## Build & run (concrete)

Notes: the provided `CMakeLists.txt` currently lists only `project.c` as the executable source. To build and run quickly (Windows PowerShell):

1) Using CMake (recommended after updating CMakeLists to include `spimcore.c`):

```powershell
cmake -S . -B build
cmake --build build --config Release
.\build\CDA3103project.exe path\to\input.txt
```

2) Quick direct compile with gcc (no CMake change required):

```powershell
gcc -std=c11 project.c spimcore.c -o CDA3103project
.\CDA3103project.exe path\to\input.txt
```

Run flags
- The program requires one argument: an input memory image file. Optionally add `-r` as a second arg to enable redirected output mode.

Example run:

```powershell
.\CDA3103project.exe tests/example_mem.txt -r
```

## Where an AI agent should focus (concrete tasks / priorities)

1. Implement the remaining functions in `project.c` (these are the functional targets called by `Step()`):
   - `instruction_partition` — extract fields from 32-bit instruction (example: `op = instruction >> 26; r1 = (instruction >> 21) & 0x1F;`)
   - `instruction_decode` — fill `struct_controls` according to opcode (and funct for R-type). `struct_controls` is in `spimcore.h`.
   - `read_register` — read two register values from the `Reg` array passed in.
   - `sign_extend` — sign-extend 16-bit offset to 32-bit unsigned.
   - `ALU_operations` — set ALUControl from ALUOp/funct, call `ALU()`, and handle result/Zero.
   - `rw_memory` — perform memory read/write using the word addressed `Mem` array and `ALUresult` as byte address (note use of `MEM(addr)` macro in `spimcore.c`).
   - `write_register` — write `Reg` based on `RegWrite`, `RegDst`, `MemtoReg` control signals.
   - `PC_update` — update PC for branch/jump using `extended_value` (already sign-extended) and `jsec`.

2. Small, low-risk static checks before changing behavior:
   - Avoid editing `spimcore.c`'s `main()` or its interactive loop unless adding tests. `spimcore.c` is the driver; `project.c` is intended to be the student's implementation.
   - Keep types as `unsigned` for bitwise behavior; use masks when extracting fields.

## Important repository-specific conventions & gotchas

- Memory is word addressed: `MEMSIZE` is defined as `(65536 >> 2)` and `MEM(addr)` macro does `Mem[addr >> 2]`. Many functions pass/expect byte addresses — convert to word index with `addr >> 2`.
- Registers: `Reg` array contains 32 standard registers plus 4 special entries: PC, Status, LO, HI. Helper `Nreg(name)` and `RegName[]` are provided. The special entries are at indices `REGSIZE .. REGSIZE+3`.
- Constants of interest: `PCINIT = 0x4000` (where memory load begins), `SPINIT = 0xFFFC`, `GPINIT = 0xC000`.
- Interactive commands (from `Loop()` in `spimcore.c`) are essential for manual testing and debugging: `g` show control signals, `r` dump registers, `m` dump memory, `s [n]` step n cycles, `c` continue until halt, `h` show halt flag, `x/q` quit.

## Small implementation examples (safe snippets to follow)

- Extract fields (in `instruction_partition`):

```
*op = instruction >> 26;
*r1 = (instruction >> 21) & 0x1F;
*r2 = (instruction >> 16) & 0x1F;
*r3 = (instruction >> 11) & 0x1F;
*funct = instruction & 0x3F;
*offset = instruction & 0xFFFF;
*jsec = instruction & 0x03FFFFFF;
```

- Sign extend (in `sign_extend`):

```
if (offset & 0x8000) *extended_value = offset | 0xFFFF0000;
else *extended_value = offset & 0x0000FFFF;
```

## Tests & validation tips

- Create small memory files with hex words (one per line) — these are read starting at PCINIT; the file format expected by `main()` is one hex value per line.
- Use the interactive shell to step and inspect registers/memory after implementing a function.
- If builds fail, check that the compile includes both `project.c` and `spimcore.c` (linker errors or missing `main` indicate mismatched sources).

## Merge guidance (if file already exists)

- If an existing `.github/copilot-instructions.md` is present, preserve any high-value project-specific notes and merge by keeping the above sections but appending any unique examples from the pre-existing file.

## Closing / feedback

I added the most relevant facts discovered in the code: where the runtime lives (`spimcore.c`), which functions the AI should implement (`project.c`), exact build/run commands for PowerShell, and runtime/testing patterns (memory image format and interactive commands). Tell me which part you'd like expanded (e.g., a small example memory file, a suggested CMake fix, or a unit test harness) and I'll update the file.
