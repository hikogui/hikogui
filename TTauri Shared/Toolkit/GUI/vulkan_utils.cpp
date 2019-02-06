//
//  vulkan_utils.cpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-05.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#include "vulkan_utils.hpp"

namespace TTauri {
namespace Toolkit {
namespace GUI {

void checkRequiredExtensions(const std::vector<const char *> &requiredExtensions)
{
    auto availableExtensions = std::unordered_set<std::string>();
    for (auto availableExtensionProperties: vk::enumerateInstanceExtensionProperties()) {
        availableExtensions.insert(std::string(availableExtensionProperties.extensionName));
    }

    for (auto requiredExtension: requiredExtensions) {
        if (availableExtensions.count(requiredExtension) == 0) {
            BOOST_THROW_EXCEPTION(VulkanError());
        }
    }
}

bool meetsRequiredLimits(const vk::PhysicalDevice &physicalDevice, const vk::PhysicalDeviceLimits &requiredLimits)
{
    auto meetsLimits = true;

    return meetsLimits;
}

bool hasRequiredFeatures(const vk::PhysicalDevice &physicalDevice, const vk::PhysicalDeviceFeatures &requiredFeatures)
{
    auto availableFeatures = physicalDevice.getFeatures();
    auto meetsRequirements = true;

    meetsRequirements &= requiredFeatures.robustBufferAccess ? availableFeatures.robustBufferAccess : true;
    meetsRequirements &= requiredFeatures.fullDrawIndexUint32 ? availableFeatures.fullDrawIndexUint32 : true;
    meetsRequirements &= requiredFeatures.imageCubeArray ? availableFeatures.imageCubeArray : true;
    meetsRequirements &= requiredFeatures.independentBlend ? availableFeatures.independentBlend : true;
    meetsRequirements &= requiredFeatures.geometryShader ? availableFeatures.geometryShader : true;
    meetsRequirements &= requiredFeatures.tessellationShader ? availableFeatures.tessellationShader : true;
    meetsRequirements &= requiredFeatures.sampleRateShading ? availableFeatures.sampleRateShading : true;
    meetsRequirements &= requiredFeatures.dualSrcBlend ? availableFeatures.dualSrcBlend : true;
    meetsRequirements &= requiredFeatures.logicOp ? availableFeatures.logicOp : true;
    meetsRequirements &= requiredFeatures.multiDrawIndirect ? availableFeatures.multiDrawIndirect : true;
    meetsRequirements &= requiredFeatures.drawIndirectFirstInstance ? availableFeatures.drawIndirectFirstInstance : true;
    meetsRequirements &= requiredFeatures.depthClamp ? availableFeatures.depthClamp : true;
    meetsRequirements &= requiredFeatures.depthBiasClamp ? availableFeatures.depthBiasClamp : true;
    meetsRequirements &= requiredFeatures.fillModeNonSolid ? availableFeatures.fillModeNonSolid : true;
    meetsRequirements &= requiredFeatures.depthBounds ? availableFeatures.depthBounds : true;
    meetsRequirements &= requiredFeatures.wideLines ? availableFeatures.wideLines : true;
    meetsRequirements &= requiredFeatures.largePoints ? availableFeatures.largePoints : true;
    meetsRequirements &= requiredFeatures.alphaToOne ? availableFeatures.alphaToOne : true;
    meetsRequirements &= requiredFeatures.multiViewport ? availableFeatures.multiViewport : true;
    meetsRequirements &= requiredFeatures.samplerAnisotropy ? availableFeatures.samplerAnisotropy : true;
    meetsRequirements &= requiredFeatures.textureCompressionETC2 ? availableFeatures.textureCompressionETC2 : true;
    meetsRequirements &= requiredFeatures.textureCompressionASTC_LDR ? availableFeatures.textureCompressionASTC_LDR : true;
    meetsRequirements &= requiredFeatures.textureCompressionBC ? availableFeatures.textureCompressionBC : true;
    meetsRequirements &= requiredFeatures.occlusionQueryPrecise ? availableFeatures.occlusionQueryPrecise : true;
    meetsRequirements &= requiredFeatures.pipelineStatisticsQuery ? availableFeatures.pipelineStatisticsQuery : true;
    meetsRequirements &= requiredFeatures.vertexPipelineStoresAndAtomics ? availableFeatures.vertexPipelineStoresAndAtomics : true;
    meetsRequirements &= requiredFeatures.fragmentStoresAndAtomics ? availableFeatures.fragmentStoresAndAtomics : true;
    meetsRequirements &= requiredFeatures.shaderTessellationAndGeometryPointSize ? availableFeatures.shaderTessellationAndGeometryPointSize : true;
    meetsRequirements &= requiredFeatures.shaderImageGatherExtended ? availableFeatures.shaderImageGatherExtended : true;
    meetsRequirements &= requiredFeatures.shaderStorageImageExtendedFormats ? availableFeatures.shaderStorageImageExtendedFormats : true;
    meetsRequirements &= requiredFeatures.shaderStorageImageMultisample ? availableFeatures.shaderStorageImageMultisample : true;
    meetsRequirements &= requiredFeatures.shaderStorageImageReadWithoutFormat ? availableFeatures.shaderStorageImageReadWithoutFormat : true;
    meetsRequirements &= requiredFeatures.shaderStorageImageWriteWithoutFormat ? availableFeatures.shaderStorageImageWriteWithoutFormat : true;
    meetsRequirements &= requiredFeatures.shaderUniformBufferArrayDynamicIndexing ? availableFeatures.shaderUniformBufferArrayDynamicIndexing : true;
    meetsRequirements &= requiredFeatures.shaderSampledImageArrayDynamicIndexing ? availableFeatures.shaderSampledImageArrayDynamicIndexing : true;
    meetsRequirements &= requiredFeatures.shaderStorageBufferArrayDynamicIndexing ? availableFeatures.shaderStorageBufferArrayDynamicIndexing : true;
    meetsRequirements &= requiredFeatures.shaderStorageImageArrayDynamicIndexing ? availableFeatures.shaderStorageImageArrayDynamicIndexing : true;
    meetsRequirements &= requiredFeatures.shaderClipDistance ? availableFeatures.shaderClipDistance : true;
    meetsRequirements &= requiredFeatures.shaderCullDistance ? availableFeatures.shaderCullDistance : true;
    meetsRequirements &= requiredFeatures.shaderFloat64 ? availableFeatures.shaderFloat64 : true;
    meetsRequirements &= requiredFeatures.shaderInt64 ? availableFeatures.shaderInt64 : true;
    meetsRequirements &= requiredFeatures.shaderInt16 ? availableFeatures.shaderInt16 : true;
    meetsRequirements &= requiredFeatures.shaderResourceResidency ? availableFeatures.shaderResourceResidency : true;
    meetsRequirements &= requiredFeatures.shaderResourceMinLod ? availableFeatures.shaderResourceMinLod : true;
    meetsRequirements &= requiredFeatures.sparseBinding ? availableFeatures.sparseBinding : true;
    meetsRequirements &= requiredFeatures.sparseResidencyBuffer ? availableFeatures.sparseResidencyBuffer : true;
    meetsRequirements &= requiredFeatures.sparseResidencyImage2D ? availableFeatures.sparseResidencyImage2D : true;
    meetsRequirements &= requiredFeatures.sparseResidencyImage3D ? availableFeatures.sparseResidencyImage3D : true;
    meetsRequirements &= requiredFeatures.sparseResidency2Samples ? availableFeatures.sparseResidency2Samples : true;
    meetsRequirements &= requiredFeatures.sparseResidency4Samples ? availableFeatures.sparseResidency4Samples : true;
    meetsRequirements &= requiredFeatures.sparseResidency8Samples ? availableFeatures.sparseResidency8Samples : true;
    meetsRequirements &= requiredFeatures.sparseResidency16Samples ? availableFeatures.sparseResidency16Samples : true;
    meetsRequirements &= requiredFeatures.sparseResidencyAliased ? availableFeatures.sparseResidencyAliased : true;
    meetsRequirements &= requiredFeatures.variableMultisampleRate ? availableFeatures.variableMultisampleRate : true;
    meetsRequirements &= requiredFeatures.inheritedQueries ? availableFeatures.inheritedQueries : true;

    return meetsRequirements;
}

}}}
