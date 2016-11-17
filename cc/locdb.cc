#include "locdb.hh"
#include "export.hh"

// ----------------------------------------------------------------------

void LocDb::importFrom(std::string aFilename)
{
    locdb_import(aFilename, *this);

} // LocDb::importFrom

// ----------------------------------------------------------------------

void LocDb::exportTo(std::string aFilename, bool aPretty) const
{
    if (aPretty)
        locdb_export_pretty(aFilename, *this);
    else
        locdb_export(aFilename, *this);

} // LocDb::exportTo

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
