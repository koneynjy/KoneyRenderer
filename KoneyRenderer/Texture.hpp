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
	__forceinline DirectX::XMFLOAT4 NearestSampler(DirectX::XMFLOAT2& uv){
		int wi = w * uv.x, he = h * uv.y;
		return *((DirectX::XMFLOAT4*)(bmpData) + biwidth * he + wi);
	}

	__forceinline DirectX::XMFLOAT4 BilinearSampler(DirectX::XMFLOAT2& uv){
		float wf = w * uv.x, hf = h * uv.y;
		int wi = wf, hi = hf;
		float lerpx = wf - wi, lerpy = hf - hi;
		DirectX::XMFLOAT4* pd = (DirectX::XMFLOAT4*)(bmpData) + biwidth * hi + wi, *pu = pd + biwidth;
		DirectX::XMFLOAT4 ret;
		XMStoreFloat4(&ret, DirectX::XMVectorLerp(
			DirectX::XMVectorLerp(XMLoadFloat4(pd), XMLoadFloat4(pd + 1), lerpx),
			DirectX::XMVectorLerp(XMLoadFloat4(pu), XMLoadFloat4(pu + 1), lerpx),
			lerpy));
		return ret;
	}

	__forceinline DirectX::XMFLOAT4 MipMapNearestSampler(DirectX::XMFLOAT2& uv, int z){
		int wi = mipW[z] * uv.x, he = mipH[z] * uv.y;
		return *((DirectX::XMFLOAT4*)(mipmap[z])+mipBiWidth[z] * he + wi);
	}

	__forceinline DirectX::XMFLOAT4 MipMapNearestSampler(DirectX::XMFLOAT2& uv, float z){
		int zz = z / zThreshold * mipS;
		zz = zz > mipS ? mipS : zz;
		return MipMapNearestSampler(uv, zz);
	}

	__forceinline DirectX::XMVECTOR MipMapBilinearSampler(DirectX::XMFLOAT2& uv, int z){
		float wf = mipW[z] * uv.x, hf = mipH[z] * uv.y;
		int wi = wf, hi = hf;
		float lerpx = wf - wi, lerpy = hf - hi;
		DirectX::XMFLOAT4* pd = (DirectX::XMFLOAT4*)(mipmap[z]) + mipBiWidth[z] * hi + wi, *pu = pd + mipBiWidth[z];
		DirectX::XMFLOAT4 ret;
		return DirectX::XMVectorLerp(
			DirectX::XMVectorLerp(XMLoadFloat4(pd), XMLoadFloat4(pd + 1), lerpx),
			DirectX::XMVectorLerp(XMLoadFloat4(pu), XMLoadFloat4(pu + 1), lerpx),
			lerpy);
	}

	__forceinline DirectX::XMVECTOR MipMapBilinearSampler(DirectX::XMFLOAT2& uv, float z){
		int zz = z / zThreshold * mipS;
		zz = zz > mipS ? mipS : zz;
		return MipMapBilinearSampler(uv, zz);
	}


	DirectX::XMFLOAT4 TrilinearSampler(DirectX::XMFLOAT2& uv, float z){
		float zz = z / zThreshold * mipS;
		int zi = zz;
		DirectX::XMFLOAT4 ret;
		if (zi == mipS) XMStoreFloat4(&ret, MipMapBilinearSampler(uv, zi));
		else{
			float lerpz = zz - zi;
			XMStoreFloat4(&ret, DirectX::XMVectorLerp(
				MipMapBilinearSampler(uv, zi),
				MipMapBilinearSampler(uv, zi + 1), lerpz));
		}
		return ret;
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