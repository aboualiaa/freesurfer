#ifndef _AppendBundleFilter_h_
#define _AppendBundleFilter_h_
#include <iostream>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

#include "itkObject.h"

class AppendBundleFilter : public itk::Object {
public:
  using Self         = AppendBundleFilter;
  using Superclass   = Object;
  using Pointer      = itk::SmartPointer<Self>;
  using ConstPointer = itk::SmartPointer<const Self>;

  itkNewMacro(Self);
  itkTypeMacro(AppendBundleFilter, Object);

  void SetInput(std::vector<vtkSmartPointer<vtkPolyData>> list) {
    this->bundleList = list;
  }
  void         SetNumberOfColours(int number) { this->colorNumber = number; }
  void         SetRepresentatives(bool rep) { this->rep = rep; }
  vtkPolyData *GetOutput() { return this->allBundles; }
  void         Update();

protected:
  AppendBundleFilter() {}
  ~AppendBundleFilter(){};

private:
  int  colorNumber;
  bool rep;

  vtkSmartPointer<vtkPolyData>              allBundles;
  std::vector<vtkSmartPointer<vtkPolyData>> bundleList;
};

#include "AppendBundleFilter.txx"
#endif
