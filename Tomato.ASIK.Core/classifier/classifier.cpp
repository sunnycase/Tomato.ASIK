//
// Tomato ASIK
// 分类器
//
// (c) 2014 SunnyCase
// 创建日期: 2015-02-05
#include "stdafx.h"
#include "../stdafx.h"
#include "classifier_impl.h"

using namespace NS_ASIK_CORE;
using namespace NS_ASIK_CORE_CLASSIFIER;
using namespace wrl;
using namespace concurrency;
using namespace concurrency::graphics;

#define PRINT_LOG 1

classifier_impl::classifier_impl(size_t min_length, size_t max_length)
	:min_length(min_length), max_length(max_length), max_spectrogram_length(0)
{

}

void ASIKCALL classifier_impl::add_input(class_id_t class_id, std::unique_ptr<spectrogram>&& spectrogram)
{
	if (spectrogram->length > max_spectrogram_length)
		max_spectrogram_length = spectrogram->length;

	spectrograms.emplace(class_id,
		std::unique_ptr<spectrogram_impl>((spectrogram_impl*)spectrogram.release()));
	class_ids.emplace(class_id);
}

void ASIKCALL classifier_impl::set_ck_distance_service(ck_distance_service * service)
{
	this->ck_service = service;
}

void ASIKCALL classifier_impl::compute_fingerprint()
{
	fingerprints.clear();
	for (auto&& id : class_ids)
		fingerprints.emplace(id, compute_fingerprint(id));
}

std::vector<fingerprint> classifier_impl::compute_fingerprint(class_id_t class_id)
{
#if PRINT_LOG
	std::cout << "Computing Class " << class_id << "..." << std::endl;
#endif
	std::vector<spectrogram_impl*> targets, compares;
	targets.reserve(spectrograms.size());
	compares.reserve(spectrograms.size());
	for (const auto& pair : spectrograms)
	{
		if (pair.first == class_id)
			targets.emplace_back(pair.second.get());
		else
			compares.emplace_back(pair.second.get());
	}
	org_entropy = compute_entropy(targets.size(), compares.size(), spectrograms.size());

	std::vector<fingerprint> prints;
	auto max_length = std::min(this->max_length, max_spectrogram_length);
	auto min_length = std::min(this->min_length, max_length);

	for (size_t len = min_length; len <= max_length; len++)
	{
#if PRINT_LOG
		std::cout << "-- Fingerprint Length: " << len << std::endl;
#endif
		compute_fingerprint(prints, targets, compares, len);
	}

	return std::move(prints);
}

void classifier_impl::compute_fingerprint(std::vector<fingerprint>& prints,
	const std::vector<spectrogram_impl*>& targets, const std::vector<spectrogram_impl*>& compares, size_t finger_length)
{
	for (auto target : targets)
	{
		size_t bestIndex = 0;
		auto bestValue = fingerprint_value::bad();
		std::shared_ptr<sample> bestSample;

		auto endIndex = target->length - finger_length;
		assert(target->length >= finger_length);
		for (size_t startIndex = 0; startIndex <= endIndex; startIndex++)
		{
			auto target_sample = target->get_sample(startIndex, finger_length);
			auto value = evaluate_fingerprint(target_sample.get(), targets, compares, bestValue);
			if (value > bestValue)
			{
				bestValue = value;
				bestIndex = startIndex;
				bestSample = std::move(target_sample);
			}
		}
#if PRINT_LOG
		std::cout << "-- Best fingerprint: Index " << bestIndex <<
			", Entropy Gain " << bestValue.entropy_gain << ", Threshold "
			<< bestValue.threshold << ", Margin " << bestValue.margin << std::endl;
#endif
		prints.emplace_back(fingerprint{ std::move(bestSample), bestValue.threshold });
	}
}

classifier_impl::fingerprint_value classifier_impl::evaluate_fingerprint(sample * target, const std::vector<spectrogram_impl*>& targets, const std::vector<spectrogram_impl*>& compares, const fingerprint_value& best_so_far)
{
	auto value = fingerprint_value::bad();

	const auto pCount = targets.size();
	const auto uCount = compares.size();
	const auto totalCount = pCount + uCount;

	std::multiset<float> target_d;
	// true : target
	// false : compare
	std::vector<std::pair<bool, float>> distances;
	for (const auto& pair : targets)
	{
		auto distance = compute_distance(target, pair);
		distances.emplace_back(std::make_pair(true, distance));
		target_d.emplace(distance);
	}

	auto min_compare = std::numeric_limits<float>::max();
	for (const auto& pair : compares)
	{
		auto distance = compute_distance(target, pair);
		distances.emplace_back(std::make_pair(false, distance));

		if (distance < min_compare)
		{
			min_compare = distance;
			auto left_p = std::distance(target_d.begin(), target_d.lower_bound(distance));
			auto best_entropy = compute_entropy(left_p, 0, totalCount) +
				compute_entropy(pCount - left_p, uCount, totalCount);
			auto best_entropy_gain = org_entropy - best_entropy;
			if (best_entropy_gain <= best_so_far.entropy_gain)
				return value;
		}
	}

	std::sort(distances.begin(), distances.end(),
		[](const std::pair<bool, float>& left, const std::pair<bool, float>& right) {
		return std::less<float>()(left.second, right.second);
	});

	auto prevDistance = distances.front().second;
	size_t left_p = distances.front().first ? 1 : 0;
	size_t left_u = 1 - left_p;
	for (auto it = ++distances.cbegin(); it != distances.cend(); ++it)
	{
		auto cntDistance = it->second;
		auto threhold = (prevDistance + cntDistance) / 2.f;
		auto margin = cntDistance - prevDistance;
		prevDistance = cntDistance;

		if (margin != 0.0f)
		{
			auto right_p = pCount - left_p;
			auto right_u = uCount - left_u;

			auto entropy = compute_entropy(left_p, left_u, totalCount) +
				compute_entropy(right_p, right_u, totalCount);

			fingerprint_value newValue = { org_entropy - entropy, threhold, margin };

			if (newValue > value)
				value = newValue;
		}

		it->first ? left_p++ : left_u++;
	}
	return value;
}

float classifier_impl::compute_distance(sample * target, spectrogram_impl* compare)
{
	float bestDistance = std::numeric_limits<float>::max();
	auto finger_length = target->width;
	auto endIndex = compare->length - finger_length;
	for (size_t startIndex = 0; startIndex <= endIndex; startIndex++)
	{
		auto compare_sample = compare->get_sample(startIndex, finger_length);
		auto distance = ck_service->compute(target, compare_sample.get());
		if (distance < bestDistance)
			bestDistance = distance;
	}
	return bestDistance;
}

float classifier_impl::compute_entropy(size_t x_count, size_t y_count, size_t total_count)
{
	float range_count = x_count + y_count;
	float pX = float(x_count) / range_count;
	float pY = float(y_count) / range_count;

	auto eX = pX == 0.f ? 0.f : pX * std::log2(pX);
	auto eY = pY == 0.f ? 0.f : pY * std::log2(pY);

	return (-eX - eY) * (range_count / total_count);
}

void ASIKCALL CreateClassifier(size_t min_length, size_t max_length, std::unique_ptr<NS_ASIK_CORE_CLASSIFIER::classifier>& classi)
{
	classi = std::make_unique<classifier_impl>(min_length, max_length);
}