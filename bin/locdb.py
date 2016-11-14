#! /usr/bin/env python3
# license
# license.

"""
Looks for location in the locations database
"""

import sys, os, pprint, traceback
if sys.version_info.major != 3: raise RuntimeError("Run script with python3")
from pathlib import Path
sys.path[:0] = [str(Path(sys.argv[0]).resolve().parents[1].joinpath("py"))]
import logging; module_logger = logging.getLogger(__name__)
from locationdb import find, find_cdc_abbreviation, country, continent, geonames, LocationNotFound

# ======================================================================

# list by geonames
# add from geonames
# add cdc abbreviation for name
# add: name, country, division, lat, long - check for lat,long existence
# add: name for another name
# add replacement

# acmacs:
# [plot.markings] has_antigens_isolated_in_country(chart, country_to_find, include_reference=False) -> bool : Returns if the passed chart has antigens isolated in the passed country
# ? continents_to_antigens(chart) -> {name: [indices]} : for the passed chart returns mapping of continents names to sets of antigens indices
# ? has_antigens_isolated_in_continent(chart, continent_to_find) -> bool : Returns if the passed chart has antigens isolated in the passed continent

# ----------------------------------------------------------------------

def main(args):
    if not os.environ.get("ACMACS_LOCATIONDB"):
        os.environ["ACMACS_LOCATIONDB"] = str(Path(sys.argv[0]).resolve().parents[1].joinpath("data", "locationdb.json.xz"))
    for look_for in args.look_for:
        try:
            if args.cdc_abbreviation:
                print(look_for, find_cdc_abbreviation(cdc_abbreviation=look_for))
            elif args.country:
                print(look_for, ": ", country(name=look_for), sep="")
            elif args.continent:
                print(look_for, ": ", continent(name=look_for), sep="")
            elif args.geonames:
                print(look_for, ": ", geonames(name=look_for), sep="")
            else:
                print(look_for, find(name=look_for, like=args.like, handle_replacement=True))
        except LocationNotFound as err:
            print(look_for, "NOT FOUND", err)

# ----------------------------------------------------------------------

try:
    import argparse
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('-d', '--debug', action='store_const', dest='loglevel', const=logging.DEBUG, default=logging.INFO, help='Enable debugging output.')

    parser.add_argument('look_for', nargs="+", help='locations to look for.')
    parser.add_argument('-l', '--like', action="store_true", dest='like', default=False, help='kinda fuzzy search.')
    parser.add_argument('-c', '--cdc-abbreviation', action="store_true", dest='cdc_abbreviation', default=False, help='find cdc abbreviation.')
    parser.add_argument('--country', action="store_true", dest='country', default=False, help='report just country.')
    parser.add_argument('--continent', action="store_true", dest='continent', default=False, help='report just continent, look for either location name or country.')
    parser.add_argument('-g', '--geonames', action="store_true", dest='geonames', default=False, help='look in the geonames in order to update locationdb.')

    args = parser.parse_args()
    logging.basicConfig(level=args.loglevel, format="%(levelname)s %(asctime)s: %(message)s")
    exit_code = main(args)
except Exception as err:
    logging.error('{}\n{}'.format(err, traceback.format_exc()))
    exit_code = 1
exit(exit_code)

# ======================================================================
### Local Variables:
### eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
### End:
