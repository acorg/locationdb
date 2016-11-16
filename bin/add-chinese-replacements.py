#! /usr/bin/env python3
# license
# license.

"""
Updates locationdb with various Chinese replacements, e.g. YUNNAN-HONGTA is replaced with 'YUNNAN HONGTA'
"""

import sys, os, pprint, traceback
if sys.version_info.major != 3: raise RuntimeError("Run script with python3")
from pathlib import Path
sys.path[:0] = [str(Path(sys.argv[0]).resolve().parents[1].joinpath("py"))]
import logging; module_logger = logging.getLogger(__name__)
from locationdb import read
from locationdb.utilities import write_json, is_chinese

# ======================================================================

def main(args):
    if not os.environ.get("ACMACS_LOCATIONDB"):
        os.environ["ACMACS_LOCATIONDB"] = str(Path(sys.argv[0]).resolve().parents[1].joinpath("data", "locationdb.json.xz"))
    ldb = read.location_db()
    fix_hanzi_replacements(ldb)
    fix_hanzi_replacements(ldb, report=True)
    add_dash_replacements(ldb)
    add_dash_replacements(ldb, report=True)
    ldb.save()

# ----------------------------------------------------------------------

def add_dash_replacements(ldb, report=False):
    chinese_locations = set(loc for loc, entry in ldb.data["locations"].items() if entry[2] == "CHINA")
    new_replacements = {}

    def add(new_name, replacement):
        nonlocal new_replacements
        if new_name not in ldb.data["replacements"]:
            new_replacements[new_name] = replacement

    for name in (name for name, loc in ldb.data["names"].items() if loc in chinese_locations):
        if name.count(" ") == 1:
            add(name.replace(" ", "-"), name)
    for name, replacement in ((name, replacement) for name, replacement in ldb.data["replacements"].items() if ldb.data["names"][replacement] in chinese_locations):
        if name.count(" ") == 1:
            add(name.replace(" ", "-"), replacement)
    ldb.data["replacements"].update(new_replacements)
    module_logger.info('dash_replacements_added {}'.format(len(new_replacements)))
    if report and new_replacements:
        pprint.pprint(new_replacements)
        raise RuntimeError("New replacements in the second pass")

# ----------------------------------------------------------------------

def fix_hanzi_replacements(ldb, report=False):
    # old locations database has province-prefecture-county replacement for hanzi name
    # we now use province-county replacement that correspond to what they use in China
    names = ldb.data["names"]
    locations = ldb.data["locations"]
    new_replacements = {}
    new_names = {}

    def _remove_field(field_no, name, replacement):
        f = replacement.split()
        del f[field_no]
        new_replacement = " ".join(f)
        if new_replacement not in names:
            loc = names[replacement]
            new_names[new_replacement] = loc
        new_replacements[name] = new_replacement

    for name, replacement in ldb.data["replacements"].items():
        if is_chinese(name):
            if replacement.startswith("NEI MONGOL"):
                if replacement.count(" ") == 3:
                    _remove_field(2, name, replacement)
            elif replacement.count(" ") == 2:
                _remove_field(1, name, replacement)
            # elif replacement.count(" ") > 2:
            #     module_logger.info('long {!r}'.format(replacement))

    names.update(new_names)
    ldb.data["replacements"].update(new_replacements)
    module_logger.info('hanzi_replacements_added: {}  names added: {}'.format(len(new_replacements), len(new_names)))
    if report and (new_names or new_replacements):
        pprint.pprint(new_names)
        pprint.pprint(new_replacements)
        raise RuntimeError("New replacements in the second pass")

# ======================================================================

try:
    import argparse
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('-d', '--debug', action='store_const', dest='loglevel', const=logging.DEBUG, default=logging.INFO, help='Enable debugging output.')

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
