#include <string.h>

unsigned char g_ww_keys[][40] = {{"tP%Dco|#q,@6SGg_#8$j5>M]tDR7,wA!"}, };


int set_ww_pass_by_id(int id, unsigned char *pass)
{
    if (id < 0 || id >= 5)
        return -1;
    
    strncpy((char*)g_ww_keys[id], (char*)pass, 32);
    return 0;
}

int get_ww_pass_by_id(int id, unsigned char* pKey, size_t *len)
{
    int i;
    if (id < 0 || id >= 5)
        return -1;

    *len = strlen((const char *)g_ww_keys[id]);
    for (i = 0; i < *len; i++)
        pKey[i] = g_ww_keys[id][i] ^ 0xff;
    
    return 0;
}

int ww_encrypt(unsigned char *in, size_t in_len, unsigned char *key, size_t key_len, unsigned char *out)
{
    int i;
    
    if (!in || in_len <= 0
        || !key || key_len <= 2 
        || !out )
        return -1;

    for (i = 0; i < in_len; i++)
    {
        out[i] = (in[i] + key[i % key_len]) ^ key[key_len - 1 - i % key_len];
    }

    return 0;
}

int ww_decrypt(unsigned char *in, size_t in_len, unsigned char *key, size_t key_len, unsigned char *out)
{
    int i;
    
    if (!in || in_len <= 0
        || !key || key_len <= 2 
        || !out )
        return -1;

    for (i = 0; i < in_len; i++)
    {
        out[i] = (in[i] ^ key[key_len - 1 - i % key_len]) - key[i % key_len];
    }

    return 0;
}

int encrypt_with_default_pass(unsigned char *in, size_t in_len, unsigned char *out)
{
    int ret = -1;
    unsigned char key[32];
    size_t key_len = 0;
    ret = get_ww_pass_by_id(0, key, &key_len);

    if (ret != 0)
        return ret;
        
    return ww_encrypt(in, in_len, key, key_len, out);
}

int decrypt_with_default_pass(unsigned char *in, size_t in_len, unsigned char *out)
{
    int ret = -1;
    unsigned char key[32];
    size_t key_len = 0;
    ret = get_ww_pass_by_id(0, key, &key_len);

    if (ret != 0)
        return ret;
    
    return ww_decrypt(in, in_len, key, key_len, out);
}

