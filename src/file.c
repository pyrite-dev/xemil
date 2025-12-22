#include <xmllib.h>

static int xl_driver_file_open(xmllib_t* handle) {
	if((handle->drv_opaque = fopen(handle->drv_arg, "rb")) == NULL) return 0;

	return 1;
}

static int xl_driver_file_read(xmllib_t* handle, void* data, int size) {
	return fread(data, 1, size, handle->drv_opaque);
}

static void xl_driver_file_close(xmllib_t* handle) {
	fclose(handle->drv_opaque);
}

xl_driver_t xl_driver_file_rec = {
    xl_driver_file_open,
    xl_driver_file_read,
    xl_driver_file_close,
};
xl_driver_t* xl_driver_file = &xl_driver_file_rec;

xmllib_t* xl_open_file(const char* filename) {
	return xl_open(xl_driver_file, (void*)filename);
}
