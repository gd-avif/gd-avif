/****************************************************************************/
/*  image_loader_avif.h                                                     */
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

#ifndef IMAGE_LOADER_AVIF_H
#define IMAGE_LOADER_AVIF_H

#ifdef GDEXTENSION

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/global_constants_binds.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/image_format_loader_extension.hpp>

using namespace godot;

#else // Module

#include "core/io/file_access.h"
#include "core/io/image.h"
#include "core/io/image_loader.h"

#endif

#include <avif/avif.h>

class ImageLoaderAVIF : public ImageFormatLoaderExtension {
	GDCLASS(ImageLoaderAVIF, ImageFormatLoaderExtension);

protected:
	static void _bind_methods();

	static Error avif_load_image_from_buffer(Image *p_image, const uint8_t *p_buffer, int p_buffer_len, avifDecoder *p_decoder);
	static Ref<Image> _avif_mem_loader_func(const uint8_t *p_buffer, int p_size);

public:
#ifdef GDEXTENSION
	virtual Error _load_image(const Ref<Image> &p_image, const Ref<FileAccess> &p_file, BitField<ImageFormatLoader::LoaderFlags> p_flags, double p_scale) override;
	virtual PackedStringArray _get_recognized_extensions() const override;
#else
	virtual Error load_image(Ref<Image> p_image, Ref<FileAccess> p_file, BitField<ImageFormatLoader::LoaderFlags> p_flags, float p_scale) override;
	virtual void get_recognized_extensions(List<String> *r_extension) const override;
#endif

	static Ref<Image> load_avif_from_buffer(PackedByteArray p_buffer);

	ImageLoaderAVIF();
};

#endif
