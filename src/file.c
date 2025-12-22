#include <xmllib.h>

static int xmllib_driver_file_open(xmllib_t* handle) {
	if((handle->drv_opaque = fopen(handle->drv_arg, "rb")) == NULL) return 0;

	return 1;
}

static int xmllib_driver_file_read(xmllib_t* handle, void* data, int size) {
	return fread(data, 1, size, handle->drv_opaque);
}

static void xmllib_driver_file_close(xmllib_t* handle) {
	fclose(handle->drv_opaque);
}

xmllib_driver_t xmllib_driver_file_rec = {
    xmllib_driver_file_open,
    xmllib_driver_file_read,
    xmllib_driver_file_close,
};
xmllib_driver_t* xmllib_driver_file = &xmllib_driver_file_rec;

xmllib_t* xmllib_open_file(const char* filename) {
	return xmllib_open(xmllib_driver_file, (void*)filename);
}
