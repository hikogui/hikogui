Code Quality
============

Static Analysis
---------------

### CodeQL (disabled)

CodeQL reports too many false positives to be useable.
Reporting bugs to the CodeQL maintainers result in acknowledgment
of the bug with a reply that they will not fix these bugs.

### EspXEngine.dll Core Guideline checker (disabled)

Most rules of the Core Guidelines can't be followed in non-trivial
applications let alone libraries.

For example:

 - reinterpret_cast must be used when using SSE intrinsics.
 - Even when checking for nullptr it requires pointers to be marked non-null
 - Using .at() everywhere add lots of bounds checks and disables auto-vectorization
 - Marking function noexcept when a inner function throws is useful but not allowed.
 - etc.
