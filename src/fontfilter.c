#include "fontfilter.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

#include <fontconfig/fontconfig.h>

#include <tyrant.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))

static void destroy_condition(FfCondition *condition);
static bool inc_ref_count(unsigned long long *ref_count);
static bool dec_ref_count(unsigned long long *ref_count);

FfCondition *ff_compare(const char *object, FfOperation operation, ...)
{
	va_list va;
	va_start(va, operation);

	FcType type = FcNameGetObjectType(object)->type;
	FcValue value = ff_create_fc_value_va(type, va);

	va_end(va);

	return ff_compare_value(object, operation, value);
}

FfCondition *ff_compare_value(const char *object, FfOperation operation,
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
			.operation = operation
		},

		.ref_count = 1
	};
	return condition;
}

FfCondition *ff_compose(FfCondition *p, FfTruthTable truth_table,
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
		.value.composition = (FfComposition){
			.p = p,
			.q = q,
			.truth_table = truth_table
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
	for (unsigned long long i = 0; i < list.len; ++i) {
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

		unsigned long long cap;
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

bool inc_ref_count(unsigned long long *ref_count)
{
	if (*ref_count == ULLONG_MAX) {
		return false;
	}

	++*ref_count;
	return true;
}

bool dec_ref_count(unsigned long long *ref_count)
{
	if (*ref_count == 0) {
		return false;
	}

	--*ref_count;

	return true;
}
