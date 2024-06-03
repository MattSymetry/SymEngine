#include "compute_pipeline.h"
#include "../../logging.h"

vkInit::ComputePipelineBuilder::ComputePipelineBuilder(vk::Device device) {
	this->m_device = device;
	reset();

	//Some stages are fixed with sensible defaults and don't
	//need to be reconfigured
    m_pipelineInfo.basePipelineHandle = nullptr;
}

vkInit::ComputePipelineBuilder::~ComputePipelineBuilder() {
	reset();
}

void vkInit::ComputePipelineBuilder::reset() {

    m_pipelineInfo.flags = vk::PipelineCreateFlags();

	reset_shader_modules();
	reset_descriptor_set_layouts();
}

void vkInit::ComputePipelineBuilder::reset_shader_modules() {
	if (m_computeShader) {
        m_device.destroyShaderModule(m_computeShader);
        m_computeShader = nullptr;
	}
}

void vkInit::ComputePipelineBuilder::specify_compute_shader(const char* filename) {

	if (m_computeShader) {
        m_device.destroyShaderModule(m_computeShader);
        m_computeShader = nullptr;
	}

	vkLogging::Logger::get_logger()->print("Create compute shader module");
    m_computeShader = vkUtil::createModule(filename, m_device); 
    m_computeShaderInfo = make_shader_info(m_computeShader, vk::ShaderStageFlagBits::eCompute);
}

vk::PipelineShaderStageCreateInfo vkInit::ComputePipelineBuilder::make_shader_info(
	const vk::ShaderModule& shaderModule, const vk::ShaderStageFlagBits& stage) {

	vk::PipelineShaderStageCreateInfo shaderInfo = {};
	shaderInfo.flags = vk::PipelineShaderStageCreateFlags();
	shaderInfo.stage = stage;
	shaderInfo.module = shaderModule; 
	shaderInfo.pName = "main";  
	return shaderInfo;
}

void vkInit::ComputePipelineBuilder::add_descriptor_set_layout(vk::DescriptorSetLayout descriptorSetLayout) {
    m_descriptorSetLayouts.push_back(descriptorSetLayout);
}

void vkInit::ComputePipelineBuilder::reset_descriptor_set_layouts() {
    m_descriptorSetLayouts.clear();
}

vkInit::ComputePipelineOutBundle vkInit::ComputePipelineBuilder::build() {

	//Compute Shader
    m_pipelineInfo.stage = m_computeShaderInfo;

	//Pipeline Layout
	vkLogging::Logger::get_logger()->print("Create Pipeline Layout");
	vk::PipelineLayout pipelineLayout = make_pipeline_layout();
    m_pipelineInfo.layout = pipelineLayout;

	//Make the Pipeline
	vkLogging::Logger::get_logger()->print("Create Compute Pipeline");
	vk::Pipeline computePipeline;
	try {
        computePipeline = (m_device.createComputePipeline(nullptr, m_pipelineInfo)).value;
	}
	catch (vk::SystemError err) {
		vkLogging::Logger::get_logger()->print("Failed to create Pipeline");
	}

	ComputePipelineOutBundle output;
	output.layout = pipelineLayout;
	output.pipeline = computePipeline;

	return output;
}

vk::PipelineLayout vkInit::ComputePipelineBuilder::make_pipeline_layout() {

	/*
	typedef struct VkPipelineLayoutCreateInfo {
		VkStructureType                 sType;
		const void*                     pNext;
		VkPipelineLayoutCreateFlags     flags;
		uint32_t                        setLayoutCount;
		const VkDescriptorSetLayout*    pSetLayouts;
		uint32_t                        pushConstantRangeCount;
		const VkPushConstantRange*      pPushConstantRanges;
	} VkPipelineLayoutCreateInfo;
	*/

	vk::PipelineLayoutCreateInfo layoutInfo;
	layoutInfo.flags = vk::PipelineLayoutCreateFlags();

	layoutInfo.setLayoutCount = static_cast<uint32_t>(m_descriptorSetLayouts.size());
	layoutInfo.pSetLayouts = m_descriptorSetLayouts.data();

	layoutInfo.pushConstantRangeCount = 0;

	try {
		return m_device.createPipelineLayout(layoutInfo);
	}
	catch (vk::SystemError err) {
		vkLogging::Logger::get_logger()->print("Failed to create pipeline layout!");
	}
}
