/***************************************************************************
# Copyright (c) 2015, NVIDIA CORPORATION. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#  * Neither the name of NVIDIA CORPORATION nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***************************************************************************/
#pragma once
#include <vector>
#include <map>
#include "glm/mat4x4.hpp"
#include "glm/vec3.hpp"
#include "Graphics/Material/BasicMaterial.h"
#include "Graphics/Model/Mesh.h"
#include "Core/Sampler.h"
#include "Graphics/Model/AnimationController.h"

namespace Falcor
{
    class AssimpModelImporter;
    class BinaryModelImporter;
    class SimpleModelImporter;
    class Buffer;
    class Camera;

    /** Class representing a complete model object, including meshes, animations and materials
    */

    class Model : public std::enable_shared_from_this<Model>
    {
    public:
        using SharedPtr = std::shared_ptr<Model>;
        using SharedConstPtr = std::shared_ptr<const Model>;

        enum
        {
            None,
            CompressTextures            = 1,    ///< When loading textures, compress them if they are uncompressed
            GenerateTangentSpace        = 2,    ///< Calculate tangent/bitangent vectors if they are missing. This require the model to have normals and texture coordinates
            FindDegeneratePrimitives    = 4,    ///< Replace degenerate triangles/lines with lines/points. This can create a meshes with topology that wasn't present in the original model.
            AssumeLinearSpaceTextures   = 8,    ///< By default, textures representing colors (diffuse/specular) are interpreted as sRGB data. Use this flag to force linear space for color textures.
            DontMergeMeshes             = 16,   ///< Preserve the original list of meshes in the scene, don't merge meshes with the same material
        };

        /** create a new model from file
        */
        static SharedPtr createFromFile(const std::string& filename, uint32_t flags);

        static const char* kSupportedFileFormatsStr;

        ~Model();

        /** Permanently transform all meshes of the object by the given transform
            \param[in] Transform The transform to apply
        */
        void applyTransform(const glm::mat4& transform);

        /** Export the model to a binary file
        */
        void exportToBinaryFile(const std::string& filename);

        /** Get the model radius
        */
        float getRadius() const { return mRadius; }
        /** Get the model center
        */
        const glm::vec3& getCenter() const { return mCenter; }
        /** Get the number of vertices in the model
        */
        uint32_t getVertexCount() const { return mVertexCount; }
        /** Get the number of primitives in the model
        */
        uint32_t getPrimitiveCount() const { return mPrimitiveCount; }
        /** Get the number of meshes in the model
        */
        uint32_t getMeshCount() const {return uint32_t(mpMeshes.size());}

        /** Get the number of mesh instances in the model
        */
        uint32_t getInstanceCount() const { return mInstanceCount; }

        /** Get the number of unique textures in the model
        */
        uint32_t getTextureCount() const { return (uint32_t)mpTextures.size(); }

        /** Get the number of unique materials in the model
        */
        uint32_t getMaterialCount() const { return (uint32_t)mpMaterials.size(); }

        /** Get the number of unique buffers in the model
        */
        uint32_t getBufferCount() const { return (uint32_t)mpBuffers.size(); }

        /** Get a texture by ID
        */
        const Texture::SharedConstPtr& getTexture(uint32_t MeshID) const { return mpTextures[MeshID]; }

        /** Get a mesh by ID
        */
        const Mesh::SharedPtr& getMesh(uint32_t MeshID) const { return mpMeshes[MeshID]; }

        /** Get a material by ID
        */
        const Material::SharedPtr getMaterial(uint32_t materialID) const { return mpMaterials[materialID]; }

        /** Check if the model contains animations
        */
        bool hasAnimations() const;
        /** Get the number of animations in the model
        */
        uint32_t getAnimationsCount() const;
        /** Animate the active animation. Use SetActiveAnimation() to switch between different animations.
            \param[in] CurrentTime The current global time
        */
        void animate(double currentTime);
        /** Get the animation name from animation ID
        */
        const std::string& getAnimationName(uint32_t animationID) const;
        /** Turn animations off and use bind pose for rendering
        */
        void setBindPose();
        /** Turn animation on and select active animation. Changing the active animation will cause the new animation to play from the beginning
        */
        void setActiveAnimation(uint32_t animationID);

        /** Turn animation on and select active animation. Changing the active animation will cause the new animation to play from the beginning
        */
        uint32_t getActiveAnimation() const;

        /** Check if the model has bones
        */
        bool hasBones() const;
        /** Get the number of bone matrices
        */
        uint32_t getBonesCount() const;
        /** Get a pointer to the bone matrices
        */
        const glm::mat4* getBonesMatrices() const;

        /** Force all texture maps in all materials to use a specific texture sampler with one of their maps
            \param[in] Type The map Type to bind the sampler with
        */
        void bindSamplerToMaterials(const Sampler::SharedPtr& pSampler);

        /** Delete meshes from the model culled by the camera's frustum.\n
            The function will also delete buffers, textures and materials not in use anymore.
        */
        void deleteCulledMeshes(const Camera* pCamera);

        /** Name the model
        */
        void setName(const std::string& Name) { mName = Name; }

        /** Get the model's name
        */
        const std::string& getName() const { return mName; }

		const uint32_t getId() const { return mId; }
        
        /** Reset all global id counter of model, mesh and material
        */
        static void resetGlobalIdCounter();

    protected:
        friend class AssimpModelImporter;
        friend class BinaryModelImporter;
        friend class SimpleModelImporter;
        Model();
        void setAnimationController(AnimationController::UniquePtr pAnimController);
        void addMesh(Mesh::SharedPtr pMesh);
        Material::SharedPtr getOrAddMaterial(const Material::SharedPtr& pMaterial); // If a similar material already exists, will return the existing one. Otherwise, will return the material in pMaterial

        void addBuffer(const Buffer::SharedConstPtr& pBuffer);
        void addTexture(const Texture::SharedConstPtr& pTexture);
    private:
        float mRadius;
        glm::vec3 mCenter;

        uint32_t mVertexCount;
        uint32_t mPrimitiveCount;
        uint32_t mInstanceCount;

		uint32_t mId;

        std::vector<Material::SharedPtr> mpMaterials;

        std::vector<Mesh::SharedPtr> mpMeshes;
        AnimationController::UniquePtr mpAnimationController;
        std::vector<Buffer::SharedConstPtr> mpBuffers;
        std::vector<Texture::SharedConstPtr> mpTextures;

        std::string mName;

		static uint32_t sModelCounter;

        void calculateModelProperties();
        void deleteUnusedMaterials(std::map<const Material*, bool> usedMaterials);
        void deleteUnusedBuffers(std::map<const Buffer*, bool> usedBuffers);
        void compressAllTextures();
    };
}
