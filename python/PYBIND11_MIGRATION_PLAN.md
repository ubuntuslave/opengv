# Pybind11 Migration Plan for `python/pyopengv.cpp` and `python/types.hpp`

## Goal
Migrate Python bindings from Boost.Python (`boost::python::numpy`) to `pybind11` so the module works on NumPy 2.x while preserving the current Python API surface and numerical behavior.

## Scope
- In scope:
  - `python/pyopengv.cpp`
  - `python/types.hpp`
  - `python/CMakeLists.txt` (build wiring needed by migration)
- Out of scope for this phase:
  - C++ core library algorithms in `src/` and `include/opengv/`
  - MATLAB bindings

## Current Baseline
- Exported Python functions in `pyopengv.cpp`: 28
- Current runtime blocker on NumPy 2.x is tied to Boost.Python NumPy initialization/ABI behavior.

## Compatibility Contract
- Keep module name: `pyopengv`
- Keep exported function names unchanged
- Keep return shapes and dtypes unchanged where currently tested (`python/tests.py`)
- Keep tuple/list return structure unchanged for RANSAC and multi-solution solvers

## Phase Plan

### Phase 1: Introduce pybind11-based array utilities (`types.hpp`)
Create a pybind11-native replacement for current array helpers.

1. Replace Boost includes/namespaces:
- Remove:
  - `#include <boost/python.hpp>`
  - `#include <boost/python/numpy.hpp>`
- Add:
  - `#include <pybind11/pybind11.h>`
  - `#include <pybind11/numpy.h>`
  - `#include <pybind11/stl.h>`

2. Define canonical aliases:
- `namespace py = pybind11;`
- Replace `ndarray` alias with `py::array` or typed `py::array_t<T>` wrappers.

3. Rebuild helper primitives:
- `bpn_array_from_data` -> `array_from_data`
  - Use `py::array_t<T>` with explicit shape and contiguous storage.
- `bpn_array_from_vector` -> `array_from_vector`
  - Return `py::array_t<T>`.

4. Replace `PyArrayContiguousView<T>` with `PyArrayView<T>`:
- Store `py::array_t<T, py::array::c_style | py::array::forcecast>`.
- Cache buffer info: `ptr`, `ndim`, `shape`.
- Provide same accessors used by current code:
  - `data()`
  - `ndim()`
  - `shape(dim)`
  - `get(i)`, `get(i, j)`, `get2D(i, k)`

Exit criteria:
- `types.hpp` contains no Boost.Python dependencies.
- Array helper unit behavior is equivalent for current callsites.

### Phase 2: Port binding implementation (`pyopengv.cpp`)
Keep algorithm logic intact and only migrate binding/runtime glue.

1. Replace module definition and registration:
- `BOOST_PYTHON_MODULE(pyopengv)` -> `PYBIND11_MODULE(pyopengv, m)`
- `def("name", fn)` -> `m.def("name", fn, ...)`

2. Replace binding types:
- `bp::object` -> `py::object`
- `bp::tuple` -> `py::tuple`
- `bp::list` -> `py::list`
- `ndarray &` parameters -> `const py::array &` or typed `py::array_t<double, ...>` where beneficial.

3. Preserve adapter logic:
- Keep `CentralAbsoluteAdapter`, `NonCentralAbsoluteAdapter`, `CentralRelativeAdapter` class logic unchanged except array wrapper types.
- Continue using conversion helpers (`bearingVectorFromArray`, etc.) backed by `PyArrayView<double>`.

4. Preserve return semantics:
- `ransac` functions must still return `(result, inlier_indices)` tuples.
- Multi-result methods should still return Python lists.

Exit criteria:
- `pyopengv.cpp` compiles with pybind11 and no Boost.Python includes.
- Exactly same 28 function names exported.

### Phase 3: CMake migration
Switch target dependency stack from Boost.Python to pybind11.

1. Add pybind11 dependency strategy:
- Preferred: `find_package(pybind11 CONFIG REQUIRED)`
- Fallback option (if needed later): vendor via submodule.

2. Create/replace module target:
- `pybind11_add_module(pyopengv pyopengv.cpp)`
- Link `opengv` library.

3. Remove Boost.Python requirements for Python module target:
- Drop `BOOST_PYTHON_COMPONENT`/`BOOST_NUMPY_COMPONENT` from Python binding target.
- Keep core library build unchanged.

4. Keep interpreter alignment:
- Continue explicit interpreter and NumPy include diagnostics.

Exit criteria:
- Python target configures without Boost.Python libraries.
- Build works in NumPy 1.x and 2.x environments.

### Phase 4: Validation and parity checks

1. Functional tests:
- Run `python/tests.py` against the pybind11 module.

2. API parity checks:
- Confirm all 28 symbol names are present.
- Confirm tuple/list return types and element dtypes.

3. Numerical parity:
- Compare selected outputs from Boost backend (NumPy 1.x env) vs pybind11 backend (NumPy 2.x env).

4. Safety checks:
- Import `pyopengv` under NumPy 2.x with no warnings/tracebacks.
- Run key API smoke calls with no segfaults.

## Recommended PR Breakdown

### PR 1: `types.hpp` pybind11 utilities
- Introduce pybind11 array wrapper utilities and compile-only scaffolding.
- No function export behavior changes yet.

### PR 2: `pyopengv.cpp` module port
- Convert module registration and all function signatures/returns to pybind11.
- Keep names and algorithm paths unchanged.

### PR 3: CMake switch + cleanup
- Move Python module build to pybind11.
- Remove Boost.Python-only Python target logic.
- Keep temporary guardrails/messages as needed.

### PR 4: parity hardening
- Add checks for exported names and return-type/dtype invariants.
- Optional: add small Python regression script for API contract checks.

## Risk Register
- Risk: subtle dtype changes (`int` vs `int64`) in inlier arrays.
  - Mitigation: assert dtype explicitly in tests.
- Risk: shape/layout mismatches from row-major assumptions.
  - Mitigation: enforce C-contiguous arrays in `PyArrayView` and test transformations.
- Risk: silent API drift in return container types.
  - Mitigation: parity checks on tuple/list contracts.
- Risk: build portability across Linux/macOS.
  - Mitigation: keep target properties and verify both paths.

## First Implementation Tasks (Next)
1. Create a pybind11 version of `types.hpp` helpers in place (or via temporary `types_pybind11.hpp` if staging is preferred).
2. Port `BOOST_PYTHON_MODULE` block and one small namespace (`triangulation`) to pybind11 as a vertical slice.
3. Build and run `python/tests.py` subset focused on triangulation to validate plumbing before full port.
