//
// Tomato ASIK
// 分类器实现
//
// (c) 2014 SunnyCase
// 创建日期: 2015-02-05
#pragma once
#include "../../include/core/classifier/classifier.h"
#include "../sample_impl.h"
#include "../spectrogram_impl.h"

NSDEF_ASIK_CORE_CLASSIFIER

typedef uint32_t class_id_t;

struct fingerprint
{
	std::shared_ptr<sample> sample;
	float threhold;
};

class classifier_impl : public classifier
{
	struct spectrogram_pair
	{
		class_id_t class_id;
		spectrogram_impl* spec;
	};

	struct fingerprint_value
	{
		float entropy;
		float threhold;
		float margin;

		bool operator >(const fingerprint_value& value) const noexcept
		{
			return entropy < value.entropy || (entropy == value.entropy && margin > value.margin);
		}

		static fingerprint_value bad() noexcept
		{
			return { std::numeric_limits<float>::max(), 0.0f, 0.0f };
		}
	};

	typedef std::unordered_multimap<class_id_t, std::unique_ptr<spectrogram_impl>> spectrograms_t;
	typedef boolinq::Enumerator<spectrogram_pair, spectrograms_t::iterator> spectrogram_enumerator_t;
	typedef boolinq::LinqObj<spectrogram_enumerator_t> spectrogram_linq_t;
public:
	///<summary>创建分类器的新实例</summary>
	///<param name="min_length">指纹的最小长度</param>
	///<param name="max_length">指纹的最大长度</param>
	classifier_impl(size_t min_length, size_t max_length);

	virtual void ASIKCALL add_input(class_id_t class_id, std::unique_ptr<spectrogram>&& spectrogram);
	virtual void ASIKCALL set_ck_distance_service(ck_distance_service* service);
	virtual void ASIKCALL compute_fingerprint();
private:
	std::vector<fingerprint> compute_fingerprint(class_id_t class_id);
	void compute_fingerprint(std::vector<fingerprint>& prints, const std::vector<spectrogram_pair>& targets, const std::vector<spectrogram_pair>& compares, size_t finger_length);
	spectrogram_linq_t from_spectrograms();
	spectrogram_linq_t from_spectrograms(class_id_t class_id);
	fingerprint_value evaluate_fingerprint(sample* target, const std::vector<spectrogram_pair>& targets, const std::vector<spectrogram_pair>& compares);
	float compute_distance(sample* target, const spectrogram_pair& compare);

	static float compute_entropy(size_t x_count, size_t y_count, size_t total_count);
private:
	const size_t min_length;
	const size_t max_length;
	size_t max_spectrogram_length;
	ck_distance_service* ck_service;

	std::set<class_id_t> class_ids;
	spectrograms_t spectrograms;
	std::unordered_map<class_id_t, std::vector<fingerprint>> fingerprints;
};

NSED_ASIK_CORE_CLASSIFIER