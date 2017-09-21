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
            .def("export_to", &LocDb::exportTo, py::arg("filename"), py::arg("pretty") = false, py::arg("timer") = false)
            .def("import_from", &LocDb::importFrom, py::arg("filename"), py::arg("timer") = false)
            .def("find", &LocDb::find, py::arg("name"))
            .def("find_cdc_abbreviation", &LocDb::find_cdc_abbreviation, py::arg("abbreviation"))
            .def("country", &LocDb::country, py::arg("name"), py::arg("for_not_found") = "")
            .def("continent", &LocDb::continent, py::arg("name"), py::arg("for_not_found") = "")
            ;

    py::register_exception<LocationNotFound>(m, "LocationNotFound");
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
