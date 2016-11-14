# -*- Python -*-
# license
# license.
# ======================================================================

import os
import logging; module_logger = logging.getLogger(__name__)
from . import read
from .utilities import backup_file, write_json

# ----------------------------------------------------------------------

class CannotAdd (Exception): pass

# ======================================================================

def add(name, country, division, lat, long):
    ldb = read.location_db()
    try:
        name = name.upper()
        ldb.find(name=name)
        raise CannotAdd("{!r} already in the database".format(name))
    except read.LocationNotFound:
        pass
    country = country.upper()
    if not ldb.country_exists(country=country):
        raise CannotAdd("Country {!r} unknown".format(country))
    lat = float(lat)
    long = float(long)
    name_lat_long = ldb.find_by_lat_long(lat=lat, long=long)
    if name_lat_long is not None:
        raise CannotAdd("Entry with lat/long already exists: {!r}".format(name_lat_long))
    division = division.upper()
    ldb.data["locations"][name] = [lat, long, country, division]
    write_json(ldb.dbfile, ldb.data, indent=1, sort_keys=True, backup=True)

# ======================================================================
### Local Variables:
### eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
### End:
