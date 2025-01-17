{
  const int kPhysChannelMax  = 1728;
  const int kASIC            = 24;
  const int kFEM             = 7;
  const int kFemNumber       = 0;
  const int kNPoints = 4;

  double x[kPhysChannelMax][kNPoints];
  double y[kPhysChannelMax][kNPoints];

  char filename[] = "etc/geom/keystonePadGeom.txt";

  ifstream padGeomFile;
  padGeomFile.open(filename, std::ios::binary);
  if ( !padGeomFile.is_open() ) {
    cout << "Can't open file : " << filename << endl;
    return -1;
  }

  char   varName[20];
  double width;
  double ymin, ymax;

  padGeomFile >> varName >> width;  // half width of the pad panel
  padGeomFile >> varName >> ymin;
  padGeomFile >> varName >> ymax;

  double tpcPanelWidth  = 0.0;
  double tpcPanelHeight = 0.0;

  tpcPanelWidth  = 2. * width;         // total width  of the pad array
  tpcPanelHeight = ymax - ymin;        // total height of the pad array

  cout << "Pad Panel: w,ymin,ymax = "
       << width << ", " << ymin << ", " << ymax << endl;
  TGraph* graph = new TGraph();
  int point = 0;

  for (int i=0; i<kPhysChannelMax; i++) {
    int id, row, col;
    double x0, y0, x1, y1, x2, y2, x3, y3;
    //padGeomFile >> id >> row >> col
    padGeomFile >> row >> col
                >> x0 >> y0 >> x1 >> y1 >> x2 >> y2 >> x3 >> y3;
    id = i;
    x[id][0] = x0;    y[id][0] = y0;
    x[id][1] = x1;    y[id][1] = y1;
    x[id][2] = x2;    y[id][2] = y2;
    x[id][3] = x3;    y[id][3] = y3;
  }

  padGeomFile.close();

  const double scale  = 1.;
  const double xscale = scale * 1./tpcPanelWidth;
  const double yscale = scale * 1./tpcPanelHeight;

  for (int iFem=0; iFem<kFEM; iFem++) {
   for (int ipad=0; ipad<kPhysChannelMax; ipad++) {
    double xCorner[kNPoints];
    double yCorner[kNPoints];

     for (int ic=0; ic<kNPoints; ic++) {
        double xCor = x[ipad][ic] * xscale ;
        double yCor = ( y[ipad][ic] + 1520 )* yscale ;
        if (iFem==2) {
          xCorner[ic] = xCor*cos(0.149488369/1.33) - yCor*sin(0.149488369/1.33) /*+ 0.5*/;
          yCorner[ic] = yCor*cos(0.149488369/1.33) + xCor*sin(0.149488369/1.33) /*+ 0.5*/ - 1520*yscale;
        }
        if (iFem==3) {
          xCorner[ic] = xCor*cos(0.003055244/1.33) - yCor*sin(0.003055244/1.33) /*+ 0.5*/;
          yCorner[ic] = yCor*cos(0.003055244/1.33) + xCor*sin(0.003055244/1.33) /*+ 0.5*/ - 1520*yscale;
        }
        if (iFem==4) {
          xCorner[ic] = xCor*cos(-0.143203347/1.33) - yCor*sin(-0.143203347/1.33) /*+ 0.5*/;
          yCorner[ic] = yCor*cos(-0.143203347/1.33) + xCor*sin(-0.143203347/1.33) /*+ 0.5*/ - 1520*yscale;
        }
        if (iFem==0) {
          xCorner[ic] = xCor*cos(0.088401845/1.33) - yCor*sin(0.088401845/1.33) /*+ 0.5*/;
          yCorner[ic] = yCor*cos(0.088401845/1.33) + xCor*sin(0.088401845/1.33) /*+ 0.5*/ - 1520*yscale+175*yscale;
        }
        if (iFem==1) {
          xCorner[ic] = xCor*cos(-0.05803128/1.33) - yCor*sin(-0.05803128/1.33) /*+ 0.5*/;
          yCorner[ic] = yCor*cos(-0.05803128/1.33) + xCor*sin(-0.05803128/1.33) /*+ 0.5*/ - 1520*yscale+175*yscale;
        }

        if (iFem==5) {
          xCorner[ic] = xCor*cos(0.05349526/1.33) - yCor*sin(0.05349526/1.33) /*+ 0.5*/;
          yCorner[ic] = yCor*cos(0.05349526/1.33) + xCor*sin(0.05349526/1.33) /*+ 0.5*/ - 1520*yscale - 175*yscale;
        }
        if (iFem==6) {
          xCorner[ic] = xCor*cos(-0.092937865/1.33) - yCor*sin(-0.092937865/1.33) /*+ 0.5*/;
          yCorner[ic] = yCor*cos(-0.092937865/1.33) + xCor*sin(-0.092937865/1.33) /*+ 0.5*/ - 1520*yscale - 175*yscale;
        }

        xCorner[ic] *= tpcPanelWidth;
        yCorner[ic] *= tpcPanelHeight;
      }
      graph->SetPoint(point, xCorner[0], yCorner[0]);
      ++point;
    }
    // print data
    cout << "FEM = " <<  iFem << "   x0[0][0] = " << xCorner[0] << "   y0[0][0] =  " << yCorner[0] << endl;
    cout << "wifth = " << tpcPanelWidth << "   Height = " << tpcPanelHeight << endl;

  }
  graph->SetMarkerColor(4);
  graph->SetMarkerSize(0.2);
  graph->SetMarkerStyle(21);
  graph->Draw("ap");
}
