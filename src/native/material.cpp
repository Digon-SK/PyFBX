#include "bindings.hpp"

#include <sstream>
#include <string>

#include <pybind11/stl.h>

namespace pyfbx {
namespace {

py::object map_value(const ufbx_material_map& map, const std::uint8_t expected_components) {
    const std::uint8_t components = map.value_components != 0
        ? map.value_components
        : expected_components;
    switch (components) {
    case 1: return py::float_{map.value_real};
    case 2: return to_tuple(map.value_vec2);
    case 3: return to_tuple(map.value_vec3);
    case 4: return to_tuple(map.value_vec4);
    default: return py::none();
    }
}

py::dict describe_map(
    const scene_owner& owner,
    const ufbx_material_map& map,
    const std::uint8_t expected_components) {
    py::dict result;
    result["value"] = map_value(map, expected_components);
    result["has_value"] = map.has_value;
    result["texture_enabled"] = map.texture_enabled;
    result["texture"] = map.texture == nullptr
        ? py::object{py::none()}
        : py::cast(texture_view{owner, map.texture});
    return result;
}

py::dict pbr_maps(const material_view& self) {
    const auto& pbr = self.material->pbr;
    py::dict result;
    result["base_factor"] = describe_map(self.owner, pbr.base_factor, 1);
    result["base_color"] = describe_map(self.owner, pbr.base_color, 3);
    result["roughness"] = describe_map(self.owner, pbr.roughness, 1);
    result["metalness"] = describe_map(self.owner, pbr.metalness, 1);
    result["specular_color"] = describe_map(self.owner, pbr.specular_color, 3);
    result["emission_color"] = describe_map(self.owner, pbr.emission_color, 3);
    result["opacity"] = describe_map(self.owner, pbr.opacity, 1);
    result["normal_map"] = describe_map(self.owner, pbr.normal_map, 3);
    result["displacement_map"] = describe_map(self.owner, pbr.displacement_map, 1);
    return result;
}

py::dict fbx_maps(const material_view& self) {
    const auto& fbx = self.material->fbx;
    py::dict result;
    result["diffuse_color"] = describe_map(self.owner, fbx.diffuse_color, 3);
    result["diffuse_factor"] = describe_map(self.owner, fbx.diffuse_factor, 1);
    result["specular_color"] = describe_map(self.owner, fbx.specular_color, 3);
    result["specular_factor"] = describe_map(self.owner, fbx.specular_factor, 1);
    result["emission_color"] = describe_map(self.owner, fbx.emission_color, 3);
    result["transparency_factor"] = describe_map(self.owner, fbx.transparency_factor, 1);
    result["normal_map"] = describe_map(self.owner, fbx.normal_map, 3);
    result["bump"] = describe_map(self.owner, fbx.bump, 1);
    return result;
}

py::tuple material_textures(const material_view& self) {
    const auto& source = self.material->textures;
    py::tuple result{source.count};
    for (std::size_t i = 0; i < source.count; ++i) {
        result[i] = py::cast(texture_view{self.owner, source.data[i].texture});
    }
    return result;
}

}  // namespace

void bind_material(py::module_& module) {
    py::class_<texture_view>(module, "Texture")
        .def_property_readonly("name", [](const texture_view& self) {
            return to_string(self.texture->name);
        })
        .def_property_readonly("filename", [](const texture_view& self) {
            return to_string(self.texture->filename);
        })
        .def_property_readonly("relative_filename", [](const texture_view& self) {
            return to_string(self.texture->relative_filename);
        })
        .def_property_readonly("uv_set", [](const texture_view& self) {
            return to_string(self.texture->uv_set);
        })
        .def_property_readonly("embedded_content", [](const texture_view& self) -> py::object {
            const auto& content = self.texture->content;
            if (content.data == nullptr || content.size == 0) {
                return py::none();
            }
            return py::bytes{static_cast<const char*>(content.data), content.size};
        })
        .def("__repr__", [](const texture_view& self) {
            return "Texture(name='" + to_string(self.texture->name) + "', filename='" +
                to_string(self.texture->filename) + "')";
        });

    py::class_<material_view>(module, "Material")
        .def_property_readonly("name", [](const material_view& self) {
            return to_string(self.material->name);
        })
        .def_property_readonly("element_id", [](const material_view& self) {
            return self.material->element_id;
        })
        .def_property_readonly("shading_model", [](const material_view& self) {
            return to_string(self.material->shading_model_name);
        })
        .def_property_readonly("shader_type", [](const material_view& self) {
            return static_cast<std::uint32_t>(self.material->shader_type);
        })
        .def_property_readonly("pbr", &pbr_maps)
        .def_property_readonly("fbx", &fbx_maps)
        .def_property_readonly("textures", &material_textures)
        .def("__repr__", [](const material_view& self) {
            return "Material(name='" + to_string(self.material->name) + "')";
        });
}

}  // namespace pyfbx
