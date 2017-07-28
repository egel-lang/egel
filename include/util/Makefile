# compiler and compile options
CC=g++ --std=c++0x
CFLAGS=-c -Wall -pedantic
LIBS= \
	-licudata \
	-licui18n \
	-licuio \
	-licule \
	-liculx \
	-licutest \
	-licutu \
	-licuuc
LDFLAGS=$(LIBS)

# source files and objects
SOURCES= \
	util.cpp

#    utils.cpp 

OBJECTS=$(SOURCES:.cpp=.o)

# dynamic object
SHARED=../util.ego

# targets
all: CC=g++ --std=c++0x
all: $(SOURCES) $(SHARED)

O3: CC=g++ --std=c++0x -O3
O3: $(SOURCES) $(SHARED)

gprof: CC=g++ -std=c++0x -O3 -pg
gprof: $(SOURCES) $(SHARED)

debug: CC=g++ -std=c++0x -g
debug: $(SOURCES) $(SHARED)

strip: CC=g++ -std=c++0x -fdata-sections -ffunction-sections -Wl,--gc-sections -Wl,-s
strip: $(SOURCES) $(SHARED)


$(SHARED): $(OBJECTS) 
	$(CC) -shared $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) -fPIC $(CFLAGS) $< -o $@

clean:
	-rm -f $(OBJECTS) $(SHARED) gmon.out massive.out out
