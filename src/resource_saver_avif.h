/****************************************************************************/
/*  resource_saver_avif.h                                                   */
/****************************************************************************/
/* Copyright (c) 2020-present Fabio Alessandrelli, Tim Erskine, Maffle LLC. */
/*                                                                          */
/* Permission is hereby granted, free of charge, to any person obtaining    */
/* a copy of this software and associated documentation files (the          */
/* "Software"), to deal in the Software without restriction, including      */
/* without limitation the rights to use, copy, modify, merge, publish,      */
/* distribute, sublicense, and/or sell copies of the Software, and to       */
/* permit persons to whom the Software is furnished to do so, subject to    */
/* the following conditions:                                                */
/*                                                                          */
/* The above copyright notice and this permission notice shall be           */
/* included in all copies or substantial portions of the Software.          */
/*                                                                          */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,          */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF       */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.   */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY     */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,     */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE        */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                   */
/****************************************************************************/

#ifndef RESOURCE_SAVER_AVIF_H
#define RESOURCE_SAVER_AVIF_H

#include "godot_cpp/classes/image.hpp"
#include "godot_cpp/classes/resource_format_saver.hpp"
#include "godot_cpp/variant/dictionary.hpp"

#include <avif/avif.h>

class ResourceSaverAVIF : public godot::ResourceFormatSaver {
	GDCLASS(ResourceSaverAVIF, godot::ResourceFormatSaver);

protected:
	static void _bind_methods();

public:
	enum PixelFormat {
		AVIF_PIXEL_DEFAULT,
		AVIF_PIXEL_YUV444,
		AVIF_PIXEL_YUV422,
		AVIF_PIXEL_YUV420,
		AVIF_PIXEL_YUV400,
	};

private:
	godot::Dictionary encoder_options;
	PixelFormat pixel_format = AVIF_PIXEL_YUV422;

	static void _configure_encoder(avifEncoder *p_encoder, const godot::Dictionary &p_config);
	static godot::Error _avif_save_image_to_buffer(const godot::Ref<godot::Image> p_img, avifEncoder *p_encoder, avifRWData *r_output, ResourceSaverAVIF::PixelFormat p_format);

	static godot::Error _avif_save_image_func(const godot::String &p_path, const godot::Ref<godot::Image> &p_img, const godot::Dictionary &p_options, PixelFormat p_format);
	static godot::PackedByteArray _avif_save_buffer_func(const godot::Ref<godot::Image> &p_img, const godot::Dictionary &p_options, PixelFormat p_format);

public:
	static ResourceSaverAVIF *singleton;

	virtual godot::Error _save(const godot::Ref<godot::Resource> &p_resource, const godot::String &p_path, uint32_t p_flags = 0) override;
	virtual bool _recognize(const godot::Ref<godot::Resource> &p_resource) const override;
	virtual godot::PackedStringArray _get_recognized_extensions(const godot::Ref<godot::Resource> &p_resource) const override;

	static godot::Error save_avif(godot::Ref<godot::Image> p_image, const godot::String &p_path, const godot::Dictionary &p_options = godot::Dictionary(), PixelFormat p_format = AVIF_PIXEL_YUV422);
	static godot::PackedByteArray save_avif_to_buffer(godot::Ref<godot::Image> p_image, const godot::Dictionary &p_options = godot::Dictionary(), PixelFormat p_format = AVIF_PIXEL_YUV422);
	static void set_avif_options_and_format(const godot::Dictionary &p_options = {}, PixelFormat p_format = AVIF_PIXEL_YUV422);
	static void reset_avif_options_and_format();
	static godot::Dictionary get_avif_encoder_options();
	static PixelFormat get_avif_pixel_format();

	ResourceSaverAVIF();
	~ResourceSaverAVIF() { singleton = nullptr; }
};

VARIANT_ENUM_CAST(ResourceSaverAVIF::PixelFormat);

#endif // RESOURCE_SAVER_AVIF_H
