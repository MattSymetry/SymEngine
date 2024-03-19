#include "engine.h"
#include "logging.h"
#include "vulkan/vkInit/instance.h"
#include "vulkan/vkInit/device.h"
#include "vulkan/vkInit/swapchain.h"
#include "vulkan/vkInit/pipeline.h"
#include "vulkan/vkInit/compute_pipeline.h"
#include "vulkan/vkInit/commands.h"
#include "vulkan/vkInit/sync.h"
#include "vulkan/vkInit/descriptors.h"

Engine::Engine(int width, int height, SDL_Window* window, Scene* scene) {

	m_width = width;
	m_height = height;
	m_window = window;

	vkLogging::Logger::get_logger()->print("Making a graphics engine...");
	//Set this to false to see framerate improvement
	vkLogging::Logger::get_logger()->set_debug_mode(false);
	make_instance();
	make_device(scene);
	make_descriptor_set_layouts(scene);
	make_pipelines();
	finalize_setup(scene);
	make_assets(scene);
}

void Engine::make_instance() {

    m_instance = vkInit::make_instance("ID Tech 12", m_window);
    m_dldi = vk::DispatchLoaderDynamic(m_instance, vkGetInstanceProcAddr);
	VkSurfaceKHR c_style_surface;
	if (SDL_Vulkan_CreateSurface(m_window, m_instance, &c_style_surface) != SDL_TRUE) {
        std::cerr << "SDL_Vulkan_CreateSurface failed: " << SDL_GetError() << std::endl;
		vkLogging::Logger::get_logger()->print("Failed to abstract SDL surface for Vulkan.");
	}
	else {
		vkLogging::Logger::get_logger()->print(
			"Successfully abstracted SDL surface for Vulkan.");
	}
	//copy constructor converts to hpp convention
    m_surface = c_style_surface;
}

void Engine::make_device(Scene* scene) {
    m_physicalDevice = vkInit::choose_physical_device(m_instance);;
    m_device = vkInit::create_logical_device(m_physicalDevice, m_surface);
	std::array<vk::Queue, 2> queues = vkInit::get_queues(m_physicalDevice, m_device, m_surface);
    m_graphicsQueue = queues[0];
    m_presentQueue = queues[1];
	make_swapchain(scene);
    m_frameNumber = 0;
}

/**
* Make a swapchain
*/
void Engine::make_swapchain(Scene* scene) {
	vkInit::SwapChainBundle bundle = vkInit::create_swapchain(
            m_device, m_physicalDevice, m_surface, m_width, m_height, scene
	);
    m_swapchain = bundle.swapchain;
    m_swapchainFrames = bundle.frames;
    m_swapchainFormat = bundle.format;
    m_swapchainExtent = bundle.extent;
    m_maxFramesInFlight = static_cast<int>(m_swapchainFrames.size());
	for (vkUtil::SwapChainFrame& frame : m_swapchainFrames) {
		frame.logicalDevice = m_device;
		frame.physicalDevice = m_physicalDevice;
		frame.width = m_swapchainExtent.width;
		frame.height = m_swapchainExtent.height;
	}

}

/**
* The swapchain must be recreated upon resize or minimization, among other cases
*/
void Engine::recreate_swapchain(Scene* scene) {

    m_width = 0;
    m_height = 0;
	while (m_width == 0 || m_height == 0) {
        SDL_GetWindowSize(m_window, &m_width, &m_height);
        SDL_Delay(10);
	}

    m_device.waitIdle();

	cleanup_swapchain();
	make_swapchain(scene);
	make_frame_resources(scene);
	vkInit::commandBufferInputChunk commandBufferInput = { m_device, m_commandPool, m_swapchainFrames };
	vkInit::make_frame_command_buffers(commandBufferInput);

}

void Engine::make_descriptor_set_layouts(Scene* scene) {

	//Binding once per frame
	vkInit::descriptorSetLayoutData bindings;
	bindings.count = scene->buffers.size()+1;

	bindings.indices.push_back(0);
	bindings.types.push_back(vk::DescriptorType::eStorageImage);
	bindings.counts.push_back(1);
	bindings.stages.push_back(vk::ShaderStageFlagBits::eCompute);

	int index = 1;
	for (BufferInitParams buff : scene->buffers) {
		bindings.indices.push_back(index);
		bindings.types.push_back(buff.descriptorType);
		bindings.counts.push_back(1);
		bindings.stages.push_back(vk::ShaderStageFlagBits::eCompute);
		index++;
	}

    m_frameSetLayout[pipelineType::COMPUTE] = vkInit::make_descriptor_set_layout(m_device, bindings);

}

void Engine::make_pipelines() {

	//Raytracing
	vkInit::ComputePipelineBuilder computePipelineBuilder(m_device);

	computePipelineBuilder.specify_compute_shader("/shaders/symScene.comp.spv");
	computePipelineBuilder.add_descriptor_set_layout(m_frameSetLayout[pipelineType::COMPUTE]);

	vkInit::ComputePipelineOutBundle computeOutput = computePipelineBuilder.build();

    m_pipelineLayout[pipelineType::COMPUTE] = computeOutput.layout;
    m_pipeline[pipelineType::COMPUTE] = computeOutput.pipeline;
	computePipelineBuilder.reset();

	vkInit::PipelineBuilder pipelineBuilder(m_device);

}

void Engine::finalize_setup(Scene* scene) {

    m_commandPool = vkInit::make_command_pool(m_device, m_physicalDevice, m_surface);

	vkInit::commandBufferInputChunk commandBufferInput = { m_device, m_commandPool, m_swapchainFrames };
    m_mainCommandBuffer = vkInit::make_command_buffer(commandBufferInput);
    m_mainFence = vkInit::make_fence(m_device);
	vkInit::make_frame_command_buffers(commandBufferInput);

	make_frame_resources(scene);

}

void Engine::make_frame_resources(Scene* scene) {

	vkInit::descriptorSetLayoutData bindings;
	bindings.count = scene->buffers.size() + 1;
	bindings.types.push_back(vk::DescriptorType::eStorageImage);
    
    for (BufferInitParams buff : scene->buffers) {
        bindings.types.push_back(buff.descriptorType);
    }

    m_frameDescriptorPool[pipelineType::COMPUTE] = vkInit::make_descriptor_pool(m_device, static_cast<uint32_t>(m_swapchainFrames.size()), bindings);

	for (vkUtil::SwapChainFrame& frame : m_swapchainFrames) {

		frame.imageAvailable = vkInit::make_semaphore(m_device);
		frame.renderFinished = vkInit::make_semaphore(m_device);
		frame.inFlight = vkInit::make_fence(m_device);

		frame.make_descriptor_resources(m_device, m_physicalDevice);
		frame.descriptorSet[pipelineType::COMPUTE] = vkInit::allocate_descriptor_set(m_device, m_frameDescriptorPool[pipelineType::COMPUTE], m_frameSetLayout[pipelineType::COMPUTE]);
		frame.record_write_operations();
	}

}

void Engine::make_assets(Scene* scene) {

	for (int i = 0; i < m_maxFramesInFlight; ++i) {
		prepare_frame(i, scene);
	}
}

void Engine::prepare_frame(uint32_t imageIndex, Scene* scene) {

	vkUtil::SwapChainFrame& frame = m_swapchainFrames[imageIndex];
    
    for (auto& bufferSetup : frame.bufferSetups) {
        m_device.waitForFences(1, &m_mainFence, VK_TRUE, UINT64_MAX);
        m_device.resetFences(1, &m_mainFence);
        bufferSetup.buffer.blit((void*)bufferSetup.dataPtr, bufferSetup.dataSize, m_graphicsQueue, m_mainCommandBuffer, m_mainFence);
    }

	frame.write_descriptor_set();
}

void Engine::prepare_scene(vk::CommandBuffer commandBuffer) {

	//Bind any global info.

}

void Engine::prepare_to_trace_barrier(vk::CommandBuffer commandBuffer, vk::Image image) {

	/*
	typedef struct VkImageSubresourceRange {
		VkImageAspectFlags    aspectMask;
		uint32_t              baseMipLevel;
		uint32_t              levelCount;
		uint32_t              baseArrayLayer;
		uint32_t              layerCount;
	} VkImageSubresourceRange;
	*/
	vk::ImageSubresourceRange access;
	access.aspectMask = vk::ImageAspectFlagBits::eColor;
	access.baseMipLevel = 0;
	access.levelCount = 1;
	access.baseArrayLayer = 0;
	access.layerCount = 1;

	/*
	typedef struct VkImageMemoryBarrier {
		VkStructureType            sType;
		const void* pNext;
		VkAccessFlags              srcAccessMask;
		VkAccessFlags              dstAccessMask;
		VkImageLayout              oldLayout;
		VkImageLayout              newLayout;
		uint32_t                   srcQueueFamilyIndex;
		uint32_t                   dstQueueFamilyIndex;
		VkImage                    image;
		VkImageSubresourceRange    subresourceRange;
	} VkImageMemoryBarrier;
	*/
	vk::ImageMemoryBarrier barrier;
	barrier.oldLayout = vk::ImageLayout::eUndefined;
	barrier.newLayout = vk::ImageLayout::eGeneral;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange = access;

	vk::PipelineStageFlags sourceStage, destinationStage;

	barrier.srcAccessMask = vk::AccessFlagBits::eNoneKHR;
	sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;

	barrier.dstAccessMask = vk::AccessFlagBits::eMemoryWrite;
	destinationStage = vk::PipelineStageFlagBits::eComputeShader;

	commandBuffer.pipelineBarrier(sourceStage, destinationStage, vk::DependencyFlags(), nullptr, nullptr, barrier);
}

void Engine::dispatch_compute(vk::CommandBuffer commandBuffer, uint32_t imageIndex) {

	commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, m_pipeline[pipelineType::COMPUTE]);

	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_pipelineLayout[pipelineType::COMPUTE], 0, m_swapchainFrames[imageIndex].descriptorSet[pipelineType::COMPUTE], nullptr);

	commandBuffer.dispatch(static_cast<uint32_t>(m_swapchainExtent.width / 8), static_cast<uint32_t>(m_swapchainExtent.height / 8), 1);

}

void Engine::prepare_to_present_barrier(vk::CommandBuffer commandBuffer, vk::Image image) {

	/*
	typedef struct VkImageSubresourceRange {
		VkImageAspectFlags    aspectMask;
		uint32_t              baseMipLevel;
		uint32_t              levelCount;
		uint32_t              baseArrayLayer;
		uint32_t              layerCount;
	} VkImageSubresourceRange;
	*/
	vk::ImageSubresourceRange access;
	access.aspectMask = vk::ImageAspectFlagBits::eColor;
	access.baseMipLevel = 0;
	access.levelCount = 1;
	access.baseArrayLayer = 0;
	access.layerCount = 1;

	/*
	typedef struct VkImageMemoryBarrier {
		VkStructureType            sType;
		const void* pNext;
		VkAccessFlags              srcAccessMask;
		VkAccessFlags              dstAccessMask;
		VkImageLayout              oldLayout;
		VkImageLayout              newLayout;
		uint32_t                   srcQueueFamilyIndex;
		uint32_t                   dstQueueFamilyIndex;
		VkImage                    image;
		VkImageSubresourceRange    subresourceRange;
	} VkImageMemoryBarrier;
	*/
	vk::ImageMemoryBarrier barrier;
	barrier.oldLayout = vk::ImageLayout::eGeneral;
	barrier.newLayout = vk::ImageLayout::ePresentSrcKHR;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange = access;

	vk::PipelineStageFlags sourceStage, destinationStage;

	barrier.srcAccessMask = vk::AccessFlagBits::eMemoryWrite;
	sourceStage = vk::PipelineStageFlagBits::eComputeShader;

	barrier.dstAccessMask = vk::AccessFlagBits::eNoneKHR;
	destinationStage = vk::PipelineStageFlagBits::eBottomOfPipe;

	commandBuffer.pipelineBarrier(sourceStage, destinationStage, vk::DependencyFlags(), nullptr, nullptr, barrier);
}

void Engine::render(Scene* scene) {
    m_device.waitForFences(1, &(m_swapchainFrames[m_frameNumber].inFlight), VK_TRUE, UINT64_MAX);
    m_device.resetFences(1, &(m_swapchainFrames[m_frameNumber].inFlight));

	uint32_t imageIndex;
	try {
		vk::ResultValue acquire = m_device.acquireNextImageKHR(
            m_swapchain, UINT64_MAX,
            m_swapchainFrames[m_frameNumber].imageAvailable, nullptr
		);
		imageIndex = acquire.value;
	}
	catch (vk::OutOfDateKHRError error) {
		std::cout << "Recreate" << std::endl;
		recreate_swapchain(scene);
		return;
	}
	catch (vk::IncompatibleDisplayKHRError error) {
		std::cout << "Recreate" << std::endl;
		recreate_swapchain(scene);
		return;
	}
	catch (vk::SystemError error) {
		std::cout << "Failed to acquire swapchain image!" << std::endl;
	}
	
	prepare_frame(imageIndex, scene);

	vk::CommandBuffer commandBuffer = m_swapchainFrames[m_frameNumber].commandBuffer;
	commandBuffer.reset();
	vk::CommandBufferBeginInfo beginInfo = {};
	try {
		commandBuffer.begin(beginInfo);
	}
	catch (vk::SystemError err) {
		vkLogging::Logger::get_logger()->print("Failed to begin recording command buffer!");
	}
	prepare_to_trace_barrier(commandBuffer, m_swapchainFrames[imageIndex].image);
	dispatch_compute(commandBuffer, imageIndex);
	prepare_to_present_barrier(commandBuffer, m_swapchainFrames[imageIndex].image);
	try {
		commandBuffer.end();
	}
	catch (vk::SystemError err) {

		vkLogging::Logger::get_logger()->print("failed to record command buffer!");
	}
	vk::SubmitInfo submitInfo = {};
	vk::Semaphore waitSemaphores[] = { m_swapchainFrames[m_frameNumber].imageAvailable };
	vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	vk::Semaphore signalSemaphores[] = { m_swapchainFrames[m_frameNumber].renderFinished };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;
	try {
        m_graphicsQueue.submit(submitInfo, m_swapchainFrames[m_frameNumber].inFlight);
	}
	catch (vk::SystemError err) {
		vkLogging::Logger::get_logger()->print("failed to submit draw command buffer!");
	}

	vk::PresentInfoKHR presentInfo = {};
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	vk::SwapchainKHR swapChains[] = { m_swapchain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	vk::Result present;
	try {
		present = m_presentQueue.presentKHR(presentInfo);
	}
	catch (vk::OutOfDateKHRError error) {
		present = vk::Result::eErrorOutOfDateKHR;
	}
	if (present == vk::Result::eErrorOutOfDateKHR || present == vk::Result::eSuboptimalKHR) {
		std::cout << "Recreate" << std::endl;
		recreate_swapchain(scene);
		return;
	}
    m_frameNumber = (m_frameNumber + 1) % m_maxFramesInFlight;

}

/**
* Free the memory associated with the swapchain objects
*/
void Engine::cleanup_swapchain() {

	for (vkUtil::SwapChainFrame& frame : m_swapchainFrames) {
		frame.destroy();
	}
    m_device.destroySwapchainKHR(m_swapchain);

    m_device.destroyDescriptorPool(m_frameDescriptorPool[pipelineType::COMPUTE]);

}

Engine::~Engine() {

    m_device.waitIdle();

	vkLogging::Logger::get_logger()->print("Goodbye see you!");

    m_device.destroyFence(m_mainFence);

    m_device.destroyCommandPool(m_commandPool);

	for (pipelineType pipeline_type : m_pipelineTypes) {
        m_device.destroyPipeline(m_pipeline[pipeline_type]);
        m_device.destroyPipelineLayout(m_pipelineLayout[pipeline_type]);
	}

	cleanup_swapchain();
	for (pipelineType pipeline_type : m_pipelineTypes) {
        m_device.destroyDescriptorSetLayout(m_frameSetLayout[pipeline_type]);
	}

    m_device.destroy();

    m_instance.destroySurfaceKHR(m_surface);
	if (vkLogging::Logger::get_logger()->get_debug_mode()) {
		// TODOinstance.destroyDebugUtilsMessengerEXT(debugMessenger, nullptr, dldi);
	}
	/*
	* from vulkan_funcs.hpp:
	*
	* void Instance::destroy( Optional<const VULKAN_HPP_NAMESPACE::AllocationCallbacks> allocator = nullptr,
											Dispatch const & d = ::vk::getDispatchLoaderStatic())
	*/
    m_instance.destroy();
}
