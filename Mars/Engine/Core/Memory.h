#ifndef MEMORY_H
#define MEMORY_H

#pragma once

#ifdef MRS_DEBUG
#define MRS_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)

template <typename T>
using Ref = std::shared_ptr<T>;
template <typename T, typename... Args>
constexpr Ref<T> CreateRef(Args &&...args)
{
	return std::make_shared<T>(std::forward<Args>(args)...);
}

#else
#define MRS_NEW new

template <typename T>
using Ref = std::shared_ptr<T>;
template <typename T, typename... Args>
constexpr Ref<T> CreateRef(Args &&...args)
{
	return std::make_shared<T>(std::forward<Args>(args)...);
}
#endif

#endif