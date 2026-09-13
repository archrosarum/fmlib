CC       := gcc
PREFIX   ?= /usr/local

SRCDIR   := src
OBJDIR   := obj
LIBDIR   := lib

TARGET   := $(LIBDIR)/liboptic.a
PCFILE   := $(LIBDIR)/optic.pc

CFLAGS   := -Wall -Wextra -O2 -I$(SRCDIR) $(shell pkg-config --cflags sdl3 sdl3-image sdl3-ttf)
LIBS     := $(shell pkg-config --libs sdl3 sdl3-image sdl3-ttf)

SRC      := $(wildcard $(SRCDIR)/*.c)
OBJ      := $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRC))

INCDIR   := $(PREFIX)/include/optic
LIBOUT   := $(PREFIX)/lib
PCOUT    := $(PREFIX)/lib/pkgconfig

all: $(TARGET)

$(TARGET): $(OBJ)
	@mkdir -p $(LIBDIR)
	ar rcs $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(PCFILE):
	@mkdir -p $(LIBDIR)
	@echo "prefix=$(PREFIX)"                                  >  $(PCFILE)
	@echo "libdir=\$${prefix}/lib"                             >> $(PCFILE)
	@echo "includedir=\$${prefix}/include/optic"               >> $(PCFILE)
	@echo ""                                                   >> $(PCFILE)
	@echo "Name: optic"                                        >> $(PCFILE)
	@echo "Description: Minimal SDL3-based windowing/UI library" >> $(PCFILE)
	@echo "Version: 0.1.0"                                     >> $(PCFILE)
	@echo "Requires: sdl3, sdl3-image, sdl3-ttf"                >> $(PCFILE)
	@echo "Libs: -L\$${libdir} -loptic"                         >> $(PCFILE)
	@echo "Cflags: -I\$${includedir}"                           >> $(PCFILE)

install: $(TARGET) $(PCFILE)
	@mkdir -p $(INCDIR)
	@mkdir -p $(LIBOUT)
	@mkdir -p $(PCOUT)
	install -m 644 $(SRCDIR)/*.h $(INCDIR)/
	install -m 644 $(TARGET) $(LIBOUT)/
	install -m 644 $(PCFILE) $(PCOUT)/
	ldconfig 2>/dev/null || true

uninstall:
	rm -f $(LIBOUT)/liboptic.a
	rm -f $(PCOUT)/optic.pc
	rm -rf $(INCDIR)

clean:
	rm -rf $(OBJDIR) $(LIBDIR)

.PHONY: all clean install uninstall