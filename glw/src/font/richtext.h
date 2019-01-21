/***
Author: Mario J. Martin <dominonurbs$gmail.com>

Increase the funtionalities of the fonts to write rich text formats
with bold, italic and unicode characters

*******************************************************************************/

#ifndef RT_RICHTEXT_H
#define RT_RICHTEXT_H

#define RT_REGULAR 0
#define RT_BOLD 1
#define RT_ITALIC 2

#ifdef	__cplusplus
extern "C" {
#endif

    const char* char2unicode( const char* text, unsigned long* unicode );

#ifdef	__cplusplus
}
#endif

#endif /* RT_RICHTEXT_H */

