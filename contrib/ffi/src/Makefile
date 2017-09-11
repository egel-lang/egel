# compiler and compile options
CC=g++ --std=c++0x
EGEL_SRC_DIR=../../../src/
LIBS= \
	-ldl \
	-licudata \
	-licui18n \
	-licuio \
	-licule \
	-liculx \
	-licutest \
	-licutu \
	-licuuc \
	-legel
CFLAGS=-c -Wall -pedantic -I$(EGEL_SRC_DIR)
LDFLAGS=$(LIBS) -L$(EGEL_SRC_DIR)

# source files and objects
SOURCES= \
	test.cpp 

OBJECTS=$(SOURCES:.cpp=.o)

# dynamic object
SHARED=../test.ego

# targets
all: CC=g++ --std=c++0x
all: $(SOURCES) $(SHARED)

O3: CC=g++ --std=c++0x -O3
O3: $(SOURCES) $(SHARED)

gprof: CC=g++ -std=c++0x -O3 -pg
gprof: $(SOURCES) $(SHARED)

debug: CC=g++ -std=c++0x -g
debug: $(SOURCES) $(SHARED)

$(SHARED): $(OBJECTS)
	$(CC) -shared $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) -fPIC $(CFLAGS) $< -o $@

clean:
	-rm -f $(OBJECTS) $(SHARED) gmon.out massive.out out
