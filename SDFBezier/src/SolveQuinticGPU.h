#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <string>

#include "glm/vec4.hpp"
#include "glm/vec2.hpp"

struct Roots
{
    glm::vec4 _roots[5];

    glm::vec4& operator[](size_t index)
    {
        return _roots[index];
    }

    bool operator==(const Roots& a) const
    {
        for (size_t i = 0; i < 5; i++)
            if (a._roots[i] != _roots[i])
                return false;
        return true;
    }
};

struct Coeff
{
    glm::vec4 _coeff[6];

    Coeff(const glm::vec2& c)
    {
        _coeff[0] = glm::vec4(c, 0, 0);
    }

    Coeff(const std::vector<glm::vec2>& cs)
    {
        for (size_t i = 0; i < cs.size() && i < 6; i++)
            _coeff[i] = glm::vec4(cs[i], 0, 0);
    }

    Coeff()
    {
    }

    glm::vec4& operator[](size_t index)
    {
        return _coeff[index];
    }
};

class SolveQuinticGPU
{
public:
	SolveQuinticGPU() { initVulkan(); }
	~SolveQuinticGPU() { cleanup(); }

    void execute();
    std::vector<Roots> getResult();
    std::vector<Coeff> getCoeff();


private:
    size_t m_Width = 540, m_Height = 960;
    VkInstance m_Instance;
    VkDebugUtilsMessengerEXT m_DebugMessenger;

    VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
    VkFence m_Fence;
    VkDevice m_Device;

    VkCommandPool m_CommandPool;
    VkCommandBuffer m_ComputeCommandBuffer;

    VkQueue m_ComputeQueue;
    VkDescriptorSetLayout m_ComputeDescriptorSetLayout;

    VkPipelineLayout m_ComputePipelineLayout;
    VkPipeline m_ComputePipeline;
    VkPipelineLayout m_BuildCoefPipelineLayout;
    VkPipeline m_BuildCoefPipeline;
    VkPipelineLayout m_ComputePipelineInitLayout;
    VkPipeline m_ComputePipelineInit;

    VkBuffer m_CoefBuffer;
    VkDeviceMemory m_CoefBufferMemory;
    VkBuffer m_ApproximationBuffer;
    VkDeviceMemory m_ApproximationBufferMemory;

    VkBuffer m_UniformBuffer;
    VkDeviceMemory m_UniformBufferMemory;
    void* m_UniformBufferMapped;

    VkDescriptorPool m_DescriptorPool;
    VkDescriptorSet m_ComputeDescriptorSet;


	void initVulkan();

    void createInstance();
    void setupDebugMessenger();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createComputeDescriptorSetLayout();
    void createComputePipeline();
    void createCommandPool();
    void createShaderStorageBuffers();
    void createUniformBuffers();
    void createDescriptorPool();
    void createComputeDescriptorSets();
    void createComputeCommandBuffers();
    VkShaderModule createShaderModule(const std::vector<char>& code);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void cleanup();

    void recordComputeCommandBuffer(VkCommandBuffer commandBuffer, VkPipeline pipeline, VkPipelineLayout pipelineLayout, const size_t count);
    void submitCommandBuffer(VkCommandBuffer commandBuffer);
    void createSyncObjects();

    void updateUBO();
    void createComputePipelineHelper(const std::string& path, VkPipeline& pipeline, VkPipelineLayout& pipelineLayout);

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
};