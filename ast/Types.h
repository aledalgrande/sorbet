#ifndef SRUBY_TYPES_H
#define SRUBY_TYPES_H

#include "Context.h"
#include "Symbols.h"
#include <memory>
#include <string>

namespace ruby_typer {
namespace ast {
/** Dmitry: unlike in Dotty, those types are always dealiased. For now */
class Type;
class Types {
public:
    /** Greater lower bound: the widest type that is subtype of both t1 and t2 */
    static std::shared_ptr<Type> glb(ast::Context ctx, std::shared_ptr<Type> &t1, std::shared_ptr<Type> &t2);
    inline static std::shared_ptr<Type> glb(ast::Context ctx, std::shared_ptr<Type> &t1, std::shared_ptr<Type> &&t2) {
        return glb(ctx, t1, t2);
    }
    inline static std::shared_ptr<Type> glb(ast::Context ctx, std::shared_ptr<Type> &&t1, std::shared_ptr<Type> &t2) {
        return glb(ctx, t1, t2);
    }
    inline static std::shared_ptr<Type> glb(ast::Context ctx, std::shared_ptr<Type> &&t1, std::shared_ptr<Type> &&t2) {
        return glb(ctx, t1, t2);
    }

    /** Lower upper bound: the narrowest type that is supper type of both t1 and t2 */
    static std::shared_ptr<Type> lub(ast::Context ctx, std::shared_ptr<Type> &t1, std::shared_ptr<Type> &t2);
    inline static std::shared_ptr<Type> lub(ast::Context ctx, std::shared_ptr<Type> &t1, std::shared_ptr<Type> &&t2) {
        return lub(ctx, t1, t2);
    }
    inline static std::shared_ptr<Type> lub(ast::Context ctx, std::shared_ptr<Type> &&t1, std::shared_ptr<Type> &t2) {
        return lub(ctx, t1, t2);
    }
    inline static std::shared_ptr<Type> lub(ast::Context ctx, std::shared_ptr<Type> &&t1, std::shared_ptr<Type> &&t2) {
        return lub(ctx, t1, t2);
    }

    /** is every instance of  t1 an  instance of t2? */
    static bool isSubType(ast::Context ctx, std::shared_ptr<Type> &t1, std::shared_ptr<Type> &t2);
    inline static bool isSubType(ast::Context ctx, std::shared_ptr<Type> &t1, std::shared_ptr<Type> &&t2) {
        return isSubType(ctx, t1, t2);
    }
    inline static bool isSubType(ast::Context ctx, std::shared_ptr<Type> &&t1, std::shared_ptr<Type> &t2) {
        return isSubType(ctx, t1, t2);
    }
    inline static bool isSubType(ast::Context ctx, std::shared_ptr<Type> &&t1, std::shared_ptr<Type> &&t2) {
        return isSubType(ctx, t1, t2);
    }

    static bool equiv(ast::Context ctx, std::shared_ptr<Type> &t1, std::shared_ptr<Type> &t2);
    inline static bool equiv(ast::Context ctx, std::shared_ptr<Type> &t1, std::shared_ptr<Type> &&t2) {
        return equiv(ctx, t1, t2);
    }
    inline static bool equiv(ast::Context ctx, std::shared_ptr<Type> &&t1, std::shared_ptr<Type> &t2) {
        return equiv(ctx, t1, t2);
    }
    inline static bool equiv(ast::Context ctx, std::shared_ptr<Type> &&t1, std::shared_ptr<Type> &&t2) {
        return equiv(ctx, t1, t2);
    }

    static std::shared_ptr<Type> top();
    static std::shared_ptr<Type> bottom();
    static std::shared_ptr<Type> nil();
    static std::shared_ptr<Type> dynamic();
};

class TypeAndOrigins {
public:
    std::shared_ptr<ast::Type> type;
    std::vector<ast::Loc> origins; // todo: use tiny vector
    std::vector<ast::Reporter::ErrorLine> origins2Explanations(ast::Context ctx) {
        std::vector<ast::Reporter::ErrorLine> result;
        for (auto o : origins) {
            result.emplace_back(o, "");
        }
        return result;
    }
};

class Type {
public:
    Type() = default;
    Type(const Type &obj) = delete;
    virtual ~Type() = default;
    virtual std::string toString(ast::Context ctx, int tabs = 0) = 0;
    virtual std::string typeName() = 0;
    virtual std::shared_ptr<Type> dispatchCall(ast::Context ctx, ast::NameRef name, ast::Loc callLoc,
                                               std::vector<TypeAndOrigins> &args, std::shared_ptr<Type> fullType) = 0;
    virtual std::shared_ptr<Type> getCallArgumentType(ast::Context ctx, ast::NameRef name, int i) = 0;
    bool isDynamic();
};

class GroundType : public Type {
public:
    virtual int kind() = 0;
};

class ProxyType : public Type {
public:
    // TODO: use shared pointers that use inline counter
    std::shared_ptr<Type> underlying;
    ProxyType(std::shared_ptr<Type> underlying);

    virtual std::shared_ptr<Type> dispatchCall(ast::Context ctx, ast::NameRef name, ast::Loc callLoc,
                                               std::vector<TypeAndOrigins> &args, std::shared_ptr<Type> fullType);
    virtual std::shared_ptr<Type> getCallArgumentType(ast::Context ctx, ast::NameRef name, int i);
};

class ClassType : public GroundType {
public:
    ast::SymbolRef symbol;
    ClassType(ast::SymbolRef symbol);
    virtual int kind();

    virtual std::string toString(ast::Context ctx, int tabs = 0);
    virtual std::string typeName();
    virtual std::shared_ptr<Type> dispatchCall(ast::Context ctx, ast::NameRef name, ast::Loc callLoc,
                                               std::vector<TypeAndOrigins> &args, std::shared_ptr<Type> fullType);
    virtual std::shared_ptr<Type> getCallArgumentType(ast::Context ctx, ast::NameRef name, int i);
};

class OrType : public GroundType {
public:
    std::shared_ptr<Type> left;
    std::shared_ptr<Type> right;
    virtual int kind();
    OrType(std::shared_ptr<Type> left, std::shared_ptr<Type> right);

    virtual std::string toString(ast::Context ctx, int tabs = 0);
    virtual std::string typeName();
    virtual std::shared_ptr<Type> dispatchCall(ast::Context ctx, ast::NameRef name, ast::Loc callLoc,
                                               std::vector<TypeAndOrigins> &args, std::shared_ptr<Type> fullType);
    virtual std::shared_ptr<Type> getCallArgumentType(ast::Context ctx, ast::NameRef name, int i);
};

class AndType : public GroundType {
public:
    std::shared_ptr<Type> left;
    std::shared_ptr<Type> right;
    virtual int kind();
    AndType(std::shared_ptr<Type> left, std::shared_ptr<Type> right);

    virtual std::string toString(ast::Context ctx, int tabs = 0);
    virtual std::string typeName();
    virtual std::shared_ptr<Type> dispatchCall(ast::Context ctx, ast::NameRef name, ast::Loc callLoc,
                                               std::vector<TypeAndOrigins> &args, std::shared_ptr<Type> fullType);

    virtual std::shared_ptr<Type> getCallArgumentType(ast::Context ctx, ast::NameRef name, int i);
};

class Literal : public ProxyType {
public:
    int value;
    Literal(int val);
    Literal(float val);
    Literal(ast::NameRef val);
    Literal(bool val);

    virtual std::string toString(ast::Context ctx, int tabs = 0);
    virtual std::string typeName();
};

class HashType : public ProxyType {
public:
    std::vector<std::shared_ptr<Literal>> keys; // TODO: store sorted by whatever
    std::vector<std::shared_ptr<Type>> values;
    HashType(std::vector<std::shared_ptr<Literal>> &keys, std::vector<std::shared_ptr<Type>> &values);

    virtual std::string toString(ast::Context ctx, int tabs = 0);
    virtual std::string typeName();
};

class ArrayType : public ProxyType {
public:
    std::vector<std::shared_ptr<Type>> elems;
    ArrayType(std::vector<std::shared_ptr<Type>> &elems);

    virtual std::string toString(ast::Context ctx, int tabs = 0);
    virtual std::string typeName();
};
} // namespace ast
} // namespace ruby_typer
#endif // SRUBY_TYPES_H
