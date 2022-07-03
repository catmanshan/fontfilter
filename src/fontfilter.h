#ifndef fontfilter_h
#define fontfilter_h

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

#include <fontconfig/fontconfig.h>

#define FF_SUCCESS 0
#define FF_FAILURE (-1)

typedef enum FfConditionType {
	FF_COMPARISON,
	FF_COMPOSITION
} FfConditionType;

typedef enum FfRelationalOperator {
	FF_NOT_EQUAL = 0,
	FF_EQUAL,
	FF_LESS_THAN,
	FF_GREATER_THAN,
	FF_LESS_THAN_EQUAL,
	FF_GREATER_THAN_EQUAL,
} FfRelationalOperator;

typedef struct FfLogicalOperator FfLogicalOperator;
typedef struct FfComparison FfComparison;
typedef struct FfLogicalComposition FfLogicalComposition;
typedef union FfConditionValue FfConditionValue;
typedef struct FfCondition FfCondition;
typedef struct FfList FfList;

struct FfLogicalOperator {
	bool pt_qt;
	bool pt_qf;
	bool pf_qt;
	bool pf_qf;
};

//                                           PTQT  PTQF  PFQT  PFQF
#define FF_ALWAYS_FALSE (FfLogicalOperator){    0,    0,    0,    0 }
#define FF_NOR          (FfLogicalOperator){    0,    0,    0,    1 }
#define FF_Q_NOT_P      (FfLogicalOperator){    0,    0,    1,    0 }
#define FF_NOT_P        (FfLogicalOperator){    0,    0,    1,    1 }
#define FF_P_NOT_Q      (FfLogicalOperator){    0,    1,    0,    0 }
#define FF_NOT_Q        (FfLogicalOperator){    0,    1,    0,    1 }
#define FF_XOR          (FfLogicalOperator){    0,    1,    1,    0 }
#define FF_NAND         (FfLogicalOperator){    0,    1,    1,    1 }
#define FF_AND          (FfLogicalOperator){    1,    0,    0,    0 }
#define FF_XNOR         (FfLogicalOperator){    1,    0,    0,    1 }
#define FF_Q            (FfLogicalOperator){    1,    0,    1,    0 }
#define FF_IF_P_THEN_Q  (FfLogicalOperator){    1,    0,    1,    1 }
#define FF_P            (FfLogicalOperator){    1,    1,    0,    0 }
#define FF_IF_Q_THEN_P  (FfLogicalOperator){    1,    1,    0,    1 }
#define FF_OR           (FfLogicalOperator){    1,    1,    1,    0 }
#define FF_ALWAYS_TRUE  (FfLogicalOperator){    1,    1,    1,    1 }

struct FfComparison {
	const char *object;
	FcValue value;
	FfRelationalOperator operator;
};

struct FfLogicalComposition {
	FfLogicalOperator operator;
	FfCondition *p;
	FfCondition *q;
};

union FfConditionValue {
	FfComparison comparison;
	FfLogicalComposition composition;
};

struct FfCondition {
	FfConditionType type;
	FfConditionValue value;

	size_t ref_count;
};

struct FfList {
	FfCondition **conditions;
	size_t len;
	size_t cap;
};

FfCondition *ff_compare(const char *object, FfRelationalOperator operator,
		FcType type, ...);
FfCondition *ff_compare_value(const char *object, FfRelationalOperator operator,
		FcValue value);
FfCondition *ff_compose(FfCondition *p, FfLogicalOperator operator,
		FfCondition *q);

FfCondition *ff_compose_unref(FfCondition *p, FfLogicalOperator operator,
		FfCondition *q);

FfCondition *ff_condition_ref(FfCondition *condition);
void ff_condition_unref(FfCondition *condition);

FfList ff_list_create(int *ret_status);
FfList ff_list_create_with_cap(int cap, int *ret_status);
void ff_list_destroy(FfList list);
bool ff_list_add(FfList *list, FfCondition *condition);
bool ff_list_add_unref(FfList *list, FfCondition *condition);

FcValue ff_create_fc_value(FcType type, ...);
FcValue ff_create_fc_value_va(FcType type, va_list va);

bool ff_condition_test_fc_pattern(FfCondition *condition, FcPattern *pattern);
bool ff_list_test_fc_pattern(FfList list, FcPattern *pattern);

FcFontSet *ff_condition_filter(FfCondition *condition, FcFontSet *set);
FcFontSet *ff_list_filter(FfList list, FcFontSet *set);

FcFontSet *ff_list_filter_soft(FfList list, FcFontSet *set);

#endif // fontfilter_h
