C_OPTIMISE = -O6
JAVA_OPTIMISE = -O

all:
	@mkdir -p bin
	$(CC) $(C_OPTIMISE) -std=gnu99 -Wall -Wextra -pedantic -fPIC -shared src/argparser.c -o bin/argparser.so
	javac $(JAVA_OPTIMISE) -cp src -s src -d bin src/ArgParser.java
	javac $(JAVA_OPTIMISE) -cp src -s src -d bin src/Test.java
	cd bin ; jar cf ArgParser.jar ArgParser*.class

