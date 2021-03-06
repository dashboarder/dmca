#include <unittest.h>
#include <lib/paint.h>
#include <stdlib.h>
#include <string.h>
#include <lib/mib.h>
#include <drivers/display.h>

#define LINEAR_STRIDE_ALIGNMENT         64
#define LINEAR_STRIDE_ALIGNMENT_MASK    (LINEAR_STRIDE_ALIGNMENT - 1)
#define MAX_FLATTENED_HD_SIZE 0x1000

struct test_paint_data {
	struct display_window w;
	addr_t fb_base;
	size_t fb_size;
	enum colorspace color;
	uint32_t x;
       	uint32_t y;
       	uint32_t width;
       	uint32_t height;
	addr_t scratch_base;
	size_t scratch_size;
};

//Test Data
struct test_paint_data test_CS_RGB888_10_x_10 = {
	.fb_size = 0,
	.color = CS_RGB888,
	.x = 0,
       	.y = 0,
       	.width = 10,
       	.height = 10,
	.scratch_base = 0,
	.scratch_size = 0,
};


//BitMaps
uint8_t CS_RGB888_10_x_10_blue[] =
{
	//10 pixels, stride 64 = 1 horizontal row
	//row 1
	//B    G    R      A
	0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 
	0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 
	0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	//row 2
	0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 
	0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 
	0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	//row 3
	0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 
	0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 
	0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	//row 4
	0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 
	0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 
	0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	//row 5
	0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 
	0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 
	0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	//row 6
	0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 
	0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 
	0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	//row 7
	0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 
	0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 
	0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	//row 8
	0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 
	0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 
	0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	//row 9
	0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 
	0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 
	0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	//row 10
	0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 
	0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 
	0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
};

void test_suite_init_paint(void)
{
}

void test_paint_init_with_data(struct test_paint_data *data)
{
	TEST_ASSERT_NOT_NULL(data);

	uint32_t stridelen = data->width;
	uint32_t depth;

	switch (data->color) {
		case CS_RGB565 :
			depth = 16;
			break;

		case CS_RGB888 :
			depth = 32;
			break;

		default :
			depth = 0;
			break;
	}

	data->w.active = false;
	data->w.cs = data->color;
	data->w.depth = depth;
	data->w.pos_x = data->x;
	data->w.pos_y = data->y;
	data->w.width = data->width;
	data->w.height = data->height;
	stridelen = (stridelen * depth) / 8;
	stridelen = (stridelen + LINEAR_STRIDE_ALIGNMENT_MASK) & ~LINEAR_STRIDE_ALIGNMENT_MASK;
	data->w.stride = stridelen;
	stridelen = stridelen / (depth/8);
	//Minimum is a page
	data->fb_size = (data->w.height * data->w.stride + 0xFFF) & ~0xFFF;
	data->fb_base = (addr_t) malloc(data->fb_size);
	TEST_ASSERT_NOT_NULL(data->fb_base);

	if (data->fb_base) bzero((void *)data->fb_base, data->fb_size);
	set_canvas(&(data->w.c), (void *)data->fb_base, data->fb_size, data->width, data->height, stridelen, data->color);

	if (data->scratch_size == 0) {
		//if not specified size, the assume fb_size and allocate another buffer + MAX_FLATTENED_HD_SIZE
		data->scratch_size = data->fb_size + MAX_FLATTENED_HD_SIZE;
	}
	data->scratch_base = (addr_t) malloc(data->scratch_size); 
	bzero((void *)data->scratch_base, data->scratch_size);
	TEST_ASSERT_EQ(paint_init((void *)&(data->w), data->scratch_base, data->scratch_size), 0);
}

void test_paint_cleanup_with_data(struct test_paint_data *data)
{
	TEST_ASSERT_NOT_NULL(data);
	TEST_ASSERT_NOT_NULL(data->fb_base);
	TEST_ASSERT_NOT_NULL(data->scratch_base);

	free((void *)data->fb_base);
	free((void *)data->scratch_base);
}

static void hexdump(const void *_ptr, size_t len)
{
	const unsigned char *ptr = _ptr;
	unsigned int i;

	printf("hexdump ptr %p, len %zd\n", ptr, len);
	
	while (len > 0) {
		printf("%p: ", ptr);
		for (i=0; i < __min(len, 16u); i++) {
			printf("%02x ", *ptr);
			ptr++;
		}
		printf("\n");
		if (len < 16)
			break;
		len -= 16;
	}
}

static uint32_t picture_scale_value = 2;
static uint32_t picture_rotate_value = 0;
static size_t page_size = 4 * 1024;
MIB_VARIABLE(kMIBTargetOsPictureScale, kOIDTypeInt32, picture_scale_value);
MIB_VARIABLE(kMIBTargetPictureRotate, kOIDTypeInt32, picture_rotate_value);
MIB_VARIABLE(kMIBPlatformPageSize, kOIDTypeSize, page_size);

void test_paint_CS_RGB888_blue(uintptr_t param)
{
	struct test_paint_data * data;
	data = &test_CS_RGB888_10_x_10;
	test_paint_init_with_data(data);
	size_t size_to_compare = data->w.height * data->w.stride;
	size_t size_blue_bitmap = sizeof(CS_RGB888_10_x_10_blue)/sizeof(CS_RGB888_10_x_10_blue[0]);

	TEST_ASSERT_LTE(size_to_compare, size_blue_bitmap);
	paint_set_bgcolor(0, 0, 0xff);
	paint_update_image();
	//hexdump((void *)CS_RGB888_10_x_10_blue, size_blue_bitmap);
	//hexdump((void *)data->fb_base, data->fb_size);
	TEST_ASSERT_MEM_EQ(CS_RGB888_10_x_10_blue, data->fb_base, size_to_compare);
	test_paint_cleanup_with_data(data);
}

static struct test_suite paint_test_suite = {
	.name = "paint",
	.description = "tests the paint library",
	.setup_function = test_suite_init_paint,
	.test_cases = {
		{"paint_CS_RGB888_blue", test_paint_CS_RGB888_blue, 0 },
		TEST_CASE_LAST
	}
};

TEST_SUITE(paint_test_suite);
