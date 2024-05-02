#include <lua.h>
#include <lualib.h>
#include <luacode.h>
#include <string>
#include <Luau/CodeGen.h>
#include <Luau/Compiler.h>

// I copy pasted this from a source file of luau and tweaked it , i forgot which one it was lol

void SetupState(lua_State *L, bool sandbox_libs = false) {
    luaL_openlibs(L);
    if (sandbox_libs)
        luaL_sandbox(L);
}

void RegisterFunctions(lua_State *L) {
    // register ur custom funcs here
}

static Luau::CompileOptions copts()
{
    Luau::CompileOptions result = {};
    result.optimizationLevel = 0;
    result.debugLevel = 0;

    return result;
}

std::string runCode(lua_State *L, const std::string &code) {
    const auto& bytecode = Luau::compile(code, copts(), {});
    int result = luau_load(L, "=main", bytecode.c_str(), bytecode.length(), 0);
    if (result != LUA_OK)
    {
        size_t len;
        const char *msg = lua_tolstring(L, -1, &len);

        std::string error(msg, len);
        lua_pop(L, 1);

        return error;
    }

    lua_State *T = lua_newthread(L);

    lua_pushvalue(L, -2);
    lua_remove(L, -3);
    lua_xmove(L, T, 1);

    int status = lua_resume(T, NULL, 0);

    if (status == 0)
    {
        int n = lua_gettop(T);

        if (n)
        {
            luaL_checkstack(T, LUA_MINSTACK, "too many results to print");
            lua_getglobal(T, "error");
            lua_insert(T, 1);
            lua_pcall(T, n, 0, 0);
        }

        lua_pop(L, 1); // pop T
        return std::string();
    }
    else
    {
        std::string error;

        lua_Debug ar;
        if (lua_getinfo(L, 0, "sln", &ar))
        {
            error += ar.short_src;
            error += ':';
            error += std::to_string(ar.currentline);
            error += ": ";
        }

        if (status == LUA_YIELD)
        {
            error += "thread yielded unexpectedly";
        }
        else if (const char *str = lua_tostring(T, -1))
        {
            error += str;
        }

        error += "\nstack backtrace:\n";
        error += lua_debugtrace(T);

        lua_pop(L, 1); // pop T
        return error;
    }
}

std::string executeScript(std::string script, bool sandbox_libs = false) {
    for (Luau::FValue<bool>* flag = Luau::FValue<bool>::list; flag; flag = flag->next)
    if (strncmp(flag->name, "Luau", 4) == 0)
        flag->value = true;
    
    std::unique_ptr<lua_State, void (*)(lua_State*)> globalState(luaL_newstate(), lua_close);
    lua_State* L = globalState.get();

    // setup state
    SetupState(L);

    // sandbox thread
    if (sandbox_libs)
        luaL_sandboxthread(L);

    // static string for caching result (prevents dangling ptr on function exit)
    static std::string result;
    RegisterFunctions(L);
    // run code + collect error
    result = runCode(L, script);

    return result.empty() ? "" : result;
}

void main()
{
    executeScript("print('hello world !')");
}