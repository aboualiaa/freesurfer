#ifndef __kvlAtlasMeshViewingConsole_h
#define __kvlAtlasMeshViewingConsole_h

#include "kvlAtlasMeshCollection.h"
#include "kvlAtlasMeshViewingConsoleGUI.h"
#include "kvlCompressionLookupTable.h"

namespace kvl {

class AtlasMeshViewingConsole : public kvlAtlasMeshViewingConsoleGUI {

public:
  //
  AtlasMeshViewingConsole();

  //
  //  virtual ~AtlasMeshViewingConsole();

  //
  void LoadMeshCollection(
      const char *fileName, const int *templateSize = 0,
      const std::string &backgroundImageFileName = std::string());

  //
  void Show();

  //
  void ShowSelectedView() override;

  //
  void GetScreenShot() override;

  //
  void GetScreenShotSeries() override;

  //
  void DumpImage() override;

  //
  void GetScreenShotSeries(int directionNumber) override;

protected:
  //
  void Draw() override;

  void SelectTriangleContainingPoint(float, float) override {};

  void SetSliceLocation(unsigned int sagittalSliceNumber,
                        unsigned int coronalSliceNumber,
                        unsigned int axialSliceNumber) override;

  //
  typedef kvl::ImageViewer::ImageType ImageType;
  static ImageType::Pointer
  ReadBackgroundImage(const std::string &backgroundImageFileName);

private:
  AtlasMeshCollection::Pointer m_MeshCollection;
  // AtlasMesh::CellIdentifier  m_EdgeIdToHighlight;
  ImageType::Pointer m_BackgroundImage;

  CompressionLookupTable::Pointer m_Compressor;
};

} // end namespace kvl

#endif
