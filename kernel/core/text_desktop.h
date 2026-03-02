#ifndef TEXT_DESKTOP_H
#define TEXT_DESKTOP_H

/* Display a minimal text-mode "desktop" using VGA text buffer at 0xB8000.
   This is used as a fallback when no linear framebuffer is available. */
void text_desktop_run(void);

#endif
