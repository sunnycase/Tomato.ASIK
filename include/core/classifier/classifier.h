//
// Tomato ASIK
// 分类器
//
// (c) 2014 SunnyCase
// 创建日期: 2015-02-05
#pragma once
#include "../../platform.h"
#include "../spectrogram.h"
#include <amp_graphics.h>

#define NSDEF_ASIK_CORE_CLASSIFIER namespace Tomato{namespace ASIK{namespace Core{namespace Classifier{
#define NSED_ASIK_CORE_CLASSIFIER }}}}
#define NS_ASIK_CORE_CLASSIFIER Tomato::ASIK::Core::Classifier

NSDEF_ASIK_CORE_CLASSIFIER

// 分类器
class classifier
{
public:
	virtual ~classifier(){}
};

NSED_ASIK_CORE_CLASSIFIER