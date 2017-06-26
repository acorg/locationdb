#include <iostream>
#include <stack>

#pragma GCC diagnostic push
#include "acmacs-base/rapidjson-diagnostics.hh"
#include "rapidjson/reader.h"
#include "rapidjson/error/en.h"
#pragma GCC diagnostic pop

#include "acmacs-base/json-writer.hh"
#include "acmacs-base/json-reader.hh"

#include "export.hh"

// ----------------------------------------------------------------------

void locdb_export(std::string /*aFilename*/, const LocDb& /*aLocDb*/)
{

} // locdb_export

// ----------------------------------------------------------------------

void locdb_export_pretty(std::string /*aFilename*/, const LocDb& /*aLocDb*/)
{

} // locdb_export_pretty

// ----------------------------------------------------------------------

using HandlerBase = json_reader::HandlerBase<LocDb>;
using StringMappingHandler = json_reader::StringMappingHandler<LocDb>;
using ContinentsHandler = json_reader::StringListHandler<LocDb>;

// ----------------------------------------------------------------------

class CountriesHandler : public HandlerBase
{
 public:
    inline CountriesHandler(LocDb& aLocDb, Countries& aData)
        : HandlerBase(aLocDb), mData(aData), mStarted(false) {}

    inline virtual HandlerBase* StartObject()
        {
            if (mStarted)
                throw json_reader::Failure();
            mStarted = true;
            return nullptr;
        }

    inline virtual HandlerBase* Key(const char* str, rapidjson::SizeType length)
        {
            mKey.assign(str, length);
            return nullptr;
        }

    inline virtual HandlerBase* Uint(unsigned u)
        {
            if (mKey.empty())
                throw json_reader::Failure();
            mData.emplace_back(mKey, u);
            return nullptr;
        }

 private:
    Countries& mData;
    bool mStarted;
    std::string mKey;

}; // class CountriesHandler

// ----------------------------------------------------------------------

class LocationsHandler : public HandlerBase
{
 public:
    inline LocationsHandler(LocDb& aLocDb, Locations& aData)
        : HandlerBase(aLocDb), mData(aData), mStarted(false), mIndex(5) {}

    inline virtual HandlerBase* StartObject()
        {
            if (mStarted)
                throw json_reader::Failure();
            mStarted = true;
            return nullptr;
        }

    inline virtual HandlerBase* StartArray()
        {
            if (mKey.empty() || mIndex != 5)
                throw json_reader::Failure();
            mIndex = 0;
            return nullptr;
        }

    inline virtual HandlerBase* EndArray()
        {
            if (mKey.empty() || mIndex != 4)
                throw json_reader::Failure();
            mData.emplace_back(mKey, LocationEntry(mLatitude, mLongitude, mCountry, mDivision));
            mIndex = 5;
            return nullptr;
        }

    inline virtual HandlerBase* Key(const char* str, rapidjson::SizeType length)
        {
            mKey.assign(str, length);
            return nullptr;
        }

    inline virtual HandlerBase* Double(double d)
        {
            if (mKey.empty())
                throw json_reader::Failure();
            switch (mIndex) {
              case 0:
                  mLatitude = d;
                  break;
              case 1:
                  mLongitude = d;
                  break;
              default:
                throw json_reader::Failure();
            }
            ++mIndex;
            return nullptr;
        }

    inline virtual HandlerBase* String(const char* str, rapidjson::SizeType length)
        {
            if (mKey.empty())
                throw json_reader::Failure();
            switch (mIndex) {
              case 2:
                  mCountry.assign(str, length);
                  break;
              case 3:
                  mDivision.assign(str, length);
                  break;
              default:
                throw json_reader::Failure();
            }
            ++mIndex;
            return nullptr;
        }

 private:
    Locations& mData;
    bool mStarted;
    std::string mKey;
    size_t mIndex;
    Latitude mLatitude;
    Longitude mLongitude;
    std::string mCountry;
    std::string mDivision;

}; // class LocationsHandler

// ----------------------------------------------------------------------

class LocDbRootHandler : public HandlerBase
{
 private:
    enum class Keys { Unknown, Version, Date, CdcAbbreviations, Continents, Countries, Locations, Names, Replacements };
    static const std::vector<std::pair<std::string, Keys>> sKeys;

 public:
    inline LocDbRootHandler(LocDb& aLocDb) : HandlerBase(aLocDb), mKey(Keys::Unknown) {}

    inline virtual HandlerBase* Key(const char* str, rapidjson::SizeType length)
        {
            HandlerBase* result = nullptr;
            Keys new_key = Keys::Unknown;
            const std::string found_key(str, length);
            for (const auto& key: sKeys) {
                if (key.first == found_key) {
                    new_key = key.second;
                    break;
                }
            }
            mKey = Keys::Unknown;
            switch (new_key) {
              case Keys::Version:
              case Keys::Date:
                  mKey = new_key;
                  break;
              case Keys::CdcAbbreviations:
                  result = new StringMappingHandler(mTarget, mTarget.mCdcAbbreviations);
                  break;
              case Keys::Continents:
                  result = new ContinentsHandler(mTarget, mTarget.mContinents);
                  break;
              case Keys::Countries:
                  result = new CountriesHandler(mTarget, mTarget.mCountries);
                  break;
              case Keys::Locations:
                  result = new LocationsHandler(mTarget, mTarget.mLocations);
                  break;
              case Keys::Names:
                  result = new StringMappingHandler(mTarget, mTarget.mNames);
                  break;
              case Keys::Replacements:
                  result = new StringMappingHandler(mTarget, mTarget.mReplacements);
                  break;
              case Keys::Unknown:
                  result = HandlerBase::Key(str, length);
                  break;
            }
            return result;
        }

    inline virtual HandlerBase* String(const char* str, rapidjson::SizeType length)
        {
            HandlerBase* result = nullptr;
            switch (mKey) {
              case Keys::Version:
                  if (strncmp(str, "locationdb-v2", std::min(length, 13U))) {
                      std::cerr << "Unsupported version: \"" << std::string(str, length) << '"' << std::endl;
                      throw json_reader::Failure();
                  }
                  break;
              case Keys::Date:
                  mTarget.mDate.assign(str, length);
                  break;
              case Keys::CdcAbbreviations:
              case Keys::Continents:
              case Keys::Countries:
              case Keys::Locations:
              case Keys::Names:
              case Keys::Replacements:
              case Keys::Unknown:
                  result = HandlerBase::String(str, length);
                  break;
            }
            return result;
        }

 private:
    Keys mKey;

};

#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wexit-time-destructors"
#pragma GCC diagnostic ignored "-Wglobal-constructors"
#endif
const std::vector<std::pair<std::string, LocDbRootHandler::Keys>> LocDbRootHandler::sKeys {
    {"  version", Keys::Version},
    {" date", Keys::Date},
    {"cdc_abbreviations", Keys::CdcAbbreviations},
    {"continents", Keys::Continents},
    {"countries", Keys::Countries},
    {"locations", Keys::Locations},
    {"names", Keys::Names},
    {"replacements", Keys::Replacements}
};
#pragma GCC diagnostic pop

// ----------------------------------------------------------------------

void locdb_import(std::string aFilename, LocDb& aLocDb)
{
    json_reader::read_from_file<LocDb, LocDbRootHandler>(aFilename, aLocDb);

} // locdb_import

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
