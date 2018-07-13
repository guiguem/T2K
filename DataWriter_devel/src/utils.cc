#include "interface/utils.hh"


/// ------------------- just refining the ROOT TStyle
#include <TStyle.h>
#include <TROOT.h>
#include <TColor.h>
#include <TTree.h>
#include <TChain.h>

#include <string>
#include <fstream>
#include <iostream>

TChain* addInputFiles( std::string filelist, std::string treename ) {
  TChain *ztree = new TChain( treename.c_str() );
  
  std::ifstream input(filelist.c_str());  
  while ( input.good() && !input.eof() ) {
    std::string file;
    getline(input,file,'\n');    
    /// comment
    if( file.find("#") != std::string::npos ) continue;
    if( file.size() < 2 ) continue;
    std::cout << " adding file: " << file << std::endl;
    ztree->Add( file.c_str() );
  }
  input.close();
  return ztree;
}

std::string firstFileInList( std::string filelist ) {
  std::string out = "unknown";

  std::ifstream input(filelist.c_str());  
  while ( input.good() && !input.eof() ) {
    std::string file;
    getline(input,file,'\n');    
    /// comment
    if( file.find("#") != std::string::npos ) continue;
    if( file.size() < 2 ) continue;
    out = file;
    break;
  }
  input.close();

  return out;
}

void rootStyle(void) {
   int font = 132;
   /// Dzero font 
   font = 62; 

  
  gROOT->SetStyle("Plain");

  TStyle *fabercStyle= new TStyle("ZFitterFabStyle","ZFitter plots");

  // use plain black on white colors
  fabercStyle->SetFrameBorderMode(0);
  fabercStyle->SetCanvasBorderMode(0);
  fabercStyle->SetPadBorderMode(0);
  fabercStyle->SetPadColor(0);
  fabercStyle->SetCanvasColor(0);
  //fabercStyle->SetTitleColor(0);
  fabercStyle->SetStatColor(0);
  fabercStyle->SetTitleFillColor(0);

  // set the paper & margin sizes
  fabercStyle->SetPaperSize(20,26);
  fabercStyle->SetPadTopMargin(0.08);
  //  fabercStyle->SetPadRightMargin(0.06);
  fabercStyle->SetPadRightMargin(0.15);
  fabercStyle->SetPadBottomMargin(0.13);
  fabercStyle->SetPadLeftMargin(0.12);
  fabercStyle->SetTitleOffset(1.,"y");
  fabercStyle->SetTitleOffset(0.9,"x");

  // use large Times-Roman fonts
  fabercStyle->SetTitleFont(font,"xyz");  // set the all 3 axes title font
  fabercStyle->SetTitleFont(font," ");    // set the pad title font
  fabercStyle->SetTitleSize(0.06,"xyz"); // set the 3 axes title size
  fabercStyle->SetTitleSize(0.06," ");   // set the pad title size
  fabercStyle->SetLabelFont(font,"xyz");
  fabercStyle->SetLabelSize(0.05,"xyz");
  fabercStyle->SetTextFont(42);
  fabercStyle->SetTextSize(0.09);
  fabercStyle->SetStatFont(font);

  // add axis decimal (same for all)
  fabercStyle->SetStripDecimals(0);

  // use bold lines and markers
  fabercStyle->SetMarkerStyle(8);
  fabercStyle->SetMarkerSize(0.8);
  fabercStyle->SetHistLineWidth(2);
  fabercStyle->SetLineWidth(2);
  fabercStyle->SetLineStyleString(2,"[12 12]"); // postscript dashes

  //..Get rid of X error bars
  //fabercStyle->SetErrorX(0.001);

  // do not display any of the standard histogram decorations
  fabercStyle->SetOptTitle(0);  
  //  fabercStyle->SetOptStat(111111);
  //  fabercStyle->SetOptStat(1111);
  fabercStyle->SetOptStat(0);
  fabercStyle->SetOptFit(0);
  // fabercStyle->SetPalette(1);

  fabercStyle->SetTitleX(0.2);

  unsigned NRGBs = 5;
  double Red[5]    = { 0.00, 0.2, 0.0, 1.0, 0.70};
  double Green[5]  = { 0.00, 0.8, 0.8, 1.0,  0.00};
  double Blue[5]   = { 0.60, 1.0, 0.1, 0.0,  0.00};
  double Length[5] = { 0.00, 0.42, 0.50, 0.58, 1.00 };
  int nb=50;
  TColor::CreateGradientColorTable(NRGBs, Length, Red, Green, Blue, nb);
  fabercStyle->SetNumberContours(nb);


  // put tick marks on top and RHS of plots
  fabercStyle->SetPadTickX(1);
  fabercStyle->SetPadTickY(1);

  // restore the plain style. Add tick marks and extra stats
  gROOT->SetStyle("ZFitterFabStyle");

}
