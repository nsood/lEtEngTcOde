#ifndef TQ_JSON_H_
#define TQ_JSON_H_

#include <json/json.h>

#include <assert.h>

#define INT_INV (0x7FFFFFF)
//#define INT_INV 0

#define jo_array_string_check(_jo_,__str__,_len_,__len__) do {\
	if (!_jo_)\
		return ;\
	if (json_object_get_type(_jo_) != json_type_string)\
		return ;\
	__str__ = json_object_get_string(_jo_);\
	if (!__str__)\
		return ;\
	_len_ = strlen(__str__);\
	if (_len_ < __len__)\
		return;\
	}while(0)

const char *tq_json_get_string(struct json_object *jo,const char *name);
struct json_object *tq_json_get_array(struct json_object *jo,const char *name);
int tq_json_get_int(struct json_object *jo,const char *name);
#endif
