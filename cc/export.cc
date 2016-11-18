#include <iostream>
#include <stack>

#include "rapidjson/reader.h"
#include "rapidjson/error/en.h"

#include "json-writer.hh"

#include "export.hh"
#include "read-file.hh"

// ----------------------------------------------------------------------

void locdb_export(std::string aFilename, const LocDb& aLocDb)
{

} // locdb_export

// ----------------------------------------------------------------------

void locdb_export_pretty(std::string aFilename, const LocDb& aLocDb)
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

    inline virtual HandlerBase* StartObject() { throw Failure(); }
    inline virtual HandlerBase* EndObject() { throw Pop(); }
    inline virtual HandlerBase* StartArray() { throw Failure(); }
    inline virtual HandlerBase* EndArray() { throw Pop(); }
    inline virtual HandlerBase* Double(double d) { std::cerr << "Double: " << d << std::endl; throw Failure(); }

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
    inline StringMappingHandler(LocDb& aLocDb, std::vector<std::pair<std::string, std::string>>& aMapping) : HandlerBase(aLocDb), mMapping(aMapping), mStarted(false) {}

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
            mMapping.emplace_back(mKey, std::string(str, length));
            return nullptr;
        }

 private:
    std::vector<std::pair<std::string, std::string>> mMapping;
    bool mStarted;
    std::string mKey;

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
            mKey = Keys::Unknown;
            const std::string found_key(str, length);
            for (const auto& key: sKeys) {
                if (key.first == found_key) {
                    mKey = key.second;
                    break;
                }
            }
            if (mKey == Keys::Unknown)
                result = HandlerBase::Key(str, length);
            // // for (const char* const key: {"  version", " date", "cdc_abbreviations", "continents", "countries", "locations", "names", "replacements"}) {
            // // }
            // if (!strncmp(str, "  version", std::min(length, 9U))) {
            //     mKey = Keys::Version;
            // }
            // else if (!strncmp(str, " date", std::min(length, 5U))) {
            //     mKey = Keys::Date;
            // }
            // else {
            //     result = HandlerBase::Key(str, length);
            // }
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
                  result = new StringMappingHandler(mLocDb, mLocDb.mCdcAbbreviations);
                  break;
              case Keys::Continents:
                  break;
              case Keys::Countries:
                  break;
              case Keys::Locations:
                  break;
              case Keys::Names:
                  break;
              case Keys::Replacements:
                  break;
              case Keys::Unknown:
                  result = HandlerBase::String(str, length);
                  break;
            }
            return result;
        }

 private:
    Keys mKey;

};

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

// ----------------------------------------------------------------------

class DocRootHandler : public HandlerBase
{
 public:
    inline DocRootHandler(LocDb& aLocDb) : HandlerBase(aLocDb) {}

    inline virtual HandlerBase* StartObject() { return new LocDbRootHandler(mLocDb); }
    inline virtual HandlerBase* EndObject(rapidjson::SizeType /*memberCount*/) { throw Failure(); }
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
    inline bool Double(double d) { return handler(&HandlerBase::Double, d); }

    inline bool Bool(bool b) { return false; }
    inline bool Int(int /*i*/) { return false; }
    // bool Uint(unsigned u) { return Int(static_cast<int>(u)); }

    inline bool Null() { std::cout << "Null()" << std::endl; return false; }
    bool Int64(int64_t i) { std::cout << "Int64(" << i << ")" << std::endl; return false; }
    bool Uint64(uint64_t u) { std::cout << "Uint64(" << u << ")" << std::endl; return false; }

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
