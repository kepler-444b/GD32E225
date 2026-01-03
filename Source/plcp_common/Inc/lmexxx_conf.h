#include "../../Source/device/device_manager.h"
#if defined PLCP_DEVICE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "Common.h"
#include "Uapps.h"
#include "APP_RxBuffer.h"
#include "APP_TxBuffer.h"

#define LOGINFO0(x, a)                   printf(a)
#define LOGINFO1(x, a, b)                printf((a), (b))
#define LOGINFO2(x, a, b, c)             printf((a), (b), (c))
#define LOGINFO3(x, a, b, c, d)          printf((a), (b), (c), (d))
#define LOGINFO4(x, a, b, c, d, e)       printf((a), (b), (c), (d), (e))
#define LOGINFO5(x, a, b, c, d, e, f)    printf((a), (b), (c), (d), (e), (f))
#define LOGINFO6(x, a, b, c, d, e, f, g) printf((a), (b), (c), (d), (e), (f), (g))
#endif