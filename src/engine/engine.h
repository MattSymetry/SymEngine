#pragma once
#include "../common/config.h"
#include "vulkan/vkUtil/frame.h"
#include "scene.h"
#include "vulkan/vkJob/job.h"
#include <functional>

class Engine {

public:

	Engine(int width, int height, SDL_Window* window, Scene* scene);

	~Engine();

	void render(Scene* scene);
    
    void immediate_submit(std::function<void(vk::CommandBuffer cmd)>&& function);

private:

	//glfw-related variables
	int m_width;
	int m_height;
	SDL_Window* m_window;

	//instance-related variables
	vk::Instance m_instance{ nullptr };
	vk::DebugUtilsMessengerEXT m_debugMessenger{ nullptr };
	vk::DispatchLoaderDynamic m_dldi;
	vk::SurfaceKHR m_surface;

	//device-related variables
	vk::PhysicalDevice m_physicalDevice{ nullptr };
	vk::Device m_device{ nullptr };
	vk::Queue m_graphicsQueue{ nullptr };
	vk::Queue m_presentQueue{ nullptr };
	vk::SwapchainKHR m_swapchain{ nullptr };
	std::vector<vkUtil::SwapChainFrame> m_swapchainFrames;
	vk::Format m_swapchainFormat;
	vk::Extent2D m_swapchainExtent;

	//pipeline-related variables
	std::vector<pipelineType> m_pipelineTypes =  {pipelineType::COMPUTE} ;
	std::unordered_map<pipelineType, vk::PipelineLayout> m_pipelineLayout;
	std::unordered_map<pipelineType, vk::Pipeline> m_pipeline;

	//descriptor-related variables
	std::unordered_map<pipelineType, vk::DescriptorSetLayout> m_frameSetLayout;
	std::unordered_map<pipelineType, vk::DescriptorPool> m_frameDescriptorPool; //Descriptors bound on a "per frame" basis

	//Command-related variables
	vk::CommandPool m_commandPool;
	vk::CommandBuffer m_mainCommandBuffer;
	vk::Fence m_mainFence;

	//Synchronization objects
	int m_maxFramesInFlight, m_frameNumber;
    
    // immidiate submit structs
    vk::Fence m_immFence;
    vk::CommandBuffer m_immCommandBuffer;
    vk::CommandPool m_immCommandPool;
    vk::DescriptorPool m_imguiPool;

	//instance setup
	void make_instance();
    
    // imgui
    void init_imgui();

	//device setup
	void make_device(Scene* scene);
	void make_swapchain(Scene* scene);
    void recreate_swapchain(Scene* scene);

	//pipeline setup
	void make_descriptor_set_layouts(Scene* scene);
	void make_pipelines();

	//final setup steps
	void finalize_setup(Scene* scene);
	void make_frame_resources(Scene* scene);

	//asset creation
	void make_assets(Scene* scene);

	void prepare_frame(uint32_t imageIndex, Scene* scene);
	void prepare_scene(vk::CommandBuffer commandBuffer);
	void prepare_to_trace_barrier(vk::CommandBuffer commandBuffer, vk::Image image);
	void dispatch_compute(vk::CommandBuffer commandBuffer, uint32_t imageIndex);
	void prepare_to_present_barrier(vk::CommandBuffer commandBuffer, vk::Image image);
    
    vk::RenderingAttachmentInfoKHR attachment_info(
        vk::ImageView view, vk::ClearValue* clear, vk::ImageLayout layout);
    vk::RenderingInfoKHR rendering_info(
        vk::Extent2D renderExtent, vk::RenderingAttachmentInfoKHR* colorAttachment, vk::RenderingAttachmentInfoKHR* depthAttachment);
    void draw_imgui(vk::CommandBuffer cmd, vk::ImageView targetImageView);

	//Cleanup functions
	void cleanup_swapchain();
};
