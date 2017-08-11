# compiler and compile options
CC=g++ --std=c++0x
AR=ar rvs
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

# archive
ARCHIVE=libegel.a

# targets
all: CC=g++ --std=c++0x
all: $(SOURCES) $(EXECUTABLE)

O3: CC=g++ --std=c++0x -O3
O3: $(SOURCES) $(EXECUTABLE)

gprof: CC=g++ --std=c++0x -O3 -pg
gprof: $(SOURCES) $(EXECUTABLE)

debug: CC=g++ --std=c++0x -g
debug: $(SOURCES) $(EXECUTABLE)

strip: CC=g++ --std=c++0x -fdata-sections -ffunction-sections -Wl,--gc-sections -Wl,-s
strip: $(SOURCES) $(EXECUTABLE)

archive: CC=g++ --std=c++0x -O3
archive: $(SOURCES) $(ARCHIVE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

$(ARCHIVE): $(OBJECTS)
	$(AR) $(ARCHIVE) $(filter-out egel.o, $(OBJECTS))

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	-rm -f $(OBJECTS) $(EXECUTABLE) $(ARCHIVE) gmon.out massive.out out
