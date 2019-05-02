#pragma once

#include <string>

#include "locdb.hh"

// ----------------------------------------------------------------------

void locdb_export(std::string aFilename, const LocDb& aLocDb);
void locdb_export_pretty(std::string aFilename, const LocDb& aLocDb);
void locdb_import(std::string aFilename, LocDb& aLocDb, locdb_suppress_error suppress_error);

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
