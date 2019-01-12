# compiler and compile options
include ../conf.mk
AR=ar rvs
CFLAGS=-c -Wall -pedantic
LIBS= \
	-ldl -lpthread \
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
	builtin/system.cpp \
	builtin/math.cpp \
	builtin/string.cpp \
	builtin/evaluate.cpp \
	builtin/thread.cpp \
	egel.cpp

OBJECTS=$(SOURCES:.cpp=.o)

# executable
EXECUTABLE=egel

# archive
ARCHIVE=libegel.a

# shared
SHARED=libegel.so

# targets
all: CFLAGS+= -fPIC
all: $(SOURCES) $(EXECUTABLE)

O3: CFLAGS+= -fPIC -O3
O3: $(SOURCES) $(EXECUTABLE)

gprof: LDFLAGS+= -pg
gprof: CFLAGS+= -O3 -pg
gprof: $(SOURCES) $(EXECUTABLE)

debug: CFLAGS+= -g
debug: $(SOURCES) $(EXECUTABLE)

archive: CFLAGS+= -O3
archive: $(SOURCES) $(ARCHIVE)

shared: CFLAGS+= -O3 -fPIC
shared: $(SOURCES) $(SHARED)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

$(ARCHIVE): $(OBJECTS)
	$(AR) $(ARCHIVE) $(filter-out egel.o, $(OBJECTS))

$(SHARED): $(OBJECTS)
	$(CC) -shared $(filter-out egel.o, $(OBJECTS)) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	-rm -f $(OBJECTS) $(EXECUTABLE) $(ARCHIVE) $(SHARED) gmon.out massive.out out core.*

