#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

// ----------------------------------------------------------------------

class NotFound : public std::runtime_error { public: using std::runtime_error::runtime_error; };

// ----------------------------------------------------------------------

template <typename Value> inline const Value& find_indexed_by_name(const std::vector<std::pair<std::string, Value>>& aData, std::string aName)
{
    const auto it = std::lower_bound(aData.begin(), aData.end(), aName, [](const auto& entry, const auto& look_for) -> bool { return entry.first < look_for; });
    if (it == aData.end() || it->first != aName)
        throw NotFound(aName);
    return it->second;
}

// ----------------------------------------------------------------------

// Abbreviation, name
class CdcAbbreviations : public std::vector<std::pair<std::string, std::string>>
{
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
    inline LocationEntry() : mLatitude(90), mLongitude(0) {}
    inline LocationEntry(Latitude aLatitude, Longitude aLongitude, std::string aCountry, std::string aDivision)
        : mLatitude(aLatitude), mLongitude(aLongitude), mCountry(aCountry), mDivision(aDivision) {}

    inline bool empty() const { return mCountry.empty(); }
    inline Latitude latitude() const { return mLatitude; }
    inline Longitude longitude() const { return mLongitude; }
    inline std::string country() const { return mCountry; }
    inline std::string division() const { return mDivision; }

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
    inline LookupResult(const LookupResult&) = default;
    inline LookupResult(LookupResult&&) = default;

    const std::string look_for;
    const std::string replacement;
    const std::string name;           // the same as normalized look_for or replacement
    const std::string location_name;  // location entry name
    const LocationEntry& location;

    inline Latitude latitude() const { return location.latitude(); }
    inline Longitude longitude() const { return location.longitude(); }
    inline std::string country() const { return location.country(); }
    inline std::string division() const { return location.division(); }

 private:
    inline LookupResult(std::string a_look_for, std::string a_replacement, std::string a_name, std::string a_location_name, const LocationEntry& a_location)
        : look_for(a_look_for), replacement(a_replacement), name(a_name), location_name(a_location_name), location(a_location) {}
    friend class LocDb;
};

// ----------------------------------------------------------------------

class LocDb
{
 public:
    inline LocDb() = default;

    void importFrom(std::string aFilename);
    void exportTo(std::string aFilename, bool aPretty) const;

    LookupResult find(std::string aName) const;
    LookupResult find_cdc_abbreviation(std::string aAbbreviation) const;
    inline std::string continent_of_country(std::string aCountry) const { return mContinents[find_indexed_by_name(mCountries, aCountry)]; }

    inline std::string country(std::string aName) const { return find(aName).country(); }
    inline std::string continent(std::string aName) const { return continent_of_country(find(aName).country()); }

    std::string stat() const;

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

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
