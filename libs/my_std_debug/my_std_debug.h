#ifndef __MY_STD_DEBUG_H__  
#define __MY_STD_DEBUG_H__

#include <stdio.h>
#include "my_std_ret.h"


#define MY_STD_DEBUG_MSG(msg) printf("file: %s, func: %s, line: %d -> %s\n", __FILE__, __func__, __LINE__, msg)
#define MY_STD_DEBUG_ERROR_CHECK(func) if(func != MY_STD_RET_OK){ MY_STD_DEBUG_MSG("Error check"); }



#endif