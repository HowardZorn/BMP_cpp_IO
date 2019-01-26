#define _CRT_SECURE_NO_WARNINGS
#include <string>
#include <iostream>
#include <iomanip>
#include "bmp.h"
using namespace std;


int main() {
	printf("Generating sample.bmp. . .\n");
	BMP bmp(77, 77, "sample.bmp");
	bmp.line(Point(8, 8, 0), Point(52, 8, 0));
	bmp.line(Point(8, 8, 0), Point(8, 52, 0));
	bmp.line(Point(8, 52, 0), Point(52, 52, 0));
	bmp.line(Point(52, 8, 0), Point(52, 52, 0));
	bmp.line(Point(23, 23, 0), Point(23, 67, 0));
	bmp.line(Point(23, 23, 0), Point(67, 23, 0));
	bmp.line(Point(23, 67, 0), Point(67, 67, 0));
	bmp.line(Point(67, 23, 0), Point(67, 67, 0));
	bmp.fill(Point(18, 18, 0xFF0000));
	bmp.fill(Point(40, 40, 0xFFF200));
	bmp.fill(Point(60, 60, RGB(181, 230, 29)));
	bmp.circle(Point(23, 52, 0x000000), 10);
	bmp.print();
	bmp.write();
}
