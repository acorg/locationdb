#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include <optional>

#include "acmacs-base/timeit.hh"
#include "acmacs-base/string.hh"

// ----------------------------------------------------------------------

namespace acmacs::locationdb::inline v1
{
    class LocationNotFound : public std::runtime_error
    {
      public:
        using std::runtime_error::runtime_error;
        LocationNotFound(std::string_view msg) : runtime_error(std::string(msg)) {}
    };

    enum class locdb_suppress_error { no, yes };

    // ----------------------------------------------------------------------

    // Abbreviation, name
    class CdcAbbreviations : public std::vector<std::pair<std::string, std::string>>
    {
      public:
        std::string find_abbreviation_by_name(std::string_view aName) const
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
        LocationEntry(Latitude aLatitude, Longitude aLongitude, std::string_view aCountry, std::string_view aDivision)
            : mLatitude(aLatitude), mLongitude(aLongitude), mCountry(aCountry), mDivision(aDivision)
        {
        }

        bool empty() const { return mCountry.empty(); }
        Latitude latitude() const { return mLatitude; }
        Longitude longitude() const { return mLongitude; }
        std::string_view country() const { return mCountry; }
        std::string_view division() const { return mDivision; }

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

    struct LookupResult
    {
        const std::string look_for;
        const std::string replacement;
        const std::string name;          // the same as normalized look_for or replacement
        const std::string location_name; // location entry name
        std::string continent;
        const LocationEntry& location;

        Latitude latitude() const { return location.latitude(); }
        Longitude longitude() const { return location.longitude(); }
        std::string_view country() const { return location.country(); }
        std::string_view division() const { return location.division(); }
    };

    // ----------------------------------------------------------------------

    enum class include_continent { no, yes };

    class LocDb
    {
      public:
        LocDb() = default;

        void importFrom(std::string_view aFilename, locdb_suppress_error suppress_error, report_time timer = report_time::no);
        // void exportTo(std::string_view aFilename, bool aPretty, report_time timer = report_time::no) const;

        bool empty() const { return mNames.empty(); }
        operator bool() const { return !empty(); }

        // If aName starts with # - it is cdc abbreviation
        std::optional<LookupResult> find(std::string_view aName, include_continent inc_continent = include_continent::no) const;
        LookupResult find_or_throw(std::string_view aName) const;
        LookupResult find_cdc_abbreviation(std::string_view aAbbreviation) const;
        std::string_view continent_of_country(std::string_view aCountry) const;
        std::string abbreviation(std::string_view aName) const;

        std::string_view country(std::string_view aName, std::string_view for_not_found = {}) const
        {
            try {
                return find_or_throw(aName).country();
            }
            catch (std::exception&) {
                if (for_not_found.empty())
                    throw;
                return for_not_found;
            }
        }

        std::string_view continent(std::string_view aName, std::string_view for_not_found = {}) const
        {
            try {
                return continent_of_country(find_or_throw(aName).country());
            }
            catch (std::exception&) {
                if (for_not_found.empty())
                    throw;
                return for_not_found;
            }
        }

        Latitude latitude(std::string_view aName, Latitude for_not_found = 360.0) const
        {
            try {
                return find_or_throw(aName).latitude();
            }
            catch (std::exception&) {
                return for_not_found;
            }
        }

        Longitude longitude(std::string_view aName, Longitude for_not_found = 360.0) const
        {
            try {
                return find_or_throw(aName).longitude();
            }
            catch (std::exception&) {
                return for_not_found;
            }
        }

        std::string stat() const;

        const auto& names() const { return mNames; }
        const auto& replacements() const { return mReplacements; }

        // deprecated
        std::optional<typename Names::const_iterator> find_by_name_no_fixes(std::string_view aName) const;
        std::optional<typename Replacements::const_iterator> find_by_replacement_no_fixes(std::string_view aName) const;

      private:
        std::string mDate;
        CdcAbbreviations mCdcAbbreviations;
        Continents mContinents;
        Countries mCountries;
        Locations mLocations;
        Names mNames;
        Replacements mReplacements;

        // LookupResult make_result(std::string_view look_for, std::string_view name, std::string_view found, include_continent inc_continent) const;
        // LookupResult make_result(std::string_view look_for, std::string_view found, include_continent inc_continent) const;

        friend class LocDbRootHandler;
    };

    // not thread safe!
    void setup(std::string_view aFilename, bool aVerbose);

    // not thread safe!
    const LocDb& get(locdb_suppress_error suppress_error = locdb_suppress_error::no, report_time timer = report_time::no);

} // namespace acmacs::locationdb::inline v1

// ----------------------------------------------------------------------

using LocDb = acmacs::locationdb::v1::LocDb;

// not thread safe!
inline void locdb_setup(std::string_view filename, bool verbose) { acmacs::locationdb::v1::setup(filename, verbose); }

// not thread safe!
inline const LocDb& get_locdb(acmacs::locationdb::v1::locdb_suppress_error suppress_error = acmacs::locationdb::v1::locdb_suppress_error::no, report_time timer = report_time::no)
{
    return acmacs::locationdb::v1::get(suppress_error, timer);
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
