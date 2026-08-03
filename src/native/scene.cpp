#include "bindings.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include <pybind11/stl.h>

namespace pyfbx {
namespace {

scene_view load_file(const std::string& filename, const py::dict& values) {
    ufbx_load_opts opts = parse_load_options(values, filename);
    ufbx_error error{};
    ufbx_scene* scene{};
    {
        py::gil_scoped_release release;
        scene = ufbx_load_file_len(filename.data(), filename.size(), &opts, &error);
    }
    if (scene == nullptr) {
        throw load_error{format_error(error), error.type};
    }
    return scene_view{own_scene(scene)};
}

scene_view load_memory(
    const py::buffer& data,
    const std::string& filename,
    const py::dict& values) {
    const py::buffer_info info = data.request();
    if (info.ndim != 1 || (info.size > 1 && info.strides[0] != info.itemsize)) {
        throw py::value_error{"data must be a contiguous one-dimensional buffer"};
    }
    const auto bytes = static_cast<std::size_t>(info.size) * static_cast<std::size_t>(info.itemsize);
    ufbx_load_opts opts = parse_load_options(values, filename);
    ufbx_error error{};
    ufbx_scene* scene{};
    {
        py::gil_scoped_release release;
        scene = ufbx_load_memory(info.ptr, bytes, &opts, &error);
    }
    if (scene == nullptr) {
        throw load_error{format_error(error), error.type};
    }
    return scene_view{own_scene(scene)};
}

py::tuple scene_nodes(const scene_view& self) {
    const auto& source = self.get()->nodes;
    py::tuple result{source.count};
    for (std::size_t i = 0; i < source.count; ++i) {
        result[i] = py::cast(node_view{self.owner, source.data[i]});
    }
    return result;
}

py::tuple scene_meshes(const scene_view& self) {
    const auto& source = self.get()->meshes;
    py::tuple result{source.count};
    for (std::size_t i = 0; i < source.count; ++i) {
        result[i] = py::cast(mesh_view{self.owner, source.data[i]});
    }
    return result;
}

py::tuple scene_materials(const scene_view& self) {
    const auto& source = self.get()->materials;
    py::tuple result{source.count};
    for (std::size_t i = 0; i < source.count; ++i) {
        result[i] = py::cast(material_view{self.owner, source.data[i]});
    }
    return result;
}

py::tuple scene_textures(const scene_view& self) {
    const auto& source = self.get()->textures;
    py::tuple result{source.count};
    for (std::size_t i = 0; i < source.count; ++i) {
        result[i] = py::cast(texture_view{self.owner, source.data[i]});
    }
    return result;
}

py::tuple scene_animations(const scene_view& self) {
    const auto& source = self.get()->anim_stacks;
    py::tuple result{source.count};
    for (std::size_t i = 0; i < source.count; ++i) {
        result[i] = py::cast(animation_stack_view{self.owner, source.data[i]});
    }
    return result;
}

py::tuple scene_warnings(const scene_view& self) {
    const auto& source = self.get()->metadata.warnings;
    py::tuple result{source.count};
    for (std::size_t i = 0; i < source.count; ++i) {
        const auto& warning = source.data[i];
        py::dict item;
        item["type"] = static_cast<std::uint32_t>(warning.type);
        item["description"] = to_string(warning.description);
        item["element_id"] = warning.element_id == UFBX_NO_INDEX
            ? py::object{py::none()}
            : py::cast(warning.element_id);
        item["count"] = warning.count;
        result[i] = std::move(item);
    }
    return result;
}

py::object find_node(const scene_view& self, const std::string& name) {
    const ufbx_node* node = ufbx_find_node_len(self.get(), name.data(), name.size());
    return node == nullptr ? py::object{py::none()} : py::cast(node_view{self.owner, node});
}

py::object scene_getitem(const scene_view& self, const py::object& key) {
    if (py::isinstance<py::str>(key)) {
        const std::string name = key.cast<std::string>();
        const py::object node = find_node(self, name);
        if (node.is_none()) {
            throw py::key_error{name};
        }
        return node;
    }
    if (py::isinstance<py::int_>(key)) {
        py::ssize_t index = key.cast<py::ssize_t>();
        const auto count = static_cast<py::ssize_t>(self.get()->nodes.count);
        if (index < 0) {
            index += count;
        }
        if (index < 0 || index >= count) {
            throw py::index_error{};
        }
        return py::cast(node_view{self.owner, self.get()->nodes.data[index]});
    }
    throw py::type_error{"scene indices must be integers or node names"};
}

scene_view evaluate_scene(
    const scene_view& self,
    const double time,
    const py::object& animation,
    const bool evaluate_skinning) {
    const ufbx_anim* anim = self.get()->anim;
    if (!animation.is_none()) {
        const auto stack = animation.cast<animation_stack_view>();
        if (stack.owner.get() != self.owner.get()) {
            throw py::value_error{"animation belongs to a different scene"};
        }
        anim = stack.stack->anim;
    }
    ufbx_evaluate_opts opts{};
    opts.evaluate_skinning = evaluate_skinning;
    ufbx_error error{};
    ufbx_scene* evaluated{};
    {
        py::gil_scoped_release release;
        evaluated = ufbx_evaluate_scene(self.get(), anim, time, &opts, &error);
    }
    if (evaluated == nullptr) {
        throw load_error{format_error(error), error.type};
    }
    return scene_view{own_scene(evaluated)};
}

py::object node_parent(const node_view& self) {
    return self.node->parent == nullptr
        ? py::object{py::none()}
        : py::cast(node_view{self.owner, self.node->parent});
}

py::tuple node_children(const node_view& self) {
    const auto& source = self.node->children;
    py::tuple result{source.count};
    for (std::size_t i = 0; i < source.count; ++i) {
        result[i] = py::cast(node_view{self.owner, source.data[i]});
    }
    return result;
}

py::object node_mesh(const node_view& self) {
    return self.node->mesh == nullptr
        ? py::object{py::none()}
        : py::cast(mesh_view{self.owner, self.node->mesh});
}

py::tuple node_materials(const node_view& self) {
    const auto& source = self.node->materials;
    py::tuple result{source.count};
    for (std::size_t i = 0; i < source.count; ++i) {
        result[i] = py::cast(material_view{self.owner, source.data[i]});
    }
    return result;
}

std::string node_path(const node_view& self) {
    std::vector<std::string> parts;
    for (const ufbx_node* node = self.node; node != nullptr; node = node->parent) {
        const std::string name = to_string(node->name);
        if (!name.empty()) {
            parts.push_back(name);
        }
    }
    std::string result{"/"};
    for (auto it = parts.rbegin(); it != parts.rend(); ++it) {
        if (result.size() > 1) {
            result += '/';
        }
        result += *it;
    }
    return result;
}

py::object node_find(const node_view& self, const std::string& name) {
    std::vector<const ufbx_node*> pending{self.node};
    while (!pending.empty()) {
        const ufbx_node* current = pending.back();
        pending.pop_back();
        if (to_string(current->name) == name) {
            return py::cast(node_view{self.owner, current});
        }
        for (std::size_t i = current->children.count; i > 0; --i) {
            pending.push_back(current->children.data[i - 1]);
        }
    }
    return py::none();
}

}  // namespace

void bind_scene(py::module_& module) {
    py::class_<animation_stack_view>(module, "AnimationStack")
        .def_property_readonly("name", [](const animation_stack_view& self) {
            return to_string(self.stack->name);
        })
        .def_property_readonly("time_begin", [](const animation_stack_view& self) {
            return self.stack->time_begin;
        })
        .def_property_readonly("time_end", [](const animation_stack_view& self) {
            return self.stack->time_end;
        })
        .def_property_readonly("duration", [](const animation_stack_view& self) {
            return self.stack->time_end - self.stack->time_begin;
        })
        .def("__repr__", [](const animation_stack_view& self) {
            return "AnimationStack(name='" + to_string(self.stack->name) + "', duration=" +
                std::to_string(self.stack->time_end - self.stack->time_begin) + ")";
        });

    py::class_<node_view>(module, "Node")
        .def_property_readonly("name", [](const node_view& self) {
            return to_string(self.node->name);
        })
        .def_property_readonly("path", &node_path)
        .def_property_readonly("element_id", [](const node_view& self) {
            return self.node->element_id;
        })
        .def_property_readonly("attribute_type", [](const node_view& self) {
            return std::string{element_type_name(self.node->attrib_type)};
        })
        .def_property_readonly("parent", &node_parent)
        .def_property_readonly("children", &node_children)
        .def_property_readonly("mesh", &node_mesh)
        .def_property_readonly("materials", &node_materials)
        .def_property_readonly("visible", [](const node_view& self) { return self.node->visible; })
        .def_property_readonly("is_root", [](const node_view& self) { return self.node->is_root; })
        .def_property_readonly("local_transform", [](const node_view& self) {
            return to_transform(self.node->local_transform);
        })
        .def_property_readonly("geometry_transform", [](const node_view& self) {
            return to_transform(self.node->geometry_transform);
        })
        .def_property_readonly("node_to_world", [](const node_view& self) {
            return to_tuple(self.node->node_to_world);
        })
        .def_property_readonly("geometry_to_world", [](const node_view& self) {
            return to_tuple(self.node->geometry_to_world);
        })
        .def("find", &node_find, py::arg("name"), "Find a descendant by exact name.")
        .def("__len__", [](const node_view& self) { return self.node->children.count; })
        .def("__iter__", [](const node_view& self) { return py::iter(node_children(self)); })
        .def("__repr__", [](const node_view& self) {
            return "Node(name='" + to_string(self.node->name) + "', type='" +
                element_type_name(self.node->attrib_type) + "')";
        });

    py::class_<scene_view>(module, "Scene")
        .def_property_readonly("root", [](const scene_view& self) {
            return node_view{self.owner, self.get()->root_node};
        })
        .def_property_readonly("nodes", &scene_nodes)
        .def_property_readonly("meshes", &scene_meshes)
        .def_property_readonly("materials", &scene_materials)
        .def_property_readonly("textures", &scene_textures)
        .def_property_readonly("animations", &scene_animations)
        .def_property_readonly("warnings", &scene_warnings)
        .def_property_readonly("filename", [](const scene_view& self) {
            return to_string(self.get()->metadata.filename);
        })
        .def_property_readonly("creator", [](const scene_view& self) {
            return to_string(self.get()->metadata.creator);
        })
        .def_property_readonly("file_format", [](const scene_view& self) {
            return std::string{file_format_name(self.get()->metadata.file_format)};
        })
        .def_property_readonly("file_version", [](const scene_view& self) {
            return self.get()->metadata.version;
        })
        .def_property_readonly("is_ascii", [](const scene_view& self) {
            return self.get()->metadata.ascii;
        })
        .def_property_readonly("unit_meters", [](const scene_view& self) {
            return self.get()->settings.unit_meters;
        })
        .def_property_readonly("frames_per_second", [](const scene_view& self) {
            return self.get()->settings.frames_per_second;
        })
        .def_property_readonly("memory_used", [](const scene_view& self) {
            return self.get()->metadata.result_memory_used;
        })
        .def("find_node", &find_node, py::arg("name"))
        .def("evaluate", &evaluate_scene,
            py::arg("time"), py::arg("animation") = py::none(),
            py::arg("evaluate_skinning") = true,
            "Evaluate node transforms and optional skinning at time in seconds.")
        .def("__len__", [](const scene_view& self) { return self.get()->nodes.count; })
        .def("__iter__", [](const scene_view& self) { return py::iter(scene_nodes(self)); })
        .def("__getitem__", &scene_getitem)
        .def("__repr__", [](const scene_view& self) {
            return "Scene(format='" + std::string{file_format_name(self.get()->metadata.file_format)} +
                "', nodes=" + std::to_string(self.get()->nodes.count) + ", meshes=" +
                std::to_string(self.get()->meshes.count) + ")";
        });

    module.def("load", &load_file, py::arg("filename"), py::arg("options"));
    module.def("loads", &load_memory, py::arg("data"), py::arg("filename"), py::arg("options"));
}

}  // namespace pyfbx

