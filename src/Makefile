#
# Copyright (C) 2020 Linux Studio Plugins Project <https://lsp-plug.in/>
#           (C) 2020 Vladimir Sadovnikov <sadko4u@gmail.com>
#
# This file is part of timbre-mill
#
# timbre-mill is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
#
# timbre-mill is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with timbre-mill.  If not, see <https://www.gnu.org/licenses/>.
#
ifneq ($(VERBOSE),1)
.SILENT:
endif

MODDIR                  = $(CURDIR)/..
CONFIG                 := $(MODDIR)/.config.mk

include $(CONFIG)
include $(MODDIR)/project.mk
include $(MODDIR)/dependencies.mk
include $(MODDIR)/make/functions.mk

ifeq ($(TEST),1)
  UNIQ_DEPENDENCIES      := $(call uniq, $(DEPENDENCIES) $(TEST_DEPENDENCIES))
  DEPENDENCIES            = $(UNIQ_DEPENDENCIES)
endif

# Artifact settings
ifeq ($(BUILD_HOST),1)
  override HOST                    = HOST_
else
  override HOST                    =
endif

ARTIFACT_NAME           = $($(ARTIFACT_ID)_NAME)
ARTIFACT_VERSION        = $($(ARTIFACT_ID)_VERSION)
ARTIFACT_DEPENDENCIES   = $(addprefix $(HOST),$(DEPENDENCIES))

ARTIFACT_BIN            = $($(HOST)$(ARTIFACT_ID)_BIN)
ARTIFACT_INC            = $($(HOST)$(ARTIFACT_ID)_INC)
ARTIFACT_OBJ            = $($(HOST)$(ARTIFACT_ID)_OBJ)
ARTIFACT_OBJ_TEST       = $($(HOST)$(ARTIFACT_ID)_OBJ_TEST)
ARTIFACT_MFLAGS         = $($(HOST)$(ARTIFACT_ID)_MFLAGS) $(foreach dep,$(DEPENDENCIES),-DUSE_$(dep))

ARTIFACT_TEST_BIN       = $(ARTIFACT_BIN)/$(ARTIFACT_NAME)-test$(EXECUTABLE_EXT)
ARTIFACT_EXE            = $(ARTIFACT_BIN)/$(ARTIFACT_NAME)-$(ARTIFACT_VERSION)$(EXECUTABLE_EXT)
ARTIFACT_EXELINK        = $(ARTIFACT_NAME)$(EXECUTABLE_EXT)
ARTIFACT_DEPS           = $(call dquery, OBJ, $(ARTIFACT_DEPENDENCIES))
ARTIFACT_CFLAGS         = $(call query, CFLAGS, $(ARTIFACT_DEPENDENCIES) $(HOST)$(ARTIFACT_ID))
ARTIFACT_LDFLAGS        = $(call query, LDFLAGS, $(ARTIFACT_DEPENDENCIES) $(HOST)$(ARTIFACT_ID))
ARTIFACT_OBJFILES       = $(call query, OBJ, $(ARTIFACT_DEPENDENCIES) $(HOST)$(ARTIFACT_ID))

ARTIFACT_TARGETS        = $(ARTIFACT_EXE)

# Source code
CXX_SRC_MAIN            = $(filter-out main/main.cpp,$(call rwildcard, main, *.cpp))
CXX_SRC_TEST            = $(call rwildcard, test, *.cpp)
CXX_SRC_NOTEST          =
CXX_SRC_EXT             =
CXX_SRC_NOTEST          = main/main.cpp  
CXX_SRC                 = $(CXX_SRC_MAIN) $(CXX_SRC_EXT)

CXX_OBJ_MAIN            = $(patsubst %.cpp, $(ARTIFACT_BIN)/%.o, $(CXX_SRC_MAIN))
CXX_OBJ_EXPORT          = $(patsubst %.cpp, $(ARTIFACT_BIN)/%.o, $(CXX_SRC_EXPORT))
CXX_OBJ_TEST            = $(patsubst %.cpp, $(ARTIFACT_BIN)/%.o, $(CXX_SRC_TEST))
CXX_OBJ_NOTEST          = $(patsubst %.cpp, $(ARTIFACT_BIN)/%.o, $(CXX_SRC_NOTEST))
CXX_OBJ_EXT             = $(patsubst %.cpp, $(ARTIFACT_BIN)/%.o, $(CXX_SRC_EXT))
CXX_OBJ                 = $(CXX_OBJ_MAIN) $(CXX_OBJ_EXT)

ALL_HEADERS             = $(call rwildcard, $(ARTIFACT_INC), *.h)
CXX_FILE                = $(patsubst $(ARTIFACT_BIN)/%.o,%.cpp, $(@))
CXX_HDR_PATHS           = $(foreach hdr,$(ARTIFACT_HEADERS),$(ARTIFACT_INC)/$(hdr))
CXX_HEADERS             = $(foreach path,$(CXX_HDR_PATHS),$(call rwildcard, $(path), *.h))
CXX_INSTHEADERS         = $(patsubst $(ARTIFACT_INC)/%,$(DESTDIR)$(INCDIR)/%,$(CXX_HEADERS))
CXX_INSTALL_HEADERS     = 

ifeq ($(HEADERS),1)
  CXX_INSTALL_HEADERS     = 1
endif

BUILD_ALL               = $(ARTIFACT_LIB) $(ARTIFACT_SLIB) $(ARTIFACT_PC)

ifeq ($($(ARTIFACT_ID)_TESTING),1)
  ARTIFACT_TARGETS       += $(ARTIFACT_TEST_BIN)
endif

DEP_CXX                 = $(foreach src,$(CXX_SRC_MAIN) $(CXX_SRC_EXPORT) $(CXX_SRC_EXT) $(CXX_SRC_TEST),$(patsubst %.cpp,$(ARTIFACT_BIN)/%.d,$(src)))
DEP_CXX_FILE            = $(patsubst $(ARTIFACT_BIN)/%.d,%.cpp,$(@))
DEP_DEP_FILE            = $(patsubst $(ARTIFACT_BIN)/%.d,%.o,$(@))

.DEFAULT_GOAL = all
.PHONY: compile depend dep_clean all install uninstall
.PHONY: $(ARTIFACT_DEPS)

# Dependencies
dep_clean:

$(DEP_CXX): dep_clean
	echo "  dep  [$(ARTIFACT_NAME)] $(DEP_CXX_FILE)"
	mkdir -p $(dir $(@))
	$(CXX) -MM -MT "\$$($(ARTIFACT_ID)_BIN)/$(DEP_DEP_FILE)" -MF $(@) $(DEP_CXX_FILE) $(CXXFLAGS) $(INCLUDE) $(ARTIFACT_CFLAGS)

depend: $(DEP_CXX)
	$(foreach dep,$(ARTIFACT_DEPENDENCIES) $(HOST)$(ARTIFACT_ID),\
	  $(if $($(dep)_INC), \
	    $(eval SED_RULES += s/$(shell echo "$($(dep)_INC)" | sed "s/\\//\\\\\//g")/\$$$$\\($(dep)_INC\\)/g;)\
	  )\
	  $(if $($(dep)_BIN), \
	    $(eval SED_RULES += s/$(shell echo "$($(dep)_BIN)" | sed "s/\\//\\\\\//g")/\\$$$$\\($(dep)_BIN\\)/g;)\
	  )\
	)
	cat $(DEP_CXX) | sed -E "$(SED_RULES)" >Makefile.d

$(ARTIFACT_DEPS):
	echo "make $(notdir $($(@)_OBJ))"
	$(MAKE) -C "$($(@)_PATH)" compile VERBOSE="$(VERBOSE)" CONFIG="$(CONFIG)" DEMO_TEST="0" BUILD_HOST="$(BUILD_HOST)"

# Compilation
compile: $(ARTIFACT_OBJ)

$(CXX_OBJ_MAIN) $(CXX_OBJ_EXPORT) $(CXX_OBJ_EXT) $(CXX_OBJ_TEST) $(CXX_OBJ_NOTEST):
	echo "  $($(HOST)CXX)  [$(ARTIFACT_NAME)] $(CXX_FILE)"
	mkdir -p $(dir $@)
	$($(HOST)CXX) -o $(@) -c $(CXX_FILE) -fPIC $($(HOST)CXXFLAGS) $(ARTIFACT_MFLAGS) $($(HOST)EXT_FLAGS) $(INCLUDE) $(ARTIFACT_CFLAGS)
	
# Producing large object files
$(ARTIFACT_OBJ): $(CXX_OBJ)
	@echo "  $($(HOST)LD)   [$(ARTIFACT_NAME)] $(notdir $(ARTIFACT_OBJ))"
	@$($(HOST)LD) -o $(ARTIFACT_OBJ) $($(HOST)LDFLAGS) $(CXX_OBJ)

$(ARTIFACT_OBJ_TEST): $(CXX_OBJ_TEST)
	echo "  $($(HOST)LD)   [$(ARTIFACT_NAME)] $(notdir $(ARTIFACT_OBJ_TEST))"
	$($(HOST)LD) -o $(ARTIFACT_OBJ_TEST) $($(HOST)LDFLAGS) $(CXX_OBJ_TEST)

# Linking
all: $(ARTIFACT_TARGETS)

$(ARTIFACT_EXE): $(ARTIFACT_DEPS) $(ARTIFACT_OBJ) $(CXX_OBJ_EXPORT) $(CXX_OBJ_NOTEST)
	echo "  $($(HOST)CXX)  [$(ARTIFACT_NAME)] $(notdir $(ARTIFACT_EXE))"
	$($(HOST)CXX) -o $(ARTIFACT_EXE) $(ARTIFACT_OBJFILES) $(CXX_OBJ_EXPORT) $(CXX_OBJ_NOTEST) $($(HOST)EXE_FLAGS) $(ARTIFACT_LDFLAGS)
	
$(ARTIFACT_TEST_BIN): $(ARTIFACT_DEPS) $(ARTIFACT_OBJ) $(ARTIFACT_OBJ_TEST)
	echo "  $($(HOST)CXX)  [$(ARTIFACT_NAME)] $(notdir $(ARTIFACT_TEST_BIN))"
	$($(HOST)CXX) -o $(ARTIFACT_TEST_BIN) $(ARTIFACT_OBJFILES) $(ARTIFACT_OBJ_TEST) $($(HOST)EXE_FLAGS) $(ARTIFACT_LDFLAGS)

# Installation/deinstallation
install: all
	echo "Installing $($(ARTIFACT_ID)_NAME)"
	mkdir -p "$(DESTDIR)$(BINDIR)"
	cp $(ARTIFACT_EXE) -t "$(DESTDIR)$(BINDIR)"
	ln -sf $(notdir $(ARTIFACT_EXE)) "$(DESTDIR)$(BINDIR)/$(ARTIFACT_EXELINK)"
	echo "Install OK"

uninstall:
	echo "Uninstalling $($(ARTIFACT_ID)_NAME)"
	-rm -f "$(DESTDIR)$(BINDIR)/$(ARTIFACT_EXELINK)"
	-rm -f "$(DESTDIR)$(BINDIR)/$(notdir $(ARTIFACT_EXE))"
	echo "Uninstall OK"

# Dependencies
-include $(CURDIR)/Makefile.d
