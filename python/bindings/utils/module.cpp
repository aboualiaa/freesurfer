#include <pybind11/pybind11.h>

#include "morph.h"
#include "surface.h"
#include "volume.h"
#include "xform.h"

PYBIND11_MODULE(bindings, m) {
  throwExceptions(true);

  py::module mvol = m.def_submodule("vol");
  vol::bind(mvol);

  py::module msurf = m.def_submodule("surf");
  surf::bind(msurf);

  py::module mtransform = m.def_submodule("transform");
  transform::bind(mtransform);

  py::module mmorph = m.def_submodule("morph");
  morph::bind(mmorph);
}
