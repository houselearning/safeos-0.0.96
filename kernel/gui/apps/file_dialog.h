// file_dialog.h
typedef enum {
    FILE_DIALOG_OPEN,
    FILE_DIALOG_SAVE
} file_dialog_mode_t;

typedef void (*file_dialog_callback_t)(const char *path);

void file_dialog_open(file_dialog_mode_t mode,
                      const char *initial_path,
                      file_dialog_callback_t cb);
