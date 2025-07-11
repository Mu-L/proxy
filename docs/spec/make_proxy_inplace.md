# Function template `make_proxy_inplace`

> Header: `proxy.h`  
> Module: `proxy`  
> Namespace: `pro::inline v4`

The definition of `make_proxy_inplace` makes use of an exposition-only class template *inplace-ptr*. Similar to [`std::optional`](https://en.cppreference.com/w/cpp/utility/optional), `inplace-ptr<T>` contains the storage for an object of type `T`, manages its lifetime, and provides `operator*` for access with the same qualifiers. However, it does not necessarily support the state where the contained object is absent. `inplace-ptr<T>` has the same size and alignment as `T`.

```cpp
// (1)
template <facade F, class T, class... Args>
proxy<F> make_proxy_inplace(Args&&... args)
    noexcept(std::is_nothrow_constructible_v<T, Args...>)
    requires(std::is_constructible_v<T, Args...>);

// (2)
template <facade F, class T, class U, class... Args>
proxy<F> make_proxy_inplace(std::initializer_list<U> il, Args&&... args)
    noexcept(std::is_nothrow_constructible_v<
        T, std::initializer_list<U>&, Args...>)
    requires(std::is_constructible_v<T, std::initializer_list<U>&, Args...>);

// (3)
template <facade F, class T>
proxy<F> make_proxy_inplace(T&& value)
    noexcept(std::is_nothrow_constructible_v<std::decay_t<T>, T>)
    requires(std::is_constructible_v<std::decay_t<T>, T>);
```

`(1)` Creates a `proxy<F>` object containing a value `p` of type `inplace-ptr<T>`, where `*p` is direct-non-list-initialized with `std::forward<Args>(args)...`.

`(2)` Creates a `proxy<F>` object containing a value `p` of type `inplace-ptr<T>`, where `*p` is direct-non-list-initialized with `il, std::forward<Args>(args)...`.

`(3)` Creates a `proxy<F>` object containing a value `p` of type `inplace-ptr<std::decay_t<T>>`, where `*p` is direct-non-list-initialized with `std::forward<T>(value)`.

*Since 3.3.0*: For `(1-3)`, if [`inplace_proxiable_target<std::decay_t<T>, F>`](inplace_proxiable_target.md) is `false`, the program is ill-formed and diagnostic messages are generated.

## Return Value

The constructed `proxy` object.

## Exceptions

Throws any exception thrown by the constructor of `T`.

## Example

```cpp
#include <array>

#include <proxy/proxy.h>

// By default, the maximum pointer size defined by pro::facade_builder
// is 2 * sizeof(void*). This value can be overridden by `restrict_layout`.
struct Any : pro::facade_builder::build {};

int main() {
  // sizeof(int) is usually not greater than sizeof(void*) for modern
  // 32/64-bit compilers
  pro::proxy<Any> p1 = pro::make_proxy_inplace<Any>(123);

  // Won't compile because sizeof(std::array<int, 100>) is usually greater than
  // 2 * sizeof(void*) pro::proxy<Any> p2 = pro::make_proxy_inplace<Any,
  // std::array<int, 100>>();
}
```

## See Also

- [concept `inplace_proxiable_target`](inplace_proxiable_target.md)
- [function template `make_proxy`](make_proxy.md)
