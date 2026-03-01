#ifndef GUI_EVENT_H
#define GUI_EVENT_H

typedef struct gui_event {
    int type;
    int x;
    int y;
    int button;
    int key;
} gui_event_t;

#endif
