#include "bindings.hpp"

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>

namespace pyfbx {
namespace {

py::tuple clusters(const skin_deformer_view& self) {
    const auto& source = self.deformer->clusters;
    py::tuple result{source.count};
    for (std::size_t i = 0; i < source.count; ++i) {
        result[i] = py::cast(skin_cluster_view{self.owner, source.data[i]});
    }
    return result;
}

py::tuple vertex_weights(const skin_deformer_view& self, const std::size_t vertex_index) {
    if (vertex_index >= self.deformer->vertices.count) {
        throw py::index_error{};
    }
    const ufbx_skin_vertex vertex = self.deformer->vertices.data[vertex_index];
    py::tuple result{vertex.num_weights};
    for (std::uint32_t i = 0; i < vertex.num_weights; ++i) {
        const ufbx_skin_weight weight = self.deformer->weights.data[vertex.weight_begin + i];
        py::dict item;
        item["cluster"] = py::cast(skin_cluster_view{
            self.owner, self.deformer->clusters.data[weight.cluster_index]});
        item["weight"] = weight.weight;
        result[i] = std::move(item);
    }
    return result;
}

py::object cluster_bone(const skin_cluster_view& self) {
    return self.cluster->bone_node == nullptr
        ? py::object{py::none()}
        : py::cast(node_view{self.owner, self.cluster->bone_node});
}

py::tuple cluster_weights(const skin_cluster_view& self) {
    py::tuple result{self.cluster->num_weights};
    for (std::size_t i = 0; i < self.cluster->num_weights; ++i) {
        result[i] = py::make_tuple(
            self.cluster->vertices.data[i], self.cluster->weights.data[i]);
    }
    return result;
}

py::tuple blend_channels(const blend_deformer_view& self) {
    const auto& source = self.deformer->channels;
    py::tuple result{source.count};
    for (std::size_t i = 0; i < source.count; ++i) {
        result[i] = py::cast(blend_channel_view{self.owner, source.data[i]});
    }
    return result;
}

py::tuple blend_keyframes(const blend_channel_view& self) {
    const auto& source = self.channel->keyframes;
    py::tuple result{source.count};
    for (std::size_t i = 0; i < source.count; ++i) {
        const auto& keyframe = source.data[i];
        py::dict item;
        item["shape"] = py::cast(blend_shape_view{self.owner, keyframe.shape});
        item["target_weight"] = keyframe.target_weight;
        item["effective_weight"] = keyframe.effective_weight;
        result[i] = std::move(item);
    }
    return result;
}

py::tuple shape_offsets(const blend_shape_view& self) {
    py::tuple result{self.shape->num_offsets};
    for (std::size_t i = 0; i < self.shape->num_offsets; ++i) {
        py::dict item;
        item["vertex"] = self.shape->offset_vertices.data[i];
        item["position"] = to_tuple(self.shape->position_offsets.data[i]);
        item["normal"] = self.shape->normal_offsets.count > i
            ? py::object{to_tuple(self.shape->normal_offsets.data[i])}
            : py::object{py::none()};
        item["weight"] = self.shape->offset_weights.count > i
            ? py::object{py::float_{self.shape->offset_weights.data[i]}}
            : py::object{py::none()};
        result[i] = std::move(item);
    }
    return result;
}

const char* interpolation_name(const ufbx_interpolation interpolation) noexcept {
    switch (interpolation) {
    case UFBX_INTERPOLATION_CONSTANT_PREV: return "constant_previous";
    case UFBX_INTERPOLATION_CONSTANT_NEXT: return "constant_next";
    case UFBX_INTERPOLATION_LINEAR: return "linear";
    case UFBX_INTERPOLATION_CUBIC: return "cubic";
    default: return "unknown";
    }
}

py::tuple curve_keyframes(const animation_curve_view& self) {
    const auto& source = self.curve->keyframes;
    py::tuple result{source.count};
    for (std::size_t i = 0; i < source.count; ++i) {
        const auto& keyframe = source.data[i];
        py::dict item;
        item["time"] = keyframe.time;
        item["value"] = keyframe.value;
        item["interpolation"] = interpolation_name(keyframe.interpolation);
        item["left_tangent"] = py::make_tuple(keyframe.left.dx, keyframe.left.dy);
        item["right_tangent"] = py::make_tuple(keyframe.right.dx, keyframe.right.dy);
        result[i] = std::move(item);
    }
    return result;
}

py::tuple layer_curves(const animation_layer_view& self) {
    std::size_t count{};
    for (std::size_t i = 0; i < self.layer->anim_values.count; ++i) {
        const ufbx_anim_value* value = self.layer->anim_values.data[i];
        for (const ufbx_anim_curve* curve : value->curves) {
            count += curve != nullptr ? 1 : 0;
        }
    }
    py::tuple result{count};
    std::size_t dst{};
    for (std::size_t i = 0; i < self.layer->anim_values.count; ++i) {
        const ufbx_anim_value* value = self.layer->anim_values.data[i];
        for (const ufbx_anim_curve* curve : value->curves) {
            if (curve != nullptr) {
                result[dst++] = py::cast(animation_curve_view{self.owner, curve});
            }
        }
    }
    return result;
}

}  // namespace

void bind_deform_animation(py::module_& module) {
    py::class_<skin_cluster_view>(module, "SkinCluster")
        .def_property_readonly("name", [](const skin_cluster_view& self) {
            return to_string(self.cluster->name);
        })
        .def_property_readonly("bone", &cluster_bone)
        .def_property_readonly("geometry_to_bone", [](const skin_cluster_view& self) {
            return to_tuple(self.cluster->geometry_to_bone);
        })
        .def_property_readonly("weights", &cluster_weights);

    py::class_<skin_deformer_view>(module, "SkinDeformer")
        .def_property_readonly("name", [](const skin_deformer_view& self) {
            return to_string(self.deformer->name);
        })
        .def_property_readonly("clusters", &clusters)
        .def_property_readonly("num_vertices", [](const skin_deformer_view& self) {
            return self.deformer->vertices.count;
        })
        .def_property_readonly("max_weights_per_vertex", [](const skin_deformer_view& self) {
            return self.deformer->max_weights_per_vertex;
        })
        .def("weights", &vertex_weights, py::arg("vertex_index"));

    py::class_<blend_shape_view>(module, "BlendShape")
        .def_property_readonly("name", [](const blend_shape_view& self) {
            return to_string(self.shape->name);
        })
        .def_property_readonly("offsets", &shape_offsets)
        .def_property_readonly("num_offsets", [](const blend_shape_view& self) {
            return self.shape->num_offsets;
        });

    py::class_<blend_channel_view>(module, "BlendChannel")
        .def_property_readonly("name", [](const blend_channel_view& self) {
            return to_string(self.channel->name);
        })
        .def_property_readonly("weight", [](const blend_channel_view& self) {
            return self.channel->weight;
        })
        .def_property_readonly("keyframes", &blend_keyframes);

    py::class_<blend_deformer_view>(module, "BlendDeformer")
        .def_property_readonly("name", [](const blend_deformer_view& self) {
            return to_string(self.deformer->name);
        })
        .def_property_readonly("channels", &blend_channels);

    py::class_<animation_curve_view>(module, "AnimationCurve")
        .def_property_readonly("name", [](const animation_curve_view& self) {
            return to_string(self.curve->name);
        })
        .def_property_readonly("keyframes", &curve_keyframes)
        .def_property_readonly("time_range", [](const animation_curve_view& self) {
            return py::make_tuple(self.curve->min_time, self.curve->max_time);
        })
        .def_property_readonly("value_range", [](const animation_curve_view& self) {
            return py::make_tuple(self.curve->min_value, self.curve->max_value);
        })
        .def("evaluate", [](const animation_curve_view& self, const double time, const double fallback) {
            return ufbx_evaluate_curve(self.curve, time, fallback);
        }, py::arg("time"), py::arg("fallback") = 0.0);

    py::class_<animation_layer_view>(module, "AnimationLayer")
        .def_property_readonly("name", [](const animation_layer_view& self) {
            return to_string(self.layer->name);
        })
        .def_property_readonly("weight", [](const animation_layer_view& self) {
            return self.layer->weight;
        })
        .def_property_readonly("additive", [](const animation_layer_view& self) {
            return self.layer->additive;
        })
        .def_property_readonly("curves", &layer_curves);
}

}  // namespace pyfbx
