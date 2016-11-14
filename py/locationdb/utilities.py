# -*- Python -*-
# license
# license.
# ======================================================================

import datetime, json, bz2, lzma
import logging; module_logger = logging.getLogger(__name__)

# ======================================================================

from contextlib import contextmanager

@contextmanager
def timeit(name, logging_level=logging.DEBUG):
    start = datetime.datetime.utcnow()
    try:
        yield
    except Exception as err:
        module_logger.warning('{} <{}> with error {}'.format(name, datetime.datetime.utcnow() - start, err))
        raise
    else:
        module_logger.log(logging_level, '{} <{}>'.format(name, datetime.datetime.utcnow() - start))


# ----------------------------------------------------------------------

def read_json(filename):
    return json.loads(read_text(filename))

# ----------------------------------------------------------------------

def read_text(filename):
    filename = str(filename)
    try:
        data = lzma.LZMAFile(filename).read()
    except (IOError, EOFError, lzma.LZMAError):
        try:
            data = bz2.BZ2File(filename).read()
        except (IOError, EOFError):
            data = open(filename).read()
    if isinstance(data, bytes):
        data = data.decode("utf-8")
    return data

# ======================================================================
### Local Variables:
### eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
### End:
