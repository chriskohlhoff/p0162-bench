#include <experimental/executor>
#include <cassert>
#include <chrono>
#include <iostream>
#include "os.hpp"

using namespace std::experimental::net;

//--------------------------------------------------------------

class default_alloc_base
{
protected:
  static const std::size_t block_size_ = 128 - sizeof(void*);
  static thread_local void* recycled_;
};

thread_local void* default_alloc_base::recycled_;

template <class T> class default_alloc : default_alloc_base
{
public:
  typedef T* pointer;
  typedef const T* const_pointer;
  typedef T value_type;
  typedef std::size_t size_type;
  typedef std::ptrdiff_t difference_type;

  template <class U>
  struct rebind
  {
    typedef default_alloc<U> other;
  };

  default_alloc() {}
  template <class U> default_alloc(const default_alloc<U>&) {}

  pointer allocate(size_t n, pointer = 0)
  {
    if (sizeof(T) * n <= block_size_)
    {
      if (recycled_)
      {
        void* p = recycled_;
        recycled_ = nullptr;
        return static_cast<T*>(p);
      }
      else
        return static_cast<T*>(::operator new(block_size_));
    }
    else
      return static_cast<T*>(::operator new(n * sizeof(T)));
  }

  void deallocate(pointer p, size_t n)
  {
    if (sizeof(T) * n <= block_size_ && !recycled_)
      recycled_ = p;
    else
      ::operator delete(p);
  }
};

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

template <class CompletionHandler>
struct Handler : HandlerBase
{
  CompletionHandler handler;

  template <class CompletionHandlerFwd>
  explicit Handler(CompletionHandlerFwd&& h)
    : HandlerBase(&Handler::callback),
      handler(std::forward<CompletionHandlerFwd>(h))
  {
  }

  static void callback(OsResultType r, OsContext* o)
  {
    auto alloc = get_associated_allocator(static_cast<Handler*>(o)->handler, default_alloc<void>());
    typename decltype(alloc)::template rebind<Handler>::other alloc2(alloc);
    CompletionHandler handler(std::move(static_cast<Handler*>(o)->handler));
    static_cast<Handler*>(o)->~Handler();
    alloc2.deallocate(static_cast<Handler*>(o), 1);
    handler(r);
  }
};

//--------------------------------------------------------------

template <class CompletionToken>
inline auto async_xyz(ParamType p, CompletionToken&& token)
{
  async_completion<CompletionToken, void(OsResultType)> completion(token);
  typedef typename async_completion<CompletionToken, void(OsResultType)>::completion_handler_type H;
  auto alloc = get_associated_allocator(completion.completion_handler, default_alloc<void>());
  typename decltype(alloc)::template rebind<Handler<H>>::other alloc2(alloc);
  Handler<H>* h = alloc2.allocate(1);
  new (h) Handler<H>(std::move(completion.completion_handler));
  os_xyz(p, h);
  return completion.result.get();
}

//--------------------------------------------------------------

struct handle_xyz
{
  void operator()(OsResultType)
  {
    async_xyz(0, *this);
  }
};

//--------------------------------------------------------------

int main()
{
  os_associate_completion_callback(&HandlerBase::callback);

  async_xyz(0, handle_xyz());
  async_xyz(0, handle_xyz());
  async_xyz(0, handle_xyz());
  async_xyz(0, handle_xyz());
  async_xyz(0, handle_xyz());
  async_xyz(0, handle_xyz());
  async_xyz(0, handle_xyz());
  async_xyz(0, handle_xyz());
  async_xyz(0, handle_xyz());
  async_xyz(0, handle_xyz());

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
