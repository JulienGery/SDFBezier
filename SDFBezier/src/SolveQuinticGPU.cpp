#include "SolveQuinticGPU.h"
#include "glm/vec2.hpp"
#include "glm/vec4.hpp"
#include "glm/geometric.hpp"

#include <iostream>
#include <cstring>
#include <exception>
#include <array>
#include <optional>
#include <fstream>
#include <Walnut/Timer.h>
#include <string>

//#define NDEBUG 1
#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

// Buffers:
// 0     ubo
// 1     coef
// 2     approximations = root = closestPoints
// 3     ClosestPoints 

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};


struct UniformBufferObject
{
    glm::vec2 P_0, p1, p2, p3;
    glm::vec4 bis;
    int maxIndex, width, height, curveIndex;
};

void coutRoots(const Roots& roots)
{
    for (size_t i = 0; i < 5; i++)
        std::cout << roots._roots[i].x << ' ' << roots._roots[i].y << "  ";
    std::cout << '\n';
}

struct QueueFamilyIndices {
    std::optional<uint32_t> computeFamily;

    bool isComplete() { return computeFamily.has_value(); }
};

std::vector<char> readFile(const std::string& fileName)
{
    std::ifstream file(fileName, std::ios::binary | std::ios::ate);
    if (!file.is_open())
        throw std::runtime_error("failed to open " + fileName);

    size_t fileSize = file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice m_Device) {
    QueueFamilyIndices indices;

    // Assign index to queue families that could be found
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_Device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_Device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) {
            indices.computeFamily = i;
        }
        if (indices.isComplete()) {
            break;
        }

        i++;
    }

    return indices;
}

bool isDeviceSuitable(VkPhysicalDevice m_Device) {
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(m_Device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(m_Device, &deviceFeatures);

    // will not run on integrated gpu at 4k
    //if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
        //return false;

    std::cout << "\033[36m" << deviceProperties.deviceName << "\033[0m" << '\n';
    QueueFamilyIndices indices = findQueueFamilies(m_Device);
    return indices.isComplete();
}


void SolveQuinticGPU::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = SolveQuinticGPU::debugCallback;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}


bool checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}

void SolveQuinticGPU::execute(const size_t index)
{
    updateUBO();

    //Walnut::ScopedTimer total{ "total" };
    {
        //Walnut::ScopedTimer timer{ "buildCoef" };
        submitCommandBuffer(m_ComputeCommandBuffers[0]);
    }

    {
        //Walnut::ScopedTimer timer{ "init" };
        if(index != 3)
            submitCommandBuffer(m_ComputeCommandBuffers[1]);
    }

    {
        //Walnut::ScopedTimer timer{ "solve" };
        submitCommandBuffer(m_ComputeCommandBuffers[2]);
    }

    {
        //Walnut::ScopedTimer timer{ "compare" };
        if(index != 2)
            submitCommandBuffer(m_ComputeCommandBuffers[3]);
    }

}

void SolveQuinticGPU::initVulkan()
{
    createInstance();
    setupDebugMessenger();
    pickPhysicalDevice();
    createLogicalDevice();
    createCommandPool();
    createComputeDescriptorSetLayout();
    createComputePipeline();
    createShaderStorageBuffers();
    createUniformBuffers();
    createDescriptorPool();
    createComputeDescriptorSets();
    createComputeCommandBuffers();
    createSyncObjects();
}

void SolveQuinticGPU::createInstance()
{
    if (enableValidationLayers && !checkValidationLayerSupport()) {
        throw std::runtime_error("validation layers requested, but not available!");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    const std::vector<const char*> requiredExtensions = { "VK_EXT_debug_report", "VK_EXT_debug_utils" };
    createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();


    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }
    else {
        createInfo.enabledLayerCount = 0;

        createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &m_Instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }
}

void SolveQuinticGPU::setupDebugMessenger()
{
    if (!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(m_Instance, &createInfo, nullptr, &m_DebugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

void SolveQuinticGPU::pickPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);
    if (deviceCount == 0)
        throw std::runtime_error("failed to find GPU wih vulkan support!");

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());

    for (const auto& m_Device : devices) {
        if (isDeviceSuitable(m_Device)) {
            m_PhysicalDevice = m_Device;
            break;
        }
    }

    if (m_PhysicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

void SolveQuinticGPU::createLogicalDevice()
{
    QueueFamilyIndices indices = findQueueFamilies(m_PhysicalDevice);

    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = indices.computeFamily.value();
    queueCreateInfo.queueCount = 1;

    float queuePriority = 1.f;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    VkPhysicalDeviceFeatures deviceFeatures{};
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.queueCreateInfoCount = 1;

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = 0;

    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical m_Device!");
    }

    vkGetDeviceQueue(m_Device, indices.computeFamily.value(), 0, &m_ComputeQueue);
}

void SolveQuinticGPU::createComputeDescriptorSetLayout()
{
    std::array<VkDescriptorSetLayoutBinding, 4> layoutBindings{};
    layoutBindings[0].binding = 0;
    layoutBindings[0].descriptorCount = 1;
    layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layoutBindings[0].pImmutableSamplers = nullptr;
    layoutBindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    layoutBindings[1].binding = 1;
    layoutBindings[1].descriptorCount = 1;
    layoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    layoutBindings[1].pImmutableSamplers = nullptr;
    layoutBindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    layoutBindings[2].binding = 2;
    layoutBindings[2].descriptorCount = 1;
    layoutBindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    layoutBindings[2].pImmutableSamplers = nullptr;
    layoutBindings[2].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    layoutBindings[3].binding = 3;
    layoutBindings[3].descriptorCount = 1;
    layoutBindings[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    layoutBindings[3].pImmutableSamplers = nullptr;
    layoutBindings[3].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 4;
    layoutInfo.pBindings = layoutBindings.data();

    if (vkCreateDescriptorSetLayout(m_Device, &layoutInfo, nullptr, &m_ComputeDescriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create compute descriptor set layout!");
    }
}

void SolveQuinticGPU::createComputePipelineHelper(const std::string& path, VkPipeline& pipeline, VkPipelineLayout& pipelineLayout)
{
    auto computeShaderCode = readFile(path);

    VkShaderModule computeShaderModule = createShaderModule(computeShaderCode);

    VkPipelineShaderStageCreateInfo computeShaderStageInfo{};
    computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    computeShaderStageInfo.module = computeShaderModule;
    computeShaderStageInfo.pName = "main";

    /*VkPushConstantRange pushConstant;
    pushConstant.offset = 0;
    pushConstant.size = sizeof(UniformBufferObject);
    pushConstant.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;*/

    // TODO switch to push constants

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &m_ComputeDescriptorSetLayout;
    //pipelineLayoutInfo.pPushConstantRanges = &pushConstant;
    //pipelineLayoutInfo.pushConstantRangeCount = 1;

    if (vkCreatePipelineLayout(m_Device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create compute pipeline layout!");
    }

    VkComputePipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.stage = computeShaderStageInfo;


    if (vkCreateComputePipelines(m_Device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create compute pipeline!");
    }

    vkDestroyShaderModule(m_Device, computeShaderModule, nullptr);
}

void SolveQuinticGPU::createComputePipeline()
{   
    const std::vector<std::string> shaders = 
    {
        "../shaders/out/buildCoef2.spv",
        "../shaders/out/solve2.spv",
        "../shaders/out/final2.spv",

        "../shaders/out/buildCoef3.spv",
        "../shaders/out/init3.spv",
        "../shaders/out/solve3.spv",
        "../shaders/out/final3.spv",

        "../shaders/out/buildCoef4.spv",
        "../shaders/out/init4.spv",
        "../shaders/out/solve4.spv",
        "../shaders/out/final4.spv"
    };

    m_Pipelines.resize(shaders.size());
    m_PipelinesLayouts.resize(shaders.size());

    for (size_t i = 0; i < shaders.size(); i++)
        createComputePipelineHelper(shaders[i], m_Pipelines[i], m_PipelinesLayouts[i]);
}

std::vector<glm::vec4> SolveQuinticGPU::getResult()
{
    std::vector<glm::vec4> result(m_Width * m_Height);
    const size_t BufferSize = result.size() * sizeof(glm::vec4);

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(BufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(m_Device, stagingBufferMemory, 0, BufferSize, 0, &data);
    copyBuffer(m_resultBuffer, stagingBuffer, BufferSize);
    memcpy(result.data(), data, BufferSize);
    vkUnmapMemory(m_Device, stagingBufferMemory);

    vkDestroyBuffer(m_Device, stagingBuffer, nullptr);
    vkFreeMemory(m_Device, stagingBufferMemory, nullptr);

    return result;
}

void SolveQuinticGPU::getCoeff(std::vector<glm::vec4[4]>& result)
{
    const size_t BufferSize = result.size() * sizeof(glm::vec4[4]);
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(BufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(m_Device, stagingBufferMemory, 0, BufferSize, 0, &data);
    copyBuffer(m_CoefBuffer, stagingBuffer, BufferSize);
    memcpy(result.data(), data, BufferSize);
    vkUnmapMemory(m_Device, stagingBufferMemory);

    vkDestroyBuffer(m_Device, stagingBuffer, nullptr);
    vkFreeMemory(m_Device, stagingBufferMemory, nullptr);

}

void SolveQuinticGPU::createCommandPool()
{
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(m_PhysicalDevice);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.computeFamily.value();

    if (vkCreateCommandPool(m_Device, &poolInfo, nullptr, &m_CommandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics command pool!");
    }
}

void SolveQuinticGPU::createShaderStorageBuffers()
{
    VkDeviceSize bufferSize = m_Width * m_Height;
    createBuffer(bufferSize * sizeof(Coeff), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_CoefBuffer, m_CoefBufferMemory);
    createBuffer(bufferSize * sizeof(Roots), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_ApproximationBuffer, m_ApproximationBufferMemory);
    createBuffer(bufferSize * sizeof(glm::vec4), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_resultBuffer, m_resultBufferMemory);
}

void SolveQuinticGPU::createUniformBuffers()
{
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_UniformBuffer, m_UniformBufferMemory);

    vkMapMemory(m_Device, m_UniformBufferMemory, 0, bufferSize, 0, &m_UniformBufferMapped);
}

void SolveQuinticGPU::createDescriptorPool()
{
    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(1);

    poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(3);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 2;
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(1);

    if (vkCreateDescriptorPool(m_Device, &poolInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void SolveQuinticGPU::createComputeDescriptorSets()
{
    std::vector<VkDescriptorSetLayout> layouts(1, m_ComputeDescriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_DescriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(1);
    allocInfo.pSetLayouts = layouts.data();

    if (vkAllocateDescriptorSets(m_Device, &allocInfo, &m_ComputeDescriptorSet) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    VkDescriptorBufferInfo uniformBufferInfo{};
    uniformBufferInfo.buffer = m_UniformBuffer;
    uniformBufferInfo.offset = 0;
    uniformBufferInfo.range = sizeof(UniformBufferObject);

    std::array<VkWriteDescriptorSet, 4> descriptorWrites{};
    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = m_ComputeDescriptorSet;
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &uniformBufferInfo;

    VkDescriptorBufferInfo storageCoef{};
    storageCoef.buffer = m_CoefBuffer;
    storageCoef.offset = 0;
    storageCoef.range = sizeof(Coeff) * m_Width * m_Height;

    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = m_ComputeDescriptorSet;
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pBufferInfo = &storageCoef;

    VkDescriptorBufferInfo storageCurrentApproximation{};
    storageCurrentApproximation.buffer = m_ApproximationBuffer;
    storageCurrentApproximation.offset = 0;
    storageCurrentApproximation.range = sizeof(Roots) * m_Width * m_Height;

    descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[2].dstSet = m_ComputeDescriptorSet;
    descriptorWrites[2].dstBinding = 2;
    descriptorWrites[2].dstArrayElement = 0;
    descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorWrites[2].descriptorCount = 1;
    descriptorWrites[2].pBufferInfo = &storageCurrentApproximation;

    VkDescriptorBufferInfo result{};
    result.buffer = m_resultBuffer;
    result.offset = 0;
    result.range = sizeof(glm::vec4) * m_Width * m_Height;

    descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[3].dstSet = m_ComputeDescriptorSet;
    descriptorWrites[3].dstBinding = 3;
    descriptorWrites[3].dstArrayElement = 0;
    descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorWrites[3].descriptorCount = 1;
    descriptorWrites[3].pBufferInfo = &result;

    vkUpdateDescriptorSets(m_Device, 4, descriptorWrites.data(), 0, nullptr);
}

void SolveQuinticGPU::createComputeCommandBuffers()
{

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_CommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)4;
    m_ComputeCommandBuffers.resize(4);

    if (vkAllocateCommandBuffers(m_Device, &allocInfo, m_ComputeCommandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate compute command buffers!");
    }
}

VkShaderModule SolveQuinticGPU::createShaderModule(const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
    //createInfo.pNext = 

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(m_Device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
}

uint32_t SolveQuinticGPU::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

void SolveQuinticGPU::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(m_Device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_Device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(m_Device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(m_Device, buffer, bufferMemory, 0);
}


void SolveQuinticGPU::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = m_CommandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(m_Device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(m_ComputeQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_ComputeQueue);

    vkFreeCommandBuffers(m_Device, m_CommandPool, 1, &commandBuffer);
}

void SolveQuinticGPU::cleanup()
{
    for (size_t i = 0; i < m_Pipelines.size(); i++)
    {
        vkDestroyPipeline(m_Device, m_Pipelines[i], nullptr);
        vkDestroyPipelineLayout(m_Device, m_PipelinesLayouts[i], nullptr);

    }
    
    vkDestroyBuffer(m_Device, m_UniformBuffer, nullptr);
    vkFreeMemory(m_Device, m_UniformBufferMemory, nullptr);
    vkDestroyBuffer(m_Device, m_ApproximationBuffer, nullptr);
    vkFreeMemory(m_Device, m_ApproximationBufferMemory, nullptr);
    vkDestroyBuffer(m_Device, m_CoefBuffer, nullptr);
    vkFreeMemory(m_Device, m_CoefBufferMemory, nullptr);
    vkDestroyBuffer(m_Device, m_resultBuffer, nullptr);
    vkFreeMemory(m_Device, m_resultBufferMemory, nullptr);

    vkDestroyDescriptorPool(m_Device, m_DescriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(m_Device, m_ComputeDescriptorSetLayout, nullptr);

    vkDestroyFence(m_Device, m_Fence, nullptr);

    vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);
    vkDestroyDevice(m_Device, nullptr);

    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
    }
}



void SolveQuinticGPU::recordComputeCommandBuffer(VkCommandBuffer commandBuffer, VkPipeline pipeline, VkPipelineLayout pipelineLayout, const size_t count)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
        throw std::runtime_error("failed to record command buffer");

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1, &m_ComputeDescriptorSet, 0, nullptr);

    vkCmdDispatch(commandBuffer, count, 1, 1);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record compute command buffer!");
    }
}

void SolveQuinticGPU::recordComputeCommandBuffers(const size_t curveSize)
{
    for (size_t i = 0; i < m_ComputeCommandBuffers.size(); i++)
        vkResetCommandBuffer(m_ComputeCommandBuffers[i], 0);

    switch (curveSize)
    {
        case 2:
            for (size_t i = 0; i < 3; i++)
                recordComputeCommandBuffer(m_ComputeCommandBuffers[i], m_Pipelines[i], m_PipelinesLayouts[i], m_Width * m_Height / 64 + 1);
            break;

        case 3:
            for (size_t i = 0; i < 4; i++)
                recordComputeCommandBuffer(m_ComputeCommandBuffers[i], m_Pipelines[i + 3], m_PipelinesLayouts[i + 3], m_Width * m_Height / 64 + 1);
            break;

        case 4:
            for (size_t i = 0; i < 4; i++)
                recordComputeCommandBuffer(m_ComputeCommandBuffers[i], m_Pipelines[i + 7], m_PipelinesLayouts[i + 7], m_Width * m_Height / 64 + 1);
            break;
    }

}

void SolveQuinticGPU::submitCommandBuffer(VkCommandBuffer commandBuffer)
{
    vkResetFences(m_Device, 1, &m_Fence);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.pSignalSemaphores = nullptr;

    if (vkQueueSubmit(m_ComputeQueue, 1, &submitInfo, m_Fence) != VK_SUCCESS)
        throw std::runtime_error("failed to submit command buffer! ");

    vkWaitForFences(m_Device, 1, &m_Fence, VK_TRUE, UINT64_MAX);
    vkResetFences(m_Device, 1, &m_Fence);
}

void SolveQuinticGPU::createSyncObjects()
{
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if (vkCreateFence(m_Device, &fenceInfo, nullptr, &m_Fence) != VK_SUCCESS)
        throw std::runtime_error("failed to create fence\n");
}

void SolveQuinticGPU::updateUBO()
{
    UniformBufferObject ubo{};
    ubo.P_0 = P_0;
    ubo.p1 = p1;
    ubo.p2 = p2;
    ubo.p3 = p3;
    ubo.width = m_Width;
    ubo.height = m_Height;
    ubo.maxIndex = m_Width * m_Height;
    ubo.curveIndex = m_CurveIndex;
    ubo.bis = m_Bis;

    memcpy(m_UniformBufferMapped, &ubo, sizeof(ubo));
}



VKAPI_ATTR VkBool32 VKAPI_CALL SolveQuinticGPU::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}