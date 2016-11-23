#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wreserved-id-macro" // in Python.h
#pragma GCC diagnostic ignored "-Wextra-semi"
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wmissing-noreturn"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wdeprecated"
#pragma GCC diagnostic ignored "-Wfloat-equal"
#pragma GCC diagnostic ignored "-Wrange-loop-analysis"
#pragma GCC diagnostic ignored "-Wexit-time-destructors"
#pragma GCC diagnostic ignored "-Wundef"
#pragma GCC diagnostic ignored "-Wdocumentation"
#pragma GCC diagnostic ignored "-Wcovered-switch-default"
#endif
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#pragma GCC diagnostic pop

namespace py = pybind11;

#include "export.hh"

// ----------------------------------------------------------------------

#ifdef __clang__
#pragma GCC diagnostic ignored "-Wmissing-prototypes"
// #pragma GCC diagnostic ignored "-Wexit-time-destructors"
#endif

// ----------------------------------------------------------------------

PYBIND11_PLUGIN(locationdb_backend)
{
    py::module m("locationdb_backend", "locationdb access plugin");

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
            .def("export_to", &LocDb::exportTo, py::arg("filename"), py::arg("pretty") = false)
            .def("import_from", &LocDb::importFrom, py::arg("filename"))
            .def("find", &LocDb::find, py::arg("name"))
            .def("find_cdc_abbreviation", &LocDb::find_cdc_abbreviation, py::arg("abbreviation"))
            .def("country", &LocDb::country, py::arg("name"), py::arg("for_not_found") = "")
            .def("continent", &LocDb::continent, py::arg("name"), py::arg("for_not_found") = "")
            ;

    py::register_exception<LocationNotFound>(m, "LocationNotFound");

      // ----------------------------------------------------------------------

    return m.ptr();
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
