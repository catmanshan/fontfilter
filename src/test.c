#include <stdio.h>
#include <stdlib.h>

#undef NDEBUG
#include <assert.h>

#include "fontfilter.h"

int main()
{
	FfCondition *not_proport = ff_compare(FC_SPACING, FF_NOT_EQUAL,
			FC_PROPORTIONAL);
	FfCondition *bold = ff_compare_value(FC_WEIGHT, FF_GREATER_THAN_EQUAL,
			ff_create_fc_value(FcTypeInteger, FC_WEIGHT_BOLD));
	FfCondition *italic = ff_compare(FC_SLANT, FF_EQUAL, FC_SLANT_ITALIC);

	assert(not_proport && bold && italic);

	int status;
	FfList list = ff_list_create(&status);
	assert(status == FF_SUCCESS);

	ff_list_add(&list, not_proport);
	ff_list_add(&list, bold);
	ff_list_add(&list, italic);

	ff_condition_unref(not_proport);
	ff_condition_unref(bold);
	ff_condition_unref(italic);

	FcFontSet *sys_fonts = FcConfigGetFonts(NULL, FcSetSystem);
	assert(sys_fonts);

	FcFontSet *filtered = ff_list_filter(list, sys_fonts);

	ff_list_destroy(list);

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
