# -*- Makefile -*-
# Eugene Skepner 2016

# submodules and git: https://git-scm.com/book/en/v2/Git-Tools-Submodules

# ----------------------------------------------------------------------

MAKEFLAGS = -w

# ----------------------------------------------------------------------

LOCDB_SOURCES = locdb.cc export.cc
LOCDB_PY_SOURCES = py.cc $(LOCDB_SOURCES)

# ----------------------------------------------------------------------

include $(ACMACSD_ROOT)/share/Makefile.g++

PYTHON_VERSION = $(shell python3 -c 'import sys; print("{0.major}.{0.minor}".format(sys.version_info))')
PYTHON_CONFIG = python$(PYTHON_VERSION)-config
PYTHON_MODULE_SUFFIX = $(shell $(PYTHON_CONFIG) --extension-suffix)

# -fvisibility=hidden and -flto make resulting lib smaller (pybind11) but linking is much slower
OPTIMIZATION = -O3 #-fvisibility=hidden -flto
PROFILE = # -pg
CXXFLAGS = -MMD -g $(OPTIMIZATION) $(PROFILE) -fPIC -std=$(STD) $(WEVERYTHING) $(WARNINGS) -I$(BUILD)/include -I$(ACMACSD_ROOT)/include $(PKG_INCLUDES)
LDFLAGS = $(OPTIMIZATION) $(PROFILE)
LIB_DIR = $(ACMACSD_ROOT)/lib
LOCDB_LDLIBS = -L$(LIB_DIR) -lacmacsbase -lboost_filesystem -lboost_system $$(pkg-config --libs liblzma)
LOCDB_PY_LDLIBS = $(LOCDB_LDLIBS) $$($(PYTHON_CONFIG) --ldflags | sed -E 's/-Wl,-stack_size,[0-9]+//')

PKG_INCLUDES = $$(pkg-config --cflags liblzma) $$($(PYTHON_CONFIG) --includes)

# ----------------------------------------------------------------------

BUILD = build
DIST = $(abspath dist)

LOCATION_DB_LIB = $(DIST)/liblocationdb.so

all: check-acmacsd-root $(DIST)/locationdb_backend$(PYTHON_MODULE_SUFFIX) $(LOCATION_DB_LIB)

install: check-acmacsd-root install-headers $(DIST)/locationdb_backend$(PYTHON_MODULE_SUFFIX) $(LOCATION_DB_LIB)
	ln -sf $(LOCATION_DB_LIB) $(ACMACSD_ROOT)/lib
	if [ $$(uname) = "Darwin" ]; then /usr/bin/install_name_tool -id $(ACMACSD_ROOT)/lib/$(notdir $(LOCATION_DB_LIB)) $(ACMACSD_ROOT)/lib/$(notdir $(LOCATION_DB_LIB)); fi
	ln -sf $(DIST)/locationdb_backend$(PYTHON_MODULE_SUFFIX) $(ACMACSD_ROOT)/py
	ln -sf $(realpath data/locationdb.json.xz) $(ACMACSD_ROOT)/data
	ln -sf $(abspath bin)/locations $(ACMACSD_ROOT)/bin

install-headers:
	if [ ! -d $(ACMACSD_ROOT)/include/locationdb ]; then mkdir $(ACMACSD_ROOT)/include/locationdb; fi
	ln -sf $(abspath cc)/*.hh $(ACMACSD_ROOT)/include/locationdb

test: check-acmacsd-root $(DIST)/locationdb_backend$(PYTHON_MODULE_SUFFIX) $(LOCATION_DB_LIB)
	env LD_LIBRARY_PATH=$(LIB_DIR) bin/locations moscow | diff test/moscow.txt -
	env LD_LIBRARY_PATH=$(LIB_DIR) bin/locations -c ug | diff test/ug.txt -

# ----------------------------------------------------------------------

-include $(BUILD)/*.d

# ----------------------------------------------------------------------

$(DIST)/locationdb_backend$(PYTHON_MODULE_SUFFIX): $(patsubst %.cc,$(BUILD)/%.o,$(LOCDB_PY_SOURCES)) | $(DIST)
	$(GXX) -shared $(LDFLAGS) -o $@ $^ $(LOCDB_PY_LDLIBS)
	@#strip $@

$(LOCATION_DB_LIB): $(patsubst %.cc,$(BUILD)/%.o,$(LOCDB_SOURCES)) | $(DIST)
	$(GXX) -shared $(LDFLAGS) -o $@ $^ $(LOCDB_LDLIBS)

clean:
	rm -rf $(DIST) $(BUILD)/*.o $(BUILD)/*.d

distclean: clean
	rm -rf $(BUILD)

# ----------------------------------------------------------------------

$(BUILD)/%.o: cc/%.cc | $(BUILD) install-headers
	@echo $<
	@$(GXX) $(CXXFLAGS) -c -o $@ $<

# ----------------------------------------------------------------------

# $(BUILD)/submodules:
#	git submodule init
#	git submodule update
#	git submodule update --remote
#	touch $@

# ----------------------------------------------------------------------

check-acmacsd-root:
ifndef ACMACSD_ROOT
	$(error ACMACSD_ROOT is not set)
endif

$(DIST):
	mkdir -p $(DIST)

$(BUILD):
	mkdir -p $(BUILD)

.PHONY: check-acmacsd-root

# ======================================================================
### Local Variables:
### eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
### End:
