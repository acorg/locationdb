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

LookupResult LocDb::find(std::string aName) const
{
    std::string name = aName;
    std::string replacement;
    std::string location_name;
    try {
        location_name = find_indexed_by_name(mNames, name);
    }
    catch (LocationNotFound&) {
        if (name[0] == '#') {
            name.erase(0, 1);
            std::cerr << "Look for cdc abbreviation: " << name << std::endl;
            location_name = find_indexed_by_name(mCdcAbbreviations, name);
        }
        else {
            replacement = find_indexed_by_name(mReplacements, name);
            name = replacement;
            location_name = find_indexed_by_name(mNames, name);
        }
    }
    return LookupResult(aName, replacement, name, location_name, find_indexed_by_name(mLocations, location_name));

} // LocDb::find

// ----------------------------------------------------------------------

LookupResult LocDb::find_cdc_abbreviation(std::string aAbbreviation) const
{
    if (aAbbreviation[0] == '#')
        aAbbreviation.erase(0, 1);
    const std::string location_name = find_indexed_by_name(mCdcAbbreviations, aAbbreviation);
    return LookupResult(aAbbreviation, std::string(), aAbbreviation, location_name, find_indexed_by_name(mLocations, location_name));

} // LocDb::find_cdc_abbreviation

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
