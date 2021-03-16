// If all I need is offline rendering I think this is fine
/* #include <vulkan/vulkan.h> */

// The following is if we want to access Windows specific stuff.
// GLFW should by default handle platform specific stuff, so this shouldn't be necessary
/*
 * #define VK_USE_PLATFORM_WIN32_KHR
 * #define GLFW_INCLUDE_VULKAN
 * #include <GLFW/glfw3.h>
 * #define GLFW_EXPOSE_NATIVE_WIN32
 * #include <GLFW/glfw3native.h>
 */

// NOTE: can use libshaderc to compile shaders from within code instead of externally

// GLFW is necessary for online rendering
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
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
#include "molecule_reader.h"
#include "molecule_kernel.h"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;
const int MAX_FRAMES_IN_FLIGHT = 2;

// Specifies what optional validation layers are used to check the code
// for development. Is turned off when compiled not in debug mode
const std::vector<const char*> validationLayers = {
  "VK_LAYER_KHRONOS_validation"
};

// Swap chains are needed for displaying images
const std::vector<const char*> deviceExtensions = {
  VK_KHR_SWAPCHAIN_EXTENSION_NAME
};
// If compiled in debug mode, we use validation layers. This is great
#ifdef NDEBUG
  const bool enableValidationLayers = false;
#else
  const bool enableValidationLayers = true;
#endif

// proxy function that loads an extension debug utility function if it exists
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
  if (func != nullptr) {
    return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
  } else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

// Cleans up the debug utils messenger
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
  if (func != nullptr) {
    func(instance, debugMessenger, pAllocator);
  }
}

// glm types match GLSL types
struct Vertex {
  glm::vec2 pos;
  glm::vec3 color;

  static VkVertexInputBindingDescription getBindingDescription() {
    // The rate of memory loading throughout vertices and memory layout and access behavior
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0; // index of binding (maybe all bindings stored globally?)
    bindingDescription.stride = sizeof(Vertex); // obvious
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // get next vertex after each vertex
    return bindingDescription;
  }

  // There are 2, that describe how to extract position and color respectively
  static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
    std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
    // position is first
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT; // This just means vec2 :(
    attributeDescriptions[0].offset = offsetof(Vertex, pos); // structs just have this!
    // color is second
    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT; // This just means vec3 :(
    attributeDescriptions[1].offset = offsetof(Vertex, color); // structs just have this!
    return attributeDescriptions;
  }
};

// note that with glm, the data is EXACTLY the same as in the shaders, so we can wholesale copy it
struct UniformBufferObject {
  alignas(16) glm::mat4 model;
  alignas(16) glm::mat4 view;
  alignas(16) glm::mat4 proj;
};

// AOS raw data hardcoded
const std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
};

// Each triple is a triangle
// TODO: change to uint32_t for more than 65535 vertices
const std::vector<uint16_t> indices = {
  0,1,2,2,3,0
};


class HelloTriangleApplication {
public:
  void run() {
#ifdef NDEBUG
    printf("RELEASE MODE\n");
#else
    printf("DEBUG MODE\n");
#endif
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
  }

private:
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



  // High level of customizability in this function, requires going through the Vulkan SDK
  void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr; // Optional
  }

  // need to instantiate debugging stuff
  void setupDebugMessenger() {
    if (!enableValidationLayers) return;
    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    // instance is the first argument because this object is specific to the instance
    if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
      throw std::runtime_error("Failed to set up debug messenger!");
    }
  }

  // helper function that writes specific application info
  void createInstance() {
    // First make sure all desired validation layers are available
    if (enableValidationLayers && !checkValidationLayerSupport()) {
      throw std::runtime_error("Validation layers requested, but not available!");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1,0,0);
    appInfo.apiVersion = VK_API_VERSION_1_0;
    // pNext points to future extension information

    // The main struct carries the appInfo as a member
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    // Need extensions from GLFW to interface directly with this particular system.
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        auto extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    // prints the available extensions
    std::cout << "Available extensions:\n";
    for (const auto& extension : extensions) {
      std::cout << "\t" << extension << "\n";
    }

    const char ** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&extensionCount);

    // Need to set up different debug create info for operatiosn that come before the instantiation of the main debug messenger
    // This debug messenger will report on the instance creation AND deletion as an extension in the pNext attribute
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    if (enableValidationLayers) {
      createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
      createInfo.ppEnabledLayerNames = validationLayers.data();

      populateDebugMessengerCreateInfo(debugCreateInfo);
      createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
    }
    else {
      createInfo.enabledLayerCount = 0;
      createInfo.pNext = nullptr;
    }

    // All this function needs to do
    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) // This nullptr is a custom allocator callback
      throw std::runtime_error("Failed to create instance!");
  }

  // must be a static function because GLFW can't handle methods
  static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
    app->framebufferResized = true;
  }

  // Actually creates a physical window using GLFW, not necessary for offline rendering
  void initWindow() {
    // Initializes GLFW libraries
    glfwInit();

    // Initialization settings
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Tells it not to do OpenGL stuff
    /* glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // Disables resizing windows */


    // Window initialization
    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr); // 4th parameter specifies a monitor
    glfwSetWindowUserPointer(window, this); // so that callback has access to this instance
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
  }

  void initVulkan() {
    createInstance(); // initializes the Vulkan library:
    setupDebugMessenger(); // initializes the debugger
    createSurface(); // The actual rendering surface
    pickPhysicalDevice(); // picks the right graphics card
    createLogicalDevice(); // creates a logical device
    createSwapChain(); // actually puts the images on the screen
    createImageViews(); // Does images
    createRenderPass(); // Tells Vulkan about what framebuffers there are to write to
    createDescriptorSetLayout(); // Descriptor sets are what become the uniform buffers eventually I think
    createGraphicsPipeline();
    createFramebuffers();
    createCommandPool(); // Organizes and allocates command buffers
    createVertexBuffer(); // Need to have the vertices before we know how many commands there are? Not sure
    createIndexBuffer(); // Need to have the vertices before we know how many commands there are? Not sure
    createUniformBuffers(); // Store the uniforms
    createDescriptorPool(); // Descriptor sets must be allocated from descriptor pools
    createDescriptorSets();
    createCommandBuffers();
    createSyncObjects();
  }

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
  QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    // We need to support the graphics bit
    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
      // Checks if the queue family has support for presenting to the screen
      VkBool32 presentSupport = false;
      vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
      if (presentSupport)
        indices.presentFamily = i;

      if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        indices.graphicsFamily = i;
      if (indices.isComplete())
        break;
      i++;
    }

    return indices;
  }

  // This goes through desired device-level extensions and makes sure they are support
  bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
    for (const auto& extension: availableExtensions) {
      requiredExtensions.erase(extension.extensionName);
    }
    return requiredExtensions.empty();
  }

  // This is what we use to filter the devices by what type of device we need and what features it should have
  // One way to extend this is by ordering the devices by score and just choosing the highest scoring one, so we
  // can fall back on the CPU in the worst case or something like that.
  bool isDeviceSuitable(VkPhysicalDevice device) {
    // Properties are like name, type, and supported Vulkan version
    // Features are optional support items like texture compression, 64 bit floats, and multi-viewport rendering (needed for VR)
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
    std::cout << "DeviceName: " << deviceProperties.deviceName << std::endl;

    // Checks if there are device extensions supported (probably can't be false if swapChainAdequate but whatever)
    bool extensionsSupported = checkDeviceExtensionSupport(device);


    // Checks if swap chains carry formats and presentation modes.
    // Swap chain support is only possible if extensions are supported in the first place
    bool swapChainAdequate = false;
    if (extensionsSupported) {
      SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
      swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    // Need to make sure a queue supported by this device meets our requirements
    QueueFamilyIndices indices = findQueueFamilies(device);
    return indices.isComplete() && extensionsSupported && swapChainAdequate;

    // TODO: Uncomment this when the program runs on a GPU.
    /* return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU; */
    /* return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU; */
  }

  // Picks a GPU (lavapipe software renderer pretends to be a GPU I think)
  void pickPhysicalDevice() {
    physicalDevice = VK_NULL_HANDLE;
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (deviceCount == 0)
      throw std::runtime_error("Failed to find devices (GPUs or software abstractions) with Vulkan support");

    // Go through all valid devices and see if they meet our requirements
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
    for (const auto& device: devices) {
      if (isDeviceSuitable(device)) {
        physicalDevice = device;
        break;
      }
    }
    if (physicalDevice == VK_NULL_HANDLE) {
      throw std::runtime_error("Failed to find a suitable device");
    }
  }


  // LOGICAL DEVICE CREATION

  void createLogicalDevice() {
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

    // Ensures that the cases where the queues are the same and different are both handled
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    float queuePriority = 1.; // different queues have different priority, but that doesn't matter when there is only one
    for (uint32_t queueFamily : uniqueQueueFamilies) {
      VkDeviceQueueCreateInfo queueCreateInfo{};
      queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
      queueCreateInfo.queueCount = 1;
      queueCreateInfo.pQueuePriorities = &queuePriority;
      queueCreateInfos.push_back(queueCreateInfo);
    }

    // This is where we specify what features we want from the physical device
    // So far there are none
    VkPhysicalDeviceFeatures deviceFeatures{};

    // Fill out the device struct
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;


    // Deals with device extensions
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    // The same validation layers should work on device and instance, so this is deprecated.
    // Older versions of Vulkan distinguished between device level validation and instance level validation
    if (enableValidationLayers) {
      createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
      createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else
      createInfo.enabledLayerCount = 0;

    // Creates the logical device
    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
      throw std::runtime_error("Failed to create a logical device!");
    }
    // Gives us the pointers to the queues we need
    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
  }

  // WINDOW SURFACE CREATION

  void createSurface() {
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
      throw std::runtime_error("failed to create window surface!");
    }
  }


  struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
  };

  SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities); // basic surface capabilities

    // supported surface formats
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
    if (formatCount != 0) {
      details.formats.resize(formatCount); // This makes sense because of how we are copying the data over
      vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    // presentation modes supported
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
    if (presentModeCount != 0) {
      details.presentModes.resize(presentModeCount); // This makes sense because of how we are copying the data over
      vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
  }

  // Surface format setting
  VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    // Each entry has a format and a colorSpace
    // We use SRGB here because it corresponds better with color perception
    for (const auto& availableFormat : availableFormats)
      if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        return availableFormat;

    printf("Desired swap surface format not avilable!\n");
    return availableFormats[0];
  }

  // presentation mode is most important setting in swap chain because it has the conditions for showing the final images
  // Four different modes:
  //  Immediate: images get displayed right away
  //  FIFO: swap chain is a queue and screen takes from the front every refresh
  //  FIFO relaxed: Same thing, but if application is late and the queue is empty, the next image is transferred right away instead of going to the queue
  //  Mailbox: Same is FIFO but insatead of blocking when queue is full the images in the queue are replaced with newer ones, used for triple buffering
  VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    // check if the awesome mailbox is available, and otherwise just use the default
    for (const auto& availablePresentMode : availablePresentModes)
      if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        return availablePresentMode;
    
    return VK_PRESENT_MODE_FIFO_KHR; // Guaranteed to be present
  }

  // There is a special case where the capabilities max extent listed is the maximum 32 bit int
  // In this case, we have to go through GLFW and figure out in pixel coordinates what the extent actually is
  VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != UINT32_MAX) {
      return capabilities.currentExtent;
    }
    else {
      int width, height;
      glfwGetFramebufferSize(window, &width, &height);
      VkExtent2D actualExtent = {
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height)
      };

      actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
      actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));
      return actualExtent;
    }
  }

  void createSwapChain() {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    // Doing 1 more than the minimum because apparently that helps with dealing with the driver
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
      imageCount = swapChainSupport.capabilities.maxImageCount;
    

    // Actually specifies the swap chain parameters
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1; // Number of layers in each image, should be 1 unless stereoscopic 3D is rendered
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // This is where we put what we use the swap chain for
    // VK_IMAGE_USAGE_TRANSFER_DST_BIT for transferring from 1 swap chain to another for post-processing

        // Different settings if presenting and computing graphics are the same queue or not
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
        uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};
        if (indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0; // Optional
            createInfo.pQueueFamilyIndices = nullptr; // Optional
        }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform; // Can uniformly apply transforms to any images in the swap chain

    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // ignore the alpha channel, which allows blending with other windows
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE; // We clip pixels that are obscured by another object

    // if a new swapchain is created while drawing it must have a reference to the old one and get rid of it
    // when it finishes
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    // The actual creation
    // Note that requires the logical device, not the physical one
    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
      throw std::runtime_error("Failed to create swap chain!");

    // Sets the other member variables accordingly
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
  }

  void cleanupSwapChain() {
    for (size_t i = 0; i < swapChainFramebuffers.size(); i++) {
      vkDestroyFramebuffer(device, swapChainFramebuffers[i], nullptr);
    }

    vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

    for (size_t i = 0; i < swapChainImages.size(); i++) { 
      vkDestroyBuffer(device, uniformBuffers[i], nullptr);
      vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
    }

    vkDestroyDescriptorPool(device, descriptorPool, nullptr);

    vkDestroyPipeline(device, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyRenderPass(device, renderPass, nullptr);

    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
      vkDestroyImageView(device, swapChainImageViews[i], nullptr);
    }

    vkDestroySwapchainKHR(device, swapChain, nullptr); 
  }

  // Whenever somethinig changes we probably have to recreate the entire swap chain from scratch
  // This recreates everything dependent on the swap chain as well
  void recreateSwapChain() {

    // when the app is minimized, the width and height become 0 and we just wait
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0) {
      glfwGetFramebufferSize(window, &width, &height);
      glfwWaitEvents();
    }

    // now we wait until the GPU isn't working on the pipeline
    vkDeviceWaitIdle(device); // heuristic for resources being freed up

    createSwapChain();
    createImageViews(); // direct dependence
    createRenderPass(); // format of swap chain images
    createGraphicsPipeline(); // viewport and scissor rectangle size
    createFramebuffers(); // direct dependence
    createUniformBuffers(); // direct dependence
    createDescriptorPool(); // direct dependence
    createDescriptorSets();
    createCommandBuffers(); // direct dependence
  }

  // IMAGE VIEWS

  // Image views are created by us while "images" aren't
  void createImageViews() {
    swapChainImageViews.resize(swapChainImages.size());
    for (size_t i = 0; i < swapChainImages.size(); i++) { 
      VkImageViewCreateInfo createInfo{};
      createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
      createInfo.image = swapChainImages[i];
      createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; // Change this for 1D or 3D textures
      createInfo.format = swapChainImageFormat;

      // default order for rgba
      createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

      // This is about what the image is for and what part of the image is accessible
      // Can control the mipmap levels and how many layers there are. For VR you want multiple layers for left and right eyes
      createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      createInfo.subresourceRange.baseMipLevel = 0;
      createInfo.subresourceRange.levelCount = 1;
      createInfo.subresourceRange.baseArrayLayer = 0;
      createInfo.subresourceRange.layerCount = 1;

      // NOTE: Can do multiple image views with left/right by accessing different layers.
      if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create image views!");
      }
    }
  }


  // 1. Acquires an image from the swap chain
  // 2. Executes command buffer with that image as in attachment in the framebuffer
  // 3. Return the image to the swap chain for presentation to the window
  void drawFrame() {
    // waits for the fence to be signalled by a completing task if necessary
    vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    // 1.
    uint32_t imageIndex; // the index in swapChainImages of the current image
    // UINT54_MAX specifies no timeout
    VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    // if Vulkan thinks the swap chain image is out of date we just recreate it.
    // We wouldn't be able to present it anyway
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
      recreateSwapChain();
      return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
      throw std::runtime_error("Failed to acquire swap chain image!");

    // Check if previous frame is using this swap chain image
    if (imagesInFlight[imageIndex] != VK_NULL_HANDLE)
      vkWaitForFences(device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    imagesInFlight[imageIndex] = inFlightFences[currentFrame];

    updateUniformBuffer(imageIndex); // TODO: maybe put this right after the vkAcquireNextImageKHR?

    // 2.
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    // wait semaphores are the ones the command buffer waits on
    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    // These are the command buffers submitted
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

    // Signal semaphores are called once command buffer is finished
    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(device, 1, &inFlightFences[currentFrame]);

    // submits to the graphics queue with an array of VkSubmitInfos
    // signals the in flight fence when the command is completed
    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
      throw std::runtime_error("Failed to submit draw command buffer!");

    // 3.
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    VkSwapchainKHR swapChains[] = {swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;

    // Does the operation and checks if swap chain needs to be recreated
    result = vkQueuePresentKHR(presentQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
      framebufferResized = false;
      recreateSwapChain();
    }
    else if (result != VK_SUCCESS)
      throw std::runtime_error("failed to present swap chain image!");

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT; // update frame
  }

  //  MAIN LOOP

  // Event loop to keep window open until it should be closed
  void mainLoop() {
    while (!glfwWindowShouldClose(window)) {
      glfwPollEvents();
      drawFrame();
    }

    // Wait on logical device to finish before we start destroying the window
    vkDeviceWaitIdle(device);
  }

  void cleanup() {
        cleanupSwapChain();

    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

    vkDestroyBuffer(device, vertexBuffer, nullptr);
    vkFreeMemory(device, vertexBufferMemory, nullptr);
    vkDestroyBuffer(device, indexBuffer, nullptr);
    vkFreeMemory(device, indexBufferMemory, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(device, inFlightFences[i], nullptr);
    }

    vkDestroyCommandPool(device, commandPool, nullptr);

    vkDestroyDevice(device, nullptr);

    if (enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }

    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);

    glfwDestroyWindow(window);

    glfwTerminate();
  }

  bool checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    // Go through all desired layers and check that they are available
    for (const char* layerName : validationLayers) {
      bool layerFound = false;
      for (const auto& layerProperties : availableLayers) {
        if (strcmp(layerName, layerProperties.layerName) == 0) {
          layerFound = true;
          break;
        }
      }
      if (!layerFound)
        return false;
    }
    return true;
  }

  // This gets a list of all required extensions, including both GLFW required ones and 
  // ones specified for the validation layers we want
    std::vector<const char*> getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    /* extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME); */

    return extensions;
  } 

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

  VkShaderModule createShaderModule(const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
      throw std::runtime_error("Failed to create shader module!");
    return shaderModule;
  }

  void createGraphicsPipeline() {
    auto vertShaderCode = readFile("../obj/hardcoded.vert.spv"); 
    auto fragShaderCode = readFile("../obj/hardcoded.frag.spv"); 

    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

    // Fully describe the programmable stages of the graphics pipeline
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT; // The stage in the graphics pipeline
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    // Fragment stage
    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    // Describes the vertex data passed into the vertex shader
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    // Get the vertex pipeline descriptions from our struct vertex buffer
    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    // Describes the geometry drawn from vertices and if "primitive restart" is allowed
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // Every 3 points makes a triangle
    inputAssembly.primitiveRestartEnable = VK_FALSE; // no idea what this is

    // Viewport specification (usually (0,0) -> (width, height))
    VkViewport viewport{};
    viewport.x = 0.;
    viewport.y = 0.;
    viewport.width = (float) swapChainExtent.width; // swapChain used for framebuffers
    viewport.height = (float) swapChainExtent.height;
    viewport.minDepth = 0.;
    viewport.maxDepth = 1.; // These have to be in [0, 1] for some reason

    // Scissor rectanlge crops input image as much as we want
    VkRect2D scissor{};
    scissor.offset = {0,0};
    scissor.extent = swapChainExtent; // Covers full image because we don't want a crop

    // Combine viewport and scissor into Viewport State
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1; // sometimes possible to use multiple
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    // Rasterizer turns geometry from vertex shader and turns it into fragments for the frament shader
    // More advanced feature selection requires turning on GPU features
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE; // we clip geometry outside of the frustrum instead of clamping
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL; // we pass area of polygon into shader
    rasterizer.lineWidth = 1.; // number of fragments
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT; // Culling back is the default in a lot of graphics
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; // counter-clockwise because flipping Y axis at some point
    rasterizer.depthBiasEnable = VK_FALSE; // Can bias depth based on angle for a shadow map
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

    // This tutorial turns multisampling OFF
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = nullptr; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional

    // Also need a depth or stencil test for depth buffers
    
    // Color blending requires two structs for some reason

    // Alpha blending
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    // how to blend colors. We specify to use the color blending attachment
    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional

    
    // Some parts of the state can be changed without recreating the whole pipeline
    // This specifies which ones
    VkDynamicState dynamicStates[] = {
      VK_DYNAMIC_STATE_VIEWPORT,
      VK_DYNAMIC_STATE_LINE_WIDTH
    };
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates = dynamicStates;

    // specifies uniform values in the shaders
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional
    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
      throw std::runtime_error("failed to create pipeline layout!");

    // The full pipeline putting everything together
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    // Relevant to shaders
    pipelineInfo.stageCount = 2; // number of shader, or "programmable" stages
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr; // Optional
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = nullptr; // Optional

    pipelineInfo.layout = pipelineLayout; // fixed-function stage
    pipelineInfo.renderPass = renderPass; // Can potentially swap in another render pass
    pipelineInfo.subpass = 0; // the index

    // Can create a pipeline based off of an existing one!
    // Need VK_PIPELINE_CREATE_DERIVATIVE_BIT set in pipelineInfo.flags
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    // Can make multiple pipelines at once.
    // Second argument is a pipeline cache that can be used to store and reuse data. Advanced technique
    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) 
        != VK_SUCCESS)
          throw std::runtime_error("Failed to create graphics pipeline!");

    vkDestroyShaderModule(device, fragShaderModule, nullptr);
    vkDestroyShaderModule(device, vertShaderModule, nullptr);
  }

  // These are not the actual framebuffers, I think they are objects describing what
  // frame buffers the swap chain will render to
  void createRenderPass() {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapChainImageFormat; // should have same image format as where we get the image
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; // no multisampling
    // make framebuffer black before drawing next frame
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    // The contents of the framebuffer go to the screen
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    // Don't care about stencil data
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    // What memory layout to use, so that we can optimize for how we're getting them maybe?
    // We are presenting from the swap chain so we use that one
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // We don't use data from existing render pass
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // We get it from swap chain

    // Every render pass can have 1 or more subpasse, each has its own description

    VkAttachmentReference colorAttachmentRef{}; // refers to the above color attachment
    colorAttachmentRef.attachment = 0; // index in attachment descriptions array
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // The final description of the color subpass
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // This is a graphics subpass, not compute
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    // subpass can also do input attachmnets, resolve attachments (multisampling), depth/stencil data,
    // and asttachments that are not used by the subpass but must be preserved for later

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment; // strange that we need attachment here too
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    // Need to specify dependencies so that subpasses transition properly between each other
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL; // implicit subpass before the render pass
    dependency.dstSubpass = 0; // the color subpass
    // We wait for the swap chain to finish reading from the image
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    // Writing on the color attachment stage needs to wait on the render pass
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
      throw std::runtime_error("failed to create render pass!");
  }

  void createFramebuffers() {
    swapChainFramebuffers.resize(swapChainImageViews.size());
    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
      // We explicitly make each frame buffer contain a single image view attachment, for color only
      VkImageView attachments[] = {
        swapChainImageViews[i]
      };

      VkFramebufferCreateInfo framebufferInfo{};
      framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
      framebufferInfo.renderPass = renderPass;
      framebufferInfo.attachmentCount = 1;
      framebufferInfo.pAttachments = attachments;
      framebufferInfo.width = swapChainExtent.width;
      framebufferInfo.height = swapChainExtent.height;
      framebufferInfo.layers = 1;

      if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)
        throw std::runtime_error("Failed to create framebuffer!");
        }
  }


  // Commands


  // Commands submitted to device queues, and have to choose which queue to use
  void createCommandPool() {
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    poolInfo.flags = 0; // Optional
    if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
      throw std::runtime_error("Failed to create command pool!");
  }

  void createCommandBuffers() {
    // There is one command for every framebuffer
    commandBuffers.resize(swapChainFramebuffers.size());

    // Allocates the command buffors for the framebuffers
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // submitted to queue but can't be referenced by others
    // can make secondary command buffers to reuse data
    allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();

    if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
      throw std::runtime_error("Failed to allocate command buffers!");

    for (size_t i = 0; i < commandBuffers.size(); i++) { 
      VkCommandBufferBeginInfo beginInfo{};
      beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
      beginInfo.flags = 0; // need to set these for special types of behavior
      beginInfo.pInheritanceInfo = nullptr;

      // every call to vkBeginCommandBuffer resets the buffer, it cannot append
      if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS)
        throw std::runtime_error("Failed to begin recording command buffer!");

      
      VkRenderPassBeginInfo renderPassInfo{};
      renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
      renderPassInfo.renderPass = renderPass;
      renderPassInfo.framebuffer = swapChainFramebuffers[i];

      // pixels outside of this area do not get rendered
      renderPassInfo.renderArea.offset = {0, 0};
      renderPassInfo.renderArea.extent = swapChainExtent;
      // Since we use LOAD_OP_CLEAR we clear to this color (black) with 100% opacity
      VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
      renderPassInfo.clearValueCount = 1;
      renderPassInfo.pClearValues = &clearColor;
      // Does the render pass
      vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
      // Binds the pipeline to the command buffer
      vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

      // Need to bind vertex buffer to command buffer :)
      VkBuffer vertexBuffers[] = {vertexBuffer};
      VkDeviceSize offsets[] = {0};
      vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);
      vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT16); // TODO 32 bit
      vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[i], 0, nullptr);
      // Draws the triangle: nvertices, num_instances, vertex offset, instance offset
      /* vkCmdDraw(commandBuffers[i], static_cast<uint32_t>(vertices.size()), 1, 0, 0); */
      // The command buffer has the vertex buffer so it knows what to draw with the indices
      vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0,0);
      vkCmdEndRenderPass(commandBuffers[i]);
      if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
        throw std::runtime_error("Failed to record command buffer!");
    }
  }

  // Drawing

  void createSyncObjects() {
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    imagesInFlight.resize(swapChainImages.size(), VK_NULL_HANDLE); // Want no fences at first

    // no real info needed for the semaphores or fences
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Instantiates the fence in the signalled state instead of blocking

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) { 
      if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
              vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
              vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
        throw std::runtime_error("failed to create synchronization objects for a frame!");
    }
  }


  // Vertex Buffer

  // returns the index of the memory type in the physical device that satisfies the
  // flags in typeFilter and the properties in properties
  uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    // query available memory types in GPU
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
    // there are memoryTypes and memoryHeaps, but this is where it gets pretty complicated
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) { 
      // memory type i must be one of the allowed ones
      bool satisfiesType = typeFilter & (1 << i); 
      // all our desired properties must be satisfied
      bool satisfiesProperties = (memProperties.memoryTypes[i].propertyFlags & properties)  == properties;
      if (satisfiesType && satisfiesProperties)
        return i;
    }
    throw std::runtime_error("Failed to find a suitable memory type");
  }

  void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage; // different types of buffers
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // we want only the graphics queue to use this buffer
    bufferInfo.flags = 0; // configures sparse buffer memory if needed

    if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
      throw std::runtime_error("Failed to create buffer!");

    // Need to allocate memory to the buffer
    // This struct contains: size, offset, usage, and flags
    // Interesting that this is the logical device... that must be why it can work with just CPU
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

    // We allow any memory type in GPU and we require
    // memory to be able to map to CPU memory and be "coherent". Don't know why yet
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);
    /* allocInfo.memoryTypeIndex = findMemoryType(0, 0); */

    if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
      throw std::runtime_error("Failed to allocate buffer memory!");
    vkBindBufferMemory(device, buffer, bufferMemory, 0); // offset must be divisible by alignment
  }

  void createVertexBuffer() {
    VkDeviceSize bufferSize = sizeof(vertices[0])*vertices.size();

    // Will want to copy from CPU to GPU staging buffer and then later copy from
    // staging buffer to actual vertex buffer, which uses unmappable but faster GPU memory
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
    // Copy vertices into buffer memory
    // Gives us a pointer to the buffer memory I think, but maybe also allocates it?
    void *data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), (size_t) bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    // Want device-local after the transfer
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);
    // NOTE: driver might not actually write to memory right away, so to guarantee that it does we
    // use host coherence and don't worry about that weird case.
    // Another solution is to explicitly flush the memory afterwards and before reading
    // Driver transfers data between CPU and GPU on the backend, which is pretty cool.

    copyBuffer(stagingBuffer, vertexBuffer, bufferSize);
    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
  }

  void createIndexBuffer() {
    VkDeviceSize bufferSize = sizeof(indices[0])*indices.size();

    // Will want to copy from CPU to GPU staging buffer and then later copy from
    // staging buffer to actual index buffer, which uses unmappable but faster GPU memory
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
    // Copy vertices into buffer memory
    // Gives us a pointer to the buffer memory I think, but maybe also allocates it?
    void *data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), (size_t) bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    // Want device-local after the transfer
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);
    // NOTE: driver might not actually write to memory right away, so to guarantee that it does we
    // use host coherence and don't worry about that weird case.
    // Another solution is to explicitly flush the memory afterwards and before reading
    // Driver transfers data between CPU and GPU on the backend, which is pretty cool.

    copyBuffer(stagingBuffer, indexBuffer, bufferSize);
    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
  }

  // need to use command buffers to transfer the vertices from staging buffer to vertex buffer
  void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool; // potentially make new pool for transient commands
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);
    
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // We only intend to use this once

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    // specifies where and how much to copy
    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
    vkEndCommandBuffer(commandBuffer);

    // execute the command buffer
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    // want to transfer immediately, without waiting for other commands to execute
    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue); // fences might be more efficient if multiple memory transfers at once

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
  }

  // Model View Projection

  void createDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0; // which binding/location uniform/descriptor
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1; // number of descriptors in array
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // vertex shader
    uboLayoutBinding.pImmutableSamplers = nullptr; // image sampling

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
      throw std::runtime_error("Failed to create descriptor set layout!");
  }

  // There is one uniform buffer for every swap chain image
  void createUniformBuffers() {
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);
    uniformBuffers.resize(swapChainImages.size());
    uniformBuffersMemory.resize(swapChainImages.size());

    for (size_t i = 0; i < swapChainImages.size(); i++) { 
      // These bits necessary for direct transfer between host and device
      createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);
    }
  }

  void updateUniformBuffer(uint32_t currentImage) {
    // somehow this is supposed to get the time at the beginning of the render???
    static auto startTime = std::chrono::high_resolution_clock::now();

    // gets the time since render started
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    UniformBufferObject ubo{};
    // Rotation of identity about the z axis of 90 degrees

    // ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.,0.,1.)); // rotation
    ubo.model = glm::mat4(1.0f); // no transformation in object space

    // eye, point, and up vector. We never have to change our up :)
    ubo.view = glm::lookAt(glm::vec3(2., 2., 2.), glm::vec3(0., 0., 0.), glm::vec3(0., 0., 1.));
    // perspective projection with 45 degree FOV, the window aspect ratio, and z = 0, 10 as the near and far planes
    ubo.proj = glm::perspective(glm::radians(45.f), swapChainExtent.width / (float) swapChainExtent.height, 0.1f, 10.f); // 10 is the max depth of view
    ubo.proj[1][1] *= -1; // glm was designed for OpenGL, and in Vulkan -1 is the top and 1 is the bottom

    void* data;
    vkMapMemory(device, uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
    vkUnmapMemory(device, uniformBuffersMemory[currentImage]);
    // push constants are more efficient but I don't think this is a problem
  }

  void createDescriptorPool() {
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = static_cast<uint32_t>(swapChainImages.size());

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = static_cast<uint32_t>(swapChainImages.size());
    poolInfo.flags = 0;

    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
      throw std::runtime_error("Failed to create descriptor pool");
  }

  void createDescriptorSets() {
    // need to copy the same descriptor set many times because want the same one
    std::vector<VkDescriptorSetLayout> layouts(swapChainImages.size(), descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChainImages.size());
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets.resize(swapChainImages.size());
    if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
      throw std::runtime_error("Failed to allocate descriptor sets!");

    // descriptors need to be configured. These are uniform buffer descriptors.
    for (size_t i = 0; i < swapChainImages.size(); i++) { 
      VkDescriptorBufferInfo bufferInfo{};
      bufferInfo.buffer = uniformBuffers[i];
      bufferInfo.offset = 0;
      bufferInfo.range = sizeof(UniformBufferObject);

      VkWriteDescriptorSet descriptorWrite{};
      descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorWrite.dstSet = descriptorSets[i];
      descriptorWrite.dstBinding = 0; // binding index for the uniform
      descriptorWrite.dstArrayElement = 0; // assumes the data is an array

      descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      descriptorWrite.descriptorCount = 1;

      descriptorWrite.pBufferInfo = &bufferInfo;
      descriptorWrite.pImageInfo = nullptr;
      descriptorWrite.pTexelBufferView = nullptr;

      // Can take copy descriptors as well, but we don't have those for now
      vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
    }
  }
};

int main() {
    HelloTriangleApplication app;

    try {
        std::vector<MoleculeStruct::MolecularDataOneFrame*> trajectory
            = MoleculeReader::readWholeTrajectory("./molecule_demo/demo");

        MoleculeReader::clearTrajectory(trajectory);

        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
