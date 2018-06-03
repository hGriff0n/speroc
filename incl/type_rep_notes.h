
#include <deque>
#include <unordered_map>

#include "spero_string.h"
#include "analysis/SymTable.h"

// NOTE: I can work on the actual interfaces as I develop the type-checker, etc. (I don't know what I'll need)

/*
 * Base class for internal type representation
 */
class Type {
	const Type* cannonical_type;
	spero::String name;
	
	bool is_mutable;
	bool is_view;
	bool is_reference;
	bool is_pointer;

    public:
        Type();
        virtual ~Type();
};

/*
 * Represent a collection of types that are variously considered as a single type for analysis
 */
class TypeList : public virtual Type {
    std::deque<std::unique_ptr<Type>> parts;

    public:
        TypeList();
        virtual ~TypeList();
};

/*
 * Represent an intrinsic function within the internal type system
 *
 * This class exists to provide a recursive base for implementing many aspects of the internal system
 * eg. Functions are represented as a CompositeType (ie. a type with an interface)
 * If part of this interface includes Functions, then there's a possibility for infinite recursion
 */
struct IntrinsicFunction : public virtual Type {
    std::unique_ptr<TupleType> args;
    std::unique_ptr<Type> ret;

    public:
        IntrinsicFunction();
        virtual ~IntrinsicFunction();
};

/*
 * Represent a type that provides a interface (ie. is sub-typable)
 *
 * NOTE: Keeping `interface` helps with determining what "functions" are required for subtypes
 * TODO: Determine how I'll keep track of internal state data
 */
struct CompositeType : public virtual Type {
    bool is_abstract;
	ref_t<spero::analysis::SymTable> interface_scope;
    std::deque<std::unique_ptr<Type>> impls;

    // TODO: Implement allocator for specifying how internal details are organized
    // Allocator allocation_strategy;

    public:
        CompositeType();
        virtual ~CompositeType();
};

/*
 * Represent a type with possibly bound generic parameters
 *
 * NOTE: I'm using "template" here to differentiate between `GenericType` and `AdtType`
 * Both classes require this functionality, but are otherwise separate
 *
 * TODO: How to handle impl and variance relations of parameters
 */
struct GenericParam;
struct TemplateType : public virtual Type {
    std::deque<std::unique_ptr<GenericParam>> params;

    public:
        TemplateType();
        virtual ~TemplateType();
};

/*
 * Represent a usable spero functions
 *
 * NOTE: Not all functions will be represented with this type, as that would create a recursion
 */
struct FunctionType : public CompositeType, public IntrinsicFunction {


    public:
        FunctionType();
        virtual ~FunctionType();
};

/*
 * Extension of functions to better interface with methods
 *
 * NOTE: This type mostly exists to better map how functions will actually be stored
 *   and accessed in the compiler; as special free-functions, not as methods
 */
struct MemberFunction : public FunctionType {
    std::unique_ptr<CompositeType> decl_type;

    public:
        MemberFunction();
        virtual ~MemberFunction();
};

/*
 * Represent an Adt type with it's variety of named constructors
 */
struct AdtType : public CompositeType, public TemplateType {
    std::unordered_map<spero::String, std::unique_ptr<Type>> values;

    public:
        AdtType();
        virtual ~AdtType();
};

/*
 * Represent a tuple type
 */
struct TupleType : public CompositeType, public TypeList {


    public:
        TupleType();
        virtual ~TupleType();
};

/*
 * Represent a type defined within spero itself
 */
struct ClassType : public CompositeType {
    std::deque<std::unique_ptr<IntrinsicFunction>> constructors;

    public:
        ClassType();
        virtual ~ClassType();
};

/*
 * Add generic resolution to spero-defined types
 */
struct GenericType : public ClassType, public TemplateType {


    public:
        GenericType();
        virtual ~GenericType();
};

/*
 * Check membership/satisfaction against a collection of type constraints
 *
 * NOTE: This is separate from `TypeList` to possibly add special analysis behavior
 */
struct JunctionType : public TypeList {
    bool is_conjunction;

    public:
        JunctionType();
        virtual ~JunctionType();
};


/*
 * Base type for generic parameter typing
 *
 * NOTE: I think we should be able to use `String` for TypeParams
 */
struct GenericParam {
	spero::String name;
    std::optional<JunctionType*> interface_req;
};

/*
 * Represent a bondable generic parameter
 *
 * NOTE: A parameter has been "bond" iff `bonded_value.has_value() == true`
 */
template<class BondType>
struct BondableParam : GenericParam {
    std::optional<BondType> bonded_value;
};

/*
 * Type to connect a variable parameter to the bonded value
 */
struct Value {};

// TODO: Determine how the "bound" type name will be held
    // See if I can unify that across VarParam and TypeParam
using TypeParam = BondableParam<Type*>;
using VarParam = BondableParam<Value*>;
