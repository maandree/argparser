# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.  This file is offered as-is,
# without any warranty.

PREFIX = /usr
DATA = /share
LIB = /lib
INCLUDE = /include
BIN = /bin
ENV = /usr$(BIN)/env
BASHSHEBANG = $(ENV) bash
PY3SHEBANG = $(ENV) python3
PKGNAME = argparser
LICENSES = $(PREFIX)$(DATA)
PY3VERSION = 3.3
LIBPY3 = $(PREFIX)$(LIB)/python$(PY3VERSION)
LIBBASH = $(PREFIX)$(LIB)
LIBJAVA = $(PREFIX)$(LIB)
LIBC = $(PREFIX)$(LIB)
VERSION = 3.0


C_OPTIMISE = -O6
JAVA_OPTIMISE = -O

JAVAC = javac


WARN = -Wall -Wextra -pedantic -Wdouble-promotion -Wformat=2 -Winit-self -Wmissing-include-dirs  \
       -Wtrampolines -Wfloat-equal -Wshadow -Wmissing-prototypes -Wmissing-declarations          \
       -Wredundant-decls -Wnested-externs -Winline -Wno-variadic-macros -Wsync-nand              \
       -Wunsafe-loop-optimizations -Wcast-align -Wstrict-overflow -Wdeclaration-after-statement  \
       -Wundef -Wbad-function-cast -Wcast-qual -Wwrite-strings -Wlogical-op -Waggregate-return   \
       -Wstrict-prototypes -Wold-style-definition -Wpacked -Wvector-operation-performance        \
       -Wunsuffixed-float-constants -Wsuggest-attribute=const -Wsuggest-attribute=noreturn       \
       -Wsuggest-attribute=pure -Wsuggest-attribute=format -Wnormalized=nfkc -Wconversion        \
       -fstrict-aliasing -fstrict-overflow -fipa-pure-const -ftree-vrp -fstack-usage             \
       -funsafe-loop-optimizations





.PHONY: all
all: python bash java c doc

.PHONY: doc
doc: info

.PHONY: info
info: argparser.info

%.info: info/%.texinfo
	makeinfo "$<"


.PHONY: python
python: bin/argparser.py
bin/argparser.py: src/argparser.py
	@mkdir -p bin
	cp "$<" "$@"
	sed -i 's:^#!/usr/bin/env python3$$:#!$(PY3SHEBANG):' "$@"

.PHONY: bash
bash: bin/argparser.bash
bin/argparser.bash: src/argparser.bash
	@mkdir -p bin
	cp "$<" "$@"
	sed -i 's:^#!/bin/bash$$:#!$(BASHSHEBANG):' "$@"

.PHONY: java
java: bin/ArgParser.jar
bin/ArgParser.jar: src/argparser/ArgParser.java
	@mkdir -p bin
	$(JAVAC) $(JAVA_OPTIMISE) -encoding UTF-8 -cp src -s src -d bin src/argparser/ArgParser.java
	$(JAVAC) $(JAVA_OPTIMISE) -encoding UTF-8 -cp src -s src -d bin src/Test.java
	cd bin ; jar cf ArgParser.jar argparser/ArgParser*.class

.PHONY: c
c: bin/argparser.so
bin/argparser.so: src/argparser.c src/argparser.h
	@mkdir -p bin
	$(CC) $(C_OPTIMISE) -std=gnu99 $(WARN) -fPIC -c src/argparser.c -o bin/argparser.o
	$(CC) $(C_OPTIMISE) -std=gnu99 $(WARN) -shared bin/argparser.o -o bin/argparser.so
	$(CC) $(C_OPTIMISE) -std=gnu99 $(WARN) src/test.c bin/argparser.o -o bin/test



.PHONY: install
install: install-python install-bash install-java install-c install-license install-doc

.PHONY: install-python
install-python: bin/argparser.py
	install -Dm644 bin/argparser.py "$(DESTDIR)$(LIBPY3)/argparser.py"

.PHONY: install-bash
install-bash: bin/argparser.bash
	install -Dm644 bin/argparser.bash "$(DESTDIR)$(LIBBASH)/argparser.bash"

.PHONY: install-java
install-java: bin/ArgParser.jar
	install -Dm644 bin/ArgParser.jar "$(DESTDIR)$(LIBJAVA)/ArgParser.jar"

.PHONY: install-c
install-c: bin/argparser.so src/argparser.h
	install -Dm644 bin/argparser.so "$(DESTDIR)$(LIBC)/libargparser.so.$(VERSION)"
	ln -s "libargparser.so.$(VERSION)" "$(DESTDIR)$(LIBC)/libargparser.so"
	install -Dm644 src/argparser.h "$(DESTDIR)$(PREFIX)$(INCLUDE)/argparser.h"

.PHONY: install-license
install-license:
	install -d "$(DESTDIR)$(LICENSES)/$(PKGNAME)"
	install -m644 COPYING LICENSE "$(DESTDIR)$(LICENSES)/$(PKGNAME)"

.PHONY: install-doc
install-doc: install-info

.PHONY: install-info
install-info: argparser.info
	install -Dm644 argparser.info "$(DESTDIR)$(PREFIX)$(DATA)/info/$(PKGNAME).info"



.PHONY: uninstall
uninstall: uninstall-python uninstall-bash uninstall-java uninstall-c uninstall-license uninstall-doc

.PHONY: uninstall-python
uninstall-python:
	rm -- "$(DESTDIR)$(LIBPY3)/argparser.py"

.PHONY: uninstall-bash
uninstall-bash:
	rm -- "$(DESTDIR)$(LIBBASH)/argparser.bash"

.PHONY: uninstall-java
uninstall-java:
	rm -- "$(DESTDIR)$(LIBJAVA)/ArgParser.jar"

.PHONY: uninstall-c
uninstall-c:
	rm -- "$(DESTDIR)$(LIBC)/libargparser.so"
	rm -- "$(DESTDIR)$(LIBC)/libargparser.so.1.0"
	rm -- "$(DESTDIR)$(PREFIX)$(INCLUDE)/argparser.h"

.PHONY: uninstall-license
uninstall-license:
	rm -- "$(DESTDIR)$(LICENSES)/$(PKGNAME)/LICENSE"
	rm -- "$(DESTDIR)$(LICENSES)/$(PKGNAME)/COPYING"
	-rmdir -- "$(DESTDIR)$(LICENSES)/$(PKGNAME)"

.PHONY: uninstall-doc
uninstall-doc: uninstall-info

.PHONY: uninstall-info
uninstall-info:
	rm -- "$(DESTDIR)$(PREFIX)$(DATA)/info/$(PKGNAME).info"



.PHONY: clean
clean:
	-rm -r -- bin

