#ifndef fontfilter_h
#define fontfilter_h

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

#endif // fontfilter_h
