// generated by Fast Light User Interface Designer (fluid) version 1.0305

#ifndef kvlAtlasMeshViewingConsoleGUI_h
#define kvlAtlasMeshViewingConsoleGUI_h
#include <FL/Fl.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Round_Button.H>
#include <FL/Fl_Value_Slider.H>

#include "kvlImageViewer.h"

class kvlAtlasMeshViewingConsoleGUI {
public:
  kvlAtlasMeshViewingConsoleGUI();
  Fl_Double_Window *m_Window;

private:
  inline void cb_m_Window_i(Fl_Double_Window *, void *);
  static void cb_m_Window(Fl_Double_Window *, void *);

public:
  kvl::ImageViewer *m_ImageViewer;

private:
  inline void cb_m_ImageViewer_i(kvl::ImageViewer *, void *);
  static void cb_m_ImageViewer(kvl::ImageViewer *, void *);

public:
  Fl_Choice *m_MeshNumber;

private:
  inline void cb_m_MeshNumber_i(Fl_Choice *, void *);
  static void cb_m_MeshNumber(Fl_Choice *, void *);

public:
  Fl_Check_Button *m_ShowMesh;

private:
  inline void cb_m_ShowMesh_i(Fl_Check_Button *, void *);
  static void cb_m_ShowMesh(Fl_Check_Button *, void *);

public:
  Fl_Choice *m_LabelNumber;

private:
  inline void cb_m_LabelNumber_i(Fl_Choice *, void *);
  static void cb_m_LabelNumber(Fl_Choice *, void *);

public:
  Fl_Value_Slider *m_CoronalSliceNumber;

private:
  inline void cb_m_CoronalSliceNumber_i(Fl_Value_Slider *, void *);
  static void cb_m_CoronalSliceNumber(Fl_Value_Slider *, void *);

public:
  Fl_Value_Slider *m_SagittalSliceNumber;

private:
  inline void cb_m_SagittalSliceNumber_i(Fl_Value_Slider *, void *);
  static void cb_m_SagittalSliceNumber(Fl_Value_Slider *, void *);

public:
  Fl_Value_Slider *m_AxialSliceNumber;

private:
  inline void cb_m_AxialSliceNumber_i(Fl_Value_Slider *, void *);
  static void cb_m_AxialSliceNumber(Fl_Value_Slider *, void *);

public:
  Fl_Check_Button *m_ShowSummary;

private:
  inline void cb_m_ShowSummary_i(Fl_Check_Button *, void *);
  static void cb_m_ShowSummary(Fl_Check_Button *, void *);

public:
  Fl_Round_Button *m_ViewOne;

private:
  inline void cb_m_ViewOne_i(Fl_Round_Button *, void *);
  static void cb_m_ViewOne(Fl_Round_Button *, void *);

public:
  Fl_Round_Button *m_ViewTwo;

private:
  inline void cb_m_ViewTwo_i(Fl_Round_Button *, void *);
  static void cb_m_ViewTwo(Fl_Round_Button *, void *);

public:
  Fl_Round_Button *m_ViewFour;

private:
  inline void cb_m_ViewFour_i(Fl_Round_Button *, void *);
  static void cb_m_ViewFour(Fl_Round_Button *, void *);

public:
  Fl_Round_Button *m_ViewThree;

private:
  inline void cb_m_ViewThree_i(Fl_Round_Button *, void *);
  static void cb_m_ViewThree(Fl_Round_Button *, void *);

public:
  Fl_Round_Button *m_ViewFive;

private:
  inline void cb_m_ViewFive_i(Fl_Round_Button *, void *);
  static void cb_m_ViewFive(Fl_Round_Button *, void *);
  inline void cb_Screen_i(Fl_Button *, void *);
  static void cb_Screen(Fl_Button *, void *);
  inline void cb_Screen1_i(Fl_Button *, void *);
  static void cb_Screen1(Fl_Button *, void *);
  inline void cb_Dump_i(Fl_Button *, void *);
  static void cb_Dump(Fl_Button *, void *);
  inline void cb_Screen2_i(Fl_Button *, void *);
  static void cb_Screen2(Fl_Button *, void *);
  inline void cb_Screen3_i(Fl_Button *, void *);
  static void cb_Screen3(Fl_Button *, void *);
  inline void cb_Screen4_i(Fl_Button *, void *);
  static void cb_Screen4(Fl_Button *, void *);

public:
  Fl_Check_Button *m_InvertOrder;
  ~kvlAtlasMeshViewingConsoleGUI() = default;
  ;
  virtual void Draw() = 0;
  //  virtual void SelectTriangleContainingPoint(float, float)                = 0;
  virtual void SetSliceLocation(unsigned int, unsigned int, unsigned int) = 0;
  virtual void ShowSelectedView()                                         = 0;
  virtual void GetScreenShot()                                            = 0;
  virtual void GetScreenShotSeries()                                      = 0;
  virtual void DumpImage()                                                = 0;
  virtual void GetScreenShotSeries(int directionNumber)                   = 0;
};
#endif
