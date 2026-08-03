#include "bindings.hpp"

#include <cstddef>
#include <string>

namespace pyfbx {
namespace {

const char* constraint_type_name(const ufbx_constraint_type type) noexcept {
    switch (type) {
    case UFBX_CONSTRAINT_AIM: return "aim";
    case UFBX_CONSTRAINT_PARENT: return "parent";
    case UFBX_CONSTRAINT_POSITION: return "position";
    case UFBX_CONSTRAINT_ROTATION: return "rotation";
    case UFBX_CONSTRAINT_SCALE: return "scale";
    case UFBX_CONSTRAINT_SINGLE_CHAIN_IK: return "single_chain_ik";
    default: return "unknown";
    }
}

py::object optional_node(const scene_owner& owner, const ufbx_node* node) {
    return node == nullptr ? py::object{py::none()} : py::cast(node_view{owner, node});
}

py::tuple constraint_targets(const constraint_view& self) {
    const auto& source = self.constraint->targets;
    py::tuple result{source.count};
    for (std::size_t i = 0; i < source.count; ++i) {
        const auto& target = source.data[i];
        py::dict item;
        item["node"] = optional_node(self.owner, target.node);
        item["weight"] = target.weight;
        item["transform"] = to_transform(target.transform);
        result[i] = std::move(item);
    }
    return result;
}

py::tuple pose_bones(const pose_view& self) {
    const auto& source = self.pose->bone_poses;
    py::tuple result{source.count};
    for (std::size_t i = 0; i < source.count; ++i) {
        const auto& pose = source.data[i];
        py::dict item;
        item["node"] = py::cast(node_view{self.owner, pose.bone_node});
        item["bone_to_world"] = to_tuple(pose.bone_to_world);
        item["bone_to_parent"] = to_tuple(pose.bone_to_parent);
        result[i] = std::move(item);
    }
    return result;
}

py::object blob(const ufbx_blob& source) {
    return source.data == nullptr
        ? py::object{py::bytes{}}
        : py::object{py::bytes{static_cast<const char*>(source.data), source.size}};
}

py::tuple audio_clips(const audio_layer_view& self) {
    const auto& source = self.layer->clips;
    py::tuple result{source.count};
    for (std::size_t i = 0; i < source.count; ++i) {
        result[i] = py::cast(audio_clip_view{self.owner, source.data[i]});
    }
    return result;
}

py::object dom_value(const ufbx_dom_value& value) {
    switch (value.type) {
    case UFBX_DOM_VALUE_NUMBER: return py::float_{value.value_float};
    case UFBX_DOM_VALUE_STRING: return py::str{to_string(value.value_str)};
    default: return blob(value.value_blob);
    }
}

py::tuple dom_values(const dom_node_view& self) {
    const auto& source = self.node->values;
    py::tuple result{source.count};
    for (std::size_t i = 0; i < source.count; ++i) {
        result[i] = dom_value(source.data[i]);
    }
    return result;
}

py::tuple dom_children(const dom_node_view& self) {
    const auto& source = self.node->children;
    py::tuple result{source.count};
    for (std::size_t i = 0; i < source.count; ++i) {
        result[i] = py::cast(dom_node_view{self.owner, source.data[i]});
    }
    return result;
}

}  // namespace

void bind_miscellaneous(py::module_& module) {
    py::class_<constraint_view>(module, "Constraint")
        .def_property_readonly("name", [](const constraint_view& self) {
            return to_string(self.constraint->name);
        })
        .def_property_readonly("type", [](const constraint_view& self) {
            return std::string{constraint_type_name(self.constraint->type)};
        })
        .def_property_readonly("node", [](const constraint_view& self) {
            return optional_node(self.owner, self.constraint->node);
        })
        .def_property_readonly("targets", &constraint_targets)
        .def_property_readonly("weight", [](const constraint_view& self) {
            return self.constraint->weight;
        })
        .def_property_readonly("active", [](const constraint_view& self) {
            return self.constraint->active;
        })
        .def_property_readonly("transform_offset", [](const constraint_view& self) {
            return to_transform(self.constraint->transform_offset);
        });

    py::class_<pose_view>(module, "Pose")
        .def_property_readonly("name", [](const pose_view& self) {
            return to_string(self.pose->name);
        })
        .def_property_readonly("is_bind_pose", [](const pose_view& self) {
            return self.pose->is_bind_pose;
        })
        .def_property_readonly("bones", &pose_bones);

    py::class_<audio_clip_view>(module, "AudioClip")
        .def_property_readonly("name", [](const audio_clip_view& self) {
            return to_string(self.clip->name);
        })
        .def_property_readonly("filename", [](const audio_clip_view& self) {
            return to_string(self.clip->filename);
        })
        .def_property_readonly("relative_filename", [](const audio_clip_view& self) {
            return to_string(self.clip->relative_filename);
        })
        .def_property_readonly("embedded_content", [](const audio_clip_view& self) {
            return blob(self.clip->content);
        });

    py::class_<audio_layer_view>(module, "AudioLayer")
        .def_property_readonly("name", [](const audio_layer_view& self) {
            return to_string(self.layer->name);
        })
        .def_property_readonly("clips", &audio_clips);

    py::class_<dom_node_view>(module, "DomNode")
        .def_property_readonly("name", [](const dom_node_view& self) {
            return to_string(self.node->name);
        })
        .def_property_readonly("values", &dom_values)
        .def_property_readonly("children", &dom_children)
        .def("__len__", [](const dom_node_view& self) {
            return self.node->children.count;
        })
        .def("__iter__", [](const dom_node_view& self) {
            return py::iter(dom_children(self));
        });
}

}  // namespace pyfbx
