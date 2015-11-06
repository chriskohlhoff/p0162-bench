#ifndef OS_HPP
#define OS_HPP

#ifdef OS_HAS_DLL
# ifdef OS_BUILD_DLL
#  define OS_DECL extern "C" __declspec(dllexport)
# else
#  define OS_DECL extern "C" __declspec(dllimport)
# endif
#else
# define OS_DECL
#endif

using OsResultType = int;

struct OsContext
{
};

using CallbackFnPtr = void(*)(OsResultType r, OsContext*);

OS_DECL void os_associate_completion_callback(CallbackFnPtr cb);
OS_DECL void os_trigger_completion();

using ParamType = int;
OS_DECL void os_xyz(ParamType p, OsContext* o);
OS_DECL int os_get_xyz_count();

#endif
