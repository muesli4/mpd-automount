ifeq ($(PREFIX),)
	PREFIX := /usr/local
endif

PKGS=udisks2

CXXFLAGS+=$(shell pkg-config --cflags $(PKGS)) -std=c++17
LDLIBS+=$(shell pkg-config --libs $(PKGS))

CXXFLAGS += -Wall -Wextra -Wno-unused-parameter -Werror

#CFLAGS += -O0 -g
CXXFLAGS += -O3

.PHONY: all clean distclean

all: mpd-automount

install: all
	install -d "$(DESTDIR)/$(PREFIX)/bin/"
	install -D mpd-automount mpd-automount-link.sh "$(DESTDIR)/$(PREFIX)/bin/"

clean:

distclean: clean
	rm mpd-automount
