struct Aqwin;
typedef struct Aqwin Aqwin;

Aqwin *aqwin_open(const char *cfg);
void aqwin_close(Aqwin *);
void aqwin_swap(Aqwin *);
void aqwin_geom(Aqwin *win, int *w, int *h, int *x, int *y); 
const char *aqwin_title(Aqwin *, const char *new);
int aqwin_eventfd(Aqwin *, const char *filter);