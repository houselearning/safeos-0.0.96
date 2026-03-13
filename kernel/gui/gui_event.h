#ifndef GUI_EVENT_H
#define GUI_EVENT_H

#define MOUSE_DOWN 1
#define MOUSE_UP 2
#define MOUSE_MOVE 3
#define KEY_DOWN 4
#define KEY_UP 5
#define KEY_CHAR 6

typedef struct gui_event {
    int type;
    int x;
    int y;
    int button;
    int key;
    int ch; /* character code for text input events */
} gui_event_t;

#endif
