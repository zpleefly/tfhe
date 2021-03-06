#include <cstdlib>
#include <iostream>
#include <random>
#include <cassert>
#include <ccomplex>
#include "tfhe_core.h"
#include "numeric_functions.h"
#include "lweparams.h"
#include "lwekey.h"
#include "lwesamples.h"
#include "lwe-functions.h"
#include "tlwe_functions.h"
#include "tgsw_functions.h"
#include "polynomials_arithmetic.h"
#include "lagrangehalfc_arithmetic.h"
#include "lwebootstrappingkey.h"

using namespace std;

#ifndef NDEBUG
extern const TLweKey* debug_accum_key;
extern const LweKey* debug_extract_key;
extern const LweKey* debug_in_key;
#endif


EXPORT void tLweToFFTConvert(TLweSampleFFT* result, const TLweSample* source, const TLweParams* params){
    const int k = params->k;
    
    for (int i = 0; i <= k; ++i)
	TorusPolynomial_ifft(result->a+i,source->a+i);
}

EXPORT void tLweFromFFTConvert(TLweSample* result, const TLweSampleFFT* source, const TLweParams* params){
    const int k = params->k;
    
    for (int i = 0; i <= k; ++i)
	TorusPolynomial_fft(result->a+i,source->a+i);
}



//Arithmetic operations on TLwe samples
/** result = (0,0) */
EXPORT void tLweFFTClear(TLweSampleFFT* result, const TLweParams* params){
    int k = params->k;

    for (int i = 0; i <= k; ++i) LagrangeHalfCPolynomialClear(&result->a[i]);
    result->current_variance = 0.;
}

/** result = (0,mu) */
EXPORT void tLweFFTNoiselessTrivial(TLweSampleFFT* result, const TorusPolynomial* mu, const TLweParams* params){
    int k = params->k;

    for (int i = 0; i < k; ++i) LagrangeHalfCPolynomialClear(&result->a[i]);
    TorusPolynomial_ifft(result->b, mu);
    result->current_variance = 0.;
}

/** result = (0,mu) where mu is constant*/
EXPORT void tLweFFTNoiselessTrivialT(TLweSampleFFT* result, const Torus32 mu, const TLweParams* params){
    const int k = params->k;
    
    for (int i = 0; i < k; ++i) 
	LagrangeHalfCPolynomialClear(&result->a[i]);
    LagrangeHalfCPolynomialSetTorusConstant(result->b,mu);
    result->current_variance = 0.;
}

/** result = result + sample */
EXPORT void tLweFFTAddTo(TLweSampleFFT* result, const TLweSampleFFT* sample, const TLweParams* params);
//Let's postpone this to make sure we actually need it
//{
//    int k = params->k;
//
//    for (int i = 0; i < k; ++i) 
//	AddToLagrangeHalfCPolynomial(&result->a[i], &sample->a[i]);
//    AddToLagrangeHalfCPolynomial(result->b, sample->b);
//    result->current_variance += sample->current_variance; //à revoir//OK si c'est la variance
//}

/** result = result - sample */
EXPORT void tLweFFTSubTo(TLweSampleFFT* result, const TLweSampleFFT* sample, const TLweParams* params);

/** result = result + p.sample */
EXPORT void tLweFFTAddMulZTo(TLweSampleFFT* result, int p, const TLweSampleFFT* sample, const TLweParams* params);
//Let's postpone this to make sure we actually need it
//{
//    int k = params->k;
//
//    for (int i = 0; i < k; ++i) 
//	torusPolynomialAddMulZTo(&result->a[i], p, &sample->a[i]);
//    torusPolynomialAddMulZTo(result->b, p, sample->b);
//    result->current_variance += (p*p)*sample->current_variance;
//}

/** result = result - p.sample */
EXPORT void tLweFFTSubMulZTo(TLweSampleFFT* result, int p, const TLweSampleFFT* sample, const TLweParams* params);


EXPORT void tLweFFTAddMulRTo(TLweSampleFFT* result, const LagrangeHalfCPolynomial* p, const TLweSampleFFT* sample, const TLweParams* params) {
    const int k = params->k;
    
    for (int i=0; i<=k; i++)
	LagrangeHalfCPolynomialAddMul(result->a+i,p,sample->a+i);
}

EXPORT void tLweFFTMulR(TLweSampleFFT* result, const LagrangeHalfCPolynomial* p, const TLweSampleFFT* sample, const TLweParams* params) {
    const int k = params->k;
    
    for (int i=0; i<=k; i++)
	LagrangeHalfCPolynomialMul(result->a+i,p,sample->a+i);
}

EXPORT void tLweFFTSubMulRTo(TLweSampleFFT* result, const LagrangeHalfCPolynomial* p, const TLweSampleFFT* sample, const TLweParams* params) {
    const int k = params->k;
    
    for (int i=0; i<=k; i++)
	LagrangeHalfCPolynomialSubMul(result->a+i,p,sample->a+i);
}

    
EXPORT void tGswToFFTConvert(TGswSampleFFT* result, const TGswSample* source, const TGswParams* params) {
    const int kpl = params->kpl;
    
    for (int p=0; p<kpl; p++)
	tLweToFFTConvert(result->all_samples+p, source->all_sample+p, params->tlwe_params);
}

EXPORT void tGswFromFFTConvert(TGswSample* result, const TGswSampleFFT* source, const TGswParams* params){
    const int kpl = params->kpl;
    
    for (int p=0; p<kpl; p++)
	tLweFromFFTConvert(result->all_sample+p, source->all_samples+p, params->tlwe_params);
}

EXPORT void tGswFFTAddH(TGswSampleFFT* result, const TGswParams* params) {
    const int k = params->tlwe_params->k;
    const int l = params->l;

    for (int j=0; j<l; j++) {
    	Torus32 hj = params->h[j];
    	for (int i=0; i<=k; i++)
	   LagrangeHalfCPolynomialAddTorusConstant(&result->sample[i][j].a[i],hj); 
    }

}

EXPORT void tGswFFTClear(TGswSampleFFT* result, const TGswParams* params) {
    const int kpl = params->kpl;

    for (int p=0; p<kpl; p++)
	tLweFFTClear(result->all_samples+p, params->tlwe_params);
}    

EXPORT void tGswLagrangeHalfCPolynomialDecompH(LagrangeHalfCPolynomial* reps, const LagrangeHalfCPolynomial* pol, const TGswParams* params) {
    const int l = params->l;
    const int N = params->tlwe_params->N;
    //TODO attention, this prevents parallelization...
    static TorusPolynomial* a = new_TorusPolynomial(N);
    static IntPolynomial* deca = new_IntPolynomial_array(l,N);

    TorusPolynomial_fft(a,pol);
    tGswTorus32PolynomialDecompH(deca, a, params);
    for (int j=0; j<l; j++) {
	IntPolynomial_ifft(reps+j,deca+j);
    }
}

EXPORT void tGswFFTExternMulToTLwe(TLweSampleFFT* accum, TGswSampleFFT* gsw, const TGswParams* params) {
    const TLweParams* tlwe_params=params->tlwe_params;
    const int k = tlwe_params->k;
    const int l = params->l;
    const int kpl = params->kpl;
    const int N = tlwe_params->N;
    //TODO attention, this prevents parallelization...
    static LagrangeHalfCPolynomial* decomps=new_LagrangeHalfCPolynomial_array(kpl,N);

    for (int i=0; i<=k; i++)
	tGswLagrangeHalfCPolynomialDecompH(decomps+i*l,accum->a+i, params);
    tLweFFTClear(accum, tlwe_params);
    for (int p=0; p<kpl; p++)
	tLweFFTAddMulRTo(accum, decomps+p, gsw->all_samples+p, tlwe_params);
}

EXPORT void tGswFFTMulByXaiMinusOne(TGswSampleFFT* result, const int ai, const TGswSampleFFT* bki, const TGswParams* params) {
    const TLweParams* tlwe_params=params->tlwe_params;
    const int k = tlwe_params->k;
    //const int l = params->l;
    const int kpl = params->kpl;
    const int N = tlwe_params->N;
    //on calcule x^ai-1 en fft
    //TODO attention, this prevents parallelization...
    static LagrangeHalfCPolynomial* xaim1=new_LagrangeHalfCPolynomial(N);
    LagrangeHalfCPolynomialSetXaiMinusOne(xaim1,ai);
    for (int p=0; p<kpl; p++) {
        const LagrangeHalfCPolynomial* in_s = bki->all_samples[p].a;
        LagrangeHalfCPolynomial* out_s = result->all_samples[p].a;
        for (int j=0; j<=k; j++)
            LagrangeHalfCPolynomialMul(&out_s[j], xaim1, &in_s[j]); 
    }
}

EXPORT void tfhe_createLweBootstrappingKeyFFT(
	LweBootstrappingKeyFFT* bk, 
	const LweKey* key_in, 
	const TGswKey* rgsw_key) {
    assert(bk->bk_params==rgsw_key->params);
    assert(bk->in_out_params==key_in->params);

    const LweParams* in_out_params = bk->in_out_params; 
    const TGswParams* bk_params = bk->bk_params;
    const TLweParams* accum_params = bk_params->tlwe_params;
    const LweParams* extract_params = &accum_params->extracted_lweparams;
    
    //LweKeySwitchKey* ks; ///< the keyswitch key (s'->s)
    const TLweKey* accum_key = &rgsw_key->tlwe_key;
    LweKey* extracted_key = new_LweKey(extract_params);
    tLweExtractKey(extracted_key, accum_key);
    lweCreateKeySwitchKey(bk->ks, extracted_key, key_in);
    delete_LweKey(extracted_key);
    
    //TGswSample* bk; ///< the bootstrapping key (s->s")
    TGswSample* tmpsample = new_TGswSample(bk_params);
    int* kin = key_in->key;
    const double alpha = accum_params->alpha_min;
    const int n = in_out_params->n;
    for (int i=0; i<n; i++) {
	tGswSymEncryptInt(tmpsample, kin[i], alpha, rgsw_key);
	tGswToFFTConvert(&bk->bk[i], tmpsample, bk_params);
    }
    delete_TGswSample(tmpsample);
}


EXPORT void tfhe_bootstrapFFT(LweSample* result, const LweBootstrappingKeyFFT* bk, Torus32 mu1, Torus32 mu0, const LweSample* x){
    const Torus32 ab=(mu1+mu0)/2;
    const Torus32 aa = mu0-ab; // aa=(mu1-mu0)/2;
    const TGswParams* bk_params = bk->bk_params;
    const TLweParams* accum_params = bk_params->tlwe_params;
    const LweParams* extract_params = &accum_params->extracted_lweparams;
    const LweParams* in_out_params = bk->in_out_params;
    const int n=in_out_params->n;
    const int N=accum_params->N;
    const int Ns2=N/2;
    const int Nx2= 2*N;
    
    
    TorusPolynomial* testvect=new_TorusPolynomial(N);
    TorusPolynomial* testvectbis=new_TorusPolynomial(N);


    int barb=modSwitchFromTorus32(x->b,Nx2);
    //je definis le test vector (multiplié par a inclus !
    for (int i=0;i<Ns2;i++)
       testvect->coefsT[i]=aa;
    for (int i=Ns2;i<N;i++)
       testvect->coefsT[i]=-aa;
    torusPolynomialMulByXai(testvectbis, barb, testvect);

    // Accumulateur 
    TLweSample* acc = new_TLweSample(accum_params);
    TLweSampleFFT* accFFT = new_TLweSampleFFT(accum_params);

    // acc and accFFt will be used for tfhe_bootstrapFFT, acc1=acc will be used for tfhe_bootstrap
    tLweNoiselessTrivial(acc, testvectbis, accum_params);
    tLweToFFTConvert(accFFT, acc, accum_params);

    TGswSample* temp = new_TGswSample(bk_params);
    TGswSampleFFT* tempFFT = new_TGswSampleFFT(bk_params);

//NICOLAS: j'ai ajouté ce bloc
#ifndef NDEBUG
    TorusPolynomial* phase = new_TorusPolynomial(N);
    int correctOffset = barb;
    cout << "starting the test..." << endl;
#endif
    // the index 1 is given when we don't use the fft
    for (int i=0; i<n; i++) {
        int bara=modSwitchFromTorus32(-x->a[i],Nx2);
        
        if (bara!=0) {
            tGswFFTMulByXaiMinusOne(tempFFT, bara, bk->bk+i, bk_params);
            tGswFFTAddH(tempFFT, bk_params);
            tGswFFTExternMulToTLwe(accFFT, tempFFT, bk_params);
        }

//NICOLAS: et surtout, j'ai ajouté celui-ci!
#ifndef NDEBUG
            tLweFromFFTConvert(acc, accFFT, accum_params);
        tLwePhase(phase,acc,debug_accum_key);  //celui-ci, c'est la phase de acc (FFT)
	if (debug_in_key->key[i]==1) correctOffset = (correctOffset+bara)%Nx2; 
        torusPolynomialMulByXai(testvectbis, correctOffset, testvect); //celui-ci, c'est la phase idéale (calculée sans bruit avec la clé privée)
	for (int j=0; j<N; j++) {
	       printf("Iteration %d, index %d: phase %d vs noiseless %d\n",i,j,phase->coefsT[j], testvectbis->coefsT[j]);
	}
#endif

    }
    tLweFromFFTConvert(acc, accFFT, accum_params);


    LweSample* u = new_LweSample(extract_params);
    tLweExtractLweSample(u, acc, extract_params, accum_params);
    u->b += ab;
    
    lweKeySwitch(result, bk->ks, u);
    


    delete_LweSample(u);
    delete_TGswSampleFFT(tempFFT); 
    delete_TGswSample(temp);
    delete_TLweSampleFFT(accFFT);
    delete_TLweSample(acc);
    delete_TorusPolynomial(testvectbis);
    delete_TorusPolynomial(testvect);
}
