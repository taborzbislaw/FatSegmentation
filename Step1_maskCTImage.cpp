#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <queue>
#include <vector>
#include <time.h>
#include "CImg.h"

using namespace std;
using namespace cimg_library;

#define DMIN -20000

class Punkt
{
public:
  int x;
  int y;
  
  Punkt(){}
  
  Punkt(int X,int Y) {x=X;y=Y;}
  
  bool operator == (const Punkt &p) const
  {
	  if ((*this).x == p.x && (*this).y == p.y )
		  return true;
	  else
		  return false;
  }
};

int main(int argc, char *argv[])
{
	clock_t start, end;
    double cpu_time_used;

	int Ni=512,Nj=512;

	CImg<signed short> dane(Ni,Nj);
	
	dane.load_raw(argv[1],Ni,Nj,1,1,false,true);

//	cerr << "Punkt (0,0) " << dane(0,0) << " " << dane(1,0) << " " << dane(0,1) << endl;
start = clock();

	if (dane(0,0)>=0 && dane(1,0)>=0 && dane(0,1) >=0)
	{
		for(int i=0;i<Ni;i++)
		for(int j=0;j<Nj;j++)
			dane(i,j) -= 1024;
	}
//	dane.display();

	int TH;

	TH = -191;			//Według metodyki	
	if (argc>=3) TH = atoi(argv[2]);

	CImg<unsigned char> maska(Ni,Nj);
	maska.fill(0);

	for(int i=0;i<Ni;i++)
	for(int j=0;j<Nj;j++)
	{
		if (dane(i,j)>TH) maska(i,j) = 255;
	}
	
/******************************************/

	queue<Punkt> kolejka;

	int X=0,Y=0;
	int BOX = 5;
	for(X=Ni/2-BOX;X<=Ni/2+BOX;X++)
	for(Y=Nj/2-BOX;Y<=Nj/2+BOX;Y++)
	{
		if (maska(X,Y)==0) continue;
		maska(X,Y) = 1;
		kolejka.push(Punkt(X,Y));
	}
	if (kolejka.size()==0)
	{
//		cerr << "Kolejka pusta!!!" << endl;
		getchar();
	}
//	cerr << "kolejka zainicjowana" << endl;

	int licz = 0;
	while(kolejka.size())
	{
		licz++;
//		if (licz%10000==0) cerr << "Licznik =" << kolejka.size() << endl;
		Punkt p = kolejka.front();
		kolejka.pop();
		int I = p.x;
		int J = p.y;
		for(int i1=-1;i1<=1;i1++)
		for(int j1=-1;j1<=1;j1++)
		{
			if (abs(i1)+abs(j1)>=1 && I+i1>=0 && I+i1<Ni && J+j1>=0 && J+j1<Nj)
			{
				if (maska(I+i1,J+j1)==255)
				{
					maska(I+i1,J+j1) = 1;
					kolejka.push(Punkt(I+i1,J+j1));
				}
			}
		}
	}

	for(int i=0;i<Ni;i++)
	for(int j=0;j<Nj;j++)
	{
		if (maska(i,j)!=1) maska(i,j) = 0;
		else maska(i,j)=255;
	}

//	maska.display();

	for(X=0;X<=BOX;X++)
	for(Y=0;Y<=BOX;Y++)
	{
		if (maska(X,Y)!=0) continue;
		maska(X,Y) = 128;
		kolejka.push(Punkt(X,Y));
	}
	for(X=Ni-BOX;X<Ni;X++)
	for(Y=0;Y<=BOX;Y++)
	{
		if (maska(X,Y)!=0) continue;
		maska(X,Y) = 128;
		kolejka.push(Punkt(X,Y));
	}
	for(X=0;X<=BOX;X++)
	for(Y=Nj-BOX;Y<Nj;Y++)
	{
		if (maska(X,Y)!=0) continue;
		maska(X,Y) = 128;
		kolejka.push(Punkt(X,Y));
	}
	for(X=Ni-BOX;X<Ni;X++)
	for(Y=Nj-BOX;Y<Nj;Y++)
	{
		if (maska(X,Y)!=0) continue;
		maska(X,Y) = 128;
		kolejka.push(Punkt(X,Y));
	}
	if (kolejka.size()==0)
	{
//		cerr << "Kolejka pusta!!!" << endl;
		getchar();
	}
//	cerr << "kolejka zainicjowana" << endl;

	licz = 0;
	while(kolejka.size())
	{
		licz++;
//		if (licz%10000==0) cerr << "Licznik =" << kolejka.size() << endl;
		Punkt p = kolejka.front();
		kolejka.pop();
		int I = p.x;
		int J = p.y;
		for(int i1=-1;i1<=1;i1++)
		for(int j1=-1;j1<=1;j1++)
		{
			if (abs(i1)+abs(j1)>=1 && I+i1>=0 && I+i1<Ni && J+j1>=0 && J+j1<Nj)
			{
				if (maska(I+i1,J+j1)==0)
				{
					maska(I+i1,J+j1) = 128;
					kolejka.push(Punkt(I+i1,J+j1));
				}
			}
		}
	}

	for(int i=0;i<Ni;i++)
	for(int j=0;j<Nj;j++)
	{
		if (maska(i,j)==128) maska(i,j) = 0;
		else maska(i,j)=255;
	}

//	maska.display();


	for(int i=0;i<Ni;i++)
	for(int j=0;j<Nj;j++)
	{
		if (!maska(i,j)) dane(i,j) = DMIN;
	}

     end = clock();
     cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

	cerr << cpu_time_used << endl;

	char name[1000];
	sprintf(name,"maska_%s",argv[1]);
	dane.save_raw(name);

}
