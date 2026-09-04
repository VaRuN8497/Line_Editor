/**
 * ============================================================================
 * Project: Command-Line Text Line Editor in C
 * Course:  Portfolio Building - Studio Course (3rd Semester)
 * Description:
 *   A robust, interactive command-line line editor. Holds document lines in
 *   memory using a dynamic array of heap-allocated strings.
 *   Implements all Core Features (Insert, Delete, Display, Save/Load) and
 *   Bonus Features (Search, Find & Replace, Document Statistics, Undo).
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define INITIAL_CAPACITY 16
#define MAX_INPUT_LEN    1024
#define MAX_UNDO_STACK   30

/* ============================================================================
 * Data Structures
 * ============================================================================
 */

/* Represents a snapshot of the document lines for Undo functionality */
typedef struct {
    char **lines;
    int count;
} DocumentSnapshot;

/* Main Editor State */
typedef struct {
    char **lines;        /* Dynamic array of string pointers */
    int count;          /* Current number of lines (0-indexed count, 1-indexed to user) */
    int capacity;       /* Allocated capacity for line pointers */
    int is_modified;    /* 1 if unsaved changes exist, 0 otherwise */
    char current_filename[256]; /* Currently loaded/saved filename */

    /* Undo stack */
    DocumentSnapshot undo_stack[MAX_UNDO_STACK];
    int undo_top;
} Editor;

/* ============================================================================
 * Helper Prototypes
 * ============================================================================
 */
void editor_init(Editor *ed);
void editor_free(Editor *ed);
void editor_save_snapshot(Editor *ed);
void editor_free_snapshot(DocumentSnapshot *snap);
void trim_newline(char *str);
char *safe_strdup(const char *s);
int count_words_in_line(const char *s);

/* Core Functions */
int  editor_insert(Editor *ed, int line_num, const char *text);
int  editor_delete(Editor *ed, int line_num);
void editor_display(const Editor *ed, int start_line, int end_line);
int  editor_save_to_file(Editor *ed, const char *filename);
int  editor_load_from_file(Editor *ed, const char *filename);

/* Bonus Functions */
int  editor_search(const Editor *ed, const char *query);
int  editor_replace(Editor *ed, const char *find_str, const char *replace_str, int line_num);
void editor_stats(const Editor *ed);
int  editor_undo(Editor *ed);
void print_help(void);

/* ============================================================================
 * Initialization & Memory Management
 * ============================================================================
 */

void editor_init(Editor *ed) {
    ed->count = 0;
    ed->capacity = INITIAL_CAPACITY;
    ed->lines = (char **)malloc(ed->capacity * sizeof(char *));
    if (!ed->lines) {
        fprintf(stderr, "Fatal Error: Memory allocation failed on startup.\n");
        exit(EXIT_FAILURE);
    }
    ed->is_modified = 0;
    ed->current_filename[0] = '\0';
    ed->undo_top = 0;
}

void editor_free(Editor *ed) {
    if (ed->lines) {
        for (int i = 0; i < ed->count; i++) {
            free(ed->lines[i]);
        }
        free(ed->lines);
        ed->lines = NULL;
    }
    ed->count = 0;
    ed->capacity = 0;

    /* Free undo snapshots */
    for (int i = 0; i < ed->undo_top; i++) {
        editor_free_snapshot(&ed->undo_stack[i]);
    }
    ed->undo_top = 0;
}

char *safe_strdup(const char *s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    char *copy = (char *)malloc(len + 1);
    if (!copy) {
        fprintf(stderr, "Error: Memory allocation failed for line string.\n");
        return NULL;
    }
    strcpy(copy, s);
    return copy;
}

void trim_newline(char *str) {
    if (!str) return;
    size_t len = strlen(str);
    while (len > 0 && (str[len - 1] == '\r' || str[len - 1] == '\n')) {
        str[--len] = '\0';
    }
}

/* ============================================================================
 * Undo Management (Bonus)
 * ============================================================================
 */

void editor_free_snapshot(DocumentSnapshot *snap) {
    if (!snap || !snap->lines) return;
    for (int i = 0; i < snap->count; i++) {
        free(snap->lines[i]);
    }
    free(snap->lines);
    snap->lines = NULL;
    snap->count = 0;
}

void editor_save_snapshot(Editor *ed) {
    /* If stack is full, shift left and discard oldest snapshot */
    if (ed->undo_top >= MAX_UNDO_STACK) {
        editor_free_snapshot(&ed->undo_stack[0]);
        for (int i = 1; i < MAX_UNDO_STACK; i++) {
            ed->undo_stack[i - 1] = ed->undo_stack[i];
        }
        ed->undo_top = MAX_UNDO_STACK - 1;
    }

    DocumentSnapshot *snap = &ed->undo_stack[ed->undo_top++];
    snap->count = ed->count;
    if (ed->count == 0) {
        snap->lines = NULL;
    } else {
        snap->lines = (char **)malloc(ed->count * sizeof(char *));
        for (int i = 0; i < ed->count; i++) {
            snap->lines[i] = safe_strdup(ed->lines[i]);
        }
    }
}

int editor_undo(Editor *ed) {
    if (ed->undo_top == 0) {
        printf("[Undo] No previous state to undo.\n");
        return 0;
    }

    /* Free current state lines */
    for (int i = 0; i < ed->count; i++) {
        free(ed->lines[i]);
    }

    /* Pop previous snapshot */
    DocumentSnapshot snap = ed->undo_stack[--ed->undo_top];
    
    /* Ensure capacity */
    while (ed->capacity < snap.count) {
        ed->capacity *= 2;
    }
    ed->lines = (char **)realloc(ed->lines, ed->capacity * sizeof(char *));
    
    ed->count = snap.count;
    for (int i = 0; i < snap.count; i++) {
        ed->lines[i] = snap.lines[i]; /* take ownership */
    }
    free(snap.lines); /* free array of pointers only */

    ed->is_modified = 1;
    printf("[Undo] Successfully restored to previous state (%d lines).\n", ed->count);
    return 1;
}

/* ============================================================================
 * Core Features
 * ============================================================================
 */

/**
 * Inserts a new line at position line_num (1-indexed).
 * Valid range: 1 <= line_num <= ed->count + 1
 * If line_num == ed->count + 1, it appends at the end.
 * Existing lines from line_num downwards are shifted right/down.
 */
int editor_insert(Editor *ed, int line_num, const char *text) {
    if (line_num < 1 || line_num > ed->count + 1) {
        printf("Error: Line number %d out of range. Valid range: 1 to %d\n", 
               line_num, ed->count + 1);
        return 0;
    }

    editor_save_snapshot(ed);

    /* Resize dynamic array if capacity reached */
    if (ed->count >= ed->capacity) {
        int new_capacity = ed->capacity * 2;
        char **new_lines = (char **)realloc(ed->lines, new_capacity * sizeof(char *));
        if (!new_lines) {
            fprintf(stderr, "Error: Failed to reallocate memory for lines.\n");
            return 0;
        }
        ed->lines = new_lines;
        ed->capacity = new_capacity;
    }

    /* Convert 1-based index to 0-based */
    int idx = line_num - 1;

    /* Shift pointers to the right */
    for (int i = ed->count; i > idx; i--) {
        ed->lines[i] = ed->lines[i - 1];
    }

    /* Allocate and assign text */
    ed->lines[idx] = safe_strdup(text ? text : "");
    ed->count++;
    ed->is_modified = 1;

    printf("[Insert] Line %d added. Total lines: %d\n", line_num, ed->count);
    return 1;
}

/**
 * Deletes the line at line_num (1-indexed).
 * Valid range: 1 <= line_num <= ed->count
 * Shifts lines below it upwards.
 */
int editor_delete(Editor *ed, int line_num) {
    if (ed->count == 0) {
        printf("Error: Document is empty. Nothing to delete.\n");
        return 0;
    }

    if (line_num < 1 || line_num > ed->count) {
        printf("Error: Line number %d out of range. Valid range: 1 to %d\n", 
               line_num, ed->count);
        return 0;
    }

    editor_save_snapshot(ed);

    int idx = line_num - 1;
    free(ed->lines[idx]);

    /* Shift lines left/up */
    for (int i = idx; i < ed->count - 1; i++) {
        ed->lines[i] = ed->lines[i + 1];
    }

    ed->count--;
    ed->is_modified = 1;

    printf("[Delete] Line %d removed. Remaining lines: %d\n", line_num, ed->count);
    return 1;
}

/**
 * Displays document lines with 1-based line numbers.
 * Supports start_line and end_line bounds (or entire file if bounds are <= 0).
 */
void editor_display(const Editor *ed, int start_line, int end_line) {
    if (ed->count == 0) {
        printf("--- (Document is currently empty) ---\n");
        return;
    }

    if (start_line <= 0) start_line = 1;
    if (end_line <= 0 || end_line > ed->count) end_line = ed->count;

    if (start_line > end_line || start_line > ed->count) {
        printf("Error: Invalid display range [%d, %d]. Total lines: %d\n", 
               start_line, end_line, ed->count);
        return;
    }

    printf("\n=== Document Content (%d lines) ===\n", ed->count);
    for (int i = start_line; i <= end_line; i++) {
        printf("%4d | %s\n", i, ed->lines[i - 1]);
    }
    printf("===================================\n\n");
}

/**
 * Saves in-memory document to a text file.
 */
int editor_save_to_file(Editor *ed, const char *filename) {
    const char *target = filename;
    if (!target || strlen(target) == 0) {
        if (strlen(ed->current_filename) > 0) {
            target = ed->current_filename;
        } else {
            printf("Error: No filename specified. Usage: save <filename.txt>\n");
            return 0;
        }
    }

    FILE *fp = fopen(target, "w");
    if (!fp) {
        perror("Error opening file for writing");
        return 0;
    }

    for (int i = 0; i < ed->count; i++) {
        fprintf(fp, "%s\n", ed->lines[i]);
    }

    fclose(fp);
    ed->is_modified = 0;
    strncpy(ed->current_filename, target, sizeof(ed->current_filename) - 1);
    ed->current_filename[sizeof(ed->current_filename) - 1] = '\0';

    printf("[Save] Successfully wrote %d lines to '%s'.\n", ed->count, target);
    return 1;
}

/**
 * Clears current buffer and loads lines from a text file.
 */
int editor_load_from_file(Editor *ed, const char *filename) {
    if (!filename || strlen(filename) == 0) {
        printf("Error: No filename specified. Usage: load <filename.txt>\n");
        return 0;
    }

    FILE *fp = fopen(filename, "r");
    if (!fp) {
        printf("Error: Could not open file '%s'. File may not exist.\n", filename);
        return 0;
    }

    editor_save_snapshot(ed);

    /* Clear existing lines */
    for (int i = 0; i < ed->count; i++) {
        free(ed->lines[i]);
    }
    ed->count = 0;

    char buffer[MAX_INPUT_LEN];
    while (fgets(buffer, sizeof(buffer), fp)) {
        trim_newline(buffer);
        if (ed->count >= ed->capacity) {
            ed->capacity *= 2;
            ed->lines = (char **)realloc(ed->lines, ed->capacity * sizeof(char *));
        }
        ed->lines[ed->count++] = safe_strdup(buffer);
    }

    fclose(fp);
    ed->is_modified = 0;
    strncpy(ed->current_filename, filename, sizeof(ed->current_filename) - 1);
    ed->current_filename[sizeof(ed->current_filename) - 1] = '\0';

    printf("[Load] Loaded %d lines from '%s'.\n", ed->count, filename);
    return 1;
}

/* ============================================================================
 * Bonus Features: Search, Replace, Stats
 * ============================================================================
 */

/**
 * Searches for a word/phrase across all lines and prints matches with line numbers.
 */
int editor_search(const Editor *ed, const char *query) {
    if (!query || strlen(query) == 0) {
        printf("Error: Search query cannot be empty. Usage: search <word>\n");
        return 0;
    }

    if (ed->count == 0) {
        printf("Document is empty.\n");
        return 0;
    }

    int matches = 0;
    printf("\n--- Search Results for '%s' ---\n", query);
    for (int i = 0; i < ed->count; i++) {
        if (strstr(ed->lines[i], query) != NULL) {
            printf("Line %3d: %s\n", i + 1, ed->lines[i]);
            matches++;
        }
    }

    if (matches == 0) {
        printf("No matches found for '%s'.\n", query);
    } else {
        printf("Total matches: %d line(s)\n", matches);
    }
    printf("--------------------------------\n\n");
    return matches;
}

/**
 * Helper to replace all occurrences of find_str with replace_str in a single string.
 * Returns newly allocated heap string.
 */
static char *replace_in_string(const char *orig, const char *find_str, const char *replace_str, int *count_out) {
    int occurrences = 0;
    size_t find_len = strlen(find_str);
    size_t rep_len = strlen(replace_str);

    const char *p = orig;
    while ((p = strstr(p, find_str)) != NULL) {
        occurrences++;
        p += find_len;
    }

    if (count_out) *count_out = occurrences;
    if (occurrences == 0) return safe_strdup(orig);

    size_t new_len = strlen(orig) + occurrences * (rep_len - find_len);
    char *result = (char *)malloc(new_len + 1);
    if (!result) return NULL;

    char *dst = result;
    p = orig;
    while (*p) {
        if (strstr(p, find_str) == p) {
            strcpy(dst, replace_str);
            dst += rep_len;
            p += find_len;
        } else {
            *dst++ = *p++;
        }
    }
    *dst = '\0';
    return result;
}

/**
 * Replaces find_str with replace_str.
 * If line_num > 0, performs replacement only on that line.
 * If line_num == 0, replaces across the whole document.
 */
int editor_replace(Editor *ed, const char *find_str, const char *replace_str, int line_num) {
    if (!find_str || strlen(find_str) == 0) {
        printf("Error: Find pattern cannot be empty. Usage: replace <target> <replacement> [line]\n");
        return 0;
    }
    if (!replace_str) replace_str = "";

    if (ed->count == 0) {
        printf("Document is empty.\n");
        return 0;
    }

    if (line_num != 0 && (line_num < 1 || line_num > ed->count)) {
        printf("Error: Line number %d out of range (1 to %d).\n", line_num, ed->count);
        return 0;
    }

    editor_save_snapshot(ed);

    int total_replacements = 0;
    int start = (line_num == 0) ? 0 : line_num - 1;
    int end   = (line_num == 0) ? ed->count - 1 : line_num - 1;

    for (int i = start; i <= end; i++) {
        int count = 0;
        char *updated = replace_in_string(ed->lines[i], find_str, replace_str, &count);
        if (count > 0) {
            free(ed->lines[i]);
            ed->lines[i] = updated;
            total_replacements += count;
        } else {
            free(updated);
        }
    }

    if (total_replacements > 0) {
        ed->is_modified = 1;
        printf("[Replace] Replaced %d occurrence(s) of '%s' with '%s'.\n", 
               total_replacements, find_str, replace_str);
    } else {
        /* No changes made, pop unused snapshot */
        if (ed->undo_top > 0) {
            editor_free_snapshot(&ed->undo_stack[--ed->undo_top]);
        }
        printf("[Replace] Zero occurrences found for '%s'. No changes made.\n", find_str);
    }

    return total_replacements;
}

int count_words_in_line(const char *s) {
    int count = 0;
    int in_word = 0;
    while (*s) {
        if (isspace((unsigned char)*s)) {
            in_word = 0;
        } else if (!in_word) {
            in_word = 1;
            count++;
        }
        s++;
    }
    return count;
}

/**
 * Calculates document statistics: total lines, words, and characters.
 */
void editor_stats(const Editor *ed) {
    long long total_words = 0;
    long long total_chars = 0;

    for (int i = 0; i < ed->count; i++) {
        total_chars += strlen(ed->lines[i]);
        total_words += count_words_in_line(ed->lines[i]);
    }

    printf("\n=== Document Statistics ===\n");
    printf("  File Name  : %s\n", strlen(ed->current_filename) ? ed->current_filename : "[Unsaved / Untitled]");
    printf("  Status     : %s\n", ed->is_modified ? "Modified (Unsaved)" : "Saved / Clean");
    printf("  Total Lines: %d\n", ed->count);
    printf("  Total Words: %lld\n", total_words);
    printf("  Characters : %lld (excluding newlines)\n", total_chars);
    printf("===========================\n\n");
}

/* ============================================================================
 * Help & Command Loop
 * ============================================================================
 */

void print_help(void) {
    printf("\n======================= LINE EDITOR COMMANDS =======================\n");
    printf(" CORE COMMANDS:\n");
    printf("   insert <line_no> <text>         : Insert text at line_no (shifts lines down)\n");
    printf("   append <text>                   : Append text to end of document\n");
    printf("   delete <line_no>                : Remove line at line_no (shifts lines up)\n");
    printf("   display                         : Display all lines with line numbers\n");
    printf("   display <start> <end>           : Display lines within range [start, end]\n");
    printf("   save [filename]                 : Save document to disk (.txt)\n");
    printf("   load <filename>                 : Load document from disk\n");
    printf("\n BONUS COMMANDS:\n");
    printf("   search <word/phrase>            : Find lines containing the query\n");
    printf("   replace <target> <new_text>     : Replace across whole document\n");
    printf("   replace_line <line> <target> <new_text> : Replace within a single line\n");
    printf("   stats                           : Show lines, word count, char count\n");
    printf("   undo                            : Revert the last insert/delete/replace\n");
    printf("\n GENERAL:\n");
    printf("   help                            : Show this help menu\n");
    printf("   exit / quit                     : Exit the line editor\n");
    printf("====================================================================\n\n");
}

int main(int argc, char *argv[]) {
    Editor ed;
    editor_init(&ed);

    printf("=======================================================\n");
    printf("       Welcome to Simple Line Editor (C99)\n");
    printf("    Portfolio Building - Studio Course (3rd Sem)\n");
    printf("=======================================================\n");
    printf("Type 'help' to view available commands.\n\n");

    /* Startup file loading if passed as argv[1] */
    if (argc > 1) {
        printf("Attempting to open file: %s ...\n", argv[1]);
        editor_load_from_file(&ed, argv[1]);
    }

    char input[MAX_INPUT_LEN];
    int running = 1;

    while (running) {
        printf("editor%s> ", ed.is_modified ? "*" : "");
        if (!fgets(input, sizeof(input), stdin)) {
            break; /* EOF encountered (Ctrl+D / Ctrl+Z) */
        }

        trim_newline(input);

        /* Skip empty input */
        char *ptr = input;
        while (*ptr && isspace((unsigned char)*ptr)) ptr++;
        if (*ptr == '\0') continue;

        /* Extract command token */
        char cmd[64] = {0};
        int offset = 0;
        if (sscanf(ptr, "%63s%n", cmd, &offset) != 1) continue;
        char *args = ptr + offset;
        while (*args && isspace((unsigned char)*args)) args++;

        /* Command dispatch */
        if (strcmp(cmd, "help") == 0) {
            print_help();
        }
        else if (strcmp(cmd, "display") == 0 || strcmp(cmd, "d") == 0 || strcmp(cmd, "list") == 0) {
            int start = -1, end = -1;
            if (sscanf(args, "%d %d", &start, &end) == 2) {
                editor_display(&ed, start, end);
            } else if (sscanf(args, "%d", &start) == 1) {
                editor_display(&ed, start, start);
            } else {
                editor_display(&ed, 1, ed.count);
            }
        }
        else if (strcmp(cmd, "insert") == 0 || strcmp(cmd, "i") == 0) {
            int line_no = 0;
            int bytes_read = 0;
            if (sscanf(args, "%d%n", &line_no, &bytes_read) == 1) {
                char *text = args + bytes_read;
                while (*text && (*text == ' ' || *text == '\t')) text++;
                editor_insert(&ed, line_no, text);
            } else {
                printf("Usage: insert <line_no> <text to insert>\n");
            }
        }
        else if (strcmp(cmd, "append") == 0 || strcmp(cmd, "a") == 0) {
            editor_insert(&ed, ed.count + 1, args);
        }
        else if (strcmp(cmd, "delete") == 0 || strcmp(cmd, "del") == 0) {
            int line_no = 0;
            if (sscanf(args, "%d", &line_no) == 1) {
                editor_delete(&ed, line_no);
            } else {
                printf("Usage: delete <line_no>\n");
            }
        }
        else if (strcmp(cmd, "save") == 0 || strcmp(cmd, "s") == 0) {
            char fname[256] = {0};
            if (sscanf(args, "%255s", fname) == 1) {
                editor_save_to_file(&ed, fname);
            } else {
                editor_save_to_file(&ed, NULL);
            }
        }
        else if (strcmp(cmd, "load") == 0 || strcmp(cmd, "l") == 0) {
            char fname[256] = {0};
            if (sscanf(args, "%255s", fname) == 1) {
                editor_load_from_file(&ed, fname);
            } else {
                printf("Usage: load <filename.txt>\n");
            }
        }
        else if (strcmp(cmd, "search") == 0 || strcmp(cmd, "find") == 0) {
            if (strlen(args) > 0) {
                editor_search(&ed, args);
            } else {
                printf("Usage: search <word or phrase>\n");
            }
        }
        else if (strcmp(cmd, "replace") == 0) {
            char target[128] = {0};
            char replacement[128] = {0};
            if (sscanf(args, "%127s %127s", target, replacement) >= 1) {
                editor_replace(&ed, target, replacement, 0);
            } else {
                printf("Usage: replace <target> <replacement>\n");
            }
        }
        else if (strcmp(cmd, "replace_line") == 0) {
            int line_no = 0;
            char target[128] = {0};
            char replacement[128] = {0};
            if (sscanf(args, "%d %127s %127s", &line_no, target, replacement) >= 2) {
                editor_replace(&ed, target, replacement, line_no);
            } else {
                printf("Usage: replace_line <line_no> <target> <replacement>\n");
            }
        }
        else if (strcmp(cmd, "stats") == 0 || strcmp(cmd, "count") == 0) {
            editor_stats(&ed);
        }
        else if (strcmp(cmd, "undo") == 0 || strcmp(cmd, "u") == 0) {
            editor_undo(&ed);
        }
        else if (strcmp(cmd, "exit") == 0 || strcmp(cmd, "quit") == 0 || strcmp(cmd, "q") == 0) {
            if (ed.is_modified) {
                printf("Warning: You have unsaved changes. Really quit? (y/n): ");
                char confirm[16];
                if (fgets(confirm, sizeof(confirm), stdin)) {
                    if (confirm[0] == 'y' || confirm[0] == 'Y') {
                        running = 0;
                    } else {
                        printf("Quit aborted.\n");
                    }
                }
            } else {
                running = 0;
            }
        }
        else {
            printf("Unknown command: '%s'. Type 'help' for available commands.\n", cmd);
        }
    }

    editor_free(&ed);
    printf("Line editor closed. Goodbye!\n");
    return 0;
}
