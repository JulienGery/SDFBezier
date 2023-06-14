#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <string>

#include "glm/vec4.hpp"
#include "glm/vec2.hpp"
#include "glm/geometric.hpp"

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
            if (glm::distance(a._roots[i], _roots[i]) > 1e-05f)
                return false;
        return true;
    }
};


struct OUTPUTIMAGE
{
    uint32_t color;
    uint32_t padding[3];
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

class Renderer
{
public:
    Renderer() { initVulkan(); }
    ~Renderer() { cleanup(); }
    
    void recordComputeCommandBuffers(const size_t index);
    void render();
    std::vector<OUTPUTIMAGE> getImage();

    void generateImage();

    void updateUBO(const std::vector<float>& angles);


    std::vector<glm::vec4> getResult();
    //void getCoeff(std::vector<glm::vec4[4]>& result);

    glm::vec2 P_0, p1, p2, p3;
    size_t m_Width = 3840, m_Height = 2400;
    size_t m_CurveIndex;
    glm::vec4 m_Bis = {};

private:
    VkInstance m_Instance;
    VkDebugUtilsMessengerEXT m_DebugMessenger;

    VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
    VkFence m_Fence;
    VkDevice m_Device;

    VkCommandPool m_CommandPool;
    std::vector<VkCommandBuffer> m_ComputeCommandBuffers;

    VkQueue m_ComputeQueue;
    VkDescriptorSetLayout m_ComputeDescriptorSetLayout;

    std::vector<VkPipeline> m_Pipelines;
    std::vector<VkPipelineLayout> m_PipelinesLayouts;

    VkBuffer m_resultBuffer;
    VkDeviceMemory m_resultBufferMemory;
    VkBuffer m_OutputBuffer;
    VkDeviceMemory m_OutputBufferMemory;

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

    void createComputePipelineHelper(const std::string& path, VkPipeline& pipeline, VkPipelineLayout& pipelineLayout);

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
};