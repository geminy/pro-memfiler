#include "MFBacktrace.h"

#include <unwind.h>

MF_DECLS
_Unwind_Reason_Code _Unwind_Backtrace (_Unwind_Trace_Fn, void*);

/**************************************************/

/**
 * @struct MFTraceParam
 * @brief Trace parameter for _Unwind_Backtrace
 */
struct MFTraceParam
{
    void **array;    /* Backtrace. */
    int count;       /* Real backtrace stacks. */
    int size;        /* Expect backtrace stacks. */
};

// maybe multi-entry
MF_DECLS
_Unwind_Reason_Code MFUnwindTraceFn(struct _Unwind_Context *context, void *arg)
{
    struct MFTraceParam *param = (struct MFTraceParam*)arg;

    if (param->count != -1) {
        param->array[param->count] = (void*)_Unwind_GetIP(context);
    }

    if (++(param->count) == param->size) {
        return _URC_END_OF_STACK;
    }

    return _URC_NO_REASON;
}

/**************************************************/

MF_DECLS
int MFGetBacktrace(intptr_t *addr, size_t stack)
{
//    struct MFTraceParam param = { .array = (void**)addr, .size = stack, .count = -1 };
    struct MFTraceParam param;
    param.array = (void**)addr;
    param.size = stack;
    param.count = -1;

    if (stack >= 1) {
        _Unwind_Backtrace(MFUnwindTraceFn, (void**)&param);
    }

    if (param.count > 1 && NULL == param.array[param.count - 1]) {
        --param.count;
    }

    return param.count != -1 ? param.count : 0;
}
