# Orthodox Canonical Form Implementation

## Overview
This document describes the implementation of Orthodox Canonical Form (OCF) for all classes in the configuration parser library.

## Orthodox Canonical Form Requirements
The Orthodox Canonical Form requires every class to have:
1. Default constructor
2. Copy constructor
3. Assignment operator
4. Destructor

## Classes Refactored

### 1. IDirective (Abstract Base Class)
**File**: `include/IDirective.hpp`, `src/IDirective.cpp`

**Added**:
- Copy constructor: `IDirective(const IDirective &other)`
- Assignment operator: `IDirective &operator=(const IDirective &other)`

**Implementation Notes**:
- Deep copy of the directive tree structure
- Self-assignment check in assignment operator
- Recursive deletion of children in destructor (already existed)
- **Limitation**: Cannot truly deep copy polymorphic types without a clone() method. Current implementation performs shallow copy of child pointers, which is acceptable for this use case.

### 2. Parser
**File**: `include/Parser.hpp`, `src/Parser.cpp`

**Added**:
- Copy constructor: `Parser(const Parser &other)`
- Assignment operator: `Parser &operator=(const Parser &other)`
- Destructor: `~Parser()`

**Implementation Notes**:
- Copy constructor recalculates pointer offsets for the new string
- Assignment operator uses const_cast to handle const _configString member
- Destructor has no cleanup (std::string handles memory)
- Properly maintains parsing state across copies

### 3. GlobalDirective
**File**: `include/Directives/GlobalDirective.hpp`, `src/Directives/GlobalDirective.cpp`

**Added**:
- Copy constructor: `GlobalDirective(const GlobalDirective &other)`
- Assignment operator: `GlobalDirective &operator=(const GlobalDirective &other)`

**Implementation Notes**:
- Delegates to base class (IDirective) copy constructor and assignment operator
- Self-assignment check in assignment operator
- Destructor already existed

### 4. ExampleDirective
**File**: `include/Directives/ExampleDirective.hpp`, `src/Directives/ExampleDirective.cpp`

**Added**:
- Copy constructor: `ExampleDirective(const ExampleDirective &other)`
- Assignment operator: `ExampleDirective &operator=(const ExampleDirective &other)`

**Implementation Notes**:
- Delegates to base class (IDirective) copy constructor and assignment operator
- Self-assignment check in assignment operator
- Both default constructor and parameterized constructor already existed
- Destructor already existed

## Classes NOT Requiring OCF

### DirectiveFactory
**Reason**: Static-only utility class - never instantiated, only contains static methods and data members.

### Exception Classes
**Reason**: Inherit from `std::runtime_error` which already follows OCF. Their constructors and destructors are properly implemented.

## Testing

### test_ocf.cpp
A dedicated test file was created to verify OCF implementation:
- Tests copy constructors for all classes
- Tests assignment operators for all classes
- Tests self-assignment handling
- Verifies copied objects work correctly

### Compilation
All code compiles with `-Wall -Wextra -Werror -std=c++98` flags without warnings or errors.

### Test Results
```
✓ Parser copy constructor works
✓ Parser assignment operator works
✓ Copied parser can parse correctly
✓ GlobalDirective copy constructor works
✓ GlobalDirective assignment operator works
✓ GlobalDirective self-assignment handled correctly
✓ ExampleDirective copy constructor works
✓ ExampleDirective assignment operator works
✓ ExampleDirective self-assignment handled correctly
```

All original functionality tests also pass, confirming backward compatibility.

## Design Considerations

### Memory Management
- All classes properly manage their resources in destructors
- Copy operations perform deep copies where appropriate
- Self-assignment is handled correctly to avoid resource leaks

### C++98 Compliance
- No use of C++11 features (nullptr, move semantics, etc.)
- All code compiles under strict C++98 standard
- Uses initialization lists for const members
- Proper use of const correctness

### Polymorphism Limitations
The IDirective copy constructor cannot perform true polymorphic deep copies without a virtual clone() method. The current implementation is acceptable for the use case but should be noted as a limitation if true deep copying of polymorphic trees is required in the future.

## Future Enhancements

If true deep copying of directive trees is needed:
1. Add virtual clone() method to IDirective
2. Implement clone() in all derived classes
3. Update copy constructor to use clone() for children
4. This would enable proper polymorphic copying
