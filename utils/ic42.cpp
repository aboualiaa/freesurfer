/**
 * @brief 1st order icosahedral subdivision with 42 vertices.
 *
 * in memory version of ic1.tri
 */
/*
 * Original Author: Bruce Fischl
 *
 * Copyright © 2011 The General Hospital Corporation (Boston, MA) "MGH"
 *
 * Terms and conditions for use, reproduction, distribution and contribution
 * are found in the 'FreeSurfer Software License Agreement' contained
 * in the file 'LICENSE' found in the FreeSurfer distribution, and here:
 *
 * https://surfer.nmr.mgh.harvard.edu/fswiki/FreeSurferSoftwareLicense
 *
 * Reporting: freesurfer@nmr.mgh.harvard.edu
 *
 */

#include "icosahedron.h"
#include "mrisurf.h"

#define ICO_NVERTICES 42
// clang-format off
IC_VERTEX ic42_vertices[42]
    = { { .0000, .0000, 1.0000 },   { .8944, .0000, .4472 },   { .2764, .8507, .4472 },    { -.7236, .5257, .4472 },
        { -.7236, -.5257, .4472 },  { .2764, -.8507, .4472 },  { .7236, -.5257, -.4472 },  { .7236, .5257, -.4472 },
        { -.2764, .8507, -.4472 },  { -.8944, .0000, -.4472 }, { -.2764, -.8507, -.4472 }, { .0000, .0000, -1.0000 },
        { -.4253, -.3090, .8507 },  { -.8507, .0000, .5257 },  { -.4253, .3090, .8507 },   { .1625, -.5000, .8507 },
        { -.2629, -.8090, .5257 },  { .5257, .0000, .8507 },   { .6882, -.5000, .5257 },   { .1625, .5000, .8507 },
        { .6882, .5000, .5257 },    { -.2629, .8090, .5257 },  { -.5878, .8090, .0000 },   { .0000, 1.0000, .0000 },
        { -.9511, .3090, .0000 },   { -.6882, .5000, -.5257 }, { -.9511, -.3090, .0000 },  { -.5878, -.8090, .0000 },
        { -.6882, -.5000, -.5257 }, { .0000, -1.0000, .0000 }, { .5878, -.8090, .0000 },   { .2629, -.8090, -.5257 },
        { .9511, -.3090, .0000 },   { .9511, .3090, .0000 },   { .8507, .0000, -.5257 },   { .5878, .8090, .0000 },
        { .2629, .8090, -.5257 },   { -.5257, .0000, -.8507 }, { -.1625, .5000, -.8507 },  { -.1625, -.5000, -.8507 },
        { .4253, -.3090, -.8507 },  { .4253, .3090, -.8507 } };

#define ICO_NFACES 80

IC_FACE ic42_faces[ICO_NFACES]
    = { { { 1, 13, 15 } },  { { 13, 5, 14 } },  { { 15, 13, 14 } }, { { 15, 14, 4 } },  { { 1, 16, 13 } },
        { { 16, 6, 17 } },  { { 13, 16, 17 } }, { { 13, 17, 5 } },  { { 1, 18, 16 } },  { { 18, 2, 19 } },
        { { 16, 18, 19 } }, { { 16, 19, 6 } },  { { 1, 20, 18 } },  { { 20, 3, 21 } },  { { 18, 20, 21 } },
        { { 18, 21, 2 } },  { { 1, 15, 20 } },  { { 15, 4, 22 } },  { { 20, 15, 22 } }, { { 20, 22, 3 } },
        { { 4, 23, 22 } },  { { 23, 9, 24 } },  { { 22, 23, 24 } }, { { 22, 24, 3 } },  { { 4, 25, 23 } },
        { { 25, 10, 26 } }, { { 23, 25, 26 } }, { { 23, 26, 9 } },  { { 4, 14, 25 } },  { { 14, 5, 27 } },
        { { 25, 14, 27 } }, { { 25, 27, 10 } }, { { 5, 28, 27 } },  { { 28, 11, 29 } }, { { 27, 28, 29 } },
        { { 27, 29, 10 } }, { { 5, 17, 28 } },  { { 17, 6, 30 } },  { { 28, 17, 30 } }, { { 28, 30, 11 } },
        { { 6, 31, 30 } },  { { 31, 7, 32 } },  { { 30, 31, 32 } }, { { 30, 32, 11 } }, { { 6, 19, 31 } },
        { { 19, 2, 33 } },  { { 31, 19, 33 } }, { { 31, 33, 7 } },  { { 2, 34, 33 } },  { { 34, 8, 35 } },
        { { 33, 34, 35 } }, { { 33, 35, 7 } },  { { 2, 21, 34 } },  { { 21, 3, 36 } },  { { 34, 21, 36 } },
        { { 34, 36, 8 } },  { { 3, 24, 36 } },  { { 24, 9, 37 } },  { { 36, 24, 37 } }, { { 36, 37, 8 } },
        { { 9, 26, 39 } },  { { 26, 10, 38 } }, { { 39, 26, 38 } }, { { 39, 38, 12 } }, { { 10, 29, 38 } },
        { { 29, 11, 40 } }, { { 38, 29, 40 } }, { { 38, 40, 12 } }, { { 11, 32, 40 } }, { { 32, 7, 41 } },
        { { 40, 32, 41 } }, { { 40, 41, 12 } }, { { 7, 35, 41 } },  { { 35, 8, 42 } },  { { 41, 35, 42 } },
        { { 41, 42, 12 } }, { { 8, 37, 42 } },  { { 37, 9, 39 } },  { { 42, 37, 39 } }, { { 42, 39, 12 } } };
// clang-format on
MRI_SURFACE *ic42_make_surface(int max_vertices, int max_faces) {
  static int first_time = 1;

  if (first_time) {
    first_time = 0;
    int fno;
    for (fno = 0; fno < ICO_NFACES; fno++) {
      int vno                = ic42_faces[fno].vno[1];
      ic42_faces[fno].vno[1] = ic42_faces[fno].vno[2];
      ic42_faces[fno].vno[2] = vno;
    }
  }

  ICOSAHEDRON icos = {ICO_NVERTICES, ICO_NFACES, ic42_vertices, ic42_faces};

  return ICOtoMRIS(&icos, max_vertices, max_faces);
}
