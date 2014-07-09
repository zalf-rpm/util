#include "interpol.h"


#define SIM_LENGTH 1 // 30

static double RR_korrektur[4][12]={
    {31.6,33.5,26.9,18.3,12.5,10.4,10.8,10.5,12.6,15.5,21.8,26.5},
    {23.3,24.5,20.3,15.1,11.1,9.8,10.0,9.5,11.5,12.7,16.8,19.8},
    {17.3,17.9,15.5,12.7,10.1,8.8,9.1,8.5,10.2,11.0,13.3,15.0},
    {11.5,11.8,10.7,10.0,8.6,7.7,8.0,7.5,8.7,8.8,9.5,10.3}};

weather::weather(int id, int sz, int begin, int end)
{
  db* dbpw=get_dbpwth_org(); // fetch a database connection
  char buf[1024];
  MYSQL_ROW row;
  SL=3;
  klim=0;
  string str1("klim");
  int id_c;
  // select if climate or precipitation
  sprintf(buf,"select art from statlist where id=%d",id);
  dbpw->select(buf);
  if((row=dbpw->get_row())!=0){
      if(str1.compare(string(row[0]))==0)
        klim=1;
  }
  dbpw->clear_result();
  // select the nn and coordinates
  if(klim==1){  // select a climate station
      sprintf(buf,"select h, gk5_r, gk5_h from statlist  where art='klim' and id=%d;",id);
      dbpw->select(buf);
      if((row=dbpw->get_row())!=0){
          nn=atof(row[0]);
          rw=atof(row[1]);
          hw=atof(row[2]);
      }
      dat_id=id;
      cerr << id << " : nn=" << nn << " rw=" << rw << " hw=" << hw << endl;
      dbpw->clear_result();
      // fill the weather data
      sprintf(buf,"select TX,TM,TN,RR,SD,FF,RF,GS,mo from st_%d where jahr>=%d and jahr<%d and sz=%d order by jahr,mo,ta",id,begin,end,sz);
      //     cerr << buf << endl;
      dbpw->select(buf);
      while((row=dbpw->get_row())!=0){
          // cerr << row[0] << " " << row[5] << endl;
          TX.push_back(atof(row[0]));
          TM.push_back(atof(row[1]));
          TN.push_back(atof(row[2]));
          RR.push_back(atof(row[3])*(1.0+RR_korrektur[SL][atoi(row[8])-1]/100.0));
          SD.push_back(atof(row[4]));
          FF.push_back(atof(row[5]));
          RF.push_back(atof(row[6]));
          GS.push_back(atof(row[7]));
      }
      dbpw->clear_result();
  }
  if(klim==0){ // select a precipitation station and a climate station next 
      sprintf(buf,"select h, gk5_r, gk5_h from statlist  where art='Nied' and id=%d;",id);
      dbpw->select(buf);
      if((row=dbpw->get_row())!=0){
          nn=atof(row[0]);
          rw=atof(row[1]);
          hw=atof(row[2]);
      }
      dat_id=id;
      cerr << id << " : nn=" << nn << " rw=" << rw << " hw=" << hw << endl;
      dbpw->clear_result();
      // fill the weather data
      double min_dist=FLT_MAX;
      sprintf(buf,"select gk5_r, gk5_h, id from statlist  where art='klim'");
      dbpw->select(buf);
      while((row=dbpw->get_row())!=0){
          double rw_c=atof(row[0]);
          double hw_c=atof(row[1]);
          int stat_id=atoi(row[2]);
          double dist=sqrt((rw-rw_c)*(rw-rw_c)+(hw-hw_c)*(hw-hw_c));
          if(dist<min_dist){
              min_dist=dist;
              id_c=stat_id;
          }
          dbpw->clear_result();
          cerr << "climate_stat: " << id_c << " dist=" << min_dist << endl;
          sprintf(buf,"select c.TX,c.TM,c.TN,n.RR,c.SD,c.FF,c.RF,c.GS,c.mo from st_%d as c  inner join nied_%d as n on c.ta=n.ta and c.mo=n.mo and c.jahr=n.jahr and c.sz=n.sz where c.jahr>=%d and c.jahr<%d and c.sz=%d order by c.jahr,c.mo,c.ta",id_c,id,begin,end,sz);
          dbpw->select(buf);
          // cerr << buf << endl;
          while((row=dbpw->get_row())!=0){
              // cerr << row[0] << " " << row[5] << endl;
              TX.push_back(atof(row[0]));
              TM.push_back(atof(row[1]));
              TN.push_back(atof(row[2]));
              RR.push_back(atof(row[3])*(1.0+RR_korrektur[SL][atoi(row[8])-1]/100.0));
              SD.push_back(atof(row[4]));
              FF.push_back(atof(row[5]));
              RF.push_back(atof(row[6]));
              GS.push_back(atof(row[7]));
          }
          dbpw->clear_result();
      }
  }
}

weather::~weather()
{}

double weather::get_TX(int day)
{
  return TX[day];
}

double weather::get_TM(int day)
{
  double d = TM.at(day);
  return d;
}

double weather::get_TN(int day)
{
  return TN[day];
}

double weather::get_RR(int day)
{
  return RR[day];
}

double weather::get_SD(int day)
{
  return SD[day];
}

double weather::get_FF(int day)
{
  return FF[day];
}

double weather::get_RF(int day)
{
  return RF[day];
}

double weather::get_GS(int day)
{
  return GS[day];
}

double weather::dist(double hwx, double rwx)
{
  return((hwx-hw)*(hwx-hw)+(rwx-rw)*(rwx-rw));
}


// lly = 5571990
// llx = 5137900
// urx = 5137900+197500 (1975*100)
// ury = 5571990+160500 (1605*100)

interpolation::interpolation(grid* v, grid* d)
{
  initialized = false;
  voronoi=v->grid_copy();
  dgm=d->grid_copy();
  igrid=d->grid_copy();
  // make the igrid clean
  for(int i=0; i<igrid->nrows; i++)
    for(int j=0; j<igrid->ncols; j++)
      if((int)(igrid->feld[i][j]) != igrid->nodata)
        igrid->feld[i][j]=0.0;

  std::map<int,int> station_map;
  for (int i=0; i<voronoi->nrows; i++){
      for (int j=0; j<voronoi->ncols; j++) {
          int station_id = voronoi->get_xy(i,j);
          if (station_id!= voronoi->nodata) {
              station_map[station_id] = 1;
          }
      }
  }
  map<int,int>::iterator it = station_map.begin();
  for (it; it!=station_map.end(); it++) {
      if ((*it).second>0) {
          this->add_stat((*it).first,0,1991,2011);
      }
  }
  initialized = true;
}

void interpolation::add_stat(int id, int sz, int begin, int end)
{
  weather* stp;
  stp=new weather(id,sz,begin,end);
  stv.push_back(stp);
}

interpolation::~interpolation()
{
  for(int i=0; i<stv.size(); i++)
    delete stv[i];
  delete dgm;
  delete igrid;
  delete voronoi;
}

double interpolation::get_TX(int yearday,double hwx,double rwx,double nn)
{
  // calc the regression
  double varx,vary,varxy,xquer,yquer;
  double m,n,r2;
  double sum, sumz,dist;
  xquer=yquer=0.0;
  for(int i=0; i<stv.size(); i++){
      xquer+=stv[i]->nn;
      yquer+=stv[i]->get_TX(yearday);
  }
  xquer/=stv.size();
  yquer/=stv.size();
  double xdiff,ydiff;
  varx=vary=varxy=0.0;
  for(int i=0; i<stv.size(); i++){
      xdiff = stv[i]->nn-xquer;
      ydiff = stv[i]->get_TX(yearday)-yquer;
      varx += (xdiff*xdiff);
      vary += (ydiff*ydiff);
      varxy += (xdiff*ydiff);
  }
  m = varxy/varx;
  n = yquer-m*xquer;
  varx /= (stv.size()-1);
  vary /= (stv.size()-1);
  varxy /= (stv.size()-1);
  // r2 = (varxy*varxy)/(varx*vary);
  // calc inverse distance and residium
  sum=sumz=0.0;
  for(int k=0; k<stv.size(); k++){
      dist=stv[k]->dist(hwx,rwx);
      stv[k]->residium=(stv[k]->get_TX(yearday))-(m*(stv[k]->nn)+n);
      if(dist<10000.0)   // return value of weather if dist<100m
        return(stv[k]->get_TX(yearday));
      sum+=1.0/(dist);
      sumz+=(stv[k]->residium)/(dist);
  }
  return(m*nn+n+sumz/sum);
}

double interpolation::get_TM(int yearday,double hwx,double rwx,double nn)
{
  // calc the regression
  double varx,vary,varxy,xquer,yquer;
  double m,n,r2;
  double sum, sumz,dist;
  xquer=yquer=0.0;
  for(int i=0; i<stv.size(); i++){
      xquer+=stv[i]->nn;
      yquer+=stv[i]->get_TM(yearday);
  }
  xquer/=stv.size();
  yquer/=stv.size();
  double xdiff,ydiff;
  varx=vary=varxy=0.0;
  for(int i=0; i<stv.size(); i++){
      xdiff = stv[i]->nn-xquer;
      ydiff = stv[i]->get_TM(yearday)-yquer;
      varx += (xdiff*xdiff);
      vary += (ydiff*ydiff);
      varxy += (xdiff*ydiff);
  }
  m = varxy/varx;
  n = yquer-m*xquer;
  varx /= (stv.size()-1);
  vary /= (stv.size()-1);
  varxy /= (stv.size()-1);
  // r2 = (varxy*varxy)/(varx*vary);
  // calc inverse distance and residium
  sum=sumz=0.0;
  for(int k=0; k<stv.size(); k++){
      dist=stv[k]->dist(hwx,rwx);
      stv[k]->residium=(stv[k]->get_TM(yearday))-(m*(stv[k]->nn)+n);
      if(dist<10000.0)   // return value of weather if dist<100m
        return(stv[k]->get_TM(yearday));
      sum+=1.0/(dist);
      sumz+=(stv[k]->residium)/(dist);
  }
  return(m*nn+n+sumz/sum);
}

double interpolation::get_TN(int yearday,double hwx,double rwx,double nn)
{
  // calc the regression
  double varx,vary,varxy,xquer,yquer;
  double m,n,r2;
  double sum, sumz,dist;
  xquer=yquer=0.0;
  for(int i=0; i<stv.size(); i++){
      xquer+=stv[i]->nn;
      yquer+=stv[i]->get_TN(yearday);
  }
  xquer/=stv.size();
  yquer/=stv.size();
  double xdiff,ydiff;
  varx=vary=varxy=0.0;
  for(int i=0; i<stv.size(); i++){
      xdiff = stv[i]->nn-xquer;
      ydiff = stv[i]->get_TN(yearday)-yquer;
      varx += (xdiff*xdiff);
      vary += (ydiff*ydiff);
      varxy += (xdiff*ydiff);
  }
  m = varxy/varx;
  n = yquer-m*xquer;
  varx /= (stv.size()-1);
  vary /= (stv.size()-1);
  varxy /= (stv.size()-1);
  // r2 = (varxy*varxy)/(varx*vary);
  // calc inverse distance and residium
  sum=sumz=0.0;
  for(int k=0; k<stv.size(); k++){
      dist=stv[k]->dist(hwx,rwx);
      stv[k]->residium=(stv[k]->get_TN(yearday))-(m*(stv[k]->nn)+n);
      if(dist<10000.0)   // return value of weather if dist<100m
        return(stv[k]->get_TN(yearday));
      sum+=1.0/(dist);
      sumz+=(stv[k]->residium)/(dist);
  }
  return(m*nn+n+sumz/sum);
}

double interpolation::get_RR(int yearday,double hwx,double rwx,double nn)
{
  // calc the regression
  double varx,vary,varxy,xquer,yquer;
  double m,n,r2;
  double sum, sumz,dist;
  xquer=yquer=0.0;
  for(int i=0; i<stv.size(); i++){
      xquer+=stv[i]->nn;
      yquer+=stv[i]->get_RR(yearday);
  }
  if(yquer<=0.0)
    return 0.0;
  xquer/=stv.size();
  yquer/=stv.size();
  double xdiff,ydiff;
  varx=vary=varxy=0.0;
  for(int i=0; i<stv.size(); i++){
      xdiff = stv[i]->nn-xquer;
      ydiff = stv[i]->get_RR(yearday)-yquer;
      varx += (xdiff*xdiff);
      vary += (ydiff*ydiff);
      varxy += (xdiff*ydiff);
  }
  m = varxy/varx;
  n = yquer-m*xquer;
  varx /= (stv.size()-1);
  vary /= (stv.size()-1);
  varxy /= (stv.size()-1);
  // r2 = (varxy*varxy)/(varx*vary);
  // cerr << "r2=" << r2 << endl;
  // return(r2);
  // calc inverse distance and residium
  sum=sumz=0.0;
  for(int k=0; k<stv.size(); k++){
      dist=stv[k]->dist(hwx,rwx);
      stv[k]->residium=(stv[k]->get_RR(yearday))-(m*(stv[k]->nn)+n);
      if(dist<10000.0){   // return value of weather if dist<100m
          // cerr << k << " " << dist << " " << hwx << " " << stv[k]->hw <<
          //        " " << rwx << " " << stv[k]->rw << endl;
          return(stv[k]->get_RR(yearday));
      }
      sum+=1.0/(dist);
      sumz+=(stv[k]->residium)/(dist);
  }
  return(m*nn+n+sumz/sum);
}

double interpolation::get_SD(int yearday,double hwx,double rwx,double nn)
{
  // calc the regression
  double varx,vary,varxy,xquer,yquer;
  double m,n,r2;
  double sum, sumz,dist;
  xquer=yquer=0.0;
  for(int i=0; i<stv.size(); i++){
      xquer+=stv[i]->nn;
      yquer+=stv[i]->get_SD(yearday);
  }
  if(yquer<=0.0)
    return 0.0;
  xquer/=stv.size();
  yquer/=stv.size();
  double xdiff,ydiff;
  varx=vary=varxy=0.0;
  for(int i=0; i<stv.size(); i++){
      xdiff = stv[i]->nn-xquer;
      ydiff = stv[i]->get_SD(yearday)-yquer;
      varx += (xdiff*xdiff);
      vary += (ydiff*ydiff);
      varxy += (xdiff*ydiff);
  }
  m = varxy/varx;
  n = yquer-m*xquer;
  varx /= (stv.size()-1);
  vary /= (stv.size()-1);
  varxy /= (stv.size()-1);
  // r2 = (varxy*varxy)/(varx*vary);
  // cerr << "r2=" << r2 << endl;
  // return(r2);
  // calc inverse distance and residium
  sum=sumz=0.0;
  for(int k=0; k<stv.size(); k++){
      dist=stv[k]->dist(hwx,rwx);
      stv[k]->residium=(stv[k]->get_SD(yearday))-(m*(stv[k]->nn)+n);
      if(dist<10000.0){   // return value of weather if dist<100m
          // cerr << k << " " << dist << " " << hwx << " " << stv[k]->hw <<
          //        " " << rwx << " " << stv[k]->rw << endl;
          return(stv[k]->get_SD(yearday));
      }
      sum+=1.0/(dist);
      sumz+=(stv[k]->residium)/(dist);
  }
  return(m*nn+n+sumz/sum);
}

double interpolation::get_FF(int yearday,double hwx,double rwx,double nn)
{
  // calc the regression
  double varx,vary,varxy,xquer,yquer;
  double m,n,r2;
  double sum, sumz,dist;
  xquer=yquer=0.0;
  for(int i=0; i<stv.size(); i++){
      xquer+=stv[i]->nn;
      yquer+=stv[i]->get_FF(yearday);
  }
  if(yquer<=0.0)
    return 0.0;
  xquer/=stv.size();
  yquer/=stv.size();
  double xdiff,ydiff;
  varx=vary=varxy=0.0;
  for(int i=0; i<stv.size(); i++){
      xdiff = stv[i]->nn-xquer;
      ydiff = stv[i]->get_FF(yearday)-yquer;
      varx += (xdiff*xdiff);
      vary += (ydiff*ydiff);
      varxy += (xdiff*ydiff);
  }
  m = varxy/varx;
  n = yquer-m*xquer;
  varx /= (stv.size()-1);
  vary /= (stv.size()-1);
  varxy /= (stv.size()-1);
  // r2 = (varxy*varxy)/(varx*vary);
  // cerr << "r2=" << r2 << endl;
  // return(r2);
  // calc inverse distance and residium
  sum=sumz=0.0;
  for(int k=0; k<stv.size(); k++){
      dist=stv[k]->dist(hwx,rwx);
      stv[k]->residium=(stv[k]->get_FF(yearday))-(m*(stv[k]->nn)+n);
      if(dist<10000.0){   // return value of weather if dist<100m
          // cerr << k << " " << dist << " " << hwx << " " << stv[k]->hw <<
          //        " " << rwx << " " << stv[k]->rw << endl;
          return(stv[k]->get_FF(yearday));
      }
      sum+=1.0/(dist);
      sumz+=(stv[k]->residium)/(dist);
  }
  return(m*nn+n+sumz/sum);
}

double interpolation::get_RF(int yearday,double hwx,double rwx,double nn)
{
  // calc the regression
  double varx,vary,varxy,xquer,yquer;
  double m,n,r2;
  double sum, sumz,dist;
  xquer=yquer=0.0;
  for(int i=0; i<stv.size(); i++){
      xquer+=stv[i]->nn;
      yquer+=stv[i]->get_RF(yearday);
  }
  if(yquer<=0.0)
    return 0.0;
  xquer/=stv.size();
  yquer/=stv.size();
  double xdiff,ydiff;
  varx=vary=varxy=0.0;
  for(int i=0; i<stv.size(); i++){
      xdiff = stv[i]->nn-xquer;
      ydiff = stv[i]->get_RF(yearday)-yquer;
      varx += (xdiff*xdiff);
      vary += (ydiff*ydiff);
      varxy += (xdiff*ydiff);
  }
  m = varxy/varx;
  n = yquer-m*xquer;
  varx /= (stv.size()-1);
  vary /= (stv.size()-1);
  varxy /= (stv.size()-1);
  // r2 = (varxy*varxy)/(varx*vary);
  // cerr << "r2=" << r2 << endl;
  // return(r2);
  // calc inverse distance and residium
  sum=sumz=0.0;
  for(int k=0; k<stv.size(); k++){
      dist=stv[k]->dist(hwx,rwx);
      stv[k]->residium=(stv[k]->get_RF(yearday))-(m*(stv[k]->nn)+n);
      if(dist<10000.0){   // return value of weather if dist<100m
          // cerr << k << " " << dist << " " << hwx << " " << stv[k]->hw <<
          //        " " << rwx << " " << stv[k]->rw << endl;
          return(stv[k]->get_RF(yearday));
      }
      sum+=1.0/(dist);
      sumz+=(stv[k]->residium)/(dist);
  }
  return(m*nn+n+sumz/sum);
}

double interpolation::get_GS(int yearday,double hwx,double rwx,double nn)
{
  // calc the regression
  double varx,vary,varxy,xquer,yquer;
  double m,n,r2;
  double sum, sumz,dist;
  xquer=yquer=0.0;
  for(int i=0; i<stv.size(); i++){
      xquer+=stv[i]->nn;
      yquer+=stv[i]->get_GS(yearday);
  }
  xquer/=stv.size();
  yquer/=stv.size();
  double xdiff,ydiff;
  varx=vary=varxy=0.0;
  for(int i=0; i<stv.size(); i++){
      xdiff = stv[i]->nn-xquer;
      ydiff = stv[i]->get_GS(yearday)-yquer;
      varx += (xdiff*xdiff);
      vary += (ydiff*ydiff);
      varxy += (xdiff*ydiff);
  }
  m = varxy/varx;
  n = yquer-m*xquer;
  varx /= (stv.size()-1);
  vary /= (stv.size()-1);
  varxy /= (stv.size()-1);
  // r2 = (varxy*varxy)/(varx*vary);
  // calc inverse distance and residium
  sum=sumz=0.0;
  for(int k=0; k<stv.size(); k++){
      dist=stv[k]->dist(hwx,rwx);
      stv[k]->residium=(stv[k]->get_GS(yearday))-(m*(stv[k]->nn)+n);
      if(dist<10000.0)   // return value of weather if dist<100m
        return(stv[k]->get_GS(yearday));
      sum+=1.0/(dist);
      sumz+=(stv[k]->residium)/(dist);
  }
  return(m*nn+n+sumz/sum);
}

void interpolation::map_RR(int yearday)
{
  // commulative function
  double y=igrid->csize*(igrid->nrows-1)+0.5*igrid->csize+igrid->ycorner;
  double dy=igrid->csize;
  double x=igrid->xcorner+0.5*igrid->csize;
  double dx=igrid->csize;
  for(int i=0; i<igrid->nrows; i++)
    for(int j=0; j<igrid->ncols; j++)
      if((int)(igrid->feld[i][j])!=igrid->nodata)
        igrid->feld[i][j]+=get_RR(yearday,
            y-i*dy,x+j*dx,dgm->feld[i][j]);
}

void interpolation::map_TM(int yearday)
{
  // commulative function
  double y=igrid->csize*(igrid->nrows-1)+0.5*igrid->csize+igrid->ycorner;
  double dy=igrid->csize;
  double x=igrid->xcorner+0.5*igrid->csize;
  double dx=igrid->csize;
  for(int i=0; i<igrid->nrows; i++)
    for(int j=0; j<igrid->ncols; j++)
      if((int)(igrid->feld[i][j])!=igrid->nodata)
        igrid->feld[i][j]+=get_TM(yearday,
            y-i*dy,x+j*dx,dgm->feld[i][j]);
}

void interpolation::map_TN(int yearday)
{
  // commulative function
  double y=igrid->csize*(igrid->nrows-1)+0.5*igrid->csize+igrid->ycorner;
  double dy=igrid->csize;
  double x=igrid->xcorner+0.5*igrid->csize;
  double dx=igrid->csize;
  for(int i=0; i<igrid->nrows; i++)
    for(int j=0; j<igrid->ncols; j++)
      if((int)(igrid->feld[i][j])!=igrid->nodata)
        igrid->feld[i][j]+=get_TM(yearday,
            y-i*dy,x+j*dx,dgm->feld[i][j]);
}

void interpolation::map_TX(int yearday)
{
  // commulative function
  double y=igrid->csize*(igrid->nrows-1)+0.5*igrid->csize+igrid->ycorner;
  double dy=igrid->csize;
  double x=igrid->xcorner+0.5*igrid->csize;
  double dx=igrid->csize;
  for(int i=0; i<igrid->nrows; i++)
    for(int j=0; j<igrid->ncols; j++)
      if((int)(igrid->feld[i][j])!=igrid->nodata)
        igrid->feld[i][j]+=get_TM(yearday,
            y-i*dy,x+j*dx,dgm->feld[i][j]);
}

void interpolation::map_GS(int yearday)
{
  // commulative function
  double y=igrid->csize*(igrid->nrows-1)+0.5*igrid->csize+igrid->ycorner;
  double dy=igrid->csize;
  double x=igrid->xcorner+0.5*igrid->csize;
  double dx=igrid->csize;
  for(int i=0; i<igrid->nrows; i++)
    for(int j=0; j<igrid->ncols; j++)
      if((int)(igrid->feld[i][j])!=igrid->nodata)
        igrid->feld[i][j]+=get_GS(yearday,
            y-i*dy,x+j*dx,dgm->feld[i][j]);
}

void interpolation::map_SD(int yearday)
{
  // commulative function
  double y=igrid->csize*(igrid->nrows-1)+0.5*igrid->csize+igrid->ycorner;
  double dy=igrid->csize;
  double x=igrid->xcorner+0.5*igrid->csize;
  double dx=igrid->csize;
  for(int i=0; i<igrid->nrows; i++)
    for(int j=0; j<igrid->ncols; j++)
      if((int)(igrid->feld[i][j])!=igrid->nodata)
        igrid->feld[i][j]+=get_SD(yearday,
            y-i*dy,x+j*dx,dgm->feld[i][j]);
}

void interpolation::map_FF(int yearday)
{
  // commulative function
  double y=igrid->csize*(igrid->nrows-1)+0.5*igrid->csize+igrid->ycorner;
  double dy=igrid->csize;
  double x=igrid->xcorner+0.5*igrid->csize;
  double dx=igrid->csize;
  for(int i=0; i<igrid->nrows; i++)
    for(int j=0; j<igrid->ncols; j++)
      if((int)(igrid->feld[i][j])!=igrid->nodata)
        igrid->feld[i][j]+=get_FF(yearday,
            y-i*dy,x+j*dx,dgm->feld[i][j]);
}


