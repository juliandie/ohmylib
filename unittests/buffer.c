
#include <lib_log.h>
#include <lib_buffer.h>

int main() {
	lib_buffer buf;
	lib_buffer *another_buf;

	if (lib_buffer_alloc(&buf, 2048) < 0) {
		LIB_LOG_ERR("alloc");
		return -1;
	}
	lib_dump(&buf, sizeof(lib_buffer), NULL);

#if 1
	another_buf = lib_buffer_dup(&buf);
	if (another_buf == NULL) {
		LIB_LOG_ERR("dup");
		goto buf_free;
	}
	lib_dump(another_buf, sizeof(lib_buffer), NULL);
#else
	another_buf = lib_buffer_create(2048);
	if (another_buf == NULL) {
		LIB_LOG_ERR("create");
		goto buf_free;
	}
	lib_dump(another_buf, sizeof(lib_buffer), NULL);
#endif

	if (lib_buffer_resize(&buf, 4096) < 0) {
		LIB_LOG_ERR("resize");
		goto buf_destroy;
	}
	lib_dump(&buf, sizeof(lib_buffer), NULL);

buf_destroy:
	lib_buffer_destroy(another_buf);
buf_free:
	lib_buffer_free(&buf);

	return 0;
}