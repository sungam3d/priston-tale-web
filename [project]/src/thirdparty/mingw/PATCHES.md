# mingw-w64 headers (v12.0.0) - changes for the PristonWeb WebAssembly build

Source: https://github.com/mingw-w64/mingw-w64/archive/refs/tags/v12.0.0.tar.gz
Kept: `mingw-w64-headers/include` plus the `_*.h` helper files from `mingw-w64-headers/crt`
and `sec_api\`, excpt.h, vadefs.h. Archive kept in `thirdparty\dl`.
`_mingw.h` and `sdks/_mingw_ddk.h` generated from
their `.in` templates (MSVCRT version 0xE00, _WIN32_WINNT 0x601).

Edits (search for "PristonWeb"):
- `psdk_inc/intrin-impl.h`: the three x86 inline-assembly blocks are skipped on `__wasm__`
  (the generic `__sync`-builtin intrinsics still apply).
- `d3dtypes.h`, `d3dcaps.h`: 4-byte struct packing on `__wasm32__` too, as on x86 (MSVC layout).
- `winnt.h`: `__wasm32__` accepted where x86 alignment/cache sizes are chosen, and for the
  x86 `Interlocked*` name macros (the x86 TEB/fiber asm block stays x86-only).

The web build compiles with `-D_X86_` (32-bit x86 data structures) but never `__i386__`.
- All headers: every `_WIN32` test also accepts `__wasm__` (`#ifdef _WIN32`,
  `#ifndef _WIN32`, `defined(_WIN32)`; done by a script over 302 files). The web build does
  not define `_WIN32`/`WIN32`/`__MINGW32__` itself, so libc++, SDL and Emscripten's GL
  headers keep their normal (non-Windows) code paths.
- `winsock.h`, `winsock2.h`: `gethostname` not declared on `__wasm__` (unistd.h has it).
