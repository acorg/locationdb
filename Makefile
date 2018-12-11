# -*- Makefile -*-
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

all: install

CONFIGURE_PYTHON = 1
include $(ACMACSD_ROOT)/share/Makefile.config

# ----------------------------------------------------------------------

LOCDB_LDLIBS = $(AD_LIB)/$(call shared_lib_name,libacmacsbase,1,0) $(XZ_LIBS) $(CXX_LIBS)

# ----------------------------------------------------------------------

install: install-headers libs
	$(call install_lib,$(LOCATION_DB_LIB))
	$(call install_py_lib,$(LOCATION_DB_PY_LIB))
	$(call symbolic_link,$(realpath data/locationdb.json.xz),$(AD_DATA))
	$(call symbolic_link,$(abspath bin)/locations,$(AD_BIN))
	$(call symbolic_link,$(abspath bin)/locdb.py,$(AD_BIN)/locdb)

test: install
	env LD_LIBRARY_PATH=$(AD_LIB):$(LD_LIBRARY_PATH) bin/locations moscow | diff test/moscow.txt -
	env LD_LIBRARY_PATH=$(AD_LIB):$(LD_LIBRARY_PATH) bin/locations -c ug | diff test/ug.txt -

libs: $(LOCATION_DB_LIB) $(LOCATION_DB_PY_LIB)

# ----------------------------------------------------------------------

$(LOCATION_DB_PY_LIB): $(patsubst %.cc,$(BUILD)/%.o,$(LOCDB_PY_SOURCES)) | $(DIST)
	$(call echo_shared_lib,$@)
	$(call make_shared_lib,$(LOCATION_DB_PY_LIB_NAME),$(LOCATION_DB_PY_LIB_MAJOR),$(LOCATION_DB_PY_LIB_MINOR)) $(LDFLAGS) -o $@ $^ $(LOCDB_LDLIBS) $(PYTHON_LIBS)

$(LOCATION_DB_LIB): $(patsubst %.cc,$(BUILD)/%.o,$(LOCDB_SOURCES)) | $(DIST)
	$(call echo_shared_lib,$@)
	$(call make_shared_lib,$(LOCATION_DB_LIB_NAME),$(LOCATION_DB_LIB_MAJOR),$(LOCATION_DB_LIB_MINOR)) $(LDFLAGS) -o $@ $^ $(LOCDB_LDLIBS)

# ======================================================================
### Local Variables:
### eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
### End:
