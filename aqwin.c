#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <inttypes.h>
#include <errno.h>
#include <GL/gl.h>
#include "aqwin.h"
#include "aqwin-impl.h"

enum tupletag {
	Tstring,
	Tnumber
};

typedef struct aqwin_tuple aqwin_tuple;
struct aqwin_tuple {
	char *key;
	enum tupletag tag;
	union {
		int num;
		char *val;
	};
	aqwin_tuple *next;
};

struct aqwin_param {
	char *buf;
	aqwin_tuple *attr;
};

static int isws(char);
static void eatwhite(char **);
static int charff(char **s, char c);

static int parse_attr(aqwin_tuple *);
static aqwin_param *parse_cfg(const char *cfg);
static void aqwin_param_free(aqwin_param *p);
static void aqwin_tuple_free(aqwin_tuple *t);

int complain(const char *errstr, ...) {
	int rc;
	
	va_list ap;
	va_start(ap, errstr);
	rc = vfprintf(stderr, errstr, ap);
	va_end(ap);
	return rc;
}	
static int isws(char c) {
	return (c == ' '||c == '\t'||c == '\r' ||c == '\n');
}
static void eatwhite(char **s) {
	while(isws(**s)) (*s)++;
}
static int charff(char **s, char c) {
	for(;**s;(*s)++) if (**s == c) return 1;
	return 0;
}

void print_params(aqwin_param *p) {
	aqwin_tuple *t;
	
	for(t = p->attr; t; t = t->next) {
		if(t->tag == Tnumber) {
			complain("%s: %d\n", t->key, t->num);
		} else {
			complain("%s: '%s'\n", t->key, t->val);
		}
	}
}
	
Aqwin *aqwin_open(const char *cfg) {
	Aqwin *win;
	aqwin_param *p;
	
	if ((p = parse_cfg(cfg)) == NULL) {
		complain("aqwin_open: invalid config \"%s\"\n", cfg);
		return NULL;
	}
	print_params(p);
	win = aqwin_create(p);
	aqwin_param_free(p);
	return win;
}

static aqwin_param *parse_cfg(const char *cfg) {
	char *cp;
	aqwin_param *param;
	aqwin_tuple *last;

	if ((param = malloc(sizeof *param)) == NULL) {
		complain("aqwin_open: nomem param\n");
		goto e0_nomem_param;
	}
	memset(param, 0, sizeof *param);
	if ((param->buf = malloc(strlen(cfg))) == NULL) {
		complain("aqwin_open: nomem param buf\n");
		goto e1_nomem_parambuf;
	}
	strcpy(param->buf, cfg);
	if ((param->attr = malloc(sizeof *(param->attr))) == NULL) {
		complain("aqwin_open: nomem param attr\n");
		goto e2_nomem_paramattr;
	}
	memset(param->attr, 0, sizeof *(param->attr));
	
	last = param->attr;
	
	cp = param->buf;
	for(eatwhite(&cp); *cp; cp++) {
	skip_inc:
		if(*cp == '=' && last->key == NULL) {
			complain("aqwin_open: keys cannot begin with '='\n");
			goto e3_parse_error;
		} 
		else if(*cp == '=') {
			*cp = '\0';
			last->val = cp + 1;
		}
		else if(*cp == '\'') {
			*cp++ = '\0';
			if (last->val) {
				last->val = cp;
			} else {
				complain("aqwin_open: quoted key\n");
				goto e3_parse_error;
			}
			if (charff(&cp, '\'')) {
				*cp++ = '\0';
			} else {
				complain("aqwin_open: unterminated string\n");
				goto e3_parse_error;
			}
			eatwhite(&cp);
			if((last->next = malloc(sizeof *last)) == NULL) {
				complain("aqwin_open: nomem next param\n");
				goto e3_nomem_paramnext;
			}
			last = last->next;
			memset(last, 0, sizeof *last);
			goto skip_inc;
		}
		else if(isws(*cp)) {
			*cp = '\0';
			last->val = last->val ? last->val : cp;
			if (parse_attr(last)) {
				goto e3_parse_error;
			}
			cp++;
			eatwhite(&cp);
			if((last->next = malloc(sizeof *last)) == NULL) {
				complain("aqwin_open: nomem next param\n");
				goto e3_nomem_paramnext;
			}
			last = last->next;
			memset(last, 0, sizeof *last);
			goto skip_inc;
		}
		else if(last->key == NULL) {
			last->key = cp;
		}
	}
	return param;

e3_nomem_paramnext:
e3_parse_error:
	aqwin_tuple_free(param->attr);
e2_nomem_paramattr:
	free(param->buf);
e1_nomem_parambuf:
	free(param);
e0_nomem_param:
	return NULL;
}

void aqwin_param_free(aqwin_param *param) {
	aqwin_tuple_free(param->attr);
	free(param->buf);
	free(param);
}

void aqwin_tuple_free(aqwin_tuple *head) {
	aqwin_tuple *next;
	while(head) {
		next = head->next;
		free(head);
		head = next;
	}
}

static int parse_attr(aqwin_tuple *attr) {
	int w,h,x,y;
	if(!strcmp(attr->key, "geom")) {
		if(aqwin_parsegeom(attr->val,&w,&h,&x,&y)) {
			complain("aqwin_open: invalid geometry %s\n", attr->val);
			return -1;
		}
		attr->tag = Tstring;
	}
	else if (!strcmp(attr->key, "depth")) {
		errno = 0;
		attr->num = strtol(attr->val, NULL, 10);
		if (errno) {
			perror("aqwin_open: depth");
			return -1;
		}
		attr->tag = Tnumber;
	}
	else {
		attr->tag = Tstring;
	}
	return 0;
}

int aqwin_parsegeom(const char *g, int *w, int *h, int *x, int *y) {
	if(sscanf(g, "%dx%d@%d,%d",w,h,x,y) == EOF) {
		perror("aqwin_open");
		return -1;
	}
	return 0;
}

int aqwin_params(aqwin_param *p, const char *k, const char **v) {
	aqwin_tuple *t;
	for(t = p->attr; t; t = t->next) {
		if((t->tag == Tstring && !strcmp(t->key,k))) {
			if(v) *v = t->val;
			return 1;
		}
	}
	return 0;
}

int aqwin_parami(aqwin_param *p, const char *k, int *v) {
	aqwin_tuple *t;
	for(t = p->attr; t; t = t->next) {
		if((t->tag == Tnumber && !strcmp(t->key,k))) {
			if(v) *v = t->num;
			return 1;
		}
	}
	return 0;
}

