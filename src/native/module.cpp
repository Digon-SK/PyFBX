#include "bindings.hpp"

#include <pybind11/pybind11.h>

namespace pyfbx {

PYBIND11_MODULE(_native, module) {
    module.doc() = "Native ufbx bindings used by pyfbx";

    py::register_exception<load_error>(module, "LoadError", PyExc_RuntimeError);

    bind_entities(module);
    bind_deform_animation(module);
    bind_advanced_geometry(module);
    bind_material(module);
    bind_mesh(module);
    bind_scene(module);

    const std::uint32_t version = ufbx_source_version;
    module.attr("__ufbx_version__") = py::make_tuple(
        version / 1000000u,
        (version / 1000u) % 1000u,
        version % 1000u);
    module.attr("__thread_safe__") = py::bool_{ufbx_is_thread_safe()};
}

}  // namespace pyfbx
