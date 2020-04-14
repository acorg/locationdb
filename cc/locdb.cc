#include <cstdlib>
#include <cctype>

#include "acmacs-base/acmacsd.hh"
#include "acmacs-base/string-split.hh"
#include "acmacs-base/string-join.hh"
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
            return find_indexed_by_name(aData, ::string::replace(aName, '_', ' ')); // non-acmacs names may have _ instead of space, e.g. NEW_YORK
        if (aName.find('-') != std::string::npos)
            return find_indexed_by_name(aData, ::string::replace(aName, '-', ' ')); // non-acmacs names may have - instead of space, e.g. NEW-YORK
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

std::string acmacs::locationdb::v1::LocDb::stat() const noexcept
{
    return fmt::format("continents:{} countries:{} locations:{} names:{} cdc-abbr:{} replacements:{}",
                       mContinents.size(), mCountries.size(), mLocations.size(), mNames.size(), mCdcAbbreviations.size(), mReplacements.size());

} // acmacs::locationdb::v1::LocDb::stat

// ----------------------------------------------------------------------

std::optional<acmacs::locationdb::v1::LookupResult> acmacs::locationdb::v1::LocDb::find(std::string_view aName, include_continent inc_continent) const noexcept
{
    using namespace std::string_view_literals;

    const auto find_in = [](const auto& container, std::string_view look_for) -> std::string_view {
        if (const auto found = std::lower_bound(container.begin(), container.end(), look_for, [](const auto& entry, std::string_view lf) { return entry.first < lf; });
            found != container.end() && found->first == look_for)
            return found->second;
        else
            return {};
    };

    using fiws_result_t = std::pair<std::string_view, std::string>;
    // returns {found location, name with substituions}
    const auto find_in_with_substs = [find_in](const auto& container, std::string_view look_for) -> fiws_result_t {
        if (const auto found = find_in(container, look_for); !found.empty())
            return fiws_result_t{found, look_for};
        for (const auto to_subst : {'_', '-', '.'}) {
            const auto substituted = ::string::replace(look_for, to_subst, ' ');
            if (const auto found = find_in(container, substituted); !found.empty())
                return fiws_result_t{found, substituted};
        }
        return fiws_result_t{};
    };

    // ----------------------------------------------------------------------

    const auto make_result = [inc_continent, this](LookupResult&& result) {
        if (inc_continent == include_continent::yes)
            result.continent.assign(continent_of_country(result.location.country()));
        if (result.replacement.empty() && result.look_for != result.name)
            result.replacement = result.name;
        return std::move(result);
    };

    const auto find_in_with_substs_replacements = [find_in, find_in_with_substs, make_result, this](std::string_view look_for, std::string_view orig_look_for) -> std::optional<LookupResult> {
        if (const auto [found, substituted] = find_in_with_substs(mNames, look_for); !found.empty())
            return make_result(LookupResult{.look_for{std::string{orig_look_for}}, .name{substituted}, .location_name{std::string{found}}, .location = detail::find_indexed_by_name(mLocations, found)});

        if (const auto [replacement_found, substituted] = find_in_with_substs(mReplacements, look_for); !replacement_found.empty()) {
            if (const auto found = find_in(mNames, replacement_found); !found.empty())
                return make_result(LookupResult{
                        .look_for{std::string{orig_look_for}}, .replacement{std::string{replacement_found}}, .name{std::string{replacement_found}}, .location_name{std::string{found}}, .location = detail::find_indexed_by_name(mLocations, found)});
        }
        return std::nullopt;
    };

    // ----------------------------------------------------------------------

    const auto find_in_with_prefix_separated = [find_in_with_substs_replacements](std::string_view look_for) -> std::optional<LookupResult> {
        for (const auto prefix : {"SOUTH"sv, "NORTH"sv, "WEST"sv, "EAST"sv, "CENTRAL"sv}) {
            if (look_for.size() > prefix.size() && look_for.substr(0, prefix.size()) == prefix) {
                const auto substituted = fmt::format("{} {}", prefix, look_for.substr(prefix.size()));
                if (auto found = find_in_with_substs_replacements(substituted, look_for); found.has_value())
                    return found;
            }
        }
        return std::nullopt;
    };

    const auto find_in_with_prefix_removed = [find_in_with_substs_replacements](std::string_view look_for) -> std::optional<LookupResult> {
        for (const auto prefix : {"NL-"sv, "TURKEY-"sv, "FLU-"sv}) {
            if (look_for.size() > prefix.size() && look_for.substr(0, prefix.size()) == prefix) {
                const auto substituted = look_for.substr(prefix.size());
                if (auto found = find_in_with_substs_replacements(substituted, look_for); found.has_value())
                    return found;
            }
        }
        return std::nullopt;
    };

    const auto find_in_with_suffix_removed = [find_in_with_substs_replacements](std::string_view look_for) -> std::optional<LookupResult> {
        for (const auto suffix : {"_NRL"sv, ".NRL"sv, " NRL"sv, ".GRC"sv, ".GR"sv, " GR"sv, "_PT"sv, " PT"sv}) { // NRL - kazakhstan
            if (look_for.size() > suffix.size() && look_for.substr(look_for.size() - suffix.size()) == suffix) {
                const auto substituted = look_for.substr(0, look_for.size() - suffix.size());
                if (auto found = find_in_with_substs_replacements(substituted, look_for); found.has_value())
                    return found;
            }
        }
        return std::nullopt;
    };

    const auto find_in_with_camel_case_separated = [find_in_with_substs_replacements](std::string_view look_for) -> std::optional<LookupResult> {
        if (const auto parts = acmacs::string::split_camel_case(look_for); parts.size() > 1 && parts.size() < look_for.size()) {
            const auto substituted = ::string::upper(acmacs::string::join(" ", parts));
            if (auto found = find_in_with_substs_replacements(substituted, look_for); found.has_value())
                return found;
        }
        return std::nullopt;
    };

    // ----------------------------------------------------------------------

    if (aName.size() < 3)
        return std::nullopt;

    const auto name_to_look_for_s = ::string::upper(aName);
    const std::string_view name_to_look_for{name_to_look_for_s};

    if (name_to_look_for[0] == '#') {
        const auto abbr = name_to_look_for.substr(1);
        if (const auto found = find_in(mCdcAbbreviations, abbr); !found.empty())
            return make_result(LookupResult{.look_for{std::string{name_to_look_for}}, .name{std::string{abbr}}, .location_name{std::string{found}}, .location = detail::find_indexed_by_name(mLocations, found)});
    }

    if (const auto res = find_in_with_substs_replacements(name_to_look_for, name_to_look_for); res.has_value())
        return res;

    if (const auto res = find_in_with_prefix_separated(name_to_look_for); res.has_value())
        return res;

    if (const auto res = find_in_with_prefix_removed(name_to_look_for); res.has_value())
        return res;

    if (const auto res = find_in_with_suffix_removed(name_to_look_for); res.has_value())
        return res;

    if (const auto res = find_in_with_camel_case_separated(aName); res.has_value())
        return res;

    return std::nullopt;

} // acmacs::locationdb::v1::LocDb::find

// ----------------------------------------------------------------------

acmacs::locationdb::v1::LookupResult acmacs::locationdb::v1::LocDb::find_or_throw(std::string_view aName) const
{
    if (const auto found = find(aName); found.has_value())
        return *found;
    else
        throw LocationNotFound{aName};

} // acmacs::locationdb::v1::LocDb::find

// ----------------------------------------------------------------------

acmacs::locationdb::v1::LookupResult acmacs::locationdb::v1::LocDb::find_cdc_abbreviation(std::string_view aAbbreviation) const
{
    if (aAbbreviation[0] == '#')
        aAbbreviation.remove_prefix(1);
    const std::string location_name = detail::find_indexed_by_name(mCdcAbbreviations, aAbbreviation); // throws LocationNotFound
    return LookupResult{.look_for{std::string{aAbbreviation}}, .name{std::string{aAbbreviation}}, .location_name{location_name}, .location=detail::find_indexed_by_name(mLocations, location_name)};

} // acmacs::locationdb::v1::LocDb::find_cdc_abbreviation

// ----------------------------------------------------------------------

std::string acmacs::locationdb::v1::LocDb::abbreviation(std::string_view name) const
{
      // if it's in USA, use CDC abbreviation (if available)
      // if aName has multiple words, use first letters of words in upper case
      // otherwise use two letter of aName capitalized

    using namespace std::string_view_literals;
    std::string abbreviation;

    const auto use_abbreviation_of = [&abbreviation](std::string_view to_abbr) {
        abbreviation = ::string::first_letter_of_words(to_abbr);
        if (abbreviation.size() == 1 && to_abbr.size() > 1)
            abbreviation.push_back(static_cast<char>(tolower(to_abbr[1])));
    };

    if (const auto found = find(name, include_continent::no); found.has_value()) {
        if (found->country() == "UNITED STATES OF AMERICA"sv)
            abbreviation = mCdcAbbreviations.find_abbreviation_by_name(found->location_name);
        if (abbreviation.empty())
            use_abbreviation_of(found->name);
    }
    else {
        use_abbreviation_of(name);
    }
    return abbreviation;

} // acmacs::locationdb::v1::LocDb::abbreviation

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
