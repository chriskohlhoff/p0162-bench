#include <experimental/resumable>
#include <cassert>
#include <future>
#include <chrono>
#include <cstdio>
#include "os.hpp"

using namespace std::experimental;

//--------------------------------------------------------------

struct HandlerBase : OsContext
{
  CallbackFnPtr cb;

  explicit HandlerBase(CallbackFnPtr c) : cb(c) {}

  static void callback(OsResultType r, OsContext* o)
  {
    static_cast<HandlerBase*>(o)->cb(r, o);
  }
};

struct AwaitableBase : HandlerBase
{
  coroutine_handle<> resume;
  OsResultType result;

  AwaitableBase() : HandlerBase(&AwaitableBase::callback) {}

  static void callback(OsResultType r, OsContext* o)
  {
    auto me = static_cast<AwaitableBase*>(o);
    me->result = r;
    me->resume();
  }
};

//--------------------------------------------------------------

auto async_xyz(ParamType p)
{
  struct Awaiter : AwaitableBase
  {
    ParamType p;
    explicit Awaiter(ParamType & p) : p(std::move(p)) {}

    bool await_ready() { return false; } // the operation has not started yet
    auto await_resume() { return std::move(this->result); } // unpack the result when done
    void await_suspend(coroutine_handle<> h) { // call the OS and setup completion
      this->resume = h;
      os_xyz(p, this);
    }
  };
  return Awaiter{ p };
}

std::future<void> loop()
{
  for (;;)
  {
    await async_xyz(0);
  }
}

//--------------------------------------------------------------

int main()
{
  os_associate_completion_callback(&HandlerBase::callback);

  loop();
  loop();
  loop();
  loop();
  loop();
  loop();
  loop();
  loop();
  loop();
  loop();

  auto start = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < 100000000; ++i)
  {
    os_trigger_completion();
    os_trigger_completion();
    os_trigger_completion();
    os_trigger_completion();
    os_trigger_completion();
    os_trigger_completion();
    os_trigger_completion();
    os_trigger_completion();
    os_trigger_completion();
    os_trigger_completion();
  }
  auto stop = std::chrono::high_resolution_clock::now();

  std::int64_t msec = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
  std::printf("%lld ms for 1e9 iterations, which is %.3f ns per iteration\n", msec, msec / 1000.0);

  assert(os_get_xyz_count() == 1000000010);
  return os_get_xyz_count();
}
