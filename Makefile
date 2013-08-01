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
VERSION = 1.0


C_OPTIMISE = -O6
JAVA_OPTIMISE = -O

JAVAC = javac



all: python bash java c

python: bin/argparser.py
bin/argparser.py: src/argparser.py
	cp "$<" "$@"
	sed -i 's:^#!/usr/bin/env python3$$:#!$(PY3SHEBANG)":' "$@"

bash: bin/argparser.bash
bin/argparser.bash: src/argparser.bash
	cp "$<" "$@"
	sed -i 's:^#!/bin/bash$$:#!$(BASHSHEBANG)":' "$@"

java: bin/ArgParser.jar
bin/ArgParser.jar: src/argparser/ArgParser.java
	@mkdir -p bin
	$(JAVAC) $(JAVA_OPTIMISE) -cp src -s src -d bin src/argparser/ArgParser.java
	$(JAVAC) $(JAVA_OPTIMISE) -cp src -s src -d bin src/Test.java
	cd bin ; jar cf ArgParser.jar argparser/ArgParser*.class

c: bin/argparser.so
bin/argparser.so: src/argparser.c src/argparser.h
	@mkdir -p bin
	$(CC) $(C_OPTIMISE) -std=gnu99 -Wall -Wextra -pedantic -fPIC -c src/argparser.c -o bin/argparser.o
	$(CC) $(C_OPTIMISE) -std=gnu99 -Wall -Wextra -pedantic -shared bin/argparser.o -o bin/argparser.so
	$(CC) $(C_OPTIMISE) -std=gnu99 -Wall -Wextra -pedantic src/test.c bin/argparser.o -o bin/test



install: install-python install-bash install-java install-c install-license

install-python: bin/argparser.py
	install -Dm644 bin/argparser.py "$(DESTDIR)$(LIBPY3)/argparser.py"

install-bash: bin/argparser.bash
	install -Dm644 bin/argparser.bash "$(DESTDIR)$(LIBBASH)/argparser.bash"

install-java: bin/ArgParser.jar
	install -Dm644 bin/ArgParser.jar "$(DESTDIR)$(LIBJAVA)/ArgParser.jar"

install-c: bin/argparser.so src/argparser.h
	install -Dm644 bin/argparser.so "$(DESTDIR)$(LIBC)/libargparser.so.$(VERSION)"
	ln -s "libargparser.so.$(VERSION)" "$(DESTDIR)$(LIBC)/libargparser.so"
	install -Dm644 src/argparser.h "$(DESTDIR)$(PREFIX)$(INCLUDE)/argparser.h"

install-license:
	install -d "$(DESTDIR)$(LICENSES)/$(PKGNAME)"
	install -m644 COPYING LICENSE "$(DESTDIR)$(LICENSES)/$(PKGNAME)"



uninstall: uninstall-python uninstall-bash uninstall-java uninstall-c uninstall-license

uninstall-python:
	rm -- "$(DESTDIR)$(LIBPY3)/argparser.py"

uninstall-bash:
	rm -- "$(DESTDIR)$(LIBBASH)/argparser.bash"

uninstall-java:
	rm -- "$(DESTDIR)$(LIBJAVA)/ArgParser.jar"

uninstall-c:
	rm -- "$(DESTDIR)$(LIBC)/libargparser.so"
	rm -- "$(DESTDIR)$(LIBC)/libargparser.so.1.0"
	rm -- "$(DESTDIR)$(PREFIX)$(INCLUDE)/argparser.h"

uninstall-license:
	rm -- "$(DESTDIR)$(LICENSES)/$(PKGNAME)/LICENSE"
	rm -- "$(DESTDIR)$(LICENSES)/$(PKGNAME)/COPYING"
	-rmdir -- "$(DESTDIR)$(LICENSES)/$(PKGNAME)"



clean:
	-rm -r -- bin

