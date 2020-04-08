#include <cstdlib>
#include <cctype>

#include "acmacs-base/acmacsd.hh"
// #include "acmacs-base/string.hh"
#include "acmacs-base/debug.hh"
#include "acmacs-base/fmt.hh"

#include "locdb.hh"
#include "export.hh"

// ----------------------------------------------------------------------

#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wglobal-constructors"
#pragma GCC diagnostic ignored "-Wexit-time-destructors"
#endif

static std::unique_ptr<LocDb> sLocDb;
static std::string sLocDbFilename = acmacs::acmacsd_root() + "/data/locationdb.json.xz";
static bool sVerbose = false;

#pragma GCC diagnostic pop

// not thread safe!
void locdb_setup(std::string_view aFilename, bool aVerbose)
{
    sVerbose = aVerbose;
    if (!aFilename.empty())
        sLocDbFilename = aFilename;
}

// not thread safe!
const LocDb& get_locdb(locdb_suppress_error suppress_error, report_time timer)
{
    if (!sLocDb) {
        sLocDb = std::make_unique<LocDb>();
        sLocDb->importFrom(sLocDbFilename, suppress_error, sVerbose ? report_time::yes : timer);
    }
    return *sLocDb;

} // get_locdb

// ----------------------------------------------------------------------

void LocDb::importFrom(std::string_view aFilename, locdb_suppress_error suppress_error, report_time timer)
{
    Timeit timeit(fmt::format("DEBUG: LocDb loading from {}: ", aFilename), timer);
    locdb_import(aFilename, *this, suppress_error);

} // LocDb::importFrom

// ----------------------------------------------------------------------

void LocDb::exportTo(std::string_view aFilename, bool aPretty, report_time timer) const
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
    return fmt::format("continents:{} countries:{} locations:{} names:{} cdc-abbr:{} replacements:{}",
                       mContinents.size(), mCountries.size(), mLocations.size(), mNames.size(), mCdcAbbreviations.size(), mReplacements.size());

} // LocDb::stat

// ----------------------------------------------------------------------

LookupResult LocDb::find(std::string_view aName) const
{
    std::string name{aName};
    std::string replacement;
    std::string location_name;
    try {
        if (const auto it = detail::find_indexed_by_name_no_fixes(mNames, aName); it.has_value())
            location_name = it.value()->second;
        else
            throw LocationNotFound{name};
    }
    catch (LocationNotFound&) {
        // try {
        if (name[0] == '#') {
            name.erase(0, 1);
            location_name = detail::find_indexed_by_name(mCdcAbbreviations, name);
        }
        else {
            try {
                replacement = detail::find_indexed_by_name(mReplacements, name);
                name = replacement;
                location_name = detail::find_indexed_by_name(mNames, name);
            }
            catch (LocationNotFound&) {
                const auto find_with_replacement = [&](char to_replace) -> bool {
                    if (aName.find(to_replace) != std::string::npos) {
                        replacement = string::replace(aName, to_replace, ' ');
                        const auto intermediate_result = find(replacement);
                        if (!intermediate_result.replacement.empty())
                            replacement = intermediate_result.replacement;
                        location_name = intermediate_result.location_name;
                        name = replacement;
                        return true;
                    }
                    return false;
                };

                if (!find_with_replacement('_') && !find_with_replacement('-'))
                    throw;
            }
        }
        // }
        // catch (LocationNotFound& err) {
        //     std::cerr << "LocDb find: not found: " << err.what() << '\n';
        //     throw;
        // }
    }
    return LookupResult(aName, replacement, name, location_name, detail::find_indexed_by_name(mLocations, location_name));

} // LocDb::find

// ----------------------------------------------------------------------

LookupResult LocDb::find_cdc_abbreviation(std::string_view aAbbreviation) const
{
    if (aAbbreviation[0] == '#')
        aAbbreviation.remove_prefix(1);
    const std::string location_name = detail::find_indexed_by_name(mCdcAbbreviations, aAbbreviation);
    return LookupResult(aAbbreviation, std::string(), aAbbreviation, location_name, detail::find_indexed_by_name(mLocations, location_name));

} // LocDb::find_cdc_abbreviation

// ----------------------------------------------------------------------

std::string LocDb::abbreviation(std::string_view aName) const
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
