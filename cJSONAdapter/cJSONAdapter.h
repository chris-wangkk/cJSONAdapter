#ifndef __CJSONADAPTER_H__
#define	__CJSONADAPTER_H__

#include "cJSON.h"

#ifndef false
#define false 0
#endif
#ifndef true
#define true 1
#endif

/*
	cJSON type
*/
#define cJSON_False		0
#define cJSON_True		1
#define cJSON_NULL		2
#define cJSON_Number	3
#define cJSON_String	4
#define cJSON_Array		5
#define cJSON_Object	6

#define CJSON_OBJECT_MAX 7
/*
	cJSON number type
*/
#define CJSON_UNSET_TYPE		0
#define CJSON_SHORT_TYPE		1
#define CJSON_USHORT_TYPE		2
#define CJSON_INT_TYPE			3
#define CJSON_UINT_TYPE			4
#define CJSON_DOUBLE_TYPE		5

/*
	error code for cJSON adapter
*/
#define CJSON_ADAPTER_OK				0
#define CJSON_NOT_FOUND					(-1)	
#define CJSON_NOT_TYPE_MATCH			(-2)
#define CJSON_NOT_CONDITION				(-3)
#define CJSON_NOT_SETNUMTYPE			(-4)
#define CJSON_NUMTYPE_INVALID			(-5)
#define CJSON_STRING_EXCEED				(-6)
#define CJSON_OBJS_EXCEED				(-7)
#define CJSON_OBJS_DISUNITY				(-8)
#define CJSON_NOT_SETSUBITEM			(-9)
#define CJSON_JSON_NONFORMAT			(-10)

struct tagJSONItem;
struct tagJSONAdapter;
typedef int (*pfFilter)(struct tagJSONItem *);
typedef void (*pfDftSet)(void *);
typedef int (*pfJSONItemProc)(struct tagJSONItem *pAdapter, cJSON *pJson);

typedef struct tagJSONItem
{
	const char				*key;				//key info
	void					*value;				//the address of struct var by wirtten
    int						type;				//the same as cJSON->type
	char					bRequired;			//require or optional
	char					ucNumberType;		//only number and array(if subtype is number) effective
	union{
		unsigned short		usObjQuota;			//only array effective
		unsigned short		usLenthQuota;		//only string effective
	}SpecLimit;		
	pfFilter				pfFilterFunc;		//value filter(opt)
	pfDftSet				pfDftSetFunc;		//default value(opt)
    struct tagJSONAdapter	*subAdapter;		//only array(if subtype is obj) and obj effective			
}JSONItem;

typedef struct tagJSONAdapter
{
	unsigned long JSONItemNum;
	unsigned long JSONObjSize;
	JSONItem *pstJSONItem;
}JSONAdapter;


extern int cJSONAdaptProc(JSONAdapter *pAdapter, cJSON *pJson);

#endif
