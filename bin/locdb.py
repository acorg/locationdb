#! /usr/bin/env python3
# license
# license.

"""
Looks for location in the locations database
"""

import sys, os, pprint, json, traceback
if sys.version_info.major != 3: raise RuntimeError("Run script with python3")
from pathlib import Path
sys.path[:0] = [str(Path(sys.argv[0]).resolve().parents[1].joinpath("py"))]
import logging; module_logger = logging.getLogger(__name__)
from locationdb import check, fix, find, find_cdc_abbreviation, country, continent, geonames, add, add_cdc_abbreviation, add_new_name, add_replacement, find_cdc_abbreviation_for_name, LocationNotFound, save

CHINA_DISTRICT_SUFFIXES = ["XIAN", "QU", "SHI"] # county, district, city

# ======================================================================

# acmacs:
# ? continents_to_antigens(chart) -> {name: [indices]} : for the passed chart returns mapping of continents names to sets of antigens indices
# ? has_antigens_isolated_in_continent(chart, continent_to_find) -> bool : Returns if the passed chart has antigens isolated in the passed continent

# ----------------------------------------------------------------------

def main(args):
    exit_code = 0
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
    elif args.to_eval:
        do_eval(json.loads(args.to_eval))
    else:
        for look_for in args.look_for:
            look_for = look_for.upper()
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
                        name = entry["name"].upper()
                        division = entry["province"].upper()
                        if country == "CHINA" and division not in name:
                            name = f"{division} {name}"
                        return "locdb --add {name:<{max_name}s} {country:<{max_country}s} {division:<{max_division}s} {lat:>6.2f} {long:>7.2f}".format(name=f"'{name}'", max_name=max_name + 2, country=f"'{country}'", max_country=max_country + 2, division=f"'{division}'", max_division=max_division + 2, lat=float(entry["latitude"]), long=float(entry["longitude"]))
                    print(look_for, "\n".join(format_entry(e) for e in entries), sep="\n")
                    exit_code = 1
                else:
                    exit_code = xfind(look_for)
                    if exit_code == 99 and "DOU" in look_for:
                        exit_code = xfind(look_for.replace("DOU", "DU"))
                        if exit_code != 99:
                            print(f"""{{"C": "replacement", "existing": , "new": "{look_for}"}}""")
                    print()
            except LocationNotFound as err:
                print(f"ERROR: \"{look_for}\" NOT FOUND: {err}", file=sys.stderr)
    return exit_code

# ======================================================================

def do_eval(to_eval):
    if not isinstance(to_eval, list):
        to_eval = [to_eval]
    for entry in to_eval:
        cmd = entry.pop("C")
        if cmd == "add":
            print(cmd, repr(entry["name"]))
            add(**entry, save=False)
        elif cmd == "replacement":
            print(cmd, repr(entry["new"]), "->", repr(entry["existing"]))
            add_replacement(name_to_replace_with=entry["existing"], new_name=entry["new"], save=False)
        elif cmd == "add_name":
            print(cmd, repr(entry["new_name"]), "->", repr(entry["existing"]))
            add_new_name(name=entry["existing"], new_name=entry["new_name"], save=False)
        else:
            raise RuntimeError(f"Unrecognized command: {cmd}")
    print("saving locdb ...")
    save()

# ======================================================================

def xfind(look_for):
    if find_report(look_for):
        return 0
    for separator in ["-", "_"]:
        if separator in look_for:
            words = look_for.split(separator)
            if find_report(" ".join(words), orig=look_for):
                return 0
            for suffix in CHINA_DISTRICT_SUFFIXES:
                suffix_size = len(suffix)
                if words[-1][-suffix_size:] == suffix and find_report(" ".join(words)[:-suffix_size], orig=look_for):
                    return 0

            # match by last word
            def match_last_word(words):
                try:
                    for entry in find(name=words[-1], like=True, handle_replacement=True):
                        ewords = entry.name.split(" ")
                        if all(ww in ewords for ww in words):
                            print(f"look-for:{look_for!r} name:{entry.name!r} location:{entry.found!r} {replacement}division:{entry.division!r} country:{entry.country!r} continent:{entry.continent!r} lat:{entry.latitude!r} long:{entry.longitude!r}")
                            print(f"""WARNING: run to add replacement:\nlocdb -e '[{{"C": "replacement", "existing": "{entry.name}", "new": "{look_for}"}}]'""")
                            return True
                except:
                    pass
                return False

            if match_last_word(words):
                return 1
            if try_geonames(look_for=words[-1], orig_name=look_for, words=words): # geonames by last word
                return 1
            for suffix in CHINA_DISTRICT_SUFFIXES:
                suffix_size = len(suffix)
                if words[-1][-suffix_size:] == suffix:
                    words_without_suffix = words[:-1] + [words[-1][:-suffix_size]]
                    if match_last_word(words_without_suffix):
                        return 1
                    if try_geonames(look_for=words[-1][:-suffix_size], orig_name=look_for, words=words_without_suffix): # geonames by last word without XIAN/QU/SHI suffix
                        return 1

    if try_geonames(look_for=look_for):
        return 1

    print(f"ERROR: NOT FOUND: \"{look_for}\"", file=sys.stderr)
    return 99

# ======================================================================

def try_geonames(look_for, orig_name=None, words=None):
    # print(f"DEBUG: try_geonames {look_for!r} orig:{orig_name!r} {words}")
    found = False
    if not orig_name:
        orig_name = look_for
    for entry in geonames(name=look_for):
        division = entry["province"].upper()
        country = entry["country"].upper()
        name = entry["name"].upper()
        name_words = name.split(" ")
        if not words or (division == words[0] and any(nw in words for nw in name_words)):
            if country == "CHINA":
                if name_words[-1] in CHINA_DISTRICT_SUFFIXES:
                    name = " ".join(name_words[:-1])
                full_name = f"{division} {name}"
            else:
                full_name = name
            cmd = f"""locdb -e '[{{"C": "add", "name": "{full_name}", "country": "{country}", "division": "{division}", "lat": {float(entry["latitude"]):>6.2f}, "long": {float(entry["longitude"]):>7.2f}}}"""
            if full_name != orig_name:
                cmd += f""", {{"C": "replacement", "existing": "{full_name}", "new": "{orig_name}"}}"""
            cmd += "]'"
            print(cmd)
            found = True
    return found

# ======================================================================

def find_report(look_for, like=False, orig=None):
    try:
        for e in find(name=look_for, like=like, handle_replacement=True):
            # pprint.pprint(entry, width=120)
            replacement = f"replacement:{e['replacement']!r} " if e.get("replacement") else ""
            print(f"look-for:{look_for!r} name:{e.name!r} location:{e.found!r} {replacement}division:{e.division!r} country:{e.country!r} continent:{e.continent!r} lat:{e.latitude!r} long:{e.longitude!r}")
        if orig and orig != look_for:
            print(f"""WARNING: run to add replacement:\nlocdb -e '[{{"C": "replacement", "existing": "{e.name}", "new": "{orig}"}}]'""")
        return True
    except LocationNotFound as err:
        return False

# ======================================================================

sCountries = {
    "UNITED STATES": "UNITED STATES OF AMERICA",
    }

# ----------------------------------------------------------------------

try:
    import argparse
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('-d', '--debug', action='store_const', dest='loglevel', const=logging.DEBUG, default=logging.INFO, help='Enable debugging output.')

    parser.add_argument('look_for', nargs="*", help='locations to look for.')
    parser.add_argument('-e', dest="to_eval", help='Evaluate json commands, e.g. [{"C": "add", "name": "GUANGXI HEPU", "country": "CHINA", "division": "GUANGXI", "lat": 21.68, "long": 109.15}, {"C": "replacement", "existing": "GUANGXI HEPU", "new": "GUANGXI-HEPU"}].')
    parser.add_argument('-l', '--like', action="store_true", dest='like', default=False, help='kinda fuzzy search.')
    parser.add_argument('-c', '--cdc-abbreviation', action="store_true", dest='cdc_abbreviation', default=False, help='find cdc abbreviation.')
    parser.add_argument('--cdc-abbreviation-for-name', action="store_true", dest='cdc_abbreviation_for_name', default=False, help='find cdc abbreviation for given name.')
    parser.add_argument('--country', action="store_true", dest='country', default=False, help='report just country.')
    parser.add_argument('--continent', action="store_true", dest='continent', default=False, help='report just continent, look for either location name or country.')
    parser.add_argument('-g', '--geonames', action="store_true", dest='geonames', default=False, help='look in the geonames in order to update locationdb.')
    parser.add_argument('--add', action="store_true", dest='add', default=False, help='adds new entry, args: name country division lat long')
    parser.add_argument('--add-cdc-abbreviation', action="store_true", dest='add_cdc_abbreviation', default=False, help='adds cdc abbreaviation for a name, args: name cdc_abbreviation.')
    parser.add_argument('-n', '--add-name', action="store_true", dest='add_name', default=False, help='adds new name for existing location, args: existing-name new-name.')
    parser.add_argument('-r', '--add-replacement', action="store_true", dest='add_replacement', default=False, help='adds replacement for a name, args: existing-name-to-replace-with new-name.')
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
