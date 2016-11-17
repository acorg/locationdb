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
    inline HandlerBase(LocDb& aLocDb) : mLocDb(aLocDb) {}
    virtual ~HandlerBase();

    inline virtual HandlerBase* StartObject() { throw Failure(); }
    inline virtual HandlerBase* EndObject() { throw Failure(); }
    inline virtual HandlerBase* StartArray() { throw Failure(); }
    inline virtual HandlerBase* EndArray() { throw Failure(); }
    inline virtual HandlerBase* Key(const char* /*str*/, rapidjson::SizeType /*length*/, bool /*copy*/) { throw Failure(); }
    inline virtual HandlerBase* String(const char* /*str*/, rapidjson::SizeType /*length*/, bool /*copy*/) { throw Failure(); }
    inline virtual HandlerBase* Double(double /*d*/) { throw Failure(); }

 protected:
    LocDb& mLocDb;
};

HandlerBase::~HandlerBase()
{
}

// ----------------------------------------------------------------------

class LocDbRootHandler : public HandlerBase
{
 public:
    inline LocDbRootHandler(LocDb& aLocDb) : HandlerBase(aLocDb) {}

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
 public:
    inline LocDbReaderEventHandler(LocDb& aLocDb)
        : mLocDb(aLocDb)
        {
            mHandler.emplace(new DocRootHandler(mLocDb));
        }

    inline bool StartObject()
        {
            try {
                auto new_handler = mHandler.top()->StartObject();
                if (new_handler)
                    mHandler.emplace(new_handler);
            }
            catch (Failure&) {
                return false;
            }
            return true;
        }

    inline bool EndObject(rapidjson::SizeType /*memberCount*/)
        {
            try {
                auto new_handler = mHandler.top()->EndObject();
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

    inline bool StartArray() { return true; }
    inline bool EndArray(rapidjson::SizeType /*elementCount*/) { return true; }

    inline bool Key(const char* str, rapidjson::SizeType length, bool /*copy*/)
        {
            std::cout << "K: " << std::string(str, length) << std::endl;
            return true;
        }

    inline bool String(const Ch* str, rapidjson::SizeType length, bool /*copy*/) { std::cout << "S: " << std::string(str, length) << std::endl; return true; }
    inline bool Double(double /*d*/) { return true; }

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


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
