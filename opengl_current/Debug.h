#pragma once

#include "Logging.h"

#if defined(DEBUG) || defined(_DEBUG)
DECLARE_LOG_CATEGORY(DEBUG);

#define DEBUG_LOG(message, ...) ELOG_DEBUG(LOG_DEBUG, message, __VA_ARGS__)
#define DEBUG_ERROR_LOG(message, ...) ELOG_ERROR(LOG_DEBUG, message, __VA_ARGS__)

#else
#define DEBUG_LOG(message, ...)
#define DEBUG_ERROR_LOG(message, ...)
#endif