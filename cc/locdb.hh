#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include <optional>

#include "acmacs-base/timeit.hh"
#include "acmacs-base/string.hh"

// ----------------------------------------------------------------------

class LocationNotFound : public std::runtime_error { public: using std::runtime_error::runtime_error; };

enum class locdb_suppress_error { no, yes };

// ----------------------------------------------------------------------

namespace detail
{
    template <typename Value> inline auto find_indexed_by_name_no_fixes(const std::vector<std::pair<std::string, Value>>& aData, std::string aName) -> std::optional<decltype(aData.begin())>
    {
        if (const auto it = std::lower_bound(aData.begin(), aData.end(), aName, [](const auto& entry, const auto& look_for) -> bool { return entry.first < look_for; });
            it != aData.end() && it->first == aName)
            return it;
        else
            return std::nullopt;
    }

    template <typename Value> inline const Value& find_indexed_by_name(const std::vector<std::pair<std::string, Value>>& aData, std::string aName)
    {
        if (const auto it = find_indexed_by_name_no_fixes(aData, aName); it.has_value())
            return it.value()->second;
        if (aName.find('_') != std::string::npos)
            return find_indexed_by_name(aData, string::replace(aName, '_', ' ')); // non-acmacs names may have _ instead of space, e.g. NEW_YORK
        if (aName.find('-') != std::string::npos)
            return find_indexed_by_name(aData, string::replace(aName, '-', ' ')); // non-acmacs names may have - instead of space, e.g. NEW-YORK
        throw LocationNotFound(aName);
    }
} // namespace detail

// ----------------------------------------------------------------------

// Abbreviation, name
class CdcAbbreviations : public std::vector<std::pair<std::string, std::string>>
{
 public:
    std::string find_abbreviation_by_name(std::string aName) const
        {
            std::string result; // empty if not found
            const auto found = std::find_if(begin(), end(), [&aName](const auto& e) -> bool { return e.second == aName; });
            if (found != end())
                result = found->first;
            return result;
        }
};

// ----------------------------------------------------------------------

class Continents : public std::vector<std::string>
{
};

// ----------------------------------------------------------------------

// country, index in continents
class Countries : public std::vector<std::pair<std::string, size_t>>
{
};

// ----------------------------------------------------------------------

typedef double Latitude;
typedef double Longitude;

class LocationEntry
{
 public:
    LocationEntry() : mLatitude(90), mLongitude(0) {}
    LocationEntry(Latitude aLatitude, Longitude aLongitude, std::string aCountry, std::string aDivision)
        : mLatitude(aLatitude), mLongitude(aLongitude), mCountry(aCountry), mDivision(aDivision) {}

    bool empty() const { return mCountry.empty(); }
    Latitude latitude() const { return mLatitude; }
    Longitude longitude() const { return mLongitude; }
    std::string country() const { return mCountry; }
    std::string division() const { return mDivision; }

 private:
    Latitude mLatitude;
    Longitude mLongitude;
    std::string mCountry;
    std::string mDivision;
};

class Locations : public std::vector<std::pair<std::string, LocationEntry>>
{
};

// ----------------------------------------------------------------------

// name, name in locations
class Names : public std::vector<std::pair<std::string, std::string>>
{
};

// ----------------------------------------------------------------------

// name, name in names
class Replacements : public std::vector<std::pair<std::string, std::string>>
{
};

// ----------------------------------------------------------------------

class LookupResult
{
 public:
    LookupResult(const LookupResult&) = default;
    LookupResult(LookupResult&&) = default;

    const std::string look_for;
    const std::string replacement;
    const std::string name;           // the same as normalized look_for or replacement
    const std::string location_name;  // location entry name
    const LocationEntry& location;

    Latitude latitude() const { return location.latitude(); }
    Longitude longitude() const { return location.longitude(); }
    std::string country() const { return location.country(); }
    std::string division() const { return location.division(); }

 private:
    LookupResult(std::string a_look_for, std::string a_replacement, std::string a_name, std::string a_location_name, const LocationEntry& a_location)
        : look_for(a_look_for), replacement(a_replacement), name(a_name), location_name(a_location_name), location(a_location) {}
    friend class LocDb;
};

// ----------------------------------------------------------------------

class LocDb
{
 public:
    LocDb() = default;

    void importFrom(std::string aFilename, locdb_suppress_error suppress_error, report_time timer = report_time::no);
    void exportTo(std::string aFilename, bool aPretty, report_time timer = report_time::no) const;

    bool empty() const { return mNames.empty(); }
    operator bool() const { return !empty(); }

      // If aName starts with # - it is cdc abbreviation
    LookupResult find(std::string aName) const;
    LookupResult find_cdc_abbreviation(std::string aAbbreviation) const;
    std::string continent_of_country(std::string aCountry) const { return mContinents[detail::find_indexed_by_name(mCountries, aCountry)]; }
    std::string abbreviation(std::string aName) const;

    std::string country(std::string aName, std::string for_not_found = std::string()) const
        {
            try {
                return find(aName).country();
            }
            catch (LocationNotFound&) {
                if (for_not_found.empty())
                    throw;
                return for_not_found;
            }
        }

    std::string continent(std::string aName, std::string for_not_found = std::string()) const
        {
            try {
                return continent_of_country(find(aName).country());
            }
            catch (LocationNotFound&) {
                if (for_not_found.empty())
                    throw;
                return for_not_found;
            }
        }

    std::string stat() const;

    const auto& names() const { return mNames; }
    const auto& replacements() const { return mReplacements; }

 private:
    std::string mDate;
    CdcAbbreviations mCdcAbbreviations;
    Continents mContinents;
    Countries mCountries;
    Locations mLocations;
    Names mNames;
    Replacements mReplacements;

    friend class LocDbRootHandler;
};

// not thread safe!
void locdb_setup(std::string aFilename, bool aVerbose);

// not thread safe!
const LocDb& get_locdb(locdb_suppress_error suppress_error = locdb_suppress_error::no, report_time timer = report_time::no);

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
