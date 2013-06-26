C_OPTIMISE = -O0
JAVA_OPTIMISE = -O

all:
	@mkdir -p bin
	gcc $(C_OPTIMISE) -std=gnu99 -Wall -Wextra -pedantic -fPIC -shared src/argparser.c -o bin/argparser.so
	javac $(JAVA_OPTIMISE) -cp src -s src -d bin src/ArgParser.java

