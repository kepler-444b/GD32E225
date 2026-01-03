
#if 0
#include "PLCP_sequentialData.h"

U8 PLCP_seqDataInit(seqDataType *seqData, U8 itemSize, U8 *arrayStorage, U16 storageSize)
{
    if (seqData == NULL || itemSize == 0 || arrayStorage == NULL || storageSize == 0 || itemSize > storageSize) {
        return 0;
    }

    seqData->tail = 0;
    seqData->pArrayStorage = arrayStorage;
    seqData->itemSize = itemSize;
    seqData->storageSize = storageSize;

    return 1;
}

U8 PLCP_seqDataItemAddTail(seqDataType *seqData, U8 *item)
{
    if (seqData == NULL || item == NULL) {
        return 0;
    }

    if (seqData->storageSize - seqData->tail < seqData->itemSize) {
        return 0;
    }

    memcpy(&(seqData->pArrayStorage[seqData->tail]), item, seqData->itemSize);
    seqData->tail += seqData->itemSize;

    return 1;
}

U8 PLCP_seqDataItemDel(seqDataType *seqData, U8 itemIndex)
{
    if (seqData == NULL) {
        return 0;
    }

    if (seqData->tail < seqData->itemSize * (itemIndex + 1)) {
        return 0;
    }

    memcpy(&(seqData->pArrayStorage[seqData->itemSize * itemIndex]),
           &(seqData->pArrayStorage[seqData->itemSize * (itemIndex + 1)]),
           seqData->tail - seqData->itemSize * (itemIndex + 1));
    seqData->tail -= seqData->itemSize;

    return 1;
}

U8 PLCP_seqDataItemModify(seqDataType *seqData, U8 itemIndex, U8 *modifyItem)
{
    if (seqData == NULL || modifyItem == NULL) {
        return 0;
    }

    if (seqData->tail < seqData->itemSize * (itemIndex + 1)) {
        return 0;
    }

    memcpy(&(seqData->pArrayStorage[seqData->itemSize * itemIndex]), modifyItem, seqData->itemSize);

    return 1;
}

U8 PLCP_seqDataSearchItemIndex(seqDataType *seqData, U8 *serchItem)
{
    if (seqData == NULL || serchItem == NULL) {
        return 0xff;
    }

    U8 i;
    for (i = 0; i < seqData->tail / seqData->itemSize; i++) {
        if (memcmp(&(seqData->pArrayStorage[seqData->itemSize * i]),
                   serchItem, seqData->itemSize) == 0) {
            return i;
        }
    }

    return 0xff;
}

U8 PLCP_seqDataItemRead(seqDataType *seqData, U8 readItemIndex, U8 *readItem)
{
    if (seqData == NULL || readItem == NULL) {
        return 0;
    }

    if (seqData->tail < seqData->itemSize * (readItemIndex + 1)) {
        return 0;
    }

    memcpy(readItem, &(seqData->pArrayStorage[seqData->itemSize * readItemIndex]), seqData->itemSize);

    return 1;
}

U8 PLCP_seqDataItemCount(seqDataType *seqData)
{
    if (seqData == NULL) {
        return 0;
    }

    return seqData->tail / seqData->itemSize;
}

U8 PLCP_seqDataReset(seqDataType *seqData)
{
    if (seqData == NULL) {
        return 0;
    }

    seqData->tail = 0;
    return 1;
}
#endif