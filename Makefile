
#compiler
CC = g++ -std=c++11
EXT := cpp
HEXT := hpp
# Executable filename
bin = invmat
# warnings and flags
RELEASEFLAGS = -O3 -mavx -march=native -DNDEBUG
DEBUGFLAGS = -O0 -g
WARN = -Wall
WNO = -Wno-comment  -Wno-sign-compare
CFLAGS = $(RELEASEFLAGS) $(WARN) $(WNO)

LIKDIR=/usr/local/likwid
LIKDIR2=/home/soft/likwid
LIKDIR3=/usr/include/likwid
LIKDIR4=/usr/include
INC := -I./include -I./Grimoire/include -I$(LIKDIR)/include  -I$(LIKDIR)/include -L$(LIKDIR1)/lib -I$(LIKDIR2)/include -L$(LIKDIR2)/lib -I$(LIKDIR3)/include -L$(LIKDIR3)/lib -I$(LIKDIR4)/include -L$(LIKDIR4)/lib

LIB := -pthread -L lib -DLIKWID_PERFMON -lm -pthread -llikwid

SRCDIR = src
INCDIR = include
BUILDDIR = obj

SRCNAMES := $(shell find $(SRCDIR) -name '*.$(EXT)' -type f -exec basename {} \;)
HNAMES := $(shell find $(INCDIR) -name '*.$(HEXT)' -type f -exec basename {} \;)

SRCS=$(wildcard $(SRCDIR)/*.$(EXT))
HEADERS=$(wildcard $(INCDIR)/*.$(HEXT))

OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SRCS:.$(EXT)=.o))

.PHONY: all pre dirs list_srcnames doc debug set_debug assembly set_assembly rebuild clean cleanBin cleanDoc cleanAll

all: pre dirs list_srcnames $(bin)

pre:

dirs:
	@mkdir -p $(BUILDDIR)
	@mkdir -p $(INCDIR)
	@mkdir -p $(SRCDIR)
	@echo SRCS
	@echo $(SRCS)
	@echo HEADERS
	@echo $(HEADERS)
	@echo OBJECTS
	@echo $(OBJECTS)

list_srcnames:
	@echo
	@echo "Found source files to compile:"
	@echo $(SRCNAMES)
	@echo "Found header files:"
	@echo $(HNAMES)
	@echo

doc:
	doxygen doxyconfig

debug: set_debug all
set_debug:
	$(eval CFLAGS = $(DEBUGFLAGS) $(WARN) $(WNO))

assembly: set_assembly all
set_assembly:
	$(eval CFLAGS = -S $(CFLAGS))

rebuild: clean all

clean:
	rm -rf  ./$(BUILDDIR)/*.o  ./$(BUILDDIR)/*.d
cleanBin:
	rm -rf  $(bin)
cleanDoc:
	rm -rf doc/
cleanAll: clean cleanBin cleanDoc


$(bin): $(OBJECTS)
	@echo 'Building target: $@'
	@echo 'Invoking Linker'
	$(CC) $^ -o "$(bin)" $(LIB) $(INC)
	@echo 'Finished building target: $@'
	@echo

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp $(INCDIR)/%.hpp
	@echo 'Building file: $<'
	@echo 'Invoking Compiler'
	@mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) $(LIB) $(INC) ${CCARGS} -c -fmessage-length=80 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo
