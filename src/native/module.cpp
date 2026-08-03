#include "bindings.hpp"

#include <pybind11/pybind11.h>

namespace pyfbx {
namespace {

PyObject* python_load_error_class{};

void translate_load_error(const std::exception_ptr pointer) {
    if (pointer == nullptr || python_load_error_class == nullptr) {
        return;
    }
    try {
        std::rethrow_exception(pointer);
    } catch (const load_error& error) {
        const py::object error_class = py::reinterpret_borrow<py::object>(python_load_error_class);
        const py::object instance = error_class(
            py::str{error.what()}, py::arg("kind") = error_type_name(error.type()));
        PyErr_SetObject(error_class.ptr(), instance.ptr());
    }
}

}  // namespace

PYBIND11_MODULE(_native, module) {
    module.doc() = "Native ufbx bindings used by pyfbx";

    const py::object load_error_class = py::module_::import("pyfbx._errors").attr("LoadError");
    python_load_error_class = load_error_class.ptr();
    py::register_local_exception_translator(&translate_load_error);

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
