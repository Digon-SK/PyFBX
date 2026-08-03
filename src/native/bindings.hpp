#pragma once

#include <pybind11/pybind11.h>

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>

#include "ufbx.h"

namespace py = pybind11;

namespace pyfbx {

using scene_owner = std::shared_ptr<ufbx_scene>;

class load_error final : public std::runtime_error {
public:
    load_error(std::string message, ufbx_error_type type)
        : std::runtime_error{std::move(message)}, type_{type} {}

    [[nodiscard]] ufbx_error_type type() const noexcept { return type_; }

private:
    ufbx_error_type type_;
};

struct node_view;
struct mesh_view;
struct material_view;
struct texture_view;
struct animation_stack_view;
struct camera_view;
struct light_view;
struct bone_view;
struct property_view;

struct scene_view {
    scene_owner owner;

    [[nodiscard]] const ufbx_scene* get() const noexcept { return owner.get(); }
};

struct node_view {
    scene_owner owner;
    const ufbx_node* node{};
};

struct mesh_view {
    scene_owner owner;
    const ufbx_mesh* mesh{};
};

struct material_view {
    scene_owner owner;
    const ufbx_material* material{};
};

struct texture_view {
    scene_owner owner;
    const ufbx_texture* texture{};
};

struct animation_stack_view {
    scene_owner owner;
    const ufbx_anim_stack* stack{};
};

struct camera_view { scene_owner owner; const ufbx_camera* camera{}; };
struct light_view { scene_owner owner; const ufbx_light* light{}; };
struct bone_view { scene_owner owner; const ufbx_bone* bone{}; };
struct property_view { scene_owner owner; const ufbx_prop* property{}; };

[[nodiscard]] scene_owner own_scene(ufbx_scene* scene);
[[nodiscard]] std::string to_string(ufbx_string value);
[[nodiscard]] py::tuple to_tuple(ufbx_vec2 value);
[[nodiscard]] py::tuple to_tuple(ufbx_vec3 value);
[[nodiscard]] py::tuple to_tuple(ufbx_vec4 value);
[[nodiscard]] py::tuple to_tuple(ufbx_quat value);
[[nodiscard]] py::tuple to_tuple(ufbx_matrix value);
[[nodiscard]] py::dict to_transform(ufbx_transform value);
[[nodiscard]] std::string format_error(const ufbx_error& error);
[[nodiscard]] ufbx_load_opts parse_load_options(const py::dict& values, const std::string& filename);
[[nodiscard]] const char* element_type_name(ufbx_element_type type) noexcept;
[[nodiscard]] const char* file_format_name(ufbx_file_format format) noexcept;
[[nodiscard]] py::tuple properties(const scene_owner& owner, const ufbx_props& source);
[[nodiscard]] py::object find_property(
    const scene_owner& owner, const ufbx_props& source, const std::string& name);

void bind_entities(py::module_& module);
void bind_scene(py::module_& module);
void bind_mesh(py::module_& module);
void bind_material(py::module_& module);

}  // namespace pyfbx
