#include <sys/stat.h>
#include <stdio.h>
#include "tq_json.h"

int tq_json_get_int(struct json_object *jo,const char *name)
{
	assert(jo && name);
	struct json_object *tmp = NULL;
	int res = INT_INV;
	if (!json_object_object_get_ex(jo,name,&tmp) || !tmp)
		return res;
	if (json_object_get_type(tmp) != json_type_int)
		return res;
	res = json_object_get_int(tmp);
	return res;
}
const char *tq_json_get_string(struct json_object *jo,const char *name)
{
	const char *str = NULL;
	struct json_object *tmp = NULL;
	assert (jo&& name);
	
	if (!json_object_object_get_ex(jo,name,&tmp) || !tmp)
		return str;
	
	if (json_object_get_type(tmp) != json_type_string)
		return str;
	str = json_object_get_string(tmp);
	return str;
}
struct json_object *tq_json_get_array(struct json_object *jo,const char *name)
{
	assert (jo&& name);
	struct json_object *res = NULL;
	json_object_object_get_ex(jo,name,&res);
	return res;
}

