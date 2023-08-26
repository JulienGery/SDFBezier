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
    glm::vec4 color;
};

struct CurvesData
{
    glm::uint PointsCount;
    glm::uint padding[3];
    glm::vec4 centerAndTopRight;
    glm::vec4 bisector;
    glm::vec2 P_0, p1, p2, p3;
};

struct GenerateImage
{
    //glm::vec4 derivatetives;
    //glm::vec4 startAndEndCurve;
    //glm::vec4 previusCurveNextCurveColor;
    //color
    //angle

    glm::vec4 startAndEndPoint;
    glm::vec4 previusCurveEndDerivateAndNextcurveStartDerivate;
    glm::vec4 colorAndAngles;
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

    //void recordComputeCommandBuffers(const size_t index);
    void renderSDF(const size_t width, const size_t height, const uint32_t curvesCount /*const uint32_t curveIndex*/);
    void generateImage(const size_t curvesCount);
    //void render();
    std::vector<OUTPUTIMAGE> getImage();

    void updateUBO(const std::vector<float>& angles);
    void updateUBO(const std::vector<CurvesData>& curvesData);
    void updateUBO(const std::vector<GenerateImage>& vector);

    std::vector<glm::vec4> getResult();
    //void getCoeff(std::vector<glm::vec4[4]>& result);


private:
    size_t m_Width = 1920, m_Height = 1080;

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
    void getBuffferContent(const VkBuffer buffer, const size_t bufferSize, void* dst);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void cleanup();

    template <class PushConstantStruct>
    void recordComputeCommandBuffer(VkCommandBuffer commandBuffer, VkPipeline pipeline, VkPipelineLayout pipelineLayout, const size_t count, const PushConstantStruct pushConstant);
    void submitCommandBuffer(VkCommandBuffer commandBuffer);
    void createSyncObjects();

    template <class PushConstantStruct>
    void createComputePipelineHelper(const std::string& path, VkPipeline& pipeline, VkPipelineLayout& pipelineLayout);

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
};
