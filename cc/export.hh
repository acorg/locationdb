#pragma once

#include "locationdb/locdb.hh"

// ----------------------------------------------------------------------

namespace acmacs::locationdb::inline v1
{
    // void export(std::string_view aFilename, const LocDb& aLocDb);
    // void export_pretty(std::string_view aFilename, const LocDb& aLocDb);
    void import(std::string_view aFilename, LocDb& aLocDb, locdb_suppress_error suppress_error);

} // namespace acmacs::locationdb::inline v1

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
