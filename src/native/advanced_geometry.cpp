#include "bindings.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace pyfbx {
namespace {

py::tuple element_instances(const element_view& self) {
    const auto& source = self.element->instances;
    py::tuple result{source.count};
    for (std::size_t i = 0; i < source.count; ++i) {
        result[i] = py::cast(node_view{self.owner, source.data[i]});
    }
    return result;
}

py::tuple connections(const element_view& self, const ufbx_connection_list& source) {
    py::tuple result{source.count};
    for (std::size_t i = 0; i < source.count; ++i) {
        const auto& connection = source.data[i];
        py::dict item;
        item["source"] = py::cast(element_view{self.owner, connection.src});
        item["destination"] = py::cast(element_view{self.owner, connection.dst});
        item["source_property"] = to_string(connection.src_prop);
        item["destination_property"] = to_string(connection.dst_prop);
        result[i] = std::move(item);
    }
    return result;
}

py::dict basis(const ufbx_nurbs_basis& source) {
    py::dict result;
    result["valid"] = source.valid;
    result["order"] = source.order;
    result["degree"] = source.order > 0 ? source.order - 1 : 0;
    result["parameter_range"] = py::make_tuple(source.t_min, source.t_max);
    py::tuple knots{source.knot_vector.count};
    for (std::size_t i = 0; i < source.knot_vector.count; ++i) {
        knots[i] = source.knot_vector.data[i];
    }
    result["knots"] = std::move(knots);
    return result;
}

py::tuple vec4_list(const ufbx_vec4_list& source) {
    py::tuple result{source.count};
    for (std::size_t i = 0; i < source.count; ++i) {
        result[i] = to_tuple(source.data[i]);
    }
    return result;
}

py::dict evaluate_curve(const nurbs_curve_view& self, const double parameter) {
    const ufbx_curve_point point = ufbx_evaluate_nurbs_curve(self.curve, parameter);
    py::dict result;
    result["valid"] = point.valid;
    result["position"] = to_tuple(point.position);
    result["derivative"] = to_tuple(point.derivative);
    return result;
}

py::dict evaluate_surface(
    const nurbs_surface_view& self,
    const double parameter_u,
    const double parameter_v) {
    const ufbx_surface_point point =
        ufbx_evaluate_nurbs_surface(self.surface, parameter_u, parameter_v);
    py::dict result;
    result["valid"] = point.valid;
    result["position"] = to_tuple(point.position);
    result["derivative_u"] = to_tuple(point.derivative_u);
    result["derivative_v"] = to_tuple(point.derivative_v);
    return result;
}

}  // namespace

void bind_advanced_geometry(py::module_& module) {
    py::class_<element_view>(module, "Element")
        .def_property_readonly("name", [](const element_view& self) {
            return to_string(self.element->name);
        })
        .def_property_readonly("type", [](const element_view& self) {
            return std::string{element_type_name(self.element->type)};
        })
        .def_property_readonly("element_id", [](const element_view& self) {
            return self.element->element_id;
        })
        .def_property_readonly("typed_id", [](const element_view& self) {
            return self.element->typed_id;
        })
        .def_property_readonly("instances", &element_instances)
        .def_property_readonly("properties", [](const element_view& self) {
            return properties(self.owner, self.element->props);
        })
        .def_property_readonly("connections_from", [](const element_view& self) {
            return connections(self, self.element->connections_src);
        })
        .def_property_readonly("connections_to", [](const element_view& self) {
            return connections(self, self.element->connections_dst);
        })
        .def("find_property", [](const element_view& self, const std::string& name) {
            return find_property(self.owner, self.element->props, name);
        })
        .def("__eq__", [](const element_view& self, const element_view& other) {
            return self.owner.get() == other.owner.get() && self.element == other.element;
        }, py::is_operator())
        .def("__hash__", [](const element_view& self) {
            return reinterpret_cast<std::uintptr_t>(self.element);
        })
        .def("__repr__", [](const element_view& self) {
            return "Element(name='" + to_string(self.element->name) + "', type='" +
                element_type_name(self.element->type) + "')";
        });

    py::class_<nurbs_curve_view>(module, "NurbsCurve")
        .def_property_readonly("name", [](const nurbs_curve_view& self) {
            return to_string(self.curve->name);
        })
        .def_property_readonly("basis", [](const nurbs_curve_view& self) {
            return basis(self.curve->basis);
        })
        .def_property_readonly("control_points", [](const nurbs_curve_view& self) {
            return vec4_list(self.curve->control_points);
        })
        .def("evaluate", &evaluate_curve, py::arg("parameter"));

    py::class_<nurbs_surface_view>(module, "NurbsSurface")
        .def_property_readonly("name", [](const nurbs_surface_view& self) {
            return to_string(self.surface->name);
        })
        .def_property_readonly("basis_u", [](const nurbs_surface_view& self) {
            return basis(self.surface->basis_u);
        })
        .def_property_readonly("basis_v", [](const nurbs_surface_view& self) {
            return basis(self.surface->basis_v);
        })
        .def_property_readonly("control_point_dimensions", [](const nurbs_surface_view& self) {
            return py::make_tuple(
                self.surface->num_control_points_u, self.surface->num_control_points_v);
        })
        .def_property_readonly("control_points", [](const nurbs_surface_view& self) {
            return vec4_list(self.surface->control_points);
        })
        .def_property_readonly("flip_normals", [](const nurbs_surface_view& self) {
            return self.surface->flip_normals;
        })
        .def("evaluate", &evaluate_surface, py::arg("u"), py::arg("v"));
}

}  // namespace pyfbx
