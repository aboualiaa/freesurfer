#ifndef __kvlAtlasParameterEstimationConsole_h
#define __kvlAtlasParameterEstimationConsole_h

#include "kvlAtlasParameterEstimationConsoleGUI.h"
#include "kvlAtlasParameterEstimator.h"
#include <string>
#include <vector>

namespace kvl {

class AtlasParameterEstimationConsole
    : public kvlAtlasParameterEstimationConsoleGUI {

public:
  //
  AtlasParameterEstimationConsole();

  //
//  ~AtlasParameterEstimationConsole() override = default;;

  //
  void SetLabelImages(const std::vector<std::string> &fileNames);

  //
  void Show();

  //
  void Estimate() override;

  //
  void HandleEstimatorEvent(itk::Object *object, const itk::EventObject &event);

protected:
  //
  void DisplayLabelImage(unsigned int labelImageNumber) override;

  //
  void InitializeMesh() override;

  void SelectTriangleContainingPoint(float /*unused*/,
                                     float /*unused*/) override {};

  //
  void Step() override;

  //
  void Interrupt() override;

  //
  void Continue() override;

  //
  void SetPositionEstimationResolution(
      unsigned int positionEstimationResolution) override;

private:
  AtlasParameterEstimator::Pointer m_Estimator;

  std::string m_TotalProgressLabel;
  std::string m_SubProgressLabel;

  bool m_Interrupted;
  bool m_Stepping;
};

} // end namespace kvl

#endif
