# -*- Python -*-
# license
# license.
# ======================================================================

"""Looks name up in the [geonames database](http://www.geonames.org/).
[GeoNames Search Webservice API](http://www.geonames.org/export/geonames-search.html)
"""

import urllib.request, json
import logging; module_logger = logging.getLogger(__name__)

# ======================================================================

import os
from pathlib import Path
import logging; module_logger = logging.getLogger(__name__)

# ======================================================================

def geonames(name):
    if _is_chinese(name):
        r = _lookup_chinese(name=name)
    else:
        r = _lookup("search", isNameRequired="true", name=name)
    return r

# ----------------------------------------------------------------------

def _lookup(feature, **args):

    def make(entry):
        if entry["fcl"] in ["A", "P"]:
            return {
                # "local_name": entry[],
                "name": entry["toponymName"],
                "province": entry["adminName1"],
                "country": entry["countryName"],
                "latitude": entry["lat"],
                "longitude": entry["lng"],
                }
        else:
            return None

    return _get(feature, make, **args)

# ----------------------------------------------------------------------

def _get(feature, result_maker, **args):
    url = "http://api.geonames.org/{}?username=acorg&type=json&{}".format(feature, urllib.parse.urlencode(args))
    # module_logger.debug('_lookup {}'.format(url))
    return [e2 for e2 in (result_maker(e1) for e1 in json.loads(urllib.request.urlopen(url=url).read().decode("utf-8"))["geonames"]) if e2]

# ----------------------------------------------------------------------

def _lookup_chinese(name):
    if len(name) > 3:
        p
    else:
        def make(entry):
            name = _make_province_name(entry)
            return {
                "local_name": name,
                "name": name,
                "province": name,
                "country": entry["countryName"],
                "latitude": entry["lat"],
                "longitude": entry["lng"],
                }

        r = [make(e) for e in _find_chinese_province(name)]
    return r

# ----------------------------------------------------------------------

def _find_chinese_province(name):

    def make(entry):
        if entry["name"] == name:
            return entry
        else:
            return None

    r = _get("search", result_maker=make, isNameRequired="true", name_startsWith=name[:2], fclass="A", fcode="ADM1", lang="cn")
    if not r: # Inner Mongolia is written using 3 Hanzi
        r = _get("search", result_maker=make, isNameRequired="true", name_startsWith=name[:3], fclass="A", fcode="ADM1", lang="cn")
    return r

# ----------------------------------------------------------------------

def _make_province_name(entry):
    r = entry["toponymName"].upper()
    space_pos = r.find(' ', 6 if r[:6] == "INNER " else 0)
    if space_pos >= 0:
        r = r[:space_pos]
    return r;

# ----------------------------------------------------------------------

def _is_chinese(name):
    first = ord(name[0])
    return first >= 0x3400 and first <= 0x9FFFF;

# ======================================================================
### Local Variables:
### eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
### End:
