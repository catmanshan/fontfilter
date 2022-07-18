#include <stdio.h>
#include <stdlib.h>

#undef NDEBUG
#include <assert.h>

#include "fontfilter.h"

int main()
{
	FcCharSet *cs = FcCharSetCreate();
	FcCharSetAddChar(cs, 12354);

	FfCondition *cs_cond = ff_compare(FC_CHARSET, FF_CONTAINS,
			FcTypeCharSet, cs);
	assert(cs_cond);

	FcFontSet *sys_fonts = FcConfigGetFonts(NULL, FcSetSystem);
	assert(sys_fonts);

	FcFontSet *filtered = ff_condition_filter(cs_cond, sys_fonts);
	assert(filtered);

	for (int i = 0; i < filtered->nfont; ++i) {
		FcPattern *font = filtered->fonts[i];
		FcChar8 *fmt = (FcChar8 *)"%{family=}\n"
					  "%{style=}\n"
					  "%{weight=}\n"
					  "%{slant=}\n";

		FcChar8 *str = FcPatternFormat(font, fmt);
		printf("%s\n", str);
		free(str);
	}

	FcFontSetDestroy(filtered);

	FcCharSetDestroy(cs);
	
	return EXIT_SUCCESS;
}
