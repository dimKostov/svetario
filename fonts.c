#include <libpynq.h>
#include "mechanics.h"
#include "declarations.h"

void fontInit(){
      uint8_t buffer_font_title[1000] = {0,};
      uint8_t buffer_font_standard[1000] = {0,};

      // font setup
        //title
        InitFontx(fontTitle, "../../fonts/LATIN32B.FNT", "");
        GetFontx(fontTitle, 0, buffer_font_title, &fontWidthTitle, &fontHeightTitle);
        //standard
        InitFontx(fontStandard, "../../fonts/ILGH16XB.FNT", "");
        GetFontx(fontStandard, 0, buffer_font_standard, &fontWidthStandard, &fontHeightStandard);
}