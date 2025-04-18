typedef struct track_list_t {
  struct track_list_t *next;
  char *file_name;
  char *track_number;
} track_list;

void sort_tracks(track_list *track_first);
track_list *get_tracks_in_dir(char *url);