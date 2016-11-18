#pragma once

// #include <iostream>
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
