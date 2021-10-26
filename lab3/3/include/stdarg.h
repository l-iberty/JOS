#ifndef JOS_INC_STDARG_H
#define JOS_INC_STDARG_H

#include <include/types.h>

#define va_start(ap, v) (ap = (va_list)&v + sizeof(int))
#define va_arg(ap, t) (*(t *)((ap += sizeof(int)) - sizeof(int)))
#define va_end(ap) (ap = (va_list)0)

#endif /* JOS_INC_STDARG_H */