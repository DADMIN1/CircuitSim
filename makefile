MAKEFLAGS += -j --output-sync=target --warn-undefined-variables

ifndef CXX
CXX := g++
endif
CXXFLAGS := -pipe -std=c++23 -fdiagnostics-color=always
LDFLAGS := -lsfml-system -lsfml-graphics -lsfml-window
WARNFLAGS := -Wall -Wextra -Wpedantic -fmax-errors=1


target_executable = circuitsym
OBJECTFILE_DIR = build/objects
CXXFLAGS += -O3

CODEFILES := $(wildcard *.cpp)
OBJFILES := $(patsubst %.cpp,$(OBJECTFILE_DIR)/%.o, $(CODEFILES))
DEPFILES := $(OBJFILES:.o=.d)

SUBDIRS := ${OBJECTFILE_DIR}


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
	@-rm --verbose  circuitsym 2> /dev/null || true
	@-rm --verbose ${OBJFILES} 2> /dev/null || true
	@-rm --verbose ${DEPFILES} 2> /dev/null || true
# prefixed '@' prevents make from echoing the command
# prefixed '-' causes make to ignore nonzero exit-codes (instead of aborting), but it still reports these errors:
# 		'make: [Makefile:24: clean] Error 1 (ignored)'
# which is why we've appended '|| true'; it ensures the exit-code is always 0, suppressing those messages
#
# 'rm' (even without verbose) will also print it's own additional error-messages: 
# 		'rm: cannot remove 'fluidsym_dbg': No such file or directory'
# so we pipe to '/dev/null' to suppress that as well


-include $(DEPFILES)
-include $(DEPFILES_IMGUI)
# Include the .d makefiles. The '-' at the front suppresses the errors of missing depfiles.
# Initially, all the '.d' files will be missing, and we don't want those errors to show up.
