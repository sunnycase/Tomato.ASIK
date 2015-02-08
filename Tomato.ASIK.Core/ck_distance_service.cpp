//
// Tomato ASIK
// CK Distance ����
//
// (c) 2014 SunnyCase
// ��������: 2015-02-08
#include "stdafx.h"
#include "ck_distance_service_impl.h"

using namespace NS_ASIK_CORE;
using namespace wrl;
using namespace concurrency;

ck_distance_service_impl::ck_distance_service_impl(size_t height)
	:height(height)
{
	// 16 �ı������Ҵ��� 128
	if (height < 128 || height % 16 != 0)
		throw std::exception("height is not compatible with H.264.");
}

size_t ck_distance_service_impl::make_width_compatible(size_t width) noexcept
{
	if (width <= 96)
		return 96;

	// 16 �ı���
	auto newWidth = width - width % 16;
	if (newWidth < width)
		newWidth += 16;
	return newWidth;
}

std::unique_ptr<sample_impl> ck_distance_service_impl::make_sample_compatible(sample_impl * sample)
{
	if (sample->height != height)
		throw std::exception("height is not match.");

	auto oldWidth = sample->width;
	auto newWidth = make_width_compatible(oldWidth);

	auto oldView = sample->view;
	array<uint32_t, 2> newData((int)height, (int)newWidth);
	array_view<uint32_t, 2> newView(newData);
	newView.discard_data();

	parallel_for_each(oldView.extent, [oldView, newView](index<2> idx)restrict(amp)
	{
		newView[idx] = oldView[idx];
	});
	return std::make_unique<sample_impl>(newView);
}

ck_distance * ck_distance_service_impl::acquire_ck_distance_instance(size_t width)
{
	auto it = ck_instances.find(width);
	if (it != ck_instances.end())
		return it->second.get();

	return ck_instances.emplace(width, 
		std::make_unique<ck_distance>(width, height)).first->second.get();
}

float ASIKCALL ck_distance_service_impl::compute(sample* sampleA, sample* sampleB)
{
	if (sampleA->height != height || sampleB->height != height)
		throw std::exception("height is not match.");
	if (sampleA->width != sampleB->width)
		throw std::exception("width is not match.");

	auto x = make_sample_compatible((sample_impl*)sampleA);
	auto y = make_sample_compatible((sample_impl*)sampleB);
	auto ck_inst = acquire_ck_distance_instance(x->width);
	return ck_inst->compute(x->view, y->view);
}

void ASIKCALL CreateCKDistanceService(size_t height, std::unique_ptr<ck_distance_service>& service)
{
	service = std::make_unique<ck_distance_service_impl>(height);
}