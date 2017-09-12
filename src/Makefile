# compiler and compile options
CC=g++ --std=c++0x
AR=ar rvs
CFLAGS=-c -Wall -pedantic
LIBS= \
	-ldl \
	$(shell pkg-config --libs --cflags icu-uc icu-io)
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
	egel.cpp 

OBJECTS=$(SOURCES:.cpp=.o)

# executable
EXECUTABLE=egel

# archive
ARCHIVE=libegel.a

# shared
SHARED=libegel.so

# targets
all: CC=g++ --std=c++0x -fPIC
all: $(SOURCES) $(EXECUTABLE)

O3: CC=g++ --std=c++0x -O3 -fPIC
O3: $(SOURCES) $(EXECUTABLE)

gprof: CC=g++ --std=c++0x -O3 -pg
gprof: $(SOURCES) $(EXECUTABLE)

debug: CC=g++ --std=c++0x -g
debug: $(SOURCES) $(EXECUTABLE)

archive: CC=g++ --std=c++0x -O3
archive: $(SOURCES) $(ARCHIVE)

shared: CC=g++ --std=c++0x -O3 -fPIC
shared: $(SOURCES) $(SHARED)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

$(ARCHIVE): $(OBJECTS)
	$(AR) $(ARCHIVE) $(filter-out egel.o, $(OBJECTS))

$(SHARED): $(OBJECTS)
	$(CC) -shared $(filter-out egel.o, $(OBJECTS)) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	-rm -f $(OBJECTS) $(EXECUTABLE) $(ARCHIVE) $(SHARED) gmon.out massive.out out
