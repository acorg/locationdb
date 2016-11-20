#pragma once

#include <iostream>
#include <string>
#include <vector>

// ----------------------------------------------------------------------

// abbreviation, name
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
    inline LocationEntry() = default;
    inline LocationEntry(Latitude aLatitude, Longitude aLongitude, std::string aCountry, std::string aDivision)
        : mLatitude(aLatitude), mLongitude(aLongitude), mCountry(aCountry), mDivision(aDivision) {}

    inline bool empty() const { return mCountry.empty(); }

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

class LocDb
{
 public:
    inline LocDb() = default;

    void importFrom(std::string aFilename);
    void exportTo(std::string aFilename, bool aPretty) const;

    std::string find_name(std::string aName, bool aHandleReplacement=true) const;
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

template <typename Value> inline const Value& find_indexed_by_name(const std::vector<std::pair<std::string, Value>>& aData, std::string aName)
{
    static const Value empty;
    const auto it = std::lower_bound(aData.begin(), aData.end(), aName, [](const auto& entry, const auto& look_for) -> bool { return entry.first < look_for; });
    if (it != aData.end())
        std::cerr << aName << "--" << it->first << "--" << it->second << std::endl;
    else
        std::cerr << aName << "--" << "END" << "--" << aData.size() << std::endl;
    return it == aData.end() || it->first != aName ? empty : it->second;
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
