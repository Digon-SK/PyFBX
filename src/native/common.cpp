#include "bindings.hpp"

#include <array>
#include <cstddef>
#include <limits>
#include <sstream>
#include <utility>

#include <pybind11/stl.h>

namespace pyfbx {

scene_owner own_scene(ufbx_scene* scene) {
    return scene_owner{scene, [](ufbx_scene* value) noexcept {
                           if (value != nullptr) {
                               ufbx_free_scene(value);
                           }
                       }};
}

std::string to_string(const ufbx_string value) {
    return value.data == nullptr ? std::string{} : std::string{value.data, value.length};
}

py::tuple to_tuple(const ufbx_vec2 value) { return py::make_tuple(value.x, value.y); }

py::tuple to_tuple(const ufbx_vec3 value) { return py::make_tuple(value.x, value.y, value.z); }

py::tuple to_tuple(const ufbx_vec4 value) {
    return py::make_tuple(value.x, value.y, value.z, value.w);
}

py::tuple to_tuple(const ufbx_quat value) {
    return py::make_tuple(value.x, value.y, value.z, value.w);
}

py::tuple to_tuple(const ufbx_matrix value) {
    return py::make_tuple(
        py::make_tuple(value.m00, value.m01, value.m02, value.m03),
        py::make_tuple(value.m10, value.m11, value.m12, value.m13),
        py::make_tuple(value.m20, value.m21, value.m22, value.m23),
        py::make_tuple(0.0, 0.0, 0.0, 1.0));
}

py::dict to_transform(const ufbx_transform value) {
    py::dict result;
    result["translation"] = to_tuple(value.translation);
    result["rotation"] = to_tuple(value.rotation);
    result["scale"] = to_tuple(value.scale);
    return result;
}

std::string format_error(const ufbx_error& error) {
    std::array<char, 4096> buffer{};
    ufbx_format_error(buffer.data(), buffer.size(), &error);
    return std::string{buffer.data()};
}

namespace {

template <typename T>
T option(const py::dict& values, const char* name, const T fallback) {
    const py::str key{name};
    return values.contains(key) && !values[key].is_none() ? values[key].cast<T>() : fallback;
}

ufbx_coordinate_axes parse_axes(const std::string& value) {
    if (value == "right_handed_y_up") {
        return ufbx_axes_right_handed_y_up;
    }
    if (value == "right_handed_z_up") {
        return ufbx_axes_right_handed_z_up;
    }
    if (value == "left_handed_y_up") {
        return ufbx_axes_left_handed_y_up;
    }
    if (value == "left_handed_z_up") {
        return ufbx_axes_left_handed_z_up;
    }
    if (value != "original") {
        throw py::value_error{"unknown target_axes: " + value};
    }
    return {};
}

}  // namespace

ufbx_load_opts parse_load_options(const py::dict& values, const std::string& filename) {
    ufbx_load_opts opts{};
    opts.ignore_geometry = option(values, "ignore_geometry", false);
    opts.ignore_animation = option(values, "ignore_animation", false);
    opts.ignore_embedded = option(values, "ignore_embedded", false);
    opts.evaluate_skinning = option(values, "evaluate_skinning", false);
    opts.evaluate_caches = option(values, "evaluate_caches", false);
    opts.load_external_files = option(values, "load_external_files", false);
    opts.ignore_missing_external_files = option(values, "ignore_missing_external_files", false);
    opts.generate_missing_normals = option(values, "generate_missing_normals", true);
    opts.strict = option(values, "strict", false);

    const auto depth = option<std::uint64_t>(values, "node_depth_limit", 0);
    if (depth > std::numeric_limits<std::uint32_t>::max()) {
        throw py::value_error{"node_depth_limit is too large"};
    }
    opts.node_depth_limit = static_cast<std::uint32_t>(depth);

    const std::string axes = option<std::string>(values, "target_axes", "original");
    if (axes != "original") {
        opts.target_axes = parse_axes(axes);
        opts.space_conversion = UFBX_SPACE_CONVERSION_MODIFY_GEOMETRY;
    }

    const py::str unit_key{"target_unit_meters"};
    if (values.contains(unit_key) && !values[unit_key].is_none()) {
        const double unit = values[unit_key].cast<double>();
        if (!(unit > 0.0)) {
            throw py::value_error{"target_unit_meters must be greater than zero"};
        }
        opts.target_unit_meters = unit;
        opts.space_conversion = UFBX_SPACE_CONVERSION_MODIFY_GEOMETRY;
    }

    const py::str memory_key{"memory_limit_bytes"};
    if (values.contains(memory_key) && !values[memory_key].is_none()) {
        const auto limit = values[memory_key].cast<std::uint64_t>();
        if (limit == 0 || limit > std::numeric_limits<std::size_t>::max()) {
            throw py::value_error{"memory_limit_bytes is outside the supported range"};
        }
        opts.temp_allocator.memory_limit = static_cast<std::size_t>(limit);
        opts.result_allocator.memory_limit = static_cast<std::size_t>(limit);
    }

    opts.filename.data = filename.data();
    opts.filename.length = filename.size();
    return opts;
}

const char* element_type_name(const ufbx_element_type type) noexcept {
    switch (type) {
    case UFBX_ELEMENT_NODE: return "node";
    case UFBX_ELEMENT_MESH: return "mesh";
    case UFBX_ELEMENT_LIGHT: return "light";
    case UFBX_ELEMENT_CAMERA: return "camera";
    case UFBX_ELEMENT_BONE: return "bone";
    case UFBX_ELEMENT_EMPTY: return "empty";
    case UFBX_ELEMENT_MATERIAL: return "material";
    case UFBX_ELEMENT_TEXTURE: return "texture";
    case UFBX_ELEMENT_ANIM_STACK: return "animation_stack";
    case UFBX_ELEMENT_UNKNOWN: return "unknown";
    default: return "other";
    }
}

const char* file_format_name(const ufbx_file_format format) noexcept {
    switch (format) {
    case UFBX_FILE_FORMAT_FBX: return "fbx";
    case UFBX_FILE_FORMAT_OBJ: return "obj";
    case UFBX_FILE_FORMAT_MTL: return "mtl";
    default: return "unknown";
    }
}

}  // namespace pyfbx

