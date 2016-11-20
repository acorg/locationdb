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

std::string LocDb::stat() const
{
    return "continents:" + std::to_string(mContinents.size())
            + " countries:" + std::to_string(mCountries.size())
            + " locations:" + std::to_string(mLocations.size())
            + " names:" + std::to_string(mNames.size())
            + " cdc-abbr:" + std::to_string(mCdcAbbreviations.size())
            + " replacements:" + std::to_string(mReplacements.size());

} // LocDb::stat

// ----------------------------------------------------------------------

std::string LocDb::find_name(std::string aName, bool aHandleReplacement) const
{
      // name = name.upper()
      // ns = name.strip()
    std::string in_names = find_indexed_by_name(mNames, aName);
    if (in_names.empty()) {
    }
    return in_names;

    // if (!in_names.empty()) {
    //     const auto& in_locations = find_indexed_by_name(mLocations, in_names);

    // }

} // LocDb::find_name

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
