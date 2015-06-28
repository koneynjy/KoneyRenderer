#include "Texture.hpp"
#include "malloc.h"
using namespace std;
using namespace DirectX;

Texture::Texture() :bmpData(NULL),mipSize(0){};
Texture::Texture(string path){
	Load(path);
}
void Texture::Load(string path){
	FILE *fpBmp;
	if ((fpBmp = fopen(path.c_str(), "rb")) == NULL)
	{
		cout << "the bmp file can not open!" << endl;
		exit(1);
	}

	//read the BITMAPFILEHEADER
	fread(&bmpHeader.bfType, 2, 1, fpBmp);
	fread(&bmpHeader.bfSize, 4, 1, fpBmp);
	fread(&bmpHeader.bfReserved1, 2, 1, fpBmp);
	fread(&bmpHeader.bfReserved2, 2, 1, fpBmp);
	fread(&bmpHeader.bfOffBits, 4, 1, fpBmp);
	fread(&bmpInfHeader.biSize, 4, 1, fpBmp);//结构所需的字节数
	fread(&bmpInfHeader.biWidth, 4, 1, fpBmp);//位图的宽度，以像素为单位
	fread(&bmpInfHeader.biHeight, 4, 1, fpBmp);//位图的高度，以像素为单位
	fread(&bmpInfHeader.biPlanes, 2, 1, fpBmp);//目标设备的平面数，必须为1
	fread(&bmpInfHeader.biBitCount, 2, 1, fpBmp);//一个像素的位数
	fread(&bmpInfHeader.biCompression, 4, 1, fpBmp);//自下而上的压缩的位图的压缩类型，可以是BI_RGB，BI_RLE8，BI_RLE4，BI_BITFIELDS，BI_JPEG
	fread(&bmpInfHeader.biSizeImage, 4, 1, fpBmp);//指定图像的大小，以字节为单位。BI_RGB位图设置为0
	fread(&bmpInfHeader.biXPelsPerMeter, 4, 1, fpBmp);//指定目标设备的位图水平分辨率，以每米像素为单位
	fread(&bmpInfHeader.biYPelsPerMeter, 4, 1, fpBmp);//指定目标设备的位图垂直分辨率，以每米像素为单位
	fread(&bmpInfHeader.biClrUsed, 4, 1, fpBmp);//指定实际应用于位图中的颜色表中的颜色索引数
	fread(&bmpInfHeader.biClrImportant, 4, 1, fpBmp);//指定用于显示位图需要的颜色索引数。若为0，则所有颜色都需要。
	width = bmpInfHeader.biWidth;
	height = bmpInfHeader.biHeight;
	w = width - 1, h = height - 1;
	int size = width * height * 3;
	unsigned char *data = new unsigned char[size];
	fseek(fpBmp, bmpHeader.bfOffBits, 0);
	fread(data, 1, size, fpBmp);
	
	bmpData = (float*)_aligned_malloc((width + 1)  * (height + 1) * 4 * sizeof(float), 16);
	int d1 = 0, d2 = 0, w1 = width * 3, w2 = (width + 1) << 2;
	for (int i = 0; i < height; i++){
		int id1 = d1, id2 = d2;
		for (int j = 0; j < width; j++){
			bmpData[id2    ] = data[id1 + 2] / 255.0f;
			bmpData[id2 + 1] = data[id1 + 1] / 255.0f;
			bmpData[id2 + 2] = data[id1    ] / 255.0f;
			bmpData[id2 + 3] = 0.0f;
			id1 += 3, id2 += 4;
		}
		bmpData[id2    ] = bmpData[id2 - 4];
		bmpData[id2 + 1] = bmpData[id2 - 3];
		bmpData[id2 + 2] = bmpData[id2 - 2];
		bmpData[id2 + 3] = 0.0f;
		d1 += w1;
		d2 += w2;
	}
	int id2 = d2, id1 = id2 - d2;
	for (int j = 0; j < width + 1; j++){
		bmpData[id2    ] = bmpData[id1    ];
		bmpData[id2 + 1] = bmpData[id1 + 1];
		bmpData[id2 + 2] = bmpData[id1 + 2];
		bmpData[id2 + 3] = 0.0f;
		id2 += 4, id1 += 4;
	}
	biwidth = width + 1;

// 	bmpData = (float*)_aligned_malloc(width  * height * 4 * sizeof(float), 16);
// 	int d1 = 0, d2 = 0, w1 = width * 3, w2 = width << 2;
// 	for (int i = 0; i < height; i++){
// 		int id1 = d1, id2 = d2;
// 		for (int j = 0; j < width; j++){
// 			bmpData[id2    ] = data[id1 + 2] / 255.0f;
// 			bmpData[id2 + 1] = data[id1 + 1] / 255.0f;
// 			bmpData[id2 + 2] = data[id1    ] / 255.0f;
// 			bmpData[id2 + 3] = 0.0f;
// 			id1 += 3, id2 += 4;
// 		}
// 		d1 += w1;
// 		d2 += w2;
// 	}
// 	biwidth = width;
	delete[] data;
	GenMipmap();
}

Texture::~Texture(){
	if(bmpData) delete[] bmpData;
	if (mipSize){
		for (int i = 0; i < mipSize; i++)
			delete[] mipmap[i];
	}
}

void Texture::GenMipmap(){
	if (!bmpData) return;
	mipmap[0] = bmpData;
	mipWidth[0] = width;
	mipW[0] = w;
	mipHeight[0] = height;
	mipH[0] = h;
	mipBiWidth[0] = biwidth;
	mipSize = 1;
	int preMip = 0;
	while (1){
		mipWidth[mipSize] = mipWidth[preMip] >> 1;
		mipW[mipSize] = mipWidth[mipSize] - 1;
		mipHeight[mipSize] = mipHeight[preMip] >> 1;
		mipH[mipSize] = mipHeight[mipSize] - 1;
		mipBiWidth[mipSize] = mipWidth[mipSize] + 1;
		mipmap[mipSize] = new float[(mipWidth[mipSize] + 1) * (mipHeight[mipSize] + 1)<< 2];//puls one for bilinear filter
		int dy = mipBiWidth[mipSize] << 2,dx = 4, ybase = 0, x = 0;
		int dyup = mipBiWidth[preMip] << 2;
		for (int i = 0; i < mipHeight[mipSize]; i++){
			for (int j = 0; j < mipWidth[mipSize]; j++){
				int idxup[4];
				idxup[0] = (i << 1) * dyup + (j << 1 << 2);
				idxup[1] = idxup[0] + 4;
				idxup[2] = idxup[0] + dyup;
				idxup[3] = idxup[2] + 4;
				for (int t = 0; t < 4; t++){
					mipmap[mipSize][x + t] = 0.0f;
					for (int k = 0; k < 4; k++){
						mipmap[mipSize][x + t] += mipmap[preMip][idxup[k] + t];
					}
					mipmap[mipSize][x + t] /= 4.0f;
				}
				x += dx;
			}
			for (int t = 0; t < 4; t++){
				mipmap[mipSize][x + t] = mipmap[mipSize][x - dx + t];
			}
			ybase += dy;
			x = ybase;
		}
		for (int j = 0; j < mipBiWidth[mipSize]; j++){
			for (int t = 0; t < 4; t++){
				mipmap[mipSize][x + t] = mipmap[mipSize][x - dy + t];
			}
			x += dx;
		}
		mipSize++;
		preMip = mipSize - 1;
		if (mipWidth[preMip] <= 1 || mipHeight[preMip] <= 1) break;
	}
	mipS = preMip;
}

