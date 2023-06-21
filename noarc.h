//
//  noarc.hpp
//  noarc
//
//  Created by John Burkey on 8/10/21.
//

#ifndef noarc_hpp
#define noarc_hpp

#include <stdio.h>

// #ifdef __cplusplus
// extern "C" {
// #endif

void begin_DebuggingARC();
void end_DebuggingARC();
void brighten_MarkNoArc(void *object);
void brighten_StartNoArc();
int nextWorkID();


//#if// def __cplusplus
// }  /* End of the 'extern "C"' block */
// #endif

#endif /* noarc_hpp */

