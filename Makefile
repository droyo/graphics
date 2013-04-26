include config.mk

HDR = aqwin.h aqwin-impl.h
SRC = aqwin-$(GFXLIB).c aqwin.c demo.c
OBJ = $(SRC:.c=.o)
BIN = demo
DOC = LICENSE README

all: options $(BIN)

options:
	@echo Build options:
	@echo "	CC       = $(CC)"
	@echo "	CFLAGS   = $(CFLAGS)"
	@echo "	LDFLAGS  = $(LDFLAGS)"
	@echo "	PREFIX   = $(PREFIX)"
	@echo "	VERSION  = $(VERSION)"

demo: $(OBJ)
	$(CC) -o $@ $(LDFLAGS) -l$(GFXLIB) -lGL -lGLU $^
	
.c.o:
	@echo CC $<
	@$(CC) -c $(CFLAGS) $<

aqwin.o: aqwin.h aqwin-impl.h
aqwin-sdl.o: aqwin.h aqwin-impl.h
demo.o: aqwin.h

clean:
	rm -f $(OBJ) $(BIN) 

.phony: all options clean install installman dist release watch
