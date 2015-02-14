//
// Tomato ASIK
// 分类器
//
// (c) 2014 SunnyCase
// 创建日期: 2015-02-05
#pragma once
#include "../../platform.h"
#include "../spectrogram.h"
#include "../ck_distance_service.h"
#include <amp_graphics.h>

#define NSDEF_ASIK_CORE_CLASSIFIER namespace Tomato{namespace ASIK{namespace Core{namespace Classifier{
#define NSED_ASIK_CORE_CLASSIFIER }}}}
#define NS_ASIK_CORE_CLASSIFIER Tomato::ASIK::Core::Classifier

NSDEF_ASIK_CORE_CLASSIFIER

typedef uint32_t class_id_t;

// 分类器
class classifier
{
public:
	virtual ~classifier(){}

	virtual void ASIKCALL add_input(class_id_t class_id, std::unique_ptr<spectrogram>&& spectrogram) = 0;
	virtual void ASIKCALL compute_fingerprint() = 0;
	virtual void ASIKCALL compute_fingerprint(class_id_t class_id) = 0;
	virtual void ASIKCALL set_ck_distance_service(ck_distance_service* service) = 0;
};

NSED_ASIK_CORE_CLASSIFIER

extern "C" ASIKAPI void ASIKCALL CreateClassifier(size_t min_length, size_t max_length, std::unique_ptr<NS_ASIK_CORE_CLASSIFIER::classifier>& classi);