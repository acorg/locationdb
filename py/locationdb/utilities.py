# -*- Python -*-
# license
# license.
# ======================================================================

import datetime, json, bz2, lzma, shutil
import logging; module_logger = logging.getLogger(__name__)
from pathlib import Path

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

def write_json(filename, data, indent=None, sort_keys=False, backup=True, emacs_local_var=True):
    if indent is None:
        separators=[',', ':']
    else:
        separators=[',', ': ']
    dump = json.dumps(data, separators=separators, indent=indent, sort_keys=sort_keys)
    if indent is not None and emacs_local_var and "_" not in data:
        dump = '{{{empty:<{indent_1}s}"_": "-*- js-indent-level: {indent} -*-",'.format(empty="", indent=indent, indent_1=indent-1) + dump[1:]
    write_text(filename, dump, backup=backup)

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

# ----------------------------------------------------------------------

def write_text(filename, text, backup=True):
    if backup:
        backup_file(filename)
    if filename.suffix == ".xz":
        f = lzma.open(str(filename), mode='wb', preset=9 | lzma.PRESET_EXTREME)
    elif filename.suffix == ".bz2":
        f = bz2.BZ2File(str(filename), mode='w')
    else:
        f = open(str(filename), mode='wb')
    f.write(text.encode("utf-8"))
    f.close()

# ----------------------------------------------------------------------

def backup_file(filename):
    newname = filename
    suffixes = "".join(filename.suffixes)
    prefix = str(filename)[:-len(suffixes)]
    version = 1
    while newname.exists():
        newname = Path('{}.~{:03d}~{}'.format(prefix, version, suffixes))
        version += 1
    if newname != filename:
        try:
            shutil.copyfile(str(filename), str(newname))
        except Exception as err:
            module_logger.warning('Cannot create backup copy of {}: {}'.format(filename, err), exc_info=True)

# ----------------------------------------------------------------------

def is_chinese(name):
    first = ord(name[0])
    return first >= 0x3400 and first <= 0x9FFFF;

# ======================================================================
### Local Variables:
### eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
### End:
