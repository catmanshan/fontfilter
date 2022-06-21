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

typedef enum FfOperation {
	FF_NOT_EQUAL = 0,
	FF_EQUAL,
	FF_LESS_THAN,
	FF_GREATER_THAN,
	FF_LESS_THAN_EQUAL,
	FF_GREATER_THAN_EQUAL,
} FfOperation;

typedef struct FfTruthTable FfTruthTable;
typedef struct FfComparison FfComparison;
typedef struct FfComposition FfComposition;
typedef union FfConditionValue FfConditionValue;
typedef struct FfCondition FfCondition;
typedef struct FfList FfList;

struct FfTruthTable {
	bool pt_qt;
	bool pt_qf;
	bool pf_qt;
	bool pf_qf;
};

//                                      PTQT  PTQF  PFQT  PFQF
#define FF_ALWAYS_FALSE (FfTruthTable){    0,    0,    0,    0 }
#define FF_NOR          (FfTruthTable){    0,    0,    0,    1 }
#define FF_Q_NOT_P      (FfTruthTable){    0,    0,    1,    0 }
#define FF_NOT_P        (FfTruthTable){    0,    0,    1,    1 }
#define FF_P_NOT_Q      (FfTruthTable){    0,    1,    0,    0 }
#define FF_NOT_Q        (FfTruthTable){    0,    1,    0,    1 }
#define FF_XOR          (FfTruthTable){    0,    1,    1,    0 }
#define FF_NAND         (FfTruthTable){    0,    1,    1,    1 }
#define FF_AND          (FfTruthTable){    1,    0,    0,    0 }
#define FF_XNOR         (FfTruthTable){    1,    0,    0,    1 }
#define FF_Q            (FfTruthTable){    1,    0,    1,    0 }
#define FF_IF_P_THEN_Q  (FfTruthTable){    1,    0,    1,    1 }
#define FF_P            (FfTruthTable){    1,    1,    0,    0 }
#define FF_IF_Q_THEN_P  (FfTruthTable){    1,    1,    0,    1 }
#define FF_OR           (FfTruthTable){    1,    1,    1,    0 }
#define FF_ALWAYS_TRUE  (FfTruthTable){    1,    1,    1,    1 }

struct FfComparison {
	const char *object;
	FcValue value;
	FfOperation operation;
};

struct FfComposition {
	FfTruthTable truth_table;
	FfCondition *p;
	FfCondition *q;
};

union FfConditionValue {
	FfComparison comparison;
	FfComposition composition;
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

FfCondition *ff_compare(const char *object, FfOperation operation, ...);
FfCondition *ff_compare_value(const char *object, FfOperation operation,
		FcValue value);
FfCondition *ff_compose(FfCondition *p, FfTruthTable truth_table,
		FfCondition *q);

FfCondition *ff_compose_unref(FfCondition *p, FfTruthTable truth_table,
		FfCondition *q);

FfCondition *ff_condition_ref(FfCondition *condition);
void ff_condition_unref(FfCondition *condition);

FfList ff_list_create(int *ret_status);
FfList ff_list_create_with_cap(int cap, int *ret_status);
void ff_list_destroy(FfList list);
bool ff_list_add(FfList *list, FfCondition *condition);

FcValue ff_create_fc_value(FcType type, ...);
FcValue ff_create_fc_value_va(FcType type, va_list va);

#endif // fontfilter_h
