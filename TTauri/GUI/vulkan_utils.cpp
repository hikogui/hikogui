//
//  vulkan_utils.cpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-05.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#include "vulkan_utils.hpp"

namespace TTauri {
namespace GUI {

bool meetsRequiredLimits(const vk::PhysicalDevice &physicalDevice, const vk::PhysicalDeviceLimits &requiredLimits)
{
    return true;
}

bool hasRequiredFeatures(const vk::PhysicalDevice &physicalDevice, const vk::PhysicalDeviceFeatures &requiredFeatures)
{
    auto const availableFeatures = physicalDevice.getFeatures();
    auto meetsRequirements = true;

    meetsRequirements &= (requiredFeatures.robustBufferAccess == VK_TRUE) ? (availableFeatures.robustBufferAccess == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.fullDrawIndexUint32 == VK_TRUE) ? (availableFeatures.fullDrawIndexUint32 == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.imageCubeArray == VK_TRUE) ? (availableFeatures.imageCubeArray == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.independentBlend == VK_TRUE) ? (availableFeatures.independentBlend == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.geometryShader == VK_TRUE) ? (availableFeatures.geometryShader == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.tessellationShader == VK_TRUE) ? (availableFeatures.tessellationShader == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.sampleRateShading == VK_TRUE) ? (availableFeatures.sampleRateShading == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.dualSrcBlend == VK_TRUE) ? (availableFeatures.dualSrcBlend == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.logicOp == VK_TRUE) ? (availableFeatures.logicOp == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.multiDrawIndirect == VK_TRUE) ? (availableFeatures.multiDrawIndirect == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.drawIndirectFirstInstance == VK_TRUE) ? (availableFeatures.drawIndirectFirstInstance == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.depthClamp == VK_TRUE) ? (availableFeatures.depthClamp == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.depthBiasClamp == VK_TRUE) ? (availableFeatures.depthBiasClamp == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.fillModeNonSolid == VK_TRUE) ? (availableFeatures.fillModeNonSolid == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.depthBounds == VK_TRUE) ? (availableFeatures.depthBounds == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.wideLines == VK_TRUE) ? (availableFeatures.wideLines == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.largePoints == VK_TRUE) ? (availableFeatures.largePoints == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.alphaToOne == VK_TRUE) ? (availableFeatures.alphaToOne == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.multiViewport == VK_TRUE) ? (availableFeatures.multiViewport == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.samplerAnisotropy == VK_TRUE) ? (availableFeatures.samplerAnisotropy == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.textureCompressionETC2 == VK_TRUE) ? (availableFeatures.textureCompressionETC2 == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.textureCompressionASTC_LDR == VK_TRUE) ? (availableFeatures.textureCompressionASTC_LDR == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.textureCompressionBC == VK_TRUE) ? (availableFeatures.textureCompressionBC == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.occlusionQueryPrecise == VK_TRUE) ? (availableFeatures.occlusionQueryPrecise == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.pipelineStatisticsQuery == VK_TRUE) ? (availableFeatures.pipelineStatisticsQuery == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.vertexPipelineStoresAndAtomics == VK_TRUE) ? (availableFeatures.vertexPipelineStoresAndAtomics == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.fragmentStoresAndAtomics == VK_TRUE) ? (availableFeatures.fragmentStoresAndAtomics == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.shaderTessellationAndGeometryPointSize == VK_TRUE) ? (availableFeatures.shaderTessellationAndGeometryPointSize == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.shaderImageGatherExtended == VK_TRUE) ? (availableFeatures.shaderImageGatherExtended == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.shaderStorageImageExtendedFormats == VK_TRUE) ? (availableFeatures.shaderStorageImageExtendedFormats == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.shaderStorageImageMultisample == VK_TRUE) ? (availableFeatures.shaderStorageImageMultisample == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.shaderStorageImageReadWithoutFormat == VK_TRUE) ? (availableFeatures.shaderStorageImageReadWithoutFormat == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.shaderStorageImageWriteWithoutFormat == VK_TRUE) ? (availableFeatures.shaderStorageImageWriteWithoutFormat == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.shaderUniformBufferArrayDynamicIndexing == VK_TRUE) ? (availableFeatures.shaderUniformBufferArrayDynamicIndexing == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.shaderSampledImageArrayDynamicIndexing == VK_TRUE) ? (availableFeatures.shaderSampledImageArrayDynamicIndexing == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.shaderStorageBufferArrayDynamicIndexing == VK_TRUE) ? (availableFeatures.shaderStorageBufferArrayDynamicIndexing == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.shaderStorageImageArrayDynamicIndexing == VK_TRUE) ? (availableFeatures.shaderStorageImageArrayDynamicIndexing == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.shaderClipDistance == VK_TRUE) ? (availableFeatures.shaderClipDistance == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.shaderCullDistance == VK_TRUE) ? (availableFeatures.shaderCullDistance == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.shaderFloat64 == VK_TRUE) ? (availableFeatures.shaderFloat64 == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.shaderInt64 == VK_TRUE) ? (availableFeatures.shaderInt64 == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.shaderInt16 == VK_TRUE) ? (availableFeatures.shaderInt16 == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.shaderResourceResidency == VK_TRUE) ? (availableFeatures.shaderResourceResidency == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.shaderResourceMinLod == VK_TRUE) ? (availableFeatures.shaderResourceMinLod == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.sparseBinding == VK_TRUE) ? (availableFeatures.sparseBinding == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.sparseResidencyBuffer == VK_TRUE) ? (availableFeatures.sparseResidencyBuffer == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.sparseResidencyImage2D == VK_TRUE) ? (availableFeatures.sparseResidencyImage2D == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.sparseResidencyImage3D == VK_TRUE) ? (availableFeatures.sparseResidencyImage3D == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.sparseResidency2Samples == VK_TRUE) ? (availableFeatures.sparseResidency2Samples == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.sparseResidency4Samples == VK_TRUE) ? (availableFeatures.sparseResidency4Samples == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.sparseResidency8Samples == VK_TRUE) ? (availableFeatures.sparseResidency8Samples == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.sparseResidency16Samples == VK_TRUE) ? (availableFeatures.sparseResidency16Samples == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.sparseResidencyAliased == VK_TRUE) ? (availableFeatures.sparseResidencyAliased == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.variableMultisampleRate == VK_TRUE) ? (availableFeatures.variableMultisampleRate == VK_TRUE) : true;
    meetsRequirements &= (requiredFeatures.inheritedQueries == VK_TRUE) ? (availableFeatures.inheritedQueries == VK_TRUE) : true;

    return meetsRequirements;
}

}}
