# C Line Editor

A fast, lightweight, terminal-based text line editor written in pure C (C99 standard). Designed and built for the **Portfolio Building - Studio Course (3rd Semester) Coding Competition**.

---

## 👥 Team Information
- **Course:** Portfolio Building - Studio Course (3rd Semester)
- **Project:** Build a Simple Line Editor in C
- **Team Members:**
  - `Member 1:` [Enter Name & Student ID]
  - `Member 2:` [Enter Name & Student ID]
  - `Member 3:` [Enter Name & Student ID]

---

## 🚀 Features Implemented

This project implements **all Core Features** and **all Bonus Features** for a complete 100/100 point implementation:

### Core Features
1. **Insert a Line:** Insert text at any valid 1-based line number (`1` to `N + 1`), shifting existing lines down.
2. **Append a Line:** Quick command to append text at the end of the document.
3. **Delete a Line:** Remove any line by line number (`1` to `N`), shifting subsequent lines up and freeing heap memory.
4. **Display Document:** Print formatted lines with line numbers (`   1 | ...`), with optional `[start] [end]` range filtering.
5. **Save to File:** Write in-memory document to a standard `.txt` file on disk.
6. **Load from File:** Open and load an existing file on startup (`./editor filename.txt`) or dynamically inside the editor (`load filename.txt`).

### Bonus Features (Extra Credit)
1. **Search:** Search across all lines for a keyword or phrase, displaying matching line numbers and line content.
2. **Find & Replace:** Search-and-replace text across the whole document or on an individual line (`replace` / `replace_line`).
3. **Line Count / Word Count / Statistics:** Comprehensive document statistics showing total lines, words, and character count.
4. **Undo Last Action:** Multi-level undo stack (up to 30 history states) allowing users to revert recent insertions, deletions, or replacements.
5. **Unsaved Changes Guard:** Safety warning on `exit` if modifications were made without saving.

---

## 🏗️ Architecture & Data Structure Justification

The editor manages document lines using a **Dynamic Array of String Pointers (`char **lines`)**:
- **$O(1)$ Line Access:** Direct line lookup by index `lines[line_num - 1]`. Essential for frequent display and edit operations.
- **Efficient Memory Usage:** Each line string is dynamically allocated via `malloc` to its exact length, avoiding the memory waste of fixed-size 2D arrays.
- **Fast Shifting:** Pointer shifts during insertion/deletion move only 8-byte addresses in memory rather than large chunks of text.
- **Dynamic Growth:** The line pointer array automatically doubles its capacity via `realloc()` when the document grows.
- **Zero Memory Leaks:** All heap-allocated strings, pointer buffers, and undo snapshots are safely deallocated on exit.

---

## 🛠️ Compilation & Running

### Requirements
- Any standard C compiler supporting C99 or later (`gcc`, `clang`, or `MSVC`).

### Compile with GCC (Linux / macOS / Windows MinGW)
```bash
gcc -Wall -Wextra -O2 editor.c -o editor
```

### Run
**Start with an empty buffer:**
```bash
./editor
```
*(On Windows cmd/powershell: `editor.exe`)*

**Start by loading an existing file:**
```bash
./editor sample.txt
```

---

## 📖 Command Quick Reference

| Command | Usage | Description |
| :--- | :--- | :--- |
| `insert` / `i` | `insert <line_no> <text>` | Insert text at line number |
| `append` / `a` | `append <text>` | Append text at end |
| `delete` / `del` | `delete <line_no>` | Delete line at line number |
| `display` / `d` | `display [start] [end]` | View document with line numbers |
| `save` / `s` | `save [filename.txt]` | Save document to file |
| `load` / `l` | `load <filename.txt>` | Load document from file |
| `search` / `find` | `search <word>` | Find occurrences of keyword |
| `replace` | `replace <target> <new>` | Replace text document-wide |
| `replace_line` | `replace_line <line> <target> <new>` | Replace text on specific line |
| `stats` | `stats` | View line, word, and character counts |
| `undo` / `u` | `undo` | Revert the last change |
| `help` | `help` | Show command cheat sheet |
| `exit` / `quit` | `exit` or `quit` | Exit the editor |

For detailed examples and edge cases, see [HELP.md](file:///HELP.md).

---

## 📁 Repository Structure
```
.
├── editor.c          # Full C source code (Core + Bonus features)
├── HELP.md           # User documentation and usage examples
├── README.md         # Project overview, rubric alignment, build instructions
├── PAPER_DESIGN.md   # Paper design notes, mental model, handwritten code
├── Makefile          # Linux/macOS build script
├── build.bat         # Windows build script
├── sample.txt        # Sample file for testing
└── .gitignore        # Git ignore rules for compiled binaries
```
