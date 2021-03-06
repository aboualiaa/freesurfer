#ifndef FS_ITK_LEGACY_REMOVE
#include "itkPlatformMultiThreader.h"
#define ITK_THREAD_RETURN_TYPE          itk::ITK_THREAD_RETURN_TYPE
#define ITK_THREAD_RETURN_DEFAULT_VALUE itk::ITK_THREAD_RETURN_DEFAULT_VALUE
#else
#include "itkMultiThreader.h"
#define PlatformMultiThreader           MultiThreader
#define GetNumberOfWorkUnitsUsed        GetNumberOfThreadsUsed
#define MultiThreaderBase               MultiThreader
#define WorkUnitInfo                    ThreadInfoStruct
#define SetNumberOfWorkUnits            SetNumberOfThreads
#define ITK_THREAD_RETURN_DEFAULT_VALUE ITK_THREAD_RETURN_VALUE
#define NumberOfWorkUnits               NumberOfThreads
#define WorkUnitID                      ThreadID
#define GetNumberOfWorkUnits            GetNumberOfThreads
#endif
