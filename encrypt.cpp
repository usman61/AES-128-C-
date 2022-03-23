#include "opencv2\opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <time.h>
using namespace std;
using namespace cv;


void MulXor(unsigned char *op);
void display(unsigned char text[4][4]);
void GenerateKey(unsigned char key[4][4], unsigned char word[4][4], unsigned char rcon);
void MatCopy(unsigned char s1[4][4], unsigned char s2[4][4]);

void SubstituteByte(unsigned char state[4][4]);
void ShiftRows(unsigned char state[4][4]);
void MixColumns(unsigned char state[4][4]);
void AddRoundKey(unsigned char text[4][4], unsigned char key[4][4]);

unsigned char table[][17] = {
	{ 0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b ,0xfe, 0xd7, 0xab, 0x76 },
	{ 0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0 },
	{ 0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15 },
	{ 0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75 },
	{ 0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84 },
	{ 0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf },
	{ 0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8 },
	{ 0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2 },
	{ 0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73 },
	{ 0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb },
	{ 0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79 },
	{ 0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08 },
	{ 0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a },
	{ 0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e },
	{ 0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf },
	{ 0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16 }
};
unsigned char mixCol[][4] = {
	{ 0x2, 0x3, 0x1, 0x1 },
	{ 0x1, 0x2, 0x3, 0x1 },
	{ 0x1, 0x1, 0x2, 0x3 },
	{ 0x3, 0x1, 0x1, 0x2 }
};

int main() {
	clock_t start, end;
	double cpu_time_used;
	start = clock();
	// This function will read the image in gray scale mode
	Mat data = imread("lena2.png", IMREAD_GRAYSCALE); 
	unsigned char state[4][4],encrypt[256][256];
	int rounds;
/*	unsigned char state[][4] = {
		{ 0x01, 0x89, 0xfe, 0x76 },
		{ 0x23, 0xab, 0xdc, 0x54 },
		{ 0x45, 0xcd, 0xba, 0x32 },
		{ 0x67, 0xef, 0x98, 0x10 }
	};
	*/

	unsigned char words[44][4] = {
		{ 0x0f, 0x47, 0x0c, 0xaf },
		{ 0x15, 0xd9, 0xb7, 0x7f },
		{ 0x71, 0xe8, 0xad, 0x67 },
		{ 0xc9, 0x59, 0xd6, 0x98 }
	};
	/*
	unsigned char state[][4]={
	{0x32, 0x88, 0x31, 0xe0},
	{0x43, 0x5a, 0x31, 0x37},
	{0xf6, 0x30, 0x98, 0x07},
	{0xa8, 0x8d, 0xa2, 0x34}
	};

	unsigned char words[44][4]={
	{0x2b, 0x28, 0xab, 0x09},
	{0x7e, 0xae, 0xf7, 0xcf},
	{0x15, 0xd2, 0x15, 0x4f},
	{0x16, 0xa6, 0x88, 0x3c}
	};

	*/
	
	int inc, i,j;
	unsigned char hex[256],hinc=0x00;
	unsigned char rcon[10] = { 0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36 };
	int blockno=0;
	for (i = 0; i < 256; i++)
	{
		hex[i] = hinc;
		hinc = hinc + 0x01;
	}
	// Generate Words
	for (i = 1; i <= 10; i++)
		GenerateKey(words + (i * 4) - 4, words + (i * 4), rcon[i - 1]);
	// Converting Image value to hexadecimal
	
	int x, y, a, b;
	for (i = 0; i < data.rows; i+=4){
		for (j = 0; j <data.cols; j+=4) {
			x = i;
			for (a = 0; a < 4; a++) {
				y = j;
				for (b = 0; b < 4; b++) {
					state[a][b] = hex[data.at<uint8_t>(x, y)];
					y++;
				}
				x++;
			}
			x = i;
	// Starting Add Round Key
//	printf("Start of Round\n");
	AddRoundKey(state, words);
//	display(state);
	inc = 0;
	for (rounds = 1; rounds <= 10; rounds++) {
		// Step:1 Substtitue Bytes

		SubstituteByte(state);
	//	printf("After Sub Bytes\n");
	//	display(state);

		// Step:2 Shift Rows
		ShiftRows(state);
	//	printf("After Shift Rows\n");
	//	display(state);

		// Step: 3 Mix Columns
		if (rounds != 10)
			MixColumns(state);
	//	printf("After Mix Columns\n");
	//	display(state);

		// Step 4 Add round Key
		inc += 4;
		AddRoundKey(state, words + inc);
	//	printf("After Round Key\n");
	//	display(state);
      }
	

	for (a = 0; a < 4; a++) {
		y = j;
		for (b = 0; b < 4; b++) {
			encrypt[x][y]=state[a][b];
			y++;
		}
		x++;
	}
	blockno++;
	printf("Block no %d Encrypted\n", blockno);
		}
	}
	Mat mat1(data.rows, data.cols, CV_8UC1,encrypt);
	imwrite("LenaEnc.png", mat1);

	//for (i = 0; i < data.rows; i++) {
	//	for (j = 0; j < data.cols; j++) {
	////		printf("%X ", encrypt[i][j]);
	//		printf("%X ", data.at<uint8_t>(i, j));
	//	}
	//	printf("\n");
	//}
	end = clock();
	cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
	printf("Execution Time %lf", cpu_time_used);

		waitKey();
  }


void MulXor(unsigned char *op) {
	unsigned char sh = *op >> 7;
	if (sh == 1) {
		*op = *op << 1;
		*op = *op ^ 0x1B;
	}
	else
		*op = *op << 1;
}

void AddRoundKey(unsigned char text[4][4], unsigned char key[4][4]) {
	int i, j;
	for (i = 0; i<4; i++) {
		for (j = 0; j<4; j++) {
			text[i][j] = text[i][j] ^ key[i][j];
		}
	}

}
void GenerateKey(unsigned char key[4][4], unsigned char word[4][4], unsigned char rcon) {
	unsigned char col[4];
	int i, r, c;
	//MatCopy(key,key2);

	col[0] = key[1][3];
	col[1] = key[2][3];
	col[2] = key[3][3];
	col[3] = key[0][3];
	for (i = 0; i<4; i++) {
		r = col[i] / 0x10;
		c = col[i] % 0x10;
		col[i] = table[r][c];
	}
	col[0] = col[0] ^ rcon;
	for (i = 0; i<4; i++) {
		word[i][0] = col[i] ^ key[i][0];
	}
	for (i = 0; i<4; i++) {
		word[i][1] = word[i][0] ^ key[i][1];
	}

	for (i = 0; i<4; i++) {
		word[i][2] = word[i][1] ^ key[i][2];
	}

	for (i = 0; i<4; i++) {
		word[i][3] = word[i][2] ^ key[i][3];
	}
//	printf("Mission Complete\n");
	//  display(word);
}
void display(unsigned char text[4][4]) {
	int i, j;
	for (i = 0; i<4; i++) {
		for (j = 0; j<4; j++) {
			printf("%X ", text[i][j]);
		}
		printf("\n");
	}
	printf("\n");
}
void MatCopy(unsigned char s1[4][4], unsigned char s2[4][4]) {
	int i, j;
	for (i = 0; i<4; i++) {
		for (j = 0; j<4; j++) {
			s2[i][j] = s1[i][j];
		}
	}
}

void SubstituteByte(unsigned char state[4][4]) {
	int i, j;
	unsigned char r, c;
	for (i = 0; i<4; i++) {
		for (j = 0; j<4; j++) {
			r = state[i][j] / 0x10;
			c = state[i][j] % 0x10;
			state[i][j] = table[r][c];
		}
	}
}
void ShiftRows(unsigned char state[4][4]) {
	int i, j;
	unsigned char temp1, temp2;
	for (i = 1; i<4; i++) {
		for (j = 1; j <= i; j++) {
			temp1 = state[i][3];
			temp2 = state[i][2];

			state[i][3] = state[i][0];
			state[i][2] = temp1;

			temp1 = state[i][1];
			state[i][1] = temp2;
			state[i][0] = temp1;

		}
	}
}
void MixColumns(unsigned char state[4][4]) {
	int i, j, k;
	unsigned char tt, op, sum = 0x0;
	unsigned char state2[4][4];

	for (i = 0; i<4; i++) {
		for (j = 0; j<4; j++) {
			for (k = 0; k<4; k++) {
				tt = mixCol[i][k];
				op = state[k][j];
				if (tt == 1)
					sum = sum^op;
				else if (tt == 2) {
					MulXor(&op);
					sum = sum^op;
				}
				else {
					MulXor(&op);
					sum = sum^op^state[k][j];

				}
			}
			state2[i][j] = sum;
			sum = 0;
		}
	}
	MatCopy(state2, state);
}


