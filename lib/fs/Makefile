# compiler and compile options
#
include ../../conf.mk
LIBS= \
	  $(shell pkg-config --libs --cflags icu-uc icu-io)
LDFLAGS=$(LIBS)

# source files and objects
SOURCES= \
	fs.cpp

OBJECTS=$(SOURCES:.cpp=.o)

# dynamic object
SHARED=../fs.ego

# targets
all: 
all: $(SOURCES) $(SHARED)

O3: CFLAGS+= -O3
O3: $(SOURCES) $(SHARED)

gprof: CFLAGS+= -O3 -pg
gprof: $(SOURCES) $(SHARED)

debug: CFLAGS+= -g
debug: $(SOURCES) $(SHARED)

$(SHARED): $(OBJECTS) 
	$(CC) -shared $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) -fPIC $(CFLAGS) $< -o $@

clean:
	-rm -f $(OBJECTS) $(SHARED) gmon.out massive.out out
