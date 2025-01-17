#include "PadSignal.h"
#include <iostream>

using namespace std;

#include "TH1F.h"
#include "TPad.h"
#include "TROOT.h"

#include "TCanvas.h"
#include "TPolyLine.h"
#include "TMath.h"
#include "RQ_OBJECT.h"

//-----------------------------------------------------------------------------
// Primitive Class for KeyStone Pad :
//-----------------------------------
//TCanvas *PadSignal::fCanvas;
TPad *PadSignal::fPad;
//PadSignal::PadSignal(Pixel& P, TCanvas* canvas)
PadSignal::PadSignal(Pixel& P, TPad* pad)
{
  //fCanvas = canvas;
  fPad = pad;
  fi = P.coordi();
  fj = P.coordj();
  fId =P.id();
  fampl = P.ampl();
  // Initialize TPolyLine corresponding to associate Pixel
  Float_t xPad[4] = {P.coordx()-0.5*geom::dx, P.coordx()-0.5*geom::dx, P.coordx()+0.5*geom::dx, P.coordx()+0.5*geom::dx};
  Float_t yPad[4] = {P.coordy()-0.5*geom::dy, P.coordy()+0.5*geom::dy, P.coordy()+0.5*geom::dy, P.coordy()-0.5*geom::dy};
  fPolyLine = new TPolyLine(4,xPad,yPad);
  fPolyLine->SetLineColor(kGray);
  fPolyLine->SetLineWidth(1);
}


// Mouse button event handler
void PadSignal::ExecuteEvent(Int_t event, Int_t px, Int_t py, TObject *sel)
{
  //cout << "Cursor x : " << fCanvas->AbsPixeltoX(px) << " Cursor y : " << fCanvas->AbsPixeltoX(py) << endl;
  if (event == kButton1Down )
  {
      cout << "Click" << endl;
      // Is the mouse inside ?
      //if ( TMath::IsInside(fCanvas->AbsPixeltoX(px), fCanvas->AbsPixeltoY(py), 5, fPolyLine->GetX(), fPolyLine->GetY()))
      if ( TMath::IsInside(fPad->AbsPixeltoX(px), fPad->AbsPixeltoY(py), 5, fPolyLine->GetX(), fPolyLine->GetY()))
      {
        cout << "Mouse is inside pad " << fId << endl;
      }
  }
  PadSelected( fId );
}

void PadSignal::PadSelected(Int_t)
{
   // Emit PadSelected() signal.
   Emit("PadSelected(Int_t)", fId);
}
