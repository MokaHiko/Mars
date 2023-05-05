#ifndef VULKANUTILS_H
#define VULKANUTILS_H

#pragma once

#include <iostream>
#include <functional>
#include <deque>

#define VK_CHECK(x)                                                     \
	do                                                                  \
	{                                                                   \
		VkResult err = x;                                               \
		if (err)                                                        \
		{                                                               \
			std::cout << "Detected Vulkan error: " << err << std::endl; \
			abort();                                                    \
		}                                                               \
	} while (0)

namespace vkutil {
	struct DeletionQueue
	{
		// Pushes and takes ownership of callback in deletion queue for clean up in FIFO order
		void Push(std::function<void()>&& fn) 
		{
			clean_functions.push_back(fn);
		}

		// Calls all clean up functions in FIFO order
		void Flush()
		{
			for (auto it = clean_functions.rbegin(); it != clean_functions.rend(); it++)
			{
				(*it)();
			}

			clean_functions.clear();
		}
	private:
		std::deque<std::function<void()>> clean_functions = {};
	};
}
#endif