#include "input.h"
#include "gui_event.h"
#include "../arch/x86/keyboard.h"
#include "../arch/x86/mouse.h"
#include "cursor.h"

#define EVENT_QUEUE_SIZE 256

static gui_event_t event_queue[EVENT_QUEUE_SIZE];
static int queue_head = 0;
static int queue_tail = 0;

void input_init(void) {
    // Nothing special
}

void input_handle_key(uint8_t scancode, int pressed) {
    if (pressed) {
        char c = scancode_to_ascii(scancode);
        if (c && queue_tail - queue_head < EVENT_QUEUE_SIZE) {
            gui_event_t ev;
            ev.type = KEY_CHAR;
            ev.ch = c;
            event_queue[queue_tail % EVENT_QUEUE_SIZE] = ev;
            queue_tail++;
        }
    }
}

void input_handle_mouse(int dx, int dy) {
    cursor_move(dx, dy);
}

void input_handle_mouse_buttons(int buttons) {
    static int last_buttons = 0;
    int changed = buttons ^ last_buttons;
    if (changed & 1) { // left
        gui_event_t ev;
        ev.type = (buttons & 1) ? MOUSE_DOWN : MOUSE_UP;
        ev.button = 1;
        ev.x = cursor_get_x();
        ev.y = cursor_get_y();
        if (queue_tail - queue_head < EVENT_QUEUE_SIZE) {
            event_queue[queue_tail % EVENT_QUEUE_SIZE] = ev;
            queue_tail++;
        }
    }
    if (changed & 2) { // right
        gui_event_t ev;
        ev.type = (buttons & 2) ? MOUSE_DOWN : MOUSE_UP;
        ev.button = 2;
        ev.x = cursor_get_x();
        ev.y = cursor_get_y();
        if (queue_tail - queue_head < EVENT_QUEUE_SIZE) {
            event_queue[queue_tail % EVENT_QUEUE_SIZE] = ev;
            queue_tail++;
        }
    }
    last_buttons = buttons;
}

void input_dispatch(void) {
    // In gui_main_loop, call input_poll repeatedly
}

int input_poll(gui_event_t *out) {
    if (queue_head < queue_tail) {
        *out = event_queue[queue_head % EVENT_QUEUE_SIZE];
        queue_head++;
        return 1;
    }
    return 0;
}
