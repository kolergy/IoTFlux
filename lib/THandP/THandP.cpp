/* tools to calculate things with Temerature, Humidity & Pressure (if available)*/
#include <Arduino.h>

#include <THandP.h>


float get_air_density(float P_amb_Pa, float T_amb_K) {
  float rho      = P_amb_Pa/(RAir_J_kg_K*T_amb_K);
  return(rho);
}


float calcDewPoint(float t_c,float rh_pc) {                                  
  // °C Dew Point Temperature estimation based on Buck equation https://www.yaga.no/Dewpoint%20Equations.pdf
  float alph = ((18.678-t_c/234.5)*t_c)/(257.14+t_c);
  float beta = log(rh_pc/100)+alph;
  float a    = 2.0/234.5;                              
  float b    = 18.678-beta;                     
  float c    =-257.14*beta;                     
  
  float tdp_c  = 1/a*(b-sqrt(pow(b,2)+2*a*c));
  return(tdp_c);
}


float calcFrostPoint(float t_c, float tdp_c) {                                // °C Frost point temperature estimation from: https://gist.github.com/sourceperl/45587ea99ff123745428
    float tdp_k  = tdp_c + C2K;                                               //                                             https://weather.station.software/blog/what-are-dew-and-frost-points/
    float t_k    = t_c   + C2K;  
    float t_fp_k = tdp_k - t_k + 2671.02 / ((2954.61 / t_k) + 2.193665 * log(t_k) - 13.3448);
    float t_fp_c = min(t_fp_k - C2K, 0.0);
    return(t_fp_c);
}


float calcHeatIndex(float t_c,float rh_pc) { 
  // Heat index coeficients for calculation based on celsius temperature and relative humidity 
#define HI_C1 -8.78469475556
#define HI_C2  1.61139411
#define HI_C3  2.33854883889
#define HI_C4 -0.14611605
#define HI_C5 -0.012308094
#define HI_C6 -0.0164248277778
#define HI_C7  0.002211732
#define HI_C8  0.00072546
#define HI_C9 -0.000003582 
  // calculate the heat index based on the tables by Steadman. Anderson et al. (2013) https://www.ncbi.nlm.nih.gov/pmc/articles/PMC3801457
  float hi_c = HI_C1\
             + HI_C2*t_c\
             + HI_C3*rh_pc\
             + HI_C4*t_c*rh_pc\
             + HI_C5*t_c*t_c\
             + HI_C6*rh_pc*rh_pc\
             + HI_C7*t_c*t_c*rh_pc\
             + HI_C8*t_c*rh_pc*rh_pc\
             + HI_C9*t_c*t_c*rh_pc*rh_pc;
  return(hi_c);
}


float calcRoA(float P_Pa, float T_K) { return( P_Pa / (RAir_J_kg_K*(T_K))); }          // kg/m^3 Air Density Ideal Gas


float calcRoS(float P_Pa, float T_K) { return( P_Pa / (RWat_J_kg_K*(T_K))); }          // kg/m^3 Steam Density Ideal Gas


float PSvapour(float T_K) {                                                 // Pa Saturation vapour pressure for water based on buck equation
  float T_C = T_K - C2K;
  float PSvap_Pa = 1000*0.61121*exp((18.678-T_C/234.5)*(T_C/(257.14+T_C)));
  //Serial.println("PSvapour   : " + String(PSvap      ) + " Pa");
  return(PSvap_Pa);
}


float Pvapour(float T_K, float RH) {                                        // Pa Partial pressure for warter vapour based on Relative humidity
  float Pvap_Pa = PSvapour(T_K)*RH;
  //Serial.println("Pvapour    : " + String(Pvap      ) + " Pa");
  return(Pvap_Pa);
}


float PAir(float P_Pa, float T_K, float RH) {                                  // Pa Dry Air Partiual pressure 
  float PVap_Pa = Pvapour(T_K, RH);
  float PDA_Pa  = P_Pa - PVap_Pa;
  Serial.println("PAir       : " + String(PDA_Pa      ) + " Pa");
  return(PDA_Pa);
}


float WContent(float T_K, float RH) {                                       // kg/m^3 Water vapour content (Vapor concentration)
  float PVap_Pa    = Pvapour(T_K, RH);
  float roS_kg_m3  = calcRoS(PVap_Pa, T_K);
  //Serial.println("WContent   : " + String(roS*1000) + " g/m^3");
  return(roS_kg_m3);
}


float AContent(float P_Pa, float T_K, float RH) {                              // kg/m^3 Dry Air content 
  float PA_Pa     = PAir(P_Pa, T_K, RH);
  float roA_kg_m3 = calcRoA(PA_Pa, T_K);
  Serial.println("AContent   : " + String(roA_kg_m3*1000) + " g/m^3");
  return(roA_kg_m3);
}


float calcSH(float P_Pa, float T_K, float RH) {                                // kg/kg Specific humidity ratio of vapour over total 
  float MA_kg_m3 = AContent(P_Pa, T_K, RH);
  float MW_kg_m3 = WContent(      T_K, RH);
  float SH       = MW_kg_m3 / (MW_kg_m3+MA_kg_m3);
  Serial.println("Spec Hum: " + String(SH*1000) + " g/kg");
  return(SH);
}


float calcMixR(float P_Pa, float T_K, float RH) {                                // (kg/kg) Mixing ratio  
  float MA_kg_m3   = AContent(P_Pa, T_K, RH);
  float MW_kg_m3   = WContent(   T_K, RH);
  float MxA_kg_kg  = MW_kg_m3 / MA_kg_m3;
  float Pvap_Pa    = Pvapour(T_K, RH);
  float b          = 621.9907/1000.0;  // kg/kg
  float MxB_kg_kg  = b*Pvap_Pa/(P_Pa-Pvap_Pa);
  Serial.println("MxA        : " + String(MxA_kg_kg*1000) + " g/kg");
  Serial.println("MxB        : " + String(MxB_kg_kg*1000) + " g/kg");
  return(MxA_kg_kg);
}


float cpA(float T_K){                                                       // J/kgK  Cp Correlation for dry air 
    float T         = (T_K-C2K)/1000.0;
    float Cp_J_kg_K = 1000.0 * 0.992313 + 0.236688*T - 1.852148*pow(T,2.0) + 6.083152*pow(T,3.0) - 8.893933*pow(T,4.0) + 7.097112*pow(T,5.0) - 3.234725*pow(T,6.0) \
         + 0.794571*pow(T,7.0) - 0.081873*pow(T,8.0);                     // Walsh & Fletcher F3.23 (J/kgK) 200-2000K
    return(Cp_J_kg_K);   
}  


float cpS(float T_K){                                                       // J/kgK  Cp Correlation for steam air 
    float T         = (T_K-C2K)/1000.0;
    float Cp_J_kg_K = 1000.0 * 1.937043 - 0.967916*T + 3.338905*pow(T,2.0) - 3.652122*pow(T,3.0) + 2.332470*pow(T,4.0) - 0.819451*pow(T,5.0) + 0.118783*pow(T,6.0);             // Walsh & Fletcher F3.23 (J/kgK) 200-2000K
    return(Cp_J_kg_K);   
}  


float gammaA(float T_K){                                                    //      Heat capacity ratio Ideal Gas
    float Cp_J_kg_K = cpA(T_K);
    return(Cp_J_kg_K /(Cp_J_kg_K-RAir_J_kg_K));
}


float gammaS(float T_K){                                                    //      Heat capacity ratio Ideal Gas
    float Cp_J_kg_K = cpS(T_K);
    return(Cp_J_kg_K /(Cp_J_kg_K-RWat_J_kg_K));
}


float gamma(float P_Pa, float T_K, float RH){                                  //      Heat capacity ratio Ideal Gas
    float WAr_kg_kg = calcSH(P_Pa, T_K, RH); 
    float WArMolar  = WAr_kg_kg * 28.96/18.015;
    float gA        = gammaA(T_K);
    float gS        = gammaS(T_K);
    float gfac      = (WArMolar * gS + (1 - WArMolar) * gA) / gA;
    return(gfac);
}


float calcDewPoint(float T_K, float RH) {                                    // K Dew Point Temperature estimation based on Buck equation
  float T_C  = T_K     - C2K;
  float alph = ((18.678-T_C/234.5)*T_C)/(257.14+T_C);
  float beta = log(RH) + alph;
  float a    = 2.0     / 234.5;                              
  float b    = 18.678  - beta;                     
  float c    =-257.14  * beta;                     
  
  float Tdp_K = C2K + 1/a*(b-sqrt(pow(b,2)+2*a*c));
  return(Tdp_K);
}


float calcFrostPoint(float T_K, float Tdp_K) {                               // K Frost point temperature estimation from: https://gist.github.com/sourceperl/45587ea99ff123745428

    float Tfp_K = Tdp_K - T_K + 2671.02 / ((2954.61 / T_K) + 2.193665 * log(T_K) - 13.3448);
    return(Tfp_K);
}

