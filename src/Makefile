# compiler and compile options
CC=g++ --std=c++0x
CFLAGS=-c -Wall -pedantic
LIBS= \
	-ldl \
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
	utils.cpp \
	lexical.cpp \
	operators.cpp \
	syntactical.cpp \
	ast.cpp \
	transform.cpp \
	semantical.cpp \
	desugar.cpp \
	lift.cpp \
	emit.cpp \
	builtin.cpp \
	eval.cpp \
	egel.cpp 

OBJECTS=$(SOURCES:.cpp=.o)

# executable
EXECUTABLE=egel

# targets
all: CC=g++ --std=c++0x
all: $(SOURCES) $(EXECUTABLE)

O3: CC=g++ --std=c++0x -O3
O3: $(SOURCES) $(EXECUTABLE)

gprof: CC=g++ -std=c++0x -O3 -pg
gprof: $(SOURCES) $(EXECUTABLE)

debug: CC=g++ -std=c++0x -g
debug: $(SOURCES) $(EXECUTABLE)

strip: CC=g++ -std=c++0x -fdata-sections -ffunction-sections -Wl,--gc-sections -Wl,-s
strip: $(SOURCES) $(EXECUTABLE)


$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	-rm -f $(OBJECTS) $(EXECUTABLE) gmon.out massive.out out
