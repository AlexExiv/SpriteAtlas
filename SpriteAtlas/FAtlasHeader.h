#ifndef __FATLAS_HEADER_H__
#define __FATLAS_HEADER_H__


#include "types.h"

#define ATLAS_FILE_TYPE (((UI32)'a') | (((UI32)'s') << 8) || (((UI32)'f') << 16) || (((UI32)0) << 24))
#define SPRITE_ATLAS_VER 1

#define ATLAS_COMPR_NONE 1
#define ATLAS_COMPR_RLE 2
#define ATLAS_COMPR_JPG 3
#define ATLAS_COMPR_PNG 4

#define ATLAS_PACKED 0x00000001
#define ATLAS_RGB 0x00000002
#define ATLAS_ALPHA 0x00000004
#define ATLAS_RGBA 0x00000008

#define ATLAS_TYPE_FLOAT 1
#define ATLAS_TYPE_INT 2
#define ATLAS_TYPE_STR 3

//флаги анимации
#define ATLAS_ANIM_LOOPED 0x00000001 



struct FAtlasHeader
{
	UI32 iFileType;//тип файла должнобыть равно "asf"
	UI32 iVersion;//текущая версия

	union
	{
		struct
		{
			UI32 iAtlasNameOff, iAtlasNameLen;//имя файла содержащее данные текстуры аталаса в RGB или RGBA формате, если данные не запакованы
			UI32 iAtlasAlphaNameOff, iAtlasAlphaNameLen;//имя файла содержащее данные текстуры аталаса в Alpha формате, если данные не запакованы
		};
		struct
		{
			UI32 iRGBDataLen;//длина RGB или RGBA данных в файле, если файл пакован
			UI32 iAlphaDataLen;//длина  Alpha данных в файле, если файл пакован
		};
	};

	UI32 iTexWidth, iTexHeight;//ширина высота текстурного атласа
	UI32 iCompr;//наличие сжатия и его тип
	UI32 iFlags;//флаги

	UI32 iFrameCount;//количество кадров
	UI32 iAnimationCount;//количество анимаций
	UI32 iAddDataCount;//количество заголовков дополнительных данных

	UI32 iFrameDataStart;//начало данных заголовков кадров
	UI32 iAnimationDataStart;//начало данных заголовков анимаций
	UI32 iAdditionalDataStart;//начлао данных заголовков дополнительных данных
	UI32 iTexAtlasDataStart;//начало данных текстурного аталас везде сдвиг относительно начала файла
	UI32 iStringDataStart;//начало строковых данных
};

struct FFrameHeader
{
	UI32 iFrameNum;//номер кадра

	UI32 x, y;//координаты целый начала кадра
	UI32 iWidth, iHeight;//высота и ширина кадра

	F32 u0, v0, u1, v1;//текстурные координаты кадра

	UI32 iNameOff, iNameLen;//начало имени кадра, сдвиг относительно начала файла и длина имени
};

struct FAnimationHeader
{
	UI32 iStartFrame, iEndFrame;//начало и конец анимации
	UI32 iAnimFlags;//флаги анимаций, например цикличность анимации
	UI32 iNameOff, iNameLen;//начало имени анимации, сдвиг относительно начала файла и длина имени
};


struct FAddDataHeader
{
	UI32 iNameOff, iNameLen;//начало имени дополнительно поля, сдвиг относительно начала файле и длина имени
	UI32 iValType;//тип значения хранящегося в поле
	union
	{
		I32 iInt;//целое
		F32 fFloat;//вещественое
		struct//строка начало и длина
		{
			UI32 iStrOff, iStrLen;
		};
	};
};

#endif