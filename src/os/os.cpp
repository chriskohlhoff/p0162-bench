#include "os.hpp"
#include <climits>

CallbackFnPtr os_associated_callback = nullptr;
OsContext* os_queue[UCHAR_MAX + 1];
unsigned char os_queue_head = 0;
unsigned char os_queue_tail = 0;
volatile int os_xyz_count = 0;

void os_associate_completion_callback(CallbackFnPtr cb)
{
  os_associated_callback = cb;
}

void os_trigger_completion()
{
  OsContext* o = os_queue[os_queue_head];
  os_queue[os_queue_head++] = nullptr;
  os_associated_callback(0, o);
}

void os_xyz(ParamType p, OsContext* o)
{
  ++os_xyz_count;
  os_queue[os_queue_tail++] = o;
}

int os_get_xyz_count()
{
  return os_xyz_count;
}
