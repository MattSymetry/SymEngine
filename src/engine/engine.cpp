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

	this->width = width;
	this->height = height;
	this->window = window;

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

	instance = vkInit::make_instance("ID Tech 12", window);
	dldi = vk::DispatchLoaderDynamic(instance, vkGetInstanceProcAddr);
	VkSurfaceKHR c_style_surface;
	if (SDL_Vulkan_CreateSurface(window, instance, &c_style_surface) != SDL_TRUE) {
        std::cerr << "SDL_Vulkan_CreateSurface failed: " << SDL_GetError() << std::endl;
		vkLogging::Logger::get_logger()->print("Failed to abstract SDL surface for Vulkan.");
	}
	else {
		vkLogging::Logger::get_logger()->print(
			"Successfully abstracted SDL surface for Vulkan.");
	}
	//copy constructor converts to hpp convention
	surface = c_style_surface;
}

void Engine::make_device(Scene* scene) {
	physicalDevice = vkInit::choose_physical_device(instance);;
	device = vkInit::create_logical_device(physicalDevice, surface);
	std::array<vk::Queue, 2> queues = vkInit::get_queues(physicalDevice, device, surface);
	graphicsQueue = queues[0];
	presentQueue = queues[1];
	make_swapchain(scene);
	frameNumber = 0;
}

/**
* Make a swapchain
*/
void Engine::make_swapchain(Scene* scene) {
	vkInit::SwapChainBundle bundle = vkInit::create_swapchain(
		device, physicalDevice, surface, width, height, scene
	);
	swapchain = bundle.swapchain;
	swapchainFrames = bundle.frames;
	swapchainFormat = bundle.format;
	swapchainExtent = bundle.extent;
	maxFramesInFlight = static_cast<int>(swapchainFrames.size());
	for (vkUtil::SwapChainFrame& frame : swapchainFrames) {
		frame.logicalDevice = device;
		frame.physicalDevice = physicalDevice;
		frame.width = swapchainExtent.width;
		frame.height = swapchainExtent.height;
	}

}

/**
* The swapchain must be recreated upon resize or minimization, among other cases
*/
void Engine::recreate_swapchain(Scene* scene) {

	width = 0;
	height = 0;
	while (width == 0 || height == 0) {
        SDL_GetWindowSize(window, &width, &height);
        SDL_Delay(10);
	}

	device.waitIdle();

	cleanup_swapchain();
	make_swapchain(scene);
	make_frame_resources(scene);
	vkInit::commandBufferInputChunk commandBufferInput = { device, commandPool, swapchainFrames };
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

	frameSetLayout[pipelineType::COMPUTE] = vkInit::make_descriptor_set_layout(device, bindings);

}

void Engine::make_pipelines() {

	//Raytracing
	vkInit::ComputePipelineBuilder computePipelineBuilder(device);

	computePipelineBuilder.specify_compute_shader("/shaders/symScene.comp.spv");
	computePipelineBuilder.add_descriptor_set_layout(frameSetLayout[pipelineType::COMPUTE]);

	vkInit::ComputePipelineOutBundle computeOutput = computePipelineBuilder.build();

	pipelineLayout[pipelineType::COMPUTE] = computeOutput.layout;
	pipeline[pipelineType::COMPUTE] = computeOutput.pipeline;
	computePipelineBuilder.reset();

	vkInit::PipelineBuilder pipelineBuilder(device);

}

void Engine::finalize_setup(Scene* scene) {

	commandPool = vkInit::make_command_pool(device, physicalDevice, surface);

	vkInit::commandBufferInputChunk commandBufferInput = { device, commandPool, swapchainFrames };
	mainCommandBuffer = vkInit::make_command_buffer(commandBufferInput);
	mainFence = vkInit::make_fence(device);
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

	frameDescriptorPool[pipelineType::COMPUTE] = vkInit::make_descriptor_pool(device, static_cast<uint32_t>(swapchainFrames.size()), bindings);

	for (vkUtil::SwapChainFrame& frame : swapchainFrames) {

		frame.imageAvailable = vkInit::make_semaphore(device);
		frame.renderFinished = vkInit::make_semaphore(device);
		frame.inFlight = vkInit::make_fence(device);

		frame.make_descriptor_resources(device, physicalDevice);
		frame.descriptorSet[pipelineType::COMPUTE] = vkInit::allocate_descriptor_set(device, frameDescriptorPool[pipelineType::COMPUTE], frameSetLayout[pipelineType::COMPUTE]);
		frame.record_write_operations();
	}

}

void Engine::make_assets(Scene* scene) {

	for (int i = 0; i < maxFramesInFlight; ++i) {
		prepare_frame(i, scene);
	}
}

void Engine::prepare_frame(uint32_t imageIndex, Scene* scene) {

	vkUtil::SwapChainFrame& _frame = swapchainFrames[imageIndex];
    
    for (auto& bufferSetup : _frame.bufferSetups) {
		device.waitForFences(1, &mainFence, VK_TRUE, UINT64_MAX);
		device.resetFences(1, &mainFence);
        bufferSetup.buffer.blit((void*)bufferSetup.dataPtr, bufferSetup.dataSize, graphicsQueue, mainCommandBuffer, mainFence);
    }

	_frame.write_descriptor_set();
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

	commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline[pipelineType::COMPUTE]);

	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pipelineLayout[pipelineType::COMPUTE], 0, swapchainFrames[imageIndex].descriptorSet[pipelineType::COMPUTE], nullptr);

	commandBuffer.dispatch(static_cast<uint32_t>(swapchainExtent.width / 8), static_cast<uint32_t>(swapchainExtent.height / 8), 1);

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
	device.waitForFences(1, &(swapchainFrames[frameNumber].inFlight), VK_TRUE, UINT64_MAX);
	device.resetFences(1, &(swapchainFrames[frameNumber].inFlight));

	uint32_t imageIndex;
	try {
		vk::ResultValue acquire = device.acquireNextImageKHR(
			swapchain, UINT64_MAX,
			swapchainFrames[frameNumber].imageAvailable, nullptr
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

	vk::CommandBuffer commandBuffer = swapchainFrames[frameNumber].commandBuffer;
	commandBuffer.reset();
	vk::CommandBufferBeginInfo beginInfo = {};
	try {
		commandBuffer.begin(beginInfo);
	}
	catch (vk::SystemError err) {
		vkLogging::Logger::get_logger()->print("Failed to begin recording command buffer!");
	}
	prepare_to_trace_barrier(commandBuffer, swapchainFrames[imageIndex].image);
	dispatch_compute(commandBuffer, imageIndex);
	prepare_to_present_barrier(commandBuffer, swapchainFrames[imageIndex].image);
	try {
		commandBuffer.end();
	}
	catch (vk::SystemError err) {

		vkLogging::Logger::get_logger()->print("failed to record command buffer!");
	}
	vk::SubmitInfo submitInfo = {};
	vk::Semaphore waitSemaphores[] = { swapchainFrames[frameNumber].imageAvailable };
	vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	vk::Semaphore signalSemaphores[] = { swapchainFrames[frameNumber].renderFinished };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;
	try {
		graphicsQueue.submit(submitInfo, swapchainFrames[frameNumber].inFlight);
	}
	catch (vk::SystemError err) {
		vkLogging::Logger::get_logger()->print("failed to submit draw command buffer!");
	}

	vk::PresentInfoKHR presentInfo = {};
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	vk::SwapchainKHR swapChains[] = { swapchain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	vk::Result present;
	try {
		present = presentQueue.presentKHR(presentInfo);
	}
	catch (vk::OutOfDateKHRError error) {
		present = vk::Result::eErrorOutOfDateKHR;
	}
	if (present == vk::Result::eErrorOutOfDateKHR || present == vk::Result::eSuboptimalKHR) {
		std::cout << "Recreate" << std::endl;
		recreate_swapchain(scene);
		return;
	}
	frameNumber = (frameNumber + 1) % maxFramesInFlight;

}

/**
* Free the memory associated with the swapchain objects
*/
void Engine::cleanup_swapchain() {

	for (vkUtil::SwapChainFrame& frame : swapchainFrames) {
		frame.destroy();
	}
	device.destroySwapchainKHR(swapchain);

	device.destroyDescriptorPool(frameDescriptorPool[pipelineType::COMPUTE]);

}

Engine::~Engine() {

	device.waitIdle();

	vkLogging::Logger::get_logger()->print("Goodbye see you!");

	device.destroyFence(mainFence);

	device.destroyCommandPool(commandPool);

	for (pipelineType pipeline_type : pipelineTypes) {
		device.destroyPipeline(pipeline[pipeline_type]);
		device.destroyPipelineLayout(pipelineLayout[pipeline_type]);
	}

	cleanup_swapchain();
	for (pipelineType pipeline_type : pipelineTypes) {
		device.destroyDescriptorSetLayout(frameSetLayout[pipeline_type]);
	}

	device.destroy();

	instance.destroySurfaceKHR(surface);
	if (vkLogging::Logger::get_logger()->get_debug_mode()) {
		// TODOinstance.destroyDebugUtilsMessengerEXT(debugMessenger, nullptr, dldi);
	}
	/*
	* from vulkan_funcs.hpp:
	*
	* void Instance::destroy( Optional<const VULKAN_HPP_NAMESPACE::AllocationCallbacks> allocator = nullptr,
											Dispatch const & d = ::vk::getDispatchLoaderStatic())
	*/
	instance.destroy();
}
