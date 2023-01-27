/*************************************************************************/
/*  image_loader_avif.cpp                                                */
/*************************************************************************/
/* Copyright (c) 2020-2022 Fabio Alessandrell, Maffle LLC.               */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include "image_loader_avif.h"

#include <string.h>

#include "godot_cpp/classes/os.hpp"

using namespace godot;

Error ImageLoaderAVIF::avif_load_image_from_buffer(Image *p_image, const uint8_t *p_buffer, int p_buffer_len, avifDecoder *p_decoder) {
	avifRGBImage rgb;
	memset(&rgb, 0, sizeof(rgb));
	avifResult result = avifDecoderSetIOMemory(p_decoder, p_buffer, p_buffer_len);
	ERR_FAIL_COND_V_MSG(result != AVIF_RESULT_OK, FAILED, avifResultToString(result));
	result = avifDecoderParse(p_decoder);

	ERR_FAIL_COND_V_MSG(result != AVIF_RESULT_OK, FAILED, avifResultToString(result));
	result = avifDecoderNextImage(p_decoder);
	ERR_FAIL_COND_V_MSG(result != AVIF_RESULT_OK, FAILED, avifResultToString(result));

	// Now available (for this frame):
	// * All decoder->image YUV pixel data (yuvFormat, yuvPlanes, yuvRange, yuvChromaSamplePosition, yuvRowBytes)
	// * decoder->image alpha data (alphaRange, alphaPlane, alphaRowBytes)
	// * this frame's sequence timing
	avifRGBImageSetDefaults(&rgb, p_decoder->image);
	rgb.format = AVIF_RGB_FORMAT_RGBA;
	rgb.depth = 8;
	// Override YUV(A)->RGB(A) defaults here: depth, format, chromaUpsampling, ignoreAlpha, libYUVUsage, etc

	const int height = p_decoder->image->height;
	const int width = p_decoder->image->width;
	// Alternative: set rgb.pixels and rgb.rowBytes yourself, which should match your chosen rgb.format
	// Be sure to use uint16_t* instead of uint8_t* for rgb.pixels/rgb.rowBytes if (rgb.depth > 8)
	PackedByteArray data;
	data.resize(width * height * avifRGBImagePixelSize(&rgb));
	rgb.rowBytes = rgb.width * avifRGBImagePixelSize(&rgb);
	rgb.pixels = data.ptrw();

	result = avifImageYUVToRGB(p_decoder->image, &rgb);
	ERR_FAIL_COND_V_MSG(result != AVIF_RESULT_OK, FAILED, avifResultToString(result));

	p_image->set_data(width, height, false, Image::FORMAT_RGBA8, data);
	return OK;
}

Error ImageLoaderAVIF::_load_image(const Ref<Image> &p_image, const Ref<FileAccess> &p_file, BitField<ImageFormatLoader::LoaderFlags> p_flags, double p_scale) {
	ERR_FAIL_COND_V(p_file.is_null() || !p_file->is_open(), ERR_INVALID_PARAMETER);

	int src_image_len = p_file->get_length();
	ERR_FAIL_COND_V(src_image_len == 0, ERR_FILE_CORRUPT);

	PackedByteArray src_image = p_file->get_buffer(src_image_len);

	avifDecoder *decoder = avifDecoderCreate();
	godot::Error err = ImageLoaderAVIF::avif_load_image_from_buffer(const_cast<Image *>(p_image.ptr()), src_image.ptr(), src_image_len, decoder);
	avifDecoderDestroy(decoder);

	return err;
}

PackedStringArray ImageLoaderAVIF::_get_recognized_extensions() const {
	PackedStringArray psa;
	psa.push_back("avif");
	return psa;
}

Ref<Image> ImageLoaderAVIF::_avif_mem_loader_func(const uint8_t *p_buffer, int p_size) {
	Ref<Image> img;
	img.instantiate();
	avifDecoder *decoder = avifDecoderCreate();
	Error err = ImageLoaderAVIF::avif_load_image_from_buffer(img.ptr(), p_buffer, p_size, decoder);
	ERR_FAIL_COND_V(err, Ref<Image>());
	avifDecoderDestroy(decoder);
	return img;
}

ImageLoaderAVIF::ImageLoaderAVIF() {
}
