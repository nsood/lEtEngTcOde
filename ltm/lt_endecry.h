#ifndef __LT_ENDECRY_H
#define __LT_ENDECRY_H

int set_ww_pass_by_id(int id, unsigned char *pass);
int get_ww_pass_by_id(int id, unsigned char* pKey, size_t *len);
int encrypt_with_default_pass(unsigned char *in, size_t in_len, unsigned char *out);
int decrypt_with_default_pass(unsigned char *in, size_t in_len, unsigned char *out);

#endif
