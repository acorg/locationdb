# -*- Makefile -*-
# Eugene Skepner 2016

# submodules and git: https://git-scm.com/book/en/v2/Git-Tools-Submodules

# ----------------------------------------------------------------------

MAKEFLAGS = -w

# ----------------------------------------------------------------------

LOCDB_SOURCES = locdb.cc export.cc
LOCDB_PY_SOURCES = py.cc $(LOCDB_SOURCES)

# ----------------------------------------------------------------------

include $(ACMACSD_ROOT)/share/makefiles/Makefile.g++
include $(ACMACSD_ROOT)/share/makefiles/Makefile.dist-build.vars

PYTHON_VERSION = $(shell python3 -c 'import sys; print("{0.major}.{0.minor}".format(sys.version_info))')
PYTHON_CONFIG = python$(PYTHON_VERSION)-config
PYTHON_MODULE_SUFFIX = $(shell $(PYTHON_CONFIG) --extension-suffix)

CXXFLAGS = -MMD -g $(OPTIMIZATION) $(PROFILE) -fPIC -std=$(STD) $(WARNINGS) -I$(BUILD)/include -I$(AD_INCLUDE) $(PKG_INCLUDES)
LDFLAGS = $(OPTIMIZATION) $(PROFILE)
LOCDB_LDLIBS = -L$(AD_LIB) -lacmacsbase -lboost_filesystem -lboost_system $(shell pkg-config --libs liblzma)
LOCDB_PY_LDLIBS = $(LOCDB_LDLIBS) $(shell $(PYTHON_CONFIG) --ldflags | sed -E 's/-Wl,-stack_size,[0-9]+//')

PKG_INCLUDES = $(shell pkg-config --cflags liblzma) $(shell $(PYTHON_CONFIG) --includes)

# ----------------------------------------------------------------------

LOCATION_DB_LIB = $(DIST)/liblocationdb.so

all: check-acmacsd-root $(DIST)/locationdb_backend$(PYTHON_MODULE_SUFFIX) $(LOCATION_DB_LIB)

install: check-acmacsd-root install-headers $(DIST)/locationdb_backend$(PYTHON_MODULE_SUFFIX) $(LOCATION_DB_LIB)
	$(call install_lib,$(LOCATION_DB_LIB))
	ln -sf $(DIST)/locationdb_backend$(PYTHON_MODULE_SUFFIX) $(AD_PY)
	ln -sf $(realpath data/locationdb.json.xz) $(AD_DATA)
	ln -sf $(abspath bin)/locations $(AD_BIN)
	ln -sf $(abspath bin)/locdb.py $(AD_BIN)/locdb

install-headers:
	$(call install_headers,locationdb)

test: check-acmacsd-root $(DIST)/locationdb_backend$(PYTHON_MODULE_SUFFIX) $(LOCATION_DB_LIB)
	env LD_LIBRARY_PATH=$(AD_LIB) bin/locations moscow | diff test/moscow.txt -
	env LD_LIBRARY_PATH=$(AD_LIB) bin/locations -c ug | diff test/ug.txt -

# ----------------------------------------------------------------------

-include $(BUILD)/*.d
include $(ACMACSD_ROOT)/share/makefiles/Makefile.dist-build.rules
include $(ACMACSD_ROOT)/share/makefiles/Makefile.rtags

# ----------------------------------------------------------------------

$(DIST)/locationdb_backend$(PYTHON_MODULE_SUFFIX): $(patsubst %.cc,$(BUILD)/%.o,$(LOCDB_PY_SOURCES)) | $(DIST)
	@echo "SHARED     " $@ # '<--' $^
	@$(CXX) -shared $(LDFLAGS) -o $@ $^ $(LOCDB_PY_LDLIBS)
	@#strip $@

$(LOCATION_DB_LIB): $(patsubst %.cc,$(BUILD)/%.o,$(LOCDB_SOURCES)) | $(DIST)
	@echo "SHARED     " $@ # '<--' $^
	@$(CXX) -shared $(LDFLAGS) -o $@ $^ $(LOCDB_LDLIBS)

# ----------------------------------------------------------------------

# $(BUILD)/%.o: cc/%.cc | $(BUILD) install-headers
#	@echo $(CXX_NAME) $<
#	@$(CXX) $(CXXFLAGS) -c -o $@ $<

# ======================================================================
### Local Variables:
### eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
### End:
