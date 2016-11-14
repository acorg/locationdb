# -*- Python -*-
# license
# license.
# ======================================================================

import os
from pathlib import Path
import logging; module_logger = logging.getLogger(__name__)
from .utilities import timeit, read_json

# ======================================================================

class LocationNotFound (Exception):
    ''

class CountryNotFound (Exception):
    ''

class LocationReplacement (Exception):
    """Raised when replacement is found."""

    def __init__(self, replacement):
        super().__init__()
        self.replacement = replacement

    def __str__(self):
        return 'Replace with ' + self.replacement

# ======================================================================

def find(name, like=False, return_found_name=True, handle_replacement=False):
    """May raise LocationNotFound and LocationReplacement"""
    return location_db().find(name=name, like=like, handle_replacement=handle_replacement)

# ======================================================================

def find_cdc_abbreviation(cdc_abbreviation):
    return location_db().find_cdc_abbreviation(cdc_abbreviation=cdc_abbreviation)

# ======================================================================

def country(name):
    """Returns country by location name. May raise LocationNotFound
    and LocationReplacement"""
    return location_db().find(name=name)["country"]

# ======================================================================

def continent(name=None):
    """Returns continent by either location name or country. May raise
    LocationNotFound and LocationReplacement"""
    ldb = location_db()
    try:
        return ldb.find_continent(country=name)
    except LocationNotFound:
        return ldb.find_continent(country=ldb.find(name=name)["country"])

# ======================================================================

class LocationDb:

    def __init__(self):
        with timeit("Loading LocationDb"):
            dbfile = Path(os.environ["ACMACS_LOCATIONDB"])
            if not dbfile.exists():
                raise RuntimeError("LocationDb file not found (ACMACS_LOCATIONDB={})".format(os.environ.get("ACMACS_LOCATIONDB")))
            self.data = read_json(dbfile)

    # "?continents": "[continent]",
    # "?countries": "{country: index in continents}",
    # "?locations": "{name: [latitude, longitude, country, division]}",
    # "?names": "{name: name in locations}",
    # "?cdc_abbreviation": "{cdc_abbreviation: name in locations}",
    # "?replacements": "{name: name in names}",

    def find(self, name, like=False, handle_replacement=False):
        name = name.upper()
        replacement = None
        try:
            n = self.data["names"][name]
        except KeyError:
            try:
                replacement = self.data["replacements"][name]
                if not handle_replacement:
                    raise LocationReplacement(replacement)
                n = self.data["names"][replacement]
            except KeyError:
                n = name
        try:
            r = self._make_result(name=name, found=n, loc=self.data["locations"][n], replacement=replacement)
        except KeyError:
            if like:
                r = self.find_like(name)
            else:
                raise LocationNotFound(name)
        return r

    def find_cdc_abbreviation(self, cdc_abbreviation):
        return self.find(self.data["cdc_abbreviations"][cdc_abbreviation.upper()])

    def find_continent(self, country):
        try:
            return self.data["continents"][self.data["countries"][country]]
        except:
            raise LocationNotFound(country)

    def _make_result(self, name, found, loc, replacement=None):
        r = {"name": name, "found": found, "latitude": loc[0], "longitude": loc[1], "country": loc[2], "division": loc[3]}
        try:
            r["continent"] = self.data["continents"][self.data["countries"][loc[2]]]
        except:
            module_logger.error('No continent for {}'.format(loc[2]))
            r["continent"] = "UNKNOWN"
        if replacement:
            r["replacement"] = replacement
        return r

    def find_like(self, name):
        return [self._make_result(name=n, found=nn, loc=self.data["locations"][nn]) for n, nn in self.data["names"].items() if name in n]

# ======================================================================

class LocationDbNotFound:

    def find(self, name, like=False, return_found_name=False, **kwargs):
        module_logger.warning('Trying to find location {!r} in LocationDbNotFound'.format(name))
        if return_found_name:
            raise LocationNotFound('LocationDb not available')
        return None

# ======================================================================

sLocationDb = None  # singleton

def location_db():
    global sLocationDb
    if sLocationDb is None:
        try:
            sLocationDb = LocationDb()
        except Exception as err:
            module_logger.error('LocationDb creation failed: {}'.format(err))
            sLocationDb = LocationDbNotFound()
    return sLocationDb

# ======================================================================
### Local Variables:
### eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
### End:
