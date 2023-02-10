

#ifndef TH_AND_P
#define TH_AND_P

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

static const float Runiversal_K_molK    =   8314.462;                            // Universal Gaz Constant       ( K/molK)
static const float MoleightA_kg_mol     =     28.9644;                           // Air Molecular weight         (Kg/mol )
static const float MoleightS_kg_mol     =     18.01528;                          // Steam Molecular weight       (Kg/mol )
static const float RAir_J_kg_K          = Runiversal_K_molK / MoleightA_kg_mol;  // Ideal Gaz Constant R         ( J/kgK )
static const float RWat_J_kg_K          = Runiversal_K_molK / MoleightS_kg_mol;  // Ideal Gaz Constant R         ( J/kgK )
static const float BMP280TCalib_XX      =     -0.85;                             // calibration of the BMP280
static const float SEALEVELPRESSURE_HPA =   1013.25;                             // ISA conditions sea level pressure
static const float C2K                  =    273.15;                             // K for 0°C
static const float B2PA                 = 100000.0;                              // Pascals for a Bar
static const float PSI2PA               =   6894.75729;                          // Pascals for PSI

float get_air_density(    float,                  float);
float calcHeatIndex(      float,                  float);
float calcRoA(       float P_Pa, float T_K             );   // kg/m^3 Air   Density Ideal Gas
float calcRoS(       float P_Pa, float T_K             );   // kg/m^3 Steam Density Ideal Gas
float PSvapour(                  float T_K             );   // Pa     Saturation vapour pressure for steam based on buck equation
float Pvapour(                   float T_K, float RH   );   // Pa     Partial pressure for warter vapour based on Relative humidity
float PAir(          float P_Pa, float T_K, float RH   );   // Pa     Dry Air Partiual pressure 
float WContent(                  float T_K, float RH   );   // kg/m^3 Water vapour content (Vapor concentration)
float AContent(      float P_Pa, float T_K, float RH   );   // kg/m^3 Dry Air content 
float calcSH(        float P_Pa, float T_K, float RH   );   // kg/kg  Specific humidity ratio of vapour over total 
float calcMixR(      float P_Pa, float T_K, float RH   );   // kg/kg  Mixing ratio
float cpA(                       float T_K             );   // J/kgK  Cp Correlation for dry air 
float cpS(                       float T_K             );   // J/kgK  Cp Correlation for dry steam 
float gammaA(                    float T_K             );   //        Heat capacity ratio of Air   Ideal Gas
float gammaS(                    float T_K             );   //        Heat capacity ratio of Steam Ideal Gas
float gamma(         float P_Pa, float T_K, float RH   );   //        Heat capacity ratio Ideal Gas
float calcDewPoint(              float T_K, float RH   );   // K      Dew Point Temperature estimation based on Buck equation
float calcFrostPoint(            float T_K, float Tdp_K);   // K      Frost point temperature estimation from: https://gist.github.com/sourceperl/45587ea99ff123745428





#endif //TH_AND_P


