# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Documentation

- [`README.md`](README.md) — project overview, hardware, system architecture
- [`code/README.md`](code/README.md) — build instructions, code structure, architecture, naming conventions

## Key Notes

- `CMakeLists.txt` is in `code/`, not the repository root — run all build commands from there
- No heap allocation in firmware; use `StaticList`, `RingBuffer`, and fixed-size arrays
