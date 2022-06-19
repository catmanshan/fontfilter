#ifndef fontfilter_h
#define fontfilter_h

#include <stdarg.h>
#include <stdbool.h>

#include <fontconfig/fontconfig.h>

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
};

FfCondition ff_compare(const char *object, FfOperation operation, ...);
FfCondition ff_compare_value(const char *object, FfOperation operation,
		FcValue value);

FcValue ff_create_fc_value(FcType type, ...);
FcValue ff_create_fc_value_va(FcType type, va_list va);

#endif // fontfilter_h
