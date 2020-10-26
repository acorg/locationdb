#include <iostream>
#include <stack>

#include "acmacs-base/json-writer.hh"
#include "acmacs-base/json-reader.hh"

#include "export.hh"

// ----------------------------------------------------------------------

// void acmacs::locationdb::v1::export(std::string_view /*aFilename*/, const LocDb& /*aLocDb*/)
// {

// } // locdb_export

// // ----------------------------------------------------------------------

// void acmacs::locationdb::v1::export_pretty(std::string_view /*aFilename*/, const LocDb& /*aLocDb*/)
// {

// } // locdb_export_pretty

// ----------------------------------------------------------------------

using HandlerBase = json_reader::HandlerBase<LocDb>;
using StringMappingHandler = json_reader::StringMappingHandler<LocDb>;
using ContinentsHandler = json_reader::StringListHandler<LocDb>;

// ----------------------------------------------------------------------

class CountriesHandler : public HandlerBase
{
 public:
    CountriesHandler(LocDb& aLocDb, acmacs::locationdb::v1::Countries& aData)
        : HandlerBase(aLocDb), mData(aData), mStarted(false) {}

    HandlerBase* StartObject() override
        {
            if (mStarted)
                throw json_reader::Failure();
            mStarted = true;
            return nullptr;
        }

    HandlerBase* Key(const char* str, rapidjson::SizeType length) override
        {
            mKey.assign(str, length);
            return nullptr;
        }

    HandlerBase* Uint(unsigned u) override
        {
            if (mKey.empty())
                throw json_reader::Failure();
            mData.emplace_back(mKey, u);
            return nullptr;
        }

 private:
    acmacs::locationdb::v1::Countries& mData;
    bool mStarted;
    std::string mKey;

}; // class CountriesHandler

// ----------------------------------------------------------------------

class LocationsHandler : public HandlerBase
{
 public:
    LocationsHandler(LocDb& aLocDb, acmacs::locationdb::v1::Locations& aData)
        : HandlerBase(aLocDb), mData(aData), mStarted(false), mIndex(5) {}

    HandlerBase* StartObject() override
        {
            if (mStarted)
                throw json_reader::Failure();
            mStarted = true;
            return nullptr;
        }

    HandlerBase* StartArray() override
        {
            if (mKey.empty() || mIndex != 5)
                throw json_reader::Failure();
            mIndex = 0;
            return nullptr;
        }

    HandlerBase* EndArray() override
        {
            if (mKey.empty() || mIndex != 4)
                throw json_reader::Failure();
            mData.emplace_back(mKey, acmacs::locationdb::v1::LocationEntry(mLatitude, mLongitude, mCountry, mDivision));
            mIndex = 5;
            return nullptr;
        }

    HandlerBase* Key(const char* str, rapidjson::SizeType length) override
        {
            mKey.assign(str, length);
            return nullptr;
        }

    HandlerBase* Double(double d) override
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

    HandlerBase* String(const char* str, rapidjson::SizeType length) override
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
    acmacs::locationdb::v1::Locations& mData;
    bool mStarted;
    std::string mKey;
    size_t mIndex;
    acmacs::locationdb::v1::Latitude mLatitude;
    acmacs::locationdb::v1::Longitude mLongitude;
    std::string mCountry;
    std::string mDivision;

}; // class LocationsHandler

// ----------------------------------------------------------------------

namespace acmacs::locationdb::inline v1
{
    class LocDbRootHandler : public HandlerBase
    {
      private:
        enum class Keys { Unknown, Version, Date, CdcAbbreviations, Continents, Countries, Locations, Names, Replacements };
        static const std::vector<std::pair<std::string, Keys>> sKeys;

      public:
        LocDbRootHandler(LocDb& aLocDb) : HandlerBase(aLocDb), mKey(Keys::Unknown) {}

        HandlerBase* Key(const char* str, rapidjson::SizeType length) override
        {
            HandlerBase* result = nullptr;
            Keys new_key = Keys::Unknown;
            const std::string found_key(str, length);
            for (const auto& key : sKeys) {
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

        HandlerBase* String(const char* str, rapidjson::SizeType length) override
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

    }; // class LocDbRootHandler

} // namespace acmacs::locationdb::inline v1

// ----------------------------------------------------------------------

#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wexit-time-destructors"
#pragma GCC diagnostic ignored "-Wglobal-constructors"
#endif
const std::vector<std::pair<std::string, acmacs::locationdb::v1::LocDbRootHandler::Keys>> acmacs::locationdb::v1::LocDbRootHandler::sKeys {
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

void acmacs::locationdb::v1::import(std::string_view aFilename, LocDb& aLocDb, locdb_suppress_error suppress_error)
{
    try {
        json_reader::read_from_file<LocDb, LocDbRootHandler>(aFilename, aLocDb);
    }
    catch (std::exception& err) {
        if (suppress_error == locdb_suppress_error::no && !std::getenv("R_HOME")) // do not report if running under R
            std::cerr << "WARNING: (ignored) locdb not available: " << err.what() << '\n';
    }

} // acmacs::locationdb::v1::import

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
