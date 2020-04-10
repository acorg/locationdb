#include "locationdb/export.hh"
#include "acmacs-base/pybind11.hh"

// ----------------------------------------------------------------------

PYBIND11_MODULE(locationdb_backend, m)
{
    m.doc() = "locationdb access plugin";

      // ----------------------------------------------------------------------

    py::class_<acmacs::locationdb::v1::LookupResult>(m, "LookupResult")
            .def_readonly("look_for", &acmacs::locationdb::v1::LookupResult::look_for)
            .def_readonly("replacement", &acmacs::locationdb::v1::LookupResult::replacement)
            .def_readonly("name", &acmacs::locationdb::v1::LookupResult::name)
            .def_readonly("location_name", &acmacs::locationdb::v1::LookupResult::location_name)
            .def_property_readonly("latitude", &acmacs::locationdb::v1::LookupResult::latitude)
            .def_property_readonly("longitude", &acmacs::locationdb::v1::LookupResult::longitude)
            .def_property_readonly("country", &acmacs::locationdb::v1::LookupResult::country)
            .def_property_readonly("division", &acmacs::locationdb::v1::LookupResult::division)
            ;

    py::class_<acmacs::locationdb::v1::LocDb>(m, "LocDb")
            .def(py::init<>())
            // .def("export_to", [](const LocDb& aLocdb, std::string aFilename, bool aPretty, bool aTimer) { aLocdb.exportTo(aFilename, aPretty, do_report_time(aTimer)); }, py::arg("filename"), py::arg("pretty") = false, py::arg("timer") = false)
            // .def("import_from", [](LocDb& aLocdb, std::string aFilename, bool aTimer) { aLocdb.importFrom(aFilename, do_report_time(aTimer)); }, py::arg("filename"), py::arg("timer") = false)
            .def("find", &acmacs::locationdb::v1::LocDb::find_or_throw, py::arg("name"))
            .def("find_cdc_abbreviation", &acmacs::locationdb::v1::LocDb::find_cdc_abbreviation, py::arg("abbreviation"))
            .def("country", &acmacs::locationdb::v1::LocDb::country, py::arg("name"), py::arg("for_not_found") = "")
            .def("continent", &acmacs::locationdb::v1::LocDb::continent, py::arg("name"), py::arg("for_not_found") = "")
            ;

    m.def("locdb_setup", &locdb_setup, py::arg("filename"), py::arg("verbose") = false);
    m.def("get_locdb", [](bool suppress_error, bool aTimer) { return get_locdb(suppress_error ? acmacs::locationdb::v1::locdb_suppress_error::yes : acmacs::locationdb::v1::locdb_suppress_error::no, do_report_time(aTimer)); },
          py::arg("suppress_error") = false, py::arg("timer") = false, py::return_value_policy::reference);

    py::register_exception<acmacs::locationdb::v1::LocationNotFound>(m, "LocationNotFound");
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
