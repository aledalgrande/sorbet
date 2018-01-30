#include "ast/ast.h"
#include "ast/desugar/Desugar.h"
#include "common/common.h"
#include "core/Unfreeze.h"
#include "parser/parser.h"
#include "spdlog/spdlog.h"
#include "gtest/gtest.h"
#include <fstream>
#include <memory>
#include <string>
#include <vector>

namespace spd = spdlog;

TEST(DesugarTest, SimpleDesugar) { // NOLINT
    auto console = spd::stderr_color_mt("console");
    ruby_typer::core::GlobalState gs(*console);
    gs.initEmpty();
    ruby_typer::core::UnfreezeNameTable nameTableAccess(gs);
    ruby_typer::core::UnfreezeFileTable ft(gs);

    auto ast = ruby_typer::parser::Parser::run(gs, "<test>", "def hello_world; p :hello; end");
    ruby_typer::core::Context ctx(gs, gs.defn_root());
    auto o1 = ruby_typer::ast::desugar::node2Tree(ctx, move(ast));
}
