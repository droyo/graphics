/* A backend implementation must provide the following functions */
struct aqwin_param;
typedef struct aqwin_param aqwin_param;

int aqwin_params(aqwin_param *, const char *k, const char **v);
int aqwin_parami(aqwin_param *, const char *k, int *v);
Aqwin *aqwin_create(aqwin_param *);
int aqwin_parsegeom(const char *, int *w, int *h, int *x, int *y);

int complain(const char *errstr, ...);
