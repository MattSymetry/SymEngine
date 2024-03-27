#pragma once
#include "../../../common/config.h"
#include "memory.h"
#include "buffer.h"

namespace vkUtil {

	/**
		Holds the data structures associated with a "Frame"
	*/
	class SwapChainFrame {

	public:

		//For doing work
		vk::Device logicalDevice;
		vk::PhysicalDevice physicalDevice;

		//Swapchain-type stuff
		vk::Image image;
		vk::ImageView imageView;
		int width, height;

		vk::CommandBuffer commandBuffer;

		//Sync objects
		vk::Semaphore imageAvailable, renderFinished;
		vk::Fence inFlight;

		//Resources
        std::vector<BufferSetup> bufferSetups;
        

		//Resource Descriptors
		vk::DescriptorImageInfo colorBufferDescriptor;
		std::unordered_map<pipelineType, vk::DescriptorSet> descriptorSet;

		//Write Operations
		std::vector<vk::WriteDescriptorSet> writeOps;
        
        vk::DescriptorSet m_drawImageDescriptors;
        vk::DescriptorSetLayout m_drawImageDescriptorLayout;
        //AllocatedImage m_drawImage;

		SwapChainFrame(vk::PhysicalDevice physicalDevice, vk::Device logicalDevice, 
			uint32_t width, uint32_t height);
        
        void AddBuffers(const std::vector<BufferInitParams>& bufferParams, vk::Device logicalDevice, vk::PhysicalDevice physicalDevice);

		void make_descriptor_resources(vk::Device logicalDevice, vk::PhysicalDevice physicalDevice);

		void record_write_operations();

		void write_descriptor_set();

		void destroy();
	};
    
    VkRenderingInfo rendering_info(VkExtent2D swapchainExtent, VkRenderingAttachmentInfo* colorAttachment, VkRenderingAttachmentInfo* depthAttachment);
    
    void transition_image(VkCommandBuffer cmdBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);

}
