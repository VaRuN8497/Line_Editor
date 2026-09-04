# Line Editor - User Help & Command Manual (`HELP.md`)

Welcome to the **Command-Line Text Line Editor in C**. This editor operates line-by-line using 1-based line indexing.

---

## Command Reference & Usage Examples

### 1. `insert <line_no> <text>`
Inserts a new line of text at the specified line number. All existing lines from `line_no` onward are automatically shifted down.
- **Syntax:** `insert <line_number> <text to insert>`
- **Shortcut:** `i <line_number> <text>`
- **Valid Range:** `1` to `current_line_count + 1`
- **Example:**
  ```text
  editor> insert 1 First line of my notes
  [Insert] Line 1 added. Total lines: 1

  editor> insert 2 Second line
  [Insert] Line 2 added. Total lines: 2

  editor> insert 2 Inserted between 1 and 2
  [Insert] Line 2 added. Total lines: 3
  ```

---

### 2. `append <text>`
Appends a new line of text directly to the end of the document.
- **Syntax:** `append <text>`
- **Shortcut:** `a <text>`
- **Example:**
  ```text
  editor> append This line goes to the bottom
  [Insert] Line 4 added. Total lines: 4
  ```

---

### 3. `display` / `display <start> <end>`
Displays the document content with line numbers. You can view the entire document or a subset of lines.
- **Syntax:** `display` or `display <start_line> <end_line>`
- **Shortcut:** `d` or `list`
- **Example (Full Display):**
  ```text
  editor> display

  === Document Content (3 lines) ===
     1 | First line of my notes
     2 | Inserted between 1 and 2
     3 | Second line
  ===================================
  ```
- **Example (Range Display):**
  ```text
  editor> display 1 2

  === Document Content (3 lines) ===
     1 | First line of my notes
     2 | Inserted between 1 and 2
  ===================================
  ```

---

### 4. `delete <line_no>`
Removes the line at the specified line number and frees its memory. All lines below it are shifted up.
- **Syntax:** `delete <line_number>`
- **Shortcut:** `del <line_number>`
- **Valid Range:** `1` to `current_line_count`
- **Example:**
  ```text
  editor> delete 2
  [Delete] Line 2 removed. Remaining lines: 2
  ```

---

### 5. `save [filename]`
Saves all lines in memory to a text file on disk. If filename is omitted, it saves to the previously loaded/saved file.
- **Syntax:** `save [filename.txt]`
- **Shortcut:** `s [filename.txt]`
- **Example:**
  ```text
  editor*> save notes.txt
  [Save] Successfully wrote 2 lines to 'notes.txt'.
  ```
  *(Note: The `*` prompt indicator turns off once changes are saved).*

---

### 6. `load <filename>`
Clears current in-memory buffer and loads all lines from an existing file on disk.
- **Syntax:** `load <filename.txt>`
- **Shortcut:** `l <filename.txt>`
- **Example:**
  ```text
  editor> load notes.txt
  [Load] Loaded 2 lines from 'notes.txt'.
  ```

---

### 7. `search <query>` *(Bonus Feature)*
Searches across all lines for a keyword or phrase and prints every line where it appears.
- **Syntax:** `search <word or phrase>`
- **Shortcut:** `find <word>`
- **Example:**
  ```text
  editor> search notes

  --- Search Results for 'notes' ---
  Line   1: First line of my notes
  Total matches: 1 line(s)
  --------------------------------
  ```

---

### 8. `replace <find> <replacement>` *(Bonus Feature)*
Finds and replaces occurrences of a word or phrase across the entire document.
- **Syntax:** `replace <target> <replacement>`
- **Example:**
  ```text
  editor> replace notes thoughts
  [Replace] Replaced 1 occurrence(s) of 'notes' with 'thoughts'.
  ```

---

### 9. `replace_line <line_no> <find> <replacement>` *(Bonus Feature)*
Restricts text replacement to a single specific line.
- **Syntax:** `replace_line <line_no> <target> <replacement>`
- **Example:**
  ```text
  editor> replace_line 1 thoughts reflections
  [Replace] Replaced 1 occurrence(s) of 'thoughts' with 'reflections'.
  ```

---

### 10. `stats` *(Bonus Feature)*
Calculates and displays document statistics including file name, modified status, line count, word count, and character count.
- **Syntax:** `stats` or `count`
- **Example:**
  ```text
  editor> stats

  === Document Statistics ===
    File Name  : notes.txt
    Status     : Saved / Clean
    Total Lines: 2
    Total Words: 7
    Characters : 38 (excluding newlines)
  ===========================
  ```

---

### 11. `undo` *(Bonus Feature)*
Reverts the document back to the state prior to the most recent modification (`insert`, `delete`, or `replace`).
- **Syntax:** `undo` or `u`
- **Example:**
  ```text
  editor> undo
  [Undo] Successfully restored to previous state (2 lines).
  ```

---

### 12. `help`
Prints the in-terminal command cheat-sheet.
- **Syntax:** `help`

---

### 13. `exit` / `quit`
Exits the editor. If you have unsaved changes, it warns you and asks for confirmation (`y`/`n`) to prevent data loss.
- **Syntax:** `exit`, `quit`, or `q`
- **Example:**
  ```text
  editor*> quit
  Warning: You have unsaved changes. Really quit? (y/n): y
  Line editor closed. Goodbye!
  ```
