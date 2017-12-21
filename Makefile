# -*- Makefile -*-
# Eugene Skepner 2016

# submodules and git: https://git-scm.com/book/en/v2/Git-Tools-Submodules

# ----------------------------------------------------------------------

MAKEFLAGS = -w

# ----------------------------------------------------------------------

LOCDB_SOURCES = locdb.cc export.cc
LOCDB_PY_SOURCES = py.cc $(LOCDB_SOURCES)

LOCATION_DB_LIB_MAJOR = 1
LOCATION_DB_LIB_MINOR = 0
LOCATION_DB_LIB_NAME = liblocationdb
LOCATION_DB_LIB = $(DIST)/$(call shared_lib_name,$(LOCATION_DB_LIB_NAME),$(LOCATION_DB_LIB_MAJOR),$(LOCATION_DB_LIB_MINOR))

LOCATION_DB_PY_LIB_MAJOR = 1
LOCATION_DB_PY_LIB_MINOR = 0
LOCATION_DB_PY_LIB_NAME = locationdb_backend
LOCATION_DB_PY_LIB = $(DIST)/$(LOCATION_DB_PY_LIB_NAME)$(PYTHON_MODULE_SUFFIX)

# ----------------------------------------------------------------------

include $(ACMACSD_ROOT)/share/makefiles/Makefile.g++
include $(ACMACSD_ROOT)/share/makefiles/Makefile.python
include $(ACMACSD_ROOT)/share/makefiles/Makefile.dist-build.vars

CXXFLAGS = -MMD -g $(OPTIMIZATION) $(PROFILE) -fPIC -std=$(STD) $(WARNINGS) -I$(BUILD)/include -I$(AD_INCLUDE) $(PKG_INCLUDES)
LDFLAGS = $(OPTIMIZATION) $(PROFILE)
LOCDB_LDLIBS = $(AD_LIB)/$(call shared_lib_name,libacmacsbase,1,0) $(shell pkg-config --libs liblzma)

PKG_INCLUDES = $(shell pkg-config --cflags liblzma) $(shell $(PYTHON_CONFIG) --includes)

# ----------------------------------------------------------------------

all: check-acmacsd-root $(LOCATION_DB_LIB) $(LOCATION_DB_PY_LIB)

install: check-acmacsd-root install-headers $(LOCATION_DB_PY_LIB) $(LOCATION_DB_LIB)
	$(call install_lib,$(LOCATION_DB_LIB))
	$(call install_py_lib,$(LOCATION_DB_PY_LIB))
	ln -sf $(realpath data/locationdb.json.xz) $(AD_DATA)
	ln -sf $(abspath bin)/locations $(AD_BIN)
	ln -sf $(abspath bin)/locdb.py $(AD_BIN)/locdb

test: check-acmacsd-root $(LOCATION_DB_PY_LIB) $(LOCATION_DB_LIB)
	env LD_LIBRARY_PATH=$(AD_LIB):$(LD_LIBRARY_PATH) bin/locations moscow | diff test/moscow.txt -
	env LD_LIBRARY_PATH=$(AD_LIB):$(LD_LIBRARY_PATH) bin/locations -c ug | diff test/ug.txt -

# ----------------------------------------------------------------------

-include $(BUILD)/*.d
include $(ACMACSD_ROOT)/share/makefiles/Makefile.dist-build.rules
include $(ACMACSD_ROOT)/share/makefiles/Makefile.rtags

# ----------------------------------------------------------------------

$(LOCATION_DB_PY_LIB): $(patsubst %.cc,$(BUILD)/%.o,$(LOCDB_PY_SOURCES)) | $(DIST)
	@printf "%-16s %s\n" "SHARED" $@
	@$(call make_shared,$(LOCATION_DB_PY_LIB_NAME),$(LOCATION_DB_PY_LIB_MAJOR),$(LOCATION_DB_PY_LIB_MINOR)) $(LDFLAGS) -o $@ $^ $(LOCDB_LDLIBS) $(PYTHON_LDLIBS)

$(LOCATION_DB_LIB): $(patsubst %.cc,$(BUILD)/%.o,$(LOCDB_SOURCES)) | $(DIST)
	@printf "%-16s %s\n" "SHARED" $@
	@$(call make_shared,$(LOCATION_DB_LIB_NAME),$(LOCATION_DB_LIB_MAJOR),$(LOCATION_DB_LIB_MINOR)) $(LDFLAGS) -o $@ $^ $(LOCDB_LDLIBS)

# ======================================================================
### Local Variables:
### eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
### End:
