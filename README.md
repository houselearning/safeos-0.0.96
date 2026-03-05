# SafeOS (v0.0.96)

A small x86 operating system kernel with a minimal GUI and simple apps.

## Overview

SafeOS is an experimental kernel and graphical shell implemented in C/assembly. It includes a basic filesystem header, framebuffer support, a desktop GUI, and a handful of example applications (browser, calculator, notepad, spreadsheet, file explorer).

## Project structure

- `linker.ld` — linker script
- `Makefile` — build rules
- `iso/` — ISO and GRUB config (bootable image)
- `kernel/arch/x86/` — architecture-specific startup and framebuffer
- `core/` — core kernel sources (e.g. `kmain.c`, `fs.h`)
- `gui/` — GUI system, desktop and apps (`gui.c`, `desktop.c`, `cursor.c`)
- `gui/apps/` — sample applications: `browser.c`, `calculator.c`, `file_explorer.c`, `notepad.c`, `spreadsheet.c`

## Prerequisites

- GNU Make
- A C cross-compiler or native GCC capable of targeting x86 (toolchain varies by host)
- GRUB utilities and an ISO generator (`xorriso` or `genisoimage`) (if building the ISO)
- `qemu-system-x86_64` (recommended) for testing

Note: Exact toolchain names and setup depend on your host OS. On Windows you may prefer WSL or MSYS2.

## Build

From the repository root run:

```sh
make
# optionally create a bootable ISO (if supported by the Makefile)
make iso
# clean build artifacts
make clean
```

If the `Makefile` expects a specific cross-compiler prefix (e.g. `i686-elf-`), set your environment accordingly or install a matching toolchain.

## Run (QEMU)

After building, check the `iso/` directory for a generated ISO. Example run command:

```sh
qemu-system-x86_64 -cdrom iso/safeos.iso -m 512 -serial stdio
```

Adjust the ISO filename if your build outputs a different name. You can also use other emulators or write the ISO to real media at your own risk.

## Development notes

- UI code and apps live under `gui/` and `gui/apps/`.
- Kernel entry/early init is in `kernel/arch/x86/`.
- To add an app, create a new `.c` file in `gui/apps/` and register it with the desktop as appropriate.

## Contributing

Contributions are welcome. Open issues for bugs or feature requests, and send pull requests for fixes or enhancements. Keep changes focused and small. This project is a side-project and is currently not recognised as an official HouseLearning project. I find this to do in my own free time.

## License

Apache Licence 2.0
