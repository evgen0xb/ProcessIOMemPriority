
// https://stackoverflow.com/questions/17739390/different-format-of-date-macro
// Posted by sulfurandcu, modified by community. See post 'Timeline' for change history
// License - CC BY-SA 4.0

#define __MONH__    ((__DATE__[0]+__DATE__[1]+__DATE__[2]) == 281 ? '0' \
                    :(__DATE__[0]+__DATE__[1]+__DATE__[2]) == 269 ? '0' \
                    :(__DATE__[0]+__DATE__[1]+__DATE__[2]) == 288 ? '0' \
                    :(__DATE__[0]+__DATE__[1]+__DATE__[2]) == 291 ? '0' \
                    :(__DATE__[0]+__DATE__[1]+__DATE__[2]) == 295 ? '0' \
                    :(__DATE__[0]+__DATE__[1]+__DATE__[2]) == 301 ? '0' \
                    :(__DATE__[0]+__DATE__[1]+__DATE__[2]) == 299 ? '0' \
                    :(__DATE__[0]+__DATE__[1]+__DATE__[2]) == 285 ? '0' \
                    :(__DATE__[0]+__DATE__[1]+__DATE__[2]) == 296 ? '0' \
                    :(__DATE__[0]+__DATE__[1]+__DATE__[2]) == 294 ? '1' \
                    :(__DATE__[0]+__DATE__[1]+__DATE__[2]) == 307 ? '1' \
                    :(__DATE__[0]+__DATE__[1]+__DATE__[2]) == 268 ? '1' \
                    : '0')
#define __MONL__    ((__DATE__[0]+__DATE__[1]+__DATE__[2]) == 281 ? '1' \
                    :(__DATE__[0]+__DATE__[1]+__DATE__[2]) == 269 ? '2' \
                    :(__DATE__[0]+__DATE__[1]+__DATE__[2]) == 288 ? '3' \
                    :(__DATE__[0]+__DATE__[1]+__DATE__[2]) == 291 ? '4' \
                    :(__DATE__[0]+__DATE__[1]+__DATE__[2]) == 295 ? '5' \
                    :(__DATE__[0]+__DATE__[1]+__DATE__[2]) == 301 ? '6' \
                    :(__DATE__[0]+__DATE__[1]+__DATE__[2]) == 299 ? '7' \
                    :(__DATE__[0]+__DATE__[1]+__DATE__[2]) == 285 ? '8' \
                    :(__DATE__[0]+__DATE__[1]+__DATE__[2]) == 296 ? '9' \
                    :(__DATE__[0]+__DATE__[1]+__DATE__[2]) == 294 ? '0' \
                    :(__DATE__[0]+__DATE__[1]+__DATE__[2]) == 307 ? '1' \
                    :(__DATE__[0]+__DATE__[1]+__DATE__[2]) == 268 ? '2' \
                    : '0')
#define __DATE_yh__ __DATE__[0x07], __DATE__[0x08]
#define __DATE_yl__ __DATE__[0x09], __DATE__[0x0A]
#define __DATE_mm__ __MONH__,       __MONL__
#define __DATE_dd__ __DATE__[0x04], __DATE__[0x05]

#define __DATE_yyyy_mm_dd__ {__DATE_yh__, __DATE_yl__, '-', __DATE_mm__, '-', __DATE_dd__, 0}
#define __DATE_yy_mm_dd__   {             __DATE_yl__, '-', __DATE_mm__, '-', __DATE_dd__, 0}

/*
example:

char const information[][16] =
{
    __DATE_yyyy_mm_dd__,                // strDATE 2024-09-13
    __DATE_yy_mm_dd__,                  // strDATE 24-09-13
};

const TCHAR COMPILEDATE[] = __DATE_yyyy_mm_dd__;

*/
