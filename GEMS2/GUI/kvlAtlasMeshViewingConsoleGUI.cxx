// generated by Fast Light User Interface Designer (fluid) version 1.0305

#include "kvlAtlasMeshViewingConsoleGUI.h"

void kvlAtlasMeshViewingConsoleGUI::cb_m_Window_i(Fl_Double_Window *, void *) {
  exit(0);
}
void kvlAtlasMeshViewingConsoleGUI::cb_m_Window(Fl_Double_Window *o, void *v) {
  (static_cast<kvlAtlasMeshViewingConsoleGUI *>(o->user_data()))->cb_m_Window_i(o, v);
}

void kvlAtlasMeshViewingConsoleGUI::cb_m_ImageViewer_i(kvl::ImageViewer *,
                                                       void *) {
  // this->SelectTriangleContainingPoint( Fl::event_x(), m_ImageViewer->h() -
  // Fl::event_y() );
}
void kvlAtlasMeshViewingConsoleGUI::cb_m_ImageViewer(kvl::ImageViewer *o,
                                                     void *v) {
  (static_cast<kvlAtlasMeshViewingConsoleGUI *>(o->parent()->user_data()))
      ->cb_m_ImageViewer_i(o, v);
}

void kvlAtlasMeshViewingConsoleGUI::cb_m_MeshNumber_i(Fl_Choice *, void *) {
  this->Draw();
}
void kvlAtlasMeshViewingConsoleGUI::cb_m_MeshNumber(Fl_Choice *o, void *v) {
  (static_cast<kvlAtlasMeshViewingConsoleGUI *>(o->parent()->user_data()))
      ->cb_m_MeshNumber_i(o, v);
}

void kvlAtlasMeshViewingConsoleGUI::cb_m_ShowMesh_i(Fl_Check_Button *, void *) {
  this->Draw();
}
void kvlAtlasMeshViewingConsoleGUI::cb_m_ShowMesh(Fl_Check_Button *o, void *v) {
  (static_cast<kvlAtlasMeshViewingConsoleGUI *>(o->parent()->user_data()))
      ->cb_m_ShowMesh_i(o, v);
}

void kvlAtlasMeshViewingConsoleGUI::cb_m_LabelNumber_i(Fl_Choice *, void *) {
  this->Draw();
}
void kvlAtlasMeshViewingConsoleGUI::cb_m_LabelNumber(Fl_Choice *o, void *v) {
  (static_cast<kvlAtlasMeshViewingConsoleGUI *>(o->parent()->user_data()))
      ->cb_m_LabelNumber_i(o, v);
}

void kvlAtlasMeshViewingConsoleGUI::cb_m_CoronalSliceNumber_i(Fl_Value_Slider *,
                                                              void *) {
  this->SetSliceLocation(
      static_cast<unsigned int>(m_SagittalSliceNumber->value()),
      static_cast<unsigned int>(m_CoronalSliceNumber->value()),
      static_cast<unsigned int>(m_AxialSliceNumber->value()));
}
void kvlAtlasMeshViewingConsoleGUI::cb_m_CoronalSliceNumber(Fl_Value_Slider *o,
                                                            void *v) {
  (static_cast<kvlAtlasMeshViewingConsoleGUI *>(o->parent()->user_data()))
      ->cb_m_CoronalSliceNumber_i(o, v);
}

void kvlAtlasMeshViewingConsoleGUI::cb_m_SagittalSliceNumber_i(
    Fl_Value_Slider *, void *) {
  m_CoronalSliceNumber->do_callback();
}
void kvlAtlasMeshViewingConsoleGUI::cb_m_SagittalSliceNumber(Fl_Value_Slider *o,
                                                             void *v) {
  (static_cast<kvlAtlasMeshViewingConsoleGUI *>(o->parent()->user_data()))
      ->cb_m_SagittalSliceNumber_i(o, v);
}

void kvlAtlasMeshViewingConsoleGUI::cb_m_AxialSliceNumber_i(Fl_Value_Slider *,
                                                            void *) {
  m_CoronalSliceNumber->do_callback();
}
void kvlAtlasMeshViewingConsoleGUI::cb_m_AxialSliceNumber(Fl_Value_Slider *o,
                                                          void *v) {
  (static_cast<kvlAtlasMeshViewingConsoleGUI *>(o->parent()->user_data()))
      ->cb_m_AxialSliceNumber_i(o, v);
}

void kvlAtlasMeshViewingConsoleGUI::cb_m_ShowSummary_i(Fl_Check_Button *,
                                                       void *) {
  this->Draw();
}
void kvlAtlasMeshViewingConsoleGUI::cb_m_ShowSummary(Fl_Check_Button *o,
                                                     void *v) {
  (static_cast<kvlAtlasMeshViewingConsoleGUI *>(o->parent()->user_data()))
      ->cb_m_ShowSummary_i(o, v);
}

void kvlAtlasMeshViewingConsoleGUI::cb_m_ViewOne_i(Fl_Round_Button *, void *) {
  this->ShowSelectedView();
}
void kvlAtlasMeshViewingConsoleGUI::cb_m_ViewOne(Fl_Round_Button *o, void *v) {
  (static_cast<kvlAtlasMeshViewingConsoleGUI *>(o->parent()->parent()->user_data()))
      ->cb_m_ViewOne_i(o, v);
}

void kvlAtlasMeshViewingConsoleGUI::cb_m_ViewTwo_i(Fl_Round_Button *, void *) {
  this->ShowSelectedView();
}
void kvlAtlasMeshViewingConsoleGUI::cb_m_ViewTwo(Fl_Round_Button *o, void *v) {
  (static_cast<kvlAtlasMeshViewingConsoleGUI *>(o->parent()->parent()->user_data()))
      ->cb_m_ViewTwo_i(o, v);
}

void kvlAtlasMeshViewingConsoleGUI::cb_m_ViewFour_i(Fl_Round_Button *, void *) {
  this->ShowSelectedView();
}
void kvlAtlasMeshViewingConsoleGUI::cb_m_ViewFour(Fl_Round_Button *o, void *v) {
  (static_cast<kvlAtlasMeshViewingConsoleGUI *>(o->parent()->parent()->user_data()))
      ->cb_m_ViewFour_i(o, v);
}

void kvlAtlasMeshViewingConsoleGUI::cb_m_ViewThree_i(Fl_Round_Button *,
                                                     void *) {
  this->ShowSelectedView();
}
void kvlAtlasMeshViewingConsoleGUI::cb_m_ViewThree(Fl_Round_Button *o,
                                                   void *v) {
  (static_cast<kvlAtlasMeshViewingConsoleGUI *>(o->parent()->parent()->user_data()))
      ->cb_m_ViewThree_i(o, v);
}

void kvlAtlasMeshViewingConsoleGUI::cb_m_ViewFive_i(Fl_Round_Button *, void *) {
  this->ShowSelectedView();
}
void kvlAtlasMeshViewingConsoleGUI::cb_m_ViewFive(Fl_Round_Button *o, void *v) {
  (static_cast<kvlAtlasMeshViewingConsoleGUI *>(o->parent()->parent()->user_data()))
      ->cb_m_ViewFive_i(o, v);
}

void kvlAtlasMeshViewingConsoleGUI::cb_Screen_i(Fl_Button *, void *) {
  this->GetScreenShot();
}
void kvlAtlasMeshViewingConsoleGUI::cb_Screen(Fl_Button *o, void *v) {
  (static_cast<kvlAtlasMeshViewingConsoleGUI *>(o->parent()->user_data()))
      ->cb_Screen_i(o, v);
}

void kvlAtlasMeshViewingConsoleGUI::cb_Screen1_i(Fl_Button *, void *) {
  this->GetScreenShotSeries();
}
void kvlAtlasMeshViewingConsoleGUI::cb_Screen1(Fl_Button *o, void *v) {
  (static_cast<kvlAtlasMeshViewingConsoleGUI *>(o->parent()->user_data()))
      ->cb_Screen1_i(o, v);
}

void kvlAtlasMeshViewingConsoleGUI::cb_Dump_i(Fl_Button *, void *) {
  this->DumpImage();
}
void kvlAtlasMeshViewingConsoleGUI::cb_Dump(Fl_Button *o, void *v) {
  (static_cast<kvlAtlasMeshViewingConsoleGUI *>(o->parent()->user_data()))
      ->cb_Dump_i(o, v);
}

void kvlAtlasMeshViewingConsoleGUI::cb_Screen2_i(Fl_Button *, void *) {
  this->GetScreenShotSeries(0);
}
void kvlAtlasMeshViewingConsoleGUI::cb_Screen2(Fl_Button *o, void *v) {
  (static_cast<kvlAtlasMeshViewingConsoleGUI *>(o->parent()->user_data()))
      ->cb_Screen2_i(o, v);
}

void kvlAtlasMeshViewingConsoleGUI::cb_Screen3_i(Fl_Button *, void *) {
  this->GetScreenShotSeries(1);
}
void kvlAtlasMeshViewingConsoleGUI::cb_Screen3(Fl_Button *o, void *v) {
  (static_cast<kvlAtlasMeshViewingConsoleGUI *>(o->parent()->user_data()))
      ->cb_Screen3_i(o, v);
}

void kvlAtlasMeshViewingConsoleGUI::cb_Screen4_i(Fl_Button *, void *) {
  this->GetScreenShotSeries(2);
}
void kvlAtlasMeshViewingConsoleGUI::cb_Screen4(Fl_Button *o, void *v) {
  (static_cast<kvlAtlasMeshViewingConsoleGUI *>(o->parent()->user_data()))
      ->cb_Screen4_i(o, v);
}

kvlAtlasMeshViewingConsoleGUI::kvlAtlasMeshViewingConsoleGUI() {
  {
    m_Window = new Fl_Double_Window(1280, 1030, "kvlAtlasMeshViewingConsole");
    m_Window->callback(reinterpret_cast<Fl_Callback *>(cb_m_Window), (void *)(this));
    {
      m_ImageViewer = new kvl::ImageViewer(10, 10, 1000, 1000);
      m_ImageViewer->box(FL_FLAT_BOX);
      m_ImageViewer->color(FL_FOREGROUND_COLOR);
      m_ImageViewer->selection_color(FL_BACKGROUND_COLOR);
      m_ImageViewer->labeltype(FL_NORMAL_LABEL);
      m_ImageViewer->labelfont(0);
      m_ImageViewer->labelsize(14);
      m_ImageViewer->labelcolor(FL_FOREGROUND_COLOR);
      m_ImageViewer->callback(reinterpret_cast<Fl_Callback *>(cb_m_ImageViewer));
      m_ImageViewer->align(Fl_Align(FL_ALIGN_TOP));
      m_ImageViewer->when(FL_WHEN_RELEASE);
      m_ImageViewer->end();
    } // kvl::ImageViewer* m_ImageViewer
    {
      m_MeshNumber = new Fl_Choice(1145, 30, 110, 25, "Mesh number:");
      m_MeshNumber->down_box(FL_BORDER_BOX);
      m_MeshNumber->callback(reinterpret_cast<Fl_Callback *>(cb_m_MeshNumber));
    } // Fl_Choice* m_MeshNumber
    {
      m_ShowMesh = new Fl_Check_Button(1230, 125, 25, 25, "Show mesh:");
      m_ShowMesh->down_box(FL_DOWN_BOX);
      m_ShowMesh->value(1);
      m_ShowMesh->callback(reinterpret_cast<Fl_Callback *>(cb_m_ShowMesh));
      m_ShowMesh->align(Fl_Align(FL_ALIGN_LEFT));
    } // Fl_Check_Button* m_ShowMesh
    {
      m_LabelNumber = new Fl_Choice(1145, 75, 110, 25, "Label number:");
      m_LabelNumber->down_box(FL_BORDER_BOX);
      m_LabelNumber->callback(reinterpret_cast<Fl_Callback *>(cb_m_LabelNumber));
    } // Fl_Choice* m_LabelNumber
    {
      m_CoronalSliceNumber =
          new Fl_Value_Slider(1066, 250, 175, 25, "Coronal slice number:");
      m_CoronalSliceNumber->type(1);
      m_CoronalSliceNumber->step(1);
      m_CoronalSliceNumber->textsize(14);
      m_CoronalSliceNumber->callback(reinterpret_cast<Fl_Callback *>(cb_m_CoronalSliceNumber));
      m_CoronalSliceNumber->align(Fl_Align(FL_ALIGN_TOP));
      m_CoronalSliceNumber->when(FL_WHEN_RELEASE);
    } // Fl_Value_Slider* m_CoronalSliceNumber
    {
      m_SagittalSliceNumber =
          new Fl_Value_Slider(1066, 345, 175, 25, "Sagittal slice number:");
      m_SagittalSliceNumber->type(1);
      m_SagittalSliceNumber->step(1);
      m_SagittalSliceNumber->textsize(14);
      m_SagittalSliceNumber->callback(reinterpret_cast<Fl_Callback *>(cb_m_SagittalSliceNumber));
      m_SagittalSliceNumber->align(Fl_Align(FL_ALIGN_TOP));
      m_SagittalSliceNumber->when(FL_WHEN_RELEASE);
    } // Fl_Value_Slider* m_SagittalSliceNumber
    {
      m_AxialSliceNumber =
          new Fl_Value_Slider(1066, 436, 175, 25, "Axial slice number:");
      m_AxialSliceNumber->type(1);
      m_AxialSliceNumber->step(1);
      m_AxialSliceNumber->textsize(14);
      m_AxialSliceNumber->callback(reinterpret_cast<Fl_Callback *>(cb_m_AxialSliceNumber));
      m_AxialSliceNumber->align(Fl_Align(FL_ALIGN_TOP));
      m_AxialSliceNumber->when(FL_WHEN_RELEASE);
    } // Fl_Value_Slider* m_AxialSliceNumber
    {
      m_ShowSummary = new Fl_Check_Button(1230, 170, 25, 25, "Show summary:");
      m_ShowSummary->down_box(FL_DOWN_BOX);
      m_ShowSummary->callback(reinterpret_cast<Fl_Callback *>(cb_m_ShowSummary));
      m_ShowSummary->align(Fl_Align(FL_ALIGN_LEFT));
    } // Fl_Check_Button* m_ShowSummary
    {
      auto *o = new Fl_Group(1088, 598, 124, 109, "View");
      o->box(FL_DOWN_BOX);
      {
        m_ViewOne = new Fl_Round_Button(1098, 605, 29, 29);
        m_ViewOne->type(102);
        m_ViewOne->down_box(FL_ROUND_DOWN_BOX);
        m_ViewOne->callback(reinterpret_cast<Fl_Callback *>(cb_m_ViewOne));
        m_ViewOne->align(Fl_Align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE));
      } // Fl_Round_Button* m_ViewOne
      {
        m_ViewTwo = new Fl_Round_Button(1178, 604, 27, 28);
        m_ViewTwo->type(102);
        m_ViewTwo->down_box(FL_ROUND_DOWN_BOX);
        m_ViewTwo->callback(reinterpret_cast<Fl_Callback *>(cb_m_ViewTwo));
      } // Fl_Round_Button* m_ViewTwo
      {
        m_ViewFour = new Fl_Round_Button(1178, 670, 25, 29);
        m_ViewFour->type(102);
        m_ViewFour->down_box(FL_ROUND_DOWN_BOX);
        m_ViewFour->callback(reinterpret_cast<Fl_Callback *>(cb_m_ViewFour));
      } // Fl_Round_Button* m_ViewFour
      {
        m_ViewThree = new Fl_Round_Button(1098, 673, 24, 21);
        m_ViewThree->type(102);
        m_ViewThree->down_box(FL_ROUND_DOWN_BOX);
        m_ViewThree->callback(reinterpret_cast<Fl_Callback *>(cb_m_ViewThree));
      } // Fl_Round_Button* m_ViewThree
      {
        m_ViewFive = new Fl_Round_Button(1133, 637, 30, 27);
        m_ViewFive->type(102);
        m_ViewFive->down_box(FL_ROUND_DOWN_BOX);
        m_ViewFive->value(1);
        m_ViewFive->callback(reinterpret_cast<Fl_Callback *>(cb_m_ViewFive));
      } // Fl_Round_Button* m_ViewFive
      o->end();
    } // Fl_Group* o
    {
      auto *o = new Fl_Button(1080, 732, 140, 25, "Screen shot");
      o->callback(reinterpret_cast<Fl_Callback *>(cb_Screen));
    } // Fl_Button* o
    {
      auto *o = new Fl_Button(1080, 777, 140, 25, "Screen shot series");
      o->callback(reinterpret_cast<Fl_Callback *>(cb_Screen1));
    } // Fl_Button* o
    {
      auto *o = new Fl_Button(1078, 822, 140, 25, "Dump 3D image");
      o->callback(reinterpret_cast<Fl_Callback *>(cb_Dump));
    } // Fl_Button* o
    {
      auto *o = new Fl_Button(1086, 374, 135, 25, "Screen shot series");
      o->callback(reinterpret_cast<Fl_Callback *>(cb_Screen2));
    } // Fl_Button* o
    {
      auto *o = new Fl_Button(1086, 279, 135, 25, "Screen shot series");
      o->callback(reinterpret_cast<Fl_Callback *>(cb_Screen3));
    } // Fl_Button* o
    {
      auto *o = new Fl_Button(1086, 464, 135, 25, "Screen shot series");
      o->callback(reinterpret_cast<Fl_Callback *>(cb_Screen4));
    } // Fl_Button* o
    {
      m_InvertOrder = new Fl_Check_Button(1181, 500, 25, 25, "Invert order:");
      m_InvertOrder->down_box(FL_DOWN_BOX);
      m_InvertOrder->align(Fl_Align(FL_ALIGN_LEFT));
    } // Fl_Check_Button* m_InvertOrder
    m_Window->end();
  } // Fl_Double_Window* m_Window
}
