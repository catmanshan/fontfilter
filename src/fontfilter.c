#include "fontfilter.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

#include <fontconfig/fontconfig.h>

#include <tyrant.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))

static void destroy_condition(FfCondition *condition);
static bool inc_ref_count(size_t *ref_count);
static bool dec_ref_count(size_t *ref_count);
static bool test_comparison(FfComparison comparison, FcPattern *pattern);
static bool test_composition(FfLogicalComposition composition,
		FcPattern *pattern);
static bool test_comparison_for_value(FfComparison comparison, FcValue value);

FfCondition *ff_compare(const char *object, FfRelationalOperator operator, ...)
{
	va_list va;
	va_start(va, operator);

	FcType type = FcNameGetObjectType(object)->type;
	FcValue value = ff_create_fc_value_va(type, va);

	va_end(va);

	return ff_compare_value(object, operator, value);
}

FfCondition *ff_compare_value(const char *object, FfRelationalOperator operator,
		FcValue value)
{
	FfCondition *condition = tyrant_alloc(sizeof(*condition));
	if (condition == NULL) {
		return NULL;
	}

	*condition = (FfCondition){
		.type = FF_COMPARISON,
		.value.comparison = (FfComparison){
			.object = object,
			.value = value,
			.operator = operator
		},

		.ref_count = 1
	};
	return condition;
}

FfCondition *ff_compose(FfCondition *p, FfLogicalOperator operator,
		FfCondition *q)
{
	FfCondition *condition = tyrant_alloc(sizeof(*condition));
	if (condition == NULL) {
		goto err_exit;
	}

	if (ff_condition_ref(p) == NULL) {
		goto err_free_condition;
	}

	if (ff_condition_ref(q) == NULL) {
		goto err_unref_p;
	}

	*condition = (FfCondition){
		.type = FF_COMPOSITION,
		.value.composition = (FfLogicalComposition){
			.p = p,
			.q = q,
			.operator = operator
		},
		.ref_count = 1
	};
	return condition;

err_unref_p:
	ff_condition_unref(p);
err_free_condition:
	tyrant_free(condition);
err_exit:
	return NULL;
}

FfCondition *ff_compose_unref(FfCondition *p, FfLogicalOperator operator,
		FfCondition *q)
{
	FfCondition *condition = ff_compose(p, operator, q);

	ff_condition_unref(p);
	ff_condition_unref(q);

	return condition;
}

FfCondition *ff_condition_ref(FfCondition *condition)
{
	if (inc_ref_count(&condition->ref_count)) {
		return condition;
	}

	return NULL;
}

void ff_condition_unref(FfCondition *condition)
{
	if (dec_ref_count(&condition->ref_count) && condition->ref_count == 0) {
		destroy_condition(condition);
	}
}

void destroy_condition(FfCondition *condition)
{
	if (condition->type == FF_COMPOSITION) {
		ff_condition_unref(condition->value.composition.p);
		ff_condition_unref(condition->value.composition.q);
	}

	tyrant_free(condition);
}

bool inc_ref_count(size_t *ref_count)
{
	if (*ref_count == ULLONG_MAX) {
		return false;
	}

	++*ref_count;
	return true;
}

bool dec_ref_count(size_t *ref_count)
{
	if (*ref_count == 0) {
		return false;
	}

	--*ref_count;

	return true;
}

FfList ff_list_create(int *ret_status)
{
	return ff_list_create_with_cap(8, ret_status);
}

FfList ff_list_create_with_cap(int cap, int *ret_status)
{
	FfCondition **conditions = TYRANT_ALLOC_ARR(conditions, cap);
	if (conditions == NULL) {
		*ret_status = FF_FAILURE;
	}

	*ret_status = FF_SUCCESS;
	return (FfList){
		.conditions = conditions,
		.cap = cap,
		.len = 0
	};
}

void ff_list_destroy(FfList list)
{
	for (size_t i = 0; i < list.len; ++i) {
		ff_condition_unref(list.conditions[i]);
	}

	tyrant_free(list.conditions);
}

bool ff_list_add(FfList *list, FfCondition *condition)
{
	if (list->len == list->cap) {
		if (list->cap == ULLONG_MAX) {
			return NULL;
		}

		size_t cap;
		if (list->cap > ULLONG_MAX / 2) {
			cap = ULLONG_MAX;
		} else {
			cap = list->len * 2;
		}

		bool success;
		list->conditions = TYRANT_REALLOC_ARR(list->conditions, cap,
				&success);
		if (!success) {
			return false;
		}

		list->cap = cap;
	}

	list->conditions[list->len++] = ff_condition_ref(condition);

	return true;
}

bool ff_list_add_unref(FfList *list, FfCondition *condition)
{
	bool success = ff_list_add(list, condition);

	ff_condition_unref(condition);

	return success;
}

FcValue ff_create_fc_value(FcType type, ...)
{
	va_list va;
	va_start(va, type);

	FcValue value = ff_create_fc_value_va(type, va);

	va_end(va);

	return value;
}

FcValue ff_create_fc_value_va(FcType type, va_list va)
{
	FcValue value = { .type = type };

	switch (type) {
	case FcTypeInteger:
		value.u.i = va_arg(va, int);
		break;
	case FcTypeDouble:
		value.u.d = va_arg(va, double);
		break;
	case FcTypeString:
		value.u.s = va_arg(va, FcChar8 *);
		break;
	case FcTypeBool:
		value.u.b = va_arg(va, FcBool);
		break;
	case FcTypeMatrix:
		value.u.m = va_arg(va, FcMatrix *);
		break;
	case FcTypeCharSet:
		value.u.c = va_arg(va, FcCharSet *);
		break;
	case FcTypeFTFace:
		value.u.f = va_arg(va, void *);
		break;
	case FcTypeLangSet:
		value.u.l = va_arg(va, FcLangSet *);
		break;
	case FcTypeRange:
		value.u.r = va_arg(va, FcRange *);
		break;
	case FcTypeUnknown:
	case FcTypeVoid:
		break;
	default:
		value.type = FcTypeUnknown;
	}

	return value;
}

bool ff_condition_test_fc_pattern(FfCondition *condition, FcPattern *pattern)
{
	switch (condition->type) {
	case FF_COMPARISON:
		return test_comparison(condition->value.comparison, pattern);
	case FF_COMPOSITION:
		return test_composition(condition->value.composition, pattern);
	default:
		return false;
	}
}

bool ff_list_test_fc_pattern(FfList list, FcPattern *pattern)
{
	for (size_t i = 0; i < list.len; ++i) {
		FfCondition *condition = list.conditions[i];
		if (!ff_condition_test_fc_pattern(condition, pattern)) {
			return false;
		}
	}

	return true;
}

FcFontSet *ff_condition_filter(FfCondition *condition, FcFontSet *set)
{
	FcFontSet *filtered = FcFontSetCreate();
	if (filtered == NULL) {
		goto err_exit;
	}

	for (int i = 0; i < set->nfont; ++i) {
		FcPattern *font = set->fonts[i];

		if (ff_condition_test_fc_pattern(condition, font)) {
			FcPatternReference(font);
			bool success = FcFontSetAdd(filtered, font);
			if (!success) {
				goto err_destroy_filtered;
			}
		}
	}

	return filtered;

err_destroy_filtered:
	FcFontSetDestroy(filtered);
err_exit:
	return NULL;
}

FcFontSet *ff_list_filter(FfList list, FcFontSet *set)
{
	FcFontSet *filtered = FcFontSetCreate();
	if (filtered == NULL) {
		goto err_exit;
	}

	for (int i = 0; i < set->nfont; ++i) {
		FcPattern *font = set->fonts[i];
		if (ff_list_test_fc_pattern(list, font)) {
			FcPatternReference(font);
			bool success = FcFontSetAdd(filtered, font);
			if (!success) {
				goto err_destroy_filtered;
			}
		}
	}

	return filtered;

err_destroy_filtered:
	FcFontSetDestroy(filtered);
err_exit:
	return NULL;
}

FcFontSet *ff_list_filter_soft(FfList list, FcFontSet *set)
{
	FcFontSet *filtered = FcFontSetCreate();
	if (filtered == NULL) {
		goto err_exit;
	}

	for (int i = 0; i < set->nfont; ++i) {
		FcPattern *font = set->fonts[i];

		FcPatternReference(font);
		bool success = FcFontSetAdd(filtered, font);
		if (success) {
			goto err_destroy_filtered;
		}
	}

	for (size_t i = 0; i < list.len; ++i) {
		FfCondition *condition = list.conditions[i];
		FcFontSet *test_set = ff_condition_filter(condition, filtered);
		if (test_set == NULL) {
			goto err_destroy_filtered;
		}

		if (test_set->nfont > 0) {
			FcFontSetDestroy(filtered);
			filtered = test_set;
		}
	}

	return filtered;

err_destroy_filtered:
	FcFontSetDestroy(filtered);
err_exit:
	return NULL;
}

bool ff_eval_logical_operation(FfLogicalOperator operator, bool p, bool q)
{
	if (p && q) {
		return operator.pt_qt;
	}
	if (p && !q) {
		return operator.pt_qf;
	}
	if (!p && q) {
		return operator.pf_qt;
	}
	return operator.pf_qf;
}

bool test_comparison(FfComparison comparison, FcPattern *pattern)
{
	FcValue value;
	FcResult result = FcPatternGet(pattern, comparison.object, 0, &value);
	if (result != FcResultMatch) {
		return false;
	}

	return test_comparison_for_value(comparison, value);
}

bool test_composition(FfLogicalComposition composition, FcPattern *pattern)
{
	bool p_passed = ff_condition_test_fc_pattern(composition.p, pattern);
	bool q_passed = ff_condition_test_fc_pattern(composition.q, pattern);

	return ff_eval_logical_operation(composition.operator, p_passed,
			q_passed);
}

bool test_comparison_for_value(FfComparison comparison, FcValue value)
{
	FcValue a = value;
	FcValue b = comparison.value;
	FfRelationalOperator operator = comparison.operator;

	bool a_is_real = a.type == FcTypeInteger || a.type == FcTypeDouble;
	bool b_is_real = b.type == FcTypeInteger || b.type == FcTypeDouble;
	if (a_is_real && b_is_real) {
		double a_d = a.type == FcTypeDouble ? a.u.d : a.u.i;
		double b_d = b.type == FcTypeDouble ? b.u.d : b.u.i;

		switch (operator) {
		case FF_NOT_EQUAL:
			return a_d != b_d;
		case FF_EQUAL:
			return a_d == b_d;
		case FF_LESS_THAN:
			return a_d < b_d;
		case FF_GREATER_THAN:
			return a_d > b_d;
		case FF_LESS_THAN_EQUAL:
			return a_d <= b_d;
		case FF_GREATER_THAN_EQUAL:
			return a_d >= b_d;
		}
	}

	switch (operator) {
	case FF_NOT_EQUAL:
		return !FcValueEqual(a, b);
	case FF_EQUAL:
		return FcValueEqual(a, b);
	default:
		return false;
	}
}
