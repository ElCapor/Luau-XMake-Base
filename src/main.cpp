#include <lua.h>
#include <lualib.h>
#include <Luau/CodeGen.h>
#include <Luau/BytecodeBuilder.h>
#include <Luau/Compiler.h>

static Luau::CompileOptions copts()
{
    Luau::CompileOptions result = {};
    result.optimizationLevel = 0;
    result.debugLevel = 0;

    return result;
}

void main()
{
    lua_State* L = luaL_newstate();
    // example of compiling and building bytecode
    const auto& bytecode = Luau::compile("print(\"Hello World !\")", copts(), {});
    int result = luau_load(L, "main", bytecode.c_str(), bytecode.length(), 0);
    lua_close(L);
}