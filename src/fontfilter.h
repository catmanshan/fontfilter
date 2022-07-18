#ifndef fontfilter_h
#define fontfilter_h

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

#include <fontconfig/fontconfig.h>

#define FF_SUCCESS 0
#define FF_FAILURE (-1)

//                                          PTQT PTQF PFQT PFQF
#define FF_ALWAYS_FALSE (FfLogicalOperator){   0,   0,   0,   0 }
#define FF_NOR          (FfLogicalOperator){   0,   0,   0,   1 }
#define FF_Q_NOT_P      (FfLogicalOperator){   0,   0,   1,   0 }
#define FF_NOT_P        (FfLogicalOperator){   0,   0,   1,   1 }
#define FF_P_NOT_Q      (FfLogicalOperator){   0,   1,   0,   0 }
#define FF_NOT_Q        (FfLogicalOperator){   0,   1,   0,   1 }
#define FF_XOR          (FfLogicalOperator){   0,   1,   1,   0 }
#define FF_NAND         (FfLogicalOperator){   0,   1,   1,   1 }
#define FF_AND          (FfLogicalOperator){   1,   0,   0,   0 }
#define FF_XNOR         (FfLogicalOperator){   1,   0,   0,   1 }
#define FF_Q            (FfLogicalOperator){   1,   0,   1,   0 }
#define FF_IF_P_THEN_Q  (FfLogicalOperator){   1,   0,   1,   1 }
#define FF_P            (FfLogicalOperator){   1,   1,   0,   0 }
#define FF_IF_Q_THEN_P  (FfLogicalOperator){   1,   1,   0,   1 }
#define FF_OR           (FfLogicalOperator){   1,   1,   1,   0 }
#define FF_ALWAYS_TRUE  (FfLogicalOperator){   1,   1,   1,   1 }

typedef enum FfConditionType {
	FF_COMPARISON,
	FF_COMPOSITION,
	FF_CHAR_REQUIREMENT
} FfConditionType;

typedef enum FfRelationalOperator {
	FF_NOT_EQUAL = 0,
	FF_EQUAL,
	FF_LESS_THAN,
	FF_GREATER_THAN,
	FF_LESS_THAN_EQUAL,
	FF_GREATER_THAN_EQUAL,
	FF_CONTAINS,
	FF_DOES_NOT_CONTAIN,
	FF_CONTAINED_IN,
	FF_NOT_CONTAINED_IN
} FfRelationalOperator;

typedef struct FfLogicalOperator FfLogicalOperator;
typedef struct FfComparison FfComparison;
typedef struct FfLogicalComposition FfLogicalComposition;
typedef struct FfCharRequirement FfCharRequirement;
typedef union FfConditionValue FfConditionValue;
typedef struct FfCondition FfCondition;
typedef struct FfList FfList;

struct FfLogicalOperator {
	bool pt_qt;
	bool pt_qf;
	bool pf_qt;
	bool pf_qf;
};

struct FfComparison {
	const char *object;
	FcValue value;
	FfRelationalOperator oper;
};

struct FfLogicalComposition {
	FfLogicalOperator oper;
	FfCondition *p;
	FfCondition *q;
};

struct FfCharRequirement {
	FcChar32 c;
};

union FfConditionValue {
	FfComparison comparison;
	FfLogicalComposition composition;
	FfCharRequirement char_requirement;
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

/// Converts the (first and only) variadic argument to an `FcValue` with the
/// given type and calls `ff_compare_value()`.
FfCondition *ff_compare(const char *object, FfRelationalOperator oper,
		FcType type, ...);

/// Creates a condition representing a comparison between `value` and the value
/// which is associated with the property `object` for some pattern.
FfCondition *ff_compare_value(const char *object, FfRelationalOperator oper,
		FcValue value);

/// Creates a condition representing a logical operation between two conditions.
FfCondition *ff_compose(FfCondition *p, FfLogicalOperator oper, FfCondition *q);

/// Creates a condition representing a requirement that a pattern's charset
/// contains `c`.
FfCondition *ff_require_char(FcChar32 c);

/// Calls `ff_compose()` and decrements the reference counts of `p` and `q`.
/**
 * Intention is to transfer "ownership" of the references to the resulting
 * condition.
 *
 * Reference counts are decremented unconditionally (i.e. even if `ff_compose()`
 * fails).
 */
FfCondition *ff_compose_unref(FfCondition *p, FfLogicalOperator oper,
		FfCondition *q);

/// Increments `condition`'s reference count.
FfCondition *ff_condition_ref(FfCondition *condition);

/// Decrements `condition`'s reference count and destroys it if the count
/// becomes zero.
void ff_condition_unref(FfCondition *condition);

/// Creates a list.
FfList ff_list_create(int *ret_status);

/// Creates a list with the given initial capacity.
FfList ff_list_create_with_cap(int init_cap, int *ret_status);

/// Destroys `list`.
void ff_list_destroy(FfList list);

/// Adds `condition` to the end of `list`.
bool ff_list_add(FfList *list, FfCondition *condition);

/// Calls `ff_list_add()` and decrements `condition`'s reference count.
/**
 * Intention is to transfer "ownership" of the reference to the list.
 *
 * Reference count is decremented unconditionally (i.e. even if `ff_compose()`
 * fails).
 */
bool ff_list_add_unref(FfList *list, FfCondition *condition);

/// Converts the (first and only) variadic argument to an `FcValue` with the
/// given type.
FcValue ff_create_fc_value(FcType type, ...);

/// Converts the (first and only) argument given by `va` to an `FcValue` with
/// the given type.
FcValue ff_create_fc_value_va(FcType type, va_list va);

/// Tests whether a pattern satisfies a condition.
bool ff_condition_test_fc_pattern(FfCondition *condition, FcPattern *pattern);

/// Tests whether a pattern satisfies a list of conditions.
bool ff_list_test_fc_pattern(FfList list, FcPattern *pattern);

/// Creates a font set containing all the fonts in `set` which satisfy
/// `condition`.
FcFontSet *ff_condition_filter(FfCondition *condition, FcFontSet *set);

/// Returns a font set containing all the fonts in `set` which satisfy a list of
/// conditions.
FcFontSet *ff_list_filter(FfList list, FcFontSet *set);

/// Returns a font set containing all the fonts in `set` which satisfy each
/// condition in `list`, but skips any conditions which would result in an empty
/// set.
/**
 * Conditions are evaluated in order.
 */
FcFontSet *ff_list_filter_soft(FfList list, FcFontSet *set);

#endif // fontfilter_h
