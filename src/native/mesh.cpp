#include "bindings.hpp"

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

#include <pybind11/stl.h>

namespace pyfbx {
namespace {

py::tuple logical_vertices(const mesh_view& self) {
    const auto& source = self.mesh->vertices;
    py::tuple result{source.count};
    for (std::size_t i = 0; i < source.count; ++i) {
        result[i] = to_tuple(source.data[i]);
    }
    return result;
}

template <typename Attribute, typename Converter>
py::object corner_attribute(
    const mesh_view& self,
    const Attribute& attribute,
    Converter&& convert) {
    if (!attribute.exists) {
        return py::none();
    }
    py::tuple result{self.mesh->num_indices};
    for (std::size_t i = 0; i < self.mesh->num_indices; ++i) {
        const std::uint32_t index = attribute.indices.data[i];
        if (index == UFBX_NO_INDEX || index >= attribute.values.count) {
            throw std::runtime_error{"ufbx returned an invalid vertex attribute index"};
        }
        result[i] = convert(attribute.values.data[index]);
    }
    return result;
}

py::tuple faces(const mesh_view& self) {
    py::tuple result{self.mesh->faces.count};
    for (std::size_t face_index = 0; face_index < self.mesh->faces.count; ++face_index) {
        const ufbx_face face = self.mesh->faces.data[face_index];
        py::tuple polygon{face.num_indices};
        for (std::uint32_t i = 0; i < face.num_indices; ++i) {
            polygon[i] = self.mesh->vertex_indices.data[face.index_begin + i];
        }
        result[face_index] = std::move(polygon);
    }
    return result;
}

std::vector<std::uint32_t> triangulated_corner_indices(const ufbx_mesh& mesh) {
    std::vector<std::uint32_t> result;
    result.reserve(mesh.num_triangles * 3);
    std::vector<std::uint32_t> scratch(mesh.max_face_triangles * 3);

    for (std::size_t face_index = 0; face_index < mesh.faces.count; ++face_index) {
        const ufbx_face face = mesh.faces.data[face_index];
        if (face.num_indices < 3) {
            continue;
        }
        const std::uint32_t triangle_count =
            ufbx_triangulate_face(scratch.data(), scratch.size(), &mesh, face);
        result.insert(result.end(), scratch.begin(), scratch.begin() + triangle_count * 3);
    }
    return result;
}

py::tuple triangle_indices(const mesh_view& self) {
    const auto indices = triangulated_corner_indices(*self.mesh);
    py::tuple result{indices.size() / 3};
    for (std::size_t i = 0; i < indices.size(); i += 3) {
        result[i / 3] = py::make_tuple(indices[i], indices[i + 1], indices[i + 2]);
    }
    return result;
}

py::tuple triangles(const mesh_view& self) {
    const auto indices = triangulated_corner_indices(*self.mesh);
    py::tuple result{indices.size() / 3};
    for (std::size_t i = 0; i < indices.size(); i += 3) {
        result[i / 3] = py::make_tuple(
            self.mesh->vertex_indices.data[indices[i]],
            self.mesh->vertex_indices.data[indices[i + 1]],
            self.mesh->vertex_indices.data[indices[i + 2]]);
    }
    return result;
}

py::object optional_uint32_list(const ufbx_uint32_list& source) {
    if (source.data == nullptr) {
        return py::none();
    }
    py::tuple result{source.count};
    for (std::size_t i = 0; i < source.count; ++i) {
        result[i] = source.data[i];
    }
    return result;
}

py::tuple mesh_materials(const mesh_view& self) {
    const auto& source = self.mesh->materials;
    py::tuple result{source.count};
    for (std::size_t i = 0; i < source.count; ++i) {
        result[i] = py::cast(material_view{self.owner, source.data[i]});
    }
    return result;
}

py::tuple mesh_instances(const mesh_view& self) {
    const auto& source = self.mesh->instances;
    py::tuple result{source.count};
    for (std::size_t i = 0; i < source.count; ++i) {
        result[i] = py::cast(node_view{self.owner, source.data[i]});
    }
    return result;
}

py::tuple skin_deformers(const mesh_view& self) {
    const auto& source = self.mesh->skin_deformers;
    py::tuple result{source.count};
    for (std::size_t i = 0; i < source.count; ++i) {
        result[i] = py::cast(skin_deformer_view{self.owner, source.data[i]});
    }
    return result;
}

py::tuple blend_deformers(const mesh_view& self) {
    const auto& source = self.mesh->blend_deformers;
    py::tuple result{source.count};
    for (std::size_t i = 0; i < source.count; ++i) {
        result[i] = py::cast(blend_deformer_view{self.owner, source.data[i]});
    }
    return result;
}

}  // namespace

void bind_mesh(py::module_& module) {
    py::class_<mesh_view>(module, "Mesh")
        .def_property_readonly("name", [](const mesh_view& self) {
            return to_string(self.mesh->name);
        })
        .def_property_readonly("element_id", [](const mesh_view& self) {
            return self.mesh->element_id;
        })
        .def_property_readonly("num_vertices", [](const mesh_view& self) {
            return self.mesh->num_vertices;
        })
        .def_property_readonly("num_indices", [](const mesh_view& self) {
            return self.mesh->num_indices;
        })
        .def_property_readonly("num_faces", [](const mesh_view& self) {
            return self.mesh->num_faces;
        })
        .def_property_readonly("num_triangles", [](const mesh_view& self) {
            return self.mesh->num_triangles;
        })
        .def_property_readonly("vertices", &logical_vertices)
        .def_property_readonly("vertex_positions", [](const mesh_view& self) {
            return corner_attribute(self, self.mesh->vertex_position, [](const ufbx_vec3 value) {
                return to_tuple(value);
            });
        })
        .def_property_readonly("normals", [](const mesh_view& self) {
            return corner_attribute(self, self.mesh->vertex_normal, [](const ufbx_vec3 value) {
                return to_tuple(value);
            });
        })
        .def_property_readonly("uvs", [](const mesh_view& self) {
            return corner_attribute(self, self.mesh->vertex_uv, [](const ufbx_vec2 value) {
                return to_tuple(value);
            });
        })
        .def_property_readonly("colors", [](const mesh_view& self) {
            return corner_attribute(self, self.mesh->vertex_color, [](const ufbx_vec4 value) {
                return to_tuple(value);
            });
        })
        .def_property_readonly("faces", &faces)
        .def_property_readonly("triangles", &triangles,
            "Triangulated logical vertex indices.")
        .def_property_readonly("triangle_indices", &triangle_indices,
            "Triangulated indices into per-corner attribute arrays.")
        .def_property_readonly("face_material_indices", [](const mesh_view& self) {
            return optional_uint32_list(self.mesh->face_material);
        })
        .def_property_readonly("materials", &mesh_materials)
        .def_property_readonly("instances", &mesh_instances)
        .def_property_readonly("skin_deformers", &skin_deformers)
        .def_property_readonly("blend_deformers", &blend_deformers)
        .def_property_readonly("properties", [](const mesh_view& self) {
            return properties(self.owner, self.mesh->props);
        })
        .def("find_property", [](const mesh_view& self, const std::string& name) {
            return find_property(self.owner, self.mesh->props, name);
        })
        .def_property_readonly("generated_normals", [](const mesh_view& self) {
            return self.mesh->generated_normals;
        })
        .def("__repr__", [](const mesh_view& self) {
            return "Mesh(name='" + to_string(self.mesh->name) + "', vertices=" +
                std::to_string(self.mesh->num_vertices) + ", faces=" +
                std::to_string(self.mesh->num_faces) + ")";
        });
}

}  // namespace pyfbx
