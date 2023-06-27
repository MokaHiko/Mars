#ifndef MEMORY_H
#define MEMORY_H

#pragma once

#ifdef _DEBUG
#define MRS_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#else
#define MRS_NEW new
#endif

#endif