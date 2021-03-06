project(utils)

# configure cmake version macros
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/include)
configure_file(version_info.h.in version_info.h)

set(COMMON_LIBS
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/packages/jpeg
    ${CMAKE_SOURCE_DIR}/packages/minc
    ${CMAKE_SOURCE_DIR}/packages/netcdf
    ${CMAKE_SOURCE_DIR}/packages/nrrdio
    ${CMAKE_SOURCE_DIR}/packages/dicom
    ${CMAKE_SOURCE_DIR}/packages/cephes
    ${CMAKE_SOURCE_DIR}/packages/nifti
    ${CMAKE_SOURCE_DIR}/packages/gifti
    ${FS_INCLUDE_DIRS}
    SYSTEM
    ${ITK_INCLUDE_DIRS}
    )

set(INCOMPATIBLE_BUILD_TYPES "cfisan;fulllto;thinlto;asan")
if(CMAKE_BUILD_TYPE IN_LIST INCOMPATIBLE_BUILD_TYPES)
  set(UTILS_BUILD_TYPE "STATIC")
else()
  set(UTILS_BUILD_TYPE "SHARED")
endif()

add_library(utils
            ${UTILS_BUILD_TYPE}
            afni_exec.cpp
            annotation.cpp
            argparse.cpp
            autoencoder.cpp
            bfileio.cpp
            box.cpp
            Bruker.cpp
            chklc.cpp
            class_array.cpp
            cluster.cpp
            cma.cpp
            cmat.cpp
            cmdargs.cpp
            colortab.cpp
            connectcomp.cpp
            corio.cpp
            cpputils.cpp
            ctrpoints.cpp
            cvector.cpp
            dct.cpp
            density.cpp
            diag.cpp
            DICOMRead.cpp
            diffuse.cpp
            dmatrix.cpp
            dti.cpp
            dtk.fs.cpp
            evschutils.cpp
            fcd.cpp
            fftutils.cpp
            field_code.cpp
            filter.cpp
            fio.cpp
            flash.cpp
            fmriutils.cpp
            fs_cost_function.cpp
            fs_lbfgs.cpp
            fs_lbfgs_observer.cpp
            fs_lbfgs_subject.cpp
            fs_powell.cpp
            fsenv.cpp
            fsgdf.cpp
            fsglm.cpp
            fsinit.cpp
            fsPrintHelp.cpp
            gca.cpp
            gcaboundary.cpp
            gcalinearnode.cpp
            gcalinearprior.cpp
            gcamcomputeLabelsLinearCPU.cpp
            gcamorph.cpp
            gcamorphtestutils.cpp
            gcautils.cpp
            gclass.cpp
            gcsa.cpp
            geos.cpp
            getdelim.cpp
            getline.cpp
            gifti.cpp
            gtm.cpp
            gw_ic2562.cpp
            gw_utils.cpp
            hippo.cpp
            hips.cpp
            histo.cpp
            ic12.cpp
            ic163842.cpp
            ic2562.cpp
            ic42.cpp
            ic642.cpp
            icosahedron.cpp
            image.cpp
            imageio.cpp
            imautils.cpp
            intensity_eig.cpp
            ipmpar.cpp
            joint_histo.cpp
            kernel.cpp
            label.cpp
            log.cpp
            lut.cpp
            machine.cpp
            MARS_DT_Boundary.cpp
            matfile.cpp
            matrix.cpp
            mgh_filter.cpp
            min_heap.cpp
            morph.cpp
            mosaic.cpp
            mri.cpp
            mri2.cpp
            mri_conform.cpp
            mri_fastmarching.cpp
            mri_identify.cpp
            mri_level_set.cpp
            mri_tess.cpp
            mri_topology.cpp
            mriBSpline.cpp
            mriclass.cpp
            mricurv.cpp
            mrifilter.cpp
            mriflood.cpp
            mrihisto.cpp
            mriio.cpp
            MRIio_old.cpp
            mrimorph.cpp
            mrinorm.cpp
            mripolv.cpp
            mriprob.cpp
            mris_compVolFrac.cpp
            mris_fastmarching.cpp
            mrisegment.cpp
            mriset.cpp
            mrishash.cpp
            mrisp.cpp
            MRISrigidBodyAlignGlobal.cpp
            mrisurf.cpp
            mrisurf_base.cpp
            mrisurf_compute_dxyz.cpp
            mrisurf_defect.cpp
            mrisurf_deform.cpp
            mrisurf_integrate.cpp
            mrisurf_io.cpp
            mrisurf_io_stl.cpp
            mrisurf_metricProperties.cpp
            mrisurf_metricProperties_faster.cpp
            mrisurf_mri.cpp
            mrisurf_project.cpp
            mrisurf_sphere_interp.cpp
            mrisurf_sseTerms.cpp
            mrisurf_timeStep.cpp
            mrisurf_topology.cpp
            mrisurf_vals.cpp
            mrisutils.cpp
            mriTransform.cpp
            mrivoxel.cpp
            numerics.cpp
            offset.cpp
            path.cpp
            pdf.cpp
            pgmstubs.cpp
            pointset.cpp
            prime.cpp
            Progname.cpp
            randomfields.cpp
            rbf.cpp
            rbm.cpp
            realm.cpp
            region.cpp
            registerio.cpp
            resample.cpp
            retinotopy.cpp
            rfa.cpp
            rforest.cpp
            rfutils.cpp
            rgb.cpp
            romp_support.cpp
            selxavgio.cpp
            sig.cpp
            signa.cpp
            stats.cpp
            surfcluster.cpp
            surfgrad.cpp
            svm.cpp
            tags.cpp
            talairachex.cpp
            timer.cpp
            transform.cpp
            tritri.cpp
            tukey.cpp
            utils.cpp
            version.cpp
            vertexRotator.cpp
            vlabels.cpp
            volcluster.cpp
            voxlist.cpp
            xDebug.cpp
            xTypes.cpp
            xVoxel.cpp
            )

target_include_directories(utils PUBLIC ${COMMON_LIBS})

if(NOT APPLE)
  target_link_libraries(utils tiff)
endif()

target_link_libraries(utils
                      jpeg
                      ${TIFF_LIBRARIES}
                      ${LIBXML2_LIBRARIES}
                      ${EXPAT_LIBRARIES}
                      netcdf
                      minc
                      nrrdio
                      dicom
                      cephes
                      nifti
                      gifti
                      ${ZLIB_LIBRARIES}
                      ${ITK_LIBRARIES}
                      fmt::fmt
                      )

if(NOT APPLE)
  target_link_libraries(utils crypt rt atomic)
endif()

# utils binaries

# xmlToHtml
# add_executable(xmlToHtml xmlToHtml.cpp)
# target_link_libraries(xmlToHtml xml2 ${ZLIB_LIBRARIES})
# install(TARGETS xmlToHtml DESTINATION bin)

# fsPrintHelp
add_executable(fsPrintHelp fsPrintHelp.cpp)
target_compile_definitions(fsPrintHelp PRIVATE BUILD_MAIN)
target_include_directories(fsPrintHelp PUBLIC ${COMMON_LIBS})
target_link_libraries(fsPrintHelp xml2 ${ZLIB_LIBRARIES})
install(TARGETS fsPrintHelp DESTINATION bin)

add_subdirectory(test)

if(BUILD_TESTING)
  add_subdirectory(unittests)
endif()

install(TARGETS utils LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
