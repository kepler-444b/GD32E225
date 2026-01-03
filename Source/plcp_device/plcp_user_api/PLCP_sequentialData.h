
#if 0
#ifndef _PLCP_SEQDATA_H_
#define _PLCP_SEQDATA_H_

#include "../../Source/plcp_common/Inc/lmexxx_conf.h"


typedef struct
{
	U16 tail;
	U8* pArrayStorage;
	U8 itemSize;
	U16 storageSize;
}seqDataType;


U8 PLCP_seqDataInit(seqDataType* seqData, U8 itemSize, U8* arrayStorage, U16 storageSize);
U8 PLCP_seqDataItemAddTail(seqDataType* seqData, U8* item);
U8 PLCP_seqDataItemDel(seqDataType* seqData, U8 itemIndex);
U8 PLCP_seqDataItemModify(seqDataType* seqData, U8 itemIndex, U8* modifyItem);
U8 PLCP_seqDataSearchItemIndex(seqDataType* seqData, U8* serchItem);
U8 PLCP_seqDataItemRead(seqDataType* seqData, U8 readItemIndex, U8* readItem);
U8 PLCP_seqDataItemCount(seqDataType* seqData);
U8 PLCP_seqDataReset(seqDataType* seqData);

#endif
#endif
