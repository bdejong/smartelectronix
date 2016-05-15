// Compressor.cpp: implementation of the CCompressor class.
//
//////////////////////////////////////////////////////////////////////

#include "Compressor.h"
#include "math.h"
#include "stdio.h"

#define IS_DENORMAL(f) (((*(unsigned int *)&(f))&0x7f800000)==0)

#ifdef MAC
        #define iexp_                           0
        #define iman_                           1
#else //LittleEndian
        #define iexp_                           1
        #define iman_                           0
#endif

const double _double2fixmagic = 68719476736.0*1.5;
const long   _shiftamt        = 16;

inline long fastcast(double val)
{
   val += _double2fixmagic;
   return ((long*)&val)[iman_] >> _shiftamt;
};

inline long fastcast(float val)
{
   return fastcast((double)val);
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCompressor::CCompressor(float samplerate)
{
	this->samplerate = samplerate;
	attack = 40.f;
	release = 40.f;

	nLUT = 4000; //more than enough!
	
	LUT = new float[nLUT];
	dLUT = new float[nLUT];

	setSamplerate(samplerate); //this sets attack and release too...

	setRelease(0.1f);
	setAttack(0.1f);
	setPostamp(0.1f);
	setPreamp(0.5f);
	setAmount(0.1f);
	setSaturate(1.f);

	Suspend();

	msmooth = 0.f;
}

CCompressor::~CCompressor()
{
	delete LUT;
	delete dLUT;
}

void CCompressor::process(float **input, float **output, long nSamples)
{
	float *in1  =  input[0];
	float *in2  =  input[1];
	float *out1 = output[0];
	float *out2 = output[1];

	int n;
	float alpha;
	float ntmp;
	float inL,inR,in;
	float m;
	
	float nLUTf_1 = (float)nLUT - 1.f;

	if(saturate)
	while(nSamples--)
	{
		//preamplify
		inL = (*in1++)*preamp;
		inR = (*in2++)*preamp;

		//this is just input = max(abs(L),abs(R))
		if(inL>0.f)
		{
			if(inR>0.f)
			{
				if(inL>inR)
					in = inL;
				else
					in = inR;
			}
			else
			{
				if(inL>-inR)
					in = inL;
				else
					in = -inR;
			}
		}
		else
		{
			if(inR>0.f)
			{
				if(-inL>inR)
					in = -inL;
				else
					in = inR;
			}
			else
			{
				if(-inL>-inR)
					in = -inL;
				else
					in = -inR;
			}

		}
			
		//get the envelope
		if(e < in)
			e = ga*(e-in) + in;
		else
			e = gr*(e-in) + in;
		
		//lopass
		eLP += 0.001f*(e - eLP);
		
		//calc the compressor response

		ntmp = eLP*nLUTf_1;
		n = fastcast(ntmp);

		if(n >= nLUT)
			m = LUT[nLUT-1];
		else
		{
			alpha = (float)n - ntmp;
			m = LUT[n] + alpha*dLUT[n];
		}

		//m = multiplier...

		//msmooth += 0.001f*(m - msmooth);

		inL *= m;//smooth;
		inR *= m;//smooth;
		
		//saturate left
		//signum(x)*(2*abs(x) - x*x), stolen from POD
		if(inL>0.f)
		{
			if(inL>1.f)
				inL = 1.f;
			else
				inL = 2.f*inL - inL*inL;
		}
		else
		{
			if(inL<-1.f)
				inL = -1.f;
			else
				inL  = 2.f*inL + inL*inL;
		}

		if(inR>0.f)
		{
			if(inR>1.f)
				inR = 1.f;
			else
				inR = 2.f*inR - inR*inR;
		}
		else
		{
			if(inR<-1.f)
				inR = -1.f;
			else
				inR  = 2.f*inR + inR*inR;
		}

		//return
		*out1++ += inL;
		*out2++ += inR;
	}
	else
	while(nSamples--) //non saturating
	{
		//preamplify
		inL = (*in1++)*preamp;
		inR = (*in2++)*preamp;

		if(inL>0.f)
		{
			if(inR>0.f)
			{
				if(inL>inR)
					in = inL;
				else
					in = inR;
			}
			else
			{
				if(inL>-inR)
					in = inL;
				else
					in = -inR;
			}
		}
		else
		{
			if(inR>0.f)
			{
				if(-inL>inR)
					in = -inL;
				else
					in = inR;
			}
			else
			{
				if(-inL>-inR)
					in = -inL;
				else
					in = -inR;
			}

		}
			
		//get the envelope
		if(e < in)
			e = ga*(e-in) + in;
		else
			e = gr*(e-in) + in;
		
		//lopass
		eLP += 0.001f*(e - eLP);
		
		//calc the compressor response

		ntmp = eLP*nLUTf_1;
		n = fastcast(ntmp);

		if(n >= nLUT)
			m = LUT[nLUT-1];
		else
		{
			alpha = (float)n - ntmp;
			m = LUT[n] + alpha*dLUT[n];
		}

		//m = multiplier...

		//msmooth += 0.001f*(m - msmooth);

		inL *= m;//smooth;
		inR *= m;//smooth;
		
		//saturate left
		//signum(x)*(2*abs(x) - x*x), stolen from POD
		if(inL>0.f)
		{
			if(inL>1.f)
				inL = 1.f;
		}
		else
		{
			if(inL<-1.f)
				inL = -1.f;
		}

		if(inR>0.f)
		{
			if(inR>1.f)
				inR = 1.f;
		}
		else
		{
			if(inR<-1.f)
				inR = -1.f;
		}

		//return
		*out1++ += inL;
		*out2++ += inR;
	}


	if(IS_DENORMAL(e))
		e = 0.f;

	if(IS_DENORMAL(eLP))
		eLP = 0.f;

	if(IS_DENORMAL(msmooth))
		msmooth = 0.f;
}

void CCompressor::processReplacing(float **input, float **output, long nSamples)
{	
	float *in1  =  input[0];
	float *in2  =  input[1];
	float *out1 = output[0];
	float *out2 = output[1];

	int n;
	float alpha;
	float ntmp;
	float inL,inR,in;
	float m;
	
	float nLUTf_1 = (float)nLUT - 1.f;

	if(saturate)
	while(nSamples--)
	{
		//preamplify
		inL = (*in1++)*preamp;
		inR = (*in2++)*preamp;

		//this is just input = max(abs(L),abs(R))
		if(inL>0.f)
		{
			if(inR>0.f)
			{
				if(inL>inR)
					in = inL;
				else
					in = inR;
			}
			else
			{
				if(inL>-inR)
					in = inL;
				else
					in = -inR;
			}
		}
		else
		{
			if(inR>0.f)
			{
				if(-inL>inR)
					in = -inL;
				else
					in = inR;
			}
			else
			{
				if(-inL>-inR)
					in = -inL;
				else
					in = -inR;
			}

		}
			
		//get the envelope
		if(e < in)
			e = ga*(e-in) + in;
		else
			e = gr*(e-in) + in;
		
		//lopass
		eLP += 0.001f*(e - eLP);
		
		//calc the compressor response

		ntmp = eLP*nLUTf_1;
		n = fastcast(ntmp);

		if(n >= nLUT)
			m = LUT[nLUT-1];
		else
		{
			alpha = (float)n - ntmp;
			m = LUT[n] + alpha*dLUT[n];
		}

		//m = multiplier...

		//msmooth += 0.001f*(m - msmooth);

		inL *= m;//smooth;
		inR *= m;//smooth;
		
		//saturate left
		//signum(x)*(2*abs(x) - x*x), stolen from POD
		if(inL>0.f)
		{
			if(inL>1.f)
				inL = 1.f;
			else
				inL = 2.f*inL - inL*inL;
		}
		else
		{
			if(inL<-1.f)
				inL = -1.f;
			else
				inL  = 2.f*inL + inL*inL;
		}

		if(inR>0.f)
		{
			if(inR>1.f)
				inR = 1.f;
			else
				inR = 2.f*inR - inR*inR;
		}
		else
		{
			if(inR<-1.f)
				inR = -1.f;
			else
				inR  = 2.f*inR + inR*inR;
		}

		//return
		*out1++ = inL;
		*out2++ = inR;
	}
	else
	while(nSamples--) //non saturating
	{
		//preamplify
		inL = (*in1++)*preamp;
		inR = (*in2++)*preamp;

		if(inL>0.f)
		{
			if(inR>0.f)
			{
				if(inL>inR)
					in = inL;
				else
					in = inR;
			}
			else
			{
				if(inL>-inR)
					in = inL;
				else
					in = -inR;
			}
		}
		else
		{
			if(inR>0.f)
			{
				if(-inL>inR)
					in = -inL;
				else
					in = inR;
			}
			else
			{
				if(-inL>-inR)
					in = -inL;
				else
					in = -inR;
			}

		}
			
		//get the envelope
		if(e < in)
			e = ga*(e-in) + in;
		else
			e = gr*(e-in) + in;
		
		//lopass
		eLP += 0.001f*(e - eLP);
		
		//calc the compressor response

		ntmp = eLP*nLUTf_1;
		n = fastcast(ntmp);

		if(n >= nLUT)
			m = LUT[nLUT-1];
		else
		{
			alpha = (float)n - ntmp;
			m = LUT[n] + alpha*dLUT[n];
		}

		//m = multiplier...

		//msmooth += 0.001f*(m - msmooth);

		inL *= m;//smooth;
		inR *= m;//smooth;
		
		//saturate left
		//signum(x)*(2*abs(x) - x*x), stolen from POD
		if(inL>0.f)
		{
			if(inL>1.f)
				inL = 1.f;
		}
		else
		{
			if(inL<-1.f)
				inL = -1.f;
		}

		if(inR>0.f)
		{
			if(inR>1.f)
				inR = 1.f;
		}
		else
		{
			if(inR<-1.f)
				inR = -1.f;
		}

		//return
		*out1++ = inL;
		*out2++ = inR;
	}


	if(IS_DENORMAL(e))
		e = 0.f;

	if(IS_DENORMAL(eLP))
		eLP = 0.f;

	if(IS_DENORMAL(msmooth))
		msmooth = 0.f;
}

//calculates the LUT
void CCompressor::CalcLUT()
{
	float nLUTf_1 = (float)nLUT - 1.f;
	float nLUTf_2 = (float)nLUT - 2.f;

	float y,z,zz;

	float k = 2.f*amount/(1.f-amount);
		
	for(int i=0;i<nLUT-1;i++)
	{
		if(i==0)
			z = 1/nLUTf_2;
		else
			z = (float)i/nLUTf_2;
		
		zz = z*z;
		y = (zz + z*10.f)/(zz + z*9.f + 1);
		
		LUT[i] = 0.5f*postamp*(1.f+k)*y/(1.f+k*y)/z;
	}
	LUT[nLUT-1] = LUT[nLUT-2];

	for(i=0;i<nLUT-1;i++)
		dLUT[i] = LUT[i+1] - LUT[i];
	dLUT[nLUT-1] = 0.f;

	/*FILE *pf = fopen("c:\\amp.ff","wb");
	fwrite(LUT,sizeof(float),nLUT,pf);
	fclose(pf);*/
}

//attack in ms
void CCompressor::setAttack(float a)
{
	attack = 5.f + a*145.f;
	if(attack<1.f)
		attack = 1.f;
	ga = (float) exp(-1/(samplerate*attack/1000.f));
}

//release in ms
void CCompressor::setRelease(float r)
{
	release = 5.f + r*145.f;
	if(release<1.f)
		release = 1.f;
	gr = (float) exp(-1/(samplerate*release/1000.f));
}

void CCompressor::Suspend()
{
	msmooth=e=eLP=0;
}

void CCompressor::setSamplerate(float sr)
{
	samplerate = sr;
	setAttack((attack-5.f)/145.f);
	setRelease((release-5.f)/145.f);
}

void CCompressor::setPreamp(float p)
{
	if(p<0.5f)
	{
		float k = -0.95f;
		preamp = p*2*(1.f+k)/(1.f+k*p*2);
	}
	else
	{
		preamp = p*2.f;
	}
}

void CCompressor::setAmount(float a)
{
	this->a = a;
	amount = a*0.995f;
	CalcLUT();
}

void CCompressor::setPostamp(float p)
{
	float k = -0.95f;
	postamp = p*(1.f+k)/(1.f+k*p);
	CalcLUT();
}

void CCompressor::setSaturate(float s)
{
	saturate = (s > 0.5f);
}
