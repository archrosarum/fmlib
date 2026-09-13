CC       := gcc
PREFIX   ?= /usr/local

SRCDIR   := src
OBJDIR   := obj
LIBDIR   := lib

TARGET   := $(LIBDIR)/libfmgui.a
PCFILE   := $(LIBDIR)/fmgui.pc

CFLAGS   := -Wall -Wextra -O2 -I$(SRCDIR) $(shell pkg-config --cflags sdl3 sdl3-ttf sdl3-image)
LIBS     := $(shell pkg-config --libs sdl3 sdl3-ttf sdl3-image) -lm

SRC      := $(wildcard $(SRCDIR)/*.c)
OBJ      := $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRC))

INCDIR   := $(PREFIX)/include/fmgui
LIBOUT   := $(PREFIX)/lib
PCOUT    := $(PREFIX)/lib/pkgconfig

all: $(TARGET)

$(TARGET): $(OBJ) Makefile
	@mkdir -p $(LIBDIR)
	rm -f $@
	ar rcs $@ $(OBJ)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(SRCDIR)/fmgui.h $(SRCDIR)/internal.h
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(PCFILE): Makefile
	@mkdir -p $(LIBDIR)
	@echo "prefix=$(PREFIX)"                                  >  $(PCFILE)
	@echo "libdir=\$${prefix}/lib"                             >> $(PCFILE)
	@echo "includedir=\$${prefix}/include/fmgui"               >> $(PCFILE)
	@echo ""                                                   >> $(PCFILE)
	@echo "Name: fmgui"                                        >> $(PCFILE)
	@echo "Description: Minimal SDL3-based windowing/UI library" >> $(PCFILE)
	@echo "Version: 0.1.0"                                     >> $(PCFILE)
	@echo "Requires: sdl3 sdl3-ttf sdl3-image"                >> $(PCFILE)
	@echo "Libs: -L\$${libdir} -lfmgui"                         >> $(PCFILE)
	@echo "Libs.private: -lm" >> $(PCFILE)
	@echo "Cflags: -I\$${includedir}"                           >> $(PCFILE)

install: $(TARGET) $(PCFILE)
	@mkdir -p $(INCDIR)
	@mkdir -p $(LIBOUT)
	@mkdir -p $(PCOUT)
	install -m 644 $(SRCDIR)/fmgui.h $(INCDIR)/
	install -m 644 $(TARGET) $(LIBOUT)/
	install -m 644 $(PCFILE) $(PCOUT)/
	ldconfig 2>/dev/null || true

uninstall:
	rm -f $(LIBOUT)/libfmgui.a
	rm -f $(PCOUT)/fmgui.pc
	rm -rf $(INCDIR)

clean:
	rm -rf $(OBJDIR) $(LIBDIR)

.PHONY: all clean install uninstall

demo: $(TARGET)
	@mkdir -p bin
	$(CC) $(CFLAGS) main.c $(TARGET) $(LIBS) -o bin/fmgui

.PHONY: demo

SANITIZERS ?= undefined

test:
	@mkdir -p bin
	$(CC) $(CFLAGS) -Werror -g -fsanitize=$(SANITIZERS) tests/safety.c $(SRC) $(LIBS) -o bin/safety
	SDL_VIDEODRIVER=dummy ./bin/safety
	$(CC) $(CFLAGS) -Werror -g -fsanitize=$(SANITIZERS) tests/text.c $(SRC) $(LIBS) -o bin/text-test
	SDL_VIDEODRIVER=dummy ./bin/text-test
	$(CC) $(CFLAGS) -Werror -g -fsanitize=$(SANITIZERS) tests/image.c $(SRC) $(LIBS) -o bin/image-test
	SDL_VIDEODRIVER=dummy ./bin/image-test
	$(CC) $(CFLAGS) -Werror -g -fsanitize=$(SANITIZERS) tests/hidpi.c $(SRC) $(LIBS) -o bin/hidpi-test
	SDL_VIDEODRIVER=dummy ./bin/hidpi-test
	$(CC) $(CFLAGS) -Werror -g -fsanitize=$(SANITIZERS) tests/window.c $(SRC) $(LIBS) -o bin/window-test
	SDL_VIDEODRIVER=dummy ./bin/window-test

	$(CC) $(CFLAGS) -Werror -g -fsanitize=$(SANITIZERS) tests/render_cache.c $(SRC) $(LIBS) -o bin/cache-test
	SDL_VIDEODRIVER=dummy ./bin/cache-test

.PHONY: test

benchmark: $(TARGET)
	@mkdir -p bin
	$(CC) $(CFLAGS) tests/performance.c $(TARGET) $(LIBS) -o bin/performance
	SDL_VIDEODRIVER=dummy ./bin/performance

.PHONY: benchmark
