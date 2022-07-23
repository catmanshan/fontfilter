#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#undef NDEBUG
#include <assert.h>

#include <fontfilter.h>

static void add_condition(FfCondition *condition, const char *desc);
static void print_conditions(void);
static void destroy_everything(void);

static void soft_filter_example(void);

enum { NMAX_CONDITIONS = 64 };
static size_t nconditions;
static FfCondition *conditions[NMAX_CONDITIONS];
static char *descs[NMAX_CONDITIONS];

int main(void)
{
	FfCondition *mono = ff_compare(FC_SPACING, FF_EQUAL, FcTypeInteger,
			FC_MONO);
	add_condition(mono, "Monospace font");

	/* ******** */

	FfCondition *non_proportional = ff_compare(FC_SPACING, FF_NOT_EQUAL,
			FcTypeInteger, FC_PROPORTIONAL);
	add_condition(non_proportional, "Non-proportional font");

	/* ******** */

	FcChar8 a_hiragana_utf8[] = "あ";
	FcChar32 a_hiragana_utf32;
	FcUtf8ToUcs4(a_hiragana_utf8, &a_hiragana_utf32,
			sizeof(a_hiragana_utf8) - 1);
	FfCondition *has_char = ff_require_char(a_hiragana_utf32);
	add_condition(has_char, "Font that supports the character \"あ\"");

	/* ******** */

	FfCondition *sans = ff_compare(FC_FAMILY, FF_CONTAINS, FcTypeString,
			(const FcChar8 []){ "Sans" });
	add_condition(sans, "Font whose family contains the word \"Sans\"");

	/* ******** */

	FfCondition *bold = ff_compare(FC_WEIGHT, FF_EQUAL, FcTypeInteger,
			FC_WEIGHT_BOLD);
	FfCondition *italic = ff_compare(FC_SLANT, FF_EQUAL, FcTypeInteger,
			FC_SLANT_ITALIC);

	FfCondition *bold_and_italic = ff_compose(bold, FF_AND, italic);
	add_condition(bold_and_italic, "Font that is bold and italic");

	FfCondition *bold_xor_italic = ff_compose(bold, FF_XOR, italic);
	add_condition(bold_xor_italic,
			"Font that is bold or italic, but not both");

	ff_condition_unref(bold);
	ff_condition_unref(italic);

	/* ******** */

	FfCondition *gte_bold = ff_compare(FC_WEIGHT, FF_GREATER_THAN_EQUAL,
			FcTypeInteger, FC_WEIGHT_BOLD);
	FfCondition *lte_heavy = ff_compare(FC_WEIGHT, FF_LESS_THAN_EQUAL,
			FcTypeInteger, FC_WEIGHT_HEAVY);

	FfCondition *bold_to_heavy = ff_compose_unref(gte_bold, FF_AND,
			lte_heavy);
	add_condition(bold_to_heavy,
			"Font whose weight is between bold and heavy");

	/* ******** */

	print_conditions();
	destroy_everything();

	soft_filter_example();

	return EXIT_SUCCESS;
}

static void add_condition(FfCondition *condition, const char *desc)
{
	assert(nconditions < NMAX_CONDITIONS);

	size_t desc_len = strlen(desc);
	char *desc_cpy = malloc(desc_len + 1);
	memcpy(desc_cpy, desc,desc_len + 1);

	assert(condition);
	assert(desc_cpy);

	conditions[nconditions] = condition;
	descs[nconditions] = desc_cpy;

	++nconditions;
}

static void print_conditions(void)
{
	FcFontSet *sys_fonts = FcConfigGetFonts(NULL, FcSetSystem);
	assert(sys_fonts);

	for (size_t i = 0; i < nconditions; ++i) {
		FfCondition *condition = conditions[i];
		char *desc = descs[i];

		FcPattern *font;
		{
			FcFontSet *filtered = ff_condition_filter(condition,
					sys_fonts);
			assert(filtered);

			if (filtered->nfont == 0) {
				continue;
			}

			font = filtered->fonts[0];

			FcFontSetDestroy(filtered);
		}

		FcChar8 *font_fullname = (FcChar8 *) "??";
		FcPatternGetString(font, FC_FULLNAME, 0, &font_fullname);

		printf("%s:\n"
		       "    %s\n\n",
				desc, font_fullname);
	}
}

void destroy_everything(void)
{
	for (size_t i = 0; i < nconditions; ++i) {
		ff_condition_unref(conditions[i]);
		free(descs[i]);
	}
}

void soft_filter_example(void)
{
	FcFontSet *sys_fonts = FcConfigGetFonts(NULL, FcSetSystem);
	assert(sys_fonts);

	int status;
	FfList list = ff_list_create(&status);
	assert(status == FF_SUCCESS);

	FfCondition *not_roman = ff_compare(FC_SLANT, FF_NOT_EQUAL,
			FcTypeInteger, FC_SLANT_ROMAN);
	FfCondition *italic = ff_compare(FC_SLANT, FF_EQUAL, FcTypeInteger,
			FC_SLANT_ITALIC);

	bool success;
	success = ff_list_add_unref(&list, not_roman);
	assert(success);
	success = ff_list_add_unref(&list, italic);
	assert(success);

	FcFontSet *filtered = ff_list_filter_soft(list, sys_fonts);
	assert(filtered);

	if (filtered->nfont > 0) {
		FcPattern *font = filtered->fonts[0];

		FcChar8 *font_fullname = (FcChar8 *)"??";
		FcPatternGetString(font, FC_FULLNAME, 0, &font_fullname);

		printf("Font that is oblique or italic, but preferably italic:\n"
		       "    %s\n", font_fullname);
	}

	FcFontSetDestroy(filtered);

	ff_list_destroy(list);
}
