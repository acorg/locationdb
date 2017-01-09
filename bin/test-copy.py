#! /usr/bin/env python3
# license
# license.

"""
Test: reads and writes locationdb, compares results
"""

import sys, os, pprint, traceback
if sys.version_info.major != 3: raise RuntimeError("Run script with python3")
from pathlib import Path
sys.path[:0] = [str(Path(os.environ["ACMACSD_ROOT"]).resolve().joinpath("py"))]
import logging; module_logger = logging.getLogger(__name__)
import locationdb_backend as ldb_m
from acmacs_base import timeit, json

# ======================================================================

# acmacs:
# ? continents_to_antigens(chart) -> {name: [indices]} : for the passed chart returns mapping of continents names to sets of antigens indices
# ? has_antigens_isolated_in_continent(chart, continent_to_find) -> bool : Returns if the passed chart has antigens isolated in the passed continent

# ----------------------------------------------------------------------

def main(args):
    if not os.environ.get("ACMACS_LOCATIONDB"):
        os.environ["ACMACS_LOCATIONDB"] = str(Path(sys.argv[0]).resolve().parents[1].joinpath("data", "locationdb.json.xz"))
    ldb = ldb_m.LocDb()
    with timeit("Reading locationdb"):
        ldb.import_from(os.environ["ACMACS_LOCATIONDB"])
    with timeit("Writing locationdb"):
        json.write_json(Path("/tmp/locdb.json.xz"), , indent=1, compact=False)
    # ldb._updated = True
    # ldb.dbfile =

# ----------------------------------------------------------------------

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
