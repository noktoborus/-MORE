// gcc -o any2tga -lm $0
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

#define TGA_STATE_ERROR	1
#define TGA_STATE_HEAD	(1 << 1)
#define TGA_STATE_ID	(1 << 2)
#define TGA_STATE_CMAP	(1 << 3)
#define TGA_STATE_PIXEL	(1 << 4)
#define TGA_STATE_GET(X)	(X->_private.state)
struct TGAHeader_t
{
	uint8_t idLength; /* Identification field length (after header) */
	uint8_t colormapType;
		/*
		       0		No color map
			   1		Present
			   2-127	reserved by Tryevision
			   128-255	for developer use
		*/
	uint8_t imageType;
		/*
		   allowed values for ImageType:
				0	No image data is present,
				1	Color-mapped image,
				2	True-color image,
				3	Black-and-white image,
				|8	RLE-compressed
		*/
	/* 3 bytes */
	struct 
	{
		uint16_t offsetto;	/* offset into the color map table */
		uint16_t number;	/* number of entries */
		uint8_t bpp;		/* bits per pixel */
	} colormap;
	/* 7 bytes */
	struct
	{
		uint16_t origin[2]; /* [X, Y]-axis origin */
		uint16_t size[2]; /* [X, Y]-axis size of image */
		uint8_t depth; /* bits per pixel in image */
		uint8_t descript; /* TODO: XYNTA */
	} imageSpec;
	/* 17 bytes */
	struct
	{
		uint8_t *id;
		uint8_t *colormap;
		uint32_t *pixel;
	} p; /* pointers :3 */
	struct
	{
		int state;
		unsigned int offset; /* offset for stream feel */
	} _private;
};
#define TGA_HEADER_SIZE 17

/* TODO: detect byteorder */
#define TGA_BIG_ENDIAN		1
#define TGA_LITTLE_ENDIAN	2
#define TGA_ENDIAN TGA_LITTLE_ENDIAN

void
tgaUnpack (const uint8_t *bin, size_t len, struct TGAHeader_t *hout)
{
	size_t i = 0;
	/* TODO: complite stream unpack */
	do
	{
		switch (i)
		{
			case 0:
				hout->idLength = bin[i];
				break;
			case 1:
				hout->colormapType = bin[i];
				break;
			case 2:
				hout->imageType = bin[i];
				break;
			case 3:
			case 4:
#if TGA_ENDIAN == TGA_LITTLE_ENDIAN
				((uint8_t*)&hout->colormap.offsetto)[i % 3] = bin[i];
#elif TGA_ENDIAN == TGA_BIG_ENDIAN
				((uint8_t*)&hout->colormap.offsetto)[i % 4] = bin[i];
#endif
				break;
			case 5:
			case 6:
#if TGA_ENDIAN == TGA_LITTLE_ENDIAN
				((uint8_t*)&hout->colormap.number)[i % 3] = bin[i];
#elif TGA_ENDIAN == TGA_BIG_ENDIAN
				((uint8_t*)&hout->colormap.number)[i % 4] = bin[i];
#endif
				break;
			case 7:
				hout->colormap.bpp = bin[i];
				break;
			case 8:
			case 9:
			case 10:
			case 11:
#if TGA_ENDIAN == TGA_LITTLE_ENDIAN
				((uint8_t*)&hout->imageSpec.origin[(i & 2) >> 1])[i & 1] =
						bin[i];
#elif TGA_ENDIAN == TGA_BIG_ENDIAN
				((uint8_t*)&hout->imageSpec.origin[(i & 2) >> 1])[i & 1 ^ 1] =
						bin[i];
#endif
				break;
			case 12:
			case 13:
			case 14:
			case 15:
#if TGA_ENDIAN == TGA_LITTLE_ENDIAN
				((uint8_t*)&hout->imageSpec.size[(i & 2) >> 1])[i & 1] =
						bin[i];
#elif TGA_ENDIAN == TGA_BIG_ENDIAN
				((uint8_t*)&hout->imageSpec.size[(i & 2) >> 1])[i & 1 ^ 1] =
						bin[i];
#endif
				break;
			case 16:
				hout->imageSpec.depth = bin[i];
				break;
			case 17:
				hout->imageSpec.descript = bin[i];
				if (hout->imageSpec.descript & 16) /* bit 4 must set to 0 */
					hout->_private.state |= TGA_STATE_ERROR;
				hout->_private.state |= TGA_STATE_HEAD;
				break;
			default:
				break;
		}
	}
	while (++i < len);
	/* TODO: unpack id, colormap, image */
}

void
tgaPack (uint8_t *bout, size_t *_offset, size_t len, struct TGAHeader_t *tga)
{
	size_t i = 0;
	size_t offset = 0;

	if(_offset)
		offset = *_offset;
	if (!len)
		return;

	do
	{
		switch (i)
		{
			case 0:
				bout[i] = tga->idLength;
				break;
			case 1:
				bout[i] = tga->colormapType;
				break;
			case 2:
				bout[i] = tga->imageType;
				break;
			case 3:
			case 4:
#if TGA_ENDIAN == TGA_LITTLE_ENDIAN
				bout[i] = ((uint8_t*)&tga->colormap.offsetto)[i % 3];
#elif TGA_ENDIAN == TGA_BIG_ENDIAN
				bout[i] = ((uint8_t*)&tga->colormap.offsetto)[i % 4];
#endif
				break;
			case 5:
			case 6:
#if TGA_ENDIAN == TGA_LITTLE_ENDIAN
				bout[i] = ((uint8_t*)&tga->colormap.number)[i % 3];
#elif TGA_ENDIAN == TGA_BIG_ENDIAN
				bout[i] = ((uint8_t*)&tga->colormap.number)[i % 4];
#endif
				break;
			case 7:
				bout[i] = tga->colormap.bpp;
				break;
			case 8:
			case 9:
			case 10:
			case 11:
#if TGA_ENDIAN == TGA_LITTLE_ENDIAN
				bout[i] =
					((uint8_t*)&tga->imageSpec.origin[(i & 2) >> 1])[i & 1];
#elif TGA_ENDIAN == TGA_BIG_ENDIAN
				bout[i] =
					((uint8_t*)&tga->imageSpec.origin[(i & 2) >> 1])[i & 1 ^ 1];
#endif
				break;
			case 12:
			case 13:
			case 14:
			case 15:
#if TGA_ENDIAN == TGA_LITTLE_ENDIAN
				bout[i] =
					((uint8_t*)&tga->imageSpec.size[(i & 2) >> 1])[i & 1];
#elif TGA_ENDIAN == TGA_BIG_ENDIAN
				bout[i] =
					((uint8_t*)&tga->imageSpec.size[(i & 2) >> 1])[i & 1 ^ 1];
#endif
				break;
			case 16:
				bout[i] = tga->imageSpec.depth;
				break;
			case 17:
				bout[i] = tga->imageSpec.descript;
				break;
			default:
				break;
		}
	}
	while (i++ < len);
	/* TODO: pack id, colormap, image */
}

void
tgaPrintHeader (struct TGAHeader_t *tga, char *head)
{
	fprintf (stderr, "TGA -> %s\n", head);
	fprintf (stderr, "\tID length: %d\n", tga->idLength);
	fprintf (stderr, "\tColor map type: %d\n", tga->colormapType);
	fprintf (stderr, "\tImage type: %d\n", tga->imageType);
	fprintf (stderr, "\tColor map ->\n");
	fprintf (stderr, "\t\toffset: %d\n", tga->colormap.offsetto);
	fprintf (stderr, "\t\tnumber: %d\n", tga->colormap.number);
	fprintf (stderr, "\t\tbpp: %d\n", tga->colormap.bpp);
	fprintf (stderr, "\tImage ->\n");
	fprintf (stderr, "\t\torigin: (%d, %d)\n", tga->imageSpec.origin[0],
			tga->imageSpec.origin[1]);
	fprintf (stderr, "\t\tsize: (%d, %d)\n", tga->imageSpec.size[0],
			tga->imageSpec.size[1]);
	fprintf (stderr, "\t\tdepth: %d\n", tga->imageSpec.depth);
	fprintf (stderr, "\t\tdescription: %d\n", tga->imageSpec.descript);
}

/* ################### */
struct TGAHeader_t ntga =
{
	0,
	0,
	2,
	{
		0,
		0,
		0,
	},
	{
		{0, 0},
		{0, 0},
		32,
		0,
	},
	{
		NULL,
		NULL,
		NULL,
	},
	{
		0,
		0,
	},
};

void
sub1 (FILE *fin, FILE *fout)
{
	size_t cread = 0;
	size_t cnum = 0;
	size_t pixcc = 0;
	uint8_t rbuf[1024];
	uint8_t cbuf[2048];
	size_t ccread;
	uint8_t lastc = 0;
	size_t ite = 0;

	fprintf (stderr, "begin\n");
	if (!fwrite (&ntga, TGA_HEADER_SIZE, 1, fout))
	{
		fprintf (stderr, "init write error\n");
		return;
	}
	while ((cread = fread (rbuf, sizeof (uint8_t), 1024, fin)))
	{
		ccread = 0;
		cnum = 0;
		do
		{
			ite++;
			cbuf[cnum++] = (rbuf[ccread] % 127) +128;
			lastc = rbuf[ccread] & 0xf0;
			if (!(ite & 3))
			{
				cbuf[cnum++] = 0xff; /* skeep alpha */
				pixcc++;
				ite++;
			}
		}
		while (++ccread < cread);
		fwrite (cbuf, sizeof (uint8_t), cnum, fout);
	}
	// feel values
	ntga.imageSpec.size[0] = sqrt ((double)pixcc);
	ntga.imageSpec.size[1] = ntga.imageSpec.size[0];
	// pack header
	tgaPack (rbuf, NULL, TGA_HEADER_SIZE, &ntga);
	// rewind pos
	fseek (fout, 0, SEEK_SET);
	if (!fwrite (&rbuf, TGA_HEADER_SIZE, 1, fout))
	{
		fprintf (stderr, "finilize error\n");
		return;
	}
	tgaPrintHeader (&ntga, "in");
}

int
main (int argc, char *argv[])
{
	FILE *fin = NULL;
	FILE *fout = NULL;

	if (argc < 3)
	{
		fprintf (stderr, "Usage:\n\t%s in_file out_file.tga\n", argv[0]);
		return 1;
	}

	fin = fopen (argv[1], "rb");
	if (fin)
	{
		fout = fopen (argv[2], "wb");
		if (fout)
		{
			sub1 (fin, fout);
			fclose (fout);
		}
		else
			return 2;
		fclose (fin);
	}
	else
		return 1;
	return 0;
}

