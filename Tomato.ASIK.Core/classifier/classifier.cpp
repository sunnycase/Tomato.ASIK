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
using namespace boolinq;

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
#if _DEBUG
	std::cout << "Computing Class " << class_id << "..." << std::endl;
#endif
	auto targets = from_spectrograms(class_id).toVector();
	auto compares = from_spectrograms().where([=](const spectrogram_pair& spec)
	{
		return spec.class_id != class_id;
	}).toVector();

	std::vector<fingerprint> prints;
	auto max_length = std::min(this->max_length, max_spectrogram_length);
	auto min_length = std::min(this->min_length, max_length);

	for (size_t len = min_length; len <= max_length; len++)
	{
#if _DEBUG
		std::cout << "-- Fingerprint Length: " << len << std::endl;
#endif
		compute_fingerprint(prints, targets, compares, len);
	}

	return std::move(prints);
}

void classifier_impl::compute_fingerprint(std::vector<fingerprint>& prints,
	const std::vector<spectrogram_pair>& targets, const std::vector<spectrogram_pair>& compares, size_t finger_length)
{
	for (const auto& target : targets)
	{
		size_t bestIndex = 0;
		auto bestValue = fingerprint_value::bad();
		std::shared_ptr<sample> bestSample;

		auto target_spec = target.spec;
		auto endIndex = target_spec->length - finger_length;
		assert(target_spec->length >= finger_length);
		for (size_t startIndex = 0; startIndex <= endIndex; startIndex++)
		{
			auto target_sample = target_spec->get_sample(startIndex, finger_length);
			auto value = evaluate_fingerprint(target_sample.get(), targets, compares);
			if (value > bestValue)
			{
				bestValue = value;
				bestIndex = startIndex;
				bestSample = std::move(target_sample);
			}
		}
		prints.emplace_back(fingerprint{ std::move(bestSample), bestValue.threhold });
	}
}

classifier_impl::spectrogram_linq_t classifier_impl::from_spectrograms()
{
	auto end = spectrograms.end();
	return spectrogram_enumerator_t([end](classifier_impl::spectrograms_t::iterator& iter)
	{
		return (iter == end) ? throw EnumeratorEndException() :
			iter++, spectrogram_pair{ iter->first, iter->second.get() };
	}, spectrograms.begin());
}

classifier_impl::spectrogram_linq_t classifier_impl::from_spectrograms(class_id_t class_id)
{
	auto range = spectrograms.equal_range(class_id);
	auto end = range.second;
	return spectrogram_enumerator_t([end](classifier_impl::spectrograms_t::iterator& iter)
	{
		return (iter == end) ? throw EnumeratorEndException() :
			iter++, spectrogram_pair{ iter->first, iter->second.get() };
	}, range.first);
}

classifier_impl::fingerprint_value classifier_impl::evaluate_fingerprint(sample * target, const std::vector<spectrogram_pair>& targets, const std::vector<spectrogram_pair>& compares)
{
	auto value = fingerprint_value::bad();

	// true : target
	// false : compare
	std::vector<std::pair<bool, float>> distances;
	for (const auto& pair : targets)
		distances.emplace_back(std::make_pair(true, compute_distance(target, pair)));
	for (const auto& pair : compares)
		distances.emplace_back(std::make_pair(false, compute_distance(target, pair)));

	std::sort(distances.begin(), distances.end(),
		[](const std::pair<bool, float>& left, const std::pair<bool, float>& right) {
		return std::less<float>()(left.second, right.second);
	});

	const auto totalCount = distances.size();
	const auto pCount = targets.size();
	const auto uCount = compares.size();

	auto prevDistance = distances.front().second;
	size_t left_p = distances.front().first ? 1 : 0;
	size_t left_u = 1 - left_p;
	for (auto it = ++distances.cbegin(); it != distances.cend(); ++it)
	{
		auto cntDistance = it->second;
		auto threhold = (prevDistance + cntDistance) / 2.f;
		auto margin = cntDistance - prevDistance;
		prevDistance = cntDistance;

		auto right_p = pCount - left_p;
		auto right_u = uCount - left_u;

		fingerprint_value newValue = { compute_entropy(left_p, left_u, totalCount) +
			compute_entropy(right_p, right_u, totalCount), threhold, margin };

		if (newValue > value)
			value = newValue;

		it->first ? left_p++ : left_u++;
	}
	return value;
}

float classifier_impl::compute_distance(sample * target, const spectrogram_pair & compare)
{
	float bestDistance = std::numeric_limits<float>::max();
	auto finger_length = target->width;
	auto compare_spec = compare.spec;
	auto endIndex = compare_spec->length - finger_length;
	for (size_t startIndex = 0; startIndex <= endIndex; startIndex++)
	{
		auto compare_sample = compare_spec->get_sample(startIndex, finger_length);
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

	return (-pX * std::log(pX) - pY * std::log(pY)) * (range_count / total_count);
}
