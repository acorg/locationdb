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
from locationdb import check, fix, find, find_cdc_abbreviation, country, continent, geonames, add, add_cdc_abbreviation, add_new_name, add_replacement, find_cdc_abbreviation_for_name, LocationNotFound

# ======================================================================

# acmacs:
# ? continents_to_antigens(chart) -> {name: [indices]} : for the passed chart returns mapping of continents names to sets of antigens indices
# ? has_antigens_isolated_in_continent(chart, continent_to_find) -> bool : Returns if the passed chart has antigens isolated in the passed continent

# ----------------------------------------------------------------------

def main(args):
    if not os.environ.get("ACMACS_LOCATIONDB"):
        os.environ["ACMACS_LOCATIONDB"] = str(Path(sys.argv[0]).resolve().parents[1].joinpath("data", "locationdb.json.xz"))
    if args.check:
        check()
    if args.fix:
        fix()
    if args.add:
        if len(args.look_for) != 5:
            module_logger.error('5 arguments required for adding: name country division lat long')
            return 1
        add(*args.look_for)
    elif args.add_cdc_abbreviation:
        if len(args.look_for) != 2:
            module_logger.error('2 arguments required for adding cdc abbreviation: name cdc_abbreviation')
            return 1
        add_cdc_abbreviation(*args.look_for)
    elif args.add_name:
        if len(args.look_for) != 2:
            module_logger.error('2 arguments required for adding new name: existing-name new-name')
            return 1
        add_new_name(*args.look_for)
    elif args.add_replacement:
        if len(args.look_for) < 2 or (len(args.look_for) % 2) != 0:
            module_logger.error('Even arguments required for adding replacement: existing-name-to-replace-with new-name')
            return 1
        for a_no in range(0, len(args.look_for), 2):
            add_replacement(args.look_for[a_no], args.look_for[a_no+1])
    else:
        for look_for in args.look_for:
            try:
                if args.cdc_abbreviation:
                    print(look_for, find_cdc_abbreviation(cdc_abbreviation=look_for))
                elif args.cdc_abbreviation_for_name:
                    print(look_for, find_cdc_abbreviation_for_name(look_for))
                elif args.country:
                    print(look_for, ": ", country(name=look_for), sep="")
                elif args.continent:
                    print(look_for, ": ", continent(name=look_for), sep="")
                elif args.geonames:
                    entries = list(geonames(name=look_for))
                    max_name = max((len(e["name"]) for e in entries), default=1)
                    max_country = max((len(sCountries.get(e["country"].upper(), e["country"])) for e in entries), default=1)
                    max_division = max((len(e["province"]) for e in entries), default=1)
                    def format_entry(entry):
                        country = entry["country"].upper()
                        country = sCountries.get(country, country)
                        return "--add {name:<{max_name}s} {country:<{max_country}s} {division:<{max_division}s} {lat:>6.2f} {long:>7.2f}".format(name="'{}'".format(entry["name"].upper()), max_name=max_name + 2, country="'{}'".format(country), max_country=max_country + 2, division="'{}'".format(entry["province"].upper()), max_division=max_division + 2, lat=float(entry["latitude"]), long=float(entry["longitude"]))
                    print(look_for, "\n".join(format_entry(e) for e in entries), sep="\n")
                else:
                    entry = find(name=look_for, like=args.like, handle_replacement=True)
                    if not isinstance(entry, list):
                        entry = [entry]
                    # pprint.pprint(entry, width=120)
                    for e in entry:
                        print("look-for:{!r} name:{!r} location:{!r} {}division:{!r} country:{!r} continent:{!r} lat:{!r} long:{!r}".format(look_for, e.name, e.found, "replacement:{!r} ".format(e["replacement"]) if e.get("replacement") else "", e.division, e.country, e.continent, e.latitude, e.longitude))
            except LocationNotFound as err:
                print(look_for, "NOT FOUND", err)

# ----------------------------------------------------------------------

sCountries = {
    "UNITED STATES": "UNITED STATES OF AMERICA",
    }

# ----------------------------------------------------------------------

try:
    import argparse
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('-d', '--debug', action='store_const', dest='loglevel', const=logging.DEBUG, default=logging.INFO, help='Enable debugging output.')

    parser.add_argument('look_for', nargs="+", help='locations to look for.')
    parser.add_argument('-l', '--like', action="store_true", dest='like', default=False, help='kinda fuzzy search.')
    parser.add_argument('-c', '--cdc-abbreviation', action="store_true", dest='cdc_abbreviation', default=False, help='find cdc abbreviation.')
    parser.add_argument('--cdc-abbreviation-for-name', action="store_true", dest='cdc_abbreviation_for_name', default=False, help='find cdc abbreviation for given name.')
    parser.add_argument('--country', action="store_true", dest='country', default=False, help='report just country.')
    parser.add_argument('--continent', action="store_true", dest='continent', default=False, help='report just continent, look for either location name or country.')
    parser.add_argument('-g', '--geonames', action="store_true", dest='geonames', default=False, help='look in the geonames in order to update locationdb.')
    parser.add_argument('--add', action="store_true", dest='add', default=False, help='adds new entry, args: name country division lat long')
    parser.add_argument('--add-cdc-abbreviation', action="store_true", dest='add_cdc_abbreviation', default=False, help='adds cdc abbreaviation for a name, args: name cdc_abbreviation.')
    parser.add_argument('--add-name', action="store_true", dest='add_name', default=False, help='adds new name for existing location, args: existing-name new-name.')
    parser.add_argument('--add-replacement', action="store_true", dest='add_replacement', default=False, help='adds replacement for a name, args: existing-name-to-replace-with new-name.')
    parser.add_argument('--no-check', action="store_false", dest='check', default=True, help='do not validate location db structure.')
    parser.add_argument('--fix', action="store_true", dest='fix', default=False, help='fix location db structure.')

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
