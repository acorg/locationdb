# -*- Makefile -*-
# Eugene Skepner 2016

# submodules and git: https://git-scm.com/book/en/v2/Git-Tools-Submodules

# ----------------------------------------------------------------------

MAKEFLAGS = -w

# ----------------------------------------------------------------------

LOCDB_SOURCES = locdb.cc export.cc read-file.cc xz.cc
LOCDB_PY_SOURCES = py.cc $(LOCDB_SOURCES)

# ----------------------------------------------------------------------

CLANG = $(shell if g++ --version 2>&1 | grep -i llvm >/dev/null; then echo Y; else echo N; fi)
ifeq ($(CLANG),Y)
  WEVERYTHING = -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-padded
  WARNINGS = -Wno-weak-vtables # -Wno-padded
  STD = c++14
else
  WEVERYTHING = -Wall -Wextra
  WARNINGS =
  STD = c++14
endif

PYTHON_VERSION = $(shell python3 -c 'import sys; print("{0.major}.{0.minor}".format(sys.version_info))')
PYTHON_CONFIG = python$(PYTHON_VERSION)-config
PYTHON_MODULE_SUFFIX = $(shell $(PYTHON_CONFIG) --extension-suffix)

# -fvisibility=hidden and -flto make resulting lib smaller (pybind11) but linking is much slower
OPTIMIZATION = -O3 #-fvisibility=hidden -flto
PROFILE = # -pg
CXXFLAGS = -MMD -g $(OPTIMIZATION) $(PROFILE) -fPIC -std=$(STD) $(WEVERYTHING) $(WARNINGS) -I$(BUILD)/include -I$(ACMACSD_ROOT)/include $(PKG_INCLUDES)
LDFLAGS = $(OPTIMIZATION) $(PROFILE)
LOCDB_LDLIBS = $$(pkg-config --libs liblzma)
LOCDB_PY_LDLIBS = $(LOCDB_LDLIBS) $$($(PYTHON_CONFIG) --ldflags | sed -E 's/-Wl,-stack_size,[0-9]+//')

PKG_INCLUDES = $$(pkg-config --cflags liblzma) $$($(PYTHON_CONFIG) --includes)

# ----------------------------------------------------------------------

BUILD = build
DIST = $(abspath dist)

# Do NOT use name locationdb.so for a non-python library because it will conflict with locationdb python module
LOCATION_DB_LIB = $(DIST)/location-db.so

all: check-acmacsd-root $(DIST)/locationdb_backend$(PYTHON_MODULE_SUFFIX) $(LOCATION_DB_LIB)

install: check-acmacsd-root $(DIST)/locationdb_backend$(PYTHON_MODULE_SUFFIX) $(LOCATION_DB_LIB)
	ln -sf $(LOCATION_DB_LIB) $(ACMACSD_ROOT)/lib
	if [ $$(uname) = "Darwin" ]; then /usr/bin/install_name_tool -id $(ACMACSD_ROOT)/lib/$(notdir $(LOCATION_DB_LIB)) $(ACMACSD_ROOT)/lib/$(notdir $(LOCATION_DB_LIB)); fi
	if [ ! -d $(ACMACSD_ROOT)/include/locationdb ]; then mkdir $(ACMACSD_ROOT)/include/locationdb; fi
	ln -sf $(abspath cc)/locdb.hh $(ACMACSD_ROOT)/include/locationdb
	ln -sf $(DIST)/locationdb_backend$(PYTHON_MODULE_SUFFIX) $(ACMACSD_ROOT)/py
	ln -sf $(realpath data/locationdb.json.xz) $(ACMACSD_ROOT)/data
	ln -sf $(abspath bin)/locations $(ACMACSD_ROOT)/bin

-include $(BUILD)/*.d

# ----------------------------------------------------------------------

$(DIST)/locationdb_backend$(PYTHON_MODULE_SUFFIX): $(patsubst %.cc,$(BUILD)/%.o,$(LOCDB_PY_SOURCES)) | $(DIST)
	g++ -shared $(LDFLAGS) -o $@ $^ $(LOCDB_PY_LDLIBS)
	@#strip $@

$(LOCATION_DB_LIB): $(patsubst %.cc,$(BUILD)/%.o,$(LOCDB_SOURCES)) | $(DIST)
	g++ -shared $(LDFLAGS) -o $@ $^ $(LOCDB_LDLIBS)

clean:
	rm -rf $(DIST) $(BUILD)/*.o $(BUILD)/*.d

distclean: clean
	rm -rf $(BUILD)

# ----------------------------------------------------------------------

$(BUILD)/%.o: cc/%.cc | $(BUILD)
	@echo $<
	@g++ $(CXXFLAGS) -c -o $@ $<

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
