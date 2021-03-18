#pragma once

// GLFW is necessary for online rendering
// GLFW includes the vulkan files for us so we don't think about it
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
// Offsets of data in data structures passed to the shaders are multiples of 16 bytes
// NOTE: With nested data structures, this will not work properly, and still need std::alignas
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp> // vector and matrix types
#include <glm/gtc/matrix_transform.hpp> // transformations

#include <chrono> // time
#include <iostream>
#include <vector>
#include <array>
#include <set>
#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include <optional>
#include <cstdint> // for UINT32_MAX
#include <algorithm> // for std::min/max
#include <fstream>

#include "molecule_struct.h"


// glm types match GLSL types exactly
struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec3 normal;
    uint32_t render_type; // 0 for molecule, 1 for orbital

    static VkVertexInputBindingDescription getBindingDescription() {
        // The rate of memory loading throughout vertices and memory layout and access behavior
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0; // index of binding (maybe all bindings stored globally?)
        bindingDescription.stride = sizeof(Vertex); // obvious
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // get next vertex after each vertex
        return bindingDescription;
    }

    // There are 2, that describe how to extract position and color respectively
    static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};
        // position is first
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT; // VK_FORMAT_R32G32_SFLOAT means vec2, and I change it to vec3 :)
        attributeDescriptions[0].offset = offsetof(Vertex, pos); // structs just have this!
        // color is second
        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT; // This just means vec3 :(
        attributeDescriptions[1].offset = offsetof(Vertex, color); // structs just have this!
        // normal is third
        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, normal);
        // render_type is third
        attributeDescriptions[3].binding = 0;
        attributeDescriptions[3].location = 3;
        attributeDescriptions[3].format = VK_FORMAT_R32_UINT; // This just means uint32_t :(
        attributeDescriptions[3].offset = offsetof(Vertex, render_type); // structs just have this!
        return attributeDescriptions;
    }
};

// note that with glm, the data is EXACTLY the same as in the shaders, so we can wholesale copy it
// The alignas is necessary because in the shaders it will be aligned to multiples of 16 bytes so we have to match that 
struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
    alignas(16) glm::vec3 camera_pos;
    alignas(16) glm::vec3 light_pos;
    alignas(16) glm::vec3 light_color;
};


class TriangleRenderer
{
public:
    void run();

    TriangleRenderer(const std::vector<MoleculeStruct::MolecularDataOneFrame*>& set_trajectory) : trajectory(set_trajectory) {
        vertices = {
			{{-4.5f, -4.5f, 7.0f}, {1.0f, 0.0f, 0.0f}, {0, 1.f, 0}, 0},
			{{-3, -4.5f, 6.0f}, {0.0f, 1.0f, 0.0f}, {0, 1.f, 0}, 0},
			{{-3.f, 4.5f, 7.0f}, {0.0f, 0.0f, 1.0f}, {0, 1.f, 0}, 0},
			{{-4.5f, -3.f, 7.5f}, {1.0f, 1.0f, 1.0f}, {0, 1.f, 0}, 0}
		};

        indices = {
            0, 1, 2, 2, 3, 0
        };

    }

private:
    // AOS raw data hardcoded
    std::vector<Vertex> vertices;
    // Each triple is a triangle
    std::vector<uint32_t> indices;

    std::vector<MoleculeStruct::MolecularDataOneFrame*> trajectory;

    GLFWwindow * window; // the window rendering everything
    VkInstance instance; // holds all the Vulkan information
    VkDebugUtilsMessengerEXT debugMessenger; // Need an object for this because it is specific to our application and layers.
    VkSurfaceKHR surface; // The software window that we render to. Do NOT need this to do offline rendering
    VkPhysicalDevice physicalDevice; // Implicitly destroyed when the instance is destroyed
    VkDevice device; // physical devices can have multiple logical devices
    VkQueue graphicsQueue; // All operations are passed into this work queue that handles telling the device to actually run them
    VkQueue presentQueue; // Same as graphics queue but for operations that can be presented to the screen. Same queue.
    // swapchain related variables
    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages; // The image handles in the swap chain
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    // swapchain image views
    std::vector<VkImageView> swapChainImageViews;

    // Graphics pipeline
    VkRenderPass renderPass; // this is what to do with the output of the fragment shader
    VkDescriptorSetLayout descriptorSetLayout; // how descriptors for shaders layed out
    VkDescriptorPool descriptorPool; // Where descriptors can be allocated
    std::vector<VkDescriptorSet> descriptorSets; // The descriptor sets themselves
    VkPipelineLayout pipelineLayout; // Graphics pipeline
    VkPipeline graphicsPipeline;
    std::vector<VkFramebuffer> swapChainFramebuffers; // a frame buffer holds an image view for each pipeline attachment
  
    // Commands
    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;

    // semaphores are for parts of the pipeline to wait until the last has been completed
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences; // for the CPU to wait until GPU has room for more images in drawFrame
    std::vector<VkFence> imagesInFlight; // For if another frame is using this image, same fences as above
    size_t currentFrame = 0;

    bool framebufferResized = false;

    // Not sure why we need both of these. Maybe vertexBufferMemory is GPU memory?
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    // vectors containing a uniform buffer per swap chain, so that different frames in flight can
    // access and update concurrently without stepping on each others' toes.
    // This makes sense because different frames have different associated timestamps and different
    // perspective transforms
    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;

    // Depth related fields
    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

    // High level of customizability in this function, requires going through the Vulkan SDK
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

    // need to instantiate debugging stuff
    void setupDebugMessenger();

    // helper function that writes specific application info
    void createInstance();

    // must be a static function because GLFW can't handle methods
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<TriangleRenderer*>(glfwGetWindowUserPointer(window));
    app->framebufferResized = true;
    }

    // Actually creates a physical window using GLFW, not necessary for offline rendering
    void initWindow();

    void initVulkan();

    // DEVICE SUITABILITY

    // Need to be able to tell which queue to use, if there are multiple options.
    // We need at least one that supports graphics operations and one that supports presenting stuff to the screen
    // They will in practice likely be the same queue index, but whatever
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily; // The main thing
        std::optional<uint32_t> presentFamily; // Tells us if device can present images to the surface

        bool isComplete() {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    // We need a type of queue that supports our desired commands
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

    // This goes through desired device-level extensions and makes sure they are support
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);

    // This is what we use to filter the devices by what type of device we need and what features it should have
    // One way to extend this is by ordering the devices by score and just choosing the highest scoring one, so we
    // can fall back on the CPU in the worst case or something like that.
    bool isDeviceSuitable(VkPhysicalDevice device);

    // Picks a GPU (lavapipe software renderer pretends to be a GPU I think)
    void pickPhysicalDevice();


    // LOGICAL DEVICE CREATION

    void createLogicalDevice();

    // WINDOW SURFACE CREATION

    void createSurface();


    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

    // Surface format setting
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

    // presentation mode is most important setting in swap chain because it has the conditions for showing the final images
    // Four different modes:
    //  Immediate: images get displayed right away
    //  FIFO: swap chain is a queue and screen takes from the front every refresh
    //  FIFO relaxed: Same thing, but if application is late and the queue is empty, the next image is transferred right away instead of going to the queue
    //  Mailbox: Same is FIFO but insatead of blocking when queue is full the images in the queue are replaced with newer ones, used for triple buffering
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

    // There is a special case where the capabilities max extent listed is the maximum 32 bit int
    // In this case, we have to go through GLFW and figure out in pixel coordinates what the extent actually is
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    void createSwapChain();

    void cleanupSwapChain();

    // Whenever somethinig changes we probably have to recreate the entire swap chain from scratch
    // This recreates everything dependent on the swap chain as well
    void recreateSwapChain();

    // IMAGE VIEWS

    // Image views are created by us while "images" aren't
    void createImageViews();


    // 1. Acquires an image from the swap chain
    // 2. Executes command buffer with that image as in attachment in the framebuffer
    // 3. Return the image to the swap chain for presentation to the window
    void drawFrame();

    //  MAIN LOOP

    // Event loop to keep window open until it should be closed
    void mainLoop();

    void cleanup();

    bool checkValidationLayerSupport();

    std::vector<const char*> getRequiredExtensions();

    // This gets called for debug messages
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, // an integer representing verbose < info < warning < error
        VkDebugUtilsMessageTypeFlagsEXT messageType, // general, validation, or performance (efficiency)
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, // struct of pMessage, pObjects, objectCount
        void* pUserData) { // allows user defined data to be passed through

        if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
            std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
        }

        return VK_FALSE;
    }

    // GRAPHICS PIPELINE

    // to read a binary file into a byte array
    static std::vector<char> readFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);
        if (!file.is_open()) {
            std::cout << filename << std::endl;
            throw std::runtime_error("Failed to open file");
        }

        // We start at the end of the file and find out how long it is before seeking back
        size_t fileSize = (size_t) file.tellg();
        std::vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();
        return buffer;
    }

    VkShaderModule createShaderModule(const std::vector<char>& code);

    void createGraphicsPipeline();

    // These are not the actual framebuffers, I think they are objects describing what
    // frame buffers the swap chain will render to
    void createRenderPass();

    void createFramebuffers();


    // Commands


    // Commands submitted to device queues, and have to choose which queue to use
    void createCommandPool();

    void createCommandBuffers();

    void renderCommandBuffers();

    // Drawing

    void createSyncObjects();


    // Vertex Buffer

    // returns the index of the memory type in the physical device that satisfies the
    // flags in typeFilter and the properties in properties
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

    void createVertexBuffer();

    void createIndexBuffer();

    void updateVertexAndIndexBuffer(float time);

    // need to use command buffers to transfer the vertices from staging buffer to vertex buffer
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    // Model View Projection

    void createDescriptorSetLayout();

    // There is one uniform buffer for every swap chain image
    void createUniformBuffers();

    void updateUniformBuffer(float time, uint32_t currentImage);

    void createDescriptorPool();

    void createDescriptorSets();

    void createDepthResources();
    VkFormat findDepthFormat();
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
    void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
    bool hasStencilComponent(VkFormat format);
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
};

