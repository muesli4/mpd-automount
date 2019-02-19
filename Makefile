PKGS=udisks2

CXXFLAGS+=$(shell pkg-config --cflags $(PKGS)) -std=c++17
LDLIBS+=$(shell pkg-config --libs $(PKGS))

CXXFLAGS += -Wall -Wextra -Wno-unused-parameter #-Werror

#CFLAGS += -O0 -g
CXXFLAGS += -O3

.PHONY: all clean distclean

all: usermount

clean:

distclean: clean
	-rm usermount
