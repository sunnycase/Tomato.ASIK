// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once
#define NOMINMAX

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#include <iostream>
#include <sstream>
#include <iomanip>


// TODO:  在此处引用程序需要的其他头文件
#include "../include/core/io/io.h"
#include "../include/block_buffer.hpp"
#include "../include/core/spectrogram.h"
#include "../include/core/ck_distance.h"
#include "../include/core/ck_distance_service.h"
#include "../include/core/classifier/classifier.h"