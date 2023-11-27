#include "Font.h"
#include "Core/Log.h"

mrs::Font::Font(Ref<Sprite> atlas) 
{
	static char alphabet[26] = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z'};
	static char alphabet_c[26] = {'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z'};
	static char nums[10] = {'0','1','2','3','4','5','6','7','8','9'};

	// Calculate UVS
	Vector2 a_coords = { 2, 1};
	Vector2 zero_coords = {13, 0};

	float w = atlas->Atlas()->_width;
	float h = atlas->Atlas()->_height;

	Vector2 dims_unit_size = {13.0f, 7.0f}; // Dimensions in unit size;
	Vector2 unit_size = {w / 13.0f, h / 7.0f};
	
	// Fill letters
	int x_offset = 0;
	int y_offset = 0;
	for(int i = 0; i < 26; i++)
	{
		Rect rect = {};
		rect.x = a_coords.x * (1 + (unit_size.x * x_offset));
		rect.y = a_coords.y * (1 + (unit_size.y * y_offset));

		rect.width = unit_size.x;
		rect.height = unit_size.y;

		x_offset++;
		if(x_offset > dims_unit_size.x)
		{
			x_offset = 0;
			y_offset++;
		}

		_uvs[alphabet[i]] = rect;
		_uvs[alphabet_c[i]] = rect;
	}

	// Fill numbers
	x_offset = 0;
	y_offset = 0;
	for(int i = 0; i < 10; i++)
	{
		Rect rect = {};
		rect.x = zero_coords.x * (1 + (unit_size.x * x_offset));
		rect.y = zero_coords.y * (1 + (unit_size.y * y_offset));

		rect.width = unit_size.x;
		rect.height = unit_size.y;

		x_offset++;
		if(x_offset > dims_unit_size.x)
		{
			x_offset = 0;
			y_offset++;
		}

		_uvs[nums[i]] = rect;
		_uvs[nums[i]] = rect;
	}
}

const mrs::Rect& mrs::Font::GetUVS(const char c) const
{
	const auto& it = _uvs.find(c);

	if(it != _uvs.end())
	{
		MRS_ERROR("Font does not contain character %c", c);
		return it->second;
	}

	return {};
}
