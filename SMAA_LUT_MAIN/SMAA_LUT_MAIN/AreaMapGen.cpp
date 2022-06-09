#include "AreaMapGen.h"
#include <xutility>
#include <algorithm>
#include <cstring>
#include "DirectXTex/DirectXTex.h"
#include "DirectXTex.inl"
#include <cmath>
namespace LUT
{

	AreaMapGen::AreaMapGen()
	{
		Initialize();
	}

	AreaMapGen::~AreaMapGen()
	{
		Finalize();
	}

	void AreaMapGen::Initialize() noexcept
	{
		mAreaMap.format = DXGI_FORMAT_R8G8_UNORM;
		mAreaMap.width = SIZE_ORTHO * 5 + SIZE_DIAG * 4;
		mAreaMap.height = SIZE_ORTHO * 5 * sizeof(SUBSAMPLE_OFFSETS_ORTHO) / sizeof(float);
		mAreaMap.rowPitch = 2 * mAreaMap.width;
		mAreaMap.slicePitch = 2 * mAreaMap.width * mAreaMap.height;
		mAreaMap.pixels = new uint8_t[mAreaMap.slicePitch];
		std::memset(mAreaMap.pixels, 0, mAreaMap.slicePitch);
	}

	void AreaMapGen::Finalize() noexcept
	{
		if (mAreaMap.pixels)
		{
			delete[] mAreaMap.pixels;
			mAreaMap.pixels = nullptr;
		}
	}

	void AreaMapGen::GenDiag() noexcept
	{

	}

	void AreaMapGen::GenOrtho() noexcept
	{
		for (size_t i = 0; i < std::size(SUBSAMPLE_OFFSETS_ORTHO); ++i)
		{
			for(int left = 0; left < SIZE_ORTHO; ++left)
				for (int right = 0; right < SIZE_ORTHO; ++right)
				{
					for (int pattern = 0; pattern < 16; ++pattern)
						GenOrtho(pattern, left, right, i);
				}
		}
	}

	void AreaMapGen::GenOrtho(int pattern, int left, int right, int offset_index)
	{
		float d = left + right + 1;
		float o1 = 0.5 + SUBSAMPLE_OFFSETS_ORTHO[offset_index];
		float o2 = 0.5 + SUBSAMPLE_OFFSETS_ORTHO[offset_index] - 1;
		auto pos = CalOrthoPixelPos(pattern, left, right, offset_index);
		if (pos == 32 + 640)
		{
			int a = 10;
		}
		switch (pattern)
		{
		case 0:
			/**
			 * ------
			 */
			mAreaMap.pixels[pos] = 0; //R
			mAreaMap.pixels[pos + 1] = 0; //G
			break;
		case 1:
			/**
			 * -------
			 * |
			 */
			if (left > right)
			{
				mAreaMap.pixels[pos] = 0; //Rx
				mAreaMap.pixels[pos + 1] = 0; //G
			}
			else
			{
				auto pixel = CalWight(0, o2, d / 2.f, 0, left);
				mAreaMap.pixels[pos] = std::get<0>(pixel);
				mAreaMap.pixels[pos + 1] = std::get<1>(pixel);
			}
			break;
		case 2:
			/**
			* ------
			*      |
			*/
			if (left >= right)
			{
				auto pixel = CalWight(d / 2.f, 0, d, o2, left);
				mAreaMap.pixels[pos] = std::get<0>(pixel);
				mAreaMap.pixels[pos + 1] = std::get<1>(pixel);
			}
			else
			{
				mAreaMap.pixels[pos] = 0; //Rx
				mAreaMap.pixels[pos + 1] = 0; //G
			}
			break;
		case 3:
			/**
			*  ------
			*  |    |
			*/
			break;
		default:
			break;
		}
	}

	size_t AreaMapGen::CalOrthoPixelPos(int pattern, int left, int right, int offset)
	{
		if (pattern == 1 && offset == 0)
		{
 			int debug = 10l;
		}
		int row = offset * SIZE_ORTHO * 5;
		int col = 0;
		int cluster = pattern / 4;
		row += cluster > 1 ? 3 * SIZE_ORTHO : 0;
		col += cluster % 2 ? 3 * SIZE_ORTHO : 0;

		row += (pattern % 4) > 1 ? SIZE_ORTHO: 0;
		col += (pattern % 4) % 2 ? SIZE_ORTHO : 0;
		row += right;
		col += left;

		return row * mAreaMap.rowPitch + col * 2;
	}

	std::tuple<float, float> AreaMapGen::CalWight(float _x1, float _y1, float _x2, float _y2, float left)
	{
		float dirX = _x2 - _x1;
		float dirY = _y2 - _y1;

		float px1 = left;
		float px2 = left + 1;

		float py1 = _y1 + (dirY / dirX) * (px1 - _x1);/* (y = k(x - x0) + b)*/
		float py2 = _y1 + (dirY / dirX) * (px2 - _x1);

		if ((px1 >= _x1 && px1 < _x2) || (px2 > _x1 || px2 <= _x2))
		{
			// is trapezoid
			if ((std::copysign(1.0, py1) == std::copysign(1.0, py2)) || (std::fabs(py1) <= 1e-4 || std::fabs(py2) <= 1e-4))
			{
				float a = (py1 + py2) / 2.f;
				auto pos_a = std::fabs(a);
				float norm_value = (pos_a * 255.f);
				return std::make_tuple(a < 0 ? norm_value : 0, a < 0 ? 0 : norm_value);
			}
			// 2 triangles
			else
			{

				float x = -_y1 * (dirX / dirY) + _x1;
				int int_part = x;
				float float_part = std::fabs(x - int_part);
				float a1 = x > _x1 ? py1 * float_part / 2 : 0;
				float a2 = x < _x2 ? py2 * (1 - float_part) / 2 : 0;
				float a = std::fabs(a1) > std::fabs(a2) ? a1 : -a2;

				float norm_value1 = (std::fabs(a1) * 255.f);
				float norm_value2 = (std::fabs(a2) * 255.f);
				return std::make_tuple(a < 0 ? norm_value1 : norm_value2, a < 0 ? norm_value2 : norm_value1);
			}
		}
		else // no insects
		{
			return std::make_tuple(0, 0);
		}
	}

	void AreaMapGen::Generate() noexcept
	{
		GenOrtho();
		GenDiag();

		const wchar_t* path = L"test.dds";

		DirectX::SaveToDDSFile(mAreaMap, DirectX::DDS_FLAGS::DDS_FLAGS_NONE, path);
	}
}