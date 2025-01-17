//
// Read the map file created by David, and create another map.
//
// Map : 

#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <cstdio>

using namespace std;

//void ReadMapping() {
int main( int argc, char **argv)
{
  FILE *fLayout = fopen("MapLayoutFile.txt","r");

  // Skip 8 comment lines
  for (int i=0; i<8; i++) {
    char line[160];
    fgets(line, 160, fLayout);
  }


  const int kPhysChanPerAsic = 72;
  const int kPhysChannelMax  = 1728;

  int iASIC[kPhysChannelMax]; // Global ASIC ID       : 0-23
  int iFEC[kPhysChannelMax];  //        FEC  ID       : 0-5
  int iROW[kPhysChannelMax];  // Global Row number    : same as in the map
  int iCOL[kPhysChannelMax];  // Global Column number : same as in the map
  int iPAD[kPhysChannelMax];  // <-- This is the one that we can get 
                              //     from channel data !!!

  for (int i=0; i<kPhysChannelMax; i++)
    iPAD[i] = -1;

  vector<string> missingChanInfo;

  int asic_fec;   // Local ASIC id in a FEC         : 0-3
  int pad;        // Local Pad  id in a ASIC sector : 0-71
  int row;        // 
  int column;     //

  //
  // Read Map 
  //
  for (int i=0 ; i<kPhysChannelMax; i++)
  {
    fscanf(fLayout, "%d %d %d %d", &asic_fec, &row, &column, &pad);

    //
    // Each FEC handles 12 columns sequentially !
    //
    // NOTE: Modify iFEC assignment if we need to change mapping
    //
    int fec;
    if(column>=60)              fec=0;
    if(column>=48 && column<60) fec=1;
    if(column>=36 && column<48) fec=2;
    if(column>=24 && column<36) fec=3;
    if(column>=12 && column<24) fec=4;
    if(column<12)               fec=5;

    //
    // The Map : PhysChan --> (Row, Col)
    //
    // NOTE: PhysChan is the most important one,
    //       which is the one that we can get from the ACQ data file, and
    //       which should be used to find (row, col) of the pad from it !
    //

    //
    // NOTE : 2 pads denoted by -1 are being used for HV.
    //        This results in two missing channel numbers : 72 and 73 !!!
    //
    if ( pad == -1 ) {
      stringstream ss;
      ss << "#  pad at " << row << "," << column << " is for HV";
      missingChanInfo.push_back( ss.str() );
      continue;
    }

    int asic = asic_fec + 4*fec;            // global asic id
    int chan = pad + asic*kPhysChanPerAsic; // to be mapped to (row,col) !!!

    iASIC[chan] = asic; 
    iFEC[chan]  = fec;
    iROW[chan]  = row;
    iCOL[chan]  = column;

    // Check overlapping 
    if ( iPAD[chan] == -1 )
      iPAD[chan]  = chan;
    else {
      cout << "chan " << chan << " is already occupied by other(s)" << endl;
      cout << asic_fec << "," << row << "," << column << "," << pad << endl;
    }

    //cout << "PhysChan: "  << i 
    //     << "\tFEC: "     << iFEC
    //     << "\tASIC: "    << iASIC
    //     << "\tPad: "     << iPAD[i] 
    //     << "\trow,col: " << iROW[iPAD[i]] << "," << iCOL[iPAD[i]];
    //cout << "\t" << asic_fec << "," << row << "," << column;

    //cout << "PhysChan: " << setw(4) << i     << "   "
    //     << "FEC: "      << setw(1) << iFEC  << "   "
    //     << "ASIC: "     << setw(2) << iASIC << "   "
    //     << "row,col: "  << setw(2) << iROW  << "," 
    //                     << setw(2) << iCOL << endl;

  }

  fclose(fLayout);

  //
  // Write a new MAP : chan --> (row, col)
  //
  // To make line parsing simple '#' is prepended to comment lines.
  //
  cout << "#                                          " << endl;
  cout << "# MAP for Data Channel to Layout (Row,Col) " << endl;
  cout << "#                                          " << endl;
  cout << "# Convention:                              " << endl;
  cout << "#    - Looking at the Pad Surface          " << endl;
  cout << "#    - Lower-Left corner is (0,0)          " << endl;
  cout << "#                                          " << endl;
  cout << "# Create by Yun-Ha using David's Map       " << endl;
  cout << "#                                          " << endl;
  cout << "# NOTE:                                    " << endl;
  for ( int i=0; i<missingChanInfo.size(); i++)
    cout << missingChanInfo[i] << endl;
  cout << "#                                          " << endl;
  cout << "#-------------------------------           " << endl;
  cout << "# PhysChan  FEC  ASIC  Row  Col            " << endl;
  cout << "#-------------------------------           " << endl;
  
  for (int chan=0; chan<kPhysChannelMax; chan++)
  {
    // Skip missing pads for HV
    if ( iPAD[chan] == -1 )
      continue;

    cout << setw(9) << chan
         << setw(5) << iFEC[chan]
         << setw(6) << iASIC[chan]
         << setw(5) << iROW[chan]
         << setw(5) << iCOL[chan] << endl;
  }

  return 0;
}
