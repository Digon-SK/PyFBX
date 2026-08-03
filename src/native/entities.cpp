#include "bindings.hpp"

#include <cstddef>
#include <cstdint>
#include <string>

namespace pyfbx {
namespace {

const char* property_type_name(const ufbx_prop_type type) noexcept {
    switch (type) {
    case UFBX_PROP_BOOLEAN: return "boolean";
    case UFBX_PROP_INTEGER: return "integer";
    case UFBX_PROP_NUMBER: return "number";
    case UFBX_PROP_VECTOR: return "vector";
    case UFBX_PROP_COLOR: return "color";
    case UFBX_PROP_COLOR_WITH_ALPHA: return "color_with_alpha";
    case UFBX_PROP_STRING: return "string";
    case UFBX_PROP_DATE_TIME: return "date_time";
    case UFBX_PROP_TRANSLATION: return "translation";
    case UFBX_PROP_ROTATION: return "rotation";
    case UFBX_PROP_SCALING: return "scaling";
    case UFBX_PROP_DISTANCE: return "distance";
    case UFBX_PROP_COMPOUND: return "compound";
    case UFBX_PROP_BLOB: return "blob";
    case UFBX_PROP_REFERENCE: return "reference";
    default: return "unknown";
    }
}

py::object property_value(const ufbx_prop& property) {
    switch (property.type) {
    case UFBX_PROP_BOOLEAN:
        return py::bool_{property.value_int != 0};
    case UFBX_PROP_INTEGER:
        return py::int_{property.value_int};
    case UFBX_PROP_STRING:
    case UFBX_PROP_DATE_TIME:
        return py::str{to_string(property.value_str)};
    case UFBX_PROP_COLOR_WITH_ALPHA:
        return to_tuple(property.value_vec4);
    case UFBX_PROP_VECTOR:
    case UFBX_PROP_COLOR:
    case UFBX_PROP_TRANSLATION:
    case UFBX_PROP_ROTATION:
    case UFBX_PROP_SCALING:
        return to_tuple(property.value_vec3);
    case UFBX_PROP_BLOB:
        if (property.value_blob.data == nullptr) {
            return py::bytes{};
        }
        return py::bytes{
            static_cast<const char*>(property.value_blob.data), property.value_blob.size};
    case UFBX_PROP_NUMBER:
    case UFBX_PROP_DISTANCE:
        return py::float_{property.value_real};
    default:
        if ((property.flags & UFBX_PROP_FLAG_VALUE_STR) != 0) {
            return py::str{to_string(property.value_str)};
        }
        if ((property.flags & UFBX_PROP_FLAG_VALUE_VEC4) != 0) {
            return to_tuple(property.value_vec4);
        }
        if ((property.flags & UFBX_PROP_FLAG_VALUE_VEC3) != 0) {
            return to_tuple(property.value_vec3);
        }
        if ((property.flags & UFBX_PROP_FLAG_VALUE_INT) != 0) {
            return py::int_{property.value_int};
        }
        if ((property.flags & UFBX_PROP_FLAG_VALUE_REAL) != 0) {
            return py::float_{property.value_real};
        }
        return py::none();
    }
}

const char* light_type_name(const ufbx_light_type type) noexcept {
    switch (type) {
    case UFBX_LIGHT_POINT: return "point";
    case UFBX_LIGHT_DIRECTIONAL: return "directional";
    case UFBX_LIGHT_SPOT: return "spot";
    case UFBX_LIGHT_AREA: return "area";
    case UFBX_LIGHT_VOLUME: return "volume";
    default: return "unknown";
    }
}

}  // namespace

py::tuple properties(const scene_owner& owner, const ufbx_props& source) {
    py::tuple result{source.props.count};
    for (std::size_t i = 0; i < source.props.count; ++i) {
        result[i] = py::cast(property_view{owner, &source.props.data[i]});
    }
    return result;
}

py::object find_property(
    const scene_owner& owner,
    const ufbx_props& source,
    const std::string& name) {
    const ufbx_prop* property = ufbx_find_prop_len(&source, name.data(), name.size());
    return property == nullptr ? py::object{py::none()} : py::cast(property_view{owner, property});
}

void bind_entities(py::module_& module) {
    py::class_<property_view>(module, "Property")
        .def_property_readonly("name", [](const property_view& self) {
            return to_string(self.property->name);
        })
        .def_property_readonly("type", [](const property_view& self) {
            return std::string{property_type_name(self.property->type)};
        })
        .def_property_readonly("value", [](const property_view& self) {
            return property_value(*self.property);
        })
        .def_property_readonly("is_user_defined", [](const property_view& self) {
            return (self.property->flags & UFBX_PROP_FLAG_USER_DEFINED) != 0;
        })
        .def_property_readonly("is_animatable", [](const property_view& self) {
            return (self.property->flags & UFBX_PROP_FLAG_ANIMATABLE) != 0;
        })
        .def("__repr__", [](const property_view& self) {
            return "Property(name='" + to_string(self.property->name) + "', type='" +
                property_type_name(self.property->type) + "')";
        });

    py::class_<camera_view>(module, "Camera")
        .def_property_readonly("name", [](const camera_view& self) {
            return to_string(self.camera->name);
        })
        .def_property_readonly("projection", [](const camera_view& self) {
            return self.camera->projection_mode == UFBX_PROJECTION_MODE_PERSPECTIVE
                ? "perspective" : "orthographic";
        })
        .def_property_readonly("resolution", [](const camera_view& self) {
            return to_tuple(self.camera->resolution);
        })
        .def_property_readonly("field_of_view", [](const camera_view& self) {
            return to_tuple(self.camera->field_of_view_deg);
        })
        .def_property_readonly("orthographic_size", [](const camera_view& self) {
            return to_tuple(self.camera->orthographic_size);
        })
        .def_property_readonly("aspect_ratio", [](const camera_view& self) {
            return self.camera->aspect_ratio;
        })
        .def_property_readonly("near_plane", [](const camera_view& self) {
            return self.camera->near_plane;
        })
        .def_property_readonly("far_plane", [](const camera_view& self) {
            return self.camera->far_plane;
        })
        .def_property_readonly("focal_length_mm", [](const camera_view& self) {
            return self.camera->focal_length_mm;
        })
        .def_property_readonly("properties", [](const camera_view& self) {
            return properties(self.owner, self.camera->props);
        })
        .def("find_property", [](const camera_view& self, const std::string& name) {
            return find_property(self.owner, self.camera->props, name);
        });

    py::class_<light_view>(module, "Light")
        .def_property_readonly("name", [](const light_view& self) {
            return to_string(self.light->name);
        })
        .def_property_readonly("type", [](const light_view& self) {
            return std::string{light_type_name(self.light->type)};
        })
        .def_property_readonly("color", [](const light_view& self) {
            return to_tuple(self.light->color);
        })
        .def_property_readonly("intensity", [](const light_view& self) {
            return self.light->intensity;
        })
        .def_property_readonly("local_direction", [](const light_view& self) {
            return to_tuple(self.light->local_direction);
        })
        .def_property_readonly("inner_angle", [](const light_view& self) {
            return self.light->inner_angle;
        })
        .def_property_readonly("outer_angle", [](const light_view& self) {
            return self.light->outer_angle;
        })
        .def_property_readonly("casts_shadows", [](const light_view& self) {
            return self.light->cast_shadows;
        })
        .def_property_readonly("properties", [](const light_view& self) {
            return properties(self.owner, self.light->props);
        });

    py::class_<bone_view>(module, "Bone")
        .def_property_readonly("name", [](const bone_view& self) {
            return to_string(self.bone->name);
        })
        .def_property_readonly("radius", [](const bone_view& self) {
            return self.bone->radius;
        })
        .def_property_readonly("relative_length", [](const bone_view& self) {
            return self.bone->relative_length;
        })
        .def_property_readonly("is_root", [](const bone_view& self) {
            return self.bone->is_root;
        })
        .def_property_readonly("properties", [](const bone_view& self) {
            return properties(self.owner, self.bone->props);
        });
}

}  // namespace pyfbx
