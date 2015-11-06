#include "os.hpp"

CallbackFnPtr os_associated_callback = nullptr;
OsContext* os_current_context = nullptr;
volatile int os_xyz_count = 0;

void os_associate_completion_callback(CallbackFnPtr cb)
{
  os_associated_callback = cb;
}

void os_trigger_completion()
{
  os_associated_callback(0, os_current_context);
}

void os_xyz(ParamType p, OsContext* o)
{
  ++os_xyz_count;
  os_current_context = o;
}

int os_get_xyz_count()
{
  return os_xyz_count;
}
