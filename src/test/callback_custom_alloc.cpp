#include <experimental/executor>
#include <cassert>
#include <chrono>
#include <iostream>
#include "os.hpp"

using namespace std::experimental::net;

//--------------------------------------------------------------

class custom_alloc_arena
{
  template <class> friend class custom_alloc;
  unsigned char prealloc_[128 - sizeof(bool)] alignas(double);
  bool in_use_ = false;
};

template <class T> class custom_alloc
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
    typedef custom_alloc<U> other;
  };

  custom_alloc(custom_alloc_arena* a) : arena_(a) {}
  template <class U> custom_alloc(const custom_alloc<U>& a) : arena_(a.arena_) {}

  pointer allocate(size_t n, pointer = 0)
  {
    if (sizeof(T) * n <= sizeof(arena_->prealloc_) && !arena_->in_use_)
    {
      arena_->in_use_ = true;
      return static_cast<T*>(static_cast<void*>(arena_->prealloc_));
    }
    else
      return static_cast<T*>(::operator new(n * sizeof(T)));
  }

  void deallocate(pointer p, size_t n)
  {
    if (static_cast<void*>(p) == static_cast<void*>(arena_->prealloc_))
      arena_->in_use_ = false;
    else
      ::operator delete(p);
  }

private:
  template <class> friend class custom_alloc;
  custom_alloc_arena* arena_;
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
    auto alloc = get_associated_allocator(static_cast<Handler*>(o)->handler);
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
  auto alloc = get_associated_allocator(completion.completion_handler);
  typename decltype(alloc)::template rebind<Handler<H>>::other alloc2(alloc);
  Handler<H>* h = alloc2.allocate(1);
  new (h) Handler<H>(std::move(completion.completion_handler));
  os_xyz(p, h);
  return completion.result.get();
}

//--------------------------------------------------------------

struct handle_xyz
{
  custom_alloc_arena* arena_;

  typedef custom_alloc<char> allocator_type;
  allocator_type get_allocator() const noexcept { return allocator_type(arena_); }

  void operator()(OsResultType)
  {
    async_xyz(0, *this);
  }
};

//--------------------------------------------------------------

int main()
{
  os_associate_completion_callback(&HandlerBase::callback);

  custom_alloc_arena handle_xyz_arena[10];
  async_xyz(0, handle_xyz{handle_xyz_arena + 0});
  async_xyz(0, handle_xyz{handle_xyz_arena + 1});
  async_xyz(0, handle_xyz{handle_xyz_arena + 2});
  async_xyz(0, handle_xyz{handle_xyz_arena + 3});
  async_xyz(0, handle_xyz{handle_xyz_arena + 4});
  async_xyz(0, handle_xyz{handle_xyz_arena + 5});
  async_xyz(0, handle_xyz{handle_xyz_arena + 6});
  async_xyz(0, handle_xyz{handle_xyz_arena + 7});
  async_xyz(0, handle_xyz{handle_xyz_arena + 8});
  async_xyz(0, handle_xyz{handle_xyz_arena + 9});

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
