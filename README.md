<div align="center">
  <img src="https://surfer.nmr.mgh.harvard.edu/pub/data/tmp/brain.png">
  <p align="center">An open-source software suite for processing human brain MRI</p>
</div>

<table>
<tr>
    <td><img src="https://img.shields.io/github/followers/aboualiaa?style=social" /></td>
    <td><img src="https://img.shields.io/github/forks/aboualiaa/freesurfer?style=social" /></td>
    <td><a href="https://travis-ci.com/aboualiaa/freesurfer">
    <img src="https://travis-ci.com/aboualiaa/freesurfer.svg?branch=fs-modernize" /></a></td>
</tr>
</table>

| Builds | AppleClang 11 | Clang 9 | GCC 9 |
|:--:|:--:|:--:|:--:|
| macOS 10.14| ![Mojave](https://travis-matrix-badges.herokuapp.com/repos/aboualiaa/freesurfer/branches/fs-modernize/1?use_travis_com=true) | ![Mojave](https://travis-matrix-badges.herokuapp.com/repos/aboualiaa/freesurfer/branches/fs-modernize/8?use_travis_com=true) | ![Mojave](https://travis-matrix-badges.herokuapp.com/repos/aboualiaa/freesurfer/branches/fs-modernize/9?use_travis_com=true) |
| Ubuntu 14.04 | N/A | ![Trusty](https://travis-matrix-badges.herokuapp.com/repos/aboualiaa/freesurfer/branches/fs-modernize/5?use_travis_com=true) | ![Trusty](https://travis-matrix-badges.herokuapp.com/repos/aboualiaa/freesurfer/branches/fs-modernize/2?use_travis_com=true) |
| Ubuntu 16.04 | N/A | ![Xenial](https://travis-matrix-badges.herokuapp.com/repos/aboualiaa/freesurfer/branches/fs-modernize/6?use_travis_com=true) | ![Xenial](https://travis-matrix-badges.herokuapp.com/repos/aboualiaa/freesurfer/branches/fs-modernize/3?use_travis_com=true) |
| Ubuntu 18.04 | N/A | ![Bionic](https://travis-matrix-badges.herokuapp.com/repos/aboualiaa/freesurfer/branches/fs-modernize/7?use_travis_com=true) | ![Bionic](https://travis-matrix-badges.herokuapp.com/repos/aboualiaa/freesurfer/branches/fs-modernize/4?use_travis_com=true) |

|Tests|
|:--:|
|[![codecov](https://codecov.io/gh/aboualiaa/freesurfer/branch/fs-modernize/graph/badge.svg)](https://codecov.io/gh/aboualiaa/freesurfer)|

|Code Quality|
|:--:|
|![Code Size](https://img.shields.io/github/repo-size/aboualiaa/freesurfer)|
|![Issues](https://img.shields.io/github/issues/aboualiaa/freesurfer)|
|[![Codacy Badge](https://api.codacy.com/project/badge/Grade/152dd9ab1aff497999ffb1872b9718f5)](https://www.codacy.com/manual/aboualiaa/freesurfer?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=aboualiaa/freesurfer&amp;utm_campaign=Badge_Grade)|

|Activity|
|:--:|
|![Contributers](https://img.shields.io/github/contributors/aboualiaa/freesurfer)|
|![Pull Requests](https://img.shields.io/github/issues-pr/aboualiaa/freesurfer)|
|![Last Commit](https://img.shields.io/github/last-commit/aboualiaa/freesurfer)|
|![Version](https://img.shields.io/github/v/tag/aboualiaa/freesurfer)|

## Introduction

FreeSurfer is a software package for the analysis and visualization of neuroimaging data from cross-sectional and longitudinal studies. It is developed by the [Laboratory for Computational Neuroimaging](https://www.martinos.org/lab/lcn) at the [Martinos Center for Biomedical Imaging](https://www.nmr.mgh.harvard.edu).

FreeSurfer provides full processing streams for structural and functional MRI and includes tools for linear and nonlinear registration, cortical and subcortical segmentation, cortical surface reconstruction, statistical analysis of group morphometry, diffusion MRI, PET analysis, and *much more*. It is also the structural MRI analysis software of choice for the [Human Connectome Project](http://www.humanconnectomeproject.org/about).

For expansive documentation on using and understanding FreeSurfer tools, please visit the [FS Wiki](https://surfer.nmr.mgh.harvard.edu/fswiki).

## Developer

Please review the [contribution guidelines](https://github.com/freesurfer/freesurfer/blob/dev/CONTRIBUTING.md) and visit our [build guide](https://surfer.nmr.mgh.harvard.edu/fswiki/BuildGuide) for step-by-step instructions on configuring and building the source code.

## Support

If you've encountered an issue or have a question about using FreeSurfer, follow these steps to get help:

  1. **Search the archives:** It is highly likely that someone else has run into the same problem before you. The fastest way to get answers is to [search the archives](https://www.mail-archive.com/freesurfer@nmr.mgh.harvard.edu) of our mailing list.

  2. **Ask a question:** [Subscribe](http://mail.nmr.mgh.harvard.edu/mailman/listinfo/freesurfer) to the mailing list and start a discussion!

If you've found a bug in the software, feel free to open an issue or submit a patch after reviewing our [CONTRIBUTING.md](https://github.com/freesurfer/freesurfer/blob/dev/CONTRIBUTING.md) documentation.

## License

Terms and conditions for use, reproduction, distribution and contribution are found in [LICENSE.txt](https://github.com/freesurfer/freesurfer/blob/dev/LICENSE.txt).

## Milestones

- [x] ITK 5
- [x] Catalina: Apple Clang 11
- [ ] ContiniousIntegration
    - [x] macOS
    - [ ] Ubuntu 14.04
    - [ ] Ubuntu 16.04
    - [ ] Ubuntu 18.04
    - [ ] Catalina: LLVM Clang 9
    - [ ] Catalina: GCC 9
- [ ] no banned functions
- [ ] Unify Interface using Boost Program Options
- [ ] TDD (Boost/Google/Catch2?)
- [ ] Benchmarks
- [ ] DataOrientedDesign
- [ ] Logging (spdlog)
- [ ] C++17/20
- [ ] Documentation
- [ ] Guides
