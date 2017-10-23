#include <cstdlib>

#include "acmacs-base/string.hh"
#include "acmacs-base/debug.hh"

#include "locdb.hh"
#include "export.hh"

using namespace std::string_literals;

// ----------------------------------------------------------------------

#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wglobal-constructors"
#pragma GCC diagnostic ignored "-Wexit-time-destructors"
#endif

static std::unique_ptr<LocDb> sLocDb;
static std::string sLocDbFilename = std::getenv("HOME") + "/AD/data/locationdb.json.xz"s;

#pragma GCC diagnostic pop

void locdb_setup(std::string aFilename)
{
    sLocDbFilename = aFilename;
}

const LocDb& get_locdb(report_time timer)
{
    if (!sLocDb) {
        sLocDb = std::make_unique<LocDb>();
        sLocDb->importFrom(sLocDbFilename, timer);
    }
    return *sLocDb;

} // get_locdb

// ----------------------------------------------------------------------

void LocDb::importFrom(std::string aFilename, report_time timer)
{
    Timeit timeit("DEBUG: locdb loading: ", timer);
    locdb_import(aFilename, *this);

} // LocDb::importFrom

// ----------------------------------------------------------------------

void LocDb::exportTo(std::string aFilename, bool aPretty, report_time timer) const
{
    Timeit timeit("locdb exporting: ", timer);
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
      // std::cerr << "DEBUG: LocDb::find " << aName << DEBUG_LINE_FUNC << '\n';
    std::string name = aName;
    std::string replacement;
    std::string location_name;
    try {
        location_name = find_indexed_by_name(mNames, name);
    }
    catch (LocationNotFound&) {
        if (name[0] == '#') {
            name.erase(0, 1);
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

std::string LocDb::abbreviation(std::string aName) const
{
      // if it's in USA, use CDC abbreviation (if available)
      // if aName has multiple words, use first letters of words in upper case
      // otherwise use two letter of aName capitalized
    std::string abbreviation;
    try {
        const auto found = find(aName);
        if (found.country() == "UNITED STATES OF AMERICA")
            abbreviation = mCdcAbbreviations.find_abbreviation_by_name(found.location_name);
        if (abbreviation.empty())
            aName = found.name;
    }
    catch (LocationNotFound&) {
    }
    if (abbreviation.empty()) {
        abbreviation = string::first_letter_of_words(aName);
        if (abbreviation.size() == 1 && aName.size() > 1)
            abbreviation.push_back(static_cast<char>(tolower(aName[1])));
    }
    return abbreviation;

} // LocDb::abbreviation

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
