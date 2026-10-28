#pragma once
#include "BuildImage.h"

namespace rw
{
class DeployBuildImageInstallManifest;

class DeployBuildImageInstallChunk : public BuildImageInstallChunk
{
public:
    DeployBuildImageInstallChunk(DeployBuildImageInstallManifest* manifest, const Guid& id = c_emptyGuid, uint index = 0)
        : BuildImageInstallChunk((BuildImageInstallManifest*)manifest, id, index)
    {}

public:
    const Vec<Guid>& getRequiredChunks() const
        { return m_requiredChunks; }

    FB_STORAGE_API void addRequiredInstallChunk(const DeployBuildImageInstallChunk* chunk);
    FB_STORAGE_API void removeRequiredInstallChunk(const Guid& id);

private:
    Vec<Guid> m_requiredChunks;
};

class DeployBuildImageLayout;

class DeployBuildImageInstallManifest : public BuildImageInstallManifest
{
public:
    DeployBuildImageInstallManifest(DeployBuildImageLayout* layout = nullptr)
        : BuildImageInstallManifest((BuildImageLayout*)layout)
    {}


};

class DeployBuildImage;

class DeployBuildImageLayout : public BuildImageLayout
{
public:
    DeployBuildImageLayout(DeployBuildImage* buildImage = nullptr)
        : BuildImageLayout((BuildImage*)buildImage)
    {}
};

class DeployBuildImage : public BuildImage
{
public:
    DeployBuildImage(VirtualFileSystem* vfs, const char* mount, const char* dir)
        : BuildImage(vfs, mount, dir)
    {}
};
}