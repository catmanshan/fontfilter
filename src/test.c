#include <stdio.h>
#include <stdlib.h>

#undef NDEBUG
#include <assert.h>

#include "fontfilter.h"

int main()
{
	FfCondition *has_char = ff_require_char(12354);
	assert(has_char);

	FcFontSet *sys_fonts = FcConfigGetFonts(NULL, FcSetSystem);
	assert(sys_fonts);

	FcFontSet *filtered = ff_condition_filter(has_char, sys_fonts);
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

	return EXIT_SUCCESS;
}
