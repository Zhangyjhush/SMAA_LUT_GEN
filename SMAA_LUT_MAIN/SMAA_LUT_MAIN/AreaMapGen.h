#pragma once
#include <cstdint>
#include <tuple>
#include "DirectXTex/DirectXTex.h"
namespace LUT
{
	constexpr uint32_t SIZE_ORTHO = 16;
	constexpr uint32_t SIZE_DIAG = 20;

	constexpr uint32_t SAMPLES_DIAG = 30;
	constexpr uint32_t SMOOTH_MAX_DISTANCE = 32;

	constexpr float SUBSAMPLE_OFFSETS_ORTHO[] = 
	{
		0.0,
		-0.25,
		0.25,
		-0.125,
		0.125,
		-0.375,
		0.375 
	};

	constexpr float SUBSAMPLE_OFFSETS_DIAG[][2] =
	{
		{0, 0},
		{0.25, -0.25},
		{-0.25, 0.25},
		{0.125, -0.125},
		{-0.125, 0.125}
	};

	class AreaMapGen
	{
	public:
		AreaMapGen();
		~AreaMapGen();

		void Generate() noexcept;
	protected:
		void Initialize() noexcept;
		void Finalize() noexcept;
		void GenDiag() noexcept;
		void GenOrtho() noexcept;
		void GenOrtho(int pattern, int left, int right, int offset_index);
		size_t CalOrthoPixelPos(int pattern, int left, int right, int offset);
		std::tuple<float, float> CalWight(float x1, float y1, float x2, float y2, float left);
	private:
		DirectX::Image mAreaMap;
	};
}