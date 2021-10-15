#ifndef _JOS_STDARG_H_
#define _JOS_STDARG_H_

#include <include/types.h>

#define va_start(ap,v) ( ap = (va_list)&v + sizeof(int) )
#define va_arg(ap,t)   ( *(t *)((ap += sizeof(int)) - sizeof(int)) )
#define va_end(ap)     ( ap = (va_list)0 )

#endif /* _JOS_STDARG_H_ */