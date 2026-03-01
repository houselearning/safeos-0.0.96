#include "file_explorer.h"
#include "fs.h"
#include "window.h"
#include "gui.h"

static window_t *win;
static fs_entry_t entries[128];
static int entry_count;
static int selected_index = -1;
static int context_open = 0;
static int context_x, context_y;

void file_explorer_open(const char *path) {
    win = window_create("File Explorer - A:/", 50, 50, 600, 400);
    entry_count = fs_list("A:/", entries, 128);
}

void file_explorer_handle_event(gui_event_t *ev) {
    if (!win) return;

    if (ev->type == GUI_EVENT_MOUSE_DOWN) {
        if (ev->button == GUI_MOUSE_RIGHT) {
            context_open = 1;
            context_x = ev->x;
            context_y = ev->y;
        } else if (ev->button == GUI_MOUSE_LEFT) {
            // detect clicked entry, set selected_index
        }
    } else if (ev->type == GUI_EVENT_MOUSE_UP && context_open) {
        // if clicked on "New File" area
        //   fs_create_file("A:/newfile.txt");
        // if clicked on "Delete File" and selected_index >= 0
        //   fs_delete_file("A:/...from entries[selected_index]...");
        context_open = 0;
        entry_count = fs_list("A:/", entries, 128);
    }
}

void file_explorer_draw(void) {
    if (!win) return;
    window_begin_draw(win);
    // draw entries as rows
    // highlight selected_index
    if (context_open) {
        // draw small context menu at (context_x, context_y)
        // options: "New File", "Delete File"
    }
    window_end_draw(win);
}
