#include <cstdlib>
#include <cctype>

#include "acmacs-base/acmacsd.hh"
// #include "acmacs-base/string.hh"
#include "acmacs-base/debug.hh"
#include "acmacs-base/fmt.hh"

#include "locationdb/locdb.hh"
#include "locationdb/export.hh"

// ----------------------------------------------------------------------

#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wglobal-constructors"
#pragma GCC diagnostic ignored "-Wexit-time-destructors"
#endif

static std::unique_ptr<LocDb> sLocDb;
static std::string sLocDbFilename = acmacs::acmacsd_root() + "/data/locationdb.json.xz";
static bool sVerbose = false;

#pragma GCC diagnostic pop

// not thread safe!
void acmacs::locationdb::v1::setup(std::string_view aFilename, bool aVerbose)
{
    sVerbose = aVerbose;
    if (!aFilename.empty())
        sLocDbFilename = aFilename;
}

// not thread safe!
const acmacs::locationdb::v1::LocDb& acmacs::locationdb::v1::get(locdb_suppress_error suppress_error, report_time timer)
{
    if (!sLocDb) {
        sLocDb = std::make_unique<LocDb>();
        sLocDb->importFrom(sLocDbFilename, suppress_error, sVerbose ? report_time::yes : timer);
    }
    return *sLocDb;

} // get_locdb

// ----------------------------------------------------------------------

namespace acmacs::locationdb::inline v1::detail
{
    template <typename Value> inline auto find_indexed_by_name_no_fixes(const std::vector<std::pair<std::string, Value>>& aData, std::string_view aName) -> std::optional<decltype(aData.begin())>
    {
        if (const auto it = std::lower_bound(aData.begin(), aData.end(), aName, [](const auto& entry, const auto& look_for) -> bool { return entry.first < look_for; });
            it != aData.end() && it->first == aName)
            return it;
        else
            return std::nullopt;
    }

    template <typename Value, typename S> inline const Value& find_indexed_by_name(const std::vector<std::pair<std::string, Value>>& aData, S aName)
    {
        if (const auto it = find_indexed_by_name_no_fixes(aData, aName); it.has_value())
            return it.value()->second;
        if (aName.find('_') != std::string::npos)
            return find_indexed_by_name(aData, string::replace(aName, '_', ' ')); // non-acmacs names may have _ instead of space, e.g. NEW_YORK
        if (aName.find('-') != std::string::npos)
            return find_indexed_by_name(aData, string::replace(aName, '-', ' ')); // non-acmacs names may have - instead of space, e.g. NEW-YORK
        throw LocationNotFound(aName);
    }

} // namespace detail

// ----------------------------------------------------------------------

std::optional<typename acmacs::locationdb::v1::Names::const_iterator> acmacs::locationdb::v1::LocDb::find_by_name_no_fixes(std::string_view aName) const
{
    return detail::find_indexed_by_name_no_fixes(mNames, aName);

} // acmacs::locationdb::v1::LocDb::find_by_name_no_fixes

// ----------------------------------------------------------------------

std::optional<typename acmacs::locationdb::v1::Replacements::const_iterator> acmacs::locationdb::v1::LocDb::find_by_replacement_no_fixes(std::string_view aName) const
{
    return detail::find_indexed_by_name_no_fixes(mReplacements, aName);

} // acmacs::locationdb::v1::LocDb::find_by_replacement_no_fixes

// ----------------------------------------------------------------------

std::string_view acmacs::locationdb::v1::LocDb::continent_of_country(std::string_view aCountry) const
{
    return mContinents[detail::find_indexed_by_name(mCountries, aCountry)];

} // acmacs::locationdb::v1::LocDb::continent_of_country

// ----------------------------------------------------------------------

void acmacs::locationdb::v1::LocDb::importFrom(std::string_view aFilename, locdb_suppress_error suppress_error, report_time timer)
{
    Timeit timeit(fmt::format("DEBUG: LocDb loading from {}: ", aFilename), timer);
    import(aFilename, *this, suppress_error);

} // acmacs::locationdb::v1::LocDb::importFrom

// ----------------------------------------------------------------------

// void acmacs::locationdb::v1::LocDb::exportTo(std::string_view aFilename, bool aPretty, report_time timer) const
// {
//     throw std::runtime_error("LocDb::exportTo not implemented");

//     // Timeit timeit("locdb exporting: ", timer);
//     // if (aPretty)
//     //     locdb_export_pretty(aFilename, *this);
//     // else
//     //     locdb_export(aFilename, *this);

// } // acmacs::locationdb::v1::LocDb::exportTo

// ----------------------------------------------------------------------

std::string acmacs::locationdb::v1::LocDb::stat() const
{
    return fmt::format("continents:{} countries:{} locations:{} names:{} cdc-abbr:{} replacements:{}",
                       mContinents.size(), mCountries.size(), mLocations.size(), mNames.size(), mCdcAbbreviations.size(), mReplacements.size());

} // acmacs::locationdb::v1::LocDb::stat

// ----------------------------------------------------------------------

std::optional<acmacs::locationdb::v1::LookupResult> acmacs::locationdb::v1::LocDb::find(std::string_view aName, include_continent inc_continent) const
{
    const auto find_in = [](const auto& container, std::string_view look_for) {
        if (const auto found = std::lower_bound(container.begin(), container.end(), look_for, [](const auto& entry, std::string_view lf) { return entry.first < lf; }); found != container.end() && found->first == look_for)
            return found->second;
        else
            return std::string{};
    };

    const auto find_in_with_substs = [find_in](const auto& container, std::string_view look_for) {
        if (const auto found = find_in(container, look_for); !found.empty())
            return found;
        for (const auto to_subst : {'_', '-'}) {
            if (const auto found = find_in(container, string::replace(look_for, to_subst, ' ')); !found.empty())
                return found;
        }
        return std::string{};
    };

    const auto make_result = [inc_continent, this](LookupResult&& result) {
        if (inc_continent == include_continent::yes)
            result.continent.assign(continent_of_country(result.location.country()));
        return std::move(result);
    };

    if (aName.size() < 3)
        return std::nullopt;

    if (aName[0] == '#') {
        const auto abbr = aName.substr(1);
        if (const auto found = find_in(mCdcAbbreviations, abbr); !found.empty())
            return make_result(LookupResult{.look_for{aName}, .name{abbr}, .location_name{found}, .location=detail::find_indexed_by_name(mLocations, found)});
    }

    if (const auto found = find_in_with_substs(mNames, aName); !found.empty())
        return make_result(LookupResult{.look_for{aName}, .name{aName}, .location_name{found}, .location=detail::find_indexed_by_name(mLocations, found)});

    if (const auto replacement_found = find_in_with_substs(mReplacements, aName); !replacement_found.empty()) {
        if (const auto found = find_in(mNames, replacement_found); !found.empty())
            return make_result(LookupResult{.look_for{aName}, .replacement{replacement_found}, .name{replacement_found}, .location_name{found}, .location=detail::find_indexed_by_name(mLocations, found)});
    }

    // camel case

    return std::nullopt;

} // acmacs::locationdb::v1::LocDb::find

// ----------------------------------------------------------------------

acmacs::locationdb::v1::LookupResult acmacs::locationdb::v1::LocDb::find_or_throw(std::string_view aName) const
{
    if (const auto found = find(aName); found.has_value())
        return *found;
    else
        throw LocationNotFound{aName};

    // std::string name{aName};
    // std::string replacement;
    // std::string location_name;
    // try {
    //     if (const auto it = detail::find_indexed_by_name_no_fixes(mNames, aName); it.has_value())
    //         location_name = it.value()->second;
    //     else
    //         throw LocationNotFound{name};
    // }
    // catch (LocationNotFound&) {
    //     // try {
    //     if (name[0] == '#') {
    //         name.erase(0, 1);
    //         location_name = detail::find_indexed_by_name(mCdcAbbreviations, name);
    //     }
    //     else {
    //         try {
    //             replacement = detail::find_indexed_by_name(mReplacements, name);
    //             name = replacement;
    //             location_name = detail::find_indexed_by_name(mNames, name);
    //         }
    //         catch (LocationNotFound&) {
    //             const auto find_with_replacement = [&](char to_replace) -> bool {
    //                 if (aName.find(to_replace) != std::string::npos) {
    //                     replacement = string::replace(aName, to_replace, ' ');
    //                     const auto intermediate_result = find_or_throw(replacement);
    //                     if (!intermediate_result.replacement.empty())
    //                         replacement = intermediate_result.replacement;
    //                     location_name = intermediate_result.location_name;
    //                     name = replacement;
    //                     return true;
    //                 }
    //                 return false;
    //             };

    //             if (!find_with_replacement('_') && !find_with_replacement('-'))
    //                 throw;
    //         }
    //     }
    //     // }
    //     // catch (LocationNotFound& err) {
    //     //     std::cerr << "LocDb find: not found: " << err.what() << '\n';
    //     //     throw;
    //     // }
    // }
    // return LookupResult{.look_for{aName}, .replacement{replacement}, .name{name}, .location_name{location_name}, .location=detail::find_indexed_by_name(mLocations, location_name)};

} // acmacs::locationdb::v1::LocDb::find

// ----------------------------------------------------------------------

acmacs::locationdb::v1::LookupResult acmacs::locationdb::v1::LocDb::find_cdc_abbreviation(std::string_view aAbbreviation) const
{
    if (aAbbreviation[0] == '#')
        aAbbreviation.remove_prefix(1);
    const std::string location_name = detail::find_indexed_by_name(mCdcAbbreviations, aAbbreviation);
    return LookupResult{.look_for{aAbbreviation}, .name{aAbbreviation}, .location_name{location_name}, .location=detail::find_indexed_by_name(mLocations, location_name)};

} // acmacs::locationdb::v1::LocDb::find_cdc_abbreviation

// ----------------------------------------------------------------------

std::string acmacs::locationdb::v1::LocDb::abbreviation(std::string_view aName) const
{
      // if it's in USA, use CDC abbreviation (if available)
      // if aName has multiple words, use first letters of words in upper case
      // otherwise use two letter of aName capitalized
    std::string abbreviation;
    try {
        const auto found = find_or_throw(aName);
        if (found.country() == "UNITED STATES OF AMERICA")
            abbreviation = mCdcAbbreviations.find_abbreviation_by_name(found.location_name);
        if (abbreviation.empty())
            aName = found.name;
    }
    catch (LocationNotFound&) {
    }
    if (abbreviation.empty()) {
        abbreviation = string::first_letter_of_words(aName);
        if (abbreviation.size() == 1 && aName.size() > 1)
            abbreviation.push_back(static_cast<char>(tolower(aName[1])));
    }
    return abbreviation;

} // acmacs::locationdb::v1::LocDb::abbreviation

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
