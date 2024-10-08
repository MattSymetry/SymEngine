#pragma once
#include "engine.h"
#include "logging.h"
#include "vulkan/vkInit/instance.h"
#include "vulkan/vkInit/device.h"
#include "vulkan/vkInit/swapchain.h"
#include "vulkan/vkInit/pipeline.h"
#include "vulkan/vkInit/commands.h"
#include "vulkan/vkInit/sync.h"
#include "vulkan/vkInit/descriptors.h"
#include "glslang/Public/ShaderLang.h"
#include "vulkan/vkImage/lodepng.h"
#include "tinyfiledialogs.h"



Engine::Engine(int width, int height, SDL_Window* window, Scene* scene) {

	m_width = width;
	m_height = height;
	m_window = window;
	m_scene = scene;

	vkLogging::Logger::get_logger()->print("Making a graphics engine...");
	vkLogging::Logger::get_logger()->set_debug_mode(false);
	make_instance();
	make_device(scene);
	glslang::InitializeProcess();
	make_descriptor_set_layouts(scene);
	make_pipelines();
	finalize_setup(scene);
	make_assets(scene);
    init_imgui();
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

void Engine::recreate_swapchain(Scene* scene) {
    std::cout << "Recreating swapchain" << std::endl;
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
	vkInit::ComputePipelineBuilder computePipelineBuilder(m_device);
	m_computePipelineBuilder = computePipelineBuilder;

	m_computePipelineBuilder.specify_compute_shader(m_scene->getShaderCode().c_str());
	m_computePipelineBuilder.add_descriptor_set_layout(m_frameSetLayout[pipelineType::COMPUTE]);

	vkInit::ComputePipelineOutBundle computeOutput = m_computePipelineBuilder.build();

	m_pipelineLayout[pipelineType::COMPUTE] = computeOutput.layout;
	m_pipeline[pipelineType::COMPUTE] = computeOutput.pipeline;
	m_computePipelineBuilder.reset();

	m_computePipelineBuilder.specify_compute_shader(m_scene->getShaderCode().c_str());
	m_computePipelineBuilder.add_descriptor_set_layout(m_frameSetLayout[pipelineType::COMPUTE]);

	vkInit::ComputePipelineOutBundle computeOutputSecond = m_computePipelineBuilder.build();

	m_pipelineLayout[pipelineType::COMPUTE2] = computeOutputSecond.layout;
	m_pipeline[pipelineType::COMPUTE2] = computeOutputSecond.pipeline;
	m_computePipelineBuilder.reset();

	vkInit::PipelineBuilder pipelineBuilder(m_device);
}

void Engine::finalize_setup(Scene* scene) {

	m_commandPool = vkInit::make_command_pool(m_device, m_physicalDevice, m_surface);
    m_immCommandPool = vkInit::make_command_pool(m_device, m_physicalDevice, m_surface);

	vkInit::commandBufferInputChunk commandBufferInput = { m_device, m_commandPool, m_swapchainFrames };
    vkInit::commandBufferInputChunk immCommandBufferInput = { m_device, m_immCommandPool, m_swapchainFrames };
	m_mainCommandBuffer = vkInit::make_command_buffer(commandBufferInput);
    m_immCommandBuffer = vkInit::make_command_buffer(immCommandBufferInput);
    
	m_mainFence = vkInit::make_fence(m_device);
    m_immFence = vkInit::make_fence(m_device);
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

void Engine::SetupImGuiFonts(ImGuiIO& io) {
	// Load icon font from memory
	std::vector<char> iconFontData = LoadEmbeddedFontResource(IDR_FONT2);
	static const ImWchar icons_ranges[] = { ICON_MIN_LC, ICON_MAX_LC, 0 };
	ImFontConfig icons_config;
	icons_config.MergeMode = true;
	icons_config.GlyphOffset = ImVec2(0, 3);
	//icons_config.PixelSnapH = true;

	// Load main font from memory
	std::vector<char> fontData = LoadEmbeddedFontResource(IDR_FONT1);
	ImFont* myFont = io.Fonts->AddFontFromMemoryTTF(fontData.data(), fontData.size(), 16.0f);
	io.Fonts->AddFontFromMemoryTTF(iconFontData.data(), iconFontData.size(), 16.0f, &icons_config, icons_ranges);
	ImFont* myFontSmall = io.Fonts->AddFontFromMemoryTTF(fontData.data(), fontData.size(), 14.0f);
	io.Fonts->AddFontFromMemoryTTF(iconFontData.data(), iconFontData.size(), 14.0f, &icons_config, icons_ranges);
	io.Fonts->Build();
}

void Engine::createHighResImage(uint32_t width, uint32_t height) {
	vk::ImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.imageType = vk::ImageType::e2D;
	imageCreateInfo.extent.width = width;
	imageCreateInfo.extent.height = height;
	imageCreateInfo.extent.depth = 1;
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.format = vk::Format::eR8G8B8A8Unorm;
	imageCreateInfo.tiling = vk::ImageTiling::eOptimal;
	imageCreateInfo.initialLayout = vk::ImageLayout::eUndefined;
	imageCreateInfo.usage = vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferSrc;
	imageCreateInfo.samples = vk::SampleCountFlagBits::e1;
	imageCreateInfo.sharingMode = vk::SharingMode::eExclusive;

	m_highResImage = m_device.createImage(imageCreateInfo);

	vk::MemoryRequirements memRequirements = m_device.getImageMemoryRequirements(m_highResImage);
	vk::MemoryAllocateInfo allocInfo = {};
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = vkUtil::findMemoryTypeIndex(m_physicalDevice, memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);

	m_highResImageMemory = m_device.allocateMemory(allocInfo);
	m_device.bindImageMemory(m_highResImage, m_highResImageMemory, 0);

	vk::ImageViewCreateInfo viewCreateInfo = {};
	viewCreateInfo.image = m_highResImage;
	viewCreateInfo.viewType = vk::ImageViewType::e2D;
	viewCreateInfo.format = vk::Format::eR8G8B8A8Unorm;
	viewCreateInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	viewCreateInfo.subresourceRange.baseMipLevel = 0;
	viewCreateInfo.subresourceRange.levelCount = 1;
	viewCreateInfo.subresourceRange.baseArrayLayer = 0;
	viewCreateInfo.subresourceRange.layerCount = 1;

	m_highResImageView = m_device.createImageView(viewCreateInfo);
}

void Engine::createHgihResComputePipeline(vk::ShaderModule computeShaderModule, Scene* scene) {
	// Descriptor Layout
	vkInit::descriptorSetLayoutData bindings;
	bindings.count = scene->buffers.size() + 1;

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

	m_HighResDescriptorSetLayout = vkInit::make_descriptor_set_layout(m_device, bindings);

	vk::PipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &m_HighResDescriptorSetLayout;

	m_HighResPipelineLayout = m_device.createPipelineLayout(pipelineLayoutInfo);

	vk::ComputePipelineCreateInfo pipelineInfo = {};
	pipelineInfo.stage = vk::PipelineShaderStageCreateInfo()
		.setStage(vk::ShaderStageFlagBits::eCompute)
		.setModule(computeShaderModule)
		.setPName("main");
	pipelineInfo.layout = m_HighResPipelineLayout;

	m_HighResComputePipeline = m_device.createComputePipeline(nullptr, pipelineInfo).value;
}

vk::CommandBuffer Engine::allocateHighResCommandBuffer(vk::CommandPool commandPool) {
	vk::CommandBufferAllocateInfo allocInfo = {};
	allocInfo.level = vk::CommandBufferLevel::ePrimary;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	vk::CommandBuffer commandBuffer;
	m_device.allocateCommandBuffers(&allocInfo, &commandBuffer);
	return commandBuffer;
}

void Engine::dispatchHighResCompute(vk::CommandPool commandPool, vk::Queue computeQueue, vk::DescriptorSet descriptorSet, uint32_t width, uint32_t height) {
	vk::CommandBuffer commandBuffer = allocateHighResCommandBuffer(commandPool);

	vk::CommandBufferBeginInfo beginInfo = {};
	commandBuffer.begin(beginInfo);

	commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, m_HighResComputePipeline);
	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_HighResPipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

	commandBuffer.dispatch((width + 7) / 8, (height +7) / 8, 1);

	commandBuffer.end();

	vk::SubmitInfo submitInfo = {};
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vk::Fence fence = m_device.createFence({});
	computeQueue.submit(1, &submitInfo, fence);
	m_device.waitForFences(1, &fence, VK_TRUE, UINT64_MAX);

	m_device.destroyFence(fence);
	m_device.freeCommandBuffers(commandPool, 1, &commandBuffer);
}

void Engine::createReadBackBuffer(vk::DeviceSize size) {
	vk::BufferCreateInfo bufferInfo = {};
	bufferInfo.size = size;
	bufferInfo.usage = vk::BufferUsageFlagBits::eTransferDst;
	bufferInfo.sharingMode = vk::SharingMode::eExclusive;

	m_readBackBuffer = m_device.createBuffer(bufferInfo);

	vk::MemoryRequirements memRequirements = m_device.getBufferMemoryRequirements(m_readBackBuffer);

	vk::MemoryAllocateInfo allocInfo = {};
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = vkUtil::findMemoryTypeIndex(m_physicalDevice, memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

	m_readBackBufferMemory = m_device.allocateMemory(allocInfo);
	m_device.bindBufferMemory(m_readBackBuffer, m_readBackBufferMemory, 0);
}

void Engine::readBackHighResImage(vk::CommandPool commandPool, vk::Queue graphicsQueue, vk::Buffer readBackBuffer, uint32_t width, uint32_t height) {
	vk::CommandBuffer commandBuffer = allocateHighResCommandBuffer(commandPool);

	vk::CommandBufferBeginInfo beginInfo = {};
	commandBuffer.begin(beginInfo);

	vk::ImageMemoryBarrier barrier = {};
	barrier.oldLayout = vk::ImageLayout::eGeneral;
	barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
	barrier.srcAccessMask = vk::AccessFlagBits::eShaderWrite;
	barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
	barrier.image = m_highResImage;
	barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	commandBuffer.pipelineBarrier(
		vk::PipelineStageFlagBits::eComputeShader,
		vk::PipelineStageFlagBits::eTransfer,
		vk::DependencyFlags(),
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	vk::BufferImageCopy copyRegion = {};
	copyRegion.bufferOffset = 0;
	copyRegion.bufferRowLength = 0;
	copyRegion.bufferImageHeight = 0;
	copyRegion.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
	copyRegion.imageSubresource.mipLevel = 0;
	copyRegion.imageSubresource.baseArrayLayer = 0;
	copyRegion.imageSubresource.layerCount = 1;
	copyRegion.imageOffset = vk::Offset3D(0, 0, 0);
	copyRegion.imageExtent = vk::Extent3D( width, height, 1 );

	commandBuffer.copyImageToBuffer(m_highResImage, vk::ImageLayout::eTransferSrcOptimal, readBackBuffer, 1, &copyRegion);

	vk::MemoryBarrier memoryBarrier = {};
	memoryBarrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
	memoryBarrier.dstAccessMask = vk::AccessFlagBits::eHostRead;

	commandBuffer.pipelineBarrier(
		vk::PipelineStageFlagBits::eTransfer,
		vk::PipelineStageFlagBits::eHost,
		vk::DependencyFlags(),
		1, &memoryBarrier,
		0, nullptr,
		0, nullptr
	);

	commandBuffer.end();

	vk::SubmitInfo submitInfo = {};
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vk::Fence fence = m_device.createFence({});
	graphicsQueue.submit(1, &submitInfo, fence);
	m_device.waitForFences(1, &fence, VK_TRUE, UINT64_MAX);

	m_device.destroyFence(fence);
	m_device.freeCommandBuffers(commandPool, 1, &commandBuffer);
}

void Engine::saveImageAsPNG(const std::string& filename, const std::vector<uint8_t>& imageData, uint32_t width, uint32_t height) {
	std::ofstream file(filename, std::ios::binary);
	if (!file.is_open()) {
		std::cout << "Error: Unable to open file for writing: " << filename << std::endl;
		setPopupText("Error: Unable to open file for writing: " + filename, popupStates::PERROR);
		return;
	}
	file.close();
	
	unsigned error = lodepng::encode(filename, imageData, width, height);
	if (error) {
		setPopupText("Error encoding PNG: " + std::string(lodepng_error_text(error)), popupStates::PERROR);
	}
	else {
		setPopupText("Image saved!" , popupStates::PSUCCESS );
	}
}

void Engine::setPopupText(std::string text, popupStates state) {
	popupText = text;
	popupState = state;
	showPopup = true;
}

void Engine::renderHighResImage(Scene* scene, uint32_t width, uint32_t height) {
	const char* filters[] = { "*.png" };
	std::string name = scene->getFilename();
	name = name.substr(0, name.find_last_of("."));
	if (name.empty()) {
		name = "SymysRender";
	}
	const char* saveFileName = tinyfd_saveFileDialog(
		"Save High-Resolution Image",
		(name + ".png").c_str(),
		1,
		filters,
		nullptr
	);

	if (!saveFileName) {
		std::cout << "Save operation cancelled by user." << std::endl;
		return;
	}

	// check for .png extension
	std::string saveFileNameStr(saveFileName);
	if (saveFileName) {
		if (saveFileNameStr.find(".png") == std::string::npos) {
			saveFileNameStr += ".png";
			saveFileName = saveFileNameStr.c_str();
		}
	}

	createHighResImage(width, height);
	std::string shaderCode = scene->getShaderCode();
	createHgihResComputePipeline(vkUtil::createModule(shaderCode, m_device, true), scene);
	createReadBackBuffer(width * height * 4); // Assuming 4 bytes per pixel (R8G8B8A8)

	// Descriptor Set
	vkInit::descriptorSetLayoutData bindings2;
	bindings2.count = scene->buffers.size() + 1;
	bindings2.types.push_back(vk::DescriptorType::eStorageImage);

	for (BufferInitParams buff : scene->buffers) {
		bindings2.types.push_back(buff.descriptorType);
	}

	vk::DescriptorPool descPool = vkInit::make_descriptor_pool(m_device, static_cast<uint32_t>(m_swapchainFrames.size()), bindings2);

	vk::DescriptorSet descriptorSet = vkInit::allocate_descriptor_set(m_device, descPool, m_HighResDescriptorSetLayout);

	vk::DescriptorImageInfo imageInfo = {};
	imageInfo.imageView = m_highResImageView;
	imageInfo.imageLayout = vk::ImageLayout::eGeneral;

	std::vector<vk::WriteDescriptorSet> writeOps;

	vk::WriteDescriptorSet descriptorWrite = {};
	descriptorWrite.dstSet = descriptorSet;
	descriptorWrite.dstBinding = 0;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = vk::DescriptorType::eStorageImage;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pImageInfo = &imageInfo;
	writeOps.push_back(descriptorWrite);

	for (auto& bufferSetup : m_swapchainFrames[0].bufferSetups) {
		vk::WriteDescriptorSet bufferOp;
		bufferOp.dstSet = descriptorSet;
		bufferOp.dstBinding = bufferSetup.dstBinding;
		bufferOp.dstArrayElement = 0; //byte offset within binding for inline uniform blocks
		bufferOp.descriptorCount = 1;
		bufferOp.descriptorType = bufferSetup.descriptorType;
		bufferOp.pBufferInfo = &bufferSetup.buffer.descriptor;
		writeOps.push_back(bufferOp);
	}

	m_device.updateDescriptorSets(writeOps, nullptr);

	// Dispatch compute shader
	dispatchHighResCompute(m_commandPool, m_graphicsQueue, descriptorSet, width, height);

	// Read back image data
	readBackHighResImage(m_commandPool, m_graphicsQueue, m_readBackBuffer, width, height);

	// Copy image data to host
	std::vector<uint8_t> highResImageData(width * height * 4); // Assuming 4 bytes per pixel (R8G8B8A8)
	void* mappedMemory = m_device.mapMemory(m_readBackBufferMemory, 0, highResImageData.size(), vk::MemoryMapFlags());
	memcpy(highResImageData.data(), mappedMemory, highResImageData.size());
	m_device.unmapMemory(m_readBackBufferMemory);

	saveImageAsPNG(saveFileNameStr, highResImageData, width, height);

	// Clean up
	m_device.destroyImageView(m_highResImageView);
	m_device.destroyImage(m_highResImage);
	m_device.freeMemory(m_highResImageMemory);
	m_device.destroyBuffer(m_readBackBuffer);
	m_device.freeMemory(m_readBackBufferMemory);
}

void Engine::init_imgui()
{
    VkDescriptorPoolSize pool_sizes[] = { { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 } };

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000;
    pool_info.poolSizeCount = (uint32_t)std::size(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;
    
    m_imguiPool = m_device.createDescriptorPool(pool_info);

    ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.IniFilename = NULL;
	
	SetupImGuiFonts(io);

    ImGui_ImplSDL2_InitForVulkan(m_window);
	VkPipelineRenderingCreateInfoKHR pipelineRenderingCreateInfo = {};
	VkFormat form = VkFormat::VK_FORMAT_B8G8R8A8_UNORM;
	pipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
	pipelineRenderingCreateInfo.colorAttachmentCount = 1;
	pipelineRenderingCreateInfo.pColorAttachmentFormats = &form;


    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = m_instance;
    init_info.PhysicalDevice = m_physicalDevice;
    init_info.Device = m_device;
    init_info.Queue = m_graphicsQueue;
    init_info.DescriptorPool = m_imguiPool;
    init_info.MinImageCount = 3;
    init_info.ImageCount = 3;
    init_info.UseDynamicRendering = true;
	init_info.PipelineRenderingCreateInfo = pipelineRenderingCreateInfo;
    //init_info.ColorAttachmentFormat = VkFormat::VK_FORMAT_B8G8R8A8_UNORM;

    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&init_info);

    immediate_submit([&](VkCommandBuffer cmd) { ImGui_ImplVulkan_CreateFontsTexture(); });

    //ImGui_ImplVulkan_DestroyFontUploadObjects();
}

vk::RenderingAttachmentInfoKHR Engine::attachment_info(
    vk::ImageView view, vk::ClearValue* clear, vk::ImageLayout layout = vk::ImageLayout::eColorAttachmentOptimal)
{
    vk::RenderingAttachmentInfoKHR colorAttachment{};
    colorAttachment.sType = vk::StructureType::eRenderingAttachmentInfoKHR;
    colorAttachment.pNext = nullptr;
    
    colorAttachment.imageView = view;
    colorAttachment.imageLayout = layout;
    colorAttachment.loadOp = clear ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eLoad;
    colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
    if (clear) {
        colorAttachment.clearValue = *clear;
    }

    return colorAttachment;
}

vk::RenderingInfoKHR Engine::rendering_info(
    vk::Extent2D renderExtent, vk::RenderingAttachmentInfoKHR* colorAttachment,
    vk::RenderingAttachmentInfoKHR* depthAttachment)
{
    vk::RenderingInfoKHR renderInfo{};
    renderInfo.sType = vk::StructureType::eRenderingInfoKHR;
    renderInfo.pNext = nullptr;

    renderInfo.renderArea = vk::Rect2D{ vk::Offset2D{0, 0}, renderExtent };
    renderInfo.layerCount = 1;
    renderInfo.colorAttachmentCount = 1;
    renderInfo.pColorAttachments = colorAttachment;
    renderInfo.pDepthAttachment = depthAttachment;
    renderInfo.pStencilAttachment = nullptr;

    return renderInfo;
}

void Engine::draw_imgui(vk::CommandBuffer cmd, vk::ImageView targetImageView)
{
    vk::RenderingAttachmentInfoKHR colorAttachment = attachment_info(targetImageView, nullptr, vk::ImageLayout::eColorAttachmentOptimal);

    vk::RenderingInfoKHR renderInfo = rendering_info(m_swapchainExtent, &colorAttachment, nullptr);

    cmd.beginRendering(renderInfo);
    
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

    cmd.endRendering();
}

void Engine::prepare_frame(uint32_t imageIndex, Scene* scene) {

	if (scene->needsRecompilation) return;

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

	vk::ImageSubresourceRange access;
	access.aspectMask = vk::ImageAspectFlagBits::eColor;
	access.baseMipLevel = 0;
	access.levelCount = 1;
	access.baseArrayLayer = 0;
	access.layerCount = 1;

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

void Engine::recompile_shader()
{
	if (m_pipelineNumber == 0) {
		m_computePipelineBuilder.specify_compute_shader(m_scene->getShaderCode().c_str());
		m_computePipelineBuilder.add_descriptor_set_layout(m_frameSetLayout[pipelineType::COMPUTE]);

		vkInit::ComputePipelineOutBundle computeOutputSecond = m_computePipelineBuilder.build();

		m_pipelineLayout[pipelineType::COMPUTE2] = computeOutputSecond.layout;
		m_pipeline[pipelineType::COMPUTE2] = computeOutputSecond.pipeline;
		m_computePipelineBuilder.reset();
		m_pipelineNumber = 1;
	}
	else {
		m_computePipelineBuilder.specify_compute_shader(m_scene->getShaderCode().c_str());
		m_computePipelineBuilder.add_descriptor_set_layout(m_frameSetLayout[pipelineType::COMPUTE]);

		vkInit::ComputePipelineOutBundle computeOutput = m_computePipelineBuilder.build();

		m_pipelineLayout[pipelineType::COMPUTE] = computeOutput.layout;
		m_pipeline[pipelineType::COMPUTE] = computeOutput.pipeline;
		m_computePipelineBuilder.reset();
		m_pipelineNumber = 0;
	}
	m_scene->needsRecompilation = false;
}

void Engine::dispatch_compute(vk::CommandBuffer commandBuffer, uint32_t imageIndex, glm::vec4 viewport) {

	if (m_pipelineNumber == 0) {
		commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, m_pipeline[pipelineType::COMPUTE]);
		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_pipelineLayout[pipelineType::COMPUTE], 0, m_swapchainFrames[imageIndex].descriptorSet[pipelineType::COMPUTE], nullptr);
	}
	else {
		commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, m_pipeline[pipelineType::COMPUTE2]);
		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_pipelineLayout[pipelineType::COMPUTE2], 0, m_swapchainFrames[imageIndex].descriptorSet[pipelineType::COMPUTE], nullptr);
	}


	commandBuffer.dispatch(static_cast<uint32_t>((viewport.z + 7) / 8), static_cast<uint32_t>((viewport.w + 7) / 8), 1);

}

void Engine::prepare_to_present_barrier(vk::CommandBuffer commandBuffer, vk::Image image) {

	vk::ImageSubresourceRange access;
	access.aspectMask = vk::ImageAspectFlagBits::eColor;
	access.baseMipLevel = 0;
	access.levelCount = 1;
	access.baseArrayLayer = 0;
	access.layerCount = 1;

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
	dispatch_compute(commandBuffer, imageIndex, scene->m_viewport);
	vk::ImageMemoryBarrier barrierToRendering = {};
	barrierToRendering.oldLayout = vk::ImageLayout::eGeneral;
	barrierToRendering.newLayout = vk::ImageLayout::eColorAttachmentOptimal;
	barrierToRendering.srcAccessMask = vk::AccessFlagBits::eShaderWrite;
	barrierToRendering.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
	barrierToRendering.image = m_swapchainFrames[imageIndex].image;
	barrierToRendering.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	barrierToRendering.subresourceRange.baseMipLevel = 0;
	barrierToRendering.subresourceRange.levelCount = 1;
	barrierToRendering.subresourceRange.baseArrayLayer = 0;
	barrierToRendering.subresourceRange.layerCount = 1;

	commandBuffer.pipelineBarrier(
		vk::PipelineStageFlagBits::eComputeShader,
		vk::PipelineStageFlagBits::eColorAttachmentOutput,
		vk::DependencyFlags(),
		0, nullptr,
		0, nullptr,
		1, &barrierToRendering
	);
	draw_imgui(commandBuffer, m_swapchainFrames[imageIndex].imageView);

	vk::ImageMemoryBarrier barrierToPresent = {};
	barrierToPresent.oldLayout = vk::ImageLayout::eColorAttachmentOptimal;
	barrierToPresent.newLayout = vk::ImageLayout::ePresentSrcKHR;
	barrierToPresent.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
	barrierToPresent.dstAccessMask = vk::AccessFlagBits::eMemoryRead;
	barrierToPresent.image = m_swapchainFrames[imageIndex].image;
	barrierToPresent.subresourceRange = barrierToRendering.subresourceRange;

	commandBuffer.pipelineBarrier(
		vk::PipelineStageFlagBits::eColorAttachmentOutput,
		vk::PipelineStageFlagBits::eBottomOfPipe,
		vk::DependencyFlags(),
		0, nullptr,
		0, nullptr,
		1, &barrierToPresent
	);

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

	// readback last buffer of frame
	Buffer* readBackBuffer = &m_swapchainFrames[m_frameNumber].bufferSetups[m_swapchainFrames[m_frameNumber].bufferSetups.size() - 1].buffer;
	void* tempPtr = nullptr;
	vkMapMemory(m_device, readBackBuffer->deviceMemory, 0, 4, 0, &tempPtr);
	if (tempPtr != nullptr) {
		int* selectedIdPtr = (int*)tempPtr;
		int selectedIdValue = *selectedIdPtr;
		scene->hoverId = selectedIdValue;
	}
	vkUnmapMemory(m_device, readBackBuffer->deviceMemory);

	m_frameNumber = (m_frameNumber + 1) % m_maxFramesInFlight;

}

void Engine::immediate_submit(std::function<void(vk::CommandBuffer cmd)>&& function)
{
	m_device.resetFences(1, &m_immFence);
	m_immCommandBuffer.reset();

	vk::CommandBuffer cmd = m_immCommandBuffer;

	vk::CommandBufferBeginInfo cmdBeginInfo = {};
	cmdBeginInfo.sType = vk::StructureType::eCommandBufferBeginInfo;
	cmdBeginInfo.pNext = nullptr;
	cmdBeginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

	cmd.begin(cmdBeginInfo);

	function(cmd);

	cmd.end();

	vk::SubmitInfo cmdinfo = {};
	cmdinfo.pNext = nullptr;
	cmdinfo.waitSemaphoreCount = 0;
	cmdinfo.pWaitSemaphores = nullptr;
	cmdinfo.pWaitDstStageMask = nullptr;
	cmdinfo.commandBufferCount = 1;
	cmdinfo.pCommandBuffers = &cmd;
	cmdinfo.signalSemaphoreCount = 0;
	cmdinfo.pSignalSemaphores = nullptr;

	m_graphicsQueue.submit(cmdinfo, m_immFence);

	m_device.waitForFences(1, &m_immFence, VK_TRUE, UINT64_MAX);
}

void Engine::cleanup_swapchain() {

	for (vkUtil::SwapChainFrame& frame : m_swapchainFrames) {
		frame.destroy();
	}
	m_device.destroySwapchainKHR(m_swapchain);

	m_device.destroyDescriptorPool(m_frameDescriptorPool[pipelineType::COMPUTE]);

}

std::vector<char> Engine::LoadEmbeddedFontResource(int resourceID) {
	HMODULE hModule = GetModuleHandle(NULL);
	HRSRC hResource = FindResource(hModule, MAKEINTRESOURCE(resourceID), RT_FONT);
	HGLOBAL hLoadedResource = LoadResource(hModule, hResource);
	DWORD dwResourceSize = SizeofResource(hModule, hResource);
	const void* pResourceData = LockResource(hLoadedResource); 

	std::vector<char> fontData(static_cast<const char*>(pResourceData),
		static_cast<const char*>(pResourceData) + dwResourceSize);
	return fontData;
}

Engine::~Engine() {
	glslang::FinalizeProcess();
	m_device.waitIdle();

	vkLogging::Logger::get_logger()->print("Goodbye see you!");

    ImGui_ImplVulkan_Shutdown();
    m_device.destroyDescriptorPool(m_imguiPool);

	m_device.destroyFence(m_mainFence);
    m_device.destroyFence(m_immFence);

	m_device.destroyCommandPool(m_commandPool);
    m_device.destroyCommandPool(m_immCommandPool);

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
	m_instance.destroy();
}