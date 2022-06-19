#include "fontfilter.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

#include <fontconfig/fontconfig.h>

#include <tyrant.h>

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
	};
	return condition;
}

FfCondition *ff_compose(FfCondition *p, FfTruthTable truth_table,
		FfCondition *q)
{
	FfCondition *condition = tyrant_alloc(sizeof(*condition));
	if (condition == NULL) {
		return NULL;
	}

	*condition = (FfCondition){
		.type = FF_COMPOSITION,
		.value.composition = (FfComposition){
			.p = p,
			.q = q,
			.truth_table = truth_table
		}
	};
	return condition;
}

void ff_condition_destroy(FfCondition *condition)
{
	tyrant_free(condition);
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
