#include <iostream>
#include <stack>
#include <typeinfo>
#include <memory>

#include "rapidjson/reader.h"
#include "rapidjson/error/en.h"

#include "json-writer.hh"

#include "export.hh"
#include "read-file.hh"

// ----------------------------------------------------------------------

void locdb_export(std::string /*aFilename*/, const LocDb& /*aLocDb*/)
{

} // locdb_export

// ----------------------------------------------------------------------

void locdb_export_pretty(std::string /*aFilename*/, const LocDb& /*aLocDb*/)
{

} // locdb_export_pretty

// ----------------------------------------------------------------------

class Error : public std::runtime_error { public: using std::runtime_error::runtime_error; };
class Failure : public std::exception { public: using std::exception::exception; };
class Pop : public std::exception { public: using std::exception::exception; };

class HandlerBase
{
 public:
    inline HandlerBase(LocDb& aLocDb) : mLocDb(aLocDb), mIgnore(false) {}
    virtual ~HandlerBase();

    inline virtual HandlerBase* StartObject() { std::cerr << "HandlerBase StartObject " << typeid(*this).name() << std::endl; throw Failure(); }
    inline virtual HandlerBase* EndObject() { throw Pop(); }
    inline virtual HandlerBase* StartArray() { throw Failure(); }
    inline virtual HandlerBase* EndArray() { throw Pop(); }
    inline virtual HandlerBase* Double(double d) { std::cerr << "Double: " << d << std::endl; throw Failure(); }
    inline virtual HandlerBase* Int(int i) { std::cerr << "Int: " << i << std::endl; throw Failure(); }
    inline virtual HandlerBase* Uint(unsigned u) { std::cerr << "Uint: " << u << std::endl; throw Failure(); }

    inline virtual HandlerBase* Key(const char* str, rapidjson::SizeType length)
        {
            if ((length == 1 && *str == '_') || (length > 0 && *str == '?')) {
                mIgnore = true;
            }
            else {
                std::cerr << "Key: \"" << std::string(str, length) << '"' << std::endl;
                throw Failure();
            }
            return nullptr;
        }

    inline virtual HandlerBase* String(const char* str, rapidjson::SizeType length)
        {
            if (mIgnore) {
                mIgnore = false;
            }
            else {
                std::cerr << "String: \"" << std::string(str, length) << '"' << std::endl;
                throw Failure();
            }
            return nullptr;
        }

 protected:
    LocDb& mLocDb;
    bool mIgnore;
};

HandlerBase::~HandlerBase()
{
}

// ----------------------------------------------------------------------

class StringMappingHandler : public HandlerBase
{
 public:
    inline StringMappingHandler(LocDb& aLocDb, std::vector<std::pair<std::string, std::string>>& aMapping)
        : HandlerBase(aLocDb), mMapping(aMapping), mStarted(false) {}

    inline virtual HandlerBase* StartObject()
        {
            if (mStarted)
                throw Failure();
            mStarted = true;
            return nullptr;
        }

    inline virtual HandlerBase* Key(const char* str, rapidjson::SizeType length)
        {
            mKey.assign(str, length);
            return nullptr;
        }

    inline virtual HandlerBase* String(const char* str, rapidjson::SizeType length)
        {
            if (mKey.empty())
                throw Failure();
            mMapping.emplace_back(mKey, std::string(str, length));
            mKey.erase();
            return nullptr;
        }

 private:
    std::vector<std::pair<std::string, std::string>>& mMapping;
    bool mStarted;
    std::string mKey;

}; // class StringMappingHandler

// ----------------------------------------------------------------------

class ContinentsHandler : public HandlerBase
{
 public:
    inline ContinentsHandler(LocDb& aLocDb, Continents& aData)
        : HandlerBase(aLocDb), mData(aData), mStarted(false) {}

    inline virtual HandlerBase* StartArray()
        {
            if (mStarted)
                throw Failure();
            mStarted = true;
            return nullptr;
        }

    inline virtual HandlerBase* String(const char* str, rapidjson::SizeType length)
        {
            mData.emplace_back(str, length);
            return nullptr;
        }

 private:
    Continents& mData;
    bool mStarted;

}; // class StringMappingHandler

// ----------------------------------------------------------------------

class CountriesHandler : public HandlerBase
{
 public:
    inline CountriesHandler(LocDb& aLocDb, Countries& aData)
        : HandlerBase(aLocDb), mData(aData), mStarted(false) {}

    inline virtual HandlerBase* StartObject()
        {
            if (mStarted)
                throw Failure();
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
                throw Failure();
            mData.emplace_back(mKey, u);
            return nullptr;
        }

 private:
    Countries& mData;
    bool mStarted;
    std::string mKey;

}; // class StringMappingHandler

// ----------------------------------------------------------------------

class LocationsHandler : public HandlerBase
{
 public:
    inline LocationsHandler(LocDb& aLocDb, Locations& aData)
        : HandlerBase(aLocDb), mData(aData), mStarted(false), mIndex(5) {}

    inline virtual HandlerBase* StartObject()
        {
            if (mStarted)
                throw Failure();
            mStarted = true;
            return nullptr;
        }

    inline virtual HandlerBase* StartArray()
        {
            if (mKey.empty() || mIndex != 5)
                throw Failure();
            mIndex = 0;
            return nullptr;
        }

    inline virtual HandlerBase* EndArray()
        {
            if (mKey.empty() || mIndex != 4)
                throw Failure();
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
                throw Failure();
            switch (mIndex) {
              case 0:
                  mLatitude = d;
                  break;
              case 1:
                  mLongitude = d;
                  break;
              default:
                throw Failure();
            }
            ++mIndex;
            return nullptr;
        }

    inline virtual HandlerBase* String(const char* str, rapidjson::SizeType length)
        {
            if (mKey.empty())
                throw Failure();
            switch (mIndex) {
              case 2:
                  mCountry.assign(str, length);
                  break;
              case 3:
                  mDivision.assign(str, length);
                  break;
              default:
                throw Failure();
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

}; // class StringMappingHandler

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
                  result = new StringMappingHandler(mLocDb, mLocDb.mCdcAbbreviations);
                  break;
              case Keys::Continents:
                  result = new ContinentsHandler(mLocDb, mLocDb.mContinents);
                  break;
              case Keys::Countries:
                  result = new CountriesHandler(mLocDb, mLocDb.mCountries);
                  break;
              case Keys::Locations:
                  result = new LocationsHandler(mLocDb, mLocDb.mLocations);
                  break;
              case Keys::Names:
                  result = new StringMappingHandler(mLocDb, mLocDb.mNames);
                  break;
              case Keys::Replacements:
                  result = new StringMappingHandler(mLocDb, mLocDb.mReplacements);
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
                      throw Failure();
                  }
                  break;
              case Keys::Date:
                  mLocDb.mDate.assign(str, length);
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

class DocRootHandler : public HandlerBase
{
 public:
    inline DocRootHandler(LocDb& aLocDb) : HandlerBase(aLocDb) {}

    inline virtual HandlerBase* StartObject() { return new LocDbRootHandler(mLocDb); }
};

// ----------------------------------------------------------------------

class LocDbReaderEventHandler : public rapidjson::BaseReaderHandler<rapidjson::UTF8<>, LocDbReaderEventHandler>
{
 private:
    template <typename... Args> inline bool handler(HandlerBase* (HandlerBase::*aHandler)(Args... args), Args... args)
        {
            try {
                auto new_handler = ((*mHandler.top()).*aHandler)(args...);
                if (new_handler)
                    mHandler.emplace(new_handler);
            }
            catch (Pop&) {
                if (mHandler.empty())
                    return false;
                mHandler.pop();
            }
            catch (Failure&) {
                return false;
            }
            return true;
        }

 public:
    inline LocDbReaderEventHandler(LocDb& aLocDb)
        : mLocDb(aLocDb)
        {
            mHandler.emplace(new DocRootHandler(mLocDb));
        }

    inline bool StartObject() { return handler(&HandlerBase::StartObject); }
    inline bool EndObject(rapidjson::SizeType /*memberCount*/) { return handler(&HandlerBase::EndObject); }
    inline bool StartArray() { return handler(&HandlerBase::StartArray); }
    inline bool EndArray(rapidjson::SizeType /*elementCount*/) { return handler(&HandlerBase::EndArray); }
    inline bool Key(const char* str, rapidjson::SizeType length, bool /*copy*/) { return handler(&HandlerBase::Key, str, length); }
    inline bool String(const Ch* str, rapidjson::SizeType length, bool /*copy*/) { return handler(&HandlerBase::String, str, length); }
    inline bool Int(int i) { return handler(&HandlerBase::Int, i); }
    inline bool Uint(unsigned u) { return handler(&HandlerBase::Uint, u); }
    inline bool Double(double d) { return handler(&HandlerBase::Double, d); }

      // inline bool Bool(bool /*b*/) { return false; }
      // inline bool Null() { std::cout << "Null()" << std::endl; return false; }
      // inline bool Int64(int64_t i) { std::cout << "Int64(" << i << ")" << std::endl; return false; }
      // inline bool Uint64(uint64_t u) { std::cout << "Uint64(" << u << ")" << std::endl; return false; }

 private:
    LocDb& mLocDb;
    std::stack<std::unique_ptr<HandlerBase>> mHandler;

      // ----------------------------------------------------------------------

    friend void hidb_import(std::string, LocDb&);
};

// ----------------------------------------------------------------------

void locdb_import(std::string buffer, LocDb& aLocDb)
{
    if (buffer == "-")
        buffer = read_stdin();
    else
        buffer = read_file(buffer);
    if (buffer[0] == '{') { // && buffer.find("\"  version\": \"hidb-v4\"") != std::string::npos) {
        LocDbReaderEventHandler handler{aLocDb};
        rapidjson::Reader reader;
        rapidjson::StringStream ss(buffer.c_str());
        reader.Parse(ss, handler);
        if (reader.HasParseError())
            throw Error("cannot import locationdb: data parsing failed at pos " + std::to_string(reader.GetErrorOffset()) + ": " +  GetParseError_En(reader.GetParseErrorCode()) + "\n" + buffer.substr(reader.GetErrorOffset(), 50));
          // std::cout << aLocDb.stat() << std::endl;
        // if (!handler.in_init_state())
        //     throw Error("internal: not in init state on parsing completion");
    }
    else
        throw std::runtime_error("cannot import locationdb: unrecognized source format");

} // locdb_import

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
