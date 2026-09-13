#include "fmgui.h"

text mktext(const char* content)
{
    return (text){.content = content,
        .color = {0, 0, 0, 255}, .font_size = 16, .justify = TEXT_LEFT};
}
