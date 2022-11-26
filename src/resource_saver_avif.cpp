/*************************************************************************/
/*  resource_saver_avif.cpp                                              */
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

#include "resource_saver_avif.h"

#include "godot_cpp/classes/file_access.hpp"
#include "godot_cpp/classes/image.hpp"
#include "godot_cpp/classes/image_texture.hpp"
#include "godot_cpp/classes/texture.hpp"
#include "godot_cpp/templates/vector.hpp"

using namespace godot;

ResourceSaverAVIF *ResourceSaverAVIF::singleton = nullptr;

#define SET_VAL(p_key)                       \
	if (p_config.has(#p_key)) {              \
		p_encoder->p_key = p_config[#p_key]; \
	}
void ResourceSaverAVIF::_configure_encoder(avifEncoder *p_encoder, const Dictionary &p_config) {
	SET_VAL(maxThreads);
	SET_VAL(minQuantizer);
	SET_VAL(maxQuantizer);
	SET_VAL(minQuantizerAlpha);
	SET_VAL(maxQuantizerAlpha);
	SET_VAL(tileRowsLog2);
	SET_VAL(tileColsLog2);
	SET_VAL(speed);
	SET_VAL(keyframeInterval);
	SET_VAL(timescale);
}
#undef SET_VAL

Error ResourceSaverAVIF::_avif_save_image_to_buffer(const Ref<Image> p_img, avifEncoder *p_encoder, avifRWData *r_output, ResourceSaverAVIF::PixelFormat p_format) {
	ERR_FAIL_COND_V(p_img.is_null(), ERR_INVALID_PARAMETER);
	PackedByteArray out;
	Ref<Image> source_image = p_img->duplicate();
	if (source_image->is_compressed()) {
		source_image->decompress();
	}

	ERR_FAIL_COND_V(source_image->is_compressed(), ERR_FILE_CORRUPT);

	source_image->convert(Image::FORMAT_RGBA8);

	avifPixelFormat format = AVIF_PIXEL_FORMAT_NONE;
	switch (p_format) {
		case ResourceSaverAVIF::AVIF_PIXEL_YUV444:
			format = AVIF_PIXEL_FORMAT_YUV444;
			break;
		case ResourceSaverAVIF::AVIF_PIXEL_YUV422:
			format = AVIF_PIXEL_FORMAT_YUV422;
			break;
		case ResourceSaverAVIF::AVIF_PIXEL_YUV420:
			format = AVIF_PIXEL_FORMAT_YUV420;
			break;
		case ResourceSaverAVIF::AVIF_PIXEL_YUV400:
			format = AVIF_PIXEL_FORMAT_YUV400;
			break;
		default:
			ERR_FAIL_V(ERR_INVALID_PARAMETER);
	}

	avifResult result;
	avifRGBImage rgb;
	memset(&rgb, 0, sizeof(rgb));
	const int width = source_image->get_width();
	const int height = source_image->get_height();
	// these values dictate what goes into the final AVIF
	avifImage *image = avifImageCreate(width, height, 8, format);
	avifRGBImageSetDefaults(&rgb, image);

	PackedByteArray data = source_image->get_data();
	rgb.rowBytes = width * avifRGBImagePixelSize(&rgb);
	rgb.pixels = data.ptrw();
	result = avifImageRGBToYUV(image, &rgb);
	ERR_FAIL_COND_V_MSG(result != AVIF_RESULT_OK, FAILED, avifResultToString(result));
	result = avifEncoderAddImage(p_encoder, image, 1, AVIF_ADD_IMAGE_FLAG_SINGLE);
	ERR_FAIL_COND_V_MSG(result != AVIF_RESULT_OK, FAILED, avifResultToString(result));

	result = avifEncoderFinish(p_encoder, r_output);
	ERR_FAIL_COND_V_MSG(result != AVIF_RESULT_OK, FAILED, avifResultToString(result));
	return OK;
}

PackedByteArray ResourceSaverAVIF::_avif_save_buffer_func(const Ref<Image> &p_img, const Dictionary &p_options, PixelFormat p_format) {
	Error err;
	avifEncoder *encoder = avifEncoderCreate();
	encoder->speed = AVIF_SPEED_FASTEST;
	_configure_encoder(encoder, singleton->encoder_options);
	_configure_encoder(encoder, p_options);
	avifRWData avifOutput = AVIF_DATA_EMPTY;
	err = _avif_save_image_to_buffer(p_img, encoder, &avifOutput, p_format == AVIF_PIXEL_DEFAULT ? singleton->pixel_format : p_format);
	avifEncoderDestroy(encoder);
	ERR_FAIL_COND_V(err != OK, PackedByteArray());

	PackedByteArray out;
	out.resize(avifOutput.size);
	memcpy(out.ptrw(), avifOutput.data, avifOutput.size);
	avifRWDataFree(&avifOutput);
	return out;
}

Error ResourceSaverAVIF::_avif_save_image_func(const String &p_path, const Ref<Image> &p_img, const Dictionary &p_options, PixelFormat p_format) {
	avifEncoder *encoder = avifEncoderCreate();
	encoder->speed = AVIF_SPEED_FASTEST;
	_configure_encoder(encoder, singleton->encoder_options);
	_configure_encoder(encoder, p_options);
	avifRWData avifOutput = AVIF_DATA_EMPTY;
	Error err = _avif_save_image_to_buffer(p_img, encoder, &avifOutput, p_format == AVIF_PIXEL_DEFAULT ? singleton->pixel_format : p_format);
	avifEncoderDestroy(encoder);
	ERR_FAIL_COND_V_MSG(err, err, "Can't convert image to AVIF.");

	Ref<FileAccess> file = FileAccess::open(p_path, FileAccess::WRITE);
	if (!file->is_open() || file->get_error() != OK) {
		avifRWDataFree(&avifOutput);
		ERR_FAIL_V_MSG(file->get_error(), "Can't save AVIF at path: '" + p_path + "'.");
	}

	PackedByteArray pba;
	pba.resize(avifOutput.size);
	memcpy(pba.ptrw(), avifOutput.data, avifOutput.size);
	file->store_buffer(pba);
	if (file->get_error() != OK && file->get_error() != ERR_FILE_EOF) {
		avifRWDataFree(&avifOutput);
		return ERR_CANT_CREATE;
	}

	avifRWDataFree(&avifOutput);

	return OK;
}

int64_t ResourceSaverAVIF::_save(const Ref<Resource> &p_resource, const String &p_path, int64_t p_flags) {
	Ref<ImageTexture> texture = p_resource;

	ERR_FAIL_COND_V_MSG(!texture.is_valid(), ERR_INVALID_PARAMETER, "Can't save invalid texture as AVIF.");
	ERR_FAIL_COND_V_MSG(!texture->get_width(), ERR_INVALID_PARAMETER, "Can't save empty texture as AVIF.");

	Ref<Image> img = texture->get_image();

	Dictionary opts;
	Error err = _avif_save_image_func(p_path, img, opts, pixel_format);

	return err;
}

bool ResourceSaverAVIF::_recognize(const Ref<Resource> &p_resource) const {
	return (p_resource.is_valid() && p_resource->is_class("ImageTexture"));
}

PackedStringArray ResourceSaverAVIF::_get_recognized_extensions(const Ref<Resource> &p_resource) const {
	PackedStringArray psa;
	if (p_resource.is_valid() && p_resource->is_class("ImageTexture")) {
		psa.push_back("avif");
	}
	return psa;
}

Error ResourceSaverAVIF::save_image(Ref<Image> p_image, const String &p_path, const Dictionary &p_options, PixelFormat p_format) {
	ERR_FAIL_COND_V(!singleton, ERR_UNCONFIGURED);
	ERR_FAIL_COND_V(p_image.is_null(), ERR_INVALID_PARAMETER);
	return singleton->_avif_save_image_func(p_path, p_image, p_options, p_format);
}

PackedByteArray ResourceSaverAVIF::encode_image(Ref<Image> p_image, const Dictionary &p_options, PixelFormat p_format) {
	ERR_FAIL_COND_V(!singleton, PackedByteArray());
	return singleton->_avif_save_buffer_func(p_image, p_options, p_format);
}

void ResourceSaverAVIF::set_defaults(const Dictionary &p_options, PixelFormat p_format) {
	ERR_FAIL_COND(!singleton);
	ERR_FAIL_COND(p_format <= AVIF_PIXEL_DEFAULT || p_format > AVIF_PIXEL_YUV400);
	singleton->encoder_options = p_options;
	singleton->pixel_format = p_format;
}

void ResourceSaverAVIF::_bind_methods() {
	ClassDB::bind_static_method("ResourceSaverAVIF", D_METHOD("set_defaults", "options", "format"), &ResourceSaverAVIF::set_defaults);
	ClassDB::bind_static_method("ResourceSaverAVIF", D_METHOD("save_image", "image", "path", "options", "format"), &ResourceSaverAVIF::save_image, DEFVAL(godot::Dictionary()), DEFVAL(AVIF_PIXEL_DEFAULT));
	ClassDB::bind_static_method("ResourceSaverAVIF", D_METHOD("encode_image", "image", "options", "format"), &ResourceSaverAVIF::encode_image, DEFVAL(godot::Dictionary()), DEFVAL(AVIF_PIXEL_DEFAULT));

	BIND_ENUM_CONSTANT(AVIF_PIXEL_DEFAULT);
	BIND_ENUM_CONSTANT(AVIF_PIXEL_YUV444);
	BIND_ENUM_CONSTANT(AVIF_PIXEL_YUV422);
	BIND_ENUM_CONSTANT(AVIF_PIXEL_YUV420);
	BIND_ENUM_CONSTANT(AVIF_PIXEL_YUV400);
}

ResourceSaverAVIF::ResourceSaverAVIF() {
	if (singleton == nullptr) {
		singleton = this;
	}
	encoder_options["maxThreads"] = 8;
	encoder_options["minQuantizer"] = 20;
	encoder_options["maxQuantizer"] = 30;
	encoder_options["minQuantizerAlpha"] = 20;
	encoder_options["maxQuantizerAlpha"] = 30;
	encoder_options["tileRowsLog2"] = 0;
	encoder_options["tileColsLog2"] = 0;
	encoder_options["speed"] = 8;
	encoder_options["keyframeInterval"] = 0;
	encoder_options["timescale"] = 30;
}
