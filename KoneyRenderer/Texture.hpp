#ifndef _TEXTURE
#define _TEXTURE
#include "Globel.hpp"
#include <string>
class Texture{
public:
	Texture();
	Texture(std::string path);
	~Texture();
	void Load(std::string);
	void GenMipmap();
	__forceinline DirectX::XMVECTOR NearestSampler(DirectX::XMVECTOR& uv){
		int wi = w * uv.m128_f32[0], he = h * uv.m128_f32[1];
		return *((DirectX::XMVECTOR*)(bmpData)+biwidth * he + wi);
	}

	__forceinline DirectX::XMVECTOR BilinearSampler(DirectX::XMVECTOR& uv){
		float wf = w * uv.m128_f32[0], hf = h * uv.m128_f32[1];
		int wi = wf, hi = hf;
		float lerpx = wf - wi, lerpy = hf - hi;
		DirectX::XMVECTOR* pd = (DirectX::XMVECTOR*)(bmpData)+biwidth * hi + wi, *pu = pd + biwidth;
		return DirectX::XMVectorLerp(
			DirectX::XMVectorLerp(*pd, *(pd + 1), lerpx),
			DirectX::XMVectorLerp(*pu, *(pu + 1), lerpx),
			lerpy);
	}

	__forceinline DirectX::XMVECTOR MipMapNearestSampler(DirectX::XMVECTOR& uv, int z){
		int wi = mipW[z] * uv.m128_f32[0], he = mipH[z] * uv.m128_f32[1];
		return *((DirectX::XMVECTOR*)(mipmap[z]) + mipBiWidth[z] * he + wi);
	}

	__forceinline DirectX::XMVECTOR MipMapNearestSampler(DirectX::XMVECTOR& uv, float z){
		int zz = z / zThreshold * mipS;
		zz = zz > mipS ? mipS : zz;
		return MipMapNearestSampler(uv, zz);
	}

	__forceinline DirectX::XMVECTOR MipMapBilinearSampler(DirectX::XMVECTOR& uv, int z){
		float wf = mipW[z] * uv.m128_f32[0], hf = mipH[z] * uv.m128_f32[1];
		int wi = wf, hi = hf;
		float lerpx = wf - wi, lerpy = hf - hi;
		DirectX::XMVECTOR* pd = (DirectX::XMVECTOR*)(mipmap[z]) + mipBiWidth[z] * hi + wi, *pu = pd + mipBiWidth[z];
		return DirectX::XMVectorLerp(
			DirectX::XMVectorLerp(*pd, *(pd + 1), lerpx),
			DirectX::XMVectorLerp(*pu, *(pu + 1), lerpx),
			lerpy);
	}

	__forceinline DirectX::XMVECTOR MipMapBilinearSampler(DirectX::XMFLOAT2& uv, float z){
		int zz = z / zThreshold * mipS;
		zz = zz > mipS ? mipS : zz;
		return MipMapBilinearSampler(uv, zz);
	}


	DirectX::XMVECTOR TrilinearSampler(DirectX::XMFLOAT2& uv, float z){
		float zz = z / zThreshold * mipS;
		int zi = zz;
		DirectX::XMFLOAT4 ret;
		if (zi == mipS) return MipMapBilinearSampler(uv, zi);
		else{
			float lerpz = zz - zi;
			return  DirectX::XMVectorLerp(
				MipMapBilinearSampler(uv, zi),
				MipMapBilinearSampler(uv, zi + 1), lerpz);
		}
	}
	BITMAPFILEHEADER bmpHeader;
	BITMAPINFOHEADER bmpInfHeader;
	float *bmpData;
	float *mipmap[12];
	int mipBiWidth[12];
	int mipWidth[12];
	int mipHeight[12];
	int mipW[12];
	int mipH[12];
	int mipSize;
	int mipS;
	int width, height;
	int biwidth;
	int w, h;
	float zThreshold = 80;

};

#endif