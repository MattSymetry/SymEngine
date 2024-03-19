#pragma once
#include "../../../common/config.h"

namespace vkJob {

	enum class JobStatus {
		PENDING,
		IN_PROGRESS,
		COMPLETE
	};

	class Job {
	public:
		JobStatus status = JobStatus::PENDING;
		Job* next = nullptr;
		virtual void execute(vk::CommandBuffer commandBuffer, vk::Queue queue) = 0;
	};

	class WorkQueue {
	public:
		Job* first = nullptr, * last = nullptr;
		size_t length = 0;
		void add(Job* job);
		Job* get_next();
		void clear();
	};
}
