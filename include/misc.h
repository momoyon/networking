#ifndef _MISC_H_
#define _MISC_H_


// 'i/j:' or 'i/j'
bool parse_i_j_from_sv(String_view *sv, bool has_colon, int *i_out, int *j_out, const char *i_name, const char *j_name);

#endif // _MISC_H_

