#include "acmacs-base/argv.hh"
#include "locationdb/locdb.hh"

// ----------------------------------------------------------------------

using namespace acmacs::argv;
struct Options : public argv
{
    Options(int a_argc, const char* const a_argv[], on_error on_err = on_error::exit) : argv() { parse(a_argc, a_argv, on_err); }

    argument<str_array> look_for{*this, arg_name{"look-for"}};
};

// ----------------------------------------------------------------------

int main(int argc, const char* const* argv)
{
    int exit_code = 0;
    try {
        Options opt(argc, argv);
        const auto& locdb = acmacs::locationdb::get();
        for (const auto& look_for : opt.look_for) {
            if (const auto found = locdb.find(look_for, acmacs::locationdb::include_continent::yes); found.has_value()) {
                fmt::print("\"{}\" <-- \"{}\"\n", found->name, found->look_for);
                if (!found->replacement.empty())
                    AD_WARNING("replace with  \"{}\"", found->replacement);
                fmt::print("    --> \"{}\" {} {} [\"{}\" \"{}\"] {}\n",  found->location_name, found->latitude(), found->longitude(), found->country(), found->division(), found->continent);
            }
            else
                AD_ERROR("\"{}\" not found", look_for);
        }
    }
    catch (std::exception& err) {
        AD_ERROR("{}", err);
        exit_code = 1;
    }
    return exit_code;
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
