#include "file_explorer.h"
#include "../../core/fs.h"
#include "../window.h"
#include "../gui.h"
#include "../../core/string.h"
#include "../../core/stdio.h"

static window_t *win;
static fs_entry_t entries[128];
static int entry_count;
static int selected_index = -1;

static int context_open = 0;
static int context_x, context_y;

#define ROW_HEIGHT 24
#define CONTEXT_W 120
#define CONTEXT_H 48

static void refresh_entries(void) {
    entry_count = fs_list("A:/", entries, 128);
    if (selected_index >= entry_count)
        selected_index = -1;
}

void file_explorer_open(const char *path) {
    (void)path;
    win = window_create("File Explorer - A:/", 50, 50, 600, 400);
    refresh_entries();
}

void file_explorer_handle_event(gui_event_t *ev) {
    if (!win) return;

    int list_x = win->x + 10;
    int list_y = win->y + 30;

    if (ev->type == GUI_EVENT_MOUSE_DOWN) {

        /* RIGHT CLICK → OPEN CONTEXT MENU */
        if (ev->button == GUI_MOUSE_RIGHT) {
            context_open = 1;
            context_x = ev->x;
            context_y = ev->y;
            return;
        }

        /* LEFT CLICK → SELECT ENTRY */
        if (ev->button == GUI_MOUSE_LEFT) {
            context_open = 0;

            for (int i = 0; i < entry_count; i++) {
                int ry = list_y + i * ROW_HEIGHT;
                if (gui_point_in_rect(ev->x, ev->y, list_x, ry, 560, ROW_HEIGHT)) {
                    selected_index = i;
                    return;
                }
            }

            selected_index = -1;
        }
    }

    /* CONTEXT MENU ACTIONS */
    if (ev->type == GUI_EVENT_MOUSE_UP && context_open) {
        int nx = context_x;
        int ny = context_y;

        /* NEW FILE */
        if (gui_point_in_rect(ev->x, ev->y, nx, ny, CONTEXT_W, 24)) {
            fs_create_file("A:/newfile.txt");
        }

        /* DELETE FILE */
        if (gui_point_in_rect(ev->x, ev->y, nx, ny + 24, CONTEXT_W, 24)) {
            if (selected_index >= 0) {
                char path[128];
                ksnprintf(path, sizeof(path), "A:/%s", entries[selected_index].name);
                fs_delete_file(path);
                selected_index = -1;
            }
        }

        context_open = 0;
        refresh_entries();
    }
}

void file_explorer_draw(void) {
    if (!win) return;

    window_begin_draw(win);

    int list_x = win->x + 10;
    int list_y = win->y + 30;

    /* FILE LIST */
    for (int i = 0; i < entry_count; i++) {
        int y = list_y + i * ROW_HEIGHT;

        if (i == selected_index) {
            gui_draw_rect(list_x, y, 560, ROW_HEIGHT, GUI_COLOR_HIGHLIGHT);
        }

        gui_draw_text(
            list_x + 6,
            y + 6,
            entries[i].name,
            GUI_COLOR_TEXT
        );
    }

    /* CONTEXT MENU */
    if (context_open) {
        gui_draw_rect(context_x, context_y, CONTEXT_W, CONTEXT_H, GUI_COLOR_MENU);

        gui_draw_text(context_x + 8, context_y + 6, "New File", GUI_COLOR_TEXT);
        gui_draw_text(context_x + 8, context_y + 30, "Delete File", GUI_COLOR_TEXT);
    }

    window_end_draw(win);
}