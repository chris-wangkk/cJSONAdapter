/*
	JSON ADAPTOR
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//#include "ctsgw_common.h"
//#include "ctsgw_private.h"
//#include "ctsgw_agent_api.h"
#include "cJSONAdapter.h"

/*
	for debug
*/
void printpJson(cJSON *pJson)
{
	printf("------------------------------------------\n");
	printf("string %s\n", pJson->string);
	printf("type %d\n",pJson->type);
	printf("valuestring %s\n", pJson->valuestring);
	printf("valueint %d\n",pJson->valueint);
	printf("valuedouble %f\n",pJson->valuedouble);
	printf("------------------------------------------\n");
	return;
}

/*
	JSON adapter proc
*/
#define JSON_NOT_BOOL_TYPE(type) \
	(cJSON_False != (type) && cJSON_True != (type))

#define JSONITEM_PREPROC(pAdapter) \
	cJSON *pJsonData = NULL; \
	pJsonData = cJSON_GetObjectItem(pJson,(pAdapter)->key); \
	if(NULL == pJsonData) \
	{ \
		if(false == (pAdapter)->bRequired) \
		{ \
			if(NULL != (pAdapter)->pfDftSetFunc) \
				(pAdapter)->pfDftSetFunc((pAdapter)->value); \
			return CJSON_ADAPTER_OK; \
		} \
		else \
			return CJSON_NOT_FOUND; \
	} \
	if((pAdapter)->type != pJsonData->type && (JSON_NOT_BOOL_TYPE((pAdapter)->type) || JSON_NOT_BOOL_TYPE(pJsonData->type))) \
		return CJSON_NOT_TYPE_MATCH;

#define JSONITEM_POSTPROC(pAdapter) \
	if(NULL != (pAdapter)->pfFilterFunc) \
		if(true != (pAdapter)->pfFilterFunc(pAdapter)) \
			return CJSON_NOT_CONDITION; \
	return CJSON_ADAPTER_OK;

#define JSONITEM_BOOLPROC(pAdapter,pJsonData) \
	if(cJSON_False == (pJsonData)->type) \
		*(char *)((pAdapter)->value) = false; \
	else \
		*(char *)((pAdapter)->value) = true; \
	(pAdapter)->value = (void *)((char *)((pAdapter)->value) + 1);


#define JSONITEM_NUMBERPROC(pAdapter,pJsonData) \
	if(CJSON_UNSET_TYPE == (pAdapter)->ucNumberType) \
		return CJSON_NOT_SETNUMTYPE; \
	switch((pAdapter)->ucNumberType) \
	{ \
	case CJSON_SHORT_TYPE: \
		*(short *)((pAdapter)->value) = (short)((pJsonData)->valueint); \
		(pAdapter)->value = (void *)((short *)((pAdapter)->value) + 1); \
		break; \
	case CJSON_USHORT_TYPE: \
		*(unsigned short *)((pAdapter)->value) = (unsigned short)((pJsonData)->valueint); \
		(pAdapter)->value = (void *)((unsigned short *)((pAdapter)->value) + 1); \
		break; \
	case CJSON_INT_TYPE: \
		*(int *)((pAdapter)->value) = (int)((pJsonData)->valueint); \
		(pAdapter)->value = (void *)((int *)((pAdapter)->value) + 1); \
		break; \
	case CJSON_UINT_TYPE: \
		*(unsigned int *)((pAdapter)->value) = (unsigned int)((pJsonData)->valueint); \
		(pAdapter)->value = (void *)((unsigned int *)((pAdapter)->value) + 1); \
		break; \
	case CJSON_DOUBLE_TYPE: \
		*(double *)((pAdapter)->value) = (double)((pJsonData)->valuedouble); \
		(pAdapter)->value = (void *)((double *)((pAdapter)->value) + 1); \
		break; \
	default: \
		return CJSON_NUMTYPE_INVALID; \
	}

#define JSONITEM_STRINGPROC(pAdapter,pJsonData) \
	len = (unsigned long)strlen((pJsonData)->valuestring); \
	if(0 == len) \
		return CJSON_ADAPTER_OK; \
	if(len > pAdapter->SpecLimit.usLenthQuota) \
		return CJSON_STRING_EXCEED; \
	memcpy((pAdapter)->value, (const void *)((pJsonData)->valuestring), len); \
	(pAdapter)->value = (void *)((char *)((pAdapter)->value) + pAdapter->SpecLimit.usLenthQuota);

int JSONBoolItemProc(JSONItem *pAdapter, cJSON *pJson)
{
	JSONITEM_PREPROC(pAdapter)
	JSONITEM_BOOLPROC(pAdapter,pJsonData)
	JSONITEM_POSTPROC(pAdapter)
}
int JSONNullItemProc(JSONItem *pAdapter, cJSON *pJson)
{
	JSONITEM_PREPROC(pAdapter)
	JSONITEM_POSTPROC(pAdapter)
}
int JSONNumberItemProc(JSONItem *pAdapter, cJSON *pJson)
{
	JSONITEM_PREPROC(pAdapter)	
	JSONITEM_NUMBERPROC(pAdapter,pJsonData)
	JSONITEM_POSTPROC(pAdapter)
}
int JSONStringItemProc(JSONItem *pAdapter, cJSON *pJson)
{
	unsigned long len = 0; 

	JSONITEM_PREPROC(pAdapter)
	JSONITEM_STRINGPROC(pAdapter,pJsonData)
	JSONITEM_POSTPROC(pAdapter)
}
int JSONArrayItemProc(JSONItem *pAdapter, cJSON *pJson)
{
	cJSON *pSubObj = NULL;
	int	iSize = 0, iCnt, unitType, ret;
	unsigned long len = 0;

	JSONITEM_PREPROC(pAdapter)

	iSize = cJSON_GetArraySize(pJsonData);
	if(iSize > pAdapter->SpecLimit.usObjQuota)
		return CJSON_OBJS_EXCEED;

	for(iCnt = 0; iCnt < iSize; iCnt++)
	{		
		pSubObj = cJSON_GetArrayItem(pJsonData, iCnt);
		if(0 == iCnt)
			unitType = pSubObj->type;
		else if(unitType != pSubObj->type)
			return CJSON_OBJS_DISUNITY;
		if(pSubObj->type >= CJSON_OBJECT_MAX)
			continue;
		if(cJSON_Object != unitType)
		{
			switch(unitType)
			{
			case cJSON_False:
			case cJSON_True:
				JSONITEM_BOOLPROC(pAdapter,pSubObj)
				break;
			case cJSON_Number:
				JSONITEM_NUMBERPROC(pAdapter,pSubObj)
				break;
			case cJSON_String:
				JSONITEM_STRINGPROC(pAdapter,pSubObj)
				break;
			}
		}
		else
		{
			if(NULL == pAdapter->subAdapter)
				return CJSON_NOT_SETSUBITEM;

			//ret = g_cJSONItemAdaptor[unitType](pAdapter, pSubObj);
			ret = cJSONAdaptProc(pAdapter->subAdapter, pSubObj);
			if(CJSON_ADAPTER_OK != ret)			
				return ret;
		}	
	}

	JSONITEM_POSTPROC(pAdapter)
}
int JSONObjectItemProc(JSONItem *pAdapter, cJSON *pJson)
{
	int ret;

	JSONITEM_PREPROC(pAdapter)
	
	if(NULL == pAdapter->subAdapter)
		return CJSON_NOT_SETSUBITEM;
	if(NULL == pJsonData->child)
		return CJSON_JSON_NONFORMAT;
	
	ret = cJSONAdaptProc(pAdapter->subAdapter, pJsonData);
	if(CJSON_ADAPTER_OK != ret)
		return ret;

	JSONITEM_POSTPROC(pAdapter)
}

pfJSONItemProc g_cJSONItemAdaptor[CJSON_OBJECT_MAX] = 
{
	JSONBoolItemProc,
	JSONBoolItemProc,
	JSONNullItemProc,
	JSONNumberItemProc,
	JSONStringItemProc,
	JSONArrayItemProc,
	JSONObjectItemProc,
}; 

/*
	the entry of JSONAdapter
*/
int cJSONAdaptProc(JSONAdapter *pAdapter, cJSON *pJson)
{
	JSONItem *pTmp;
	int ret;
	unsigned long iCnt;
	void *pStart;

	for(pTmp =  pAdapter->pstJSONItem, iCnt = 0; iCnt < pAdapter->JSONItemNum; pTmp++, iCnt++)
	{
		if(pTmp->type >= CJSON_OBJECT_MAX)
			continue;
		
		pStart = pTmp->value;
		ret = g_cJSONItemAdaptor[pTmp->type](pTmp, pJson);
		if(CJSON_ADAPTER_OK != ret)
		{
			printf("cJSONAdaptProc err for %s as %d \n", pTmp->key, ret);
			return ret;
		}
		pTmp->value = (void *)((char *)pStart + pAdapter->JSONObjSize);
	}
	return CJSON_ADAPTER_OK;
}
