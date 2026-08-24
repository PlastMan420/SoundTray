# Google C++ Style Guide
Source: https://google.github.io/styleguide/cppguide.html

## Header Files
- Every `.cc` file should generally have an associated `.h` file; headers must be self-contained (compile on their own) and end in `.h`. Files meant for textual inclusion that aren't self-contained end in `.inc`.
- All headers need `#define` guards: `<PROJECT>_<PATH>_<FILE>_H_`, e.g. `foo/src/bar/baz.h` → `FOO_BAR_BAZ_H_`.
- Include what you use: include headers for every symbol you rely on; do not depend on transitive includes.
- Avoid forward declarations where possible; include the headers you need. Never forward-declare symbols from `std::` or entities defined in another project.
- Define functions `inline` only when they are small (≤ ~10 lines).
- Include order in `foo.cc`:
  1. `foo.h` (related header)
  2. C system headers (`<unistd.h>`)
  3. C++ standard library headers (`<string>`)
  4. Other libraries' headers
  5. Your project's headers
  Separate groups with blank lines; alphabetize within each group. Conditional (`#ifdef`) includes go last.
  ```cpp
  // In dir/foobar.cc:
  #include "dir/foobar.h"

  #include <sys/types.h>
  #include <unistd.h>

  #include <string>
  #include <vector>

  #include "absl/strings/str_cat.h"
  #include "base/basictypes.h"
  #include "dir2/foo2.h"
  ```
- Use quotes for project headers, angle brackets for system/standard headers.

## Scoping
- Place code in namespaces named after the project/path; namespace names must be unique. Do not use `using namespace foo` directives. Do not use inline namespaces.
- Terminate namespaces with a comment: `}  // namespace mynamespace`.
  ```cpp
  namespace mynamespace {

  class MyClass {
   public:
    void Foo();
  };

  }  // namespace mynamespace
  ```
- Never declare anything in namespace `std`.
- Namespace aliases are OK in `.cc` files and inside functions, but not at namespace scope in headers (except internal namespaces).
- Give internal-only (`.cc`-local) code internal linkage via an unnamed namespace or `static`; never use these in `.h` files.
- Prefer nonmember functions inside a namespace, or static member functions, over global functions. Don't use a class merely to group static members.
- Declare local variables in the narrowest scope possible, close to first use, and initialize in the declaration: `int i = f();` not `int i; i = f();`.
- Loop variables belong in the loop statement: `for (int i = 0; i < 10; ++i)`. But an object used in every loop iteration may be declared outside the loop if its constructor/destructor is costly.
- Objects with static storage duration are forbidden unless trivially destructible. Dynamic initialization of static variables is discouraged (allowed for function-local statics). Prefer `constexpr` globals; for global strings use `constexpr std::string_view` or `char[]`, not `std::string`. No `std::map`/`std::vector` etc. as globals; no smart pointers with static duration (use a function-local static raw pointer that's never deleted if needed).
- `thread_local` at namespace/class scope must be initialized with a compile-time constant (enforce with `ABSL_CONST_INIT`/`constinit`). Prefer `thread_local` over other thread-local mechanisms.

## Classes
- Avoid virtual method calls in constructors. Avoid work that can fail in a constructor if you can't report the error; consider a factory function or `Init()` method instead.
- Mark single-argument constructors and conversion operators `explicit`. Exceptions: copy/move constructors, `std::initializer_list` constructors, and (by judgment) constructors intended as transparent wrappers.
- Make it clear whether a class is copyable, move-only, or neither by explicitly declaring/defaulting/deleting the copy and move operations in the `public` section:
  ```cpp
  class Copyable {
   public:
    Copyable(const Copyable&) = default;
    Copyable& operator=(const Copyable&) = default;
  };
  class MoveOnly {
   public:
    MoveOnly(MoveOnly&&) = default;
    MoveOnly& operator=(MoveOnly&&) = default;
  };
  class NotCopyableOrMovable {
   public:
    NotCopyableOrMovable(const NotCopyableOrMovable&) = delete;
    NotCopyableOrMovable& operator=(const NotCopyableOrMovable&) = delete;
  };
  ```
- Use `struct` only for passive data carriers (all fields public, no invariants); use `class` for everything else. `struct` is also fine for stateless traits/metafunctions and small functors.
- Prefer a named `struct` over `std::pair`/`std::tuple` whenever the elements have meaning.
- Prefer composition to inheritance. All inheritance must be `public`; limit to genuine "is-a" relationships. Data members must be `private`. Explicitly annotate overrides with exactly one of `override` or `final` (and not `virtual`). Multiple implementation inheritance is strongly discouraged.
- Operator overloading: define operators only when their meaning is obvious and consistent with built-ins; define them in the same headers/namespaces as their operand types; prefer non-member binary operators. Define related operators together (e.g. `==` with `!=` semantics, or `<=>`). Never overload `&&`, `||`, `,`, unary `&`, or user-defined literals (`operator""`).
- Access control: make data members `private` unless they are constants; provide accessors as needed. (Exception: `protected` fields OK in test fixtures declared in `.cc` files.)
- Declaration order: `public:` first, then `protected:`, then `private:`; omit empty sections. Within each section: types and aliases; (structs) data members; static constants; factory functions; constructors and assignment operators; destructor; all other functions; all other data members. Don't put large method definitions inline in the class definition.

## Functions
- Prefer return values to output parameters. Return by value (or reference when the semantics call for it).
- Non-optional inputs: value or `const T&`. Non-optional outputs/in-outs: references. Optional inputs: `std::optional<T>` (by value) or `const T*`. Optional outputs/in-outs: non-const pointer.
- Parameter order: inputs first, then outputs.
- Prefer small, focused functions; roughly 40 lines is a soft threshold for splitting.
- Overload only when a reader can understand a call site without figuring out which overload is chosen; overloads should have equivalent semantics; document the set with a single comment.
- Default arguments are allowed on non-virtual functions when the default is always the same value; forbidden on virtual functions.
- Use trailing return types (`auto f() -> T`) only where required (lambdas) or where they meaningfully improve readability (rare template cases).

## Ownership and Smart Pointers
- Prefer single, fixed ownership; transfer ownership with smart pointers (`std::unique_ptr` preferred).
- Use `std::shared_ptr` sparingly; if ownership must be shared, prefer `std::shared_ptr<const T>` (immutable shared state).
- Never use `std::auto_ptr`. Do not design for ownership transfer via raw pointers.

## Other C++ Features
- **Rvalue references**: use only for move constructors/assignment, `&&`-qualified consuming methods, forwarding references with `std::forward`, and (rarely) `const T&`/`T&&` overload pairs for performance.
- **Friends**: allowed within reason, usually defined in the same file (e.g. builders, tests).
- **Exceptions**: do not use C++ exceptions; do not `throw`. (Historical/interop reasons; Windows code has a partial exemption.)
- **noexcept**: use when useful and correct — notably on move constructors/assignment. Prefer unconditional `noexcept` where exceptions are disabled; keep conditional `noexcept` clauses simple.
- **RTTI**: avoid `dynamic_cast` and `typeid` in production code (fine in tests). Prefer virtual methods or the Visitor pattern; type-based decision trees signal a design problem.
- **Casting**: use C++-style casts, never C-style casts (`(int)x` — forbidden). In order of preference:
  ```cpp
  int64_t y = int64_t{1} << 42;      // brace init for arithmetic conversion
  std::string s = std::string(cord);  // T(x) only when T is a class type
  static_cast<float>(double_value);   // value conversion / pointer upcast
  ```
  Also: `absl::implicit_cast` for safe upcasts, `absl::down_cast` for checked downcasts, `const_cast` (rare), `reinterpret_cast` (rare, dangerous), `std::bit_cast` for type punning of same-size trivially-copyable values.
- **Streams**: OK for local, ad-hoc, developer-facing output; avoid for user-facing I/O or untrusted input. Avoid stateful stream mutations (`imbue()` etc.). Overload `<<` only for value types and print only the user-visible value. Prefer logging libraries and `absl::StrCat`/`StrFormat` over stringstreams.
- **Preincrement**: use prefix `++i`/`--i` unless you need the previous value.
- **const/constexpr**: use `const` wherever it is meaningful — parameters, methods that don't mutate, variables never modified. Put `const` first (`const int* p`). Use `constexpr` for true compile-time constants and functions; use `constinit` for constant-initialized non-const globals.
- **Integer types**: use `int` for "not-too-big" numbers; use `<cstdint>` fixed-width types (`int64_t`, `int32_t`, …) otherwise. Do not use unsigned types to mean "non-negative"; use them only for bitfields/modular arithmetic. `uint64_t` for bit patterns is fine.
- **64-bit portability**: code should be 64-bit and 32-bit friendly; use `PRId64`-style macros or better, avoid printf.
- **Macros**: avoid; prefer inline functions, enums, `const`/`constexpr`, and templates. If unavoidable: ALL_CAPS names with a project-specific prefix, don't define in headers, `#define` just before use and `#undef` after where possible, no macros that affect control flow.
- **0/nullptr/NULL**: use `nullptr` for pointers, `'\0'` for chars, plain `0`/`0.0` for numbers.
- **sizeof**: prefer `sizeof(varname)` to `sizeof(type)`.
- **Type deduction (`auto`)**: use only when it makes the code clearer or safer to readers unfamiliar with the project; never merely to save typing. Fine for iterators and obviously-typed initializers. Same applies to CTAD, structured bindings (name the bound fields helpfully), and return-type deduction.
  ```cpp
  auto it = my_map.find(key);                       // OK: type is clear and noisy to spell
  auto widget = MakeWidget();                       // questionable: what is widget?
  auto [iter, success] = my_map.insert({key, val}); // OK: names convey meaning
  ```
- **Class template argument deduction (CTAD)**: use only with templates that opt in via deduction guides.
- **Designated initializers**: OK, in C++20-compliant form (fields in declaration order).
- **Lambdas**: fine, especially for short one-offs passed to algorithms. Prefer explicit captures when the lambda may escape the current scope; avoid default capture by reference (`[&]`) for escaping lambdas. Keep lambdas short; prefer named functions for complex logic.
- **Template metaprogramming**: avoid complicated TMP; use only with a big payoff, and keep interfaces understandable to non-experts with good docs and error-message hygiene.
- **Concepts (C++20)**: use judiciously; prefer the simplest mechanism that constrains adequately.
- **Boost / std alternatives**: use only approved libraries; prefer the standard library where equivalent.
- **Nonstandard extensions**: don't use them (e.g. `__attribute__` directly — use portability wrappers).
- **Aliases**: public aliases (`using Foo = ...;`) are for the benefit of the API's users and should be documented; don't put aliases in your public API merely for local convenience.

## Inclusive Language
- Avoid terms that other programmers might find disrespectful (e.g. use "primary/replica", "blocklist/allowlist", gender-neutral "they").

## Naming
- Optimize names for the reader; avoid abbreviations that aren't universally known and never delete letters within a word. Loop counters `i`, `j` are fine in small scopes. Names may be more terse the smaller the scope, and should be more descriptive the wider the visibility.

| Entity | Convention | Example |
|---|---|---|
| Files | `snake_case` | `my_useful_class.cc`, `foo_test.cc` |
| Types (class/struct/enum/alias/template param) | `UpperCamelCase` | `UrlTableProperties` |
| Ordinary variables, struct members, parameters | `snake_case` | `table_name` |
| Class data members | `snake_case_` (trailing `_`) | `table_name_` |
| Constants (static storage, constexpr/const) | `kUpperCamel` | `kDaysInAWeek` |
| Functions | `UpperCamelCase()` | `AddTableEntry()` |
| Cheap accessors/mutators | like the variable | `count()`, `set_count(int)` |
| Namespaces | `snake_case` | `websearch::index_util` |
| Enumerators | `kUpperCamel` | `kOk`, `kOutOfMemory` |
| Macros | `ALL_CAPS` + project prefix | `MYPROJECT_ROUND(x)` |

- Namespaces: based on project name/path; avoid names matching well-known top-level namespaces (`std`) and overly generic names (`util`).
- Enumerators use constant-style naming, not `ALL_CAPS` (avoids macro collisions):
  ```cpp
  enum class UrlTableError {
    kOk = 0,
    kOutOfMemory,
    kMalformedInput,
  };
  ```
- Template parameters follow the convention of their category: type template parameters like types (`T`, `PropertyMap`), non-type ones like variables or constants.

## Comments
- Use either `//` or `/* */`, but be consistent; `//` is far more common.
- **File comments**: start with license boilerplate; if the file's content needs it, add a brief description. Don't duplicate class comments.
- **Class comments**: every non-obvious class/struct gets a comment describing what it's for, how to use it, and thread-safety assumptions if relevant.
- **Function comments**: declaration comments describe use (inputs/outputs, ownership, synchronization, edge cases); written declaratively ("Opens the file"), not imperatively ("Open the file"). Definition comments explain tricky implementation details.
  ```cpp
  // Returns an iterator for this table, positioned at the first entry
  // lexically greater than or equal to `start_word`. The client must
  // not use the iterator after the underlying GargantuanTable is destroyed.
  std::unique_ptr<Iterator> GetIterator(absl::string_view start_word) const;
  ```
- **Variable comments**: only when purpose isn't obvious from the name; especially globals.
- **Implementation comments**: explain the tricky, non-obvious parts — the "why". Add clarifying comments for magic arguments at call sites (`/*is_recursive=*/true`) or use named constants.
- **TODOs**: `// TODO: bug/context — description`, referencing a bug ID or identifier of the person/issue with context:
  ```cpp
  // TODO: bug 12345678 - Remove this after the 2047q4 compatibility window expires.
  ```
- Never state the obvious; don't literally restate the code.
  ```cpp
  // BAD: Returns true if the table cannot hold any more entries.
  // GOOD (at call site): DropTable(/*bool drop_if_full=*/true);
  ```

## Formatting
- **Line length**: max 80 characters. Exceptions: literal URLs/paths, include guards, string literals that shouldn't be split.
- **Non-ASCII**: rare, must be UTF-8.
- **Indentation**: 2 spaces, never tabs.
- **Functions**: return type on the same line as the name; parameters on the same line if they fit, else wrap aligned with the first parameter or 4-space continuation indent. Open brace at end of line. Unused parameters may be left unnamed or commented: `void f(Foo /*unused*/)`.
  ```cpp
  ReturnType ClassName::ReallyLongFunctionName(Type par_name1,
                                               Type par_name2) {
    DoSomething();
  }
  ```
- **Lambdas**: format like functions; no space between `&` and the captured name (`[&x]`).
- **Function calls**: on one line if it fits; else wrap arguments aligned with first argument, or 4-space indent for all arguments on their own lines.
- **Braced initializer lists**: format like function calls.
- **Conditionals**: space after `if`, none inside parens: `if (condition) {`. Always prefer curly braces; braces may be omitted only when the entire statement fits on one line (and `if` has no `else`).
  ```cpp
  if (x == kFoo) {
    return new Foo();
  } else {
    return new Bar();
  }
  ```
- **Loops and switches**: `switch` cases have braces optional; blocks indented; annotate non-trivial fall-through with `[[fallthrough]];`. Include a `default:` case (or make sure enums are exhaustive). Empty loop bodies use `{}` or `continue;`, never a bare `;`.
  ```cpp
  switch (var) {
    case 0: {
      Foo();
      break;
    }
    default: {
      Bar();
    }
  }
  ```
- **Pointers/references**: no space between `*`/`&` and the variable OR the type — pick one style per file: `char* c;` (preferred) or `char *c;`. Never declare multiple pointers on one line (`int* x, *y` — forbidden).
- **Boolean expressions**: when wrapping a long condition, keep the operator at end of line consistently.
- **Return values**: don't parenthesize `return` expressions unless needed for a wrapped complex expression.
- **Preprocessor directives**: start at beginning of line, even inside indented code; indent after the hash if nesting: `#if`, `  #define`.
- **Class format**: `public:`/`protected:`/`private:` indented 1 space; base class on same line if it fits.
  ```cpp
  class MyClass : public OtherClass {
   public:
    MyClass();

   private:
    int some_var_;
  };
  ```
- **Constructor initializer lists**: on one line, or wrapped with 4-space indent, colon first:
  ```cpp
  MyClass::MyClass(int var)
      : some_var_(var), some_other_var_(var + 1) {}
  ```
- **Namespaces**: contents not indented.
- **Horizontal whitespace**: never at end of line; space around binary operators (`=`, `+`, `<`), typically none around `.`/`->`; no space inside parens/brackets; space before open brace; semicolons attached to preceding text (`f();`).
- **Vertical whitespace**: minimize; blank lines to separate logical sections; don't start or end a function body with a blank line; 1–2 blank lines between functions.

## Exceptions to the Rules
- Existing non-conformant code: be consistent with the local style.
- Windows code: follows this guide with limited deviations (may use exceptions with existing exception-using code, `#pragma once` discouraged but seen, etc.).
- If a style point is a "should", deviation requires good reason; parting from the guide needs consistency and justification. When in doubt: **be consistent** with surrounding code, and use common sense.
