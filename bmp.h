#pragma once

#ifndef BMP_H
#define BMP_H

#include <stdio.h>
#include <string.h>
#include <iostream>

struct RGB
{
	unsigned char blue = 0xFF;
	unsigned char green = 0xFF;
	unsigned char red = 0xFF;
	RGB(int rgb) {
		red = (rgb & 0xFF0000) >> 16;
		green = (rgb & 0x00FF00) >> 8;
		blue = (rgb & 0x0000FF);
	}
	RGB(unsigned char r, unsigned char g, unsigned char b) {
		red = r;
		green = g;
		blue = b;
	}
	RGB() {

	}
	bool operator==(const RGB& b) {
		if (this->blue == b.blue && this->green == b.green && this->red == b.red)
			return true;
		else
			return false;
	}
};

struct DIB_header {
	unsigned int header_size = 40; // DIB_header's size is determined by its type. 0x0E/14, 32 bits.
	int width; // the bitmap width in pixels (signed integer). 0x12/18, 32 bits.
	int height; // the bitmap height in pixels (signed integer). 0x16/22, 32 bits.
	unsigned short color_planes = 1; // the number of color planes (must be 1). 0x1A/26, 16 bits.
	unsigned short bits_per_pixel = 24; // the number of bits per pixel, which is the color depth of the image. Typical values are 1, 4, 8, 16, 24 and 32. 0x1C/28, 16 bits.
	unsigned int compression_method = 0; // the compression method being used. 0x1E/30, 32 bits. 	
	unsigned int raw_data_size = 0; // the image size. This is the size of the raw bitmap data; a dummy 0 can be given for BI_RGB bitmaps. 0x22/34, 32 bits.
	int h_reso = 5669; // the horizontal resolution of the image. (pixel per metre, signed integer) 0x26/38, 32 bits. 	
	int v_reso = 5669; // the vertical resolution of the image. (pixel per metre, signed integer) 0x2A/42, 32 bits.
	unsigned int colors = 0; // the number of colors in the color palette, or 0 to default to 2^n. 0x2E/46, 32 bits.
	unsigned int important_colors = 0; // the number of important colors used, or 0 when every color is important; generally ignored. 0x32/50, 32 bits.
};

struct BMP_header
{
	// BMP header's size is fixed, which equals to 14 bits. 
	char head[3] = { 'B','M','\0' };
	unsigned int file_size;
	unsigned int reserved_field = 0;
	unsigned int starting_addr = 54;
	unsigned int DIB_header_size = 40;
	DIB_header DIB;
	operator bool() {
		if (strcmp(head, "BM") == 0 || strcmp(head, "BA") == 0 || strcmp(head, "CI") == 0 || strcmp(head, "CP") == 0 || strcmp(head, "IC") == 0 || strcmp(head, "PT") == 0)
			return true;
		else
			return false;
	}
};

BMP_header check_BMP_file(FILE* bmpfile) {
	BMP_header ret;
	fread(ret.head, 1, 2, bmpfile);
	fread(&ret.file_size, 4, 1, bmpfile);
	fread(&ret.reserved_field, 4, 1, bmpfile);
	fread(&ret.starting_addr, 4, 1, bmpfile);
	fread(&ret.DIB_header_size, 4, 1, bmpfile);
	if (ret.DIB_header_size == 40) {
		ret.DIB.header_size = ret.DIB_header_size;
		fread(&ret.DIB.width, 4, 1, bmpfile);
		fread(&ret.DIB.height, 4, 1, bmpfile);
		fread(&ret.DIB.color_planes, 2, 1, bmpfile);
		fread(&ret.DIB.bits_per_pixel, 2, 1, bmpfile);
		fread(&ret.DIB.compression_method, 4, 1, bmpfile);
		fread(&ret.DIB.raw_data_size, 4, 1, bmpfile);
		fread(&ret.DIB.h_reso, 4, 1, bmpfile);
		fread(&ret.DIB.v_reso, 4, 1, bmpfile);
		fread(&ret.DIB.colors, 4, 1, bmpfile);
		fread(&ret.DIB.important_colors, 4, 1, bmpfile);
	}
	rewind(bmpfile);
	return ret;
}

int get_row_size(BMP_header h) {
	return ((h.DIB.bits_per_pixel*h.DIB.width + 31) / 32) * 4;
}

int get_data_size(BMP_header h) {
	return get_row_size(h)*abs(h.DIB.height);
}

struct Point {
	int x = 0;
	int y = 0;
	RGB color;
	Point(int x, int y, RGB rgb) : x(x), y(y), color(rgb) { }
	Point() {}
};

class BMP{
	FILE* bmpfile;
	int row_size;
public:
	BMP_header header;
	const int& width = header.DIB.width;
	const int& height = header.DIB.height;
	RGB* data;
	BMP(int width, int height, const char* path) {
		header.DIB.width = width;
		header.DIB.height = height;
		row_size = ((24 * width + 31) / 32) * 4;
		header.DIB.raw_data_size = height * row_size;
		header.file_size = 54 + header.DIB.raw_data_size;
		data = new RGB[height*width];
		bmpfile = fopen(path, "wb");
	}
	BMP(const char* path) {
		bmpfile = fopen(path, "rb");
		header = check_BMP_file(bmpfile);
		if (!header) {
			fprintf(stderr, "This file is not a bmp file!");
			exit(1);
		}
		row_size = get_row_size(header);
		int start = header.starting_addr + row_size*(header.DIB.height - 1);
		fseek(bmpfile, start, SEEK_SET);
		data = new RGB[height*width];
		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				fread(&(data[i*width+j]), 3, 1, bmpfile);
			}
			fseek(bmpfile, start - row_size*(i + 1), SEEK_SET);		
		}
	}
	~BMP() {
		fclose(bmpfile);
		delete[] data;
	}
	void print() {
		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				printf("\033[48;2;%d;%d;%dm  \033[0m", data[i*width + j].red, data[i*width + j].green, data[i*width + j].blue);
			}
			printf("\n");
		}
	}
	void write() {
		// header
		fwrite(header.head, 1, 2, bmpfile);
		fwrite(&header.file_size, 4, 1, bmpfile);
		fwrite(&header.reserved_field, 4, 1, bmpfile);
		fwrite(&header.starting_addr, 4, 1, bmpfile);
		fwrite(&header.DIB_header_size, 4, 1, bmpfile);
		fwrite(&header.DIB.width, 4, 1, bmpfile);
		fwrite(&header.DIB.height, 4, 1, bmpfile);
		fwrite(&header.DIB.color_planes, 2, 1, bmpfile);
		fwrite(&header.DIB.bits_per_pixel, 2, 1, bmpfile);
		fwrite(&header.DIB.compression_method, 4, 1, bmpfile);
		fwrite(&header.DIB.raw_data_size, 4, 1, bmpfile);
		fwrite(&header.DIB.h_reso, 4, 1, bmpfile);
		fwrite(&header.DIB.v_reso, 4, 1, bmpfile);
		fwrite(&header.DIB.colors, 4, 1, bmpfile);
		fwrite(&header.DIB.important_colors, 4, 1, bmpfile);

		// raw data!!
		for (int i = height - 1; i >= 0; i--) {
			for (int j = 0; j < width; j++) {
				fwrite(&(data[i*width + j]), 3, 1, bmpfile);
			}
			for (int k = 0; k < (row_size - width * 3); k++) {
				fputc('\0', bmpfile);
			}
		}

		// eof
	}
	RGB* operator[](int sub){
		return &data[sub*width];
	}
	void put_pixel(Point p) {
		// safe first!
		if (p.x < width && p.y < height && p.x >= 0 && p.y >= 0) {
			data[p.x + p.y*width] = p.color;
		}
	}

	void put_pixel(int x, int y, RGB color = RGB()) {
		if (x < width && y < height && x >= 0 && y >= 0) {
			data[x + y*width] = color;
		}
	}

	void line(Point p1, Point p2) {
		double dx = p2.x - p1.x;
		double dy = p2.y - p1.y;
		double dr = p2.color.red - p1.color.red;
		double dg = p2.color.green - p1.color.green;
		double db = p2.color.blue - p1.color.blue;

		// 选择步进方向
		int step = int((abs(dx) >= abs(dy)) ? abs(dx) : abs(dy));
		dx /= step;
		dy /= step;
		dr /= step;
		dg /= step;
		db /= step;

		double x = p1.x;
		double y = p1.y;
		double r = p1.color.red;
		double g = p1.color.green;
		double b = p1.color.blue;

		for (int i = 0; i <= step; i++) {
			this->put_pixel(x, y, RGB(r, g, b));
			x = x + dx;
			y = y + dy;
			r += dr;
			g += dg;
			b += db;
		}
	}
	void circle(Point p, int radius)
	{
		int x = radius - 1;
		int y = 0;
		int dx = 1;
		int dy = 1;
		int err = dx - (radius << 1);

		while (x >= y)
		{
			this->put_pixel(p.x + x, p.y + y, p.color);
			this->put_pixel(p.x + y, p.y + x, p.color);
			this->put_pixel(p.x - y, p.y + x, p.color);
			this->put_pixel(p.x - x, p.y + y, p.color);
			this->put_pixel(p.x - x, p.y - y, p.color);
			this->put_pixel(p.x - y, p.y - x, p.color);
			this->put_pixel(p.x + y, p.y - x, p.color);
			this->put_pixel(p.x + x, p.y - y, p.color);

			if (err <= 0) {
				y++;
				err += dy;
				dy += 2;
			}
			if (err > 0) {
				x--;
				dx += 2;
				err += dx - (radius << 1);
			}
		}
	}

	void fill(Point p) {
		if (p.x < width && p.y < height) {
			RGB origin = (*this)[p.y][p.x];
			fill_(p, origin);
		}
	}

private:
	void fill_(Point p, const RGB& origin) {
		if (p.x < width && p.y < height && data[p.y * width + p.x] == origin) {
			this->put_pixel(p);
			Point up = p;
			Point down = p;
			Point left = p;
			Point right = p;
			up.y--;
			down.y++;
			left.x--;
			right.x++;
			if (up.x < width && up.y < height && up.y >= 0 && up.x >= 0 && data[up.y * width + up.x] == origin) {
				fill_(up, origin);
			}
			if (down.x < width && down.y < height && down.y >= 0 && down.x >= 0 && data[down.y * width + down.x] == origin) {
				fill_(down, origin);
			}
			if (left.x < width && left.y < height && left.y >= 0 && left.x >= 0 && data[left.y * width + left.x] == origin) {
				fill_(left, origin);
			}
			if (right.x < width && right.y < height && right.y >= 0 && right.x >= 0 && data[right.y * width + right.x] == origin) {
				fill_(right, origin);
			}
		}
	}
};

#endif // !BMP_H
