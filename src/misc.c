#define COMMONLIB_REMOVE_PREFIX 
#include <commonlib.h>
#include <misc.h>

// 'i/j:'
bool parse_i_j_from_sv(String_view *sv, bool has_colon, int *i_out, int *j_out, const char *i_name, const char *j_name) {
	String_view i_sv = sv_lpop_until_char(sv, '/');
	sv_lremove(sv, 1); // Remove /
	int i_count = -1;
	int i = sv_to_int(i_sv, &i_count, 10);
	if (i_count < 0) {
		log_error("Failed to parse %s: cannot convert `"SV_FMT"` to int!", i_name, SV_ARG(i_sv));
		return false;
	}

    String_view j_sv = sv_lpop_until_char(sv, has_colon ? ':' : ' ');
    sv_lremove(sv, 1); // Remove : or space

	int j_count = -1;
	int j = sv_to_int(j_sv, &j_count, 10);
	if (j_count < 0) {
		log_error("Failed to parse %s: cannot convert `"SV_FMT"` to int!", j_name, SV_ARG(j_sv));
		return false;
	}

	*i_out = i;
	*j_out = j;

	return true;
}
