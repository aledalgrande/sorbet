#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
// ^^^ violate our poisons so they go first
#include "absl/synchronization/mutex.h"
#include "ast/ast.h"
#include "cfg/CFG.h"
#include "compiler/DefinitionRewriter/DefinitionRewriter.h"
#include "compiler/IRHelpers/IRHelpers.h"
#include "compiler/LLVMIREmitter/LLVMIREmitter.h"
#include "compiler/ObjectFileEmitter/ObjectFileEmitter.h"
#include "core/ErrorQueue.h"
#include "core/errors/errors.h"
#include "main/pipeline/semantic_extension/SemanticExtension.h"
#include <cxxopts.hpp>
#include <optional>

using namespace std;
namespace sorbet::pipeline::semantic_extension {
namespace {
string fileName2ObjectName(string sourceFile) {
    absl::c_replace(sourceFile, '/', '_');
    return sourceFile;
}
} // namespace

class ThreadState {
public:
    llvm::LLVMContext lctx;
    unique_ptr<llvm::Module> combinedModule;
};

class LLVMSemanticExtension : public SemanticExtension {
    optional<string> irOutputDir;
    mutable struct {
        UnorderedMap<std::thread::id, shared_ptr<ThreadState>> states;
        absl::Mutex mtx;
    } mutableState;

    shared_ptr<ThreadState> getThreadState() const {
        {
            absl::ReaderMutexLock lock(&mutableState.mtx);
            if (mutableState.states.contains(std::this_thread::get_id())) {
                return mutableState.states.at(std::this_thread::get_id());
            }
        }
        {
            absl::WriterMutexLock lock(&mutableState.mtx);
            return mutableState.states[std::this_thread::get_id()] = make_shared<ThreadState>();
        }
    }

public:
    LLVMSemanticExtension(optional<string> irOutputDir) {
        this->irOutputDir = move(irOutputDir);
    }

    virtual void finishTypecheckFile(const core::GlobalState &gs, const core::FileRef &f) const override {
        auto threadState = getThreadState();
        llvm::LLVMContext &lctx = threadState->lctx;
        unique_ptr<llvm::Module> &module = threadState->combinedModule;
        if (module) {
            string fileName = fileName2ObjectName((string)f.data(gs).path());
            sorbet::compiler::ObjectFileEmitter::run(lctx, move(module), irOutputDir.value(), fileName);
        }
    };
    virtual void typecheck(const core::GlobalState &gs, cfg::CFG &cfg,
                           std::unique_ptr<ast::MethodDef> &md) const override {
        if (gs.errorQueue->nonSilencedErrorCount > 0) {
            return;
        }

        auto threadState = getThreadState();
        llvm::LLVMContext &lctx = threadState->lctx;
        string functionName = cfg.symbol.data(gs)->toStringFullName(gs);
        unique_ptr<llvm::Module> &module = threadState->combinedModule;
        if (!module) {
            module = sorbet::compiler::IRHelpers::readDefaultModule(functionName.data(), lctx);
        }
        compiler::CompilerState state(gs, lctx, module.get());
        sorbet::compiler::LLVMIREmitter::run(state, cfg, md, functionName);
        string fileName = fileName2ObjectName((string)cfg.symbol.data(gs)->loc().file().data(state).path());
        sorbet::compiler::LLVMIREmitter::buildInitFor(state, cfg.symbol, fileName);
    };
    virtual void run(core::MutableContext &ctx, ast::ClassDef *klass) const override {
        if (!irOutputDir.has_value()) {
            return;
        }
        if (klass->loc.file().data(ctx).strictLevel < core::StrictLevel::True) {
            if (auto e = ctx.state.beginError(klass->loc, core::errors::Internal::InternalError)) {
                e.setHeader("File must be `typed: true` or higher to be compiled");
            }
        }
        if (!ast::isa_tree<ast::EmptyTree>(klass->name.get())) {
            return;
        }

        sorbet::compiler::DefinitionRewriter::run(ctx, klass);
    };
    virtual ~LLVMSemanticExtension(){};
    virtual std::unique_ptr<SemanticExtension> deepCopy(const core::GlobalState &from, core::GlobalState &to) override {
        return make_unique<LLVMSemanticExtension>(this->irOutputDir);
    };
    virtual void merge(const core::GlobalState &from, core::GlobalState &to, core::GlobalSubstitution &subst) override {
    }
};

class LLVMSemanticExtensionProvider : public SemanticExtensionProvider {
public:
    virtual void injectOptions(cxxopts::Options &optsBuilder) const override {
        optsBuilder.add_options("lvm")("llvm-ir-folder", "Output LLVM IR to directory", cxxopts::value<string>());
    };
    virtual std::unique_ptr<SemanticExtension> readOptions(cxxopts::ParseResult &providedOptions) const override {
        optional<string> irOutputDir;
        if (providedOptions.count("llvm-ir-folder") > 0) {
            irOutputDir = providedOptions["llvm-ir-folder"].as<string>();
        }
        return make_unique<LLVMSemanticExtension>(irOutputDir);
    };
    virtual ~LLVMSemanticExtensionProvider(){};
};

vector<SemanticExtensionProvider *> SemanticExtensionProvider::getProviders() {
    static LLVMSemanticExtensionProvider provider;
    return {&provider};
}
} // namespace sorbet::pipeline::semantic_extension
