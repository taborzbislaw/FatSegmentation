
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <queue>
#include <iostream>
#include <vector>
#include <time.h>
#include "CImg.h"


#define BORDER 100
#define FOREGROUND 200
#define BACKGROUND 50

#define DMIN -20000
#define FAT_LOW -191
#define FAT_HIGH -31
#define SKIN_TH 10
#define DMAX ULONG_MAX/100

using namespace std;
using namespace cimg_library;

class Punkt
{
public:
  int x;
  int y;
  int kolor;

  Punkt(){}
  Punkt(int X,int Y,int C) {x=X;y=Y;kolor=C;}
  Punkt(int X,int Y) {x=X;y=Y;}
};

bool operator< (const Punkt& p1, const Punkt &p2)
{
	return p1.kolor > p2.kolor;	
}

bool operator> (const Punkt& p1, const Punkt &p2)
{
	return p1.kolor < p2.kolor;	
}

int znajdzOdleglosci(CImg<signed short> &distances);
vector<Punkt> znajdzKontur(CImg<signed short> &distances,int IC,int JC,int dist);
vector<Punkt> FindShortestPath(CImg<unsigned long> &dane,Punkt S,Punkt E);
vector<Punkt> FindShortestPath(CImg<unsigned long> &dane,Punkt S,Punkt E,int flag);
vector<Punkt> FindShortestPath(CImg<unsigned long> &dane,Punkt S,Punkt E,Punkt C,int flag,double TH);
void Fill(CImg<unsigned char> &dum,Punkt p);

int main( int argc, char** argv )
{
	clock_t start, end;
    double cpu_time_used;
	
	char name[100];
	int Ni=512,Nj=512;
	double TH;

	if (argc<5)
	{
		cerr << "Usage " << argv[0] << " input_file additive_factor exponent threshold" << endl;
		exit(0);
	}

	CImg<signed short> dane(Ni,Nj);
	dane.load_raw(argv[1],Ni,Nj,1,1,false,false);

//    dane.display();

start = clock();

	int max = DMIN;
	for(int j=0;j<Nj;j++)
	{
		if (dane(0,j) > max) max= dane(0,j);
		if (dane(Ni-1,j) > max) max= dane(Ni-1,j);
	}
	if (max>DMIN)
	{
//		cerr << "Object touches image edges!!!" << endl;
//		exit(0);
		for(int j=0;j<Nj;j++)
		{
			dane(0,j) = DMIN;
			dane(1,j) = DMIN;
			dane(Ni-2,j) = DMIN;
			dane(Ni-1,j) = DMIN;
		}
	}


	CImg<unsigned char> bin(Ni,Nj);
	bin.fill(0);
	for(int i=0;i<Ni;i++)
	for(int j=0;j<Nj;j++)
	{
		if (dane(i,j)>DMIN)
		{
			if (dane(i,j)>=FAT_LOW && dane(i,j) <=FAT_HIGH) bin(i,j) = 255;
		}
	}
//	bin.erode(3);
//	bin.dilate(3);

//    bin.display("bin");

	CImg<signed short> distances(Ni,Nj);
	for(int i=0;i<Ni;i++)
	for(int j=0;j<Nj;j++)
	{
		if (dane(i,j)<=DMIN)
			distances(i,j) = 0;
		else
			distances(i,j) = -1;
	}
//    distances.display("pre");
	int HEIGHT = znajdzOdleglosci(distances);

//    distances.display("post");

	float IC = 0, JC = 0, VOL = 0;
	for(int i=0;i<Ni;i++)
	for(int j=0;j<Nj;j++)
	{
		if (dane(i,j)>DMIN)
		{
			IC += i;
			JC += j;
			VOL += 1.0;
		}
	}
	IC /= VOL;
	JC /= VOL;



	vector< vector<Punkt> > kontury;
	int dist = 1;
	vector<Punkt> kontur = znajdzKontur(distances,IC,JC,dist);
	int WIDTH = kontur.size();
	kontury.push_back(kontur);

//	cerr << IC << " " << JC << " " << WIDTH << " " << HEIGHT << endl;

#define MIN_KONTUR_SIZE 20
#define PADDING 10
#define SKIN_TH 10

	for(dist = 2;dist<HEIGHT-PADDING;dist++)
	{
		kontur = znajdzKontur(distances,IC,JC,dist);
		kontury.push_back(kontur);
		if (kontur.size()<MIN_KONTUR_SIZE) break;
	}

	CImg<unsigned long> graf(Ni,Nj);
	graf.fill(DMAX);
	for(int i=0;i<Ni;i++)
	for(int j=0;j<Nj;j++)
	{
		if (distances(i,j)==0) continue;
		if (distances(i,j)<SKIN_TH || distances(i,j) >=HEIGHT-PADDING)
		{
			graf(i,j) = DMAX;
		}
		else
		{
			double factor = (double)kontury[0].size()/(double)kontury[distances(i,j)-1].size();
			if (bin(i,j)==0)
			{
				graf(i,j) = factor*pow(distances(i,j),atof(argv[3]));
			}
			else
			{
				graf(i,j) = factor*pow(distances(i,j),atof(argv[3])) + atoi(argv[2]);
			}
		}
	}

	Punkt S1,E1;
	for(int i=0;i<IC;i++)
	{
		if (graf(i,JC)<10000 && bin(i,JC)==0)
		{
			S1.x = i;
			S1.y = JC;
			break;
		}
	}
	for(int i=Ni-1;i>=IC;i--)
	{
		if (graf(i,JC)<10000 && bin(i,JC)==0)
		{
			E1.x = i;
			E1.y = JC;
			break;
		}
	}
//	vector<Punkt> kontur1 = FindShortestPath(graf,S1,E1,1);
//	vector<Punkt> kontur2 = FindShortestPath(graf,S1,E1,-1);
	vector<Punkt> kontur1 = FindShortestPath(graf,S1,E1,Punkt(IC,JC),1,atof(argv[4]));
	vector<Punkt> kontur2 = FindShortestPath(graf,S1,E1,Punkt(IC,JC),-1,atof(argv[4]));
	kontur1.insert(kontur1.end(),kontur2.begin(),kontur2.end());

	CImg<unsigned char> dum(Ni,Nj);
	dum.fill(0);

	for(unsigned int i=0;i<kontur1.size();i++)
		dum(kontur1[i].x,kontur1[i].y)=128;

	Fill(dum,Punkt(IC,JC,128));
//	sprintf(name,"interior_%s.bmp",argv[1]);
//	dum.save_bmp(name);
//	dum.display();

	for(int i=0;i<Ni;i++)
	for(int j=0;j<Nj;j++)
	{
		if (dum(i,j))
		{
			if (bin(i,j)) bin(i,j) = 128;
			else bin(i,j) = 64;
		}
	}

	queue<Punkt> kolejka;
	for(int i=0;i<Ni;i++)
	{
		if (bin(i,0)==0) 
		{
			kolejka.push(Punkt(i,0,0));
			bin(i,0) = 1;
		}
		if (bin(i,Nj-1)==0)
		{
			kolejka.push(Punkt(i,Nj-1,0));
			bin(i,Nj-1)=1;
		}
	}
	for(int j=0;j<Nj;j++)
	{
		if (bin(0,j)==0)
		{
			kolejka.push(Punkt(0,j,0));
			bin(0,j) = 1;
		}
		if (bin(Ni-1,j)==0)
		{
			kolejka.push(Punkt(Ni-1,j,0));
			bin(Ni-1,j) = 1;
		}
	}

	while (kolejka.size())
	{
		Punkt punkt = kolejka.front();
		kolejka.pop();
		int I = punkt.x;
		int J = punkt.y;
		int C = punkt.kolor;
		for(int i1=-1;i1<=1;i1++)
		for(int j1=-1;j1<=1;j1++)
		{
			if (abs(i1)+abs(j1)>=1 && I+i1>=0 && I+i1<Ni && J+j1>=0 && J+j1<Nj)
			{
				if (bin(I+i1,J+j1)==0)
				{
					bin(I+i1,J+j1) = 1;
					kolejka.push(Punkt(I+i1,J+j1,1));
				}
			}
		}
	}

	for(int i=0;i<Ni;i++)
	for(int j=0;j<Nj;j++)
		if (bin(i,j)==0) bin(i,j) = 64;

	for(int i=0;i<Ni;i++)
	for(int j=0;j<Nj;j++)
		if (bin(i,j)==1) bin(i,j) = 0;

	for(int i=0;i<Ni;i++)
	for(int j=0;j<Nj;j++)
	{
		if (distances(i,j) >=SKIN_TH/2 && bin(i,j)==0) bin(i,j) = 64;
		if (bin(i,j) == 64 || bin(i,j) ==128) dum(i,j) = 128;
	}

	kolejka.push(Punkt(IC,JC,64));
	dum(IC,JC) = 64;
	while (kolejka.size())
	{
		Punkt punkt = kolejka.front();
		kolejka.pop();
		int I = punkt.x;
		int J = punkt.y;
		int C = punkt.kolor;
		for(int i1=-1;i1<=1;i1++)
		for(int j1=-1;j1<=1;j1++)
		{
			if (abs(i1)+abs(j1)==1 && I+i1>=0 && I+i1<Ni && J+j1>=0 && J+j1<Nj)
			{
				if (dum(I+i1,J+j1)!=64 && bin(I+i1,J+j1)!=255)
				{
					dum(I+i1,J+j1) = 64;
					kolejka.push(Punkt(I+i1,J+j1,64));
				}
			}
		}
	}

	for(int i=0;i<Ni;i++)
	for(int j=0;j<Nj;j++)
	{
		if (dum(i,j)==128 && bin(i,j)==64) bin(i,j)=0;
	}


//	for(unsigned int i=0;i<kontur1.size();i++)
//		bin(kontur1[i].x,kontur1[i].y)=0;

	bin.erode(2);
	bin.dilate(2);

//	bin.display();

     end = clock();
     cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

	cerr << cpu_time_used << endl;

	sprintf(name,"skinFat_%s.bmp",argv[1]);
	bin.save_bmp(name);


	return 0;
}

/***********************************************************************************/
/***********************************************************************************/
/***********************************************************************************/

void Fill(CImg<unsigned char> &dum,Punkt p)
{
	int Ni = dum.width();
	int Nj = dum.height();

	queue<Punkt> kolejka;
	kolejka.push(p);

	dum(p.x,p.y) = p.kolor;

	while (kolejka.size())
	{
		Punkt punkt = kolejka.front();
		kolejka.pop();
		int I = punkt.x;
		int J = punkt.y;
		int C = punkt.kolor;
		for(int i1=-1;i1<=1;i1++)
		for(int j1=-1;j1<=1;j1++)
		{
			if (abs(i1)+abs(j1)==1 && I+i1>=0 && I+i1<Ni && J+j1>=0 && J+j1<Nj)
			{
				if (dum(I+i1,J+j1)==0)
				{
					dum(I+i1,J+j1) = p.kolor;
					kolejka.push(Punkt(I+i1,J+j1,p.kolor));
				}
			}
		}
	}

}

/***********************************************************************************/
/***********************************************************************************/
/***********************************************************************************/

int znajdzOdleglosci(CImg<signed short> &distances)
{
	int Ni = distances.width();
	int Nj = distances.height();

	int borderColor = 0;
	int licz = 0;
	do
	{
		licz = 0;
		for(int i=0;i<Ni;i++)
		for(int j=0;j<Nj;j++)
		{
			if (distances(i,j)>=0) continue;
			int flag = 0;
			for(int i1=-1;i1<=1;i1++)
			for(int j1=-1;j1<=1;j1++)
			{
				if (abs(i1)+abs(j1)>=1 && i+i1>=0 && i+i1<Ni && j+j1>=0 && j+j1<Nj)
				{
					if (distances(i+i1,j+j1)==borderColor) flag = 1;
				}
			}
			if (flag)
			{
				licz++;
				distances(i,j) = borderColor+1;
			}
		}
		borderColor++;
	} while (licz!=0);

	return borderColor;
}

/***********************************************************************************/
/***********************************************************************************/
/***********************************************************************************/

vector<Punkt> znajdzKontur(CImg<signed short> &distances,int IC,int JC,int dist)
{
	int Ni = distances.width();
	int Nj = distances.height();
	vector<Punkt> kontur;

#define NOT_YET_USED -120
	queue<Punkt> kolejka;
	CImg<signed char> way(Ni,Nj);
	way.fill(NOT_YET_USED);
	for(int i=0;i<IC;i++)
	{
		if (distances(i,JC)==dist)
		{
			kolejka.push(Punkt(i,JC,dist));
			way(i,JC) = 0;
			break;
		}
	}
	int I;
	int J;
	int C;
	while (kolejka.size())
	{
		Punkt punkt = kolejka.front();
		kolejka.pop();
		kontur.push_back(punkt);
		I = punkt.x;
		J = punkt.y;
		C = punkt.kolor;
		for(int i1=-1;i1<=1;i1++)
		for(int j1=-1;j1<=1;j1++)
		{
			if (abs(i1)+abs(j1)>=1 && I+i1>=0 && I+i1<Ni && J+j1>=0 && J+j1<Nj)
			{
				if (distances(I+i1,J+j1)==dist && way(I+i1,J+j1)==NOT_YET_USED && J+j1<=JC)
				{
					kolejka.push(Punkt(I+i1,J+j1,dist));
					way(I+i1,J+j1) = 10*i1 + j1;
				}
			}
		}
	}

	kolejka.push(Punkt(I,J,dist));
	int flag = 0;
	while (kolejka.size())
	{
		Punkt punkt = kolejka.front();
		kolejka.pop();
		if (flag) kontur.push_back(punkt);
		flag = 1;
		I = punkt.x;
		J = punkt.y;
		C = punkt.kolor;
		for(int i1=-1;i1<=1;i1++)
		for(int j1=-1;j1<=1;j1++)
		{
			if (abs(i1)+abs(j1)>=1 && I+i1>=0 && I+i1<Ni && J+j1>=0 && J+j1<Nj)
			{
				if (distances(I+i1,J+j1)==dist && way(I+i1,J+j1)==NOT_YET_USED && J+j1>JC)
				{
					kolejka.push(Punkt(I+i1,J+j1,dist));
					way(I+i1,J+j1) = 10*i1 + j1;
				}
			}
		}
	}

//	cerr << kontur[0].x << " " << kontur[0].y << endl;
//	cerr << kontur[kontur.size()-1].x << " " << kontur[kontur.size()-1].y << endl;

	return kontur;
}

/***********************************************************************************/
/***********************************************************************************/
/***********************************************************************************/

vector<Punkt> znajdzKontur1(CImg<signed short> &distances,int IC,int JC,int dist)
{
	int Ni = distances.width();
	int Nj = distances.height();

#define NOT_YET_USED -120
	queue<Punkt> kolejka;
	CImg<signed char> way(Ni,Nj);
	way.fill(NOT_YET_USED);
	for(int i=0;i<IC;i++)
	{
		if (distances(i,JC)==dist)
		{
			kolejka.push(Punkt(i,JC,dist));
			way(i,JC) = 0;
			break;
		}
	}
	int I;
	int J;
	int C;
	while (kolejka.size())
	{
		Punkt punkt = kolejka.front();
		kolejka.pop();
		I = punkt.x;
		J = punkt.y;
		C = punkt.kolor;
		if (I>IC && J==JC) break;
		for(int i1=-1;i1<=1;i1++)
		for(int j1=-1;j1<=1;j1++)
		{
			if (abs(i1)+abs(j1)>=1 && I+i1>=0 && I+i1<Ni && J+j1>=0 && J+j1<Nj)
			{
				if (distances(I+i1,J+j1)==dist && way(I+i1,J+j1)==NOT_YET_USED && J+j1<=JC)
				{
					kolejka.push(Punkt(I+i1,J+j1,dist));
					way(I+i1,J+j1) = 10*i1 + j1;
				}
			}
		}
	}
	while (kolejka.size()) kolejka.pop();

	int IB = I;
	int JB = J;
	vector<Punkt> kontur;
	kontur.push_back(Punkt(I,J,dist));
	while(way(I,J)!=0)
	{
		if (way(I,J)==10) I = I-1;
		else if (way(I,J)==-10) I = I+1;
		else if (way(I,J)==1) J = J-1;
		else if (way(I,J)==-1) J = J+1;
		else if (way(I,J)==11) {I = I-1;J = J-1;}
		else if (way(I,J)==9) {I = I-1;J = J+1;}
		else if (way(I,J)==-11)  {I = I+1;J = J+1;}
		else if (way(I,J)==-9)  {I = I+1;J = J-1;}
		kontur.push_back(Punkt(I,J,dist));
	}

	way.fill(NOT_YET_USED);
	kolejka.push(Punkt(IB,JB,dist));
	way(IB,JB) = 0;
	while (kolejka.size())
	{
		Punkt punkt = kolejka.front();
		kolejka.pop();
		I = punkt.x;
		J = punkt.y;
		C = punkt.kolor;
		if (I<IC && J==JC) break;
		for(int i1=-1;i1<=1;i1++)
		for(int j1=-1;j1<=1;j1++)
		{
			if (abs(i1)+abs(j1)>=1 && I+i1>=0 && I+i1<Ni && J+j1>=0 && J+j1<Nj)
			{
				if (distances(I+i1,J+j1)==dist && way(I+i1,J+j1)==NOT_YET_USED && J+j1>=JC)
				{
					kolejka.push(Punkt(I+i1,J+j1,dist));
					way(I+i1,J+j1) = 10*i1 + j1;
				}
			}
		}
	}
	while (kolejka.size()) kolejka.pop();

	while(way(I,J)!=0)
	{
		if (way(I,J)==10) I = I-1;
		else if (way(I,J)==-10) I = I+1;
		else if (way(I,J)==1) J = J-1;
		else if (way(I,J)==-1) J = J+1;
		else if (way(I,J)==11) {I = I-1;J = J-1;}
		else if (way(I,J)==9) {I = I-1;J = J+1;}
		else if (way(I,J)==-11)  {I = I+1;J = J+1;}
		else if (way(I,J)==-9)  {I = I+1;J = J-1;}
		kontur.push_back(Punkt(I,J,dist));
	}
	kontur.erase(kontur.end());

//	cerr << kontur[0].x << " " << kontur[0].y << endl;
//	cerr << kontur[kontur.size()-1].x << " " << kontur[kontur.size()-1].y << endl;

	return kontur;
}

/***********************************************************************************/
/***********************************************************************************/
/***********************************************************************************/

std::vector<Punkt> FindShortestPath(CImg<unsigned long> &dane,Punkt S,Punkt E)
{

	CImg<signed char> way(dane.width(),dane.height());
	CImg<unsigned long> dist(dane.width(),dane.height());
	
	dist.fill(ULONG_MAX);
	way.fill(-1);
	
	dist(S.x,S.y) = dane(S.x,S.y);
	way(S.x,S.y) = 0;
	
	priority_queue<Punkt,std::vector<Punkt>,less<Punkt> > kolejka;
	S.kolor = 0;
	kolejka.push(S);
	Punkt punkt;
	
	int I=0,J=0;
	while(kolejka.size())
	{
		punkt = kolejka.top();
		kolejka.pop();
		I = punkt.x;
		J = punkt.y;
		if (I==E.x && J==E.y) break;
		for(int i1=-1;i1<=1;i1++)
		for(int j1=-1;j1<=1;j1++)
		{
			if (abs(i1)+abs(j1)==1 && I+i1>=0 && J+j1>=0 && I+i1<dane.width() && J+j1<dane.height())
			{
				if (dist(I+i1,J+j1)>dane(I+i1,J+j1)+dist(I,J))
				{
					punkt.x = I+i1;
					punkt.y = J+j1;
					dist(I+i1,J+j1) = dane(I+i1,J+j1) + dist(I,J);
					punkt.kolor = dist(I+i1,J+j1);
					kolejka.push(punkt);
					way(I+i1,J+j1) = 10*i1 + j1;
				}
			}
		}
	}

	std::vector<Punkt> lista;
	lista.clear();
	punkt.x = I;
	punkt.y = J;
	lista.push_back(punkt);
	
	while(way(I,J)!=0)
	{
		if (way(I,J)==10) I = I-1;
		else if (way(I,J)==-10) I = I+1;
		else if (way(I,J)==1) J = J-1;
		else J = J+1;
		punkt.x = I;
		punkt.y = J;
		lista.push_back(punkt);
	}

	return lista;
}

std::vector<Punkt> FindShortestPath(CImg<unsigned long> &dane,Punkt S,Punkt E,int flag)
{

	CImg<signed char> way(dane.width(),dane.height());
	CImg<unsigned long> dist(dane.width(),dane.height());
	CImg<signed long> dum(dane.width(),dane.height());

	dum = dane;
	if (flag==1)
	{
		for(int i=0;i<dane.width();i++)
		for(int j=0;j<S.y;j++)
			dum(i,j) = DMAX;
	}
	else if (flag==-1)
	{
		for(int i=0;i<dane.width();i++)
		for(int j=S.y+1;j<dane.height();j++)
			dum(i,j) = DMAX;
	}
	
	dist.fill(DMAX);
	way.fill(-1);
	
	dist(S.x,S.y) = dum(S.x,S.y);
	way(S.x,S.y) = 0;
	
	priority_queue<Punkt,std::vector<Punkt>,less<Punkt> > kolejka;
	S.kolor = 0;
	kolejka.push(S);
	Punkt punkt;
	
	int I=0,J=0;
	while(kolejka.size())
	{
		punkt = kolejka.top();
		kolejka.pop();
		I = punkt.x;
		J = punkt.y;
		if (I==E.x && J==E.y) break;
		for(int i1=-1;i1<=1;i1++)
		for(int j1=-1;j1<=1;j1++)
		{
			if (abs(i1)+abs(j1)==1 && I+i1>=0 && J+j1>=0 && I+i1<dane.width() && J+j1<dane.height())
			{
				if (dist(I+i1,J+j1)>dum(I+i1,J+j1)+dist(I,J))
				{
					punkt.x = I+i1;
					punkt.y = J+j1;
					dist(I+i1,J+j1) = dum(I+i1,J+j1) + dist(I,J);
					punkt.kolor = dist(I+i1,J+j1);
					kolejka.push(punkt);
					way(I+i1,J+j1) = 10*i1 + j1;
				}
			}
		}
	}

	std::vector<Punkt> lista;
	lista.clear();
	punkt.x = I;
	punkt.y = J;
	lista.push_back(punkt);
	
	while(way(I,J)!=0)
	{
		if (way(I,J)==10) I = I-1;
		else if (way(I,J)==-10) I = I+1;
		else if (way(I,J)==1) J = J-1;
		else J = J+1;
		punkt.x = I;
		punkt.y = J;
		lista.push_back(punkt);
	}

	return lista;
}

std::vector<Punkt> FindShortestPath(CImg<unsigned long> &dane,Punkt S,Punkt E,Punkt C,int flag,double TH)
{

	CImg<signed char> way(dane.width(),dane.height());
	CImg<unsigned long> dist(dane.width(),dane.height());
	CImg<signed long> dum(dane.width(),dane.height());

	double IC = C.x;
	double JC = C.y;

	dum = dane;
	if (flag==1)
	{
		for(int i=0;i<dane.width();i++)
		for(int j=0;j<S.y;j++)
			dum(i,j) = DMAX;
	}
	else if (flag==-1)
	{
		for(int i=0;i<dane.width();i++)
		for(int j=S.y+1;j<dane.height();j++)
			dum(i,j) = DMAX;
	}
	
	dist.fill(DMAX);
	way.fill(-1);
	
	dist(S.x,S.y) = dum(S.x,S.y);
	way(S.x,S.y) = 0;
	
	priority_queue<Punkt,std::vector<Punkt>,less<Punkt> > kolejka;
	S.kolor = 0;
	kolejka.push(S);
	Punkt punkt;

	CImgDisplay disp;
	
	int I=0,J=0;
	while(kolejka.size())
	{
		punkt = kolejka.top();
		kolejka.pop();
		I = punkt.x;
		J = punkt.y;
		if (I==E.x && J==E.y) break;
		for(int i1=-1;i1<=1;i1++)
		for(int j1=-1;j1<=1;j1++)
		{
			if (abs(i1)+abs(j1)==1 && I+i1>=0 && J+j1>=0 && I+i1<dane.width() && J+j1<dane.height())
			{
				double il;
				double d = sqrt( (double)((I-IC)*(I-IC)) + (double)(J-JC)*(J-JC) );
				il = (double)((I-IC)*j1 - (J-JC)*i1)*(double)flag / d/sqrt((double)(i1*i1+j1*j1));
				if (il > TH) continue;
//				cerr << i1 << " " << j1 << " " << il << endl;
				if (dist(I+i1,J+j1)>dum(I+i1,J+j1)+dist(I,J))
				{
					punkt.x = I+i1;
					punkt.y = J+j1;
					dist(I+i1,J+j1) = dum(I+i1,J+j1) + dist(I,J);
					punkt.kolor = dist(I+i1,J+j1);
					kolejka.push(punkt);
					way(I+i1,J+j1) = 10*i1 + j1;
				}
			}
		}
//		getchar();
//		disp.display(way);
	}

	std::vector<Punkt> lista;
	lista.clear();
	punkt.x = I;
	punkt.y = J;
	lista.push_back(punkt);
	
	while(way(I,J)!=0)
	{
		if (way(I,J)==10) I = I-1;
		else if (way(I,J)==-10) I = I+1;
		else if (way(I,J)==1) J = J-1;
		else J = J+1;
		punkt.x = I;
		punkt.y = J;
		lista.push_back(punkt);
	}

	return lista;
}
