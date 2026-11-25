// A tool for burning visible pictures on a compact disc surfase
// Copyright (C) 2008-2022 arduinocelentano

//This program is free software: you can redistribute it and/or modify it under the terms
//of the GNU General Public License as published by the Free Software Foundation,
//either version 3 of the License, or (at your option) any later version.

//This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
//without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//See the GNU General Public License for more details.

//You should have received a copy of the GNU General Public License along with this program.
//If not, see <https://www.gnu.org/licenses/>.

//Converter does the actual job to generate Audio CD track from the image.

//The implementation of this converter if mostly based on an opensource
//project created by a person with username [undefer]. Unfortunately I can't
//find her or his github or whatever repository.
//I acknowledge and am grateful to this developer for their contributions.

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "converter.h"

Converter::Converter(QObject *parent)
 : QObject(parent)
{
#define D 4
	m_tr0=22951.52052;
	m_dtr=1.3865961805;
	m_r0=24.5;
	nh=28*D-1;
	pinf=0;
	c=0;
#undef D
}

Converter::Converter(QObject *parent, double tr0, double dtr, double r0)
	:QObject(parent), m_tr0(tr0), m_dtr(dtr), m_r0(r0)
{
#define D 4
	nh=28*D-1;
	pinf=0;
	c=0;
#undef D
}

Converter::~Converter()
{
}


bool Converter::convert(const QImage img, const QString& filename)
{
	const char pallete[]={'\x10','\x21','\x28','\xAA'};
	double tr = m_tr0;
	double r = m_r0;
	double dr = m_dtr * m_r0 / m_tr0;
	int all=800*1024*1024;
	// ponytail: tuning sweep, off unless SWEEP_LO and SWEEP_HI (percent) are set.
	// Turn length drifts from x=LO to x=HI across the disc: tr = tr0*(1-x) + n*dtr*(1-2x)
	// (same-capacity disc: faster scan speed x means pitch -x). Where x hits the real
	// value, spokes show. Writes <track>.sweep.csv mapping radius -> x.
	const char *sweepLo = getenv("SWEEP_LO"), *sweepHi = getenv("SWEEP_HI");
	bool sweep = sweepLo && sweepHi;
	double xLo = sweep ? atof(sweepLo)/100 : 0, xHi = sweep ? atof(sweepHi)/100 : 0;
	double nEst = (-m_tr0 + sqrt(m_tr0*m_tr0 + 2*m_dtr*all)) / m_dtr;
	long long turn = 0;
	FILE *sweepLog = sweep ? fopen((filename + ".sweep.csv").toLocal8Bit().constData(), "w") : nullptr;
	if (sweepLog) fprintf(sweepLog, "turn,radius_mm,x_percent,tr0_equiv,dtr_equiv\n");
    m_canceled = false;
	double c=0;
	int itr;
	double ri;
	double ir = 1500;
	double alpha, xi, yi;
	double rcd=57.5;
	double cx = 1500;
	double cy = 1500;
	int zs=0;
	int zf=0;
	int ic=0;
	unsigned char color = 0, cl = 0, c1 = 0 , c2 = 0 ;
	QFile imageFile(filename);
	imageFile.remove();
	imageFile.open(QIODevice::WriteOnly);
	while (c<(all-tr))
	{
        if (m_canceled){
            imageFile.remove();
            return false;
        }
        emit progressChanged(100*c/all);
		if (sweep)
		{
			double x = xLo + (xHi - xLo) * turn / nEst;
			tr = m_tr0*(1-x) + turn*m_dtr*(1-2*x);
			if (sweepLog && turn % 500 == 0)
				fprintf(sweepLog, "%lld,%.3f,%.4f,%.4f,%.9f\n", turn, r, x*100, m_tr0*(1-x), m_dtr*(1-2*x));
		}
		turn++;
		itr = int(tr);
		ri=ir*r/rcd;
		for (long long i=0; i<itr; i++)
		{
			alpha=2*M_PI*i/itr;
			xi=cx+ri*cos(alpha);
			yi=cy+ri*sin(alpha);
			color=qGray(img.pixel(int(xi),int(yi)));
			c1=color/85;
			c2=c1+1;
			//ad (pallete[cl], imagefile);
			//imageFile.write(&pallete[cl],1);
			
			if(m_mixColors)
				cl = ( ((rand()*85/RAND_MAX)<(color%85)) || ((color%85)==84))? c2: c1;
			else
				cl=( ( (color%85)>(zs*5+zf) ) || ((color%85)==84))?c2:c1;
			ad(pallete[cl],&imageFile);
			if ((++zf)>=5)
				zf=0;
		}
		c+=tr;
		ic+=itr;
		while ( ((int) c)>ic )
		{
			//ad (pallete[color/51], imagefile);
			//imageFile.write(&pallete[cl],1);
			ad(pallete[cl],&imageFile);
			ic++;
			if ((++zf)>=4) zf=0;
		}

		tr+=m_dtr;
		r+=dr;
		
		if ((++zs)>=17) zs=0;
	}
	imageFile.close();
	if (sweepLog) fclose(sweepLog);
    return true;
}

void Converter::ad (char b, QFile *file)
{
	intseq[n2m(delays[pinf++])]=b;
	if (pinf>=24)
	{
		pinf=0;
		nh++;
		if (nh>=28*4) nh=0;
		for (int i=0; i<24; i++)
		{
			bw(intseq[n2m(i)], file);
		}
	}
}

inline int Converter::n2m(int n)
{
	if ((nh*24+n)>=(28*4*24))
		return nh*24+n-28*4*24;
	else if ((nh*24+n)<0)
		return nh*24+n+28*4*24;
	else return nh*24+n;
}

void Converter::bw (char b, QFile *file)
{
	buffer[c++]=b;
	if (c>=2352)
	{
		if (file->write(buffer, 2352)==-1)
		{ 
			qCritical ("Converter: Cannot write data to file");
			return;
		}
		c=0;
	}
}

void Converter::cancelConverting()
{
    m_canceled = true;
}
