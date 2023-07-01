/****************************************************************************/
/*  register_types.cpp                                                      */
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

#include "image_loader_avif.h"
#include "resource_saver_avif.h"

#include "register_types.h"

static Ref<ImageLoaderAVIF> loader;
static Ref<ResourceSaverAVIF> saver;

void initialize_avif_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	ClassDB::register_class<ResourceSaverAVIF>();
	ClassDB::register_class<ImageLoaderAVIF>();
	loader.instantiate();
	loader->add_format_loader();
	saver.instantiate();
#ifdef GDEXTENSION
	ResourceSaver::get_singleton()->add_resource_format_saver(saver);
#else
	ResourceSaver::add_resource_format_saver(saver);
#endif
}

void uninitialize_avif_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

#ifdef GDEXTENSION
	ResourceSaver::get_singleton()->remove_resource_format_saver(saver);
#else
	ResourceSaver::remove_resource_format_saver(saver);
#endif
	loader->remove_format_loader();
	loader.unref();
	saver.unref();
}

#ifdef GDEXTENSION
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/core/memory.hpp>

using namespace godot;

extern "C" {
GDExtensionBool GDE_EXPORT avif_extension_init(const GDExtensionInterface *p_interface, const GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization) {
	GDExtensionBinding::InitObject init_obj(p_interface, p_library, r_initialization);

	init_obj.register_initializer(initialize_avif_module);
	init_obj.register_terminator(uninitialize_avif_module);
	init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

	return init_obj.init();
}
}
#endif
