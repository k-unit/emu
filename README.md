# emu

Kernel emulation module for the *kunit* (kernel unit-test) project.

This module consits of trivial(ish) implementations of user space mock functions for the kernel services and infrastructure required by the different unit tests.

The module is devided conceptually into two parts:
- **lnx** - kernel mock functions<br>
  The sub tree under this directory is a sub-tree of the actual linux kernel, where all the functinos, macros, constants, etc... which exist here are replaced mocks.<br>
  The important thing to remember is that every mock function residing in this sub-tree has a real counterpart residing under the exact same path in the kernel.<br>
  As such, both the code under test, and infrastructure are elegable to use the functions under this sub-tree.
- **kut** - *kunit* support functions<br>
  the sub tree under this directory consists is constructed as a sub-tree of the kernel, but all helper functions under this
sub-tree have no counter part in the real kernel.<br>
  The code being tested is not aware of these functions and cannot call functions in this sub-tree directly, however infrastructure and mock functions can.
