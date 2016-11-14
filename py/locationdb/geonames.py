# -*- Python -*-
# license
# license.
# ======================================================================

"""Looks name up in the [geonames database](http://www.geonames.org/).
[GeoNames Search Webservice API](http://www.geonames.org/export/geonames-search.html)
"""

import urllib.request, json, pprint
import logging; module_logger = logging.getLogger(__name__)

# ======================================================================

import os
from pathlib import Path
import logging; module_logger = logging.getLogger(__name__)

# ======================================================================

def geonames(name):
    if _is_chinese(name):
        raise NotImplementedError()
    else:
        r = _lookup("search", isNameRequired="true", name=name)
        pprint.pprint(r)
    return r

# ----------------------------------------------------------------------

def _lookup(feature, **args):

    def make(entry):
        return {
            # "local_name": entry[],
            "name": entry["toponymName"],
            "province": entry["adminName1"],
            "country": entry["countryName"],
            "latitude": entry["lat"],
            "longitude": entry["lng"],
            }

    url = "http://api.geonames.org/{}?username=acorg&type=json&{}".format(feature, urllib.parse.urlencode(args))
    # module_logger.debug('_lookup {}'.format(url))
    return [make(e1) for e1 in json.loads(urllib.request.urlopen(url=url).read().decode("utf-8"))["geonames"] if e1["fcl"] in ["A", "P"]]

# ----------------------------------------------------------------------

def _is_chinese(name):
    return False

# ======================================================================
### Local Variables:
### eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
### End:
