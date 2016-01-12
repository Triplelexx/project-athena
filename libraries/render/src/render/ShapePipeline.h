//
//  ShapePipeline.h
//  render/src/render
//
//  Created by Zach Pomerantz on 12/31/15.
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_render_ShapePipeline_h
#define hifi_render_ShapePipeline_h

#include <gpu/Batch.h>
#include <RenderArgs.h>

namespace render {

class ShapeKey {
public:
    enum FlagBit {
        TRANSLUCENT = 0,
        LIGHTMAP,
        TANGENTS,
        SPECULAR,
        EMISSIVE,
        SKINNED,
        STEREO,
        DEPTH_ONLY,
        WIREFRAME,

        OWN_PIPELINE,
        INVALID,

        NUM_FLAGS, // Not a valid flag
    };
    using Flags = std::bitset<NUM_FLAGS>;

    Flags _flags;

    ShapeKey() : _flags{0} {}
    ShapeKey(const Flags& flags) : _flags{flags} {}

    class Builder {
    public:
        Builder() {}
        Builder(ShapeKey key) : _flags{key._flags} {}

        ShapeKey build() const { return ShapeKey{_flags}; }

        Builder& withTranslucent() { _flags.set(TRANSLUCENT); return (*this); }
        Builder& withLightmap() { _flags.set(LIGHTMAP); return (*this); }
        Builder& withTangents() { _flags.set(TANGENTS); return (*this); }
        Builder& withSpecular() { _flags.set(SPECULAR); return (*this); }
        Builder& withEmissive() { _flags.set(EMISSIVE); return (*this); }
        Builder& withSkinned() { _flags.set(SKINNED); return (*this); }
        Builder& withStereo() { _flags.set(STEREO); return (*this); }
        Builder& withDepthOnly() { _flags.set(DEPTH_ONLY); return (*this); }
        Builder& withWireframe() { _flags.set(WIREFRAME); return (*this); }

        Builder& withOwnPipeline() { _flags.set(OWN_PIPELINE); return (*this); }
        Builder& invalidate() { _flags.set(INVALID); return (*this); }

        static const ShapeKey ownPipeline() { return Builder().withOwnPipeline(); }
        static const ShapeKey invalid() { return Builder().invalidate(); }

    protected:
        friend class ShapeKey;
        Flags _flags{0};
    };
    ShapeKey(const Builder& builder) : ShapeKey{builder._flags} {}

    class Filter {
    public:
        Filter(Flags flags, Flags mask) : _flags{flags}, _mask{mask} {}
        Filter(const ShapeKey& key) : _flags{ key._flags } { _mask.set(); }

        // Build a standard filter (will always exclude OWN_PIPELINE, INVALID)
        class Builder {
        public:
            Builder();

            Filter build() const { return Filter(_flags, _mask); }

            Builder& withOpaque() { _flags.reset(TRANSLUCENT); _mask.set(TRANSLUCENT); return (*this); }
            Builder& withTranslucent() { _flags.set(TRANSLUCENT); _mask.set(TRANSLUCENT); return (*this); }

            Builder& withLightmap() { _flags.reset(LIGHTMAP); _mask.set(LIGHTMAP); return (*this); }
            Builder& withoutLightmap() { _flags.set(LIGHTMAP); _mask.set(LIGHTMAP); return (*this); }

            Builder& withTangents() { _flags.reset(TANGENTS); _mask.set(TANGENTS); return (*this); }
            Builder& withoutTangents() { _flags.set(TANGENTS); _mask.set(TANGENTS); return (*this); }

            Builder& withSpecular() { _flags.reset(SPECULAR); _mask.set(SPECULAR); return (*this); }
            Builder& withoutSpecular() { _flags.set(SPECULAR); _mask.set(SPECULAR); return (*this); }

            Builder& withEmissive() { _flags.reset(EMISSIVE); _mask.set(EMISSIVE); return (*this); }
            Builder& withoutEmissive() { _flags.set(EMISSIVE); _mask.set(EMISSIVE); return (*this); }

            Builder& withSkinned() { _flags.reset(SKINNED); _mask.set(SKINNED); return (*this); }
            Builder& withoutSkinned() { _flags.set(SKINNED); _mask.set(SKINNED); return (*this); }

            Builder& withStereo() { _flags.reset(STEREO); _mask.set(STEREO); return (*this); }
            Builder& withoutStereo() { _flags.set(STEREO); _mask.set(STEREO); return (*this); }

            Builder& withDepthOnly() { _flags.reset(DEPTH_ONLY); _mask.set(DEPTH_ONLY); return (*this); }
            Builder& withoutDepthOnly() { _flags.set(DEPTH_ONLY); _mask.set(DEPTH_ONLY); return (*this); }

            Builder& withWireframe() { _flags.reset(WIREFRAME); _mask.set(WIREFRAME); return (*this); }
            Builder& withoutWireframe() { _flags.set(WIREFRAME); _mask.set(WIREFRAME); return (*this); }

        protected:
            friend class Filter;
            Flags _flags{0};
            Flags _mask{0};
        };
        Filter(const Filter::Builder& builder) : Filter(builder._flags, builder._mask) {}
    protected:
        friend class ShapePlumber;
        Flags _flags{0};
        Flags _mask{0};
    };

    bool hasLightmap() const { return _flags[LIGHTMAP]; }
    bool hasTangents() const { return _flags[TANGENTS]; }
    bool hasSpecular() const { return _flags[SPECULAR]; }
    bool hasEmissive() const { return _flags[EMISSIVE]; }
    bool isTranslucent() const { return _flags[TRANSLUCENT]; }
    bool isSkinned() const { return _flags[SKINNED]; }
    bool isStereo() const { return _flags[STEREO]; }
    bool isDepthOnly() const { return _flags[DEPTH_ONLY]; }
    bool isWireFrame() const { return _flags[WIREFRAME]; }

    bool hasOwnPipeline() const { return _flags[OWN_PIPELINE]; }
    bool isValid() const { return !_flags[INVALID]; }

    // Hasher for use in unordered_maps
    class Hash {
    public:
        size_t operator() (const ShapeKey& key) const {
            return std::hash<ShapeKey::Flags>()(key._flags);
        }
    };

    // Comparator for use in unordered_maps
    class KeyEqual {
    public:
        bool operator()(const ShapeKey& lhs, const ShapeKey& rhs) const { return lhs._flags == rhs._flags; }
    };
};

inline QDebug operator<<(QDebug debug, const ShapeKey& renderKey) {
    if (renderKey.isValid()) {
        if (renderKey.hasOwnPipeline()) {
            debug << "[ShapeKey: OWN_PIPELINE]";
        } else {
            debug << "[ShapeKey:"
                << "hasLightmap:" << renderKey.hasLightmap()
                << "hasTangents:" << renderKey.hasTangents()
                << "hasSpecular:" << renderKey.hasSpecular()
                << "hasEmissive:" << renderKey.hasEmissive()
                << "isTranslucent:" << renderKey.isTranslucent()
                << "isSkinned:" << renderKey.isSkinned()
                << "isStereo:" << renderKey.isStereo()
                << "isDepthOnly:" << renderKey.isDepthOnly()
                << "isWireFrame:" << renderKey.isWireFrame()
                << "]";
        }
    } else {
        debug << "[ShapeKey: INVALID]";
    }
    return debug;
}

// Rendering abstraction over gpu::Pipeline and map locations
// Meta-information (pipeline and locations) to render a shape
class ShapePipeline {
public:
    class Slot {
    public:
        static const int SKINNING_GPU = 2;
        static const int MATERIAL_GPU = 3;
        static const int DIFFUSE_MAP = 0;
        static const int NORMAL_MAP = 1;
        static const int SPECULAR_MAP = 2;
        static const int LIGHTMAP_MAP = 3;
        static const int LIGHT_BUFFER = 4;
        static const int NORMAL_FITTING_MAP = 10;
    };

    class Locations {
    public:
        int texcoordMatrices;
        int diffuseTextureUnit;
        int normalTextureUnit;
        int specularTextureUnit;
        int emissiveTextureUnit;
        int emissiveParams;
        int normalFittingMapUnit;
        int skinClusterBufferUnit;
        int materialBufferUnit;
        int lightBufferUnit;
    };
    using LocationsPointer = std::shared_ptr<Locations>;

    gpu::PipelinePointer pipeline;
    std::shared_ptr<Locations> locations;

    ShapePipeline() : ShapePipeline(nullptr, nullptr) {}
    ShapePipeline(gpu::PipelinePointer pipeline, std::shared_ptr<Locations> locations) :
        pipeline(pipeline), locations(locations) {}
};
using ShapePipelinePointer = std::shared_ptr<ShapePipeline>;

class ShapePlumber {
public:
    using Key = ShapeKey;
    using Filter = Key::Filter;
    using Pipeline = ShapePipeline;
    using PipelinePointer = ShapePipelinePointer;
    using PipelineMap = std::unordered_map<ShapeKey, PipelinePointer, ShapeKey::Hash, ShapeKey::KeyEqual>;
    using Slot = Pipeline::Slot;
    using Locations = Pipeline::Locations;
    using LocationsPointer = Pipeline::LocationsPointer;
    using StateSetter = std::function<void(Key, gpu::State&)>;
    using BatchSetter = std::function<void(gpu::Batch&, PipelinePointer)>;

    ShapePlumber();
    explicit ShapePlumber(BatchSetter batchSetter);

    void addPipeline(const Key& key, gpu::ShaderPointer& vertexShader, gpu::ShaderPointer& pixelShader);
    void addPipeline(const Filter& filter, gpu::ShaderPointer& vertexShader, gpu::ShaderPointer& pixelShader);
    const PipelinePointer pickPipeline(RenderArgs* args, const Key& key) const;

protected:
    void addPipelineHelper(const Filter& filter, Key key, int bit, gpu::ShaderPointer pipeline, LocationsPointer locations);
    PipelineMap _pipelineMap;
    StateSetter _stateSetter;
    BatchSetter _batchSetter;
};
using ShapePlumberPointer = std::shared_ptr<ShapePlumber>;

}

#endif // hifi_render_ShapePipeline_h
