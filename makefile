MAKEFLAGS += -j8 --output-sync=target --warn-undefined-variables
# '--warn-undefined-variables' doesn't work when added in this way

ifndef CXX
CXX := g++
endif
CXXFLAGS := -pipe -std=c++23 -fdiagnostics-color=always
LDFLAGS := -lsfml-system -lsfml-graphics -lsfml-window
WARNFLAGS := -Wall -Wextra -Wpedantic -fmax-errors=1


ifeq (debug, $(filter debug, $(MAKECMDGOALS)))
target_executable = circuitsym_dbg
OBJECTFILE_DIR = build/objects_dbg
CXXFLAGS += -g -Og
else
target_executable = circuitsym
OBJECTFILE_DIR = build/objects
CXXFLAGS += -O3
endif


CODEFILES := $(wildcard *.cpp)
OBJFILES := $(patsubst %.cpp,$(OBJECTFILE_DIR)/%.o, $(CODEFILES))
DEPFILES := $(OBJFILES:.o=.d)

SUBDIRS := build/objects build/objects_dbg


.PHONY: subdirs
subdirs: $(SUBDIRS)
$(SUBDIRS):
	@mkdir --verbose --parents $@
# '--parents' also prevents errors if it already exists


.DEFAULT_GOAL := ${target_executable}
${target_executable}: ${OBJFILES} | ${SUBDIRS}
	${CXX} ${CXXFLAGS} ${OBJFILES} ${WARNFLAGS} -o $@ ${LDFLAGS}


# this Makefile is added as a prerequisite to trigger rebuilds whenever it's modified
$(OBJECTFILE_DIR)/%.o: %.cpp makefile | ${SUBDIRS}
	$(CXX) $(CXXFLAGS) -MMD -c $< -o $@ ${WARNFLAGS}

.PHONY: clean
clean:
	@-rm --verbose circuitsym         2> /dev/null || true
	@-rm --verbose circuitsym_dbg     2> /dev/null || true
	@-rm --verbose build/objects*/*.o 2> /dev/null || true
	@-rm --verbose build/objects*/*.d 2> /dev/null || true
	
# prefixed '@' prevents make from echoing the command
# prefixed '-' causes make to ignore nonzero exit-codes (instead of aborting), but it still reports these errors:
# 		'make: [Makefile:24: clean] Error 1 (ignored)'
# which is why we've appended '|| true'; it ensures the exit-code is always 0, suppressing those messages
#
# 'rm' (even without verbose) will also print it's own additional error-messages: 
# 		'rm: cannot remove 'circuitsym_dbg': No such file or directory'
# so we pipe to '/dev/null' to suppress that as well


# this is required to allow 'debug' on the command line;
# otherwise, it complains: "make: *** No rule to make target 'debug'. Stop."
.PHONY: debug
debug: circuitsym_dbg


-include $(DEPFILES)
# Include the .d makefiles. The '-' at the front suppresses the errors of missing depfiles.
# Initially, all the '.d' files will be missing, and we don't want those errors to show up.
