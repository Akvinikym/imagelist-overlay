#include "stdafx.h"

#include <Windows.h>
#include <CommCtrl.h>
#include <iostream>
#include <sstream>

#pragma comment(lib, "comctl32.lib")

// device - console in this case - to which the image will be drawn
class ConsoleDC {
	static HWND window_handle_;
	static HDC device_context_;

public:
	ConsoleDC() {
		if (!window_handle_ && !device_context_) {
			window_handle_ = GetConsoleWindow();
			device_context_ = GetDC(window_handle_);
		}
	}
	~ConsoleDC() {
		if (window_handle_ && device_context_) {
			ReleaseDC(window_handle_, device_context_);
			window_handle_ = NULL;
			device_context_ = NULL;
		}
	}
	HDC get() { return device_context_; }
};

HWND ConsoleDC::window_handle_ = NULL;
HDC ConsoleDC::device_context_ = NULL;

int main()
{
	// load comctl
	INITCOMMONCONTROLSEX comctl_init_flags;
	comctl_init_flags.dwICC = ICC_WIN95_CLASSES;
	comctl_init_flags.dwSize = sizeof(comctl_init_flags);
	BOOL comctl_loaded = InitCommonControlsEx(&comctl_init_flags);
	if (!comctl_loaded) {
		printf("Comctl is not loaded: %lu \n", GetLastError());
		return 1;
	}

	// default parameters for images
	int image_size = 64;
	UINT image_flags = ILC_MASK | ILC_COLOR32;

	// default parameters for image lists
	int initial_size = 100;
	int image_list_grow_size = 10;

	// create two initial image lists, to which we are going to put
	// a set of pregenerated images
	HIMAGELIST image_list1 = ImageList_Create(
		image_size,
		image_size,
		image_flags,
		initial_size,
		image_list_grow_size
	);
	if (!image_list1) {
		printf("Cannot create image list 1 %lu \n", GetLastError());
		return 1;
	}
	HIMAGELIST image_list2 = ImageList_Create(
		image_size,
		image_size,
		image_flags,
		initial_size,
		image_list_grow_size
	);
	if (!image_list2) {
		printf("Cannot create image list 2 %lu \n", GetLastError());
		return 1;
	}

	// arrays to hold indices of images - Nth image will not neccesseraly
	// have index N
	int *image_list_indexes1 = new int[initial_size];
	int *image_list_indexes2 = new int[initial_size];

	// fill two image lists with our pregenerated images, such that
	// the first list contains images from 1 to 100, and the second from
	// 101 to 200
	for (int i = 0; i < initial_size; ++i) {
		std::wstring filename1 = L"images\\test" + 
			std::to_wstring(i + 1) + 
			L".bmp";
		std::wstring filename2 = L"images\\test" +
			std::to_wstring(initial_size + i + 1) +
			L".bmp";

		HBITMAP image1 = (HBITMAP)LoadImage(
			NULL,
			filename1.c_str(),
			IMAGE_BITMAP,
			image_size,
			image_size,
			LR_LOADFROMFILE
		);
		if (!image1) {
			printf("Unable to load image1: %lu \n", GetLastError());
			return 1;
		}

		HBITMAP image2 = (HBITMAP)LoadImage(
			NULL,
			filename2.c_str(),
			IMAGE_BITMAP,
			image_size,
			image_size,
			LR_LOADFROMFILE
		);
		if (!image2) {
			printf("Unable to load image2: %lu \n", GetLastError());
			return 1;
		}

		int image1_idx = ImageList_Add(
			image_list1,
			image1,
			NULL
		);
		if (-1 == image1_idx) {
			printf("Cannot add an image to image list: %lu \n", 
				GetLastError());
			return 1;
		}
		image_list_indexes1[i] = image1_idx;

		int image2_idx = ImageList_Add(
			image_list2,
			image2,
			NULL
		);
		if (-1 == image2_idx) {
			printf("Cannot add an image to image list: %lu \n", 
				GetLastError());
			return 1;
		}
		image_list_indexes2[i] = image1_idx;

		BOOL removed = DeleteObject(image1);
		if (!removed) {
			printf("Unable to delete image object: %lu \n", GetLastError());
			return 1;
		}
		removed = DeleteObject(image2);
		if (!removed) {
			printf("Unable to delete image object: %lu \n", GetLastError());
			return 1;
		}
	}

	// initialize the third image list, which is going to contain two images,
	// copied from the two previous image lists
	HIMAGELIST image_list3 = ImageList_Create(
		image_size,
		image_size,
		image_flags,
		initial_size,
		image_list_grow_size
	);
	if (!image_list3) {
		printf("Cannot create image list 3 %lu \n", GetLastError());
		return 1;
	}

	// bitmaps, which are going to hold the copies of images from two lists
	// we can do it with one bitmap, but the code would be less straightforward
	HBITMAP 
		image1_bitmap = CreateBitmap(
			initial_size, 
			initial_size,
			1,
			32,
			NULL
		),
		image2_bitmap = CreateBitmap(
			initial_size,
			initial_size,
			1,
			32,
			NULL
		);

	// copy the images through selecting copies to the decice contexts
	// and drawing on those contexts the desired images
	HDC hdc_to_draw = CreateCompatibleDC(GetDC(NULL));
	HBITMAP bitmap_to_draw = (HBITMAP)SelectObject(hdc_to_draw, image1_bitmap);
	if (!ImageList_Draw(
		image_list1,
		image_list_indexes1[5], // image with number 6
		hdc_to_draw,
		0,
		0,
		ILD_NORMAL
	)) {
		printf("cannot draw the first bitmap: %lu", GetLastError());
		return 1;
	}
	SelectObject(hdc_to_draw, bitmap_to_draw);
	DeleteDC(hdc_to_draw);

	hdc_to_draw = CreateCompatibleDC(GetDC(NULL));
	bitmap_to_draw = (HBITMAP)SelectObject(hdc_to_draw, image2_bitmap);
	if (!ImageList_Draw(
		image_list2,
		image_list_indexes2[5], // image with number 106
		hdc_to_draw,
		0,
		0,
		ILD_NORMAL
	)) {
		printf("cannot draw the second bitmap: %lu", GetLastError());
		return 1;
	}
	SelectObject(hdc_to_draw, bitmap_to_draw);
	DeleteDC(hdc_to_draw);

	// add the copies to the third image list
	int *image_list_indexes3 = new int[5];
	int image_idx = ImageList_Add(
		image_list3,
		image1_bitmap,
		NULL
	);
	if (-1 == image_idx) {
		printf("Cannot add first image to third image list: %lu \n", 
			GetLastError());
		return 1;
	}
	image_list_indexes3[0] = image_idx;
	image_idx = ImageList_Add(
		image_list3,
		image2_bitmap,
		NULL
	);
	if (-1 == image_idx) {
		printf("Cannot add second image to third image list: %lu \n", 
			GetLastError());
		return 1;
	}
	image_list_indexes3[1] = image_idx;

	// create the console, in which the image is to be drawn
	ConsoleDC console;
	HDC console_dc = console.get();
	if (!console_dc) {
		printf("Unable to get console device context: %lu \n", GetLastError());
		return 1;
	}

	// promote the first image to an overlay with index 12
	if (!ImageList_SetOverlayImage(
		image_list3,
		image_list_indexes3[0],
		12
	)) {
		printf("Cannot set overlay image: %lu\n", GetLastError());
		return 1;
	}

	// draw the second image, covered by the first (overlay with index 12); the
	// final image is number 106, covered by 6, and so we see number 106
	if (!ImageList_Draw(
		image_list3,
		image_list_indexes3[1],
		console_dc,
		100,
		100,
		ILD_NORMAL
			| INDEXTOOVERLAYMASK(12) // if you want to see 106, comment this
	)) {
		printf("Unable to draw image: %lu\n", GetLastError());
		return 1;
	}

	// clear the resources we have taken
	if (!ImageList_Destroy(image_list1)) {
		printf("Cannot destroy image list %lu \n", GetLastError());
		return 1;
	}
	if (!ImageList_Destroy(image_list2)) {
		printf("Cannot destroy image list %lu \n", GetLastError());
		return 1;
	}
	if (!ImageList_Destroy(image_list3)) {
		printf("Cannot destroy image list %lu \n", GetLastError());
		return 1;
	}
	if (image_list_indexes1) {
		delete[]image_list_indexes1;
	}
	if (image_list_indexes2) {
		delete[]image_list_indexes2;
	}
	if (image_list_indexes3) {
		delete[]image_list_indexes3;
	}

	return 0;
}
