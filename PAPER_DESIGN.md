# Line Editor on Paper: Complete Design & Logic Guide
> **Portfolio Building - Studio Course (3rd Semester) Coding Competition**  
> **Topic:** Build a Simple Line Editor in C (2 Hours, 100 Points)

This document provides the **exact text, diagrams, and code** you and your teammates should write down on your sheet of paper during the **0:10 – 1:05 Paper Design & Core Logic Phase** (worth **15 points** for Paper Design + **25 points** for Core Functionality in the grading rubric).

---

## 📝 Part 1: Header to Write on Top of Paper

```
--------------------------------------------------------------------------------
PROJECT: Command-Line Text Line Editor in C
COURSE:  Portfolio Building - Studio Course (3rd Semester)
TEAM:    1. Varun Kumar, R25EJ172
         2. Tarhat Fatima, R25EJ160
--------------------------------------------------------------------------------
```

---

## 🧠 Part 2: Problem Understanding & User Interaction

### 1. Concept:
A **line editor** (like the classic UNIX `ed`) operates on text line-by-line using 1-based line numbers. The document lives entirely in memory during editing and only touches disk on `save` and `load`.

### 2. User Command Set:
| Command | Syntax | Description |
| :--- | :--- | :--- |
| **Insert** | `insert <line_no> <text>` | Inserts text at `line_no`, shifting lines $\ge$ `line_no` down |
| **Append** | `append <text>` | Appends text at end of file (`line_no = count + 1`) |
| **Delete** | `delete <line_no>` | Removes line at `line_no`, shifting subsequent lines up |
| **Display** | `display` or `display <start> <end>` | Shows lines with line numbers formatted as `   1 | text` |
| **Save** | `save [filename.txt]` | Writes in-memory buffer to disk file |
| **Load** | `load <filename.txt>` | Clears buffer and loads lines from disk file |
| **Search** | `search <word>` | *(Bonus)* Lists all lines matching search keyword |
| **Replace** | `replace <target> <new>` | *(Bonus)* Substitutes text across lines |
| **Stats** | `stats` | *(Bonus)* Prints total lines, words, and characters |
| **Undo** | `undo` | *(Bonus)* Restores document before last edit action |
| **Quit** | `exit` / `quit` | Prompts if modified, frees memory, and exits |

---

## ⚖️ Part 3: Data Structure Choice & Justification (Rubric: 15 Pts)

Write this comparison table and justification paragraph on your paper:

### Comparison of Candidate Data Structures:

| Data Structure | Random Access by Line No. | Insertion / Deletion | Memory Overhead | Complexity & Bug Risk on Paper |
| :--- | :--- | :--- | :--- | :--- |
| **Fixed 2D Array** `char text[1000][256]` | $O(1)$ | $O(N)$ copies of 256 bytes | Wastes memory for short lines; truncates long lines | Low, but rigid & poor practice |
| **Doubly Linked List** `struct Node*` | $O(N)$ sequential traversal | $O(1)$ pointer update once node found | High ($2$ pointers per line = 16 bytes overhead) | High risk of pointer bugs (segfaults) on paper |
| **Dynamic Array of Pointers** `char **lines` ⭐ *(Chosen)* | **$O(1)$ direct indexing** | **$O(N)$ pointer shifts (extremely fast, microseconds)** | **Minimal (exact string length + 1 byte)** | **Clean, modular, safe, easily resized with `realloc`** |

### Justification to write:
> *"We choose a **Dynamic Array of String Pointers (`char **lines`)**. In a line editor, almost every command begins with line-number lookups (e.g. `display 15 30`, `delete 10`). An array provides **$O(1)$ random access** to line pointers. Unlike static 2D arrays, each line string is dynamically allocated with `malloc()` to its exact length, preventing memory waste. Pointer shifts during insertion/deletion only copy 8-byte addresses in contiguous cache memory rather than entire string contents, making it exceptionally fast and simple to implement robustly without dangling pointer bugs."*

---

## 📐 Part 4: Memory Layout Diagram (Draw This on Paper!)

Draw this box diagram on your paper to illustrate your data structure:

```
           Editor Structure (in RAM)
      +-----------------------------------+
      | count: 3                          |
      | capacity: 4                       |
      | lines: --------------------+      |
      | is_modified: 1             |      |
      +----------------------------|------+
                                   |
                                   v
             lines (Dynamic Array of char*)
             +--------+--------+--------+--------+
  0-based -> |  [0]   |  [1]   |  [2]   |  [3]   |
  1-based -> | Line 1 | Line 2 | Line 3 | (free) |
             +---|----+---|----+---|----+--------+
                 |        |        |
                 |        |        +-> [ "Third line in editor\0" ] (Heap)
                 |        +----------> [ "World of C programming\0" ] (Heap)
                 +-------------------> [ "Hello\0" ] (Heap)
```

---

## ✍️ Part 5: Core Handwritten C Logic (Hand-write this on Paper)

The rubric explicitly checks hand-written functions for **`insert`**, **`delete`**, and **`display`**. Write this clean, verified C code on your sheet:

### 1. Data Structure Definitions
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char **lines;    /* Dynamic array of pointers to lines */
    int count;       /* Number of lines currently in document */
    int capacity;    /* Allocated capacity for line pointers */
    int is_modified; /* Dirty flag for unsaved changes */
} Editor;
```

### 2. Display Function
```c
void editor_display(Editor *ed, int start, int end) {
    if (ed->count == 0) {
        printf("--- Document is empty ---\n");
        return;
    }
    /* Default range if bounds <= 0 */
    if (start <= 0) start = 1;
    if (end <= 0 || end > ed->count) end = ed->count;

    if (start > end || start > ed->count) {
        printf("Error: Invalid line range [%d, %d]\n", start, end);
        return;
    }

    printf("\n=== Document (%d lines) ===\n", ed->count);
    for (int i = start; i <= end; i++) {
        printf("%4d | %s\n", i, ed->lines[i - 1]);
    }
    printf("===========================\n");
}
```

### 3. Insert Function (With Dynamic Growth & Pointer Shifting)
```c
int editor_insert(Editor *ed, int line_num, const char *text) {
    /* Edge Case Check: Valid range is 1 to count + 1 (append) */
    if (line_num < 1 || line_num > ed->count + 1) {
        printf("Error: Line %d out of range (1 to %d)\n", line_num, ed->count + 1);
        return 0;
    }

    /* Resize dynamic array if capacity is reached */
    if (ed->count >= ed->capacity) {
        ed->capacity = (ed->capacity == 0) ? 8 : ed->capacity * 2;
        ed->lines = (char **)realloc(ed->lines, ed->capacity * sizeof(char *));
        if (!ed->lines) {
            printf("Error: Out of memory\n");
            return 0;
        }
    }

    int idx = line_num - 1; /* 1-based to 0-based index */

    /* Shift line pointers down */
    for (int i = ed->count; i > idx; i--) {
        ed->lines[i] = ed->lines[i - 1];
    }

    /* Allocate and copy line text */
    size_t len = strlen(text);
    ed->lines[idx] = (char *)malloc(len + 1);
    strcpy(ed->lines[idx], text);

    ed->count++;
    ed->is_modified = 1;
    return 1;
}
```

### 4. Delete Function (With Boundary Checks & Pointer Shifting)
```c
int editor_delete(Editor *ed, int line_num) {
    /* Edge Case Check: Empty document or out-of-range line */
    if (ed->count == 0) {
        printf("Error: Document is empty\n");
        return 0;
    }
    if (line_num < 1 || line_num > ed->count) {
        printf("Error: Line %d out of range (1 to %d)\n", line_num, ed->count);
        return 0;
    }

    int idx = line_num - 1;

    /* Free heap memory of the deleted string */
    free(ed->lines[idx]);

    /* Shift remaining line pointers up */
    for (int i = idx; i < ed->count - 1; i++) {
        ed->lines[i] = ed->lines[i + 1];
    }

    ed->count--;
    ed->is_modified = 1;
    return 1;
}
```

---

## 🛡️ Part 6: Edge Cases & Robustness (Rubric: 10 Pts)

Write down this edge-case checklist on the bottom of your paper to impress the evaluator:
1. **Empty Document:** Attempting `delete` or `display` on an empty file reports a clean warning without crashing.
2. **Boundary Insertion:** Inserting at line 1 (head insertion) and inserting at line `count + 1` (append) both work smoothly.
3. **Deleting the Only Line:** When deleting line 1 of a 1-line document, `count` decrements to 0, memory is freed, and the array remains valid.
4. **Invalid Line Numbers:** Handling `line_no <= 0` or `line_no > count + 1` safely with feedback.
5. **Dynamic Growth:** Array doubles gracefully via `realloc()` when lines exceed initial capacity.
6. **No Memory Leaks:** Every `malloc()` has a corresponding `free()`; upon `exit`, all line pointers and array buffers are released.
