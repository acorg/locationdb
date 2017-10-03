#include "export.hh"
#include "acmacs-base/pybind11.hh"

// ----------------------------------------------------------------------

PYBIND11_MODULE(locationdb_backend, m)
{
    m.doc() = "locationdb access plugin";

      // ----------------------------------------------------------------------

    py::class_<LookupResult>(m, "LookupResult")
            .def_readonly("look_for", &LookupResult::look_for)
            .def_readonly("replacement", &LookupResult::replacement)
            .def_readonly("name", &LookupResult::name)
            .def_readonly("location_name", &LookupResult::location_name)
            .def_property_readonly("latitude", &LookupResult::latitude)
            .def_property_readonly("longitude", &LookupResult::longitude)
            .def_property_readonly("country", &LookupResult::country)
            .def_property_readonly("division", &LookupResult::division)
            ;

    py::class_<LocDb>(m, "LocDb")
            .def(py::init<>())
            // .def("export_to", [](const LocDb& aLocdb, std::string aFilename, bool aPretty, bool aTimer) { aLocdb.exportTo(aFilename, aPretty, aTimer ? report_time::Yes : report_time::No); }, py::arg("filename"), py::arg("pretty") = false, py::arg("timer") = false)
            // .def("import_from", [](LocDb& aLocdb, std::string aFilename, bool aTimer) { aLocdb.importFrom(aFilename, aTimer ? report_time::Yes : report_time::No); }, py::arg("filename"), py::arg("timer") = false)
            .def("find", &LocDb::find, py::arg("name"))
            .def("find_cdc_abbreviation", &LocDb::find_cdc_abbreviation, py::arg("abbreviation"))
            .def("country", &LocDb::country, py::arg("name"), py::arg("for_not_found") = "")
            .def("continent", &LocDb::continent, py::arg("name"), py::arg("for_not_found") = "")
            ;

    m.def("locdb_setup", &locdb_setup, py::arg("filename"));
    m.def("get_locdb", [](bool aTimer) { return get_locdb(aTimer ? report_time::Yes : report_time::No); }, py::arg("timer") = false, py::return_value_policy::reference);

    py::register_exception<LocationNotFound>(m, "LocationNotFound");
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
