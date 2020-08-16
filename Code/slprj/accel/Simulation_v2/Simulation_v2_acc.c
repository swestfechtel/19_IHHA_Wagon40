#include <math.h>
#include "Simulation_v2_acc.h"
#include "Simulation_v2_acc_private.h"
#include <stdio.h>
#include "slexec_vm_simstruct_bridge.h"
#include "slexec_vm_zc_functions.h"
#include "slexec_vm_lookup_functions.h"
#include "slsv_diagnostic_codegen_c_api.h"
#include "simtarget/slSimTgtMdlrefSfcnBridge.h"
#include "simstruc.h"
#include "fixedpoint.h"
#define CodeFormat S-Function
#define AccDefine1 Accelerator_S-Function
#include "simtarget/slAccSfcnBridge.h"
#ifndef __RTW_UTFREE__  
extern void * utMalloc ( size_t ) ; extern void utFree ( void * ) ;
#endif
boolean_T Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( int_T * bufSzPtr ,
int_T * tailPtr , int_T * headPtr , int_T * lastPtr , real_T tMinusDelay ,
real_T * * tBufPtr , real_T * * uBufPtr , real_T * * xBufPtr , boolean_T
isfixedbuf , boolean_T istransportdelay , int_T * maxNewBufSzPtr ) { int_T
testIdx ; int_T tail = * tailPtr ; int_T bufSz = * bufSzPtr ; real_T * tBuf =
* tBufPtr ; real_T * xBuf = ( NULL ) ; int_T numBuffer = 2 ; if (
istransportdelay ) { numBuffer = 3 ; xBuf = * xBufPtr ; } testIdx = ( tail <
( bufSz - 1 ) ) ? ( tail + 1 ) : 0 ; if ( ( tMinusDelay <= tBuf [ testIdx ] )
&& ! isfixedbuf ) { int_T j ; real_T * tempT ; real_T * tempU ; real_T *
tempX = ( NULL ) ; real_T * uBuf = * uBufPtr ; int_T newBufSz = bufSz + 1024
; if ( newBufSz > * maxNewBufSzPtr ) { * maxNewBufSzPtr = newBufSz ; } tempU
= ( real_T * ) utMalloc ( numBuffer * newBufSz * sizeof ( real_T ) ) ; if (
tempU == ( NULL ) ) { return ( false ) ; } tempT = tempU + newBufSz ; if (
istransportdelay ) tempX = tempT + newBufSz ; for ( j = tail ; j < bufSz ; j
++ ) { tempT [ j - tail ] = tBuf [ j ] ; tempU [ j - tail ] = uBuf [ j ] ; if
( istransportdelay ) tempX [ j - tail ] = xBuf [ j ] ; } for ( j = 0 ; j <
tail ; j ++ ) { tempT [ j + bufSz - tail ] = tBuf [ j ] ; tempU [ j + bufSz -
tail ] = uBuf [ j ] ; if ( istransportdelay ) tempX [ j + bufSz - tail ] =
xBuf [ j ] ; } if ( * lastPtr > tail ) { * lastPtr -= tail ; } else { *
lastPtr += ( bufSz - tail ) ; } * tailPtr = 0 ; * headPtr = bufSz ; utFree (
uBuf ) ; * bufSzPtr = newBufSz ; * tBufPtr = tempT ; * uBufPtr = tempU ; if (
istransportdelay ) * xBufPtr = tempX ; } else { * tailPtr = testIdx ; }
return ( true ) ; } real_T Simulation_v2_acc_rt_TDelayInterpolate ( real_T
tMinusDelay , real_T tStart , real_T * tBuf , real_T * uBuf , int_T bufSz ,
int_T * lastIdx , int_T oldestIdx , int_T newIdx , real_T initOutput ,
boolean_T discrete , boolean_T minorStepAndTAtLastMajorOutput ) { int_T i ;
real_T yout , t1 , t2 , u1 , u2 ; if ( ( newIdx == 0 ) && ( oldestIdx == 0 )
&& ( tMinusDelay > tStart ) ) return initOutput ; if ( tMinusDelay <= tStart
) return initOutput ; if ( ( tMinusDelay <= tBuf [ oldestIdx ] ) ) { if (
discrete ) { return ( uBuf [ oldestIdx ] ) ; } else { int_T tempIdx =
oldestIdx + 1 ; if ( oldestIdx == bufSz - 1 ) tempIdx = 0 ; t1 = tBuf [
oldestIdx ] ; t2 = tBuf [ tempIdx ] ; u1 = uBuf [ oldestIdx ] ; u2 = uBuf [
tempIdx ] ; if ( t2 == t1 ) { if ( tMinusDelay >= t2 ) { yout = u2 ; } else {
yout = u1 ; } } else { real_T f1 = ( t2 - tMinusDelay ) / ( t2 - t1 ) ;
real_T f2 = 1.0 - f1 ; yout = f1 * u1 + f2 * u2 ; } return yout ; } } if (
minorStepAndTAtLastMajorOutput ) { if ( newIdx != 0 ) { if ( * lastIdx ==
newIdx ) { ( * lastIdx ) -- ; } newIdx -- ; } else { if ( * lastIdx == newIdx
) { * lastIdx = bufSz - 1 ; } newIdx = bufSz - 1 ; } } i = * lastIdx ; if (
tBuf [ i ] < tMinusDelay ) { while ( tBuf [ i ] < tMinusDelay ) { if ( i ==
newIdx ) break ; i = ( i < ( bufSz - 1 ) ) ? ( i + 1 ) : 0 ; } } else { while
( tBuf [ i ] >= tMinusDelay ) { i = ( i > 0 ) ? i - 1 : ( bufSz - 1 ) ; } i =
( i < ( bufSz - 1 ) ) ? ( i + 1 ) : 0 ; } * lastIdx = i ; if ( discrete ) {
double tempEps = ( DBL_EPSILON ) * 128.0 ; double localEps = tempEps *
muDoubleScalarAbs ( tBuf [ i ] ) ; if ( tempEps > localEps ) { localEps =
tempEps ; } localEps = localEps / 2.0 ; if ( tMinusDelay >= ( tBuf [ i ] -
localEps ) ) { yout = uBuf [ i ] ; } else { if ( i == 0 ) { yout = uBuf [
bufSz - 1 ] ; } else { yout = uBuf [ i - 1 ] ; } } } else { if ( i == 0 ) {
t1 = tBuf [ bufSz - 1 ] ; u1 = uBuf [ bufSz - 1 ] ; } else { t1 = tBuf [ i -
1 ] ; u1 = uBuf [ i - 1 ] ; } t2 = tBuf [ i ] ; u2 = uBuf [ i ] ; if ( t2 ==
t1 ) { if ( tMinusDelay >= t2 ) { yout = u2 ; } else { yout = u1 ; } } else {
real_T f1 = ( t2 - tMinusDelay ) / ( t2 - t1 ) ; real_T f2 = 1.0 - f1 ; yout
= f1 * u1 + f2 * u2 ; } } return ( yout ) ; } real_T look1_binlxpw ( real_T
u0 , const real_T bp0 [ ] , const real_T table [ ] , uint32_T maxIndex ) {
real_T frac ; uint32_T iRght ; uint32_T iLeft ; uint32_T bpIdx ; if ( u0 <=
bp0 [ 0U ] ) { iLeft = 0U ; frac = ( u0 - bp0 [ 0U ] ) / ( bp0 [ 1U ] - bp0 [
0U ] ) ; } else if ( u0 < bp0 [ maxIndex ] ) { bpIdx = maxIndex >> 1U ; iLeft
= 0U ; iRght = maxIndex ; while ( iRght - iLeft > 1U ) { if ( u0 < bp0 [
bpIdx ] ) { iRght = bpIdx ; } else { iLeft = bpIdx ; } bpIdx = ( iRght +
iLeft ) >> 1U ; } frac = ( u0 - bp0 [ iLeft ] ) / ( bp0 [ iLeft + 1U ] - bp0
[ iLeft ] ) ; } else { iLeft = maxIndex - 1U ; frac = ( u0 - bp0 [ maxIndex -
1U ] ) / ( bp0 [ maxIndex ] - bp0 [ maxIndex - 1U ] ) ; } return ( table [
iLeft + 1U ] - table [ iLeft ] ) * frac + table [ iLeft ] ; } void
rt_ssGetBlockPath ( SimStruct * S , int_T sysIdx , int_T blkIdx , char_T * *
path ) { _ssGetBlockPath ( S , sysIdx , blkIdx , path ) ; } void
rt_ssSet_slErrMsg ( SimStruct * S , void * diag ) { if ( !
_ssIsErrorStatusAslErrMsg ( S ) ) { _ssSet_slErrMsg ( S , diag ) ; } else {
_ssDiscardDiagnostic ( S , diag ) ; } } void rt_ssReportDiagnosticAsWarning (
SimStruct * S , void * diag ) { _ssReportDiagnosticAsWarning ( S , diag ) ; }
static void mdlOutputs ( SimStruct * S , int_T tid ) { real_T deltaT ; real_T
rateLimiterRate ; real_T riseValLimit ; boolean_T rtb_B_120_4_0 ; real_T
rtb_B_120_5_0 ; int32_T isHit ; void * diag ; real_T tmp ; real_T tmp_0 ;
real_T tmp_1 ; real_T tmp_2 ; real_T tmp_3 ; real_T tmp_4 ; real_T tmp_5 ;
real_T tmp_6 ; real_T tmp_7 ; real_T tmp_8 ; real_T tmp_9 ; real_T tmp_a ;
real_T tmp_b ; real_T tmp_c ; real_T tmp_d ; real_T tmp_e ; real_T tmp_f ;
real_T tmp_g ; real_T tmp_h ; real_T tmp_i ; real_T tmp_j ; real_T tmp_k ;
real_T tmp_l ; real_T tmp_m ; real_T tmp_n ; real_T tmp_o ; real_T tmp_p ;
real_T tmp_q ; real_T tmp_r ; real_T tmp_s ; real_T tmp_t ; real_T tmp_u ;
real_T tmp_v ; real_T tmp_w ; real_T tmp_x ; real_T tmp_y ; B_Simulation_v2_T
* _rtB ; P_Simulation_v2_T * _rtP ; X_Simulation_v2_T * _rtX ;
DW_Simulation_v2_T * _rtDW ; _rtDW = ( ( DW_Simulation_v2_T * )
ssGetRootDWork ( S ) ) ; _rtX = ( ( X_Simulation_v2_T * ) ssGetContStates ( S
) ) ; _rtP = ( ( P_Simulation_v2_T * ) ssGetModelRtp ( S ) ) ; _rtB = ( (
B_Simulation_v2_T * ) _ssGetModelBlockIO ( S ) ) ; vm_ReadLocalDSMNoIdx ( S ,
_rtDW -> dsmIdx , ( char_T * ) "Simulation_v2/Subsystem/Data Store Read" , 0
) ; _rtB -> B_120_0_0 = _rtDW -> v0 ; if ( _rtDW -> Integrator_IWORK != 0 ) {
_rtX -> Integrator_CSTATE = _rtB -> B_120_0_0 ; } _rtB -> B_120_1_0 = _rtX ->
Integrator_CSTATE ; ssCallAccelRunBlock ( S , 120 , 2 , SS_CALL_MDL_OUTPUTS )
; isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB -> B_120_3_0
= _rtDW -> DelayInput1_DSTATE ; } rtb_B_120_4_0 = ( _rtB -> B_120_2_0 < _rtB
-> B_120_3_0 ) ; rtb_B_120_5_0 = _rtB -> B_120_1_0 * ( real_T ) rtb_B_120_4_0
; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Calculation of braking pressure/Data Store Read" , 0 ) ; _rtB
-> B_120_9_0 = ( real_T ) ! rtb_B_120_4_0 * _rtDW -> v0 + rtb_B_120_5_0 ;
_rtDW -> v0 = _rtB -> B_120_9_0 ; vm_WriteLocalDSMNoIdx ( S , _rtDW -> dsmIdx
, ( char_T * )
"Simulation_v2/Calculation of braking pressure/Data Store Write" , 0 ) ;
ssCallAccelRunBlock ( S , 120 , 11 , SS_CALL_MDL_OUTPUTS ) ; _rtB ->
B_120_12_0 = _rtB -> B_120_1_0 - _rtB -> B_120_2_0 ; _rtB -> B_120_15_0 = (
real_T ) ( _rtB -> B_120_1_0 > _rtB -> B_120_2_0 ) * look1_binlxpw ( _rtB ->
B_120_12_0 , _rtP -> P_84 , _rtP -> P_83 , 7U ) ; _rtB -> B_120_16_0 = _rtB
-> B_120_0_0_m - _rtB -> B_120_15_0 ; _rtB -> B_120_17_0 = _rtB -> B_120_16_0
+ _rtP -> P_85 ; _rtB -> B_120_18_0 = _rtP -> P_86 * _rtB -> B_120_12_0 ;
isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { ssCallAccelRunBlock
( S , 13 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP -> P_976 != 0 ) && ( ! (
_rtB -> B_13_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_976 == 1 ) { if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings == 0U ) { diag = CreateDiagnosticAsVoidPtr
( "Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model5/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model5/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_35_0_d ) ; if ( ( _rtP -> P_976 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_976 == 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings == 0U ) { diag
= CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model5/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model5/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_35_0_d ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_35_0_d ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_20_0 =
_rtB -> B_120_34_0_j [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_13_0_0 , 499U ) ] ; _rtB -> B_120_21_0 =
_rtB -> B_120_41_0 * _rtB -> B_120_20_0 ; if ( ( _rtP -> P_977 != 0 ) && ( !
( _rtB -> B_13_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_977 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model5/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model5/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_36_0 ) ; if ( ( _rtP -> P_977 != 0 ) && ( ( ! ( rtb_B_120_5_0
>= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_977 == 1 )
{ if ( _rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings == 0U ) { diag
= CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model5/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model5/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_36_0 ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_36_0 ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else { if
( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_22_0 = _rtB
-> B_120_34_0_j [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_13_0_0 , 499U ) ] ; } _rtB -> B_120_23_0 =
_rtP -> P_87 * _rtB -> B_120_17_0 ; if ( _rtB -> B_120_23_0 > _rtP -> P_88 )
{ _rtB -> B_120_24_0 = _rtP -> P_88 ; } else if ( _rtB -> B_120_23_0 < _rtP
-> P_89 ) { _rtB -> B_120_24_0 = _rtP -> P_89 ; } else { _rtB -> B_120_24_0 =
_rtB -> B_120_23_0 ; } if ( ( _rtDW -> LastMajorTimeA >= ssGetTaskTime ( S ,
0 ) ) && ( _rtDW -> LastMajorTimeB >= ssGetTaskTime ( S , 0 ) ) ) { _rtB ->
B_120_25_0 = _rtB -> B_120_24_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA <
_rtDW -> LastMajorTimeB ) && ( _rtDW -> LastMajorTimeB < ssGetTaskTime ( S ,
0 ) ) ) || ( ( _rtDW -> LastMajorTimeA >= _rtDW -> LastMajorTimeB ) && (
_rtDW -> LastMajorTimeA >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB ; rtb_B_120_5_0 = _rtDW ->
PrevYB ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA
; rtb_B_120_5_0 = _rtDW -> PrevYA ; } riseValLimit = deltaT * _rtP -> P_90 ;
rateLimiterRate = _rtB -> B_120_24_0 - rtb_B_120_5_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_120_25_0 = rtb_B_120_5_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_91 ; if ( rateLimiterRate < deltaT ) { _rtB -> B_120_25_0
= rtb_B_120_5_0 + deltaT ; } else { _rtB -> B_120_25_0 = _rtB -> B_120_24_0 ;
} } } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB ->
B_120_26_0 = _rtP -> P_92 * _rtB -> B_120_20_0 ; } _rtB -> B_120_27_0 = _rtB
-> B_120_25_0 * _rtB -> B_120_26_0 ; _rtB -> B_120_28_0 = _rtB -> B_120_22_0
* _rtB -> B_120_27_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T
* )
"Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model5/Data Store Read" ,
0 ) ; _rtB -> B_120_29_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK != 0 ) {
_rtX -> vv0at_CSTATE = _rtB -> B_120_29_0 ; } _rtB -> B_120_30_0 = _rtX ->
vv0at_CSTATE ; _rtB -> B_120_31_0 = muDoubleScalarAbs ( _rtB -> B_120_30_0 )
; _rtB -> B_120_32_0 = _rtP -> P_93 [ 0 ] ; _rtB -> B_120_32_0 = _rtB ->
B_120_32_0 * _rtB -> B_120_31_0 + _rtP -> P_93 [ 1 ] ; _rtB -> B_120_32_0 =
_rtB -> B_120_32_0 * _rtB -> B_120_31_0 + _rtP -> P_93 [ 2 ] ; _rtB ->
B_120_33_0 = _rtP -> P_94 * _rtB -> B_120_32_0 ; _rtB -> B_120_34_0 = _rtB ->
B_120_20_0 * _rtB -> B_120_33_0 ; _rtB -> B_120_35_0 = _rtB -> B_120_28_0 -
_rtB -> B_120_34_0 ; isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 )
{ _rtB -> B_120_37_0 = _rtP -> P_95 * _rtB -> B_120_21_0 ; } if ( _rtB ->
B_120_35_0 > _rtB -> B_120_21_0 ) { _rtB -> B_120_39_0 = _rtB -> B_120_21_0 ;
} else { if ( _rtB -> B_120_35_0 < _rtB -> B_120_37_0 ) { _rtB -> B_14_1_0 =
_rtB -> B_120_37_0 ; } else { _rtB -> B_14_1_0 = _rtB -> B_120_35_0 ; } _rtB
-> B_120_39_0 = _rtB -> B_14_1_0 ; } if ( _rtB -> B_120_40_0_g ) { _rtB ->
B_120_40_0 = _rtB -> B_120_39_0 ; } else { _rtB -> B_120_40_0 = _rtB ->
B_120_39_0_f ; } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) {
ssCallAccelRunBlock ( S , 7 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP ->
P_978 != 0 ) && ( ! ( _rtB -> B_7_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_978 ==
1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_a == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model3/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_a < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_a ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model3/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_19_0 ) ; if ( ( _rtP -> P_978 != 0 ) && ( ( ! ( rtb_B_120_5_0
>= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_978 == 1 )
{ if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_a == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model3/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_a < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_a ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model3/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_19_0 ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_19_0 ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else { if
( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_42_0 = _rtB
-> B_120_18_0_f [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_7_0_0 , 499U ) ] ; _rtB -> B_120_43_0 =
_rtB -> B_120_25_0_g * _rtB -> B_120_42_0 ; if ( ( _rtP -> P_979 != 0 ) && (
! ( _rtB -> B_7_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_979 == 1 ) { if ( _rtDW
-> lookupwagonbrakingefficiency_NumIssuedWarnings_j == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_j < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_j ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_20_0_g ) ; if ( ( _rtP -> P_979 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_979 == 1 ) { if ( _rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_j
== 0U ) { diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_j < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_j ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_20_0_g ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_20_0_g ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_44_0 =
_rtB -> B_120_18_0_f [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_7_0_0 , 499U ) ] ; } { real_T * * uBuffer =
( real_T * * ) & _rtDW -> TransportDelay_PWORK . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK . TUbufferPtrs [
1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay = simTime - _rtP ->
P_96 ; _rtB -> B_120_45_0 = Simulation_v2_acc_rt_TDelayInterpolate (
tMinusDelay , 0.0 , * tBuffer , * uBuffer , _rtDW -> TransportDelay_IWORK .
CircularBufSize , & _rtDW -> TransportDelay_IWORK . Last , _rtDW ->
TransportDelay_IWORK . Tail , _rtDW -> TransportDelay_IWORK . Head , _rtP ->
P_97 , 0 , ( boolean_T ) ( ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput
( S ) == ssGetT ( S ) ) ) ) ; } _rtB -> B_120_46_0 = 0.0 ; _rtB -> B_120_46_0
+= _rtP -> P_99 * _rtX -> TransferFcn_CSTATE ; _rtB -> B_120_46_0 += _rtP ->
P_100 * _rtB -> B_120_45_0 ; if ( _rtB -> B_120_40_0_g ) { if ( _rtB ->
B_120_46_0 > _rtP -> P_9 ) { _rtB -> B_12_0_0 = _rtB -> B_120_46_0 - _rtP ->
P_9 ; } else if ( _rtB -> B_120_46_0 >= _rtP -> P_8 ) { _rtB -> B_12_0_0 =
0.0 ; } else { _rtB -> B_12_0_0 = _rtB -> B_120_46_0 - _rtP -> P_8 ; } _rtB
-> B_120_48_0 = _rtB -> B_12_0_0 ; } else { _rtB -> B_120_48_0 = _rtB ->
B_120_39_0_f ; } _rtB -> B_120_49_0 = _rtP -> P_101 * _rtB -> B_120_48_0 ; if
( _rtB -> B_120_49_0 > _rtP -> P_102 ) { _rtB -> B_120_50_0 = _rtP -> P_102 ;
} else if ( _rtB -> B_120_49_0 < _rtP -> P_103 ) { _rtB -> B_120_50_0 = _rtP
-> P_103 ; } else { _rtB -> B_120_50_0 = _rtB -> B_120_49_0 ; } if ( ( _rtDW
-> LastMajorTimeA_b >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW ->
LastMajorTimeB_j >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_120_51_0 = _rtB
-> B_120_50_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_b < _rtDW ->
LastMajorTimeB_j ) && ( _rtDW -> LastMajorTimeB_j < ssGetTaskTime ( S , 0 ) )
) || ( ( _rtDW -> LastMajorTimeA_b >= _rtDW -> LastMajorTimeB_j ) && ( _rtDW
-> LastMajorTimeA_b >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime
( S , 0 ) - _rtDW -> LastMajorTimeB_j ; rtb_B_120_5_0 = _rtDW -> PrevYB_b ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_b ;
rtb_B_120_5_0 = _rtDW -> PrevYA_g ; } riseValLimit = deltaT * _rtP -> P_104 ;
rateLimiterRate = _rtB -> B_120_50_0 - rtb_B_120_5_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_120_51_0 = rtb_B_120_5_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_105 ; if ( rateLimiterRate < deltaT ) { _rtB ->
B_120_51_0 = rtb_B_120_5_0 + deltaT ; } else { _rtB -> B_120_51_0 = _rtB ->
B_120_50_0 ; } } } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) {
_rtB -> B_120_52_0 = _rtP -> P_106 * _rtB -> B_120_42_0 ; } _rtB ->
B_120_53_0 = _rtB -> B_120_51_0 * _rtB -> B_120_52_0 ; _rtB -> B_120_54_0 =
_rtB -> B_120_44_0 * _rtB -> B_120_53_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW ->
dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model3/Data Store Read" ,
0 ) ; _rtB -> B_120_55_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_b != 0 ) {
_rtX -> vv0at_CSTATE_k = _rtB -> B_120_55_0 ; } _rtB -> B_120_56_0 = _rtX ->
vv0at_CSTATE_k ; _rtB -> B_120_57_0 = muDoubleScalarAbs ( _rtB -> B_120_56_0
) ; _rtB -> B_120_58_0 = _rtP -> P_107 [ 0 ] ; _rtB -> B_120_58_0 = _rtB ->
B_120_58_0 * _rtB -> B_120_57_0 + _rtP -> P_107 [ 1 ] ; _rtB -> B_120_58_0 =
_rtB -> B_120_58_0 * _rtB -> B_120_57_0 + _rtP -> P_107 [ 2 ] ; _rtB ->
B_120_59_0 = _rtP -> P_108 * _rtB -> B_120_58_0 ; _rtB -> B_120_60_0 = _rtB
-> B_120_42_0 * _rtB -> B_120_59_0 ; _rtB -> B_120_61_0 = _rtB -> B_120_54_0
- _rtB -> B_120_60_0 ; isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0
) { _rtB -> B_120_63_0 = _rtP -> P_109 * _rtB -> B_120_43_0 ; } if ( _rtB ->
B_120_61_0 > _rtB -> B_120_43_0 ) { _rtB -> B_120_65_0 = _rtB -> B_120_43_0 ;
} else { if ( _rtB -> B_120_61_0 < _rtB -> B_120_63_0 ) { _rtB -> B_8_1_0 =
_rtB -> B_120_63_0 ; } else { _rtB -> B_8_1_0 = _rtB -> B_120_61_0 ; } _rtB
-> B_120_65_0 = _rtB -> B_8_1_0 ; } if ( _rtB -> B_120_24_0_d ) { _rtB ->
B_120_66_0 = _rtB -> B_120_65_0 ; } else { _rtB -> B_120_66_0 = _rtB ->
B_120_23_0_n ; } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) {
ssCallAccelRunBlock ( S , 4 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP ->
P_980 != 0 ) && ( ! ( _rtB -> B_4_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_980 ==
1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_n == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model2/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_n < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_n ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model2/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_11_0 ) ; if ( ( _rtP -> P_980 != 0 ) && ( ( ! ( rtb_B_120_5_0
>= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_980 == 1 )
{ if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_n == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model2/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_n < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_n ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model2/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_11_0 ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_11_0 ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else { if
( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_68_0 = _rtB
-> B_120_10_0 [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_4_0_0 , 499U ) ] ; _rtB -> B_120_69_0 =
_rtB -> B_120_17_0_c * _rtB -> B_120_68_0 ; if ( ( _rtP -> P_981 != 0 ) && (
! ( _rtB -> B_4_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_981 == 1 ) { if ( _rtDW
-> lookupwagonbrakingefficiency_NumIssuedWarnings_b == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_b < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_b ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_12_0_p ) ; if ( ( _rtP -> P_981 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_981 == 1 ) { if ( _rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_b
== 0U ) { diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_b < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_b ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_12_0_p ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_12_0_p ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_70_0 =
_rtB -> B_120_10_0 [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_4_0_0 , 499U ) ] ; } { real_T * * uBuffer =
( real_T * * ) & _rtDW -> TransportDelay_PWORK_a . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_a .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_110 ; _rtB -> B_120_71_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_f . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_f . Last , _rtDW -> TransportDelay_IWORK_f . Tail ,
_rtDW -> TransportDelay_IWORK_f . Head , _rtP -> P_111 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_120_72_0 = 0.0 ; _rtB -> B_120_72_0 += _rtP -> P_113 * _rtX
-> TransferFcn_CSTATE_m ; _rtB -> B_120_72_0 += _rtP -> P_114 * _rtB ->
B_120_71_0 ; if ( _rtB -> B_120_24_0_d ) { if ( _rtB -> B_120_72_0 > _rtP ->
P_5 ) { _rtB -> B_6_0_0 = _rtB -> B_120_72_0 - _rtP -> P_5 ; } else if ( _rtB
-> B_120_72_0 >= _rtP -> P_4 ) { _rtB -> B_6_0_0 = 0.0 ; } else { _rtB ->
B_6_0_0 = _rtB -> B_120_72_0 - _rtP -> P_4 ; } _rtB -> B_120_74_0 = _rtB ->
B_6_0_0 ; } else { _rtB -> B_120_74_0 = _rtB -> B_120_23_0_n ; } _rtB ->
B_120_75_0 = _rtP -> P_115 * _rtB -> B_120_74_0 ; if ( _rtB -> B_120_75_0 >
_rtP -> P_116 ) { _rtB -> B_120_76_0 = _rtP -> P_116 ; } else if ( _rtB ->
B_120_75_0 < _rtP -> P_117 ) { _rtB -> B_120_76_0 = _rtP -> P_117 ; } else {
_rtB -> B_120_76_0 = _rtB -> B_120_75_0 ; } if ( ( _rtDW -> LastMajorTimeA_i
>= ssGetTaskTime ( S , 0 ) ) && ( _rtDW -> LastMajorTimeB_m >= ssGetTaskTime
( S , 0 ) ) ) { _rtB -> B_120_77_0 = _rtB -> B_120_76_0 ; } else { if ( ( (
_rtDW -> LastMajorTimeA_i < _rtDW -> LastMajorTimeB_m ) && ( _rtDW ->
LastMajorTimeB_m < ssGetTaskTime ( S , 0 ) ) ) || ( ( _rtDW ->
LastMajorTimeA_i >= _rtDW -> LastMajorTimeB_m ) && ( _rtDW ->
LastMajorTimeA_i >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime (
S , 0 ) - _rtDW -> LastMajorTimeB_m ; rtb_B_120_5_0 = _rtDW -> PrevYB_l ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_i ;
rtb_B_120_5_0 = _rtDW -> PrevYA_e ; } riseValLimit = deltaT * _rtP -> P_118 ;
rateLimiterRate = _rtB -> B_120_76_0 - rtb_B_120_5_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_120_77_0 = rtb_B_120_5_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_119 ; if ( rateLimiterRate < deltaT ) { _rtB ->
B_120_77_0 = rtb_B_120_5_0 + deltaT ; } else { _rtB -> B_120_77_0 = _rtB ->
B_120_76_0 ; } } } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) {
_rtB -> B_120_78_0 = _rtP -> P_120 * _rtB -> B_120_68_0 ; } _rtB ->
B_120_79_0 = _rtB -> B_120_77_0 * _rtB -> B_120_78_0 ; _rtB -> B_120_80_0 =
_rtB -> B_120_70_0 * _rtB -> B_120_79_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW ->
dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model2/Data Store Read" ,
0 ) ; _rtB -> B_120_81_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_h != 0 ) {
_rtX -> vv0at_CSTATE_m = _rtB -> B_120_81_0 ; } _rtB -> B_120_82_0 = _rtX ->
vv0at_CSTATE_m ; _rtB -> B_120_83_0 = muDoubleScalarAbs ( _rtB -> B_120_82_0
) ; _rtB -> B_120_84_0 = _rtP -> P_121 [ 0 ] ; _rtB -> B_120_84_0 = _rtB ->
B_120_84_0 * _rtB -> B_120_83_0 + _rtP -> P_121 [ 1 ] ; _rtB -> B_120_84_0 =
_rtB -> B_120_84_0 * _rtB -> B_120_83_0 + _rtP -> P_121 [ 2 ] ; _rtB ->
B_120_85_0 = _rtP -> P_122 * _rtB -> B_120_84_0 ; _rtB -> B_120_86_0 = _rtB
-> B_120_68_0 * _rtB -> B_120_85_0 ; _rtB -> B_120_87_0 = _rtB -> B_120_80_0
- _rtB -> B_120_86_0 ; isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0
) { _rtB -> B_120_89_0 = _rtP -> P_123 * _rtB -> B_120_69_0 ; } if ( _rtB ->
B_120_87_0 > _rtB -> B_120_69_0 ) { _rtB -> B_120_91_0 = _rtB -> B_120_69_0 ;
} else { if ( _rtB -> B_120_87_0 < _rtB -> B_120_89_0 ) { _rtB -> B_5_1_0 =
_rtB -> B_120_89_0 ; } else { _rtB -> B_5_1_0 = _rtB -> B_120_87_0 ; } _rtB
-> B_120_91_0 = _rtB -> B_5_1_0 ; } if ( _rtB -> B_120_16_0_e ) { _rtB ->
B_120_92_0 = _rtB -> B_120_91_0 ; } else { _rtB -> B_120_92_0 = _rtB ->
B_120_15_0_d ; } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) {
ssCallAccelRunBlock ( S , 1 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP ->
P_982 != 0 ) && ( ! ( _rtB -> B_1_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_982 ==
1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_av == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model1/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_av < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_av ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model1/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_3_0_c ) ; if ( ( _rtP -> P_982 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_982 == 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_av == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model1/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_av < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_av ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model1/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_3_0_c ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_3_0_c ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_94_0 =
_rtB -> B_120_2_0_k [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_1_0_0 , 499U ) ] ; _rtB -> B_120_95_0 =
_rtB -> B_120_9_0_b * _rtB -> B_120_94_0 ; if ( ( _rtP -> P_983 != 0 ) && ( !
( _rtB -> B_1_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_983 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_l == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_l < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_l ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_4_0 ) ; if ( ( _rtP -> P_983 != 0 ) && ( ( ! ( rtb_B_120_5_0
>= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_983 == 1 )
{ if ( _rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_l == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_l < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_l ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_4_0 ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_4_0 ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else { if
( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_96_0 = _rtB
-> B_120_2_0_k [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_1_0_0 , 499U ) ] ; } { real_T * * uBuffer =
( real_T * * ) & _rtDW -> TransportDelay_PWORK_o . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_o .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_124 ; _rtB -> B_120_97_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_i . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_i . Last , _rtDW -> TransportDelay_IWORK_i . Tail ,
_rtDW -> TransportDelay_IWORK_i . Head , _rtP -> P_125 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_120_98_0 = 0.0 ; _rtB -> B_120_98_0 += _rtP -> P_127 * _rtX
-> TransferFcn_CSTATE_h ; _rtB -> B_120_98_0 += _rtP -> P_128 * _rtB ->
B_120_97_0 ; if ( _rtB -> B_120_16_0_e ) { if ( _rtB -> B_120_98_0 > _rtP ->
P_3 ) { _rtB -> B_3_0_0 = _rtB -> B_120_98_0 - _rtP -> P_3 ; } else if ( _rtB
-> B_120_98_0 >= _rtP -> P_2 ) { _rtB -> B_3_0_0 = 0.0 ; } else { _rtB ->
B_3_0_0 = _rtB -> B_120_98_0 - _rtP -> P_2 ; } _rtB -> B_120_100_0 = _rtB ->
B_3_0_0 ; } else { _rtB -> B_120_100_0 = _rtB -> B_120_15_0_d ; } _rtB ->
B_120_101_0 = _rtP -> P_129 * _rtB -> B_120_100_0 ; if ( _rtB -> B_120_101_0
> _rtP -> P_130 ) { _rtB -> B_120_102_0 = _rtP -> P_130 ; } else if ( _rtB ->
B_120_101_0 < _rtP -> P_131 ) { _rtB -> B_120_102_0 = _rtP -> P_131 ; } else
{ _rtB -> B_120_102_0 = _rtB -> B_120_101_0 ; } if ( ( _rtDW ->
LastMajorTimeA_e >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW -> LastMajorTimeB_p
>= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_120_103_0 = _rtB -> B_120_102_0 ;
} else { if ( ( ( _rtDW -> LastMajorTimeA_e < _rtDW -> LastMajorTimeB_p ) &&
( _rtDW -> LastMajorTimeB_p < ssGetTaskTime ( S , 0 ) ) ) || ( ( _rtDW ->
LastMajorTimeA_e >= _rtDW -> LastMajorTimeB_p ) && ( _rtDW ->
LastMajorTimeA_e >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime (
S , 0 ) - _rtDW -> LastMajorTimeB_p ; rtb_B_120_5_0 = _rtDW -> PrevYB_b0 ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_e ;
rtb_B_120_5_0 = _rtDW -> PrevYA_j ; } riseValLimit = deltaT * _rtP -> P_132 ;
rateLimiterRate = _rtB -> B_120_102_0 - rtb_B_120_5_0 ; if ( rateLimiterRate
> riseValLimit ) { _rtB -> B_120_103_0 = rtb_B_120_5_0 + riseValLimit ; }
else { deltaT *= _rtP -> P_133 ; if ( rateLimiterRate < deltaT ) { _rtB ->
B_120_103_0 = rtb_B_120_5_0 + deltaT ; } else { _rtB -> B_120_103_0 = _rtB ->
B_120_102_0 ; } } } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) {
_rtB -> B_120_104_0 = _rtP -> P_134 * _rtB -> B_120_94_0 ; } _rtB ->
B_120_105_0 = _rtB -> B_120_103_0 * _rtB -> B_120_104_0 ; _rtB -> B_120_106_0
= _rtB -> B_120_96_0 * _rtB -> B_120_105_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW
-> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model1/Data Store Read" ,
0 ) ; _rtB -> B_120_107_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_e != 0 )
{ _rtX -> vv0at_CSTATE_e = _rtB -> B_120_107_0 ; } _rtB -> B_120_108_0 = _rtX
-> vv0at_CSTATE_e ; _rtB -> B_120_109_0 = muDoubleScalarAbs ( _rtB ->
B_120_108_0 ) ; _rtB -> B_120_110_0 = _rtP -> P_135 [ 0 ] ; _rtB ->
B_120_110_0 = _rtB -> B_120_110_0 * _rtB -> B_120_109_0 + _rtP -> P_135 [ 1 ]
; _rtB -> B_120_110_0 = _rtB -> B_120_110_0 * _rtB -> B_120_109_0 + _rtP ->
P_135 [ 2 ] ; _rtB -> B_120_111_0 = _rtP -> P_136 * _rtB -> B_120_110_0 ;
_rtB -> B_120_112_0 = _rtB -> B_120_94_0 * _rtB -> B_120_111_0 ; _rtB ->
B_120_113_0 = _rtB -> B_120_106_0 - _rtB -> B_120_112_0 ; isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB -> B_120_115_0 = _rtP
-> P_137 * _rtB -> B_120_95_0 ; } if ( _rtB -> B_120_113_0 > _rtB ->
B_120_95_0 ) { _rtB -> B_120_117_0 = _rtB -> B_120_95_0 ; } else { if ( _rtB
-> B_120_113_0 < _rtB -> B_120_115_0 ) { _rtB -> B_2_1_0 = _rtB ->
B_120_115_0 ; } else { _rtB -> B_2_1_0 = _rtB -> B_120_113_0 ; } _rtB ->
B_120_117_0 = _rtB -> B_2_1_0 ; } if ( _rtB -> B_120_8_0 ) { _rtB ->
B_120_118_0 = _rtB -> B_120_117_0 ; } else { _rtB -> B_120_118_0 = _rtB ->
B_120_7_0 ; } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) {
ssCallAccelRunBlock ( S , 10 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP ->
P_984 != 0 ) && ( ! ( _rtB -> B_10_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_984 ==
1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_h == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model4/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_h < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_h ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model4/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_27_0_n ) ; if ( ( _rtP -> P_984 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_984 == 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_h == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model4/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_h < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_h ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model4/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_27_0_n ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_27_0_n ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_120_0 =
_rtB -> B_120_26_0_m [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_10_0_0 , 499U ) ] ; _rtB -> B_120_121_0 =
_rtB -> B_120_33_0_l * _rtB -> B_120_120_0 ; if ( ( _rtP -> P_985 != 0 ) && (
! ( _rtB -> B_10_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_985 == 1 ) { if ( _rtDW
-> lookupwagonbrakingefficiency_NumIssuedWarnings_d == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_d < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_d ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_28_0_p ) ; if ( ( _rtP -> P_985 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_985 == 1 ) { if ( _rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_d
== 0U ) { diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_d < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_d ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_28_0_p ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_28_0_p ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_122_0 =
_rtB -> B_120_26_0_m [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_10_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_n . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_n .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_138 ; _rtB -> B_120_123_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_f2 . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_f2 . Last , _rtDW -> TransportDelay_IWORK_f2 . Tail ,
_rtDW -> TransportDelay_IWORK_f2 . Head , _rtP -> P_139 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_120_124_0 = 0.0 ; _rtB -> B_120_124_0 += _rtP -> P_141 * _rtX
-> TransferFcn_CSTATE_l ; _rtB -> B_120_124_0 += _rtP -> P_142 * _rtB ->
B_120_123_0 ; if ( _rtB -> B_120_8_0 ) { if ( _rtB -> B_120_124_0 > _rtP ->
P_1 ) { _rtB -> B_0_0_0 = _rtB -> B_120_124_0 - _rtP -> P_1 ; } else if (
_rtB -> B_120_124_0 >= _rtP -> P_0 ) { _rtB -> B_0_0_0 = 0.0 ; } else { _rtB
-> B_0_0_0 = _rtB -> B_120_124_0 - _rtP -> P_0 ; } _rtB -> B_120_126_0 = _rtB
-> B_0_0_0 ; } else { _rtB -> B_120_126_0 = _rtB -> B_120_7_0 ; } _rtB ->
B_120_127_0 = _rtP -> P_143 * _rtB -> B_120_126_0 ; if ( _rtB -> B_120_127_0
> _rtP -> P_144 ) { _rtB -> B_120_128_0 = _rtP -> P_144 ; } else if ( _rtB ->
B_120_127_0 < _rtP -> P_145 ) { _rtB -> B_120_128_0 = _rtP -> P_145 ; } else
{ _rtB -> B_120_128_0 = _rtB -> B_120_127_0 ; } if ( ( _rtDW ->
LastMajorTimeA_i2 >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW -> LastMajorTimeB_c
>= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_120_129_0 = _rtB -> B_120_128_0 ;
} else { if ( ( ( _rtDW -> LastMajorTimeA_i2 < _rtDW -> LastMajorTimeB_c ) &&
( _rtDW -> LastMajorTimeB_c < ssGetTaskTime ( S , 0 ) ) ) || ( ( _rtDW ->
LastMajorTimeA_i2 >= _rtDW -> LastMajorTimeB_c ) && ( _rtDW ->
LastMajorTimeA_i2 >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime (
S , 0 ) - _rtDW -> LastMajorTimeB_c ; rtb_B_120_5_0 = _rtDW -> PrevYB_f ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_i2 ;
rtb_B_120_5_0 = _rtDW -> PrevYA_h ; } riseValLimit = deltaT * _rtP -> P_146 ;
rateLimiterRate = _rtB -> B_120_128_0 - rtb_B_120_5_0 ; if ( rateLimiterRate
> riseValLimit ) { _rtB -> B_120_129_0 = rtb_B_120_5_0 + riseValLimit ; }
else { deltaT *= _rtP -> P_147 ; if ( rateLimiterRate < deltaT ) { _rtB ->
B_120_129_0 = rtb_B_120_5_0 + deltaT ; } else { _rtB -> B_120_129_0 = _rtB ->
B_120_128_0 ; } } } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) {
_rtB -> B_120_130_0 = _rtP -> P_148 * _rtB -> B_120_120_0 ; } _rtB ->
B_120_131_0 = _rtB -> B_120_129_0 * _rtB -> B_120_130_0 ; _rtB -> B_120_132_0
= _rtB -> B_120_122_0 * _rtB -> B_120_131_0 ; vm_ReadLocalDSMNoIdx ( S ,
_rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model4/Data Store Read" ,
0 ) ; _rtB -> B_120_133_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_c != 0 )
{ _rtX -> vv0at_CSTATE_d = _rtB -> B_120_133_0 ; } _rtB -> B_120_134_0 = _rtX
-> vv0at_CSTATE_d ; _rtB -> B_120_135_0 = muDoubleScalarAbs ( _rtB ->
B_120_134_0 ) ; _rtB -> B_120_136_0 = _rtP -> P_149 [ 0 ] ; _rtB ->
B_120_136_0 = _rtB -> B_120_136_0 * _rtB -> B_120_135_0 + _rtP -> P_149 [ 1 ]
; _rtB -> B_120_136_0 = _rtB -> B_120_136_0 * _rtB -> B_120_135_0 + _rtP ->
P_149 [ 2 ] ; _rtB -> B_120_137_0 = _rtP -> P_150 * _rtB -> B_120_136_0 ;
_rtB -> B_120_138_0 = _rtB -> B_120_120_0 * _rtB -> B_120_137_0 ; _rtB ->
B_120_139_0 = _rtB -> B_120_132_0 - _rtB -> B_120_138_0 ; isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB -> B_120_141_0 = _rtP
-> P_151 * _rtB -> B_120_121_0 ; } if ( _rtB -> B_120_139_0 > _rtB ->
B_120_121_0 ) { _rtB -> B_120_143_0 = _rtB -> B_120_121_0 ; } else { if (
_rtB -> B_120_139_0 < _rtB -> B_120_141_0 ) { _rtB -> B_11_1_0 = _rtB ->
B_120_141_0 ; } else { _rtB -> B_11_1_0 = _rtB -> B_120_139_0 ; } _rtB ->
B_120_143_0 = _rtB -> B_11_1_0 ; } if ( _rtB -> B_120_32_0_i ) { _rtB ->
B_120_144_0 = _rtB -> B_120_143_0 ; } else { _rtB -> B_120_144_0 = _rtB ->
B_120_31_0_c ; } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) {
ssCallAccelRunBlock ( S , 28 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP ->
P_986 != 0 ) && ( ! ( _rtB -> B_28_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_986 ==
1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_p == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model5/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_p < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_p ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model5/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_75_0_j ) ; if ( ( _rtP -> P_986 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_986 == 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_p == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model5/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_p < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_p ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model5/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_75_0_j ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_75_0_j ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_146_0 =
_rtB -> B_120_74_0_b [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_28_0_0 , 499U ) ] ; _rtB -> B_120_147_0 =
_rtB -> B_120_83_0_j * _rtB -> B_120_146_0 ; if ( ( _rtP -> P_987 != 0 ) && (
! ( _rtB -> B_28_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_987 == 1 ) { if ( _rtDW
-> lookupwagonbrakingefficiency_NumIssuedWarnings_k == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model5/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_k < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_k ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model5/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_76_0_f ) ; if ( ( _rtP -> P_987 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_987 == 1 ) { if ( _rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_k
== 0U ) { diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model5/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_k < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_k ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model5/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_76_0_f ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_76_0_f ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_148_0 =
_rtB -> B_120_74_0_b [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_28_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_m . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_m .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_152 ; _rtB -> B_120_149_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_k . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_k . Last , _rtDW -> TransportDelay_IWORK_k . Tail ,
_rtDW -> TransportDelay_IWORK_k . Head , _rtP -> P_153 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_120_150_0 = 0.0 ; _rtB -> B_120_150_0 += _rtP -> P_155 * _rtX
-> TransferFcn_CSTATE_k ; _rtB -> B_120_150_0 += _rtP -> P_156 * _rtB ->
B_120_149_0 ; if ( _rtB -> B_120_32_0_i ) { if ( _rtB -> B_120_150_0 > _rtP
-> P_7 ) { _rtB -> B_9_0_0 = _rtB -> B_120_150_0 - _rtP -> P_7 ; } else if (
_rtB -> B_120_150_0 >= _rtP -> P_6 ) { _rtB -> B_9_0_0 = 0.0 ; } else { _rtB
-> B_9_0_0 = _rtB -> B_120_150_0 - _rtP -> P_6 ; } _rtB -> B_120_152_0 = _rtB
-> B_9_0_0 ; } else { _rtB -> B_120_152_0 = _rtB -> B_120_31_0_c ; } _rtB ->
B_120_153_0 = _rtP -> P_157 * _rtB -> B_120_152_0 ; if ( _rtB -> B_120_153_0
> _rtP -> P_158 ) { _rtB -> B_120_154_0 = _rtP -> P_158 ; } else if ( _rtB ->
B_120_153_0 < _rtP -> P_159 ) { _rtB -> B_120_154_0 = _rtP -> P_159 ; } else
{ _rtB -> B_120_154_0 = _rtB -> B_120_153_0 ; } if ( ( _rtDW ->
LastMajorTimeA_c >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW -> LastMajorTimeB_mm
>= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_120_155_0 = _rtB -> B_120_154_0 ;
} else { if ( ( ( _rtDW -> LastMajorTimeA_c < _rtDW -> LastMajorTimeB_mm ) &&
( _rtDW -> LastMajorTimeB_mm < ssGetTaskTime ( S , 0 ) ) ) || ( ( _rtDW ->
LastMajorTimeA_c >= _rtDW -> LastMajorTimeB_mm ) && ( _rtDW ->
LastMajorTimeA_c >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime (
S , 0 ) - _rtDW -> LastMajorTimeB_mm ; rtb_B_120_5_0 = _rtDW -> PrevYB_g ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_c ;
rtb_B_120_5_0 = _rtDW -> PrevYA_i ; } riseValLimit = deltaT * _rtP -> P_160 ;
rateLimiterRate = _rtB -> B_120_154_0 - rtb_B_120_5_0 ; if ( rateLimiterRate
> riseValLimit ) { _rtB -> B_120_155_0 = rtb_B_120_5_0 + riseValLimit ; }
else { deltaT *= _rtP -> P_161 ; if ( rateLimiterRate < deltaT ) { _rtB ->
B_120_155_0 = rtb_B_120_5_0 + deltaT ; } else { _rtB -> B_120_155_0 = _rtB ->
B_120_154_0 ; } } } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) {
_rtB -> B_120_156_0 = _rtP -> P_162 * _rtB -> B_120_146_0 ; } _rtB ->
B_120_157_0 = _rtB -> B_120_155_0 * _rtB -> B_120_156_0 ; _rtB -> B_120_158_0
= _rtB -> B_120_148_0 * _rtB -> B_120_157_0 ; vm_ReadLocalDSMNoIdx ( S ,
_rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model5/Data Store Read" ,
0 ) ; _rtB -> B_120_159_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_m != 0 )
{ _rtX -> vv0at_CSTATE_h = _rtB -> B_120_159_0 ; } _rtB -> B_120_160_0 = _rtX
-> vv0at_CSTATE_h ; _rtB -> B_120_161_0 = muDoubleScalarAbs ( _rtB ->
B_120_160_0 ) ; _rtB -> B_120_162_0 = _rtP -> P_163 [ 0 ] ; _rtB ->
B_120_162_0 = _rtB -> B_120_162_0 * _rtB -> B_120_161_0 + _rtP -> P_163 [ 1 ]
; _rtB -> B_120_162_0 = _rtB -> B_120_162_0 * _rtB -> B_120_161_0 + _rtP ->
P_163 [ 2 ] ; _rtB -> B_120_163_0 = _rtP -> P_164 * _rtB -> B_120_162_0 ;
_rtB -> B_120_164_0 = _rtB -> B_120_146_0 * _rtB -> B_120_163_0 ; _rtB ->
B_120_165_0 = _rtB -> B_120_158_0 - _rtB -> B_120_164_0 ; isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB -> B_120_167_0 = _rtP
-> P_165 * _rtB -> B_120_147_0 ; } if ( _rtB -> B_120_165_0 > _rtB ->
B_120_147_0 ) { _rtB -> B_120_169_0 = _rtB -> B_120_147_0 ; } else { if (
_rtB -> B_120_165_0 < _rtB -> B_120_167_0 ) { _rtB -> B_29_1_0 = _rtB ->
B_120_167_0 ; } else { _rtB -> B_29_1_0 = _rtB -> B_120_165_0 ; } _rtB ->
B_120_169_0 = _rtB -> B_29_1_0 ; } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if (
isHit != 0 ) { _rtB -> B_120_171_0 = _rtP -> P_166 * _rtB -> B_120_146_0 *
_rtB -> B_120_82_0_a ; } _rtB -> B_120_172_0 = _rtB -> B_120_169_0 + _rtB ->
B_120_171_0 ; if ( _rtB -> B_120_81_0_n ) { _rtB -> B_120_173_0 = _rtB ->
B_120_172_0 ; } else { _rtB -> B_120_173_0 = _rtB -> B_120_79_0_k ; } isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { ssCallAccelRunBlock ( S ,
22 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP -> P_988 != 0 ) && ( ! ( _rtB ->
B_22_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_988 == 1 ) { if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_at == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model3/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_at < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_at ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model3/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_59_0_l ) ; if ( ( _rtP -> P_988 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_988 == 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_at == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model3/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_at < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_at ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model3/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_59_0_l ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_59_0_l ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_175_0 =
_rtB -> B_120_58_0_b [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_22_0_0 , 499U ) ] ; _rtB -> B_120_176_0 =
_rtB -> B_120_65_0_b * _rtB -> B_120_175_0 ; if ( ( _rtP -> P_989 != 0 ) && (
! ( _rtB -> B_22_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_989 == 1 ) { if ( _rtDW
-> lookupwagonbrakingefficiency_NumIssuedWarnings_g == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_g < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_g ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_60_0_h ) ; if ( ( _rtP -> P_989 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_989 == 1 ) { if ( _rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_g
== 0U ) { diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_g < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_g ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_60_0_h ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_60_0_h ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_177_0 =
_rtB -> B_120_58_0_b [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_22_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_f . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_f .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_167 ; _rtB -> B_120_178_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_m . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_m . Last , _rtDW -> TransportDelay_IWORK_m . Tail ,
_rtDW -> TransportDelay_IWORK_m . Head , _rtP -> P_168 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_120_179_0 = 0.0 ; _rtB -> B_120_179_0 += _rtP -> P_170 * _rtX
-> TransferFcn_CSTATE_f ; _rtB -> B_120_179_0 += _rtP -> P_171 * _rtB ->
B_120_178_0 ; if ( _rtB -> B_120_81_0_n ) { if ( _rtB -> B_120_179_0 > _rtP
-> P_19 ) { _rtB -> B_27_0_0 = _rtB -> B_120_179_0 - _rtP -> P_19 ; } else if
( _rtB -> B_120_179_0 >= _rtP -> P_18 ) { _rtB -> B_27_0_0 = 0.0 ; } else {
_rtB -> B_27_0_0 = _rtB -> B_120_179_0 - _rtP -> P_18 ; } _rtB -> B_120_181_0
= _rtB -> B_27_0_0 ; } else { _rtB -> B_120_181_0 = _rtB -> B_120_79_0_k ; }
_rtB -> B_120_182_0 = _rtP -> P_172 * _rtB -> B_120_181_0 ; if ( _rtB ->
B_120_182_0 > _rtP -> P_173 ) { _rtB -> B_120_183_0 = _rtP -> P_173 ; } else
if ( _rtB -> B_120_182_0 < _rtP -> P_174 ) { _rtB -> B_120_183_0 = _rtP ->
P_174 ; } else { _rtB -> B_120_183_0 = _rtB -> B_120_182_0 ; } if ( ( _rtDW
-> LastMajorTimeA_b0 >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW ->
LastMajorTimeB_e >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_120_184_0 = _rtB
-> B_120_183_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_b0 < _rtDW ->
LastMajorTimeB_e ) && ( _rtDW -> LastMajorTimeB_e < ssGetTaskTime ( S , 0 ) )
) || ( ( _rtDW -> LastMajorTimeA_b0 >= _rtDW -> LastMajorTimeB_e ) && ( _rtDW
-> LastMajorTimeA_b0 >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_e ; rtb_B_120_5_0 = _rtDW
-> PrevYB_lz ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_b0 ; rtb_B_120_5_0 = _rtDW -> PrevYA_a ; } riseValLimit =
deltaT * _rtP -> P_175 ; rateLimiterRate = _rtB -> B_120_183_0 -
rtb_B_120_5_0 ; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_120_184_0 =
rtb_B_120_5_0 + riseValLimit ; } else { deltaT *= _rtP -> P_176 ; if (
rateLimiterRate < deltaT ) { _rtB -> B_120_184_0 = rtb_B_120_5_0 + deltaT ; }
else { _rtB -> B_120_184_0 = _rtB -> B_120_183_0 ; } } } isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB -> B_120_185_0 = _rtP
-> P_177 * _rtB -> B_120_175_0 ; } _rtB -> B_120_186_0 = _rtB -> B_120_184_0
* _rtB -> B_120_185_0 ; _rtB -> B_120_187_0 = _rtB -> B_120_177_0 * _rtB ->
B_120_186_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model3/Data Store Read" ,
0 ) ; _rtB -> B_120_188_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_l != 0 )
{ _rtX -> vv0at_CSTATE_o = _rtB -> B_120_188_0 ; } _rtB -> B_120_189_0 = _rtX
-> vv0at_CSTATE_o ; _rtB -> B_120_190_0 = muDoubleScalarAbs ( _rtB ->
B_120_189_0 ) ; _rtB -> B_120_191_0 = _rtP -> P_178 [ 0 ] ; _rtB ->
B_120_191_0 = _rtB -> B_120_191_0 * _rtB -> B_120_190_0 + _rtP -> P_178 [ 1 ]
; _rtB -> B_120_191_0 = _rtB -> B_120_191_0 * _rtB -> B_120_190_0 + _rtP ->
P_178 [ 2 ] ; _rtB -> B_120_192_0 = _rtP -> P_179 * _rtB -> B_120_191_0 ;
_rtB -> B_120_193_0 = _rtB -> B_120_175_0 * _rtB -> B_120_192_0 ; _rtB ->
B_120_194_0 = _rtB -> B_120_187_0 - _rtB -> B_120_193_0 ; isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB -> B_120_196_0 = _rtP
-> P_180 * _rtB -> B_120_176_0 ; } if ( _rtB -> B_120_194_0 > _rtB ->
B_120_176_0 ) { _rtB -> B_120_198_0 = _rtB -> B_120_176_0 ; } else { if (
_rtB -> B_120_194_0 < _rtB -> B_120_196_0 ) { _rtB -> B_23_1_0 = _rtB ->
B_120_196_0 ; } else { _rtB -> B_23_1_0 = _rtB -> B_120_194_0 ; } _rtB ->
B_120_198_0 = _rtB -> B_23_1_0 ; } if ( _rtB -> B_120_64_0 ) { _rtB ->
B_120_199_0 = _rtB -> B_120_198_0 ; } else { _rtB -> B_120_199_0 = _rtB ->
B_120_63_0_p ; } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) {
ssCallAccelRunBlock ( S , 19 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP ->
P_990 != 0 ) && ( ! ( _rtB -> B_19_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_990 ==
1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_o == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model2/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_o < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_o ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model2/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_51_0_o ) ; if ( ( _rtP -> P_990 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_990 == 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_o == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model2/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_o < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_o ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model2/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_51_0_o ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_51_0_o ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_201_0 =
_rtB -> B_120_50_0_l [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_19_0_0 , 499U ) ] ; _rtB -> B_120_202_0 =
_rtB -> B_120_57_0_n * _rtB -> B_120_201_0 ; if ( ( _rtP -> P_991 != 0 ) && (
! ( _rtB -> B_19_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_991 == 1 ) { if ( _rtDW
-> lookupwagonbrakingefficiency_NumIssuedWarnings_lo == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_lo < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_lo ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_52_0_b ) ; if ( ( _rtP -> P_991 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_991 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_lo == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_lo < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_lo ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_52_0_b ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_52_0_b ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_203_0 =
_rtB -> B_120_50_0_l [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_19_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_p . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_p .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_181 ; _rtB -> B_120_204_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_l . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_l . Last , _rtDW -> TransportDelay_IWORK_l . Tail ,
_rtDW -> TransportDelay_IWORK_l . Head , _rtP -> P_182 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_120_205_0 = 0.0 ; _rtB -> B_120_205_0 += _rtP -> P_184 * _rtX
-> TransferFcn_CSTATE_ln ; _rtB -> B_120_205_0 += _rtP -> P_185 * _rtB ->
B_120_204_0 ; if ( _rtB -> B_120_64_0 ) { if ( _rtB -> B_120_205_0 > _rtP ->
P_15 ) { _rtB -> B_21_0_0 = _rtB -> B_120_205_0 - _rtP -> P_15 ; } else if (
_rtB -> B_120_205_0 >= _rtP -> P_14 ) { _rtB -> B_21_0_0 = 0.0 ; } else {
_rtB -> B_21_0_0 = _rtB -> B_120_205_0 - _rtP -> P_14 ; } _rtB -> B_120_207_0
= _rtB -> B_21_0_0 ; } else { _rtB -> B_120_207_0 = _rtB -> B_120_63_0_p ; }
_rtB -> B_120_208_0 = _rtP -> P_186 * _rtB -> B_120_207_0 ; if ( _rtB ->
B_120_208_0 > _rtP -> P_187 ) { _rtB -> B_120_209_0 = _rtP -> P_187 ; } else
if ( _rtB -> B_120_208_0 < _rtP -> P_188 ) { _rtB -> B_120_209_0 = _rtP ->
P_188 ; } else { _rtB -> B_120_209_0 = _rtB -> B_120_208_0 ; } if ( ( _rtDW
-> LastMajorTimeA_m >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW ->
LastMajorTimeB_d >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_120_210_0 = _rtB
-> B_120_209_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_m < _rtDW ->
LastMajorTimeB_d ) && ( _rtDW -> LastMajorTimeB_d < ssGetTaskTime ( S , 0 ) )
) || ( ( _rtDW -> LastMajorTimeA_m >= _rtDW -> LastMajorTimeB_d ) && ( _rtDW
-> LastMajorTimeA_m >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime
( S , 0 ) - _rtDW -> LastMajorTimeB_d ; rtb_B_120_5_0 = _rtDW -> PrevYB_a ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_m ;
rtb_B_120_5_0 = _rtDW -> PrevYA_h4 ; } riseValLimit = deltaT * _rtP -> P_189
; rateLimiterRate = _rtB -> B_120_209_0 - rtb_B_120_5_0 ; if (
rateLimiterRate > riseValLimit ) { _rtB -> B_120_210_0 = rtb_B_120_5_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_190 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_120_210_0 = rtb_B_120_5_0 + deltaT ; } else { _rtB ->
B_120_210_0 = _rtB -> B_120_209_0 ; } } } isHit = ssIsSampleHit ( S , 2 , 0 )
; if ( isHit != 0 ) { _rtB -> B_120_211_0 = _rtP -> P_191 * _rtB ->
B_120_201_0 ; } _rtB -> B_120_212_0 = _rtB -> B_120_210_0 * _rtB ->
B_120_211_0 ; _rtB -> B_120_213_0 = _rtB -> B_120_203_0 * _rtB -> B_120_212_0
; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model2/Data Store Read" ,
0 ) ; _rtB -> B_120_214_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_p != 0 )
{ _rtX -> vv0at_CSTATE_dm = _rtB -> B_120_214_0 ; } _rtB -> B_120_215_0 =
_rtX -> vv0at_CSTATE_dm ; _rtB -> B_120_216_0 = muDoubleScalarAbs ( _rtB ->
B_120_215_0 ) ; _rtB -> B_120_217_0 = _rtP -> P_192 [ 0 ] ; _rtB ->
B_120_217_0 = _rtB -> B_120_217_0 * _rtB -> B_120_216_0 + _rtP -> P_192 [ 1 ]
; _rtB -> B_120_217_0 = _rtB -> B_120_217_0 * _rtB -> B_120_216_0 + _rtP ->
P_192 [ 2 ] ; _rtB -> B_120_218_0 = _rtP -> P_193 * _rtB -> B_120_217_0 ;
_rtB -> B_120_219_0 = _rtB -> B_120_201_0 * _rtB -> B_120_218_0 ; _rtB ->
B_120_220_0 = _rtB -> B_120_213_0 - _rtB -> B_120_219_0 ; isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB -> B_120_222_0 = _rtP
-> P_194 * _rtB -> B_120_202_0 ; } if ( _rtB -> B_120_220_0 > _rtB ->
B_120_202_0 ) { _rtB -> B_120_224_0 = _rtB -> B_120_202_0 ; } else { if (
_rtB -> B_120_220_0 < _rtB -> B_120_222_0 ) { _rtB -> B_20_1_0 = _rtB ->
B_120_222_0 ; } else { _rtB -> B_20_1_0 = _rtB -> B_120_220_0 ; } _rtB ->
B_120_224_0 = _rtB -> B_20_1_0 ; } if ( _rtB -> B_120_56_0_l ) { _rtB ->
B_120_225_0 = _rtB -> B_120_224_0 ; } else { _rtB -> B_120_225_0 = _rtB ->
B_120_55_0_p ; } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) {
ssCallAccelRunBlock ( S , 16 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP ->
P_992 != 0 ) && ( ! ( _rtB -> B_16_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_992 ==
1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_om == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model1/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_om < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_om ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model1/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_43_0_l ) ; if ( ( _rtP -> P_992 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_992 == 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_om == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model1/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_om < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_om ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model1/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_43_0_l ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_43_0_l ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_227_0 =
_rtB -> B_120_42_0_g [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_16_0_0 , 499U ) ] ; _rtB -> B_120_228_0 =
_rtB -> B_120_49_0_d * _rtB -> B_120_227_0 ; if ( ( _rtP -> P_993 != 0 ) && (
! ( _rtB -> B_16_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_993 == 1 ) { if ( _rtDW
-> lookupwagonbrakingefficiency_NumIssuedWarnings_e == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_e < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_e ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_44_0_d ) ; if ( ( _rtP -> P_993 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_993 == 1 ) { if ( _rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_e
== 0U ) { diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_e < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_e ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_44_0_d ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_44_0_d ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_229_0 =
_rtB -> B_120_42_0_g [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_16_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_c . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_c .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_195 ; _rtB -> B_120_230_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_g . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_g . Last , _rtDW -> TransportDelay_IWORK_g . Tail ,
_rtDW -> TransportDelay_IWORK_g . Head , _rtP -> P_196 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_120_231_0 = 0.0 ; _rtB -> B_120_231_0 += _rtP -> P_198 * _rtX
-> TransferFcn_CSTATE_p ; _rtB -> B_120_231_0 += _rtP -> P_199 * _rtB ->
B_120_230_0 ; if ( _rtB -> B_120_56_0_l ) { if ( _rtB -> B_120_231_0 > _rtP
-> P_13 ) { _rtB -> B_18_0_0 = _rtB -> B_120_231_0 - _rtP -> P_13 ; } else if
( _rtB -> B_120_231_0 >= _rtP -> P_12 ) { _rtB -> B_18_0_0 = 0.0 ; } else {
_rtB -> B_18_0_0 = _rtB -> B_120_231_0 - _rtP -> P_12 ; } _rtB -> B_120_233_0
= _rtB -> B_18_0_0 ; } else { _rtB -> B_120_233_0 = _rtB -> B_120_55_0_p ; }
_rtB -> B_120_234_0 = _rtP -> P_200 * _rtB -> B_120_233_0 ; if ( _rtB ->
B_120_234_0 > _rtP -> P_201 ) { _rtB -> B_120_235_0 = _rtP -> P_201 ; } else
if ( _rtB -> B_120_234_0 < _rtP -> P_202 ) { _rtB -> B_120_235_0 = _rtP ->
P_202 ; } else { _rtB -> B_120_235_0 = _rtB -> B_120_234_0 ; } if ( ( _rtDW
-> LastMajorTimeA_mj >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW ->
LastMajorTimeB_g >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_120_236_0 = _rtB
-> B_120_235_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_mj < _rtDW ->
LastMajorTimeB_g ) && ( _rtDW -> LastMajorTimeB_g < ssGetTaskTime ( S , 0 ) )
) || ( ( _rtDW -> LastMajorTimeA_mj >= _rtDW -> LastMajorTimeB_g ) && ( _rtDW
-> LastMajorTimeA_mj >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_g ; rtb_B_120_5_0 = _rtDW
-> PrevYB_p ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_mj ; rtb_B_120_5_0 = _rtDW -> PrevYA_eh ; } riseValLimit =
deltaT * _rtP -> P_203 ; rateLimiterRate = _rtB -> B_120_235_0 -
rtb_B_120_5_0 ; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_120_236_0 =
rtb_B_120_5_0 + riseValLimit ; } else { deltaT *= _rtP -> P_204 ; if (
rateLimiterRate < deltaT ) { _rtB -> B_120_236_0 = rtb_B_120_5_0 + deltaT ; }
else { _rtB -> B_120_236_0 = _rtB -> B_120_235_0 ; } } } isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB -> B_120_237_0 = _rtP
-> P_205 * _rtB -> B_120_227_0 ; } _rtB -> B_120_238_0 = _rtB -> B_120_236_0
* _rtB -> B_120_237_0 ; _rtB -> B_120_239_0 = _rtB -> B_120_229_0 * _rtB ->
B_120_238_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model1/Data Store Read" ,
0 ) ; _rtB -> B_120_240_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_f != 0 )
{ _rtX -> vv0at_CSTATE_e3 = _rtB -> B_120_240_0 ; } _rtB -> B_120_241_0 =
_rtX -> vv0at_CSTATE_e3 ; _rtB -> B_120_242_0 = muDoubleScalarAbs ( _rtB ->
B_120_241_0 ) ; _rtB -> B_120_243_0 = _rtP -> P_206 [ 0 ] ; _rtB ->
B_120_243_0 = _rtB -> B_120_243_0 * _rtB -> B_120_242_0 + _rtP -> P_206 [ 1 ]
; _rtB -> B_120_243_0 = _rtB -> B_120_243_0 * _rtB -> B_120_242_0 + _rtP ->
P_206 [ 2 ] ; _rtB -> B_120_244_0 = _rtP -> P_207 * _rtB -> B_120_243_0 ;
_rtB -> B_120_245_0 = _rtB -> B_120_227_0 * _rtB -> B_120_244_0 ; _rtB ->
B_120_246_0 = _rtB -> B_120_239_0 - _rtB -> B_120_245_0 ; isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB -> B_120_248_0 = _rtP
-> P_208 * _rtB -> B_120_228_0 ; } if ( _rtB -> B_120_246_0 > _rtB ->
B_120_228_0 ) { _rtB -> B_120_250_0 = _rtB -> B_120_228_0 ; } else { if (
_rtB -> B_120_246_0 < _rtB -> B_120_248_0 ) { _rtB -> B_17_1_0 = _rtB ->
B_120_248_0 ; } else { _rtB -> B_17_1_0 = _rtB -> B_120_246_0 ; } _rtB ->
B_120_250_0 = _rtB -> B_17_1_0 ; } if ( _rtB -> B_120_48_0_n ) { _rtB ->
B_120_251_0 = _rtB -> B_120_250_0 ; } else { _rtB -> B_120_251_0 = _rtB ->
B_120_47_0 ; } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) {
ssCallAccelRunBlock ( S , 25 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP ->
P_994 != 0 ) && ( ! ( _rtB -> B_25_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_994 ==
1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_hg == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model4/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_hg < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_hg ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model4/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_67_0 ) ; if ( ( _rtP -> P_994 != 0 ) && ( ( ! ( rtb_B_120_5_0
>= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_994 == 1 )
{ if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_hg == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model4/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_hg < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_hg ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model4/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_67_0 ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_67_0 ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else { if
( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_253_0 =
_rtB -> B_120_66_0_d [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_25_0_0 , 499U ) ] ; _rtB -> B_120_254_0 =
_rtB -> B_120_73_0 * _rtB -> B_120_253_0 ; if ( ( _rtP -> P_995 != 0 ) && ( !
( _rtB -> B_25_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_995 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_jw == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_jw < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_jw ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_68_0_e ) ; if ( ( _rtP -> P_995 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_995 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_jw == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_jw < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_jw ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_68_0_e ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_68_0_e ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_255_0 =
_rtB -> B_120_66_0_d [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_25_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_j . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_j .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_209 ; _rtB -> B_120_256_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_j . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_j . Last , _rtDW -> TransportDelay_IWORK_j . Tail ,
_rtDW -> TransportDelay_IWORK_j . Head , _rtP -> P_210 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_120_257_0 = 0.0 ; _rtB -> B_120_257_0 += _rtP -> P_212 * _rtX
-> TransferFcn_CSTATE_fx ; _rtB -> B_120_257_0 += _rtP -> P_213 * _rtB ->
B_120_256_0 ; if ( _rtB -> B_120_48_0_n ) { if ( _rtB -> B_120_257_0 > _rtP
-> P_11 ) { _rtB -> B_15_0_0 = _rtB -> B_120_257_0 - _rtP -> P_11 ; } else if
( _rtB -> B_120_257_0 >= _rtP -> P_10 ) { _rtB -> B_15_0_0 = 0.0 ; } else {
_rtB -> B_15_0_0 = _rtB -> B_120_257_0 - _rtP -> P_10 ; } _rtB -> B_120_259_0
= _rtB -> B_15_0_0 ; } else { _rtB -> B_120_259_0 = _rtB -> B_120_47_0 ; }
_rtB -> B_120_260_0 = _rtP -> P_214 * _rtB -> B_120_259_0 ; if ( _rtB ->
B_120_260_0 > _rtP -> P_215 ) { _rtB -> B_120_261_0 = _rtP -> P_215 ; } else
if ( _rtB -> B_120_260_0 < _rtP -> P_216 ) { _rtB -> B_120_261_0 = _rtP ->
P_216 ; } else { _rtB -> B_120_261_0 = _rtB -> B_120_260_0 ; } if ( ( _rtDW
-> LastMajorTimeA_a >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW ->
LastMajorTimeB_ms >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_120_262_0 = _rtB
-> B_120_261_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_a < _rtDW ->
LastMajorTimeB_ms ) && ( _rtDW -> LastMajorTimeB_ms < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_a >= _rtDW -> LastMajorTimeB_ms ) && (
_rtDW -> LastMajorTimeA_a >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_ms ; rtb_B_120_5_0 = _rtDW
-> PrevYB_gv ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_a ; rtb_B_120_5_0 = _rtDW -> PrevYA_gw ; } riseValLimit =
deltaT * _rtP -> P_217 ; rateLimiterRate = _rtB -> B_120_261_0 -
rtb_B_120_5_0 ; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_120_262_0 =
rtb_B_120_5_0 + riseValLimit ; } else { deltaT *= _rtP -> P_218 ; if (
rateLimiterRate < deltaT ) { _rtB -> B_120_262_0 = rtb_B_120_5_0 + deltaT ; }
else { _rtB -> B_120_262_0 = _rtB -> B_120_261_0 ; } } } isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB -> B_120_263_0 = _rtP
-> P_219 * _rtB -> B_120_253_0 ; } _rtB -> B_120_264_0 = _rtB -> B_120_262_0
* _rtB -> B_120_263_0 ; _rtB -> B_120_265_0 = _rtB -> B_120_255_0 * _rtB ->
B_120_264_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model4/Data Store Read" ,
0 ) ; _rtB -> B_120_266_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_g != 0 )
{ _rtX -> vv0at_CSTATE_m2 = _rtB -> B_120_266_0 ; } _rtB -> B_120_267_0 =
_rtX -> vv0at_CSTATE_m2 ; _rtB -> B_120_268_0 = muDoubleScalarAbs ( _rtB ->
B_120_267_0 ) ; _rtB -> B_120_269_0 = _rtP -> P_220 [ 0 ] ; _rtB ->
B_120_269_0 = _rtB -> B_120_269_0 * _rtB -> B_120_268_0 + _rtP -> P_220 [ 1 ]
; _rtB -> B_120_269_0 = _rtB -> B_120_269_0 * _rtB -> B_120_268_0 + _rtP ->
P_220 [ 2 ] ; _rtB -> B_120_270_0 = _rtP -> P_221 * _rtB -> B_120_269_0 ;
_rtB -> B_120_271_0 = _rtB -> B_120_253_0 * _rtB -> B_120_270_0 ; _rtB ->
B_120_272_0 = _rtB -> B_120_265_0 - _rtB -> B_120_271_0 ; isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB -> B_120_274_0 = _rtP
-> P_222 * _rtB -> B_120_254_0 ; } if ( _rtB -> B_120_272_0 > _rtB ->
B_120_254_0 ) { _rtB -> B_120_276_0 = _rtB -> B_120_254_0 ; } else { if (
_rtB -> B_120_272_0 < _rtB -> B_120_274_0 ) { _rtB -> B_26_1_0 = _rtB ->
B_120_274_0 ; } else { _rtB -> B_26_1_0 = _rtB -> B_120_272_0 ; } _rtB ->
B_120_276_0 = _rtB -> B_26_1_0 ; } if ( _rtB -> B_120_72_0_c ) { _rtB ->
B_120_277_0 = _rtB -> B_120_276_0 ; } else { _rtB -> B_120_277_0 = _rtB ->
B_120_71_0_n ; } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) {
ssCallAccelRunBlock ( S , 43 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP ->
P_996 != 0 ) && ( ! ( _rtB -> B_43_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_996 ==
1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_g == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model5/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_g < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_g ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model5/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_117_0_p ) ; if ( ( _rtP -> P_996 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_996 == 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_g == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model5/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_g < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_g ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model5/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_117_0_p ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_117_0_p ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_279_0 =
_rtB -> B_120_116_0 [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_43_0_0 , 499U ) ] ; _rtB -> B_120_280_0 =
_rtB -> B_120_123_0_a * _rtB -> B_120_279_0 ; if ( ( _rtP -> P_997 != 0 ) &&
( ! ( _rtB -> B_43_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_997 == 1 ) { if (
_rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_k3 == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model5/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_k3 < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_k3 ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model5/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_118_0_p ) ; if ( ( _rtP -> P_997 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_997 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_k3 == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model5/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_k3 < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_k3 ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model5/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_118_0_p ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_118_0_p ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_281_0 =
_rtB -> B_120_116_0 [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_43_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_d . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_d .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_223 ; _rtB -> B_120_282_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_jj . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_jj . Last , _rtDW -> TransportDelay_IWORK_jj . Tail ,
_rtDW -> TransportDelay_IWORK_jj . Head , _rtP -> P_224 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_120_283_0 = 0.0 ; _rtB -> B_120_283_0 += _rtP -> P_226 * _rtX
-> TransferFcn_CSTATE_j ; _rtB -> B_120_283_0 += _rtP -> P_227 * _rtB ->
B_120_282_0 ; if ( _rtB -> B_120_72_0_c ) { if ( _rtB -> B_120_283_0 > _rtP
-> P_17 ) { _rtB -> B_24_0_0 = _rtB -> B_120_283_0 - _rtP -> P_17 ; } else if
( _rtB -> B_120_283_0 >= _rtP -> P_16 ) { _rtB -> B_24_0_0 = 0.0 ; } else {
_rtB -> B_24_0_0 = _rtB -> B_120_283_0 - _rtP -> P_16 ; } _rtB -> B_120_285_0
= _rtB -> B_24_0_0 ; } else { _rtB -> B_120_285_0 = _rtB -> B_120_71_0_n ; }
_rtB -> B_120_286_0 = _rtP -> P_228 * _rtB -> B_120_285_0 ; if ( _rtB ->
B_120_286_0 > _rtP -> P_229 ) { _rtB -> B_120_287_0 = _rtP -> P_229 ; } else
if ( _rtB -> B_120_286_0 < _rtP -> P_230 ) { _rtB -> B_120_287_0 = _rtP ->
P_230 ; } else { _rtB -> B_120_287_0 = _rtB -> B_120_286_0 ; } if ( ( _rtDW
-> LastMajorTimeA_h >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW ->
LastMajorTimeB_f >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_120_288_0 = _rtB
-> B_120_287_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_h < _rtDW ->
LastMajorTimeB_f ) && ( _rtDW -> LastMajorTimeB_f < ssGetTaskTime ( S , 0 ) )
) || ( ( _rtDW -> LastMajorTimeA_h >= _rtDW -> LastMajorTimeB_f ) && ( _rtDW
-> LastMajorTimeA_h >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime
( S , 0 ) - _rtDW -> LastMajorTimeB_f ; rtb_B_120_5_0 = _rtDW -> PrevYB_n ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_h ;
rtb_B_120_5_0 = _rtDW -> PrevYA_gg ; } riseValLimit = deltaT * _rtP -> P_231
; rateLimiterRate = _rtB -> B_120_287_0 - rtb_B_120_5_0 ; if (
rateLimiterRate > riseValLimit ) { _rtB -> B_120_288_0 = rtb_B_120_5_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_232 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_120_288_0 = rtb_B_120_5_0 + deltaT ; } else { _rtB ->
B_120_288_0 = _rtB -> B_120_287_0 ; } } } isHit = ssIsSampleHit ( S , 2 , 0 )
; if ( isHit != 0 ) { _rtB -> B_120_289_0 = _rtP -> P_233 * _rtB ->
B_120_279_0 ; } _rtB -> B_120_290_0 = _rtB -> B_120_288_0 * _rtB ->
B_120_289_0 ; _rtB -> B_120_291_0 = _rtB -> B_120_281_0 * _rtB -> B_120_290_0
; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model5/Data Store Read" ,
0 ) ; _rtB -> B_120_292_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_l5 != 0 )
{ _rtX -> vv0at_CSTATE_f = _rtB -> B_120_292_0 ; } _rtB -> B_120_293_0 = _rtX
-> vv0at_CSTATE_f ; _rtB -> B_120_294_0 = muDoubleScalarAbs ( _rtB ->
B_120_293_0 ) ; _rtB -> B_120_295_0 = _rtP -> P_234 [ 0 ] ; _rtB ->
B_120_295_0 = _rtB -> B_120_295_0 * _rtB -> B_120_294_0 + _rtP -> P_234 [ 1 ]
; _rtB -> B_120_295_0 = _rtB -> B_120_295_0 * _rtB -> B_120_294_0 + _rtP ->
P_234 [ 2 ] ; _rtB -> B_120_296_0 = _rtP -> P_235 * _rtB -> B_120_295_0 ;
_rtB -> B_120_297_0 = _rtB -> B_120_279_0 * _rtB -> B_120_296_0 ; _rtB ->
B_120_298_0 = _rtB -> B_120_291_0 - _rtB -> B_120_297_0 ; isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB -> B_120_300_0 = _rtP
-> P_236 * _rtB -> B_120_280_0 ; } if ( _rtB -> B_120_298_0 > _rtB ->
B_120_280_0 ) { _rtB -> B_120_302_0 = _rtB -> B_120_280_0 ; } else { if (
_rtB -> B_120_298_0 < _rtB -> B_120_300_0 ) { _rtB -> B_44_1_0 = _rtB ->
B_120_300_0 ; } else { _rtB -> B_44_1_0 = _rtB -> B_120_298_0 ; } _rtB ->
B_120_302_0 = _rtB -> B_44_1_0 ; } if ( _rtB -> B_120_122_0_o ) { _rtB ->
B_120_303_0 = _rtB -> B_120_302_0 ; } else { _rtB -> B_120_303_0 = _rtB ->
B_120_121_0_c ; } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) {
ssCallAccelRunBlock ( S , 37 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP ->
P_998 != 0 ) && ( ! ( _rtB -> B_37_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_998 ==
1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_d == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model3/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_d < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_d ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model3/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_101_0_c ) ; if ( ( _rtP -> P_998 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_998 == 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_d == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model3/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_d < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_d ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model3/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_101_0_c ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_101_0_c ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_305_0 =
_rtB -> B_120_100_0_m [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_37_0_0 , 499U ) ] ; _rtB -> B_120_306_0 =
_rtB -> B_120_107_0_m * _rtB -> B_120_305_0 ; if ( ( _rtP -> P_999 != 0 ) &&
( ! ( _rtB -> B_37_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_999 == 1 ) { if (
_rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_o == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_o < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_o ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_102_0_m ) ; if ( ( _rtP -> P_999 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_999 == 1 ) { if ( _rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_o
== 0U ) { diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_o < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_o ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_102_0_m ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_102_0_m ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_307_0 =
_rtB -> B_120_100_0_m [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_37_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_i . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_i .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_237 ; _rtB -> B_120_308_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_p . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_p . Last , _rtDW -> TransportDelay_IWORK_p . Tail ,
_rtDW -> TransportDelay_IWORK_p . Head , _rtP -> P_238 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_120_309_0 = 0.0 ; _rtB -> B_120_309_0 += _rtP -> P_240 * _rtX
-> TransferFcn_CSTATE_hm ; _rtB -> B_120_309_0 += _rtP -> P_241 * _rtB ->
B_120_308_0 ; if ( _rtB -> B_120_122_0_o ) { if ( _rtB -> B_120_309_0 > _rtP
-> P_29 ) { _rtB -> B_42_0_0 = _rtB -> B_120_309_0 - _rtP -> P_29 ; } else if
( _rtB -> B_120_309_0 >= _rtP -> P_28 ) { _rtB -> B_42_0_0 = 0.0 ; } else {
_rtB -> B_42_0_0 = _rtB -> B_120_309_0 - _rtP -> P_28 ; } _rtB -> B_120_311_0
= _rtB -> B_42_0_0 ; } else { _rtB -> B_120_311_0 = _rtB -> B_120_121_0_c ; }
_rtB -> B_120_312_0 = _rtP -> P_242 * _rtB -> B_120_311_0 ; if ( _rtB ->
B_120_312_0 > _rtP -> P_243 ) { _rtB -> B_120_313_0 = _rtP -> P_243 ; } else
if ( _rtB -> B_120_312_0 < _rtP -> P_244 ) { _rtB -> B_120_313_0 = _rtP ->
P_244 ; } else { _rtB -> B_120_313_0 = _rtB -> B_120_312_0 ; } if ( ( _rtDW
-> LastMajorTimeA_n >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW ->
LastMajorTimeB_i >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_120_314_0 = _rtB
-> B_120_313_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_n < _rtDW ->
LastMajorTimeB_i ) && ( _rtDW -> LastMajorTimeB_i < ssGetTaskTime ( S , 0 ) )
) || ( ( _rtDW -> LastMajorTimeA_n >= _rtDW -> LastMajorTimeB_i ) && ( _rtDW
-> LastMajorTimeA_n >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime
( S , 0 ) - _rtDW -> LastMajorTimeB_i ; rtb_B_120_5_0 = _rtDW -> PrevYB_d ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_n ;
rtb_B_120_5_0 = _rtDW -> PrevYA_p ; } riseValLimit = deltaT * _rtP -> P_245 ;
rateLimiterRate = _rtB -> B_120_313_0 - rtb_B_120_5_0 ; if ( rateLimiterRate
> riseValLimit ) { _rtB -> B_120_314_0 = rtb_B_120_5_0 + riseValLimit ; }
else { deltaT *= _rtP -> P_246 ; if ( rateLimiterRate < deltaT ) { _rtB ->
B_120_314_0 = rtb_B_120_5_0 + deltaT ; } else { _rtB -> B_120_314_0 = _rtB ->
B_120_313_0 ; } } } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) {
_rtB -> B_120_315_0 = _rtP -> P_247 * _rtB -> B_120_305_0 ; } _rtB ->
B_120_316_0 = _rtB -> B_120_314_0 * _rtB -> B_120_315_0 ; _rtB -> B_120_317_0
= _rtB -> B_120_307_0 * _rtB -> B_120_316_0 ; vm_ReadLocalDSMNoIdx ( S ,
_rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model3/Data Store Read" ,
0 ) ; _rtB -> B_120_318_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_pv != 0 )
{ _rtX -> vv0at_CSTATE_j = _rtB -> B_120_318_0 ; } _rtB -> B_120_319_0 = _rtX
-> vv0at_CSTATE_j ; _rtB -> B_120_320_0 = muDoubleScalarAbs ( _rtB ->
B_120_319_0 ) ; _rtB -> B_120_321_0 = _rtP -> P_248 [ 0 ] ; _rtB ->
B_120_321_0 = _rtB -> B_120_321_0 * _rtB -> B_120_320_0 + _rtP -> P_248 [ 1 ]
; _rtB -> B_120_321_0 = _rtB -> B_120_321_0 * _rtB -> B_120_320_0 + _rtP ->
P_248 [ 2 ] ; _rtB -> B_120_322_0 = _rtP -> P_249 * _rtB -> B_120_321_0 ;
_rtB -> B_120_323_0 = _rtB -> B_120_305_0 * _rtB -> B_120_322_0 ; _rtB ->
B_120_324_0 = _rtB -> B_120_317_0 - _rtB -> B_120_323_0 ; isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB -> B_120_326_0 = _rtP
-> P_250 * _rtB -> B_120_306_0 ; } if ( _rtB -> B_120_324_0 > _rtB ->
B_120_306_0 ) { _rtB -> B_120_328_0 = _rtB -> B_120_306_0 ; } else { if (
_rtB -> B_120_324_0 < _rtB -> B_120_326_0 ) { _rtB -> B_38_1_0 = _rtB ->
B_120_326_0 ; } else { _rtB -> B_38_1_0 = _rtB -> B_120_324_0 ; } _rtB ->
B_120_328_0 = _rtB -> B_38_1_0 ; } if ( _rtB -> B_120_106_0_d ) { _rtB ->
B_120_329_0 = _rtB -> B_120_328_0 ; } else { _rtB -> B_120_329_0 = _rtB ->
B_120_105_0_g ; } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) {
ssCallAccelRunBlock ( S , 34 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP ->
P_1000 != 0 ) && ( ! ( _rtB -> B_34_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_1000
== 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_hh == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model2/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_hh < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_hh ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model2/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_93_0 ) ; if ( ( _rtP -> P_1000 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_1000 == 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_hh == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model2/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_hh < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_hh ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model2/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_93_0 ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_93_0 ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else { if
( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_331_0 =
_rtB -> B_120_92_0_o [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_34_0_0 , 499U ) ] ; _rtB -> B_120_332_0 =
_rtB -> B_120_99_0 * _rtB -> B_120_331_0 ; if ( ( _rtP -> P_1001 != 0 ) && (
! ( _rtB -> B_34_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_1001 == 1 ) { if ( _rtDW
-> lookupwagonbrakingefficiency_NumIssuedWarnings_ki == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_ki < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_ki ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_94_0_n ) ; if ( ( _rtP -> P_1001 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_1001 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_ki == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_ki < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_ki ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_94_0_n ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_94_0_n ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_333_0 =
_rtB -> B_120_92_0_o [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_34_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_l . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_l .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_251 ; _rtB -> B_120_334_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_o . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_o . Last , _rtDW -> TransportDelay_IWORK_o . Tail ,
_rtDW -> TransportDelay_IWORK_o . Head , _rtP -> P_252 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_120_335_0 = 0.0 ; _rtB -> B_120_335_0 += _rtP -> P_254 * _rtX
-> TransferFcn_CSTATE_mg ; _rtB -> B_120_335_0 += _rtP -> P_255 * _rtB ->
B_120_334_0 ; if ( _rtB -> B_120_106_0_d ) { if ( _rtB -> B_120_335_0 > _rtP
-> P_25 ) { _rtB -> B_36_0_0 = _rtB -> B_120_335_0 - _rtP -> P_25 ; } else if
( _rtB -> B_120_335_0 >= _rtP -> P_24 ) { _rtB -> B_36_0_0 = 0.0 ; } else {
_rtB -> B_36_0_0 = _rtB -> B_120_335_0 - _rtP -> P_24 ; } _rtB -> B_120_337_0
= _rtB -> B_36_0_0 ; } else { _rtB -> B_120_337_0 = _rtB -> B_120_105_0_g ; }
_rtB -> B_120_338_0 = _rtP -> P_256 * _rtB -> B_120_337_0 ; if ( _rtB ->
B_120_338_0 > _rtP -> P_257 ) { _rtB -> B_120_339_0 = _rtP -> P_257 ; } else
if ( _rtB -> B_120_338_0 < _rtP -> P_258 ) { _rtB -> B_120_339_0 = _rtP ->
P_258 ; } else { _rtB -> B_120_339_0 = _rtB -> B_120_338_0 ; } if ( ( _rtDW
-> LastMajorTimeA_d >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW ->
LastMajorTimeB_n >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_120_340_0 = _rtB
-> B_120_339_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_d < _rtDW ->
LastMajorTimeB_n ) && ( _rtDW -> LastMajorTimeB_n < ssGetTaskTime ( S , 0 ) )
) || ( ( _rtDW -> LastMajorTimeA_d >= _rtDW -> LastMajorTimeB_n ) && ( _rtDW
-> LastMajorTimeA_d >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime
( S , 0 ) - _rtDW -> LastMajorTimeB_n ; rtb_B_120_5_0 = _rtDW -> PrevYB_j ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_d ;
rtb_B_120_5_0 = _rtDW -> PrevYA_m ; } riseValLimit = deltaT * _rtP -> P_259 ;
rateLimiterRate = _rtB -> B_120_339_0 - rtb_B_120_5_0 ; if ( rateLimiterRate
> riseValLimit ) { _rtB -> B_120_340_0 = rtb_B_120_5_0 + riseValLimit ; }
else { deltaT *= _rtP -> P_260 ; if ( rateLimiterRate < deltaT ) { _rtB ->
B_120_340_0 = rtb_B_120_5_0 + deltaT ; } else { _rtB -> B_120_340_0 = _rtB ->
B_120_339_0 ; } } } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) {
_rtB -> B_120_341_0 = _rtP -> P_261 * _rtB -> B_120_331_0 ; } _rtB ->
B_120_342_0 = _rtB -> B_120_340_0 * _rtB -> B_120_341_0 ; _rtB -> B_120_343_0
= _rtB -> B_120_333_0 * _rtB -> B_120_342_0 ; vm_ReadLocalDSMNoIdx ( S ,
_rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model2/Data Store Read" ,
0 ) ; _rtB -> B_120_344_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_pu != 0 )
{ _rtX -> vv0at_CSTATE_g = _rtB -> B_120_344_0 ; } _rtB -> B_120_345_0 = _rtX
-> vv0at_CSTATE_g ; _rtB -> B_120_346_0 = muDoubleScalarAbs ( _rtB ->
B_120_345_0 ) ; _rtB -> B_120_347_0 = _rtP -> P_262 [ 0 ] ; _rtB ->
B_120_347_0 = _rtB -> B_120_347_0 * _rtB -> B_120_346_0 + _rtP -> P_262 [ 1 ]
; _rtB -> B_120_347_0 = _rtB -> B_120_347_0 * _rtB -> B_120_346_0 + _rtP ->
P_262 [ 2 ] ; _rtB -> B_120_348_0 = _rtP -> P_263 * _rtB -> B_120_347_0 ;
_rtB -> B_120_349_0 = _rtB -> B_120_331_0 * _rtB -> B_120_348_0 ; _rtB ->
B_120_350_0 = _rtB -> B_120_343_0 - _rtB -> B_120_349_0 ; isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB -> B_120_352_0 = _rtP
-> P_264 * _rtB -> B_120_332_0 ; } if ( _rtB -> B_120_350_0 > _rtB ->
B_120_332_0 ) { _rtB -> B_120_354_0 = _rtB -> B_120_332_0 ; } else { if (
_rtB -> B_120_350_0 < _rtB -> B_120_352_0 ) { _rtB -> B_35_1_0 = _rtB ->
B_120_352_0 ; } else { _rtB -> B_35_1_0 = _rtB -> B_120_350_0 ; } _rtB ->
B_120_354_0 = _rtB -> B_35_1_0 ; } if ( _rtB -> B_120_98_0_p ) { _rtB ->
B_120_355_0 = _rtB -> B_120_354_0 ; } else { _rtB -> B_120_355_0 = _rtB ->
B_120_97_0_o ; } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) {
ssCallAccelRunBlock ( S , 31 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP ->
P_1002 != 0 ) && ( ! ( _rtB -> B_31_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_1002
== 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_n2 == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model1/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_n2 < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_n2 ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model1/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_85_0_o ) ; if ( ( _rtP -> P_1002 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_1002 == 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_n2 == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model1/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_n2 < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_n2 ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model1/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_85_0_o ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_85_0_o ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_357_0 =
_rtB -> B_120_84_0_j [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_31_0_0 , 499U ) ] ; _rtB -> B_120_358_0 =
_rtB -> B_120_91_0_i * _rtB -> B_120_357_0 ; if ( ( _rtP -> P_1003 != 0 ) &&
( ! ( _rtB -> B_31_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_1003 == 1 ) { if (
_rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_en == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_en < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_en ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_86_0_n ) ; if ( ( _rtP -> P_1003 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_1003 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_en == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_en < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_en ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_86_0_n ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_86_0_n ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_359_0 =
_rtB -> B_120_84_0_j [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_31_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_g . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_g .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_265 ; _rtB -> B_120_360_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_ls . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_ls . Last , _rtDW -> TransportDelay_IWORK_ls . Tail ,
_rtDW -> TransportDelay_IWORK_ls . Head , _rtP -> P_266 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_120_361_0 = 0.0 ; _rtB -> B_120_361_0 += _rtP -> P_268 * _rtX
-> TransferFcn_CSTATE_mf ; _rtB -> B_120_361_0 += _rtP -> P_269 * _rtB ->
B_120_360_0 ; if ( _rtB -> B_120_98_0_p ) { if ( _rtB -> B_120_361_0 > _rtP
-> P_23 ) { _rtB -> B_33_0_0 = _rtB -> B_120_361_0 - _rtP -> P_23 ; } else if
( _rtB -> B_120_361_0 >= _rtP -> P_22 ) { _rtB -> B_33_0_0 = 0.0 ; } else {
_rtB -> B_33_0_0 = _rtB -> B_120_361_0 - _rtP -> P_22 ; } _rtB -> B_120_363_0
= _rtB -> B_33_0_0 ; } else { _rtB -> B_120_363_0 = _rtB -> B_120_97_0_o ; }
_rtB -> B_120_364_0 = _rtP -> P_270 * _rtB -> B_120_363_0 ; if ( _rtB ->
B_120_364_0 > _rtP -> P_271 ) { _rtB -> B_120_365_0 = _rtP -> P_271 ; } else
if ( _rtB -> B_120_364_0 < _rtP -> P_272 ) { _rtB -> B_120_365_0 = _rtP ->
P_272 ; } else { _rtB -> B_120_365_0 = _rtB -> B_120_364_0 ; } if ( ( _rtDW
-> LastMajorTimeA_f >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW ->
LastMajorTimeB_a >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_120_366_0 = _rtB
-> B_120_365_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_f < _rtDW ->
LastMajorTimeB_a ) && ( _rtDW -> LastMajorTimeB_a < ssGetTaskTime ( S , 0 ) )
) || ( ( _rtDW -> LastMajorTimeA_f >= _rtDW -> LastMajorTimeB_a ) && ( _rtDW
-> LastMajorTimeA_f >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime
( S , 0 ) - _rtDW -> LastMajorTimeB_a ; rtb_B_120_5_0 = _rtDW -> PrevYB_jd ;
} else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_f ;
rtb_B_120_5_0 = _rtDW -> PrevYA_mj ; } riseValLimit = deltaT * _rtP -> P_273
; rateLimiterRate = _rtB -> B_120_365_0 - rtb_B_120_5_0 ; if (
rateLimiterRate > riseValLimit ) { _rtB -> B_120_366_0 = rtb_B_120_5_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_274 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_120_366_0 = rtb_B_120_5_0 + deltaT ; } else { _rtB ->
B_120_366_0 = _rtB -> B_120_365_0 ; } } } isHit = ssIsSampleHit ( S , 2 , 0 )
; if ( isHit != 0 ) { _rtB -> B_120_367_0 = _rtP -> P_275 * _rtB ->
B_120_357_0 ; } _rtB -> B_120_368_0 = _rtB -> B_120_366_0 * _rtB ->
B_120_367_0 ; _rtB -> B_120_369_0 = _rtB -> B_120_359_0 * _rtB -> B_120_368_0
; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model1/Data Store Read" ,
0 ) ; _rtB -> B_120_370_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_o != 0 )
{ _rtX -> vv0at_CSTATE_p = _rtB -> B_120_370_0 ; } _rtB -> B_120_371_0 = _rtX
-> vv0at_CSTATE_p ; _rtB -> B_120_372_0 = muDoubleScalarAbs ( _rtB ->
B_120_371_0 ) ; _rtB -> B_120_373_0 = _rtP -> P_276 [ 0 ] ; _rtB ->
B_120_373_0 = _rtB -> B_120_373_0 * _rtB -> B_120_372_0 + _rtP -> P_276 [ 1 ]
; _rtB -> B_120_373_0 = _rtB -> B_120_373_0 * _rtB -> B_120_372_0 + _rtP ->
P_276 [ 2 ] ; _rtB -> B_120_374_0 = _rtP -> P_277 * _rtB -> B_120_373_0 ;
_rtB -> B_120_375_0 = _rtB -> B_120_357_0 * _rtB -> B_120_374_0 ; _rtB ->
B_120_376_0 = _rtB -> B_120_369_0 - _rtB -> B_120_375_0 ; isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB -> B_120_378_0 = _rtP
-> P_278 * _rtB -> B_120_358_0 ; } if ( _rtB -> B_120_376_0 > _rtB ->
B_120_358_0 ) { _rtB -> B_120_380_0 = _rtB -> B_120_358_0 ; } else { if (
_rtB -> B_120_376_0 < _rtB -> B_120_378_0 ) { _rtB -> B_32_1_0 = _rtB ->
B_120_378_0 ; } else { _rtB -> B_32_1_0 = _rtB -> B_120_376_0 ; } _rtB ->
B_120_380_0 = _rtB -> B_32_1_0 ; } if ( _rtB -> B_120_90_0 ) { _rtB ->
B_120_381_0 = _rtB -> B_120_380_0 ; } else { _rtB -> B_120_381_0 = _rtB ->
B_120_89_0_n ; } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) {
ssCallAccelRunBlock ( S , 40 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP ->
P_1004 != 0 ) && ( ! ( _rtB -> B_40_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_1004
== 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_c == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model4/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_c < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_c ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model4/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_109_0_h ) ; if ( ( _rtP -> P_1004 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_1004 == 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_c == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model4/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_c < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_c ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model4/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_109_0_h ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_109_0_h ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_383_0 =
_rtB -> B_120_108_0_j [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_40_0_0 , 499U ) ] ; _rtB -> B_120_384_0 =
_rtB -> B_120_115_0_c * _rtB -> B_120_383_0 ; if ( ( _rtP -> P_1005 != 0 ) &&
( ! ( _rtB -> B_40_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_1005 == 1 ) { if (
_rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_eb == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_eb < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_eb ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_110_0_c ) ; if ( ( _rtP -> P_1005 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_1005 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_eb == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_eb < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_eb ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_110_0_c ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_110_0_c ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_385_0 =
_rtB -> B_120_108_0_j [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_40_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_pc . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_pc .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_279 ; _rtB -> B_120_386_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_d . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_d . Last , _rtDW -> TransportDelay_IWORK_d . Tail ,
_rtDW -> TransportDelay_IWORK_d . Head , _rtP -> P_280 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_120_387_0 = 0.0 ; _rtB -> B_120_387_0 += _rtP -> P_282 * _rtX
-> TransferFcn_CSTATE_l3 ; _rtB -> B_120_387_0 += _rtP -> P_283 * _rtB ->
B_120_386_0 ; if ( _rtB -> B_120_90_0 ) { if ( _rtB -> B_120_387_0 > _rtP ->
P_21 ) { _rtB -> B_30_0_0 = _rtB -> B_120_387_0 - _rtP -> P_21 ; } else if (
_rtB -> B_120_387_0 >= _rtP -> P_20 ) { _rtB -> B_30_0_0 = 0.0 ; } else {
_rtB -> B_30_0_0 = _rtB -> B_120_387_0 - _rtP -> P_20 ; } _rtB -> B_120_389_0
= _rtB -> B_30_0_0 ; } else { _rtB -> B_120_389_0 = _rtB -> B_120_89_0_n ; }
_rtB -> B_120_390_0 = _rtP -> P_284 * _rtB -> B_120_389_0 ; if ( _rtB ->
B_120_390_0 > _rtP -> P_285 ) { _rtB -> B_120_391_0 = _rtP -> P_285 ; } else
if ( _rtB -> B_120_390_0 < _rtP -> P_286 ) { _rtB -> B_120_391_0 = _rtP ->
P_286 ; } else { _rtB -> B_120_391_0 = _rtB -> B_120_390_0 ; } if ( ( _rtDW
-> LastMajorTimeA_hl >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW ->
LastMajorTimeB_ga >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_120_392_0 = _rtB
-> B_120_391_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_hl < _rtDW ->
LastMajorTimeB_ga ) && ( _rtDW -> LastMajorTimeB_ga < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_hl >= _rtDW -> LastMajorTimeB_ga ) && (
_rtDW -> LastMajorTimeA_hl >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_ga ; rtb_B_120_5_0 = _rtDW
-> PrevYB_al ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_hl ; rtb_B_120_5_0 = _rtDW -> PrevYA_ir ; } riseValLimit =
deltaT * _rtP -> P_287 ; rateLimiterRate = _rtB -> B_120_391_0 -
rtb_B_120_5_0 ; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_120_392_0 =
rtb_B_120_5_0 + riseValLimit ; } else { deltaT *= _rtP -> P_288 ; if (
rateLimiterRate < deltaT ) { _rtB -> B_120_392_0 = rtb_B_120_5_0 + deltaT ; }
else { _rtB -> B_120_392_0 = _rtB -> B_120_391_0 ; } } } isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB -> B_120_393_0 = _rtP
-> P_289 * _rtB -> B_120_383_0 ; } _rtB -> B_120_394_0 = _rtB -> B_120_392_0
* _rtB -> B_120_393_0 ; _rtB -> B_120_395_0 = _rtB -> B_120_385_0 * _rtB ->
B_120_394_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model4/Data Store Read" ,
0 ) ; _rtB -> B_120_396_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_i != 0 )
{ _rtX -> vv0at_CSTATE_h3 = _rtB -> B_120_396_0 ; } _rtB -> B_120_397_0 =
_rtX -> vv0at_CSTATE_h3 ; _rtB -> B_120_398_0 = muDoubleScalarAbs ( _rtB ->
B_120_397_0 ) ; _rtB -> B_120_399_0 = _rtP -> P_290 [ 0 ] ; _rtB ->
B_120_399_0 = _rtB -> B_120_399_0 * _rtB -> B_120_398_0 + _rtP -> P_290 [ 1 ]
; _rtB -> B_120_399_0 = _rtB -> B_120_399_0 * _rtB -> B_120_398_0 + _rtP ->
P_290 [ 2 ] ; _rtB -> B_120_400_0 = _rtP -> P_291 * _rtB -> B_120_399_0 ;
_rtB -> B_120_401_0 = _rtB -> B_120_383_0 * _rtB -> B_120_400_0 ; _rtB ->
B_120_402_0 = _rtB -> B_120_395_0 - _rtB -> B_120_401_0 ; isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB -> B_120_404_0 = _rtP
-> P_292 * _rtB -> B_120_384_0 ; } if ( _rtB -> B_120_402_0 > _rtB ->
B_120_384_0 ) { _rtB -> B_120_406_0 = _rtB -> B_120_384_0 ; } else { if (
_rtB -> B_120_402_0 < _rtB -> B_120_404_0 ) { _rtB -> B_41_1_0 = _rtB ->
B_120_404_0 ; } else { _rtB -> B_41_1_0 = _rtB -> B_120_402_0 ; } _rtB ->
B_120_406_0 = _rtB -> B_41_1_0 ; } if ( _rtB -> B_120_114_0 ) { _rtB ->
B_120_407_0 = _rtB -> B_120_406_0 ; } else { _rtB -> B_120_407_0 = _rtB ->
B_120_113_0_c ; } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) {
ssCallAccelRunBlock ( S , 58 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP ->
P_1006 != 0 ) && ( ! ( _rtB -> B_58_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_1006
== 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_e == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model5/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_e < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_e ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model5/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_157_0_l ) ; if ( ( _rtP -> P_1006 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_1006 == 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_e == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model5/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_e < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_e ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model5/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_157_0_l ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_157_0_l ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_409_0 =
_rtB -> B_120_156_0_o [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_58_0_0 , 499U ) ] ; _rtB -> B_120_410_0 =
_rtB -> B_120_163_0_m * _rtB -> B_120_409_0 ; if ( ( _rtP -> P_1007 != 0 ) &&
( ! ( _rtB -> B_58_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_1007 == 1 ) { if (
_rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_n == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model5/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_n < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_n ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model5/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_158_0_m ) ; if ( ( _rtP -> P_1007 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_1007 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_n == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model5/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_n < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_n ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model5/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_158_0_m ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_158_0_m ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_411_0 =
_rtB -> B_120_156_0_o [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_58_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_c2 . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_c2 .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_293 ; _rtB -> B_120_412_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_a . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_a . Last , _rtDW -> TransportDelay_IWORK_a . Tail ,
_rtDW -> TransportDelay_IWORK_a . Head , _rtP -> P_294 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_120_413_0 = 0.0 ; _rtB -> B_120_413_0 += _rtP -> P_296 * _rtX
-> TransferFcn_CSTATE_i ; _rtB -> B_120_413_0 += _rtP -> P_297 * _rtB ->
B_120_412_0 ; if ( _rtB -> B_120_114_0 ) { if ( _rtB -> B_120_413_0 > _rtP ->
P_27 ) { _rtB -> B_39_0_0 = _rtB -> B_120_413_0 - _rtP -> P_27 ; } else if (
_rtB -> B_120_413_0 >= _rtP -> P_26 ) { _rtB -> B_39_0_0 = 0.0 ; } else {
_rtB -> B_39_0_0 = _rtB -> B_120_413_0 - _rtP -> P_26 ; } _rtB -> B_120_415_0
= _rtB -> B_39_0_0 ; } else { _rtB -> B_120_415_0 = _rtB -> B_120_113_0_c ; }
_rtB -> B_120_416_0 = _rtP -> P_298 * _rtB -> B_120_415_0 ; if ( _rtB ->
B_120_416_0 > _rtP -> P_299 ) { _rtB -> B_120_417_0 = _rtP -> P_299 ; } else
if ( _rtB -> B_120_416_0 < _rtP -> P_300 ) { _rtB -> B_120_417_0 = _rtP ->
P_300 ; } else { _rtB -> B_120_417_0 = _rtB -> B_120_416_0 ; } if ( ( _rtDW
-> LastMajorTimeA_ix >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW ->
LastMajorTimeB_mr >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_120_418_0 = _rtB
-> B_120_417_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_ix < _rtDW ->
LastMajorTimeB_mr ) && ( _rtDW -> LastMajorTimeB_mr < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_ix >= _rtDW -> LastMajorTimeB_mr ) && (
_rtDW -> LastMajorTimeA_ix >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_mr ; rtb_B_120_5_0 = _rtDW
-> PrevYB_c ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_ix ; rtb_B_120_5_0 = _rtDW -> PrevYA_mo ; } riseValLimit =
deltaT * _rtP -> P_301 ; rateLimiterRate = _rtB -> B_120_417_0 -
rtb_B_120_5_0 ; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_120_418_0 =
rtb_B_120_5_0 + riseValLimit ; } else { deltaT *= _rtP -> P_302 ; if (
rateLimiterRate < deltaT ) { _rtB -> B_120_418_0 = rtb_B_120_5_0 + deltaT ; }
else { _rtB -> B_120_418_0 = _rtB -> B_120_417_0 ; } } } isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB -> B_120_419_0 = _rtP
-> P_303 * _rtB -> B_120_409_0 ; } _rtB -> B_120_420_0 = _rtB -> B_120_418_0
* _rtB -> B_120_419_0 ; _rtB -> B_120_421_0 = _rtB -> B_120_411_0 * _rtB ->
B_120_420_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model5/Data Store Read" ,
0 ) ; _rtB -> B_120_422_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_ie != 0 )
{ _rtX -> vv0at_CSTATE_l = _rtB -> B_120_422_0 ; } _rtB -> B_120_423_0 = _rtX
-> vv0at_CSTATE_l ; _rtB -> B_120_424_0 = muDoubleScalarAbs ( _rtB ->
B_120_423_0 ) ; _rtB -> B_120_425_0 = _rtP -> P_304 [ 0 ] ; _rtB ->
B_120_425_0 = _rtB -> B_120_425_0 * _rtB -> B_120_424_0 + _rtP -> P_304 [ 1 ]
; _rtB -> B_120_425_0 = _rtB -> B_120_425_0 * _rtB -> B_120_424_0 + _rtP ->
P_304 [ 2 ] ; _rtB -> B_120_426_0 = _rtP -> P_305 * _rtB -> B_120_425_0 ;
_rtB -> B_120_427_0 = _rtB -> B_120_409_0 * _rtB -> B_120_426_0 ; _rtB ->
B_120_428_0 = _rtB -> B_120_421_0 - _rtB -> B_120_427_0 ; isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB -> B_120_430_0 = _rtP
-> P_306 * _rtB -> B_120_410_0 ; } if ( _rtB -> B_120_428_0 > _rtB ->
B_120_410_0 ) { _rtB -> B_120_432_0 = _rtB -> B_120_410_0 ; } else { if (
_rtB -> B_120_428_0 < _rtB -> B_120_430_0 ) { _rtB -> B_59_1_0 = _rtB ->
B_120_430_0 ; } else { _rtB -> B_59_1_0 = _rtB -> B_120_428_0 ; } _rtB ->
B_120_432_0 = _rtB -> B_59_1_0 ; } if ( _rtB -> B_120_162_0_c ) { _rtB ->
B_120_433_0 = _rtB -> B_120_432_0 ; } else { _rtB -> B_120_433_0 = _rtB ->
B_120_161_0_m ; } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) {
ssCallAccelRunBlock ( S , 52 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP ->
P_1008 != 0 ) && ( ! ( _rtB -> B_52_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_1008
== 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_o2 == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model3/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_o2 < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_o2 ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model3/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_141_0_i ) ; if ( ( _rtP -> P_1008 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_1008 == 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_o2 == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model3/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_o2 < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_o2 ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model3/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_141_0_i ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_141_0_i ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_435_0 =
_rtB -> B_120_140_0 [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_52_0_0 , 499U ) ] ; _rtB -> B_120_436_0 =
_rtB -> B_120_147_0_f * _rtB -> B_120_435_0 ; if ( ( _rtP -> P_1009 != 0 ) &&
( ! ( _rtB -> B_52_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_1009 == 1 ) { if (
_rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_f == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_f < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_f ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_142_0 ) ; if ( ( _rtP -> P_1009 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_1009 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_f == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_f < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_f ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_142_0 ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_142_0 ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_437_0 =
_rtB -> B_120_140_0 [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_52_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_je . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_je .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_307 ; _rtB -> B_120_438_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_dg . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_dg . Last , _rtDW -> TransportDelay_IWORK_dg . Tail ,
_rtDW -> TransportDelay_IWORK_dg . Head , _rtP -> P_308 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_120_439_0 = 0.0 ; _rtB -> B_120_439_0 += _rtP -> P_310 * _rtX
-> TransferFcn_CSTATE_c ; _rtB -> B_120_439_0 += _rtP -> P_311 * _rtB ->
B_120_438_0 ; if ( _rtB -> B_120_162_0_c ) { if ( _rtB -> B_120_439_0 > _rtP
-> P_39 ) { _rtB -> B_57_0_0 = _rtB -> B_120_439_0 - _rtP -> P_39 ; } else if
( _rtB -> B_120_439_0 >= _rtP -> P_38 ) { _rtB -> B_57_0_0 = 0.0 ; } else {
_rtB -> B_57_0_0 = _rtB -> B_120_439_0 - _rtP -> P_38 ; } _rtB -> B_120_441_0
= _rtB -> B_57_0_0 ; } else { _rtB -> B_120_441_0 = _rtB -> B_120_161_0_m ; }
_rtB -> B_120_442_0 = _rtP -> P_312 * _rtB -> B_120_441_0 ; if ( _rtB ->
B_120_442_0 > _rtP -> P_313 ) { _rtB -> B_120_443_0 = _rtP -> P_313 ; } else
if ( _rtB -> B_120_442_0 < _rtP -> P_314 ) { _rtB -> B_120_443_0 = _rtP ->
P_314 ; } else { _rtB -> B_120_443_0 = _rtB -> B_120_442_0 ; } if ( ( _rtDW
-> LastMajorTimeA_l >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW ->
LastMajorTimeB_fh >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_120_444_0 = _rtB
-> B_120_443_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_l < _rtDW ->
LastMajorTimeB_fh ) && ( _rtDW -> LastMajorTimeB_fh < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_l >= _rtDW -> LastMajorTimeB_fh ) && (
_rtDW -> LastMajorTimeA_l >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_fh ; rtb_B_120_5_0 = _rtDW
-> PrevYB_k ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_l ; rtb_B_120_5_0 = _rtDW -> PrevYA_f ; } riseValLimit =
deltaT * _rtP -> P_315 ; rateLimiterRate = _rtB -> B_120_443_0 -
rtb_B_120_5_0 ; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_120_444_0 =
rtb_B_120_5_0 + riseValLimit ; } else { deltaT *= _rtP -> P_316 ; if (
rateLimiterRate < deltaT ) { _rtB -> B_120_444_0 = rtb_B_120_5_0 + deltaT ; }
else { _rtB -> B_120_444_0 = _rtB -> B_120_443_0 ; } } } isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB -> B_120_445_0 = _rtP
-> P_317 * _rtB -> B_120_435_0 ; } _rtB -> B_120_446_0 = _rtB -> B_120_444_0
* _rtB -> B_120_445_0 ; _rtB -> B_120_447_0 = _rtB -> B_120_437_0 * _rtB ->
B_120_446_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model3/Data Store Read" ,
0 ) ; _rtB -> B_120_448_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_gs != 0 )
{ _rtX -> vv0at_CSTATE_fc = _rtB -> B_120_448_0 ; } _rtB -> B_120_449_0 =
_rtX -> vv0at_CSTATE_fc ; _rtB -> B_120_450_0 = muDoubleScalarAbs ( _rtB ->
B_120_449_0 ) ; _rtB -> B_120_451_0 = _rtP -> P_318 [ 0 ] ; _rtB ->
B_120_451_0 = _rtB -> B_120_451_0 * _rtB -> B_120_450_0 + _rtP -> P_318 [ 1 ]
; _rtB -> B_120_451_0 = _rtB -> B_120_451_0 * _rtB -> B_120_450_0 + _rtP ->
P_318 [ 2 ] ; _rtB -> B_120_452_0 = _rtP -> P_319 * _rtB -> B_120_451_0 ;
_rtB -> B_120_453_0 = _rtB -> B_120_435_0 * _rtB -> B_120_452_0 ; _rtB ->
B_120_454_0 = _rtB -> B_120_447_0 - _rtB -> B_120_453_0 ; isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB -> B_120_456_0 = _rtP
-> P_320 * _rtB -> B_120_436_0 ; } if ( _rtB -> B_120_454_0 > _rtB ->
B_120_436_0 ) { _rtB -> B_120_458_0 = _rtB -> B_120_436_0 ; } else { if (
_rtB -> B_120_454_0 < _rtB -> B_120_456_0 ) { _rtB -> B_53_1_0 = _rtB ->
B_120_456_0 ; } else { _rtB -> B_53_1_0 = _rtB -> B_120_454_0 ; } _rtB ->
B_120_458_0 = _rtB -> B_53_1_0 ; } if ( _rtB -> B_120_146_0_h ) { _rtB ->
B_120_459_0 = _rtB -> B_120_458_0 ; } else { _rtB -> B_120_459_0 = _rtB ->
B_120_145_0 ; } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) {
ssCallAccelRunBlock ( S , 49 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP ->
P_1010 != 0 ) && ( ! ( _rtB -> B_49_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_1010
== 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_i == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model2/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_i < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_i ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model2/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_133_0_l ) ; if ( ( _rtP -> P_1010 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_1010 == 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_i == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model2/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_i < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_i ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model2/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_133_0_l ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_133_0_l ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_461_0 =
_rtB -> B_120_132_0_i [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_49_0_0 , 499U ) ] ; _rtB -> B_120_462_0 =
_rtB -> B_120_139_0_o * _rtB -> B_120_461_0 ; if ( ( _rtP -> P_1011 != 0 ) &&
( ! ( _rtB -> B_49_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_1011 == 1 ) { if (
_rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_eo == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_eo < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_eo ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_134_0_o ) ; if ( ( _rtP -> P_1011 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_1011 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_eo == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_eo < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_eo ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_134_0_o ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_134_0_o ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_463_0 =
_rtB -> B_120_132_0_i [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_49_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_i5 . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_i5 .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_321 ; _rtB -> B_120_464_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_gj . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_gj . Last , _rtDW -> TransportDelay_IWORK_gj . Tail ,
_rtDW -> TransportDelay_IWORK_gj . Head , _rtP -> P_322 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_120_465_0 = 0.0 ; _rtB -> B_120_465_0 += _rtP -> P_324 * _rtX
-> TransferFcn_CSTATE_b ; _rtB -> B_120_465_0 += _rtP -> P_325 * _rtB ->
B_120_464_0 ; if ( _rtB -> B_120_146_0_h ) { if ( _rtB -> B_120_465_0 > _rtP
-> P_35 ) { _rtB -> B_51_0_0 = _rtB -> B_120_465_0 - _rtP -> P_35 ; } else if
( _rtB -> B_120_465_0 >= _rtP -> P_34 ) { _rtB -> B_51_0_0 = 0.0 ; } else {
_rtB -> B_51_0_0 = _rtB -> B_120_465_0 - _rtP -> P_34 ; } _rtB -> B_120_467_0
= _rtB -> B_51_0_0 ; } else { _rtB -> B_120_467_0 = _rtB -> B_120_145_0 ; }
_rtB -> B_120_468_0 = _rtP -> P_326 * _rtB -> B_120_467_0 ; if ( _rtB ->
B_120_468_0 > _rtP -> P_327 ) { _rtB -> B_120_469_0 = _rtP -> P_327 ; } else
if ( _rtB -> B_120_468_0 < _rtP -> P_328 ) { _rtB -> B_120_469_0 = _rtP ->
P_328 ; } else { _rtB -> B_120_469_0 = _rtB -> B_120_468_0 ; } if ( ( _rtDW
-> LastMajorTimeA_nu >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW ->
LastMajorTimeB_m5 >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_120_470_0 = _rtB
-> B_120_469_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_nu < _rtDW ->
LastMajorTimeB_m5 ) && ( _rtDW -> LastMajorTimeB_m5 < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_nu >= _rtDW -> LastMajorTimeB_m5 ) && (
_rtDW -> LastMajorTimeA_nu >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_m5 ; rtb_B_120_5_0 = _rtDW
-> PrevYB_m ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_nu ; rtb_B_120_5_0 = _rtDW -> PrevYA_im ; } riseValLimit =
deltaT * _rtP -> P_329 ; rateLimiterRate = _rtB -> B_120_469_0 -
rtb_B_120_5_0 ; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_120_470_0 =
rtb_B_120_5_0 + riseValLimit ; } else { deltaT *= _rtP -> P_330 ; if (
rateLimiterRate < deltaT ) { _rtB -> B_120_470_0 = rtb_B_120_5_0 + deltaT ; }
else { _rtB -> B_120_470_0 = _rtB -> B_120_469_0 ; } } } isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB -> B_120_471_0 = _rtP
-> P_331 * _rtB -> B_120_461_0 ; } _rtB -> B_120_472_0 = _rtB -> B_120_470_0
* _rtB -> B_120_471_0 ; _rtB -> B_120_473_0 = _rtB -> B_120_463_0 * _rtB ->
B_120_472_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model2/Data Store Read" ,
0 ) ; _rtB -> B_120_474_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_pa != 0 )
{ _rtX -> vv0at_CSTATE_a = _rtB -> B_120_474_0 ; } _rtB -> B_120_475_0 = _rtX
-> vv0at_CSTATE_a ; _rtB -> B_120_476_0 = muDoubleScalarAbs ( _rtB ->
B_120_475_0 ) ; _rtB -> B_120_477_0 = _rtP -> P_332 [ 0 ] ; _rtB ->
B_120_477_0 = _rtB -> B_120_477_0 * _rtB -> B_120_476_0 + _rtP -> P_332 [ 1 ]
; _rtB -> B_120_477_0 = _rtB -> B_120_477_0 * _rtB -> B_120_476_0 + _rtP ->
P_332 [ 2 ] ; _rtB -> B_120_478_0 = _rtP -> P_333 * _rtB -> B_120_477_0 ;
_rtB -> B_120_479_0 = _rtB -> B_120_461_0 * _rtB -> B_120_478_0 ; _rtB ->
B_120_480_0 = _rtB -> B_120_473_0 - _rtB -> B_120_479_0 ; isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB -> B_120_482_0 = _rtP
-> P_334 * _rtB -> B_120_462_0 ; } if ( _rtB -> B_120_480_0 > _rtB ->
B_120_462_0 ) { _rtB -> B_120_484_0 = _rtB -> B_120_462_0 ; } else { if (
_rtB -> B_120_480_0 < _rtB -> B_120_482_0 ) { _rtB -> B_50_1_0 = _rtB ->
B_120_482_0 ; } else { _rtB -> B_50_1_0 = _rtB -> B_120_480_0 ; } _rtB ->
B_120_484_0 = _rtB -> B_50_1_0 ; } if ( _rtB -> B_120_138_0_c ) { _rtB ->
B_120_485_0 = _rtB -> B_120_484_0 ; } else { _rtB -> B_120_485_0 = _rtB ->
B_120_137_0_j ; } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) {
ssCallAccelRunBlock ( S , 46 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP ->
P_1012 != 0 ) && ( ! ( _rtB -> B_46_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_1012
== 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_f == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model1/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_f < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_f ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model1/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_125_0 ) ; if ( ( _rtP -> P_1012 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_1012 == 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_f == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model1/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_f < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_f ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model1/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_125_0 ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_125_0 ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_487_0 =
_rtB -> B_120_124_0_e [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_46_0_0 , 499U ) ] ; _rtB -> B_120_488_0 =
_rtB -> B_120_131_0_a * _rtB -> B_120_487_0 ; if ( ( _rtP -> P_1013 != 0 ) &&
( ! ( _rtB -> B_46_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_1013 == 1 ) { if (
_rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_i == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_i < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_i ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_126_0_a ) ; if ( ( _rtP -> P_1013 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_1013 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_i == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_i < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_i ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_126_0_a ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_126_0_a ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_489_0 =
_rtB -> B_120_124_0_e [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_46_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_nh . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_nh .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_335 ; _rtB -> B_120_490_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_dp . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_dp . Last , _rtDW -> TransportDelay_IWORK_dp . Tail ,
_rtDW -> TransportDelay_IWORK_dp . Head , _rtP -> P_336 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_120_491_0 = 0.0 ; _rtB -> B_120_491_0 += _rtP -> P_338 * _rtX
-> TransferFcn_CSTATE_m1 ; _rtB -> B_120_491_0 += _rtP -> P_339 * _rtB ->
B_120_490_0 ; if ( _rtB -> B_120_138_0_c ) { if ( _rtB -> B_120_491_0 > _rtP
-> P_33 ) { _rtB -> B_48_0_0 = _rtB -> B_120_491_0 - _rtP -> P_33 ; } else if
( _rtB -> B_120_491_0 >= _rtP -> P_32 ) { _rtB -> B_48_0_0 = 0.0 ; } else {
_rtB -> B_48_0_0 = _rtB -> B_120_491_0 - _rtP -> P_32 ; } _rtB -> B_120_493_0
= _rtB -> B_48_0_0 ; } else { _rtB -> B_120_493_0 = _rtB -> B_120_137_0_j ; }
_rtB -> B_120_494_0 = _rtP -> P_340 * _rtB -> B_120_493_0 ; if ( _rtB ->
B_120_494_0 > _rtP -> P_341 ) { _rtB -> B_120_495_0 = _rtP -> P_341 ; } else
if ( _rtB -> B_120_494_0 < _rtP -> P_342 ) { _rtB -> B_120_495_0 = _rtP ->
P_342 ; } else { _rtB -> B_120_495_0 = _rtB -> B_120_494_0 ; } if ( ( _rtDW
-> LastMajorTimeA_j >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW ->
LastMajorTimeB_as >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_120_496_0 = _rtB
-> B_120_495_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_j < _rtDW ->
LastMajorTimeB_as ) && ( _rtDW -> LastMajorTimeB_as < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_j >= _rtDW -> LastMajorTimeB_as ) && (
_rtDW -> LastMajorTimeA_j >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_as ; rtb_B_120_5_0 = _rtDW
-> PrevYB_k5 ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_j ; rtb_B_120_5_0 = _rtDW -> PrevYA_k ; } riseValLimit =
deltaT * _rtP -> P_343 ; rateLimiterRate = _rtB -> B_120_495_0 -
rtb_B_120_5_0 ; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_120_496_0 =
rtb_B_120_5_0 + riseValLimit ; } else { deltaT *= _rtP -> P_344 ; if (
rateLimiterRate < deltaT ) { _rtB -> B_120_496_0 = rtb_B_120_5_0 + deltaT ; }
else { _rtB -> B_120_496_0 = _rtB -> B_120_495_0 ; } } } isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB -> B_120_497_0 = _rtP
-> P_345 * _rtB -> B_120_487_0 ; } _rtB -> B_120_498_0 = _rtB -> B_120_496_0
* _rtB -> B_120_497_0 ; _rtB -> B_120_499_0 = _rtB -> B_120_489_0 * _rtB ->
B_120_498_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model1/Data Store Read" ,
0 ) ; _rtB -> B_120_500_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_n != 0 )
{ _rtX -> vv0at_CSTATE_h3x = _rtB -> B_120_500_0 ; } _rtB -> B_120_501_0 =
_rtX -> vv0at_CSTATE_h3x ; _rtB -> B_120_502_0 = muDoubleScalarAbs ( _rtB ->
B_120_501_0 ) ; _rtB -> B_120_503_0 = _rtP -> P_346 [ 0 ] ; _rtB ->
B_120_503_0 = _rtB -> B_120_503_0 * _rtB -> B_120_502_0 + _rtP -> P_346 [ 1 ]
; _rtB -> B_120_503_0 = _rtB -> B_120_503_0 * _rtB -> B_120_502_0 + _rtP ->
P_346 [ 2 ] ; _rtB -> B_120_504_0 = _rtP -> P_347 * _rtB -> B_120_503_0 ;
_rtB -> B_120_505_0 = _rtB -> B_120_487_0 * _rtB -> B_120_504_0 ; _rtB ->
B_120_506_0 = _rtB -> B_120_499_0 - _rtB -> B_120_505_0 ; isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB -> B_120_508_0 = _rtP
-> P_348 * _rtB -> B_120_488_0 ; } if ( _rtB -> B_120_506_0 > _rtB ->
B_120_488_0 ) { _rtB -> B_120_510_0 = _rtB -> B_120_488_0 ; } else { if (
_rtB -> B_120_506_0 < _rtB -> B_120_508_0 ) { _rtB -> B_47_1_0 = _rtB ->
B_120_508_0 ; } else { _rtB -> B_47_1_0 = _rtB -> B_120_506_0 ; } _rtB ->
B_120_510_0 = _rtB -> B_47_1_0 ; } if ( _rtB -> B_120_130_0_j ) { _rtB ->
B_120_511_0 = _rtB -> B_120_510_0 ; } else { _rtB -> B_120_511_0 = _rtB ->
B_120_129_0_m ; } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) {
ssCallAccelRunBlock ( S , 55 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP ->
P_1014 != 0 ) && ( ! ( _rtB -> B_55_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_1014
== 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_i5 == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model4/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_i5 < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_i5 ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model4/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_149_0_f ) ; if ( ( _rtP -> P_1014 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_1014 == 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_i5 == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model4/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_i5 < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_i5 ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model4/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_149_0_f ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_149_0_f ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_513_0 =
_rtB -> B_120_148_0_i [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_55_0_0 , 499U ) ] ; _rtB -> B_120_514_0 =
_rtB -> B_120_155_0_c * _rtB -> B_120_513_0 ; if ( ( _rtP -> P_1015 != 0 ) &&
( ! ( _rtB -> B_55_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_1015 == 1 ) { if (
_rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_d0 == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_d0 < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_d0 ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_150_0_g ) ; if ( ( _rtP -> P_1015 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_1015 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_d0 == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_d0 < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_d0 ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_150_0_g ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_150_0_g ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_515_0 =
_rtB -> B_120_148_0_i [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_55_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_k . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_k .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_349 ; _rtB -> B_120_516_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_b . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_b . Last , _rtDW -> TransportDelay_IWORK_b . Tail ,
_rtDW -> TransportDelay_IWORK_b . Head , _rtP -> P_350 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_120_517_0 = 0.0 ; _rtB -> B_120_517_0 += _rtP -> P_352 * _rtX
-> TransferFcn_CSTATE_g ; _rtB -> B_120_517_0 += _rtP -> P_353 * _rtB ->
B_120_516_0 ; if ( _rtB -> B_120_130_0_j ) { if ( _rtB -> B_120_517_0 > _rtP
-> P_31 ) { _rtB -> B_45_0_0 = _rtB -> B_120_517_0 - _rtP -> P_31 ; } else if
( _rtB -> B_120_517_0 >= _rtP -> P_30 ) { _rtB -> B_45_0_0 = 0.0 ; } else {
_rtB -> B_45_0_0 = _rtB -> B_120_517_0 - _rtP -> P_30 ; } _rtB -> B_120_519_0
= _rtB -> B_45_0_0 ; } else { _rtB -> B_120_519_0 = _rtB -> B_120_129_0_m ; }
_rtB -> B_120_520_0 = _rtP -> P_354 * _rtB -> B_120_519_0 ; if ( _rtB ->
B_120_520_0 > _rtP -> P_355 ) { _rtB -> B_120_521_0 = _rtP -> P_355 ; } else
if ( _rtB -> B_120_520_0 < _rtP -> P_356 ) { _rtB -> B_120_521_0 = _rtP ->
P_356 ; } else { _rtB -> B_120_521_0 = _rtB -> B_120_520_0 ; } if ( ( _rtDW
-> LastMajorTimeA_p >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW ->
LastMajorTimeB_l >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_120_522_0 = _rtB
-> B_120_521_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_p < _rtDW ->
LastMajorTimeB_l ) && ( _rtDW -> LastMajorTimeB_l < ssGetTaskTime ( S , 0 ) )
) || ( ( _rtDW -> LastMajorTimeA_p >= _rtDW -> LastMajorTimeB_l ) && ( _rtDW
-> LastMajorTimeA_p >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime
( S , 0 ) - _rtDW -> LastMajorTimeB_l ; rtb_B_120_5_0 = _rtDW -> PrevYB_mx ;
} else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_p ;
rtb_B_120_5_0 = _rtDW -> PrevYA_kv ; } riseValLimit = deltaT * _rtP -> P_357
; rateLimiterRate = _rtB -> B_120_521_0 - rtb_B_120_5_0 ; if (
rateLimiterRate > riseValLimit ) { _rtB -> B_120_522_0 = rtb_B_120_5_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_358 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_120_522_0 = rtb_B_120_5_0 + deltaT ; } else { _rtB ->
B_120_522_0 = _rtB -> B_120_521_0 ; } } } isHit = ssIsSampleHit ( S , 2 , 0 )
; if ( isHit != 0 ) { _rtB -> B_120_523_0 = _rtP -> P_359 * _rtB ->
B_120_513_0 ; } _rtB -> B_120_524_0 = _rtB -> B_120_522_0 * _rtB ->
B_120_523_0 ; _rtB -> B_120_525_0 = _rtB -> B_120_515_0 * _rtB -> B_120_524_0
; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model4/Data Store Read" ,
0 ) ; _rtB -> B_120_526_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_p5 != 0 )
{ _rtX -> vv0at_CSTATE_ao = _rtB -> B_120_526_0 ; } _rtB -> B_120_527_0 =
_rtX -> vv0at_CSTATE_ao ; _rtB -> B_120_528_0 = muDoubleScalarAbs ( _rtB ->
B_120_527_0 ) ; _rtB -> B_120_529_0 = _rtP -> P_360 [ 0 ] ; _rtB ->
B_120_529_0 = _rtB -> B_120_529_0 * _rtB -> B_120_528_0 + _rtP -> P_360 [ 1 ]
; _rtB -> B_120_529_0 = _rtB -> B_120_529_0 * _rtB -> B_120_528_0 + _rtP ->
P_360 [ 2 ] ; _rtB -> B_120_530_0 = _rtP -> P_361 * _rtB -> B_120_529_0 ;
_rtB -> B_120_531_0 = _rtB -> B_120_513_0 * _rtB -> B_120_530_0 ; _rtB ->
B_120_532_0 = _rtB -> B_120_525_0 - _rtB -> B_120_531_0 ; isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB -> B_120_534_0 = _rtP
-> P_362 * _rtB -> B_120_514_0 ; } if ( _rtB -> B_120_532_0 > _rtB ->
B_120_514_0 ) { _rtB -> B_120_536_0 = _rtB -> B_120_514_0 ; } else { if (
_rtB -> B_120_532_0 < _rtB -> B_120_534_0 ) { _rtB -> B_56_1_0 = _rtB ->
B_120_534_0 ; } else { _rtB -> B_56_1_0 = _rtB -> B_120_532_0 ; } _rtB ->
B_120_536_0 = _rtB -> B_56_1_0 ; } if ( _rtB -> B_120_154_0_d ) { _rtB ->
B_120_537_0 = _rtB -> B_120_536_0 ; } else { _rtB -> B_120_537_0 = _rtB ->
B_120_153_0_k ; } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) {
ssCallAccelRunBlock ( S , 73 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP ->
P_1016 != 0 ) && ( ! ( _rtB -> B_73_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_1016
== 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_ia == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model5/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_ia < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_ia ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model5/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_197_0 ) ; if ( ( _rtP -> P_1016 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_1016 == 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_ia == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model5/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_ia < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_ia ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model5/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_197_0 ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_197_0 ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_539_0 =
_rtB -> B_120_196_0_k [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_73_0_0 , 499U ) ] ; _rtB -> B_120_540_0 =
_rtB -> B_120_203_0_p * _rtB -> B_120_539_0 ; if ( ( _rtP -> P_1017 != 0 ) &&
( ! ( _rtB -> B_73_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_1017 == 1 ) { if (
_rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_l3 == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model5/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_l3 < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_l3 ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model5/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_198_0_p ) ; if ( ( _rtP -> P_1017 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_1017 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_l3 == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model5/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_l3 < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_l3 ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model5/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_198_0_p ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_198_0_p ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_541_0 =
_rtB -> B_120_196_0_k [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_73_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_av . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_av .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_363 ; _rtB -> B_120_542_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_ai . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_ai . Last , _rtDW -> TransportDelay_IWORK_ai . Tail ,
_rtDW -> TransportDelay_IWORK_ai . Head , _rtP -> P_364 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_120_543_0 = 0.0 ; _rtB -> B_120_543_0 += _rtP -> P_366 * _rtX
-> TransferFcn_CSTATE_n ; _rtB -> B_120_543_0 += _rtP -> P_367 * _rtB ->
B_120_542_0 ; if ( _rtB -> B_120_154_0_d ) { if ( _rtB -> B_120_543_0 > _rtP
-> P_37 ) { _rtB -> B_54_0_0 = _rtB -> B_120_543_0 - _rtP -> P_37 ; } else if
( _rtB -> B_120_543_0 >= _rtP -> P_36 ) { _rtB -> B_54_0_0 = 0.0 ; } else {
_rtB -> B_54_0_0 = _rtB -> B_120_543_0 - _rtP -> P_36 ; } _rtB -> B_120_545_0
= _rtB -> B_54_0_0 ; } else { _rtB -> B_120_545_0 = _rtB -> B_120_153_0_k ; }
_rtB -> B_120_546_0 = _rtP -> P_368 * _rtB -> B_120_545_0 ; if ( _rtB ->
B_120_546_0 > _rtP -> P_369 ) { _rtB -> B_120_547_0 = _rtP -> P_369 ; } else
if ( _rtB -> B_120_546_0 < _rtP -> P_370 ) { _rtB -> B_120_547_0 = _rtP ->
P_370 ; } else { _rtB -> B_120_547_0 = _rtB -> B_120_546_0 ; } if ( ( _rtDW
-> LastMajorTimeA_fl >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW ->
LastMajorTimeB_g3 >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_120_548_0 = _rtB
-> B_120_547_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_fl < _rtDW ->
LastMajorTimeB_g3 ) && ( _rtDW -> LastMajorTimeB_g3 < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_fl >= _rtDW -> LastMajorTimeB_g3 ) && (
_rtDW -> LastMajorTimeA_fl >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_g3 ; rtb_B_120_5_0 = _rtDW
-> PrevYB_ku ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_fl ; rtb_B_120_5_0 = _rtDW -> PrevYA_o ; } riseValLimit =
deltaT * _rtP -> P_371 ; rateLimiterRate = _rtB -> B_120_547_0 -
rtb_B_120_5_0 ; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_120_548_0 =
rtb_B_120_5_0 + riseValLimit ; } else { deltaT *= _rtP -> P_372 ; if (
rateLimiterRate < deltaT ) { _rtB -> B_120_548_0 = rtb_B_120_5_0 + deltaT ; }
else { _rtB -> B_120_548_0 = _rtB -> B_120_547_0 ; } } } isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB -> B_120_549_0 = _rtP
-> P_373 * _rtB -> B_120_539_0 ; } _rtB -> B_120_550_0 = _rtB -> B_120_548_0
* _rtB -> B_120_549_0 ; _rtB -> B_120_551_0 = _rtB -> B_120_541_0 * _rtB ->
B_120_550_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model5/Data Store Read" ,
0 ) ; _rtB -> B_120_552_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_d != 0 )
{ _rtX -> vv0at_CSTATE_ji = _rtB -> B_120_552_0 ; } _rtB -> B_120_553_0 =
_rtX -> vv0at_CSTATE_ji ; _rtB -> B_120_554_0 = muDoubleScalarAbs ( _rtB ->
B_120_553_0 ) ; _rtB -> B_120_555_0 = _rtP -> P_374 [ 0 ] ; _rtB ->
B_120_555_0 = _rtB -> B_120_555_0 * _rtB -> B_120_554_0 + _rtP -> P_374 [ 1 ]
; _rtB -> B_120_555_0 = _rtB -> B_120_555_0 * _rtB -> B_120_554_0 + _rtP ->
P_374 [ 2 ] ; _rtB -> B_120_556_0 = _rtP -> P_375 * _rtB -> B_120_555_0 ;
_rtB -> B_120_557_0 = _rtB -> B_120_539_0 * _rtB -> B_120_556_0 ; _rtB ->
B_120_558_0 = _rtB -> B_120_551_0 - _rtB -> B_120_557_0 ; isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB -> B_120_560_0 = _rtP
-> P_376 * _rtB -> B_120_540_0 ; } if ( _rtB -> B_120_558_0 > _rtB ->
B_120_540_0 ) { _rtB -> B_120_562_0 = _rtB -> B_120_540_0 ; } else { if (
_rtB -> B_120_558_0 < _rtB -> B_120_560_0 ) { _rtB -> B_74_1_0 = _rtB ->
B_120_560_0 ; } else { _rtB -> B_74_1_0 = _rtB -> B_120_558_0 ; } _rtB ->
B_120_562_0 = _rtB -> B_74_1_0 ; } if ( _rtB -> B_120_202_0_o ) { _rtB ->
B_120_563_0 = _rtB -> B_120_562_0 ; } else { _rtB -> B_120_563_0 = _rtB ->
B_120_201_0_c ; } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) {
ssCallAccelRunBlock ( S , 67 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP ->
P_1018 != 0 ) && ( ! ( _rtB -> B_67_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_1018
== 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_l == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model3/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_l < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_l ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model3/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_181_0_l ) ; if ( ( _rtP -> P_1018 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_1018 == 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_l == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model3/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_l < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_l ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model3/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_181_0_l ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_181_0_l ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_565_0 =
_rtB -> B_120_180_0 [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_67_0_0 , 499U ) ] ; _rtB -> B_120_566_0 =
_rtB -> B_120_187_0_m * _rtB -> B_120_565_0 ; if ( ( _rtP -> P_1019 != 0 ) &&
( ! ( _rtB -> B_67_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_1019 == 1 ) { if (
_rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_oe == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_oe < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_oe ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_182_0_h ) ; if ( ( _rtP -> P_1019 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_1019 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_oe == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_oe < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_oe ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_182_0_h ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_182_0_h ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_567_0 =
_rtB -> B_120_180_0 [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_67_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_pt . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_pt .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_377 ; _rtB -> B_120_568_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_bw . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_bw . Last , _rtDW -> TransportDelay_IWORK_bw . Tail ,
_rtDW -> TransportDelay_IWORK_bw . Head , _rtP -> P_378 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_120_569_0 = 0.0 ; _rtB -> B_120_569_0 += _rtP -> P_380 * _rtX
-> TransferFcn_CSTATE_lb ; _rtB -> B_120_569_0 += _rtP -> P_381 * _rtB ->
B_120_568_0 ; if ( _rtB -> B_120_202_0_o ) { if ( _rtB -> B_120_569_0 > _rtP
-> P_49 ) { _rtB -> B_72_0_0 = _rtB -> B_120_569_0 - _rtP -> P_49 ; } else if
( _rtB -> B_120_569_0 >= _rtP -> P_48 ) { _rtB -> B_72_0_0 = 0.0 ; } else {
_rtB -> B_72_0_0 = _rtB -> B_120_569_0 - _rtP -> P_48 ; } _rtB -> B_120_571_0
= _rtB -> B_72_0_0 ; } else { _rtB -> B_120_571_0 = _rtB -> B_120_201_0_c ; }
_rtB -> B_120_572_0 = _rtP -> P_382 * _rtB -> B_120_571_0 ; if ( _rtB ->
B_120_572_0 > _rtP -> P_383 ) { _rtB -> B_120_573_0 = _rtP -> P_383 ; } else
if ( _rtB -> B_120_572_0 < _rtP -> P_384 ) { _rtB -> B_120_573_0 = _rtP ->
P_384 ; } else { _rtB -> B_120_573_0 = _rtB -> B_120_572_0 ; } if ( ( _rtDW
-> LastMajorTimeA_l4 >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW ->
LastMajorTimeB_gh >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_120_574_0 = _rtB
-> B_120_573_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_l4 < _rtDW ->
LastMajorTimeB_gh ) && ( _rtDW -> LastMajorTimeB_gh < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_l4 >= _rtDW -> LastMajorTimeB_gh ) && (
_rtDW -> LastMajorTimeA_l4 >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_gh ; rtb_B_120_5_0 = _rtDW
-> PrevYB_p3 ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_l4 ; rtb_B_120_5_0 = _rtDW -> PrevYA_oa ; } riseValLimit =
deltaT * _rtP -> P_385 ; rateLimiterRate = _rtB -> B_120_573_0 -
rtb_B_120_5_0 ; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_120_574_0 =
rtb_B_120_5_0 + riseValLimit ; } else { deltaT *= _rtP -> P_386 ; if (
rateLimiterRate < deltaT ) { _rtB -> B_120_574_0 = rtb_B_120_5_0 + deltaT ; }
else { _rtB -> B_120_574_0 = _rtB -> B_120_573_0 ; } } } isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB -> B_120_575_0 = _rtP
-> P_387 * _rtB -> B_120_565_0 ; } _rtB -> B_120_576_0 = _rtB -> B_120_574_0
* _rtB -> B_120_575_0 ; _rtB -> B_120_577_0 = _rtB -> B_120_567_0 * _rtB ->
B_120_576_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model3/Data Store Read" ,
0 ) ; _rtB -> B_120_578_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_k != 0 )
{ _rtX -> vv0at_CSTATE_ad = _rtB -> B_120_578_0 ; } _rtB -> B_120_579_0 =
_rtX -> vv0at_CSTATE_ad ; _rtB -> B_120_580_0 = muDoubleScalarAbs ( _rtB ->
B_120_579_0 ) ; _rtB -> B_120_581_0 = _rtP -> P_388 [ 0 ] ; _rtB ->
B_120_581_0 = _rtB -> B_120_581_0 * _rtB -> B_120_580_0 + _rtP -> P_388 [ 1 ]
; _rtB -> B_120_581_0 = _rtB -> B_120_581_0 * _rtB -> B_120_580_0 + _rtP ->
P_388 [ 2 ] ; _rtB -> B_120_582_0 = _rtP -> P_389 * _rtB -> B_120_581_0 ;
_rtB -> B_120_583_0 = _rtB -> B_120_565_0 * _rtB -> B_120_582_0 ; _rtB ->
B_120_584_0 = _rtB -> B_120_577_0 - _rtB -> B_120_583_0 ; isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB -> B_120_586_0 = _rtP
-> P_390 * _rtB -> B_120_566_0 ; } if ( _rtB -> B_120_584_0 > _rtB ->
B_120_566_0 ) { _rtB -> B_120_588_0 = _rtB -> B_120_566_0 ; } else { if (
_rtB -> B_120_584_0 < _rtB -> B_120_586_0 ) { _rtB -> B_68_1_0 = _rtB ->
B_120_586_0 ; } else { _rtB -> B_68_1_0 = _rtB -> B_120_584_0 ; } _rtB ->
B_120_588_0 = _rtB -> B_68_1_0 ; } if ( _rtB -> B_120_186_0_p ) { _rtB ->
B_120_589_0 = _rtB -> B_120_588_0 ; } else { _rtB -> B_120_589_0 = _rtB ->
B_120_185_0_g ; } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) {
ssCallAccelRunBlock ( S , 64 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP ->
P_1020 != 0 ) && ( ! ( _rtB -> B_64_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_1020
== 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_k == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model2/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_k < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_k ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model2/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_173_0_o ) ; if ( ( _rtP -> P_1020 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_1020 == 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_k == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model2/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_k < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_k ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model2/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_173_0_o ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_173_0_o ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_591_0 =
_rtB -> B_120_172_0_e [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_64_0_0 , 499U ) ] ; _rtB -> B_120_592_0 =
_rtB -> B_120_179_0_h * _rtB -> B_120_591_0 ; if ( ( _rtP -> P_1021 != 0 ) &&
( ! ( _rtB -> B_64_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_1021 == 1 ) { if (
_rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_a == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_a < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_a ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_174_0 ) ; if ( ( _rtP -> P_1021 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_1021 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_a == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_a < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_a ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_174_0 ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_174_0 ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_593_0 =
_rtB -> B_120_172_0_e [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_64_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_jo . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_jo .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_391 ; _rtB -> B_120_594_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_lz . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_lz . Last , _rtDW -> TransportDelay_IWORK_lz . Tail ,
_rtDW -> TransportDelay_IWORK_lz . Head , _rtP -> P_392 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_120_595_0 = 0.0 ; _rtB -> B_120_595_0 += _rtP -> P_394 * _rtX
-> TransferFcn_CSTATE_m3 ; _rtB -> B_120_595_0 += _rtP -> P_395 * _rtB ->
B_120_594_0 ; if ( _rtB -> B_120_186_0_p ) { if ( _rtB -> B_120_595_0 > _rtP
-> P_45 ) { _rtB -> B_66_0_0 = _rtB -> B_120_595_0 - _rtP -> P_45 ; } else if
( _rtB -> B_120_595_0 >= _rtP -> P_44 ) { _rtB -> B_66_0_0 = 0.0 ; } else {
_rtB -> B_66_0_0 = _rtB -> B_120_595_0 - _rtP -> P_44 ; } _rtB -> B_120_597_0
= _rtB -> B_66_0_0 ; } else { _rtB -> B_120_597_0 = _rtB -> B_120_185_0_g ; }
_rtB -> B_120_598_0 = _rtP -> P_396 * _rtB -> B_120_597_0 ; if ( _rtB ->
B_120_598_0 > _rtP -> P_397 ) { _rtB -> B_120_599_0 = _rtP -> P_397 ; } else
if ( _rtB -> B_120_598_0 < _rtP -> P_398 ) { _rtB -> B_120_599_0 = _rtP ->
P_398 ; } else { _rtB -> B_120_599_0 = _rtB -> B_120_598_0 ; } if ( ( _rtDW
-> LastMajorTimeA_e4 >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW ->
LastMajorTimeB_lv >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_120_600_0 = _rtB
-> B_120_599_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_e4 < _rtDW ->
LastMajorTimeB_lv ) && ( _rtDW -> LastMajorTimeB_lv < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_e4 >= _rtDW -> LastMajorTimeB_lv ) && (
_rtDW -> LastMajorTimeA_e4 >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_lv ; rtb_B_120_5_0 = _rtDW
-> PrevYB_i ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_e4 ; rtb_B_120_5_0 = _rtDW -> PrevYA_oi ; } riseValLimit =
deltaT * _rtP -> P_399 ; rateLimiterRate = _rtB -> B_120_599_0 -
rtb_B_120_5_0 ; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_120_600_0 =
rtb_B_120_5_0 + riseValLimit ; } else { deltaT *= _rtP -> P_400 ; if (
rateLimiterRate < deltaT ) { _rtB -> B_120_600_0 = rtb_B_120_5_0 + deltaT ; }
else { _rtB -> B_120_600_0 = _rtB -> B_120_599_0 ; } } } isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB -> B_120_601_0 = _rtP
-> P_401 * _rtB -> B_120_591_0 ; } _rtB -> B_120_602_0 = _rtB -> B_120_600_0
* _rtB -> B_120_601_0 ; _rtB -> B_120_603_0 = _rtB -> B_120_593_0 * _rtB ->
B_120_602_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model2/Data Store Read" ,
0 ) ; _rtB -> B_120_604_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_a != 0 )
{ _rtX -> vv0at_CSTATE_l2 = _rtB -> B_120_604_0 ; } _rtB -> B_120_605_0 =
_rtX -> vv0at_CSTATE_l2 ; _rtB -> B_120_606_0 = muDoubleScalarAbs ( _rtB ->
B_120_605_0 ) ; _rtB -> B_120_607_0 = _rtP -> P_402 [ 0 ] ; _rtB ->
B_120_607_0 = _rtB -> B_120_607_0 * _rtB -> B_120_606_0 + _rtP -> P_402 [ 1 ]
; _rtB -> B_120_607_0 = _rtB -> B_120_607_0 * _rtB -> B_120_606_0 + _rtP ->
P_402 [ 2 ] ; _rtB -> B_120_608_0 = _rtP -> P_403 * _rtB -> B_120_607_0 ;
_rtB -> B_120_609_0 = _rtB -> B_120_591_0 * _rtB -> B_120_608_0 ; _rtB ->
B_120_610_0 = _rtB -> B_120_603_0 - _rtB -> B_120_609_0 ; isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB -> B_120_612_0 = _rtP
-> P_404 * _rtB -> B_120_592_0 ; } if ( _rtB -> B_120_610_0 > _rtB ->
B_120_592_0 ) { _rtB -> B_120_614_0 = _rtB -> B_120_592_0 ; } else { if (
_rtB -> B_120_610_0 < _rtB -> B_120_612_0 ) { _rtB -> B_65_1_0 = _rtB ->
B_120_612_0 ; } else { _rtB -> B_65_1_0 = _rtB -> B_120_610_0 ; } _rtB ->
B_120_614_0 = _rtB -> B_65_1_0 ; } if ( _rtB -> B_120_178_0_p ) { _rtB ->
B_120_615_0 = _rtB -> B_120_614_0 ; } else { _rtB -> B_120_615_0 = _rtB ->
B_120_177_0_d ; } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) {
ssCallAccelRunBlock ( S , 61 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP ->
P_1022 != 0 ) && ( ! ( _rtB -> B_61_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_1022
== 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_fr == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model1/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_fr < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_fr ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model1/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_165_0_f ) ; if ( ( _rtP -> P_1022 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_1022 == 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_fr == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model1/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_fr < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_fr ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model1/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_165_0_f ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_165_0_f ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_617_0 =
_rtB -> B_120_164_0_c [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_61_0_0 , 499U ) ] ; _rtB -> B_120_618_0 =
_rtB -> B_120_171_0_p * _rtB -> B_120_617_0 ; if ( ( _rtP -> P_1023 != 0 ) &&
( ! ( _rtB -> B_61_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_1023 == 1 ) { if (
_rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_kx == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_kx < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_kx ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_166_0 ) ; if ( ( _rtP -> P_1023 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_1023 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_kx == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_kx < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_kx ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_166_0 ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_166_0 ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_619_0 =
_rtB -> B_120_164_0_c [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_61_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_no . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_no .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_405 ; _rtB -> B_120_620_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_jf . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_jf . Last , _rtDW -> TransportDelay_IWORK_jf . Tail ,
_rtDW -> TransportDelay_IWORK_jf . Head , _rtP -> P_406 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_120_621_0 = 0.0 ; _rtB -> B_120_621_0 += _rtP -> P_408 * _rtX
-> TransferFcn_CSTATE_o ; _rtB -> B_120_621_0 += _rtP -> P_409 * _rtB ->
B_120_620_0 ; if ( _rtB -> B_120_178_0_p ) { if ( _rtB -> B_120_621_0 > _rtP
-> P_43 ) { _rtB -> B_63_0_0 = _rtB -> B_120_621_0 - _rtP -> P_43 ; } else if
( _rtB -> B_120_621_0 >= _rtP -> P_42 ) { _rtB -> B_63_0_0 = 0.0 ; } else {
_rtB -> B_63_0_0 = _rtB -> B_120_621_0 - _rtP -> P_42 ; } _rtB -> B_120_623_0
= _rtB -> B_63_0_0 ; } else { _rtB -> B_120_623_0 = _rtB -> B_120_177_0_d ; }
_rtB -> B_120_624_0 = _rtP -> P_410 * _rtB -> B_120_623_0 ; if ( _rtB ->
B_120_624_0 > _rtP -> P_411 ) { _rtB -> B_120_625_0 = _rtP -> P_411 ; } else
if ( _rtB -> B_120_624_0 < _rtP -> P_412 ) { _rtB -> B_120_625_0 = _rtP ->
P_412 ; } else { _rtB -> B_120_625_0 = _rtB -> B_120_624_0 ; } if ( ( _rtDW
-> LastMajorTimeA_jf >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW ->
LastMajorTimeB_fm >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_120_626_0 = _rtB
-> B_120_625_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_jf < _rtDW ->
LastMajorTimeB_fm ) && ( _rtDW -> LastMajorTimeB_fm < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_jf >= _rtDW -> LastMajorTimeB_fm ) && (
_rtDW -> LastMajorTimeA_jf >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_fm ; rtb_B_120_5_0 = _rtDW
-> PrevYB_nf ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_jf ; rtb_B_120_5_0 = _rtDW -> PrevYA_el ; } riseValLimit =
deltaT * _rtP -> P_413 ; rateLimiterRate = _rtB -> B_120_625_0 -
rtb_B_120_5_0 ; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_120_626_0 =
rtb_B_120_5_0 + riseValLimit ; } else { deltaT *= _rtP -> P_414 ; if (
rateLimiterRate < deltaT ) { _rtB -> B_120_626_0 = rtb_B_120_5_0 + deltaT ; }
else { _rtB -> B_120_626_0 = _rtB -> B_120_625_0 ; } } } isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB -> B_120_627_0 = _rtP
-> P_415 * _rtB -> B_120_617_0 ; } _rtB -> B_120_628_0 = _rtB -> B_120_626_0
* _rtB -> B_120_627_0 ; _rtB -> B_120_629_0 = _rtB -> B_120_619_0 * _rtB ->
B_120_628_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model1/Data Store Read" ,
0 ) ; _rtB -> B_120_630_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_cz != 0 )
{ _rtX -> vv0at_CSTATE_dj = _rtB -> B_120_630_0 ; } _rtB -> B_120_631_0 =
_rtX -> vv0at_CSTATE_dj ; _rtB -> B_120_632_0 = muDoubleScalarAbs ( _rtB ->
B_120_631_0 ) ; _rtB -> B_120_633_0 = _rtP -> P_416 [ 0 ] ; _rtB ->
B_120_633_0 = _rtB -> B_120_633_0 * _rtB -> B_120_632_0 + _rtP -> P_416 [ 1 ]
; _rtB -> B_120_633_0 = _rtB -> B_120_633_0 * _rtB -> B_120_632_0 + _rtP ->
P_416 [ 2 ] ; _rtB -> B_120_634_0 = _rtP -> P_417 * _rtB -> B_120_633_0 ;
_rtB -> B_120_635_0 = _rtB -> B_120_617_0 * _rtB -> B_120_634_0 ; _rtB ->
B_120_636_0 = _rtB -> B_120_629_0 - _rtB -> B_120_635_0 ; isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB -> B_120_638_0 = _rtP
-> P_418 * _rtB -> B_120_618_0 ; } if ( _rtB -> B_120_636_0 > _rtB ->
B_120_618_0 ) { _rtB -> B_120_640_0 = _rtB -> B_120_618_0 ; } else { if (
_rtB -> B_120_636_0 < _rtB -> B_120_638_0 ) { _rtB -> B_62_1_0 = _rtB ->
B_120_638_0 ; } else { _rtB -> B_62_1_0 = _rtB -> B_120_636_0 ; } _rtB ->
B_120_640_0 = _rtB -> B_62_1_0 ; } if ( _rtB -> B_120_170_0 ) { _rtB ->
B_120_641_0 = _rtB -> B_120_640_0 ; } else { _rtB -> B_120_641_0 = _rtB ->
B_120_169_0_p ; } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) {
ssCallAccelRunBlock ( S , 70 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP ->
P_1024 != 0 ) && ( ! ( _rtB -> B_70_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_1024
== 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_m == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model4/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_m < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_m ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model4/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_189_0_h ) ; if ( ( _rtP -> P_1024 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_1024 == 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_m == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model4/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_m < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_m ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model4/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_189_0_h ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_189_0_h ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_643_0 =
_rtB -> B_120_188_0_m [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_70_0_0 , 499U ) ] ; _rtB -> B_120_644_0 =
_rtB -> B_120_195_0 * _rtB -> B_120_643_0 ; if ( ( _rtP -> P_1025 != 0 ) && (
! ( _rtB -> B_70_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_1025 == 1 ) { if ( _rtDW
-> lookupwagonbrakingefficiency_NumIssuedWarnings_c == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_c < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_c ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_190_0_c ) ; if ( ( _rtP -> P_1025 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_1025 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_c == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_c < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_c ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_190_0_c ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_190_0_c ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_645_0 =
_rtB -> B_120_188_0_m [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_70_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_mn . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_mn .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_419 ; _rtB -> B_120_646_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_m3 . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_m3 . Last , _rtDW -> TransportDelay_IWORK_m3 . Tail ,
_rtDW -> TransportDelay_IWORK_m3 . Head , _rtP -> P_420 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_120_647_0 = 0.0 ; _rtB -> B_120_647_0 += _rtP -> P_422 * _rtX
-> TransferFcn_CSTATE_mv ; _rtB -> B_120_647_0 += _rtP -> P_423 * _rtB ->
B_120_646_0 ; if ( _rtB -> B_120_170_0 ) { if ( _rtB -> B_120_647_0 > _rtP ->
P_41 ) { _rtB -> B_60_0_0 = _rtB -> B_120_647_0 - _rtP -> P_41 ; } else if (
_rtB -> B_120_647_0 >= _rtP -> P_40 ) { _rtB -> B_60_0_0 = 0.0 ; } else {
_rtB -> B_60_0_0 = _rtB -> B_120_647_0 - _rtP -> P_40 ; } _rtB -> B_120_649_0
= _rtB -> B_60_0_0 ; } else { _rtB -> B_120_649_0 = _rtB -> B_120_169_0_p ; }
_rtB -> B_120_650_0 = _rtP -> P_424 * _rtB -> B_120_649_0 ; if ( _rtB ->
B_120_650_0 > _rtP -> P_425 ) { _rtB -> B_120_651_0 = _rtP -> P_425 ; } else
if ( _rtB -> B_120_650_0 < _rtP -> P_426 ) { _rtB -> B_120_651_0 = _rtP ->
P_426 ; } else { _rtB -> B_120_651_0 = _rtB -> B_120_650_0 ; } if ( ( _rtDW
-> LastMajorTimeA_k >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW ->
LastMajorTimeB_b >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_120_652_0 = _rtB
-> B_120_651_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_k < _rtDW ->
LastMajorTimeB_b ) && ( _rtDW -> LastMajorTimeB_b < ssGetTaskTime ( S , 0 ) )
) || ( ( _rtDW -> LastMajorTimeA_k >= _rtDW -> LastMajorTimeB_b ) && ( _rtDW
-> LastMajorTimeA_k >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime
( S , 0 ) - _rtDW -> LastMajorTimeB_b ; rtb_B_120_5_0 = _rtDW -> PrevYB_h ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_k ;
rtb_B_120_5_0 = _rtDW -> PrevYA_kr ; } riseValLimit = deltaT * _rtP -> P_427
; rateLimiterRate = _rtB -> B_120_651_0 - rtb_B_120_5_0 ; if (
rateLimiterRate > riseValLimit ) { _rtB -> B_120_652_0 = rtb_B_120_5_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_428 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_120_652_0 = rtb_B_120_5_0 + deltaT ; } else { _rtB ->
B_120_652_0 = _rtB -> B_120_651_0 ; } } } isHit = ssIsSampleHit ( S , 2 , 0 )
; if ( isHit != 0 ) { _rtB -> B_120_653_0 = _rtP -> P_429 * _rtB ->
B_120_643_0 ; } _rtB -> B_120_654_0 = _rtB -> B_120_652_0 * _rtB ->
B_120_653_0 ; _rtB -> B_120_655_0 = _rtB -> B_120_645_0 * _rtB -> B_120_654_0
; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model4/Data Store Read" ,
0 ) ; _rtB -> B_120_656_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_la != 0 )
{ _rtX -> vv0at_CSTATE_dv = _rtB -> B_120_656_0 ; } _rtB -> B_120_657_0 =
_rtX -> vv0at_CSTATE_dv ; _rtB -> B_120_658_0 = muDoubleScalarAbs ( _rtB ->
B_120_657_0 ) ; _rtB -> B_120_659_0 = _rtP -> P_430 [ 0 ] ; _rtB ->
B_120_659_0 = _rtB -> B_120_659_0 * _rtB -> B_120_658_0 + _rtP -> P_430 [ 1 ]
; _rtB -> B_120_659_0 = _rtB -> B_120_659_0 * _rtB -> B_120_658_0 + _rtP ->
P_430 [ 2 ] ; _rtB -> B_120_660_0 = _rtP -> P_431 * _rtB -> B_120_659_0 ;
_rtB -> B_120_661_0 = _rtB -> B_120_643_0 * _rtB -> B_120_660_0 ; _rtB ->
B_120_662_0 = _rtB -> B_120_655_0 - _rtB -> B_120_661_0 ; isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB -> B_120_664_0 = _rtP
-> P_432 * _rtB -> B_120_644_0 ; } if ( _rtB -> B_120_662_0 > _rtB ->
B_120_644_0 ) { _rtB -> B_120_666_0 = _rtB -> B_120_644_0 ; } else { if (
_rtB -> B_120_662_0 < _rtB -> B_120_664_0 ) { _rtB -> B_71_1_0 = _rtB ->
B_120_664_0 ; } else { _rtB -> B_71_1_0 = _rtB -> B_120_662_0 ; } _rtB ->
B_120_666_0 = _rtB -> B_71_1_0 ; } if ( _rtB -> B_120_194_0_a ) { _rtB ->
B_120_667_0 = _rtB -> B_120_666_0 ; } else { _rtB -> B_120_667_0 = _rtB ->
B_120_193_0_c ; } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) {
ssCallAccelRunBlock ( S , 88 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP ->
P_1026 != 0 ) && ( ! ( _rtB -> B_88_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_1026
== 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_kw == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model5/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_kw < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_kw ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model5/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_237_0_a ) ; if ( ( _rtP -> P_1026 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_1026 == 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_kw == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model5/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_kw < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_kw ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model5/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_237_0_a ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_237_0_a ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_669_0 =
_rtB -> B_120_236_0_d [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_88_0_0 , 499U ) ] ; _rtB -> B_120_670_0 =
_rtB -> B_120_243_0_m * _rtB -> B_120_669_0 ; if ( ( _rtP -> P_1027 != 0 ) &&
( ! ( _rtB -> B_88_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_1027 == 1 ) { if (
_rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_jf == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model5/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_jf < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_jf ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model5/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_238_0_p ) ; if ( ( _rtP -> P_1027 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_1027 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_jf == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model5/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_jf < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_jf ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model5/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_238_0_p ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_238_0_p ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_671_0 =
_rtB -> B_120_236_0_d [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_88_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_dz . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_dz .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_433 ; _rtB -> B_120_672_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_ga . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_ga . Last , _rtDW -> TransportDelay_IWORK_ga . Tail ,
_rtDW -> TransportDelay_IWORK_ga . Head , _rtP -> P_434 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_120_673_0 = 0.0 ; _rtB -> B_120_673_0 += _rtP -> P_436 * _rtX
-> TransferFcn_CSTATE_gj ; _rtB -> B_120_673_0 += _rtP -> P_437 * _rtB ->
B_120_672_0 ; if ( _rtB -> B_120_194_0_a ) { if ( _rtB -> B_120_673_0 > _rtP
-> P_47 ) { _rtB -> B_69_0_0 = _rtB -> B_120_673_0 - _rtP -> P_47 ; } else if
( _rtB -> B_120_673_0 >= _rtP -> P_46 ) { _rtB -> B_69_0_0 = 0.0 ; } else {
_rtB -> B_69_0_0 = _rtB -> B_120_673_0 - _rtP -> P_46 ; } _rtB -> B_120_675_0
= _rtB -> B_69_0_0 ; } else { _rtB -> B_120_675_0 = _rtB -> B_120_193_0_c ; }
_rtB -> B_120_676_0 = _rtP -> P_438 * _rtB -> B_120_675_0 ; if ( _rtB ->
B_120_676_0 > _rtP -> P_439 ) { _rtB -> B_120_677_0 = _rtP -> P_439 ; } else
if ( _rtB -> B_120_676_0 < _rtP -> P_440 ) { _rtB -> B_120_677_0 = _rtP ->
P_440 ; } else { _rtB -> B_120_677_0 = _rtB -> B_120_676_0 ; } if ( ( _rtDW
-> LastMajorTimeA_io >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW ->
LastMajorTimeB_mi >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_120_678_0 = _rtB
-> B_120_677_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_io < _rtDW ->
LastMajorTimeB_mi ) && ( _rtDW -> LastMajorTimeB_mi < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_io >= _rtDW -> LastMajorTimeB_mi ) && (
_rtDW -> LastMajorTimeA_io >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_mi ; rtb_B_120_5_0 = _rtDW
-> PrevYB_cl ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_io ; rtb_B_120_5_0 = _rtDW -> PrevYA_d ; } riseValLimit =
deltaT * _rtP -> P_441 ; rateLimiterRate = _rtB -> B_120_677_0 -
rtb_B_120_5_0 ; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_120_678_0 =
rtb_B_120_5_0 + riseValLimit ; } else { deltaT *= _rtP -> P_442 ; if (
rateLimiterRate < deltaT ) { _rtB -> B_120_678_0 = rtb_B_120_5_0 + deltaT ; }
else { _rtB -> B_120_678_0 = _rtB -> B_120_677_0 ; } } } isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB -> B_120_679_0 = _rtP
-> P_443 * _rtB -> B_120_669_0 ; } _rtB -> B_120_680_0 = _rtB -> B_120_678_0
* _rtB -> B_120_679_0 ; _rtB -> B_120_681_0 = _rtB -> B_120_671_0 * _rtB ->
B_120_680_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model5/Data Store Read" ,
0 ) ; _rtB -> B_120_682_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_b2 != 0 )
{ _rtX -> vv0at_CSTATE_b = _rtB -> B_120_682_0 ; } _rtB -> B_120_683_0 = _rtX
-> vv0at_CSTATE_b ; _rtB -> B_120_684_0 = muDoubleScalarAbs ( _rtB ->
B_120_683_0 ) ; _rtB -> B_120_685_0 = _rtP -> P_444 [ 0 ] ; _rtB ->
B_120_685_0 = _rtB -> B_120_685_0 * _rtB -> B_120_684_0 + _rtP -> P_444 [ 1 ]
; _rtB -> B_120_685_0 = _rtB -> B_120_685_0 * _rtB -> B_120_684_0 + _rtP ->
P_444 [ 2 ] ; _rtB -> B_120_686_0 = _rtP -> P_445 * _rtB -> B_120_685_0 ;
_rtB -> B_120_687_0 = _rtB -> B_120_669_0 * _rtB -> B_120_686_0 ; _rtB ->
B_120_688_0 = _rtB -> B_120_681_0 - _rtB -> B_120_687_0 ; isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB -> B_120_690_0 = _rtP
-> P_446 * _rtB -> B_120_670_0 ; } if ( _rtB -> B_120_688_0 > _rtB ->
B_120_670_0 ) { _rtB -> B_120_692_0 = _rtB -> B_120_670_0 ; } else { if (
_rtB -> B_120_688_0 < _rtB -> B_120_690_0 ) { _rtB -> B_89_1_0 = _rtB ->
B_120_690_0 ; } else { _rtB -> B_89_1_0 = _rtB -> B_120_688_0 ; } _rtB ->
B_120_692_0 = _rtB -> B_89_1_0 ; } if ( _rtB -> B_120_242_0_l ) { _rtB ->
B_120_693_0 = _rtB -> B_120_692_0 ; } else { _rtB -> B_120_693_0 = _rtB ->
B_120_241_0_f ; } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) {
ssCallAccelRunBlock ( S , 82 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP ->
P_1028 != 0 ) && ( ! ( _rtB -> B_82_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_1028
== 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_j == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model3/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_j < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_j ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model3/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_221_0 ) ; if ( ( _rtP -> P_1028 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_1028 == 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_j == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model3/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_j < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_j ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model3/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_221_0 ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_221_0 ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_695_0 =
_rtB -> B_120_220_0_g [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_82_0_0 , 499U ) ] ; _rtB -> B_120_696_0 =
_rtB -> B_120_227_0_f * _rtB -> B_120_695_0 ; if ( ( _rtP -> P_1029 != 0 ) &&
( ! ( _rtB -> B_82_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_1029 == 1 ) { if (
_rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_lh == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_lh < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_lh ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_222_0_e ) ; if ( ( _rtP -> P_1029 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_1029 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_lh == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_lh < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_lh ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_222_0_e ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_222_0_e ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_697_0 =
_rtB -> B_120_220_0_g [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_82_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_d3 . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_d3 .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_447 ; _rtB -> B_120_698_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_ii . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_ii . Last , _rtDW -> TransportDelay_IWORK_ii . Tail ,
_rtDW -> TransportDelay_IWORK_ii . Head , _rtP -> P_448 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_120_699_0 = 0.0 ; _rtB -> B_120_699_0 += _rtP -> P_450 * _rtX
-> TransferFcn_CSTATE_e ; _rtB -> B_120_699_0 += _rtP -> P_451 * _rtB ->
B_120_698_0 ; if ( _rtB -> B_120_242_0_l ) { if ( _rtB -> B_120_699_0 > _rtP
-> P_59 ) { _rtB -> B_87_0_0 = _rtB -> B_120_699_0 - _rtP -> P_59 ; } else if
( _rtB -> B_120_699_0 >= _rtP -> P_58 ) { _rtB -> B_87_0_0 = 0.0 ; } else {
_rtB -> B_87_0_0 = _rtB -> B_120_699_0 - _rtP -> P_58 ; } _rtB -> B_120_701_0
= _rtB -> B_87_0_0 ; } else { _rtB -> B_120_701_0 = _rtB -> B_120_241_0_f ; }
_rtB -> B_120_702_0 = _rtP -> P_452 * _rtB -> B_120_701_0 ; if ( _rtB ->
B_120_702_0 > _rtP -> P_453 ) { _rtB -> B_120_703_0 = _rtP -> P_453 ; } else
if ( _rtB -> B_120_702_0 < _rtP -> P_454 ) { _rtB -> B_120_703_0 = _rtP ->
P_454 ; } else { _rtB -> B_120_703_0 = _rtB -> B_120_702_0 ; } if ( ( _rtDW
-> LastMajorTimeA_c3 >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW ->
LastMajorTimeB_h >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_120_704_0 = _rtB
-> B_120_703_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_c3 < _rtDW ->
LastMajorTimeB_h ) && ( _rtDW -> LastMajorTimeB_h < ssGetTaskTime ( S , 0 ) )
) || ( ( _rtDW -> LastMajorTimeA_c3 >= _rtDW -> LastMajorTimeB_h ) && ( _rtDW
-> LastMajorTimeA_c3 >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_h ; rtb_B_120_5_0 = _rtDW
-> PrevYB_ib ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_c3 ; rtb_B_120_5_0 = _rtDW -> PrevYA_mi ; } riseValLimit =
deltaT * _rtP -> P_455 ; rateLimiterRate = _rtB -> B_120_703_0 -
rtb_B_120_5_0 ; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_120_704_0 =
rtb_B_120_5_0 + riseValLimit ; } else { deltaT *= _rtP -> P_456 ; if (
rateLimiterRate < deltaT ) { _rtB -> B_120_704_0 = rtb_B_120_5_0 + deltaT ; }
else { _rtB -> B_120_704_0 = _rtB -> B_120_703_0 ; } } } isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB -> B_120_705_0 = _rtP
-> P_457 * _rtB -> B_120_695_0 ; } _rtB -> B_120_706_0 = _rtB -> B_120_704_0
* _rtB -> B_120_705_0 ; _rtB -> B_120_707_0 = _rtB -> B_120_697_0 * _rtB ->
B_120_706_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model3/Data Store Read" ,
0 ) ; _rtB -> B_120_708_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_cx != 0 )
{ _rtX -> vv0at_CSTATE_ady = _rtB -> B_120_708_0 ; } _rtB -> B_120_709_0 =
_rtX -> vv0at_CSTATE_ady ; _rtB -> B_120_710_0 = muDoubleScalarAbs ( _rtB ->
B_120_709_0 ) ; _rtB -> B_120_711_0 = _rtP -> P_458 [ 0 ] ; _rtB ->
B_120_711_0 = _rtB -> B_120_711_0 * _rtB -> B_120_710_0 + _rtP -> P_458 [ 1 ]
; _rtB -> B_120_711_0 = _rtB -> B_120_711_0 * _rtB -> B_120_710_0 + _rtP ->
P_458 [ 2 ] ; _rtB -> B_120_712_0 = _rtP -> P_459 * _rtB -> B_120_711_0 ;
_rtB -> B_120_713_0 = _rtB -> B_120_695_0 * _rtB -> B_120_712_0 ; _rtB ->
B_120_714_0 = _rtB -> B_120_707_0 - _rtB -> B_120_713_0 ; isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB -> B_120_716_0 = _rtP
-> P_460 * _rtB -> B_120_696_0 ; } if ( _rtB -> B_120_714_0 > _rtB ->
B_120_696_0 ) { _rtB -> B_120_718_0 = _rtB -> B_120_696_0 ; } else { if (
_rtB -> B_120_714_0 < _rtB -> B_120_716_0 ) { _rtB -> B_83_1_0 = _rtB ->
B_120_716_0 ; } else { _rtB -> B_83_1_0 = _rtB -> B_120_714_0 ; } _rtB ->
B_120_718_0 = _rtB -> B_83_1_0 ; } if ( _rtB -> B_120_226_0 ) { _rtB ->
B_120_719_0 = _rtB -> B_120_718_0 ; } else { _rtB -> B_120_719_0 = _rtB ->
B_120_225_0_g ; } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) {
ssCallAccelRunBlock ( S , 79 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP ->
P_1030 != 0 ) && ( ! ( _rtB -> B_79_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_1030
== 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_dr == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model2/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_dr < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_dr ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model2/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_213_0_o ) ; if ( ( _rtP -> P_1030 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_1030 == 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_dr == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model2/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_dr < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_dr ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model2/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_213_0_o ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_213_0_o ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_721_0 =
_rtB -> B_120_212_0_e [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_79_0_0 , 499U ) ] ; _rtB -> B_120_722_0 =
_rtB -> B_120_219_0_a * _rtB -> B_120_721_0 ; if ( ( _rtP -> P_1031 != 0 ) &&
( ! ( _rtB -> B_79_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_1031 == 1 ) { if (
_rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_jg == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_jg < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_jg ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_214_0_b ) ; if ( ( _rtP -> P_1031 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_1031 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_jg == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_jg < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_jg ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_214_0_b ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_214_0_b ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_723_0 =
_rtB -> B_120_212_0_e [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_79_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_mz . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_mz .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_461 ; _rtB -> B_120_724_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_fs . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_fs . Last , _rtDW -> TransportDelay_IWORK_fs . Tail ,
_rtDW -> TransportDelay_IWORK_fs . Head , _rtP -> P_462 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_120_725_0 = 0.0 ; _rtB -> B_120_725_0 += _rtP -> P_464 * _rtX
-> TransferFcn_CSTATE_cm ; _rtB -> B_120_725_0 += _rtP -> P_465 * _rtB ->
B_120_724_0 ; if ( _rtB -> B_120_226_0 ) { if ( _rtB -> B_120_725_0 > _rtP ->
P_55 ) { _rtB -> B_81_0_0 = _rtB -> B_120_725_0 - _rtP -> P_55 ; } else if (
_rtB -> B_120_725_0 >= _rtP -> P_54 ) { _rtB -> B_81_0_0 = 0.0 ; } else {
_rtB -> B_81_0_0 = _rtB -> B_120_725_0 - _rtP -> P_54 ; } _rtB -> B_120_727_0
= _rtB -> B_81_0_0 ; } else { _rtB -> B_120_727_0 = _rtB -> B_120_225_0_g ; }
_rtB -> B_120_728_0 = _rtP -> P_466 * _rtB -> B_120_727_0 ; if ( _rtB ->
B_120_728_0 > _rtP -> P_467 ) { _rtB -> B_120_729_0 = _rtP -> P_467 ; } else
if ( _rtB -> B_120_728_0 < _rtP -> P_468 ) { _rtB -> B_120_729_0 = _rtP ->
P_468 ; } else { _rtB -> B_120_729_0 = _rtB -> B_120_728_0 ; } if ( ( _rtDW
-> LastMajorTimeA_px >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW ->
LastMajorTimeB_fi >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_120_730_0 = _rtB
-> B_120_729_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_px < _rtDW ->
LastMajorTimeB_fi ) && ( _rtDW -> LastMajorTimeB_fi < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_px >= _rtDW -> LastMajorTimeB_fi ) && (
_rtDW -> LastMajorTimeA_px >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_fi ; rtb_B_120_5_0 = _rtDW
-> PrevYB_j1 ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_px ; rtb_B_120_5_0 = _rtDW -> PrevYA_c ; } riseValLimit =
deltaT * _rtP -> P_469 ; rateLimiterRate = _rtB -> B_120_729_0 -
rtb_B_120_5_0 ; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_120_730_0 =
rtb_B_120_5_0 + riseValLimit ; } else { deltaT *= _rtP -> P_470 ; if (
rateLimiterRate < deltaT ) { _rtB -> B_120_730_0 = rtb_B_120_5_0 + deltaT ; }
else { _rtB -> B_120_730_0 = _rtB -> B_120_729_0 ; } } } isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB -> B_120_731_0 = _rtP
-> P_471 * _rtB -> B_120_721_0 ; } _rtB -> B_120_732_0 = _rtB -> B_120_730_0
* _rtB -> B_120_731_0 ; _rtB -> B_120_733_0 = _rtB -> B_120_723_0 * _rtB ->
B_120_732_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model2/Data Store Read" ,
0 ) ; _rtB -> B_120_734_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_lc != 0 )
{ _rtX -> vv0at_CSTATE_i = _rtB -> B_120_734_0 ; } _rtB -> B_120_735_0 = _rtX
-> vv0at_CSTATE_i ; _rtB -> B_120_736_0 = muDoubleScalarAbs ( _rtB ->
B_120_735_0 ) ; _rtB -> B_120_737_0 = _rtP -> P_472 [ 0 ] ; _rtB ->
B_120_737_0 = _rtB -> B_120_737_0 * _rtB -> B_120_736_0 + _rtP -> P_472 [ 1 ]
; _rtB -> B_120_737_0 = _rtB -> B_120_737_0 * _rtB -> B_120_736_0 + _rtP ->
P_472 [ 2 ] ; _rtB -> B_120_738_0 = _rtP -> P_473 * _rtB -> B_120_737_0 ;
_rtB -> B_120_739_0 = _rtB -> B_120_721_0 * _rtB -> B_120_738_0 ; _rtB ->
B_120_740_0 = _rtB -> B_120_733_0 - _rtB -> B_120_739_0 ; isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB -> B_120_742_0 = _rtP
-> P_474 * _rtB -> B_120_722_0 ; } if ( _rtB -> B_120_740_0 > _rtB ->
B_120_722_0 ) { _rtB -> B_120_744_0 = _rtB -> B_120_722_0 ; } else { if (
_rtB -> B_120_740_0 < _rtB -> B_120_742_0 ) { _rtB -> B_80_1_0 = _rtB ->
B_120_742_0 ; } else { _rtB -> B_80_1_0 = _rtB -> B_120_740_0 ; } _rtB ->
B_120_744_0 = _rtB -> B_80_1_0 ; } if ( _rtB -> B_120_218_0_p ) { _rtB ->
B_120_745_0 = _rtB -> B_120_744_0 ; } else { _rtB -> B_120_745_0 = _rtB ->
B_120_217_0_d ; } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) {
ssCallAccelRunBlock ( S , 76 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP ->
P_1032 != 0 ) && ( ! ( _rtB -> B_76_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_1032
== 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_pf == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model1/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_pf < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_pf ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model1/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_205_0_a ) ; if ( ( _rtP -> P_1032 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_1032 == 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_pf == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model1/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_pf < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_pf ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model1/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_205_0_a ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_205_0_a ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_747_0 =
_rtB -> B_120_204_0_p [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_76_0_0 , 499U ) ] ; _rtB -> B_120_748_0 =
_rtB -> B_120_211_0_j * _rtB -> B_120_747_0 ; if ( ( _rtP -> P_1033 != 0 ) &&
( ! ( _rtB -> B_76_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_1033 == 1 ) { if (
_rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_bg == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_bg < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_bg ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_206_0 ) ; if ( ( _rtP -> P_1033 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_1033 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_bg == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_bg < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_bg ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_206_0 ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_206_0 ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_749_0 =
_rtB -> B_120_204_0_p [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_76_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_ix . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_ix .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_475 ; _rtB -> B_120_750_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_kn . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_kn . Last , _rtDW -> TransportDelay_IWORK_kn . Tail ,
_rtDW -> TransportDelay_IWORK_kn . Head , _rtP -> P_476 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_120_751_0 = 0.0 ; _rtB -> B_120_751_0 += _rtP -> P_478 * _rtX
-> TransferFcn_CSTATE_a ; _rtB -> B_120_751_0 += _rtP -> P_479 * _rtB ->
B_120_750_0 ; if ( _rtB -> B_120_218_0_p ) { if ( _rtB -> B_120_751_0 > _rtP
-> P_53 ) { _rtB -> B_78_0_0 = _rtB -> B_120_751_0 - _rtP -> P_53 ; } else if
( _rtB -> B_120_751_0 >= _rtP -> P_52 ) { _rtB -> B_78_0_0 = 0.0 ; } else {
_rtB -> B_78_0_0 = _rtB -> B_120_751_0 - _rtP -> P_52 ; } _rtB -> B_120_753_0
= _rtB -> B_78_0_0 ; } else { _rtB -> B_120_753_0 = _rtB -> B_120_217_0_d ; }
_rtB -> B_120_754_0 = _rtP -> P_480 * _rtB -> B_120_753_0 ; if ( _rtB ->
B_120_754_0 > _rtP -> P_481 ) { _rtB -> B_120_755_0 = _rtP -> P_481 ; } else
if ( _rtB -> B_120_754_0 < _rtP -> P_482 ) { _rtB -> B_120_755_0 = _rtP ->
P_482 ; } else { _rtB -> B_120_755_0 = _rtB -> B_120_754_0 ; } if ( ( _rtDW
-> LastMajorTimeA_e4x >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW ->
LastMajorTimeB_ex >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_120_756_0 = _rtB
-> B_120_755_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_e4x < _rtDW ->
LastMajorTimeB_ex ) && ( _rtDW -> LastMajorTimeB_ex < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_e4x >= _rtDW -> LastMajorTimeB_ex ) && (
_rtDW -> LastMajorTimeA_e4x >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_ex ; rtb_B_120_5_0 = _rtDW
-> PrevYB_jy ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_e4x ; rtb_B_120_5_0 = _rtDW -> PrevYA_df ; } riseValLimit =
deltaT * _rtP -> P_483 ; rateLimiterRate = _rtB -> B_120_755_0 -
rtb_B_120_5_0 ; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_120_756_0 =
rtb_B_120_5_0 + riseValLimit ; } else { deltaT *= _rtP -> P_484 ; if (
rateLimiterRate < deltaT ) { _rtB -> B_120_756_0 = rtb_B_120_5_0 + deltaT ; }
else { _rtB -> B_120_756_0 = _rtB -> B_120_755_0 ; } } } isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB -> B_120_757_0 = _rtP
-> P_485 * _rtB -> B_120_747_0 ; } _rtB -> B_120_758_0 = _rtB -> B_120_756_0
* _rtB -> B_120_757_0 ; _rtB -> B_120_759_0 = _rtB -> B_120_749_0 * _rtB ->
B_120_758_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model1/Data Store Read" ,
0 ) ; _rtB -> B_120_760_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_oj != 0 )
{ _rtX -> vv0at_CSTATE_pm = _rtB -> B_120_760_0 ; } _rtB -> B_120_761_0 =
_rtX -> vv0at_CSTATE_pm ; _rtB -> B_120_762_0 = muDoubleScalarAbs ( _rtB ->
B_120_761_0 ) ; _rtB -> B_120_763_0 = _rtP -> P_486 [ 0 ] ; _rtB ->
B_120_763_0 = _rtB -> B_120_763_0 * _rtB -> B_120_762_0 + _rtP -> P_486 [ 1 ]
; _rtB -> B_120_763_0 = _rtB -> B_120_763_0 * _rtB -> B_120_762_0 + _rtP ->
P_486 [ 2 ] ; _rtB -> B_120_764_0 = _rtP -> P_487 * _rtB -> B_120_763_0 ;
_rtB -> B_120_765_0 = _rtB -> B_120_747_0 * _rtB -> B_120_764_0 ; _rtB ->
B_120_766_0 = _rtB -> B_120_759_0 - _rtB -> B_120_765_0 ; isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB -> B_120_768_0 = _rtP
-> P_488 * _rtB -> B_120_748_0 ; } if ( _rtB -> B_120_766_0 > _rtB ->
B_120_748_0 ) { _rtB -> B_120_770_0 = _rtB -> B_120_748_0 ; } else { if (
_rtB -> B_120_766_0 < _rtB -> B_120_768_0 ) { _rtB -> B_77_1_0 = _rtB ->
B_120_768_0 ; } else { _rtB -> B_77_1_0 = _rtB -> B_120_766_0 ; } _rtB ->
B_120_770_0 = _rtB -> B_77_1_0 ; } if ( _rtB -> B_120_210_0_j ) { _rtB ->
B_120_771_0 = _rtB -> B_120_770_0 ; } else { _rtB -> B_120_771_0 = _rtB ->
B_120_209_0_i ; } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) {
ssCallAccelRunBlock ( S , 85 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP ->
P_1034 != 0 ) && ( ! ( _rtB -> B_85_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_1034
== 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_cd == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model4/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_cd < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_cd ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model4/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_229_0_e ) ; if ( ( _rtP -> P_1034 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_1034 == 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_cd == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model4/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_cd < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_cd ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model4/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_229_0_e ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_229_0_e ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_773_0 =
_rtB -> B_120_228_0_h [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_85_0_0 , 499U ) ] ; _rtB -> B_120_774_0 =
_rtB -> B_120_235_0_a * _rtB -> B_120_773_0 ; if ( ( _rtP -> P_1035 != 0 ) &&
( ! ( _rtB -> B_85_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_1035 == 1 ) { if (
_rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_m == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_m < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_m ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_230_0_c ) ; if ( ( _rtP -> P_1035 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_1035 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_m == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_m < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_m ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_230_0_c ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_230_0_c ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_775_0 =
_rtB -> B_120_228_0_h [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_85_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_d0 . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_d0 .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_489 ; _rtB -> B_120_776_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_lq . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_lq . Last , _rtDW -> TransportDelay_IWORK_lq . Tail ,
_rtDW -> TransportDelay_IWORK_lq . Head , _rtP -> P_490 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_120_777_0 = 0.0 ; _rtB -> B_120_777_0 += _rtP -> P_492 * _rtX
-> TransferFcn_CSTATE_o0 ; _rtB -> B_120_777_0 += _rtP -> P_493 * _rtB ->
B_120_776_0 ; if ( _rtB -> B_120_210_0_j ) { if ( _rtB -> B_120_777_0 > _rtP
-> P_51 ) { _rtB -> B_75_0_0 = _rtB -> B_120_777_0 - _rtP -> P_51 ; } else if
( _rtB -> B_120_777_0 >= _rtP -> P_50 ) { _rtB -> B_75_0_0 = 0.0 ; } else {
_rtB -> B_75_0_0 = _rtB -> B_120_777_0 - _rtP -> P_50 ; } _rtB -> B_120_779_0
= _rtB -> B_75_0_0 ; } else { _rtB -> B_120_779_0 = _rtB -> B_120_209_0_i ; }
_rtB -> B_120_780_0 = _rtP -> P_494 * _rtB -> B_120_779_0 ; if ( _rtB ->
B_120_780_0 > _rtP -> P_495 ) { _rtB -> B_120_781_0 = _rtP -> P_495 ; } else
if ( _rtB -> B_120_780_0 < _rtP -> P_496 ) { _rtB -> B_120_781_0 = _rtP ->
P_496 ; } else { _rtB -> B_120_781_0 = _rtB -> B_120_780_0 ; } if ( ( _rtDW
-> LastMajorTimeA_jn >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW ->
LastMajorTimeB_o >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_120_782_0 = _rtB
-> B_120_781_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_jn < _rtDW ->
LastMajorTimeB_o ) && ( _rtDW -> LastMajorTimeB_o < ssGetTaskTime ( S , 0 ) )
) || ( ( _rtDW -> LastMajorTimeA_jn >= _rtDW -> LastMajorTimeB_o ) && ( _rtDW
-> LastMajorTimeA_jn >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_o ; rtb_B_120_5_0 = _rtDW
-> PrevYB_hm ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_jn ; rtb_B_120_5_0 = _rtDW -> PrevYA_or ; } riseValLimit =
deltaT * _rtP -> P_497 ; rateLimiterRate = _rtB -> B_120_781_0 -
rtb_B_120_5_0 ; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_120_782_0 =
rtb_B_120_5_0 + riseValLimit ; } else { deltaT *= _rtP -> P_498 ; if (
rateLimiterRate < deltaT ) { _rtB -> B_120_782_0 = rtb_B_120_5_0 + deltaT ; }
else { _rtB -> B_120_782_0 = _rtB -> B_120_781_0 ; } } } isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB -> B_120_783_0 = _rtP
-> P_499 * _rtB -> B_120_773_0 ; } _rtB -> B_120_784_0 = _rtB -> B_120_782_0
* _rtB -> B_120_783_0 ; _rtB -> B_120_785_0 = _rtB -> B_120_775_0 * _rtB ->
B_120_784_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model4/Data Store Read" ,
0 ) ; _rtB -> B_120_786_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_od != 0 )
{ _rtX -> vv0at_CSTATE_gb = _rtB -> B_120_786_0 ; } _rtB -> B_120_787_0 =
_rtX -> vv0at_CSTATE_gb ; _rtB -> B_120_788_0 = muDoubleScalarAbs ( _rtB ->
B_120_787_0 ) ; _rtB -> B_120_789_0 = _rtP -> P_500 [ 0 ] ; _rtB ->
B_120_789_0 = _rtB -> B_120_789_0 * _rtB -> B_120_788_0 + _rtP -> P_500 [ 1 ]
; _rtB -> B_120_789_0 = _rtB -> B_120_789_0 * _rtB -> B_120_788_0 + _rtP ->
P_500 [ 2 ] ; _rtB -> B_120_790_0 = _rtP -> P_501 * _rtB -> B_120_789_0 ;
_rtB -> B_120_791_0 = _rtB -> B_120_773_0 * _rtB -> B_120_790_0 ; _rtB ->
B_120_792_0 = _rtB -> B_120_785_0 - _rtB -> B_120_791_0 ; isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB -> B_120_794_0 = _rtP
-> P_502 * _rtB -> B_120_774_0 ; } if ( _rtB -> B_120_792_0 > _rtB ->
B_120_774_0 ) { _rtB -> B_120_796_0 = _rtB -> B_120_774_0 ; } else { if (
_rtB -> B_120_792_0 < _rtB -> B_120_794_0 ) { _rtB -> B_86_1_0 = _rtB ->
B_120_794_0 ; } else { _rtB -> B_86_1_0 = _rtB -> B_120_792_0 ; } _rtB ->
B_120_796_0 = _rtB -> B_86_1_0 ; } if ( _rtB -> B_120_234_0_o ) { _rtB ->
B_120_797_0 = _rtB -> B_120_796_0 ; } else { _rtB -> B_120_797_0 = _rtB ->
B_120_233_0_l ; } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) {
ssCallAccelRunBlock ( S , 118 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP ->
P_1036 != 0 ) && ( ! ( _rtB -> B_118_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_1036
== 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_h2 == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem8/Wagon model5/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_h2 < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_h2 ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem8/Wagon model5/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_317_0_a ) ; if ( ( _rtP -> P_1036 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_1036 == 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_h2 == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem8/Wagon model5/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_h2 < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_h2 ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem8/Wagon model5/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_317_0_a ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_317_0_a ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_799_0 =
_rtB -> B_120_316_0_f [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_118_0_0 , 499U ) ] ; _rtB -> B_120_800_0 =
_rtB -> B_120_323_0_n * _rtB -> B_120_799_0 ; if ( ( _rtP -> P_1037 != 0 ) &&
( ! ( _rtB -> B_118_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_1037 == 1 ) { if (
_rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_h == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem8/Wagon model5/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_h < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_h ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem8/Wagon model5/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_318_0_g ) ; if ( ( _rtP -> P_1037 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_1037 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_h == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem8/Wagon model5/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_h < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_h ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem8/Wagon model5/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_318_0_g ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_318_0_g ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_801_0 =
_rtB -> B_120_316_0_f [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_118_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_n3 . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_n3 .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_503 ; _rtB -> B_120_802_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_d2 . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_d2 . Last , _rtDW -> TransportDelay_IWORK_d2 . Tail ,
_rtDW -> TransportDelay_IWORK_d2 . Head , _rtP -> P_504 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_120_803_0 = 0.0 ; _rtB -> B_120_803_0 += _rtP -> P_506 * _rtX
-> TransferFcn_CSTATE_ap ; _rtB -> B_120_803_0 += _rtP -> P_507 * _rtB ->
B_120_802_0 ; if ( _rtB -> B_120_234_0_o ) { if ( _rtB -> B_120_803_0 > _rtP
-> P_57 ) { _rtB -> B_84_0_0 = _rtB -> B_120_803_0 - _rtP -> P_57 ; } else if
( _rtB -> B_120_803_0 >= _rtP -> P_56 ) { _rtB -> B_84_0_0 = 0.0 ; } else {
_rtB -> B_84_0_0 = _rtB -> B_120_803_0 - _rtP -> P_56 ; } _rtB -> B_120_805_0
= _rtB -> B_84_0_0 ; } else { _rtB -> B_120_805_0 = _rtB -> B_120_233_0_l ; }
_rtB -> B_120_806_0 = _rtP -> P_508 * _rtB -> B_120_805_0 ; if ( _rtB ->
B_120_806_0 > _rtP -> P_509 ) { _rtB -> B_120_807_0 = _rtP -> P_509 ; } else
if ( _rtB -> B_120_806_0 < _rtP -> P_510 ) { _rtB -> B_120_807_0 = _rtP ->
P_510 ; } else { _rtB -> B_120_807_0 = _rtB -> B_120_806_0 ; } if ( ( _rtDW
-> LastMajorTimeA_o >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW ->
LastMajorTimeB_ju >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_120_808_0 = _rtB
-> B_120_807_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_o < _rtDW ->
LastMajorTimeB_ju ) && ( _rtDW -> LastMajorTimeB_ju < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_o >= _rtDW -> LastMajorTimeB_ju ) && (
_rtDW -> LastMajorTimeA_o >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_ju ; rtb_B_120_5_0 = _rtDW
-> PrevYB_a5 ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_o ; rtb_B_120_5_0 = _rtDW -> PrevYA_au ; } riseValLimit =
deltaT * _rtP -> P_511 ; rateLimiterRate = _rtB -> B_120_807_0 -
rtb_B_120_5_0 ; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_120_808_0 =
rtb_B_120_5_0 + riseValLimit ; } else { deltaT *= _rtP -> P_512 ; if (
rateLimiterRate < deltaT ) { _rtB -> B_120_808_0 = rtb_B_120_5_0 + deltaT ; }
else { _rtB -> B_120_808_0 = _rtB -> B_120_807_0 ; } } } isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB -> B_120_809_0 = _rtP
-> P_513 * _rtB -> B_120_799_0 ; } _rtB -> B_120_810_0 = _rtB -> B_120_808_0
* _rtB -> B_120_809_0 ; _rtB -> B_120_811_0 = _rtB -> B_120_801_0 * _rtB ->
B_120_810_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem8/Wagon model5/Data Store Read" ,
0 ) ; _rtB -> B_120_812_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_ff != 0 )
{ _rtX -> vv0at_CSTATE_as = _rtB -> B_120_812_0 ; } _rtB -> B_120_813_0 =
_rtX -> vv0at_CSTATE_as ; _rtB -> B_120_814_0 = muDoubleScalarAbs ( _rtB ->
B_120_813_0 ) ; _rtB -> B_120_815_0 = _rtP -> P_514 [ 0 ] ; _rtB ->
B_120_815_0 = _rtB -> B_120_815_0 * _rtB -> B_120_814_0 + _rtP -> P_514 [ 1 ]
; _rtB -> B_120_815_0 = _rtB -> B_120_815_0 * _rtB -> B_120_814_0 + _rtP ->
P_514 [ 2 ] ; _rtB -> B_120_816_0 = _rtP -> P_515 * _rtB -> B_120_815_0 ;
_rtB -> B_120_817_0 = _rtB -> B_120_799_0 * _rtB -> B_120_816_0 ; _rtB ->
B_120_818_0 = _rtB -> B_120_811_0 - _rtB -> B_120_817_0 ; isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB -> B_120_820_0 = _rtP
-> P_516 * _rtB -> B_120_800_0 ; } if ( _rtB -> B_120_818_0 > _rtB ->
B_120_800_0 ) { _rtB -> B_120_822_0 = _rtB -> B_120_800_0 ; } else { if (
_rtB -> B_120_818_0 < _rtB -> B_120_820_0 ) { _rtB -> B_119_1_0 = _rtB ->
B_120_820_0 ; } else { _rtB -> B_119_1_0 = _rtB -> B_120_818_0 ; } _rtB ->
B_120_822_0 = _rtB -> B_119_1_0 ; } if ( _rtB -> B_120_322_0_i ) { _rtB ->
B_120_823_0 = _rtB -> B_120_822_0 ; } else { _rtB -> B_120_823_0 = _rtB ->
B_120_321_0_b ; } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) {
ssCallAccelRunBlock ( S , 112 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP ->
P_1038 != 0 ) && ( ! ( _rtB -> B_112_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_1038
== 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_mh == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem8/Wagon model3/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_mh < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_mh ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem8/Wagon model3/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_301_0 ) ; if ( ( _rtP -> P_1038 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_1038 == 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_mh == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem8/Wagon model3/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_mh < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_mh ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem8/Wagon model3/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_301_0 ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_301_0 ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_825_0 =
_rtB -> B_120_300_0_m [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_112_0_0 , 499U ) ] ; _rtB -> B_120_826_0 =
_rtB -> B_120_307_0_e * _rtB -> B_120_825_0 ; if ( ( _rtP -> P_1039 != 0 ) &&
( ! ( _rtB -> B_112_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_1039 == 1 ) { if (
_rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_oo == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem8/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_oo < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_oo ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem8/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_302_0_j ) ; if ( ( _rtP -> P_1039 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_1039 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_oo == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem8/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_oo < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_oo ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem8/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_302_0_j ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_302_0_j ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_827_0 =
_rtB -> B_120_300_0_m [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_112_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_gf . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_gf .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_517 ; _rtB -> B_120_828_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_c . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_c . Last , _rtDW -> TransportDelay_IWORK_c . Tail ,
_rtDW -> TransportDelay_IWORK_c . Head , _rtP -> P_518 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_120_829_0 = 0.0 ; _rtB -> B_120_829_0 += _rtP -> P_520 * _rtX
-> TransferFcn_CSTATE_fc ; _rtB -> B_120_829_0 += _rtP -> P_521 * _rtB ->
B_120_828_0 ; if ( _rtB -> B_120_322_0_i ) { if ( _rtB -> B_120_829_0 > _rtP
-> P_79 ) { _rtB -> B_117_0_0 = _rtB -> B_120_829_0 - _rtP -> P_79 ; } else
if ( _rtB -> B_120_829_0 >= _rtP -> P_78 ) { _rtB -> B_117_0_0 = 0.0 ; } else
{ _rtB -> B_117_0_0 = _rtB -> B_120_829_0 - _rtP -> P_78 ; } _rtB ->
B_120_831_0 = _rtB -> B_117_0_0 ; } else { _rtB -> B_120_831_0 = _rtB ->
B_120_321_0_b ; } _rtB -> B_120_832_0 = _rtP -> P_522 * _rtB -> B_120_831_0 ;
if ( _rtB -> B_120_832_0 > _rtP -> P_523 ) { _rtB -> B_120_833_0 = _rtP ->
P_523 ; } else if ( _rtB -> B_120_832_0 < _rtP -> P_524 ) { _rtB ->
B_120_833_0 = _rtP -> P_524 ; } else { _rtB -> B_120_833_0 = _rtB ->
B_120_832_0 ; } if ( ( _rtDW -> LastMajorTimeA_ev >= ssGetTaskTime ( S , 0 )
) && ( _rtDW -> LastMajorTimeB_h0 >= ssGetTaskTime ( S , 0 ) ) ) { _rtB ->
B_120_834_0 = _rtB -> B_120_833_0 ; } else { if ( ( ( _rtDW ->
LastMajorTimeA_ev < _rtDW -> LastMajorTimeB_h0 ) && ( _rtDW ->
LastMajorTimeB_h0 < ssGetTaskTime ( S , 0 ) ) ) || ( ( _rtDW ->
LastMajorTimeA_ev >= _rtDW -> LastMajorTimeB_h0 ) && ( _rtDW ->
LastMajorTimeA_ev >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime (
S , 0 ) - _rtDW -> LastMajorTimeB_h0 ; rtb_B_120_5_0 = _rtDW -> PrevYB_gn ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_ev ;
rtb_B_120_5_0 = _rtDW -> PrevYA_m3 ; } riseValLimit = deltaT * _rtP -> P_525
; rateLimiterRate = _rtB -> B_120_833_0 - rtb_B_120_5_0 ; if (
rateLimiterRate > riseValLimit ) { _rtB -> B_120_834_0 = rtb_B_120_5_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_526 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_120_834_0 = rtb_B_120_5_0 + deltaT ; } else { _rtB ->
B_120_834_0 = _rtB -> B_120_833_0 ; } } } isHit = ssIsSampleHit ( S , 2 , 0 )
; if ( isHit != 0 ) { _rtB -> B_120_835_0 = _rtP -> P_527 * _rtB ->
B_120_825_0 ; } _rtB -> B_120_836_0 = _rtB -> B_120_834_0 * _rtB ->
B_120_835_0 ; _rtB -> B_120_837_0 = _rtB -> B_120_827_0 * _rtB -> B_120_836_0
; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem8/Wagon model3/Data Store Read" ,
0 ) ; _rtB -> B_120_838_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_a3 != 0 )
{ _rtX -> vv0at_CSTATE_jy = _rtB -> B_120_838_0 ; } _rtB -> B_120_839_0 =
_rtX -> vv0at_CSTATE_jy ; _rtB -> B_120_840_0 = muDoubleScalarAbs ( _rtB ->
B_120_839_0 ) ; _rtB -> B_120_841_0 = _rtP -> P_528 [ 0 ] ; _rtB ->
B_120_841_0 = _rtB -> B_120_841_0 * _rtB -> B_120_840_0 + _rtP -> P_528 [ 1 ]
; _rtB -> B_120_841_0 = _rtB -> B_120_841_0 * _rtB -> B_120_840_0 + _rtP ->
P_528 [ 2 ] ; _rtB -> B_120_842_0 = _rtP -> P_529 * _rtB -> B_120_841_0 ;
_rtB -> B_120_843_0 = _rtB -> B_120_825_0 * _rtB -> B_120_842_0 ; _rtB ->
B_120_844_0 = _rtB -> B_120_837_0 - _rtB -> B_120_843_0 ; isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB -> B_120_846_0 = _rtP
-> P_530 * _rtB -> B_120_826_0 ; } if ( _rtB -> B_120_844_0 > _rtB ->
B_120_826_0 ) { _rtB -> B_120_848_0 = _rtB -> B_120_826_0 ; } else { if (
_rtB -> B_120_844_0 < _rtB -> B_120_846_0 ) { _rtB -> B_113_1_0 = _rtB ->
B_120_846_0 ; } else { _rtB -> B_113_1_0 = _rtB -> B_120_844_0 ; } _rtB ->
B_120_848_0 = _rtB -> B_113_1_0 ; } if ( _rtB -> B_120_306_0_i ) { _rtB ->
B_120_849_0 = _rtB -> B_120_848_0 ; } else { _rtB -> B_120_849_0 = _rtB ->
B_120_305_0_o ; } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) {
ssCallAccelRunBlock ( S , 109 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP ->
P_1040 != 0 ) && ( ! ( _rtB -> B_109_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_1040
== 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_b == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem8/Wagon model2/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_b < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_b ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem8/Wagon model2/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_293_0_n ) ; if ( ( _rtP -> P_1040 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_1040 == 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_b == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem8/Wagon model2/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_b < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_b ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem8/Wagon model2/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_293_0_n ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_293_0_n ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_851_0 =
_rtB -> B_120_292_0_c [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_109_0_0 , 499U ) ] ; _rtB -> B_120_852_0 =
_rtB -> B_120_299_0 * _rtB -> B_120_851_0 ; if ( ( _rtP -> P_1041 != 0 ) && (
! ( _rtB -> B_109_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_1041 == 1 ) { if (
_rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_lv == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem8/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_lv < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_lv ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem8/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_294_0_i ) ; if ( ( _rtP -> P_1041 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_1041 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_lv == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem8/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_lv < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_lv ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem8/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_294_0_i ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_294_0_i ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_853_0 =
_rtB -> B_120_292_0_c [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_109_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_mc . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_mc .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_531 ; _rtB -> B_120_854_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_h . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_h . Last , _rtDW -> TransportDelay_IWORK_h . Tail ,
_rtDW -> TransportDelay_IWORK_h . Head , _rtP -> P_532 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_120_855_0 = 0.0 ; _rtB -> B_120_855_0 += _rtP -> P_534 * _rtX
-> TransferFcn_CSTATE_hl ; _rtB -> B_120_855_0 += _rtP -> P_535 * _rtB ->
B_120_854_0 ; if ( _rtB -> B_120_306_0_i ) { if ( _rtB -> B_120_855_0 > _rtP
-> P_75 ) { _rtB -> B_111_0_0 = _rtB -> B_120_855_0 - _rtP -> P_75 ; } else
if ( _rtB -> B_120_855_0 >= _rtP -> P_74 ) { _rtB -> B_111_0_0 = 0.0 ; } else
{ _rtB -> B_111_0_0 = _rtB -> B_120_855_0 - _rtP -> P_74 ; } _rtB ->
B_120_857_0 = _rtB -> B_111_0_0 ; } else { _rtB -> B_120_857_0 = _rtB ->
B_120_305_0_o ; } _rtB -> B_120_858_0 = _rtP -> P_536 * _rtB -> B_120_857_0 ;
if ( _rtB -> B_120_858_0 > _rtP -> P_537 ) { _rtB -> B_120_859_0 = _rtP ->
P_537 ; } else if ( _rtB -> B_120_858_0 < _rtP -> P_538 ) { _rtB ->
B_120_859_0 = _rtP -> P_538 ; } else { _rtB -> B_120_859_0 = _rtB ->
B_120_858_0 ; } if ( ( _rtDW -> LastMajorTimeA_bx >= ssGetTaskTime ( S , 0 )
) && ( _rtDW -> LastMajorTimeB_pt >= ssGetTaskTime ( S , 0 ) ) ) { _rtB ->
B_120_860_0 = _rtB -> B_120_859_0 ; } else { if ( ( ( _rtDW ->
LastMajorTimeA_bx < _rtDW -> LastMajorTimeB_pt ) && ( _rtDW ->
LastMajorTimeB_pt < ssGetTaskTime ( S , 0 ) ) ) || ( ( _rtDW ->
LastMajorTimeA_bx >= _rtDW -> LastMajorTimeB_pt ) && ( _rtDW ->
LastMajorTimeA_bx >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime (
S , 0 ) - _rtDW -> LastMajorTimeB_pt ; rtb_B_120_5_0 = _rtDW -> PrevYB_dt ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_bx ;
rtb_B_120_5_0 = _rtDW -> PrevYA_l ; } riseValLimit = deltaT * _rtP -> P_539 ;
rateLimiterRate = _rtB -> B_120_859_0 - rtb_B_120_5_0 ; if ( rateLimiterRate
> riseValLimit ) { _rtB -> B_120_860_0 = rtb_B_120_5_0 + riseValLimit ; }
else { deltaT *= _rtP -> P_540 ; if ( rateLimiterRate < deltaT ) { _rtB ->
B_120_860_0 = rtb_B_120_5_0 + deltaT ; } else { _rtB -> B_120_860_0 = _rtB ->
B_120_859_0 ; } } } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) {
_rtB -> B_120_861_0 = _rtP -> P_541 * _rtB -> B_120_851_0 ; } _rtB ->
B_120_862_0 = _rtB -> B_120_860_0 * _rtB -> B_120_861_0 ; _rtB -> B_120_863_0
= _rtB -> B_120_853_0 * _rtB -> B_120_862_0 ; vm_ReadLocalDSMNoIdx ( S ,
_rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem8/Wagon model2/Data Store Read" ,
0 ) ; _rtB -> B_120_864_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_fk != 0 )
{ _rtX -> vv0at_CSTATE_pa = _rtB -> B_120_864_0 ; } _rtB -> B_120_865_0 =
_rtX -> vv0at_CSTATE_pa ; _rtB -> B_120_866_0 = muDoubleScalarAbs ( _rtB ->
B_120_865_0 ) ; _rtB -> B_120_867_0 = _rtP -> P_542 [ 0 ] ; _rtB ->
B_120_867_0 = _rtB -> B_120_867_0 * _rtB -> B_120_866_0 + _rtP -> P_542 [ 1 ]
; _rtB -> B_120_867_0 = _rtB -> B_120_867_0 * _rtB -> B_120_866_0 + _rtP ->
P_542 [ 2 ] ; _rtB -> B_120_868_0 = _rtP -> P_543 * _rtB -> B_120_867_0 ;
_rtB -> B_120_869_0 = _rtB -> B_120_851_0 * _rtB -> B_120_868_0 ; _rtB ->
B_120_870_0 = _rtB -> B_120_863_0 - _rtB -> B_120_869_0 ; isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB -> B_120_872_0 = _rtP
-> P_544 * _rtB -> B_120_852_0 ; } if ( _rtB -> B_120_870_0 > _rtB ->
B_120_852_0 ) { _rtB -> B_120_874_0 = _rtB -> B_120_852_0 ; } else { if (
_rtB -> B_120_870_0 < _rtB -> B_120_872_0 ) { _rtB -> B_110_1_0 = _rtB ->
B_120_872_0 ; } else { _rtB -> B_110_1_0 = _rtB -> B_120_870_0 ; } _rtB ->
B_120_874_0 = _rtB -> B_110_1_0 ; } if ( _rtB -> B_120_298_0_n ) { _rtB ->
B_120_875_0 = _rtB -> B_120_874_0 ; } else { _rtB -> B_120_875_0 = _rtB ->
B_120_297_0_n ; } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) {
ssCallAccelRunBlock ( S , 106 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP ->
P_1042 != 0 ) && ( ! ( _rtB -> B_106_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_1042
== 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_l4 == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem8/Wagon model1/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_l4 < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_l4 ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem8/Wagon model1/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_285_0_k ) ; if ( ( _rtP -> P_1042 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_1042 == 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_l4 == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem8/Wagon model1/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_l4 < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_l4 ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem8/Wagon model1/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_285_0_k ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_285_0_k ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_877_0 =
_rtB -> B_120_284_0 [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_106_0_0 , 499U ) ] ; _rtB -> B_120_878_0 =
_rtB -> B_120_291_0_b * _rtB -> B_120_877_0 ; if ( ( _rtP -> P_1043 != 0 ) &&
( ! ( _rtB -> B_106_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_1043 == 1 ) { if (
_rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_ja == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem8/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_ja < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_ja ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem8/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_286_0_p ) ; if ( ( _rtP -> P_1043 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_1043 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_ja == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem8/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_ja < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_ja ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem8/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_286_0_p ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_286_0_p ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_879_0 =
_rtB -> B_120_284_0 [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_106_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_km . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_km .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_545 ; _rtB -> B_120_880_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_p5 . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_p5 . Last , _rtDW -> TransportDelay_IWORK_p5 . Tail ,
_rtDW -> TransportDelay_IWORK_p5 . Head , _rtP -> P_546 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_120_881_0 = 0.0 ; _rtB -> B_120_881_0 += _rtP -> P_548 * _rtX
-> TransferFcn_CSTATE_bw ; _rtB -> B_120_881_0 += _rtP -> P_549 * _rtB ->
B_120_880_0 ; if ( _rtB -> B_120_298_0_n ) { if ( _rtB -> B_120_881_0 > _rtP
-> P_73 ) { _rtB -> B_108_0_0 = _rtB -> B_120_881_0 - _rtP -> P_73 ; } else
if ( _rtB -> B_120_881_0 >= _rtP -> P_72 ) { _rtB -> B_108_0_0 = 0.0 ; } else
{ _rtB -> B_108_0_0 = _rtB -> B_120_881_0 - _rtP -> P_72 ; } _rtB ->
B_120_883_0 = _rtB -> B_108_0_0 ; } else { _rtB -> B_120_883_0 = _rtB ->
B_120_297_0_n ; } _rtB -> B_120_884_0 = _rtP -> P_550 * _rtB -> B_120_883_0 ;
if ( _rtB -> B_120_884_0 > _rtP -> P_551 ) { _rtB -> B_120_885_0 = _rtP ->
P_551 ; } else if ( _rtB -> B_120_884_0 < _rtP -> P_552 ) { _rtB ->
B_120_885_0 = _rtP -> P_552 ; } else { _rtB -> B_120_885_0 = _rtB ->
B_120_884_0 ; } if ( ( _rtDW -> LastMajorTimeA_hd >= ssGetTaskTime ( S , 0 )
) && ( _rtDW -> LastMajorTimeB_ho >= ssGetTaskTime ( S , 0 ) ) ) { _rtB ->
B_120_886_0 = _rtB -> B_120_885_0 ; } else { if ( ( ( _rtDW ->
LastMajorTimeA_hd < _rtDW -> LastMajorTimeB_ho ) && ( _rtDW ->
LastMajorTimeB_ho < ssGetTaskTime ( S , 0 ) ) ) || ( ( _rtDW ->
LastMajorTimeA_hd >= _rtDW -> LastMajorTimeB_ho ) && ( _rtDW ->
LastMajorTimeA_hd >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime (
S , 0 ) - _rtDW -> LastMajorTimeB_ho ; rtb_B_120_5_0 = _rtDW -> PrevYB_n4 ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_hd ;
rtb_B_120_5_0 = _rtDW -> PrevYA_n ; } riseValLimit = deltaT * _rtP -> P_553 ;
rateLimiterRate = _rtB -> B_120_885_0 - rtb_B_120_5_0 ; if ( rateLimiterRate
> riseValLimit ) { _rtB -> B_120_886_0 = rtb_B_120_5_0 + riseValLimit ; }
else { deltaT *= _rtP -> P_554 ; if ( rateLimiterRate < deltaT ) { _rtB ->
B_120_886_0 = rtb_B_120_5_0 + deltaT ; } else { _rtB -> B_120_886_0 = _rtB ->
B_120_885_0 ; } } } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) {
_rtB -> B_120_887_0 = _rtP -> P_555 * _rtB -> B_120_877_0 ; } _rtB ->
B_120_888_0 = _rtB -> B_120_886_0 * _rtB -> B_120_887_0 ; _rtB -> B_120_889_0
= _rtB -> B_120_879_0 * _rtB -> B_120_888_0 ; vm_ReadLocalDSMNoIdx ( S ,
_rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem8/Wagon model1/Data Store Read" ,
0 ) ; _rtB -> B_120_890_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_dk != 0 )
{ _rtX -> vv0at_CSTATE_gg = _rtB -> B_120_890_0 ; } _rtB -> B_120_891_0 =
_rtX -> vv0at_CSTATE_gg ; _rtB -> B_120_892_0 = muDoubleScalarAbs ( _rtB ->
B_120_891_0 ) ; _rtB -> B_120_893_0 = _rtP -> P_556 [ 0 ] ; _rtB ->
B_120_893_0 = _rtB -> B_120_893_0 * _rtB -> B_120_892_0 + _rtP -> P_556 [ 1 ]
; _rtB -> B_120_893_0 = _rtB -> B_120_893_0 * _rtB -> B_120_892_0 + _rtP ->
P_556 [ 2 ] ; _rtB -> B_120_894_0 = _rtP -> P_557 * _rtB -> B_120_893_0 ;
_rtB -> B_120_895_0 = _rtB -> B_120_877_0 * _rtB -> B_120_894_0 ; _rtB ->
B_120_896_0 = _rtB -> B_120_889_0 - _rtB -> B_120_895_0 ; isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB -> B_120_898_0 = _rtP
-> P_558 * _rtB -> B_120_878_0 ; } if ( _rtB -> B_120_896_0 > _rtB ->
B_120_878_0 ) { _rtB -> B_120_900_0 = _rtB -> B_120_878_0 ; } else { if (
_rtB -> B_120_896_0 < _rtB -> B_120_898_0 ) { _rtB -> B_107_1_0 = _rtB ->
B_120_898_0 ; } else { _rtB -> B_107_1_0 = _rtB -> B_120_896_0 ; } _rtB ->
B_120_900_0 = _rtB -> B_107_1_0 ; } if ( _rtB -> B_120_290_0_f ) { _rtB ->
B_120_901_0 = _rtB -> B_120_900_0 ; } else { _rtB -> B_120_901_0 = _rtB ->
B_120_289_0_h ; } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) {
ssCallAccelRunBlock ( S , 115 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP ->
P_1044 != 0 ) && ( ! ( _rtB -> B_115_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_1044
== 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_i0 == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem8/Wagon model4/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_i0 < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_i0 ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem8/Wagon model4/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_309_0_m ) ; if ( ( _rtP -> P_1044 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_1044 == 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_i0 == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem8/Wagon model4/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_i0 < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_i0 ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem8/Wagon model4/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_309_0_m ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_309_0_m ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_903_0 =
_rtB -> B_120_308_0_m [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_115_0_0 , 499U ) ] ; _rtB -> B_120_904_0 =
_rtB -> B_120_315_0_j * _rtB -> B_120_903_0 ; if ( ( _rtP -> P_1045 != 0 ) &&
( ! ( _rtB -> B_115_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_1045 == 1 ) { if (
_rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_hb == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem8/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_hb < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_hb ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem8/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_310_0 ) ; if ( ( _rtP -> P_1045 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_1045 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_hb == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem8/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_hb < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_hb ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem8/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_310_0 ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_310_0 ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_905_0 =
_rtB -> B_120_308_0_m [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_115_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_h . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_h .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_559 ; _rtB -> B_120_906_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_il . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_il . Last , _rtDW -> TransportDelay_IWORK_il . Tail ,
_rtDW -> TransportDelay_IWORK_il . Head , _rtP -> P_560 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_120_907_0 = 0.0 ; _rtB -> B_120_907_0 += _rtP -> P_562 * _rtX
-> TransferFcn_CSTATE_d ; _rtB -> B_120_907_0 += _rtP -> P_563 * _rtB ->
B_120_906_0 ; if ( _rtB -> B_120_290_0_f ) { if ( _rtB -> B_120_907_0 > _rtP
-> P_71 ) { _rtB -> B_105_0_0 = _rtB -> B_120_907_0 - _rtP -> P_71 ; } else
if ( _rtB -> B_120_907_0 >= _rtP -> P_70 ) { _rtB -> B_105_0_0 = 0.0 ; } else
{ _rtB -> B_105_0_0 = _rtB -> B_120_907_0 - _rtP -> P_70 ; } _rtB ->
B_120_909_0 = _rtB -> B_105_0_0 ; } else { _rtB -> B_120_909_0 = _rtB ->
B_120_289_0_h ; } _rtB -> B_120_910_0 = _rtP -> P_564 * _rtB -> B_120_909_0 ;
if ( _rtB -> B_120_910_0 > _rtP -> P_565 ) { _rtB -> B_120_911_0 = _rtP ->
P_565 ; } else if ( _rtB -> B_120_910_0 < _rtP -> P_566 ) { _rtB ->
B_120_911_0 = _rtP -> P_566 ; } else { _rtB -> B_120_911_0 = _rtB ->
B_120_910_0 ; } if ( ( _rtDW -> LastMajorTimeA_g >= ssGetTaskTime ( S , 0 ) )
&& ( _rtDW -> LastMajorTimeB_b5 >= ssGetTaskTime ( S , 0 ) ) ) { _rtB ->
B_120_912_0 = _rtB -> B_120_911_0 ; } else { if ( ( ( _rtDW ->
LastMajorTimeA_g < _rtDW -> LastMajorTimeB_b5 ) && ( _rtDW ->
LastMajorTimeB_b5 < ssGetTaskTime ( S , 0 ) ) ) || ( ( _rtDW ->
LastMajorTimeA_g >= _rtDW -> LastMajorTimeB_b5 ) && ( _rtDW ->
LastMajorTimeA_g >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime (
S , 0 ) - _rtDW -> LastMajorTimeB_b5 ; rtb_B_120_5_0 = _rtDW -> PrevYB_gy ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_g ;
rtb_B_120_5_0 = _rtDW -> PrevYA_pd ; } riseValLimit = deltaT * _rtP -> P_567
; rateLimiterRate = _rtB -> B_120_911_0 - rtb_B_120_5_0 ; if (
rateLimiterRate > riseValLimit ) { _rtB -> B_120_912_0 = rtb_B_120_5_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_568 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_120_912_0 = rtb_B_120_5_0 + deltaT ; } else { _rtB ->
B_120_912_0 = _rtB -> B_120_911_0 ; } } } isHit = ssIsSampleHit ( S , 2 , 0 )
; if ( isHit != 0 ) { _rtB -> B_120_913_0 = _rtP -> P_569 * _rtB ->
B_120_903_0 ; } _rtB -> B_120_914_0 = _rtB -> B_120_912_0 * _rtB ->
B_120_913_0 ; _rtB -> B_120_915_0 = _rtB -> B_120_905_0 * _rtB -> B_120_914_0
; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem8/Wagon model4/Data Store Read" ,
0 ) ; _rtB -> B_120_916_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_lx != 0 )
{ _rtX -> vv0at_CSTATE_o5 = _rtB -> B_120_916_0 ; } _rtB -> B_120_917_0 =
_rtX -> vv0at_CSTATE_o5 ; _rtB -> B_120_918_0 = muDoubleScalarAbs ( _rtB ->
B_120_917_0 ) ; _rtB -> B_120_919_0 = _rtP -> P_570 [ 0 ] ; _rtB ->
B_120_919_0 = _rtB -> B_120_919_0 * _rtB -> B_120_918_0 + _rtP -> P_570 [ 1 ]
; _rtB -> B_120_919_0 = _rtB -> B_120_919_0 * _rtB -> B_120_918_0 + _rtP ->
P_570 [ 2 ] ; _rtB -> B_120_920_0 = _rtP -> P_571 * _rtB -> B_120_919_0 ;
_rtB -> B_120_921_0 = _rtB -> B_120_903_0 * _rtB -> B_120_920_0 ; _rtB ->
B_120_922_0 = _rtB -> B_120_915_0 - _rtB -> B_120_921_0 ; isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB -> B_120_924_0 = _rtP
-> P_572 * _rtB -> B_120_904_0 ; } if ( _rtB -> B_120_922_0 > _rtB ->
B_120_904_0 ) { _rtB -> B_120_926_0 = _rtB -> B_120_904_0 ; } else { if (
_rtB -> B_120_922_0 < _rtB -> B_120_924_0 ) { _rtB -> B_116_1_0 = _rtB ->
B_120_924_0 ; } else { _rtB -> B_116_1_0 = _rtB -> B_120_922_0 ; } _rtB ->
B_120_926_0 = _rtB -> B_116_1_0 ; } if ( _rtB -> B_120_314_0_l ) { _rtB ->
B_120_927_0 = _rtB -> B_120_926_0 ; } else { _rtB -> B_120_927_0 = _rtB ->
B_120_313_0_c ; } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) {
ssCallAccelRunBlock ( S , 103 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP ->
P_1046 != 0 ) && ( ! ( _rtB -> B_103_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_1046
== 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_oj == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model5/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_oj < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_oj ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model5/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_277_0_m ) ; if ( ( _rtP -> P_1046 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_1046 == 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_oj == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model5/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_oj < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_oj ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model5/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_277_0_m ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_277_0_m ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_929_0 =
_rtB -> B_120_276_0_h [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_103_0_0 , 499U ) ] ; _rtB -> B_120_930_0 =
_rtB -> B_120_283_0_a * _rtB -> B_120_929_0 ; if ( ( _rtP -> P_1047 != 0 ) &&
( ! ( _rtB -> B_103_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_1047 == 1 ) { if (
_rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_nr == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model5/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_nr < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_nr ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model5/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_278_0 ) ; if ( ( _rtP -> P_1047 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_1047 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_nr == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model5/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_nr < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_nr ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model5/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_278_0 ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_278_0 ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_931_0 =
_rtB -> B_120_276_0_h [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_103_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_ln . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_ln .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_573 ; _rtB -> B_120_932_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_a4 . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_a4 . Last , _rtDW -> TransportDelay_IWORK_a4 . Tail ,
_rtDW -> TransportDelay_IWORK_a4 . Head , _rtP -> P_574 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_120_933_0 = 0.0 ; _rtB -> B_120_933_0 += _rtP -> P_576 * _rtX
-> TransferFcn_CSTATE_h2 ; _rtB -> B_120_933_0 += _rtP -> P_577 * _rtB ->
B_120_932_0 ; if ( _rtB -> B_120_314_0_l ) { if ( _rtB -> B_120_933_0 > _rtP
-> P_77 ) { _rtB -> B_114_0_0 = _rtB -> B_120_933_0 - _rtP -> P_77 ; } else
if ( _rtB -> B_120_933_0 >= _rtP -> P_76 ) { _rtB -> B_114_0_0 = 0.0 ; } else
{ _rtB -> B_114_0_0 = _rtB -> B_120_933_0 - _rtP -> P_76 ; } _rtB ->
B_120_935_0 = _rtB -> B_114_0_0 ; } else { _rtB -> B_120_935_0 = _rtB ->
B_120_313_0_c ; } _rtB -> B_120_936_0 = _rtP -> P_578 * _rtB -> B_120_935_0 ;
if ( _rtB -> B_120_936_0 > _rtP -> P_579 ) { _rtB -> B_120_937_0 = _rtP ->
P_579 ; } else if ( _rtB -> B_120_936_0 < _rtP -> P_580 ) { _rtB ->
B_120_937_0 = _rtP -> P_580 ; } else { _rtB -> B_120_937_0 = _rtB ->
B_120_936_0 ; } if ( ( _rtDW -> LastMajorTimeA_ec >= ssGetTaskTime ( S , 0 )
) && ( _rtDW -> LastMajorTimeB_i4 >= ssGetTaskTime ( S , 0 ) ) ) { _rtB ->
B_120_938_0 = _rtB -> B_120_937_0 ; } else { if ( ( ( _rtDW ->
LastMajorTimeA_ec < _rtDW -> LastMajorTimeB_i4 ) && ( _rtDW ->
LastMajorTimeB_i4 < ssGetTaskTime ( S , 0 ) ) ) || ( ( _rtDW ->
LastMajorTimeA_ec >= _rtDW -> LastMajorTimeB_i4 ) && ( _rtDW ->
LastMajorTimeA_ec >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime (
S , 0 ) - _rtDW -> LastMajorTimeB_i4 ; rtb_B_120_5_0 = _rtDW -> PrevYB_am ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_ec ;
rtb_B_120_5_0 = _rtDW -> PrevYA_il ; } riseValLimit = deltaT * _rtP -> P_581
; rateLimiterRate = _rtB -> B_120_937_0 - rtb_B_120_5_0 ; if (
rateLimiterRate > riseValLimit ) { _rtB -> B_120_938_0 = rtb_B_120_5_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_582 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_120_938_0 = rtb_B_120_5_0 + deltaT ; } else { _rtB ->
B_120_938_0 = _rtB -> B_120_937_0 ; } } } isHit = ssIsSampleHit ( S , 2 , 0 )
; if ( isHit != 0 ) { _rtB -> B_120_939_0 = _rtP -> P_583 * _rtB ->
B_120_929_0 ; } _rtB -> B_120_940_0 = _rtB -> B_120_938_0 * _rtB ->
B_120_939_0 ; _rtB -> B_120_941_0 = _rtB -> B_120_931_0 * _rtB -> B_120_940_0
; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model5/Data Store Read" ,
0 ) ; _rtB -> B_120_942_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_ng != 0 )
{ _rtX -> vv0at_CSTATE_hm = _rtB -> B_120_942_0 ; } _rtB -> B_120_943_0 =
_rtX -> vv0at_CSTATE_hm ; _rtB -> B_120_944_0 = muDoubleScalarAbs ( _rtB ->
B_120_943_0 ) ; _rtB -> B_120_945_0 = _rtP -> P_584 [ 0 ] ; _rtB ->
B_120_945_0 = _rtB -> B_120_945_0 * _rtB -> B_120_944_0 + _rtP -> P_584 [ 1 ]
; _rtB -> B_120_945_0 = _rtB -> B_120_945_0 * _rtB -> B_120_944_0 + _rtP ->
P_584 [ 2 ] ; _rtB -> B_120_946_0 = _rtP -> P_585 * _rtB -> B_120_945_0 ;
_rtB -> B_120_947_0 = _rtB -> B_120_929_0 * _rtB -> B_120_946_0 ; _rtB ->
B_120_948_0 = _rtB -> B_120_941_0 - _rtB -> B_120_947_0 ; isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB -> B_120_950_0 = _rtP
-> P_586 * _rtB -> B_120_930_0 ; } if ( _rtB -> B_120_948_0 > _rtB ->
B_120_930_0 ) { _rtB -> B_120_952_0 = _rtB -> B_120_930_0 ; } else { if (
_rtB -> B_120_948_0 < _rtB -> B_120_950_0 ) { _rtB -> B_104_1_0 = _rtB ->
B_120_950_0 ; } else { _rtB -> B_104_1_0 = _rtB -> B_120_948_0 ; } _rtB ->
B_120_952_0 = _rtB -> B_104_1_0 ; } if ( _rtB -> B_120_282_0_c ) { _rtB ->
B_120_953_0 = _rtB -> B_120_952_0 ; } else { _rtB -> B_120_953_0 = _rtB ->
B_120_281_0_i ; } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) {
ssCallAccelRunBlock ( S , 97 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP ->
P_1048 != 0 ) && ( ! ( _rtB -> B_97_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_1048
== 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_iw == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model3/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_iw < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_iw ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model3/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_261_0_k ) ; if ( ( _rtP -> P_1048 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_1048 == 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_iw == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model3/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_iw < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_iw ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model3/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_261_0_k ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_261_0_k ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_955_0 =
_rtB -> B_120_260_0_o [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_97_0_0 , 499U ) ] ; _rtB -> B_120_956_0 =
_rtB -> B_120_267_0_o * _rtB -> B_120_955_0 ; if ( ( _rtP -> P_1049 != 0 ) &&
( ! ( _rtB -> B_97_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_1049 == 1 ) { if (
_rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_kz == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_kz < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_kz ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_262_0_i ) ; if ( ( _rtP -> P_1049 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_1049 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_kz == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_kz < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_kz ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_262_0_i ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_262_0_i ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_957_0 =
_rtB -> B_120_260_0_o [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_97_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_h5 . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_h5 .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_587 ; _rtB -> B_120_958_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_e . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_e . Last , _rtDW -> TransportDelay_IWORK_e . Tail ,
_rtDW -> TransportDelay_IWORK_e . Head , _rtP -> P_588 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_120_959_0 = 0.0 ; _rtB -> B_120_959_0 += _rtP -> P_590 * _rtX
-> TransferFcn_CSTATE_n1 ; _rtB -> B_120_959_0 += _rtP -> P_591 * _rtB ->
B_120_958_0 ; if ( _rtB -> B_120_282_0_c ) { if ( _rtB -> B_120_959_0 > _rtP
-> P_69 ) { _rtB -> B_102_0_0 = _rtB -> B_120_959_0 - _rtP -> P_69 ; } else
if ( _rtB -> B_120_959_0 >= _rtP -> P_68 ) { _rtB -> B_102_0_0 = 0.0 ; } else
{ _rtB -> B_102_0_0 = _rtB -> B_120_959_0 - _rtP -> P_68 ; } _rtB ->
B_120_961_0 = _rtB -> B_102_0_0 ; } else { _rtB -> B_120_961_0 = _rtB ->
B_120_281_0_i ; } _rtB -> B_120_962_0 = _rtP -> P_592 * _rtB -> B_120_961_0 ;
if ( _rtB -> B_120_962_0 > _rtP -> P_593 ) { _rtB -> B_120_963_0 = _rtP ->
P_593 ; } else if ( _rtB -> B_120_962_0 < _rtP -> P_594 ) { _rtB ->
B_120_963_0 = _rtP -> P_594 ; } else { _rtB -> B_120_963_0 = _rtB ->
B_120_962_0 ; } if ( ( _rtDW -> LastMajorTimeA_lp >= ssGetTaskTime ( S , 0 )
) && ( _rtDW -> LastMajorTimeB_jq >= ssGetTaskTime ( S , 0 ) ) ) { _rtB ->
B_120_964_0 = _rtB -> B_120_963_0 ; } else { if ( ( ( _rtDW ->
LastMajorTimeA_lp < _rtDW -> LastMajorTimeB_jq ) && ( _rtDW ->
LastMajorTimeB_jq < ssGetTaskTime ( S , 0 ) ) ) || ( ( _rtDW ->
LastMajorTimeA_lp >= _rtDW -> LastMajorTimeB_jq ) && ( _rtDW ->
LastMajorTimeA_lp >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime (
S , 0 ) - _rtDW -> LastMajorTimeB_jq ; rtb_B_120_5_0 = _rtDW -> PrevYB_o ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_lp ;
rtb_B_120_5_0 = _rtDW -> PrevYA_ag ; } riseValLimit = deltaT * _rtP -> P_595
; rateLimiterRate = _rtB -> B_120_963_0 - rtb_B_120_5_0 ; if (
rateLimiterRate > riseValLimit ) { _rtB -> B_120_964_0 = rtb_B_120_5_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_596 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_120_964_0 = rtb_B_120_5_0 + deltaT ; } else { _rtB ->
B_120_964_0 = _rtB -> B_120_963_0 ; } } } isHit = ssIsSampleHit ( S , 2 , 0 )
; if ( isHit != 0 ) { _rtB -> B_120_965_0 = _rtP -> P_597 * _rtB ->
B_120_955_0 ; } _rtB -> B_120_966_0 = _rtB -> B_120_964_0 * _rtB ->
B_120_965_0 ; _rtB -> B_120_967_0 = _rtB -> B_120_957_0 * _rtB -> B_120_966_0
; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model3/Data Store Read" ,
0 ) ; _rtB -> B_120_968_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_hk != 0 )
{ _rtX -> vv0at_CSTATE_fm = _rtB -> B_120_968_0 ; } _rtB -> B_120_969_0 =
_rtX -> vv0at_CSTATE_fm ; _rtB -> B_120_970_0 = muDoubleScalarAbs ( _rtB ->
B_120_969_0 ) ; _rtB -> B_120_971_0 = _rtP -> P_598 [ 0 ] ; _rtB ->
B_120_971_0 = _rtB -> B_120_971_0 * _rtB -> B_120_970_0 + _rtP -> P_598 [ 1 ]
; _rtB -> B_120_971_0 = _rtB -> B_120_971_0 * _rtB -> B_120_970_0 + _rtP ->
P_598 [ 2 ] ; _rtB -> B_120_972_0 = _rtP -> P_599 * _rtB -> B_120_971_0 ;
_rtB -> B_120_973_0 = _rtB -> B_120_955_0 * _rtB -> B_120_972_0 ; _rtB ->
B_120_974_0 = _rtB -> B_120_967_0 - _rtB -> B_120_973_0 ; isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB -> B_120_976_0 = _rtP
-> P_600 * _rtB -> B_120_956_0 ; } if ( _rtB -> B_120_974_0 > _rtB ->
B_120_956_0 ) { _rtB -> B_120_978_0 = _rtB -> B_120_956_0 ; } else { if (
_rtB -> B_120_974_0 < _rtB -> B_120_976_0 ) { _rtB -> B_98_1_0 = _rtB ->
B_120_976_0 ; } else { _rtB -> B_98_1_0 = _rtB -> B_120_974_0 ; } _rtB ->
B_120_978_0 = _rtB -> B_98_1_0 ; } if ( _rtB -> B_120_266_0_j ) { _rtB ->
B_120_979_0 = _rtB -> B_120_978_0 ; } else { _rtB -> B_120_979_0 = _rtB ->
B_120_265_0_j ; } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) {
ssCallAccelRunBlock ( S , 94 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP ->
P_1050 != 0 ) && ( ! ( _rtB -> B_94_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_1050
== 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_bl == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model2/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_bl < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_bl ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model2/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_253_0_p ) ; if ( ( _rtP -> P_1050 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_1050 == 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_bl == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model2/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_bl < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_bl ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model2/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_253_0_p ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_253_0_p ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_981_0 =
_rtB -> B_120_252_0 [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_94_0_0 , 499U ) ] ; _rtB -> B_120_982_0 =
_rtB -> B_120_259_0_i * _rtB -> B_120_981_0 ; if ( ( _rtP -> P_1051 != 0 ) &&
( ! ( _rtB -> B_94_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_1051 == 1 ) { if (
_rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_ly == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_ly < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_ly ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_254_0_f ) ; if ( ( _rtP -> P_1051 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_1051 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_ly == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_ly < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_ly ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_254_0_f ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_254_0_f ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_983_0 =
_rtB -> B_120_252_0 [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_94_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_b . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_b .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_601 ; _rtB -> B_120_984_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_m2 . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_m2 . Last , _rtDW -> TransportDelay_IWORK_m2 . Tail ,
_rtDW -> TransportDelay_IWORK_m2 . Head , _rtP -> P_602 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_120_985_0 = 0.0 ; _rtB -> B_120_985_0 += _rtP -> P_604 * _rtX
-> TransferFcn_CSTATE_eg ; _rtB -> B_120_985_0 += _rtP -> P_605 * _rtB ->
B_120_984_0 ; if ( _rtB -> B_120_266_0_j ) { if ( _rtB -> B_120_985_0 > _rtP
-> P_65 ) { _rtB -> B_96_0_0 = _rtB -> B_120_985_0 - _rtP -> P_65 ; } else if
( _rtB -> B_120_985_0 >= _rtP -> P_64 ) { _rtB -> B_96_0_0 = 0.0 ; } else {
_rtB -> B_96_0_0 = _rtB -> B_120_985_0 - _rtP -> P_64 ; } _rtB -> B_120_987_0
= _rtB -> B_96_0_0 ; } else { _rtB -> B_120_987_0 = _rtB -> B_120_265_0_j ; }
_rtB -> B_120_988_0 = _rtP -> P_606 * _rtB -> B_120_987_0 ; if ( _rtB ->
B_120_988_0 > _rtP -> P_607 ) { _rtB -> B_120_989_0 = _rtP -> P_607 ; } else
if ( _rtB -> B_120_988_0 < _rtP -> P_608 ) { _rtB -> B_120_989_0 = _rtP ->
P_608 ; } else { _rtB -> B_120_989_0 = _rtB -> B_120_988_0 ; } if ( ( _rtDW
-> LastMajorTimeA_ae >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW ->
LastMajorTimeB_hb >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_120_990_0 = _rtB
-> B_120_989_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_ae < _rtDW ->
LastMajorTimeB_hb ) && ( _rtDW -> LastMajorTimeB_hb < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_ae >= _rtDW -> LastMajorTimeB_hb ) && (
_rtDW -> LastMajorTimeA_ae >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_hb ; rtb_B_120_5_0 = _rtDW
-> PrevYB_f3 ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_ae ; rtb_B_120_5_0 = _rtDW -> PrevYA_at ; } riseValLimit =
deltaT * _rtP -> P_609 ; rateLimiterRate = _rtB -> B_120_989_0 -
rtb_B_120_5_0 ; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_120_990_0 =
rtb_B_120_5_0 + riseValLimit ; } else { deltaT *= _rtP -> P_610 ; if (
rateLimiterRate < deltaT ) { _rtB -> B_120_990_0 = rtb_B_120_5_0 + deltaT ; }
else { _rtB -> B_120_990_0 = _rtB -> B_120_989_0 ; } } } isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB -> B_120_991_0 = _rtP
-> P_611 * _rtB -> B_120_981_0 ; } _rtB -> B_120_992_0 = _rtB -> B_120_990_0
* _rtB -> B_120_991_0 ; _rtB -> B_120_993_0 = _rtB -> B_120_983_0 * _rtB ->
B_120_992_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model2/Data Store Read" ,
0 ) ; _rtB -> B_120_994_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_fv != 0 )
{ _rtX -> vv0at_CSTATE_n = _rtB -> B_120_994_0 ; } _rtB -> B_120_995_0 = _rtX
-> vv0at_CSTATE_n ; _rtB -> B_120_996_0 = muDoubleScalarAbs ( _rtB ->
B_120_995_0 ) ; _rtB -> B_120_997_0 = _rtP -> P_612 [ 0 ] ; _rtB ->
B_120_997_0 = _rtB -> B_120_997_0 * _rtB -> B_120_996_0 + _rtP -> P_612 [ 1 ]
; _rtB -> B_120_997_0 = _rtB -> B_120_997_0 * _rtB -> B_120_996_0 + _rtP ->
P_612 [ 2 ] ; _rtB -> B_120_998_0 = _rtP -> P_613 * _rtB -> B_120_997_0 ;
_rtB -> B_120_999_0 = _rtB -> B_120_981_0 * _rtB -> B_120_998_0 ; _rtB ->
B_120_1000_0 = _rtB -> B_120_993_0 - _rtB -> B_120_999_0 ; isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB -> B_120_1002_0 = _rtP
-> P_614 * _rtB -> B_120_982_0 ; } if ( _rtB -> B_120_1000_0 > _rtB ->
B_120_982_0 ) { _rtB -> B_120_1004_0 = _rtB -> B_120_982_0 ; } else { if (
_rtB -> B_120_1000_0 < _rtB -> B_120_1002_0 ) { _rtB -> B_95_1_0 = _rtB ->
B_120_1002_0 ; } else { _rtB -> B_95_1_0 = _rtB -> B_120_1000_0 ; } _rtB ->
B_120_1004_0 = _rtB -> B_95_1_0 ; } if ( _rtB -> B_120_258_0 ) { _rtB ->
B_120_1005_0 = _rtB -> B_120_1004_0 ; } else { _rtB -> B_120_1005_0 = _rtB ->
B_120_257_0_d ; } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) {
ssCallAccelRunBlock ( S , 91 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP ->
P_1052 != 0 ) && ( ! ( _rtB -> B_91_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_1052
== 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_os == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model1/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_os < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_os ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model1/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_245_0_n ) ; if ( ( _rtP -> P_1052 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_1052 == 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_os == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model1/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_os < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_os ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model1/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_245_0_n ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_245_0_n ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_1007_0 =
_rtB -> B_120_244_0_o [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_91_0_0 , 499U ) ] ; _rtB -> B_120_1008_0 =
_rtB -> B_120_251_0_p * _rtB -> B_120_1007_0 ; if ( ( _rtP -> P_1053 != 0 )
&& ( ! ( _rtB -> B_91_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_1053 == 1 ) { if (
_rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_jy == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_jy < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_jy ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_246_0_l ) ; if ( ( _rtP -> P_1053 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_1053 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_jy == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_jy < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_jy ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_246_0_l ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_246_0_l ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_1009_0 =
_rtB -> B_120_244_0_o [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_91_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_jj . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_jj .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_615 ; _rtB -> B_120_1010_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_l0 . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_l0 . Last , _rtDW -> TransportDelay_IWORK_l0 . Tail ,
_rtDW -> TransportDelay_IWORK_l0 . Head , _rtP -> P_616 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_120_1011_0 = 0.0 ; _rtB -> B_120_1011_0 += _rtP -> P_618 *
_rtX -> TransferFcn_CSTATE_aq ; _rtB -> B_120_1011_0 += _rtP -> P_619 * _rtB
-> B_120_1010_0 ; if ( _rtB -> B_120_258_0 ) { if ( _rtB -> B_120_1011_0 >
_rtP -> P_63 ) { _rtB -> B_93_0_0 = _rtB -> B_120_1011_0 - _rtP -> P_63 ; }
else if ( _rtB -> B_120_1011_0 >= _rtP -> P_62 ) { _rtB -> B_93_0_0 = 0.0 ; }
else { _rtB -> B_93_0_0 = _rtB -> B_120_1011_0 - _rtP -> P_62 ; } _rtB ->
B_120_1013_0 = _rtB -> B_93_0_0 ; } else { _rtB -> B_120_1013_0 = _rtB ->
B_120_257_0_d ; } _rtB -> B_120_1014_0 = _rtP -> P_620 * _rtB -> B_120_1013_0
; if ( _rtB -> B_120_1014_0 > _rtP -> P_621 ) { _rtB -> B_120_1015_0 = _rtP
-> P_621 ; } else if ( _rtB -> B_120_1014_0 < _rtP -> P_622 ) { _rtB ->
B_120_1015_0 = _rtP -> P_622 ; } else { _rtB -> B_120_1015_0 = _rtB ->
B_120_1014_0 ; } if ( ( _rtDW -> LastMajorTimeA_cl >= ssGetTaskTime ( S , 0 )
) && ( _rtDW -> LastMajorTimeB_l3 >= ssGetTaskTime ( S , 0 ) ) ) { _rtB ->
B_120_1016_0 = _rtB -> B_120_1015_0 ; } else { if ( ( ( _rtDW ->
LastMajorTimeA_cl < _rtDW -> LastMajorTimeB_l3 ) && ( _rtDW ->
LastMajorTimeB_l3 < ssGetTaskTime ( S , 0 ) ) ) || ( ( _rtDW ->
LastMajorTimeA_cl >= _rtDW -> LastMajorTimeB_l3 ) && ( _rtDW ->
LastMajorTimeA_cl >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime (
S , 0 ) - _rtDW -> LastMajorTimeB_l3 ; rtb_B_120_5_0 = _rtDW -> PrevYB_e ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_cl ;
rtb_B_120_5_0 = _rtDW -> PrevYA_jv ; } riseValLimit = deltaT * _rtP -> P_623
; rateLimiterRate = _rtB -> B_120_1015_0 - rtb_B_120_5_0 ; if (
rateLimiterRate > riseValLimit ) { _rtB -> B_120_1016_0 = rtb_B_120_5_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_624 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_120_1016_0 = rtb_B_120_5_0 + deltaT ; } else { _rtB ->
B_120_1016_0 = _rtB -> B_120_1015_0 ; } } } isHit = ssIsSampleHit ( S , 2 , 0
) ; if ( isHit != 0 ) { _rtB -> B_120_1017_0 = _rtP -> P_625 * _rtB ->
B_120_1007_0 ; } _rtB -> B_120_1018_0 = _rtB -> B_120_1016_0 * _rtB ->
B_120_1017_0 ; _rtB -> B_120_1019_0 = _rtB -> B_120_1009_0 * _rtB ->
B_120_1018_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model1/Data Store Read" ,
0 ) ; _rtB -> B_120_1020_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_n1 != 0
) { _rtX -> vv0at_CSTATE_mn = _rtB -> B_120_1020_0 ; } _rtB -> B_120_1021_0 =
_rtX -> vv0at_CSTATE_mn ; _rtB -> B_120_1022_0 = muDoubleScalarAbs ( _rtB ->
B_120_1021_0 ) ; _rtB -> B_120_1023_0 = _rtP -> P_626 [ 0 ] ; _rtB ->
B_120_1023_0 = _rtB -> B_120_1023_0 * _rtB -> B_120_1022_0 + _rtP -> P_626 [
1 ] ; _rtB -> B_120_1023_0 = _rtB -> B_120_1023_0 * _rtB -> B_120_1022_0 +
_rtP -> P_626 [ 2 ] ; _rtB -> B_120_1024_0 = _rtP -> P_627 * _rtB ->
B_120_1023_0 ; _rtB -> B_120_1025_0 = _rtB -> B_120_1007_0 * _rtB ->
B_120_1024_0 ; _rtB -> B_120_1026_0 = _rtB -> B_120_1019_0 - _rtB ->
B_120_1025_0 ; isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB
-> B_120_1028_0 = _rtP -> P_628 * _rtB -> B_120_1008_0 ; } if ( _rtB ->
B_120_1026_0 > _rtB -> B_120_1008_0 ) { _rtB -> B_120_1030_0 = _rtB ->
B_120_1008_0 ; } else { if ( _rtB -> B_120_1026_0 < _rtB -> B_120_1028_0 ) {
_rtB -> B_92_1_0 = _rtB -> B_120_1028_0 ; } else { _rtB -> B_92_1_0 = _rtB ->
B_120_1026_0 ; } _rtB -> B_120_1030_0 = _rtB -> B_92_1_0 ; } if ( _rtB ->
B_120_250_0_k ) { _rtB -> B_120_1031_0 = _rtB -> B_120_1030_0 ; } else { _rtB
-> B_120_1031_0 = _rtB -> B_120_249_0 ; } isHit = ssIsSampleHit ( S , 2 , 0 )
; if ( isHit != 0 ) { ssCallAccelRunBlock ( S , 100 , 0 , SS_CALL_MDL_OUTPUTS
) ; if ( ( _rtP -> P_1054 != 0 ) && ( ! ( _rtB -> B_100_0_0 <= 499.0 ) ) ) {
if ( _rtP -> P_1054 == 1 ) { if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_ly == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model4/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_ly < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_ly ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model4/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_269_0_c ) ; if ( ( _rtP -> P_1054 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_1054 == 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_ly == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model4/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_ly < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_ly ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model4/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_269_0_c ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_269_0_c ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_1033_0 =
_rtB -> B_120_268_0_m [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_100_0_0 , 499U ) ] ; _rtB -> B_120_1034_0 =
_rtB -> B_120_275_0 * _rtB -> B_120_1033_0 ; if ( ( _rtP -> P_1055 != 0 ) &&
( ! ( _rtB -> B_100_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_1055 == 1 ) { if (
_rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_g5 == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_g5 < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_g5 ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_120_5_0 = muDoubleScalarFloor
( _rtB -> B_120_270_0_f ) ; if ( ( _rtP -> P_1055 != 0 ) && ( ( ! (
rtb_B_120_5_0 >= 0.0 ) ) || ( ! ( rtb_B_120_5_0 <= 2.0 ) ) ) ) { if ( _rtP ->
P_1055 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_g5 == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_g5 < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_g5 ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_120_270_0_f ) ) { rtb_B_120_5_0 = 0.0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_270_0_f ; } if ( rtb_B_120_5_0 > 2.0 ) { rtb_B_120_5_0 = 2.0 ; } else {
if ( rtb_B_120_5_0 < 0.0 ) { rtb_B_120_5_0 = 0.0 ; } } _rtB -> B_120_1035_0 =
_rtB -> B_120_268_0_m [ ( int32_T ) rtb_B_120_5_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_100_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_e . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_e .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_629 ; _rtB -> B_120_1036_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_pc . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_pc . Last , _rtDW -> TransportDelay_IWORK_pc . Tail ,
_rtDW -> TransportDelay_IWORK_pc . Head , _rtP -> P_630 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_120_1037_0 = 0.0 ; _rtB -> B_120_1037_0 += _rtP -> P_632 *
_rtX -> TransferFcn_CSTATE_pj ; _rtB -> B_120_1037_0 += _rtP -> P_633 * _rtB
-> B_120_1036_0 ; if ( _rtB -> B_120_250_0_k ) { if ( _rtB -> B_120_1037_0 >
_rtP -> P_61 ) { _rtB -> B_90_0_0 = _rtB -> B_120_1037_0 - _rtP -> P_61 ; }
else if ( _rtB -> B_120_1037_0 >= _rtP -> P_60 ) { _rtB -> B_90_0_0 = 0.0 ; }
else { _rtB -> B_90_0_0 = _rtB -> B_120_1037_0 - _rtP -> P_60 ; } _rtB ->
B_120_1039_0 = _rtB -> B_90_0_0 ; } else { _rtB -> B_120_1039_0 = _rtB ->
B_120_249_0 ; } _rtB -> B_120_1040_0 = _rtP -> P_634 * _rtB -> B_120_1039_0 ;
if ( _rtB -> B_120_1040_0 > _rtP -> P_635 ) { _rtB -> B_120_1041_0 = _rtP ->
P_635 ; } else if ( _rtB -> B_120_1040_0 < _rtP -> P_636 ) { _rtB ->
B_120_1041_0 = _rtP -> P_636 ; } else { _rtB -> B_120_1041_0 = _rtB ->
B_120_1040_0 ; } if ( ( _rtDW -> LastMajorTimeA_nb >= ssGetTaskTime ( S , 0 )
) && ( _rtDW -> LastMajorTimeB_o2 >= ssGetTaskTime ( S , 0 ) ) ) { _rtB ->
B_120_1042_0 = _rtB -> B_120_1041_0 ; } else { if ( ( ( _rtDW ->
LastMajorTimeA_nb < _rtDW -> LastMajorTimeB_o2 ) && ( _rtDW ->
LastMajorTimeB_o2 < ssGetTaskTime ( S , 0 ) ) ) || ( ( _rtDW ->
LastMajorTimeA_nb >= _rtDW -> LastMajorTimeB_o2 ) && ( _rtDW ->
LastMajorTimeA_nb >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime (
S , 0 ) - _rtDW -> LastMajorTimeB_o2 ; rtb_B_120_5_0 = _rtDW -> PrevYB_hs ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_nb ;
rtb_B_120_5_0 = _rtDW -> PrevYA_h1 ; } riseValLimit = deltaT * _rtP -> P_637
; rateLimiterRate = _rtB -> B_120_1041_0 - rtb_B_120_5_0 ; if (
rateLimiterRate > riseValLimit ) { _rtB -> B_120_1042_0 = rtb_B_120_5_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_638 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_120_1042_0 = rtb_B_120_5_0 + deltaT ; } else { _rtB ->
B_120_1042_0 = _rtB -> B_120_1041_0 ; } } } isHit = ssIsSampleHit ( S , 2 , 0
) ; if ( isHit != 0 ) { _rtB -> B_120_1043_0 = _rtP -> P_639 * _rtB ->
B_120_1033_0 ; } _rtB -> B_120_1044_0 = _rtB -> B_120_1042_0 * _rtB ->
B_120_1043_0 ; _rtB -> B_120_1045_0 = _rtB -> B_120_1035_0 * _rtB ->
B_120_1044_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model4/Data Store Read" ,
0 ) ; _rtB -> B_120_1046_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_j != 0 )
{ _rtX -> vv0at_CSTATE_lr = _rtB -> B_120_1046_0 ; } _rtB -> B_120_1047_0 =
_rtX -> vv0at_CSTATE_lr ; _rtB -> B_120_1048_0 = muDoubleScalarAbs ( _rtB ->
B_120_1047_0 ) ; _rtB -> B_120_1049_0 = _rtP -> P_640 [ 0 ] ; _rtB ->
B_120_1049_0 = _rtB -> B_120_1049_0 * _rtB -> B_120_1048_0 + _rtP -> P_640 [
1 ] ; _rtB -> B_120_1049_0 = _rtB -> B_120_1049_0 * _rtB -> B_120_1048_0 +
_rtP -> P_640 [ 2 ] ; _rtB -> B_120_1050_0 = _rtP -> P_641 * _rtB ->
B_120_1049_0 ; _rtB -> B_120_1051_0 = _rtB -> B_120_1033_0 * _rtB ->
B_120_1050_0 ; _rtB -> B_120_1052_0 = _rtB -> B_120_1045_0 - _rtB ->
B_120_1051_0 ; isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB
-> B_120_1054_0 = _rtP -> P_642 * _rtB -> B_120_1034_0 ; } if ( _rtB ->
B_120_1052_0 > _rtB -> B_120_1034_0 ) { _rtB -> B_120_1056_0 = _rtB ->
B_120_1034_0 ; } else { if ( _rtB -> B_120_1052_0 < _rtB -> B_120_1054_0 ) {
_rtB -> B_101_1_0 = _rtB -> B_120_1054_0 ; } else { _rtB -> B_101_1_0 = _rtB
-> B_120_1052_0 ; } _rtB -> B_120_1056_0 = _rtB -> B_101_1_0 ; } if ( _rtB ->
B_120_274_0_f ) { _rtB -> B_120_1057_0 = _rtB -> B_120_1056_0 ; } else { _rtB
-> B_120_1057_0 = _rtB -> B_120_273_0 ; } _rtB -> B_120_1058_0 [ 0 ] = _rtB
-> B_120_40_0 ; _rtB -> B_120_1058_0 [ 1 ] = _rtB -> B_120_66_0 ; _rtB ->
B_120_1058_0 [ 2 ] = _rtB -> B_120_92_0 ; _rtB -> B_120_1058_0 [ 3 ] = _rtB
-> B_120_118_0 ; _rtB -> B_120_1058_0 [ 4 ] = _rtB -> B_120_144_0 ; _rtB ->
B_120_1058_0 [ 5 ] = _rtB -> B_120_173_0 ; _rtB -> B_120_1058_0 [ 6 ] = _rtB
-> B_120_199_0 ; _rtB -> B_120_1058_0 [ 7 ] = _rtB -> B_120_225_0 ; _rtB ->
B_120_1058_0 [ 8 ] = _rtB -> B_120_251_0 ; _rtB -> B_120_1058_0 [ 9 ] = _rtB
-> B_120_277_0 ; _rtB -> B_120_1058_0 [ 10 ] = _rtB -> B_120_303_0 ; _rtB ->
B_120_1058_0 [ 11 ] = _rtB -> B_120_329_0 ; _rtB -> B_120_1058_0 [ 12 ] =
_rtB -> B_120_355_0 ; _rtB -> B_120_1058_0 [ 13 ] = _rtB -> B_120_381_0 ;
_rtB -> B_120_1058_0 [ 14 ] = _rtB -> B_120_407_0 ; _rtB -> B_120_1058_0 [ 15
] = _rtB -> B_120_433_0 ; _rtB -> B_120_1058_0 [ 16 ] = _rtB -> B_120_459_0 ;
_rtB -> B_120_1058_0 [ 17 ] = _rtB -> B_120_485_0 ; _rtB -> B_120_1058_0 [ 18
] = _rtB -> B_120_511_0 ; _rtB -> B_120_1058_0 [ 19 ] = _rtB -> B_120_537_0 ;
_rtB -> B_120_1058_0 [ 20 ] = _rtB -> B_120_563_0 ; _rtB -> B_120_1058_0 [ 21
] = _rtB -> B_120_589_0 ; _rtB -> B_120_1058_0 [ 22 ] = _rtB -> B_120_615_0 ;
_rtB -> B_120_1058_0 [ 23 ] = _rtB -> B_120_641_0 ; _rtB -> B_120_1058_0 [ 24
] = _rtB -> B_120_667_0 ; _rtB -> B_120_1058_0 [ 25 ] = _rtB -> B_120_693_0 ;
_rtB -> B_120_1058_0 [ 26 ] = _rtB -> B_120_719_0 ; _rtB -> B_120_1058_0 [ 27
] = _rtB -> B_120_745_0 ; _rtB -> B_120_1058_0 [ 28 ] = _rtB -> B_120_771_0 ;
_rtB -> B_120_1058_0 [ 29 ] = _rtB -> B_120_797_0 ; _rtB -> B_120_1058_0 [ 30
] = _rtB -> B_120_823_0 ; _rtB -> B_120_1058_0 [ 31 ] = _rtB -> B_120_849_0 ;
_rtB -> B_120_1058_0 [ 32 ] = _rtB -> B_120_875_0 ; _rtB -> B_120_1058_0 [ 33
] = _rtB -> B_120_901_0 ; _rtB -> B_120_1058_0 [ 34 ] = _rtB -> B_120_927_0 ;
_rtB -> B_120_1058_0 [ 35 ] = _rtB -> B_120_953_0 ; _rtB -> B_120_1058_0 [ 36
] = _rtB -> B_120_979_0 ; _rtB -> B_120_1058_0 [ 37 ] = _rtB -> B_120_1005_0
; _rtB -> B_120_1058_0 [ 38 ] = _rtB -> B_120_1031_0 ; _rtB -> B_120_1058_0 [
39 ] = _rtB -> B_120_1057_0 ; ssCallAccelRunBlock ( S , 120 , 1059 ,
SS_CALL_MDL_OUTPUTS ) ; { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_ev . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T
* * ) & _rtDW -> TransportDelay_PWORK_ev . TUbufferPtrs [ 1 ] ; real_T
simTime = ssGetT ( S ) ; real_T tMinusDelay = simTime - _rtP -> P_643 ; _rtB
-> B_120_1060_0 = Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0
, * tBuffer , * uBuffer , _rtDW -> TransportDelay_IWORK_n . CircularBufSize ,
& _rtDW -> TransportDelay_IWORK_n . Last , _rtDW -> TransportDelay_IWORK_n .
Tail , _rtDW -> TransportDelay_IWORK_n . Head , _rtP -> P_644 , 0 , (
boolean_T ) ( ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) ==
ssGetT ( S ) ) ) ) ; } _rtB -> B_120_1061_0 = 0.0 ; _rtB -> B_120_1061_0 +=
_rtP -> P_646 * _rtX -> TransferFcn_CSTATE_bm ; _rtB -> B_120_1061_0 += _rtP
-> P_647 * _rtB -> B_120_1060_0 ; _rtB -> B_120_1058_0 [ 0 ] = _rtB ->
B_120_48_0 ; _rtB -> B_120_1058_0 [ 1 ] = _rtB -> B_120_74_0 ; _rtB ->
B_120_1058_0 [ 2 ] = _rtB -> B_120_100_0 ; _rtB -> B_120_1058_0 [ 3 ] = _rtB
-> B_120_126_0 ; _rtB -> B_120_1058_0 [ 4 ] = _rtB -> B_120_152_0 ; _rtB ->
B_120_1058_0 [ 5 ] = _rtB -> B_120_181_0 ; _rtB -> B_120_1058_0 [ 6 ] = _rtB
-> B_120_207_0 ; _rtB -> B_120_1058_0 [ 7 ] = _rtB -> B_120_233_0 ; _rtB ->
B_120_1058_0 [ 8 ] = _rtB -> B_120_259_0 ; _rtB -> B_120_1058_0 [ 9 ] = _rtB
-> B_120_285_0 ; _rtB -> B_120_1058_0 [ 10 ] = _rtB -> B_120_311_0 ; _rtB ->
B_120_1058_0 [ 11 ] = _rtB -> B_120_337_0 ; _rtB -> B_120_1058_0 [ 12 ] =
_rtB -> B_120_363_0 ; _rtB -> B_120_1058_0 [ 13 ] = _rtB -> B_120_389_0 ;
_rtB -> B_120_1058_0 [ 14 ] = _rtB -> B_120_415_0 ; _rtB -> B_120_1058_0 [ 15
] = _rtB -> B_120_441_0 ; _rtB -> B_120_1058_0 [ 16 ] = _rtB -> B_120_467_0 ;
_rtB -> B_120_1058_0 [ 17 ] = _rtB -> B_120_493_0 ; _rtB -> B_120_1058_0 [ 18
] = _rtB -> B_120_519_0 ; _rtB -> B_120_1058_0 [ 19 ] = _rtB -> B_120_545_0 ;
_rtB -> B_120_1058_0 [ 20 ] = _rtB -> B_120_571_0 ; _rtB -> B_120_1058_0 [ 21
] = _rtB -> B_120_597_0 ; _rtB -> B_120_1058_0 [ 22 ] = _rtB -> B_120_623_0 ;
_rtB -> B_120_1058_0 [ 23 ] = _rtB -> B_120_649_0 ; _rtB -> B_120_1058_0 [ 24
] = _rtB -> B_120_675_0 ; _rtB -> B_120_1058_0 [ 25 ] = _rtB -> B_120_701_0 ;
_rtB -> B_120_1058_0 [ 26 ] = _rtB -> B_120_727_0 ; _rtB -> B_120_1058_0 [ 27
] = _rtB -> B_120_753_0 ; _rtB -> B_120_1058_0 [ 28 ] = _rtB -> B_120_779_0 ;
_rtB -> B_120_1058_0 [ 29 ] = _rtB -> B_120_805_0 ; _rtB -> B_120_1058_0 [ 30
] = _rtB -> B_120_831_0 ; _rtB -> B_120_1058_0 [ 31 ] = _rtB -> B_120_857_0 ;
_rtB -> B_120_1058_0 [ 32 ] = _rtB -> B_120_883_0 ; _rtB -> B_120_1058_0 [ 33
] = _rtB -> B_120_909_0 ; _rtB -> B_120_1058_0 [ 34 ] = _rtB -> B_120_935_0 ;
_rtB -> B_120_1058_0 [ 35 ] = _rtB -> B_120_961_0 ; _rtB -> B_120_1058_0 [ 36
] = _rtB -> B_120_987_0 ; _rtB -> B_120_1058_0 [ 37 ] = _rtB -> B_120_1013_0
; _rtB -> B_120_1058_0 [ 38 ] = _rtB -> B_120_1039_0 ; if ( _rtB ->
B_120_274_0_f ) { if ( _rtB -> B_120_1061_0 > _rtP -> P_67 ) { _rtB ->
B_120_1058_0 [ 39 ] = _rtB -> B_120_1061_0 - _rtP -> P_67 ; } else if ( _rtB
-> B_120_1061_0 >= _rtP -> P_66 ) { _rtB -> B_120_1058_0 [ 39 ] = 0.0 ; }
else { _rtB -> B_120_1058_0 [ 39 ] = _rtB -> B_120_1061_0 - _rtP -> P_66 ; }
} else { _rtB -> B_120_1058_0 [ 39 ] = _rtB -> B_120_273_0 ; }
ssCallAccelRunBlock ( S , 120 , 1065 , SS_CALL_MDL_OUTPUTS ) ; _rtB ->
B_120_1066_0 = _rtB -> B_120_39_0 / _rtB -> B_120_20_0 ; _rtB -> B_120_1067_0
= _rtB -> B_120_1066_0 + _rtP -> P_648 ; _rtB -> B_120_1069_0 = _rtB ->
B_120_65_0 / _rtB -> B_120_42_0 ; _rtB -> B_120_1070_0 = _rtB -> B_120_1069_0
+ _rtP -> P_649 ; _rtB -> B_120_1072_0 = _rtB -> B_120_91_0 / _rtB ->
B_120_68_0 ; _rtB -> B_120_1073_0 = _rtB -> B_120_1072_0 + _rtP -> P_650 ;
_rtB -> B_120_1075_0 = _rtB -> B_120_117_0 / _rtB -> B_120_94_0 ; _rtB ->
B_120_1076_0 = _rtB -> B_120_1075_0 + _rtP -> P_651 ; _rtB -> B_120_1078_0 =
_rtB -> B_120_143_0 / _rtB -> B_120_120_0 ; _rtB -> B_120_1079_0 = _rtB ->
B_120_1078_0 + _rtP -> P_652 ; _rtB -> B_120_1081_0 = _rtB -> B_120_172_0 /
_rtB -> B_120_146_0 ; _rtB -> B_120_1083_0 = _rtB -> B_120_198_0 / _rtB ->
B_120_175_0 ; _rtB -> B_120_1084_0 = _rtB -> B_120_1083_0 + _rtP -> P_653 ;
_rtB -> B_120_1086_0 = _rtB -> B_120_224_0 / _rtB -> B_120_201_0 ; _rtB ->
B_120_1087_0 = _rtB -> B_120_1086_0 + _rtP -> P_654 ; _rtB -> B_120_1089_0 =
_rtB -> B_120_250_0 / _rtB -> B_120_227_0 ; _rtB -> B_120_1090_0 = _rtB ->
B_120_1089_0 + _rtP -> P_655 ; _rtB -> B_120_1092_0 = _rtB -> B_120_276_0 /
_rtB -> B_120_253_0 ; _rtB -> B_120_1093_0 = _rtB -> B_120_1092_0 + _rtP ->
P_656 ; _rtB -> B_120_1095_0 = _rtB -> B_120_302_0 / _rtB -> B_120_279_0 ;
_rtB -> B_120_1096_0 = _rtB -> B_120_1095_0 + _rtP -> P_657 ; _rtB ->
B_120_1098_0 = _rtB -> B_120_328_0 / _rtB -> B_120_305_0 ; _rtB ->
B_120_1099_0 = _rtB -> B_120_1098_0 + _rtP -> P_658 ; _rtB -> B_120_1101_0 =
_rtB -> B_120_354_0 / _rtB -> B_120_331_0 ; _rtB -> B_120_1102_0 = _rtB ->
B_120_1101_0 + _rtP -> P_659 ; _rtB -> B_120_1104_0 = _rtB -> B_120_380_0 /
_rtB -> B_120_357_0 ; _rtB -> B_120_1105_0 = _rtB -> B_120_1104_0 + _rtP ->
P_660 ; _rtB -> B_120_1107_0 = _rtB -> B_120_406_0 / _rtB -> B_120_383_0 ;
_rtB -> B_120_1108_0 = _rtB -> B_120_1107_0 + _rtP -> P_661 ; _rtB ->
B_120_1110_0 = _rtB -> B_120_432_0 / _rtB -> B_120_409_0 ; _rtB ->
B_120_1111_0 = _rtB -> B_120_1110_0 + _rtP -> P_662 ; _rtB -> B_120_1113_0 =
_rtB -> B_120_458_0 / _rtB -> B_120_435_0 ; _rtB -> B_120_1114_0 = _rtB ->
B_120_1113_0 + _rtP -> P_663 ; _rtB -> B_120_1116_0 = _rtB -> B_120_484_0 /
_rtB -> B_120_461_0 ; _rtB -> B_120_1117_0 = _rtB -> B_120_1116_0 + _rtP ->
P_664 ; _rtB -> B_120_1119_0 = _rtB -> B_120_510_0 / _rtB -> B_120_487_0 ;
_rtB -> B_120_1120_0 = _rtB -> B_120_1119_0 + _rtP -> P_665 ; _rtB ->
B_120_1122_0 = _rtB -> B_120_536_0 / _rtB -> B_120_513_0 ; _rtB ->
B_120_1123_0 = _rtB -> B_120_1122_0 + _rtP -> P_666 ; _rtB -> B_120_1125_0 =
_rtB -> B_120_562_0 / _rtB -> B_120_539_0 ; _rtB -> B_120_1126_0 = _rtB ->
B_120_1125_0 + _rtP -> P_667 ; _rtB -> B_120_1128_0 = _rtB -> B_120_588_0 /
_rtB -> B_120_565_0 ; _rtB -> B_120_1129_0 = _rtB -> B_120_1128_0 + _rtP ->
P_668 ; _rtB -> B_120_1131_0 = _rtB -> B_120_614_0 / _rtB -> B_120_591_0 ;
_rtB -> B_120_1132_0 = _rtB -> B_120_1131_0 + _rtP -> P_669 ; _rtB ->
B_120_1134_0 = _rtB -> B_120_640_0 / _rtB -> B_120_617_0 ; _rtB ->
B_120_1135_0 = _rtB -> B_120_1134_0 + _rtP -> P_670 ; _rtB -> B_120_1137_0 =
_rtB -> B_120_666_0 / _rtB -> B_120_643_0 ; _rtB -> B_120_1138_0 = _rtB ->
B_120_1137_0 + _rtP -> P_671 ; _rtB -> B_120_1140_0 = _rtB -> B_120_692_0 /
_rtB -> B_120_669_0 ; _rtB -> B_120_1141_0 = _rtB -> B_120_1140_0 + _rtP ->
P_672 ; _rtB -> B_120_1143_0 = _rtB -> B_120_718_0 / _rtB -> B_120_695_0 ;
_rtB -> B_120_1144_0 = _rtB -> B_120_1143_0 + _rtP -> P_673 ; _rtB ->
B_120_1146_0 = _rtB -> B_120_744_0 / _rtB -> B_120_721_0 ; _rtB ->
B_120_1147_0 = _rtB -> B_120_1146_0 + _rtP -> P_674 ; _rtB -> B_120_1149_0 =
_rtB -> B_120_770_0 / _rtB -> B_120_747_0 ; _rtB -> B_120_1150_0 = _rtB ->
B_120_1149_0 + _rtP -> P_675 ; _rtB -> B_120_1152_0 = _rtB -> B_120_796_0 /
_rtB -> B_120_773_0 ; _rtB -> B_120_1153_0 = _rtB -> B_120_1152_0 + _rtP ->
P_676 ; _rtB -> B_120_1155_0 = _rtB -> B_120_822_0 / _rtB -> B_120_799_0 ;
_rtB -> B_120_1156_0 = _rtB -> B_120_1155_0 + _rtP -> P_677 ; _rtB ->
B_120_1158_0 = _rtB -> B_120_848_0 / _rtB -> B_120_825_0 ; _rtB ->
B_120_1159_0 = _rtB -> B_120_1158_0 + _rtP -> P_678 ; _rtB -> B_120_1161_0 =
_rtB -> B_120_874_0 / _rtB -> B_120_851_0 ; _rtB -> B_120_1162_0 = _rtB ->
B_120_1161_0 + _rtP -> P_679 ; _rtB -> B_120_1164_0 = _rtB -> B_120_900_0 /
_rtB -> B_120_877_0 ; _rtB -> B_120_1165_0 = _rtB -> B_120_1164_0 + _rtP ->
P_680 ; _rtB -> B_120_1167_0 = _rtB -> B_120_926_0 / _rtB -> B_120_903_0 ;
_rtB -> B_120_1168_0 = _rtB -> B_120_1167_0 + _rtP -> P_681 ; _rtB ->
B_120_1170_0 = _rtB -> B_120_952_0 / _rtB -> B_120_929_0 ; _rtB ->
B_120_1171_0 = _rtB -> B_120_1170_0 + _rtP -> P_682 ; _rtB -> B_120_1173_0 =
_rtB -> B_120_978_0 / _rtB -> B_120_955_0 ; _rtB -> B_120_1174_0 = _rtB ->
B_120_1173_0 + _rtP -> P_683 ; _rtB -> B_120_1176_0 = _rtB -> B_120_1004_0 /
_rtB -> B_120_981_0 ; _rtB -> B_120_1177_0 = _rtB -> B_120_1176_0 + _rtP ->
P_684 ; _rtB -> B_120_1179_0 = _rtB -> B_120_1030_0 / _rtB -> B_120_1007_0 ;
_rtB -> B_120_1180_0 = _rtB -> B_120_1179_0 + _rtP -> P_685 ; _rtB ->
B_120_1182_0 = _rtB -> B_120_1056_0 / _rtB -> B_120_1033_0 ; _rtB ->
B_120_1183_0 = _rtB -> B_120_1182_0 + _rtP -> P_686 ; if ( _rtB ->
B_120_40_0_g ) { _rtB -> B_120_1058_0 [ 0 ] = _rtB -> B_120_1067_0 ; } else {
_rtB -> B_120_1058_0 [ 0 ] = _rtB -> B_120_39_0_f ; } if ( _rtB ->
B_120_24_0_d ) { _rtB -> B_120_1058_0 [ 1 ] = _rtB -> B_120_1070_0 ; } else {
_rtB -> B_120_1058_0 [ 1 ] = _rtB -> B_120_23_0_n ; } if ( _rtB ->
B_120_16_0_e ) { _rtB -> B_120_1058_0 [ 2 ] = _rtB -> B_120_1073_0 ; } else {
_rtB -> B_120_1058_0 [ 2 ] = _rtB -> B_120_15_0_d ; } if ( _rtB -> B_120_8_0
) { _rtB -> B_120_1058_0 [ 3 ] = _rtB -> B_120_1076_0 ; } else { _rtB ->
B_120_1058_0 [ 3 ] = _rtB -> B_120_7_0 ; } if ( _rtB -> B_120_32_0_i ) { _rtB
-> B_120_1058_0 [ 4 ] = _rtB -> B_120_1079_0 ; } else { _rtB -> B_120_1058_0
[ 4 ] = _rtB -> B_120_31_0_c ; } if ( _rtB -> B_120_81_0_n ) { _rtB ->
B_120_1058_0 [ 5 ] = _rtB -> B_120_1081_0 ; } else { _rtB -> B_120_1058_0 [ 5
] = _rtB -> B_120_79_0_k ; } if ( _rtB -> B_120_64_0 ) { _rtB -> B_120_1058_0
[ 6 ] = _rtB -> B_120_1084_0 ; } else { _rtB -> B_120_1058_0 [ 6 ] = _rtB ->
B_120_63_0_p ; } if ( _rtB -> B_120_56_0_l ) { _rtB -> B_120_1058_0 [ 7 ] =
_rtB -> B_120_1087_0 ; } else { _rtB -> B_120_1058_0 [ 7 ] = _rtB ->
B_120_55_0_p ; } if ( _rtB -> B_120_48_0_n ) { _rtB -> B_120_1058_0 [ 8 ] =
_rtB -> B_120_1090_0 ; } else { _rtB -> B_120_1058_0 [ 8 ] = _rtB ->
B_120_47_0 ; } if ( _rtB -> B_120_72_0_c ) { _rtB -> B_120_1058_0 [ 9 ] =
_rtB -> B_120_1093_0 ; } else { _rtB -> B_120_1058_0 [ 9 ] = _rtB ->
B_120_71_0_n ; } if ( _rtB -> B_120_122_0_o ) { _rtB -> B_120_1058_0 [ 10 ] =
_rtB -> B_120_1096_0 ; } else { _rtB -> B_120_1058_0 [ 10 ] = _rtB ->
B_120_121_0_c ; } if ( _rtB -> B_120_106_0_d ) { _rtB -> B_120_1058_0 [ 11 ]
= _rtB -> B_120_1099_0 ; } else { _rtB -> B_120_1058_0 [ 11 ] = _rtB ->
B_120_105_0_g ; } if ( _rtB -> B_120_98_0_p ) { _rtB -> B_120_1058_0 [ 12 ] =
_rtB -> B_120_1102_0 ; } else { _rtB -> B_120_1058_0 [ 12 ] = _rtB ->
B_120_97_0_o ; } if ( _rtB -> B_120_90_0 ) { _rtB -> B_120_1058_0 [ 13 ] =
_rtB -> B_120_1105_0 ; } else { _rtB -> B_120_1058_0 [ 13 ] = _rtB ->
B_120_89_0_n ; } if ( _rtB -> B_120_114_0 ) { _rtB -> B_120_1058_0 [ 14 ] =
_rtB -> B_120_1108_0 ; } else { _rtB -> B_120_1058_0 [ 14 ] = _rtB ->
B_120_113_0_c ; } if ( _rtB -> B_120_162_0_c ) { _rtB -> B_120_1058_0 [ 15 ]
= _rtB -> B_120_1111_0 ; } else { _rtB -> B_120_1058_0 [ 15 ] = _rtB ->
B_120_161_0_m ; } if ( _rtB -> B_120_146_0_h ) { _rtB -> B_120_1058_0 [ 16 ]
= _rtB -> B_120_1114_0 ; } else { _rtB -> B_120_1058_0 [ 16 ] = _rtB ->
B_120_145_0 ; } if ( _rtB -> B_120_138_0_c ) { _rtB -> B_120_1058_0 [ 17 ] =
_rtB -> B_120_1117_0 ; } else { _rtB -> B_120_1058_0 [ 17 ] = _rtB ->
B_120_137_0_j ; } if ( _rtB -> B_120_130_0_j ) { _rtB -> B_120_1058_0 [ 18 ]
= _rtB -> B_120_1120_0 ; } else { _rtB -> B_120_1058_0 [ 18 ] = _rtB ->
B_120_129_0_m ; } if ( _rtB -> B_120_154_0_d ) { _rtB -> B_120_1058_0 [ 19 ]
= _rtB -> B_120_1123_0 ; } else { _rtB -> B_120_1058_0 [ 19 ] = _rtB ->
B_120_153_0_k ; } if ( _rtB -> B_120_202_0_o ) { _rtB -> B_120_1058_0 [ 20 ]
= _rtB -> B_120_1126_0 ; } else { _rtB -> B_120_1058_0 [ 20 ] = _rtB ->
B_120_201_0_c ; } if ( _rtB -> B_120_186_0_p ) { _rtB -> B_120_1058_0 [ 21 ]
= _rtB -> B_120_1129_0 ; } else { _rtB -> B_120_1058_0 [ 21 ] = _rtB ->
B_120_185_0_g ; } if ( _rtB -> B_120_178_0_p ) { _rtB -> B_120_1058_0 [ 22 ]
= _rtB -> B_120_1132_0 ; } else { _rtB -> B_120_1058_0 [ 22 ] = _rtB ->
B_120_177_0_d ; } if ( _rtB -> B_120_170_0 ) { _rtB -> B_120_1058_0 [ 23 ] =
_rtB -> B_120_1135_0 ; } else { _rtB -> B_120_1058_0 [ 23 ] = _rtB ->
B_120_169_0_p ; } if ( _rtB -> B_120_194_0_a ) { _rtB -> B_120_1058_0 [ 24 ]
= _rtB -> B_120_1138_0 ; } else { _rtB -> B_120_1058_0 [ 24 ] = _rtB ->
B_120_193_0_c ; } if ( _rtB -> B_120_242_0_l ) { _rtB -> B_120_1058_0 [ 25 ]
= _rtB -> B_120_1141_0 ; } else { _rtB -> B_120_1058_0 [ 25 ] = _rtB ->
B_120_241_0_f ; } if ( _rtB -> B_120_226_0 ) { _rtB -> B_120_1058_0 [ 26 ] =
_rtB -> B_120_1144_0 ; } else { _rtB -> B_120_1058_0 [ 26 ] = _rtB ->
B_120_225_0_g ; } if ( _rtB -> B_120_218_0_p ) { _rtB -> B_120_1058_0 [ 27 ]
= _rtB -> B_120_1147_0 ; } else { _rtB -> B_120_1058_0 [ 27 ] = _rtB ->
B_120_217_0_d ; } if ( _rtB -> B_120_210_0_j ) { _rtB -> B_120_1058_0 [ 28 ]
= _rtB -> B_120_1150_0 ; } else { _rtB -> B_120_1058_0 [ 28 ] = _rtB ->
B_120_209_0_i ; } if ( _rtB -> B_120_234_0_o ) { _rtB -> B_120_1058_0 [ 29 ]
= _rtB -> B_120_1153_0 ; } else { _rtB -> B_120_1058_0 [ 29 ] = _rtB ->
B_120_233_0_l ; } if ( _rtB -> B_120_322_0_i ) { _rtB -> B_120_1058_0 [ 30 ]
= _rtB -> B_120_1156_0 ; } else { _rtB -> B_120_1058_0 [ 30 ] = _rtB ->
B_120_321_0_b ; } if ( _rtB -> B_120_306_0_i ) { _rtB -> B_120_1058_0 [ 31 ]
= _rtB -> B_120_1159_0 ; } else { _rtB -> B_120_1058_0 [ 31 ] = _rtB ->
B_120_305_0_o ; } if ( _rtB -> B_120_298_0_n ) { _rtB -> B_120_1058_0 [ 32 ]
= _rtB -> B_120_1162_0 ; } else { _rtB -> B_120_1058_0 [ 32 ] = _rtB ->
B_120_297_0_n ; } if ( _rtB -> B_120_290_0_f ) { _rtB -> B_120_1058_0 [ 33 ]
= _rtB -> B_120_1165_0 ; } else { _rtB -> B_120_1058_0 [ 33 ] = _rtB ->
B_120_289_0_h ; } if ( _rtB -> B_120_314_0_l ) { _rtB -> B_120_1058_0 [ 34 ]
= _rtB -> B_120_1168_0 ; } else { _rtB -> B_120_1058_0 [ 34 ] = _rtB ->
B_120_313_0_c ; } if ( _rtB -> B_120_282_0_c ) { _rtB -> B_120_1058_0 [ 35 ]
= _rtB -> B_120_1171_0 ; } else { _rtB -> B_120_1058_0 [ 35 ] = _rtB ->
B_120_281_0_i ; } if ( _rtB -> B_120_266_0_j ) { _rtB -> B_120_1058_0 [ 36 ]
= _rtB -> B_120_1174_0 ; } else { _rtB -> B_120_1058_0 [ 36 ] = _rtB ->
B_120_265_0_j ; } if ( _rtB -> B_120_258_0 ) { _rtB -> B_120_1058_0 [ 37 ] =
_rtB -> B_120_1177_0 ; } else { _rtB -> B_120_1058_0 [ 37 ] = _rtB ->
B_120_257_0_d ; } if ( _rtB -> B_120_250_0_k ) { _rtB -> B_120_1058_0 [ 38 ]
= _rtB -> B_120_1180_0 ; } else { _rtB -> B_120_1058_0 [ 38 ] = _rtB ->
B_120_249_0 ; } if ( _rtB -> B_120_274_0_f ) { _rtB -> B_120_1058_0 [ 39 ] =
_rtB -> B_120_1183_0 ; } else { _rtB -> B_120_1058_0 [ 39 ] = _rtB ->
B_120_273_0 ; } ssCallAccelRunBlock ( S , 120 , 1186 , SS_CALL_MDL_OUTPUTS )
; _rtB -> B_120_1187_0 = _rtX -> Integrator1_CSTATE ; ssCallAccelRunBlock ( S
, 120 , 1188 , SS_CALL_MDL_OUTPUTS ) ; ssCallAccelRunBlock ( S , 120 , 1189 ,
SS_CALL_MDL_OUTPUTS ) ; isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0
) { if ( _rtB -> B_120_40_0_g ) { _rtB -> B_120_1230_0 [ 0 ] = _rtB ->
B_13_0_0 ; } else { _rtB -> B_120_1230_0 [ 0 ] = _rtB -> B_120_39_0_f ; } if
( _rtB -> B_120_24_0_d ) { _rtB -> B_120_1230_0 [ 1 ] = _rtB -> B_7_0_0 ; }
else { _rtB -> B_120_1230_0 [ 1 ] = _rtB -> B_120_23_0_n ; } if ( _rtB ->
B_120_16_0_e ) { _rtB -> B_120_1230_0 [ 2 ] = _rtB -> B_4_0_0 ; } else { _rtB
-> B_120_1230_0 [ 2 ] = _rtB -> B_120_15_0_d ; } if ( _rtB -> B_120_8_0 ) {
_rtB -> B_120_1230_0 [ 3 ] = _rtB -> B_1_0_0 ; } else { _rtB -> B_120_1230_0
[ 3 ] = _rtB -> B_120_7_0 ; } if ( _rtB -> B_120_32_0_i ) { _rtB ->
B_120_1230_0 [ 4 ] = _rtB -> B_10_0_0 ; } else { _rtB -> B_120_1230_0 [ 4 ] =
_rtB -> B_120_31_0_c ; } if ( _rtB -> B_120_81_0_n ) { _rtB -> B_120_1230_0 [
5 ] = _rtB -> B_28_0_0 ; } else { _rtB -> B_120_1230_0 [ 5 ] = _rtB ->
B_120_79_0_k ; } if ( _rtB -> B_120_64_0 ) { _rtB -> B_120_1230_0 [ 6 ] =
_rtB -> B_22_0_0 ; } else { _rtB -> B_120_1230_0 [ 6 ] = _rtB -> B_120_63_0_p
; } if ( _rtB -> B_120_56_0_l ) { _rtB -> B_120_1230_0 [ 7 ] = _rtB ->
B_19_0_0 ; } else { _rtB -> B_120_1230_0 [ 7 ] = _rtB -> B_120_55_0_p ; } if
( _rtB -> B_120_48_0_n ) { _rtB -> B_120_1230_0 [ 8 ] = _rtB -> B_16_0_0 ; }
else { _rtB -> B_120_1230_0 [ 8 ] = _rtB -> B_120_47_0 ; } if ( _rtB ->
B_120_72_0_c ) { _rtB -> B_120_1230_0 [ 9 ] = _rtB -> B_25_0_0 ; } else {
_rtB -> B_120_1230_0 [ 9 ] = _rtB -> B_120_71_0_n ; } if ( _rtB ->
B_120_122_0_o ) { _rtB -> B_120_1230_0 [ 10 ] = _rtB -> B_43_0_0 ; } else {
_rtB -> B_120_1230_0 [ 10 ] = _rtB -> B_120_121_0_c ; } if ( _rtB ->
B_120_106_0_d ) { _rtB -> B_120_1230_0 [ 11 ] = _rtB -> B_37_0_0 ; } else {
_rtB -> B_120_1230_0 [ 11 ] = _rtB -> B_120_105_0_g ; } if ( _rtB ->
B_120_98_0_p ) { _rtB -> B_120_1230_0 [ 12 ] = _rtB -> B_34_0_0 ; } else {
_rtB -> B_120_1230_0 [ 12 ] = _rtB -> B_120_97_0_o ; } if ( _rtB ->
B_120_90_0 ) { _rtB -> B_120_1230_0 [ 13 ] = _rtB -> B_31_0_0 ; } else { _rtB
-> B_120_1230_0 [ 13 ] = _rtB -> B_120_89_0_n ; } if ( _rtB -> B_120_114_0 )
{ _rtB -> B_120_1230_0 [ 14 ] = _rtB -> B_40_0_0 ; } else { _rtB ->
B_120_1230_0 [ 14 ] = _rtB -> B_120_113_0_c ; } if ( _rtB -> B_120_162_0_c )
{ _rtB -> B_120_1230_0 [ 15 ] = _rtB -> B_58_0_0 ; } else { _rtB ->
B_120_1230_0 [ 15 ] = _rtB -> B_120_161_0_m ; } if ( _rtB -> B_120_146_0_h )
{ _rtB -> B_120_1230_0 [ 16 ] = _rtB -> B_52_0_0 ; } else { _rtB ->
B_120_1230_0 [ 16 ] = _rtB -> B_120_145_0 ; } if ( _rtB -> B_120_138_0_c ) {
_rtB -> B_120_1230_0 [ 17 ] = _rtB -> B_49_0_0 ; } else { _rtB ->
B_120_1230_0 [ 17 ] = _rtB -> B_120_137_0_j ; } if ( _rtB -> B_120_130_0_j )
{ _rtB -> B_120_1230_0 [ 18 ] = _rtB -> B_46_0_0 ; } else { _rtB ->
B_120_1230_0 [ 18 ] = _rtB -> B_120_129_0_m ; } if ( _rtB -> B_120_154_0_d )
{ _rtB -> B_120_1230_0 [ 19 ] = _rtB -> B_55_0_0 ; } else { _rtB ->
B_120_1230_0 [ 19 ] = _rtB -> B_120_153_0_k ; } if ( _rtB -> B_120_202_0_o )
{ _rtB -> B_120_1230_0 [ 20 ] = _rtB -> B_73_0_0 ; } else { _rtB ->
B_120_1230_0 [ 20 ] = _rtB -> B_120_201_0_c ; } if ( _rtB -> B_120_186_0_p )
{ _rtB -> B_120_1230_0 [ 21 ] = _rtB -> B_67_0_0 ; } else { _rtB ->
B_120_1230_0 [ 21 ] = _rtB -> B_120_185_0_g ; } if ( _rtB -> B_120_178_0_p )
{ _rtB -> B_120_1230_0 [ 22 ] = _rtB -> B_64_0_0 ; } else { _rtB ->
B_120_1230_0 [ 22 ] = _rtB -> B_120_177_0_d ; } if ( _rtB -> B_120_170_0 ) {
_rtB -> B_120_1230_0 [ 23 ] = _rtB -> B_61_0_0 ; } else { _rtB ->
B_120_1230_0 [ 23 ] = _rtB -> B_120_169_0_p ; } if ( _rtB -> B_120_194_0_a )
{ _rtB -> B_120_1230_0 [ 24 ] = _rtB -> B_70_0_0 ; } else { _rtB ->
B_120_1230_0 [ 24 ] = _rtB -> B_120_193_0_c ; } if ( _rtB -> B_120_242_0_l )
{ _rtB -> B_120_1230_0 [ 25 ] = _rtB -> B_88_0_0 ; } else { _rtB ->
B_120_1230_0 [ 25 ] = _rtB -> B_120_241_0_f ; } if ( _rtB -> B_120_226_0 ) {
_rtB -> B_120_1230_0 [ 26 ] = _rtB -> B_82_0_0 ; } else { _rtB ->
B_120_1230_0 [ 26 ] = _rtB -> B_120_225_0_g ; } if ( _rtB -> B_120_218_0_p )
{ _rtB -> B_120_1230_0 [ 27 ] = _rtB -> B_79_0_0 ; } else { _rtB ->
B_120_1230_0 [ 27 ] = _rtB -> B_120_217_0_d ; } if ( _rtB -> B_120_210_0_j )
{ _rtB -> B_120_1230_0 [ 28 ] = _rtB -> B_76_0_0 ; } else { _rtB ->
B_120_1230_0 [ 28 ] = _rtB -> B_120_209_0_i ; } if ( _rtB -> B_120_234_0_o )
{ _rtB -> B_120_1230_0 [ 29 ] = _rtB -> B_85_0_0 ; } else { _rtB ->
B_120_1230_0 [ 29 ] = _rtB -> B_120_233_0_l ; } if ( _rtB -> B_120_322_0_i )
{ _rtB -> B_120_1230_0 [ 30 ] = _rtB -> B_118_0_0 ; } else { _rtB ->
B_120_1230_0 [ 30 ] = _rtB -> B_120_321_0_b ; } if ( _rtB -> B_120_306_0_i )
{ _rtB -> B_120_1230_0 [ 31 ] = _rtB -> B_112_0_0 ; } else { _rtB ->
B_120_1230_0 [ 31 ] = _rtB -> B_120_305_0_o ; } if ( _rtB -> B_120_298_0_n )
{ _rtB -> B_120_1230_0 [ 32 ] = _rtB -> B_109_0_0 ; } else { _rtB ->
B_120_1230_0 [ 32 ] = _rtB -> B_120_297_0_n ; } if ( _rtB -> B_120_290_0_f )
{ _rtB -> B_120_1230_0 [ 33 ] = _rtB -> B_106_0_0 ; } else { _rtB ->
B_120_1230_0 [ 33 ] = _rtB -> B_120_289_0_h ; } if ( _rtB -> B_120_314_0_l )
{ _rtB -> B_120_1230_0 [ 34 ] = _rtB -> B_115_0_0 ; } else { _rtB ->
B_120_1230_0 [ 34 ] = _rtB -> B_120_313_0_c ; } if ( _rtB -> B_120_282_0_c )
{ _rtB -> B_120_1230_0 [ 35 ] = _rtB -> B_103_0_0 ; } else { _rtB ->
B_120_1230_0 [ 35 ] = _rtB -> B_120_281_0_i ; } if ( _rtB -> B_120_266_0_j )
{ _rtB -> B_120_1230_0 [ 36 ] = _rtB -> B_97_0_0 ; } else { _rtB ->
B_120_1230_0 [ 36 ] = _rtB -> B_120_265_0_j ; } if ( _rtB -> B_120_258_0 ) {
_rtB -> B_120_1230_0 [ 37 ] = _rtB -> B_94_0_0 ; } else { _rtB ->
B_120_1230_0 [ 37 ] = _rtB -> B_120_257_0_d ; } if ( _rtB -> B_120_250_0_k )
{ _rtB -> B_120_1230_0 [ 38 ] = _rtB -> B_91_0_0 ; } else { _rtB ->
B_120_1230_0 [ 38 ] = _rtB -> B_120_249_0 ; } if ( _rtB -> B_120_274_0_f ) {
_rtB -> B_120_1230_0 [ 39 ] = _rtB -> B_100_0_0 ; } else { _rtB ->
B_120_1230_0 [ 39 ] = _rtB -> B_120_273_0 ; } ssCallAccelRunBlock ( S , 120 ,
1231 , SS_CALL_MDL_OUTPUTS ) ; } _rtB -> B_120_1233_0 = _rtB -> B_120_1_0_c *
look1_binlxpw ( _rtB -> B_120_18_0 , _rtP -> P_689 , _rtP -> P_688 , 7U ) ;
_rtB -> B_120_1234_0 = ( ( ( _rtB -> B_120_40_0 + _rtB -> B_120_66_0 ) + _rtB
-> B_120_92_0 ) + _rtB -> B_120_118_0 ) + _rtB -> B_120_144_0 ; _rtB ->
B_120_1235_0 = ( ( ( _rtB -> B_120_173_0 + _rtB -> B_120_199_0 ) + _rtB ->
B_120_225_0 ) + _rtB -> B_120_251_0 ) + _rtB -> B_120_277_0 ; _rtB ->
B_120_1236_0 = ( ( ( _rtB -> B_120_303_0 + _rtB -> B_120_329_0 ) + _rtB ->
B_120_355_0 ) + _rtB -> B_120_381_0 ) + _rtB -> B_120_407_0 ; _rtB ->
B_120_1237_0 = ( ( ( _rtB -> B_120_433_0 + _rtB -> B_120_459_0 ) + _rtB ->
B_120_485_0 ) + _rtB -> B_120_511_0 ) + _rtB -> B_120_537_0 ; _rtB ->
B_120_1238_0 = ( ( ( _rtB -> B_120_563_0 + _rtB -> B_120_589_0 ) + _rtB ->
B_120_615_0 ) + _rtB -> B_120_641_0 ) + _rtB -> B_120_667_0 ; _rtB ->
B_120_1239_0 = ( ( ( _rtB -> B_120_693_0 + _rtB -> B_120_719_0 ) + _rtB ->
B_120_745_0 ) + _rtB -> B_120_771_0 ) + _rtB -> B_120_797_0 ; _rtB ->
B_120_1240_0 = ( ( ( _rtB -> B_120_823_0 + _rtB -> B_120_849_0 ) + _rtB ->
B_120_875_0 ) + _rtB -> B_120_901_0 ) + _rtB -> B_120_927_0 ; _rtB ->
B_120_1241_0 = ( ( ( _rtB -> B_120_953_0 + _rtB -> B_120_979_0 ) + _rtB ->
B_120_1005_0 ) + _rtB -> B_120_1031_0 ) + _rtB -> B_120_1057_0 ; _rtB ->
B_120_1242_0 = ( ( ( ( ( ( _rtB -> B_120_1234_0 + _rtB -> B_120_1235_0 ) +
_rtB -> B_120_1236_0 ) + _rtB -> B_120_1237_0 ) + _rtB -> B_120_1238_0 ) +
_rtB -> B_120_1239_0 ) + _rtB -> B_120_1240_0 ) + _rtB -> B_120_1241_0 ; if (
_rtB -> B_120_1242_0 > _rtP -> P_691 ) { _rtB -> B_120_1243_0 = _rtB ->
B_120_1242_0 - _rtP -> P_691 ; } else if ( _rtB -> B_120_1242_0 >= _rtP ->
P_690 ) { _rtB -> B_120_1243_0 = 0.0 ; } else { _rtB -> B_120_1243_0 = _rtB
-> B_120_1242_0 - _rtP -> P_690 ; } _rtB -> B_120_1245_0 = ( real_T ) ( _rtB
-> B_120_1243_0 == Simulation_v2_rtC ( S ) -> B_120_1_0 ) * _rtB ->
B_120_1233_0 ; _rtB -> B_120_1247_0 = ( real_T ) ( _rtB -> B_120_1243_0 !=
Simulation_v2_rtC ( S ) -> B_120_0_0 ) * _rtB -> B_120_1243_0 ; _rtB ->
B_120_1248_0 = _rtB -> B_120_1245_0 + _rtB -> B_120_1247_0 ; isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { if ( _rtB -> B_120_40_0_g )
{ rtb_B_120_5_0 = _rtB -> B_120_20_0 ; } else { rtb_B_120_5_0 = _rtB ->
B_120_39_0_f ; } if ( _rtB -> B_120_24_0_d ) { deltaT = _rtB -> B_120_42_0 ;
} else { deltaT = _rtB -> B_120_23_0_n ; } if ( _rtB -> B_120_16_0_e ) {
riseValLimit = _rtB -> B_120_68_0 ; } else { riseValLimit = _rtB ->
B_120_15_0_d ; } if ( _rtB -> B_120_8_0 ) { rateLimiterRate = _rtB ->
B_120_94_0 ; } else { rateLimiterRate = _rtB -> B_120_7_0 ; } if ( _rtB ->
B_120_32_0_i ) { tmp = _rtB -> B_120_120_0 ; } else { tmp = _rtB ->
B_120_31_0_c ; } if ( _rtB -> B_120_81_0_n ) { tmp_0 = _rtB -> B_120_146_0 ;
} else { tmp_0 = _rtB -> B_120_79_0_k ; } if ( _rtB -> B_120_64_0 ) { tmp_1 =
_rtB -> B_120_175_0 ; } else { tmp_1 = _rtB -> B_120_63_0_p ; } if ( _rtB ->
B_120_56_0_l ) { tmp_2 = _rtB -> B_120_201_0 ; } else { tmp_2 = _rtB ->
B_120_55_0_p ; } if ( _rtB -> B_120_48_0_n ) { tmp_3 = _rtB -> B_120_227_0 ;
} else { tmp_3 = _rtB -> B_120_47_0 ; } if ( _rtB -> B_120_72_0_c ) { tmp_4 =
_rtB -> B_120_253_0 ; } else { tmp_4 = _rtB -> B_120_71_0_n ; } if ( _rtB ->
B_120_122_0_o ) { tmp_5 = _rtB -> B_120_279_0 ; } else { tmp_5 = _rtB ->
B_120_121_0_c ; } if ( _rtB -> B_120_106_0_d ) { tmp_6 = _rtB -> B_120_305_0
; } else { tmp_6 = _rtB -> B_120_105_0_g ; } if ( _rtB -> B_120_98_0_p ) {
tmp_7 = _rtB -> B_120_331_0 ; } else { tmp_7 = _rtB -> B_120_97_0_o ; } if (
_rtB -> B_120_90_0 ) { tmp_8 = _rtB -> B_120_357_0 ; } else { tmp_8 = _rtB ->
B_120_89_0_n ; } if ( _rtB -> B_120_114_0 ) { tmp_9 = _rtB -> B_120_383_0 ; }
else { tmp_9 = _rtB -> B_120_113_0_c ; } if ( _rtB -> B_120_162_0_c ) { tmp_a
= _rtB -> B_120_409_0 ; } else { tmp_a = _rtB -> B_120_161_0_m ; } if ( _rtB
-> B_120_146_0_h ) { tmp_b = _rtB -> B_120_435_0 ; } else { tmp_b = _rtB ->
B_120_145_0 ; } if ( _rtB -> B_120_138_0_c ) { tmp_c = _rtB -> B_120_461_0 ;
} else { tmp_c = _rtB -> B_120_137_0_j ; } if ( _rtB -> B_120_130_0_j ) {
tmp_d = _rtB -> B_120_487_0 ; } else { tmp_d = _rtB -> B_120_129_0_m ; } if (
_rtB -> B_120_154_0_d ) { tmp_e = _rtB -> B_120_513_0 ; } else { tmp_e = _rtB
-> B_120_153_0_k ; } if ( _rtB -> B_120_202_0_o ) { tmp_f = _rtB ->
B_120_539_0 ; } else { tmp_f = _rtB -> B_120_201_0_c ; } if ( _rtB ->
B_120_186_0_p ) { tmp_g = _rtB -> B_120_565_0 ; } else { tmp_g = _rtB ->
B_120_185_0_g ; } if ( _rtB -> B_120_178_0_p ) { tmp_h = _rtB -> B_120_591_0
; } else { tmp_h = _rtB -> B_120_177_0_d ; } if ( _rtB -> B_120_170_0 ) {
tmp_i = _rtB -> B_120_617_0 ; } else { tmp_i = _rtB -> B_120_169_0_p ; } if (
_rtB -> B_120_194_0_a ) { tmp_j = _rtB -> B_120_643_0 ; } else { tmp_j = _rtB
-> B_120_193_0_c ; } if ( _rtB -> B_120_242_0_l ) { tmp_k = _rtB ->
B_120_669_0 ; } else { tmp_k = _rtB -> B_120_241_0_f ; } if ( _rtB ->
B_120_226_0 ) { tmp_l = _rtB -> B_120_695_0 ; } else { tmp_l = _rtB ->
B_120_225_0_g ; } if ( _rtB -> B_120_218_0_p ) { tmp_m = _rtB -> B_120_721_0
; } else { tmp_m = _rtB -> B_120_217_0_d ; } if ( _rtB -> B_120_210_0_j ) {
tmp_n = _rtB -> B_120_747_0 ; } else { tmp_n = _rtB -> B_120_209_0_i ; } if (
_rtB -> B_120_234_0_o ) { tmp_o = _rtB -> B_120_773_0 ; } else { tmp_o = _rtB
-> B_120_233_0_l ; } if ( _rtB -> B_120_322_0_i ) { tmp_p = _rtB ->
B_120_799_0 ; } else { tmp_p = _rtB -> B_120_321_0_b ; } if ( _rtB ->
B_120_306_0_i ) { tmp_q = _rtB -> B_120_825_0 ; } else { tmp_q = _rtB ->
B_120_305_0_o ; } if ( _rtB -> B_120_298_0_n ) { tmp_r = _rtB -> B_120_851_0
; } else { tmp_r = _rtB -> B_120_297_0_n ; } if ( _rtB -> B_120_290_0_f ) {
tmp_s = _rtB -> B_120_877_0 ; } else { tmp_s = _rtB -> B_120_289_0_h ; } if (
_rtB -> B_120_314_0_l ) { tmp_t = _rtB -> B_120_903_0 ; } else { tmp_t = _rtB
-> B_120_313_0_c ; } if ( _rtB -> B_120_282_0_c ) { tmp_u = _rtB ->
B_120_929_0 ; } else { tmp_u = _rtB -> B_120_281_0_i ; } if ( _rtB ->
B_120_266_0_j ) { tmp_v = _rtB -> B_120_955_0 ; } else { tmp_v = _rtB ->
B_120_265_0_j ; } if ( _rtB -> B_120_258_0 ) { tmp_w = _rtB -> B_120_981_0 ;
} else { tmp_w = _rtB -> B_120_257_0_d ; } if ( _rtB -> B_120_250_0_k ) {
tmp_x = _rtB -> B_120_1007_0 ; } else { tmp_x = _rtB -> B_120_249_0 ; } if (
_rtB -> B_120_274_0_f ) { tmp_y = _rtB -> B_120_1033_0 ; } else { tmp_y =
_rtB -> B_120_273_0 ; } _rtB -> B_120_1297_0 = ( ( ( ( ( ( ( ( ( (
rtb_B_120_5_0 + deltaT ) + riseValLimit ) + rateLimiterRate ) + tmp ) + ( ( (
( tmp_0 + tmp_1 ) + tmp_2 ) + tmp_3 ) + tmp_4 ) ) + ( ( ( ( tmp_5 + tmp_6 ) +
tmp_7 ) + tmp_8 ) + tmp_9 ) ) + ( ( ( ( tmp_a + tmp_b ) + tmp_c ) + tmp_d ) +
tmp_e ) ) + ( ( ( ( tmp_f + tmp_g ) + tmp_h ) + tmp_i ) + tmp_j ) ) + ( ( ( (
tmp_k + tmp_l ) + tmp_m ) + tmp_n ) + tmp_o ) ) + ( ( ( ( tmp_p + tmp_q ) +
tmp_r ) + tmp_s ) + tmp_t ) ) + ( ( ( ( tmp_u + tmp_v ) + tmp_w ) + tmp_x ) +
tmp_y ) ; } _rtB -> B_120_1298_0 = _rtB -> B_120_1248_0 / _rtB ->
B_120_1297_0 ; UNUSED_PARAMETER ( tid ) ; } static void mdlOutputsTID3 (
SimStruct * S , int_T tid ) { B_Simulation_v2_T * _rtB ; P_Simulation_v2_T *
_rtP ; _rtP = ( ( P_Simulation_v2_T * ) ssGetModelRtp ( S ) ) ; _rtB = ( (
B_Simulation_v2_T * ) _ssGetModelBlockIO ( S ) ) ; _rtB -> B_120_0_0_m = _rtP
-> P_693 ; _rtB -> B_120_1_0_c = _rtP -> P_694 ; _rtB -> B_120_3_0_c = _rtP
-> P_696 ; _rtB -> B_120_4_0 = _rtP -> P_697 ; _rtB -> B_120_7_0 = _rtP ->
P_936 ; _rtB -> B_120_8_0 = ( _rtP -> P_698 >= _rtP -> P_699 ) ; _rtB ->
B_120_9_0_b = _rtP -> P_700 ; _rtB -> B_120_11_0 = _rtP -> P_702 ; _rtB ->
B_120_12_0_p = _rtP -> P_703 ; _rtB -> B_120_15_0_d = _rtP -> P_937 ; _rtB ->
B_120_16_0_e = ( _rtP -> P_704 >= _rtP -> P_705 ) ; _rtB -> B_120_17_0_c =
_rtP -> P_706 ; _rtB -> B_120_19_0 = _rtP -> P_708 ; _rtB -> B_120_20_0_g =
_rtP -> P_709 ; _rtB -> B_120_23_0_n = _rtP -> P_938 ; _rtB -> B_120_24_0_d =
( _rtP -> P_710 >= _rtP -> P_711 ) ; _rtB -> B_120_25_0_g = _rtP -> P_712 ;
_rtB -> B_120_27_0_n = _rtP -> P_714 ; _rtB -> B_120_28_0_p = _rtP -> P_715 ;
_rtB -> B_120_31_0_c = _rtP -> P_939 ; _rtB -> B_120_32_0_i = ( _rtP -> P_716
>= _rtP -> P_717 ) ; _rtB -> B_120_33_0_l = _rtP -> P_718 ; _rtB ->
B_120_35_0_d = _rtP -> P_720 ; _rtB -> B_120_36_0 = _rtP -> P_721 ; _rtB ->
B_120_39_0_f = _rtP -> P_940 ; _rtB -> B_120_40_0_g = ( _rtP -> P_722 >= _rtP
-> P_723 ) ; _rtB -> B_120_41_0 = _rtP -> P_724 ; _rtB -> B_120_43_0_l = _rtP
-> P_726 ; _rtB -> B_120_44_0_d = _rtP -> P_727 ; _rtB -> B_120_47_0 = _rtP
-> P_941 ; _rtB -> B_120_48_0_n = ( _rtP -> P_728 >= _rtP -> P_729 ) ; _rtB
-> B_120_49_0_d = _rtP -> P_730 ; _rtB -> B_120_51_0_o = _rtP -> P_732 ; _rtB
-> B_120_52_0_b = _rtP -> P_733 ; _rtB -> B_120_55_0_p = _rtP -> P_942 ; _rtB
-> B_120_56_0_l = ( _rtP -> P_734 >= _rtP -> P_735 ) ; _rtB -> B_120_57_0_n =
_rtP -> P_736 ; _rtB -> B_120_59_0_l = _rtP -> P_738 ; _rtB -> B_120_60_0_h =
_rtP -> P_739 ; _rtB -> B_120_63_0_p = _rtP -> P_943 ; _rtB -> B_120_64_0 = (
_rtP -> P_740 >= _rtP -> P_741 ) ; _rtB -> B_120_65_0_b = _rtP -> P_742 ;
_rtB -> B_120_67_0 = _rtP -> P_744 ; _rtB -> B_120_68_0_e = _rtP -> P_745 ;
_rtB -> B_120_71_0_n = _rtP -> P_944 ; _rtB -> B_120_72_0_c = ( _rtP -> P_746
>= _rtP -> P_747 ) ; _rtB -> B_120_73_0 = _rtP -> P_748 ; _rtB ->
B_120_75_0_j = _rtP -> P_750 ; _rtB -> B_120_76_0_f = _rtP -> P_751 ; _rtB ->
B_120_79_0_k = _rtP -> P_945 ; _rtB -> B_120_81_0_n = ( _rtP -> P_752 >= _rtP
-> P_753 ) ; _rtB -> B_120_82_0_a = muDoubleScalarSin ( _rtP -> P_754 ) ;
_rtB -> B_120_83_0_j = _rtP -> P_755 ; _rtB -> B_120_85_0_o = _rtP -> P_757 ;
_rtB -> B_120_86_0_n = _rtP -> P_758 ; _rtB -> B_120_89_0_n = _rtP -> P_946 ;
_rtB -> B_120_90_0 = ( _rtP -> P_759 >= _rtP -> P_760 ) ; _rtB ->
B_120_91_0_i = _rtP -> P_761 ; _rtB -> B_120_93_0 = _rtP -> P_763 ; _rtB ->
B_120_94_0_n = _rtP -> P_764 ; _rtB -> B_120_97_0_o = _rtP -> P_947 ; _rtB ->
B_120_98_0_p = ( _rtP -> P_765 >= _rtP -> P_766 ) ; _rtB -> B_120_99_0 = _rtP
-> P_767 ; _rtB -> B_120_101_0_c = _rtP -> P_769 ; _rtB -> B_120_102_0_m =
_rtP -> P_770 ; _rtB -> B_120_105_0_g = _rtP -> P_948 ; _rtB -> B_120_106_0_d
= ( _rtP -> P_771 >= _rtP -> P_772 ) ; _rtB -> B_120_107_0_m = _rtP -> P_773
; _rtB -> B_120_109_0_h = _rtP -> P_775 ; _rtB -> B_120_110_0_c = _rtP ->
P_776 ; _rtB -> B_120_113_0_c = _rtP -> P_949 ; _rtB -> B_120_114_0 = ( _rtP
-> P_777 >= _rtP -> P_778 ) ; _rtB -> B_120_115_0_c = _rtP -> P_779 ; _rtB ->
B_120_117_0_p = _rtP -> P_781 ; _rtB -> B_120_118_0_p = _rtP -> P_782 ; _rtB
-> B_120_121_0_c = _rtP -> P_950 ; _rtB -> B_120_122_0_o = ( _rtP -> P_783 >=
_rtP -> P_784 ) ; _rtB -> B_120_123_0_a = _rtP -> P_785 ; _rtB -> B_120_125_0
= _rtP -> P_787 ; _rtB -> B_120_126_0_a = _rtP -> P_788 ; _rtB ->
B_120_129_0_m = _rtP -> P_951 ; _rtB -> B_120_130_0_j = ( _rtP -> P_789 >=
_rtP -> P_790 ) ; _rtB -> B_120_131_0_a = _rtP -> P_791 ; _rtB ->
B_120_133_0_l = _rtP -> P_793 ; _rtB -> B_120_134_0_o = _rtP -> P_794 ; _rtB
-> B_120_137_0_j = _rtP -> P_952 ; _rtB -> B_120_138_0_c = ( _rtP -> P_795 >=
_rtP -> P_796 ) ; _rtB -> B_120_139_0_o = _rtP -> P_797 ; _rtB ->
B_120_141_0_i = _rtP -> P_799 ; _rtB -> B_120_142_0 = _rtP -> P_800 ; _rtB ->
B_120_145_0 = _rtP -> P_953 ; _rtB -> B_120_146_0_h = ( _rtP -> P_801 >= _rtP
-> P_802 ) ; _rtB -> B_120_147_0_f = _rtP -> P_803 ; _rtB -> B_120_149_0_f =
_rtP -> P_805 ; _rtB -> B_120_150_0_g = _rtP -> P_806 ; _rtB -> B_120_153_0_k
= _rtP -> P_954 ; _rtB -> B_120_154_0_d = ( _rtP -> P_807 >= _rtP -> P_808 )
; _rtB -> B_120_155_0_c = _rtP -> P_809 ; _rtB -> B_120_157_0_l = _rtP ->
P_811 ; _rtB -> B_120_158_0_m = _rtP -> P_812 ; _rtB -> B_120_161_0_m = _rtP
-> P_955 ; _rtB -> B_120_162_0_c = ( _rtP -> P_813 >= _rtP -> P_814 ) ; _rtB
-> B_120_163_0_m = _rtP -> P_815 ; _rtB -> B_120_165_0_f = _rtP -> P_817 ;
_rtB -> B_120_166_0 = _rtP -> P_818 ; _rtB -> B_120_169_0_p = _rtP -> P_956 ;
_rtB -> B_120_170_0 = ( _rtP -> P_819 >= _rtP -> P_820 ) ; _rtB ->
B_120_171_0_p = _rtP -> P_821 ; _rtB -> B_120_173_0_o = _rtP -> P_823 ; _rtB
-> B_120_174_0 = _rtP -> P_824 ; _rtB -> B_120_177_0_d = _rtP -> P_957 ; _rtB
-> B_120_178_0_p = ( _rtP -> P_825 >= _rtP -> P_826 ) ; _rtB -> B_120_179_0_h
= _rtP -> P_827 ; _rtB -> B_120_181_0_l = _rtP -> P_829 ; _rtB ->
B_120_182_0_h = _rtP -> P_830 ; _rtB -> B_120_185_0_g = _rtP -> P_958 ; _rtB
-> B_120_186_0_p = ( _rtP -> P_831 >= _rtP -> P_832 ) ; _rtB -> B_120_187_0_m
= _rtP -> P_833 ; _rtB -> B_120_189_0_h = _rtP -> P_835 ; _rtB ->
B_120_190_0_c = _rtP -> P_836 ; _rtB -> B_120_193_0_c = _rtP -> P_959 ; _rtB
-> B_120_194_0_a = ( _rtP -> P_837 >= _rtP -> P_838 ) ; _rtB -> B_120_195_0 =
_rtP -> P_839 ; _rtB -> B_120_197_0 = _rtP -> P_841 ; _rtB -> B_120_198_0_p =
_rtP -> P_842 ; _rtB -> B_120_201_0_c = _rtP -> P_960 ; _rtB -> B_120_202_0_o
= ( _rtP -> P_843 >= _rtP -> P_844 ) ; _rtB -> B_120_203_0_p = _rtP -> P_845
; _rtB -> B_120_205_0_a = _rtP -> P_847 ; _rtB -> B_120_206_0 = _rtP -> P_848
; _rtB -> B_120_209_0_i = _rtP -> P_961 ; _rtB -> B_120_210_0_j = ( _rtP ->
P_849 >= _rtP -> P_850 ) ; _rtB -> B_120_211_0_j = _rtP -> P_851 ; _rtB ->
B_120_213_0_o = _rtP -> P_853 ; _rtB -> B_120_214_0_b = _rtP -> P_854 ; _rtB
-> B_120_217_0_d = _rtP -> P_962 ; _rtB -> B_120_218_0_p = ( _rtP -> P_855 >=
_rtP -> P_856 ) ; _rtB -> B_120_219_0_a = _rtP -> P_857 ; _rtB -> B_120_221_0
= _rtP -> P_859 ; _rtB -> B_120_222_0_e = _rtP -> P_860 ; _rtB ->
B_120_225_0_g = _rtP -> P_963 ; _rtB -> B_120_226_0 = ( _rtP -> P_861 >= _rtP
-> P_862 ) ; _rtB -> B_120_227_0_f = _rtP -> P_863 ; _rtB -> B_120_229_0_e =
_rtP -> P_865 ; _rtB -> B_120_230_0_c = _rtP -> P_866 ; _rtB -> B_120_233_0_l
= _rtP -> P_964 ; _rtB -> B_120_234_0_o = ( _rtP -> P_867 >= _rtP -> P_868 )
; _rtB -> B_120_235_0_a = _rtP -> P_869 ; _rtB -> B_120_237_0_a = _rtP ->
P_871 ; _rtB -> B_120_238_0_p = _rtP -> P_872 ; _rtB -> B_120_241_0_f = _rtP
-> P_965 ; _rtB -> B_120_242_0_l = ( _rtP -> P_873 >= _rtP -> P_874 ) ; _rtB
-> B_120_243_0_m = _rtP -> P_875 ; _rtB -> B_120_245_0_n = _rtP -> P_877 ;
_rtB -> B_120_246_0_l = _rtP -> P_878 ; _rtB -> B_120_249_0 = _rtP -> P_966 ;
_rtB -> B_120_250_0_k = ( _rtP -> P_879 >= _rtP -> P_880 ) ; _rtB ->
B_120_251_0_p = _rtP -> P_881 ; _rtB -> B_120_253_0_p = _rtP -> P_883 ; _rtB
-> B_120_254_0_f = _rtP -> P_884 ; _rtB -> B_120_257_0_d = _rtP -> P_967 ;
_rtB -> B_120_258_0 = ( _rtP -> P_885 >= _rtP -> P_886 ) ; _rtB ->
B_120_259_0_i = _rtP -> P_887 ; _rtB -> B_120_261_0_k = _rtP -> P_889 ; _rtB
-> B_120_262_0_i = _rtP -> P_890 ; _rtB -> B_120_265_0_j = _rtP -> P_968 ;
_rtB -> B_120_266_0_j = ( _rtP -> P_891 >= _rtP -> P_892 ) ; _rtB ->
B_120_267_0_o = _rtP -> P_893 ; _rtB -> B_120_269_0_c = _rtP -> P_895 ; _rtB
-> B_120_270_0_f = _rtP -> P_896 ; _rtB -> B_120_273_0 = _rtP -> P_969 ; _rtB
-> B_120_274_0_f = ( _rtP -> P_897 >= _rtP -> P_898 ) ; _rtB -> B_120_275_0 =
_rtP -> P_899 ; _rtB -> B_120_277_0_m = _rtP -> P_901 ; _rtB -> B_120_278_0 =
_rtP -> P_902 ; _rtB -> B_120_281_0_i = _rtP -> P_970 ; _rtB -> B_120_282_0_c
= ( _rtP -> P_903 >= _rtP -> P_904 ) ; _rtB -> B_120_283_0_a = _rtP -> P_905
; _rtB -> B_120_285_0_k = _rtP -> P_907 ; _rtB -> B_120_286_0_p = _rtP ->
P_908 ; _rtB -> B_120_289_0_h = _rtP -> P_971 ; _rtB -> B_120_290_0_f = (
_rtP -> P_909 >= _rtP -> P_910 ) ; _rtB -> B_120_291_0_b = _rtP -> P_911 ;
_rtB -> B_120_293_0_n = _rtP -> P_913 ; _rtB -> B_120_294_0_i = _rtP -> P_914
; _rtB -> B_120_297_0_n = _rtP -> P_972 ; _rtB -> B_120_298_0_n = ( _rtP ->
P_915 >= _rtP -> P_916 ) ; _rtB -> B_120_299_0 = _rtP -> P_917 ; _rtB ->
B_120_301_0 = _rtP -> P_919 ; _rtB -> B_120_302_0_j = _rtP -> P_920 ; _rtB ->
B_120_305_0_o = _rtP -> P_973 ; _rtB -> B_120_306_0_i = ( _rtP -> P_921 >=
_rtP -> P_922 ) ; _rtB -> B_120_307_0_e = _rtP -> P_923 ; _rtB ->
B_120_309_0_m = _rtP -> P_925 ; _rtB -> B_120_310_0 = _rtP -> P_926 ; _rtB ->
B_120_313_0_c = _rtP -> P_974 ; _rtB -> B_120_314_0_l = ( _rtP -> P_927 >=
_rtP -> P_928 ) ; _rtB -> B_120_315_0_j = _rtP -> P_929 ; memcpy ( & _rtB ->
B_120_2_0_k [ 0 ] , & _rtP -> P_695 [ 0 ] , 1500U * sizeof ( real_T ) ) ;
memcpy ( & _rtB -> B_120_10_0 [ 0 ] , & _rtP -> P_701 [ 0 ] , 1500U * sizeof
( real_T ) ) ; memcpy ( & _rtB -> B_120_18_0_f [ 0 ] , & _rtP -> P_707 [ 0 ]
, 1500U * sizeof ( real_T ) ) ; memcpy ( & _rtB -> B_120_26_0_m [ 0 ] , &
_rtP -> P_713 [ 0 ] , 1500U * sizeof ( real_T ) ) ; memcpy ( & _rtB ->
B_120_34_0_j [ 0 ] , & _rtP -> P_719 [ 0 ] , 1500U * sizeof ( real_T ) ) ;
memcpy ( & _rtB -> B_120_42_0_g [ 0 ] , & _rtP -> P_725 [ 0 ] , 1500U *
sizeof ( real_T ) ) ; memcpy ( & _rtB -> B_120_50_0_l [ 0 ] , & _rtP -> P_731
[ 0 ] , 1500U * sizeof ( real_T ) ) ; memcpy ( & _rtB -> B_120_58_0_b [ 0 ] ,
& _rtP -> P_737 [ 0 ] , 1500U * sizeof ( real_T ) ) ; memcpy ( & _rtB ->
B_120_66_0_d [ 0 ] , & _rtP -> P_743 [ 0 ] , 1500U * sizeof ( real_T ) ) ;
memcpy ( & _rtB -> B_120_74_0_b [ 0 ] , & _rtP -> P_749 [ 0 ] , 1500U *
sizeof ( real_T ) ) ; memcpy ( & _rtB -> B_120_84_0_j [ 0 ] , & _rtP -> P_756
[ 0 ] , 1500U * sizeof ( real_T ) ) ; memcpy ( & _rtB -> B_120_92_0_o [ 0 ] ,
& _rtP -> P_762 [ 0 ] , 1500U * sizeof ( real_T ) ) ; memcpy ( & _rtB ->
B_120_100_0_m [ 0 ] , & _rtP -> P_768 [ 0 ] , 1500U * sizeof ( real_T ) ) ;
memcpy ( & _rtB -> B_120_108_0_j [ 0 ] , & _rtP -> P_774 [ 0 ] , 1500U *
sizeof ( real_T ) ) ; memcpy ( & _rtB -> B_120_116_0 [ 0 ] , & _rtP -> P_780
[ 0 ] , 1500U * sizeof ( real_T ) ) ; memcpy ( & _rtB -> B_120_124_0_e [ 0 ]
, & _rtP -> P_786 [ 0 ] , 1500U * sizeof ( real_T ) ) ; memcpy ( & _rtB ->
B_120_132_0_i [ 0 ] , & _rtP -> P_792 [ 0 ] , 1500U * sizeof ( real_T ) ) ;
memcpy ( & _rtB -> B_120_140_0 [ 0 ] , & _rtP -> P_798 [ 0 ] , 1500U * sizeof
( real_T ) ) ; memcpy ( & _rtB -> B_120_148_0_i [ 0 ] , & _rtP -> P_804 [ 0 ]
, 1500U * sizeof ( real_T ) ) ; memcpy ( & _rtB -> B_120_156_0_o [ 0 ] , &
_rtP -> P_810 [ 0 ] , 1500U * sizeof ( real_T ) ) ; memcpy ( & _rtB ->
B_120_164_0_c [ 0 ] , & _rtP -> P_816 [ 0 ] , 1500U * sizeof ( real_T ) ) ;
memcpy ( & _rtB -> B_120_172_0_e [ 0 ] , & _rtP -> P_822 [ 0 ] , 1500U *
sizeof ( real_T ) ) ; memcpy ( & _rtB -> B_120_180_0 [ 0 ] , & _rtP -> P_828
[ 0 ] , 1500U * sizeof ( real_T ) ) ; memcpy ( & _rtB -> B_120_188_0_m [ 0 ]
, & _rtP -> P_834 [ 0 ] , 1500U * sizeof ( real_T ) ) ; memcpy ( & _rtB ->
B_120_196_0_k [ 0 ] , & _rtP -> P_840 [ 0 ] , 1500U * sizeof ( real_T ) ) ;
memcpy ( & _rtB -> B_120_204_0_p [ 0 ] , & _rtP -> P_846 [ 0 ] , 1500U *
sizeof ( real_T ) ) ; memcpy ( & _rtB -> B_120_212_0_e [ 0 ] , & _rtP ->
P_852 [ 0 ] , 1500U * sizeof ( real_T ) ) ; memcpy ( & _rtB -> B_120_220_0_g
[ 0 ] , & _rtP -> P_858 [ 0 ] , 1500U * sizeof ( real_T ) ) ; memcpy ( & _rtB
-> B_120_228_0_h [ 0 ] , & _rtP -> P_864 [ 0 ] , 1500U * sizeof ( real_T ) )
; memcpy ( & _rtB -> B_120_236_0_d [ 0 ] , & _rtP -> P_870 [ 0 ] , 1500U *
sizeof ( real_T ) ) ; memcpy ( & _rtB -> B_120_244_0_o [ 0 ] , & _rtP ->
P_876 [ 0 ] , 1500U * sizeof ( real_T ) ) ; memcpy ( & _rtB -> B_120_252_0 [
0 ] , & _rtP -> P_882 [ 0 ] , 1500U * sizeof ( real_T ) ) ; memcpy ( & _rtB
-> B_120_260_0_o [ 0 ] , & _rtP -> P_888 [ 0 ] , 1500U * sizeof ( real_T ) )
; memcpy ( & _rtB -> B_120_268_0_m [ 0 ] , & _rtP -> P_894 [ 0 ] , 1500U *
sizeof ( real_T ) ) ; memcpy ( & _rtB -> B_120_276_0_h [ 0 ] , & _rtP ->
P_900 [ 0 ] , 1500U * sizeof ( real_T ) ) ; memcpy ( & _rtB -> B_120_284_0 [
0 ] , & _rtP -> P_906 [ 0 ] , 1500U * sizeof ( real_T ) ) ; memcpy ( & _rtB
-> B_120_292_0_c [ 0 ] , & _rtP -> P_912 [ 0 ] , 1500U * sizeof ( real_T ) )
; memcpy ( & _rtB -> B_120_300_0_m [ 0 ] , & _rtP -> P_918 [ 0 ] , 1500U *
sizeof ( real_T ) ) ; memcpy ( & _rtB -> B_120_308_0_m [ 0 ] , & _rtP ->
P_924 [ 0 ] , 1500U * sizeof ( real_T ) ) ; memcpy ( & _rtB -> B_120_316_0_f
[ 0 ] , & _rtP -> P_930 [ 0 ] , 1500U * sizeof ( real_T ) ) ; _rtB ->
B_120_317_0_a = _rtP -> P_931 ; _rtB -> B_120_318_0_g = _rtP -> P_932 ; _rtB
-> B_120_321_0_b = _rtP -> P_975 ; _rtB -> B_120_322_0_i = ( _rtP -> P_933 >=
_rtP -> P_934 ) ; _rtB -> B_120_323_0_n = _rtP -> P_935 ; UNUSED_PARAMETER (
tid ) ; }
#define MDL_UPDATE
static void mdlUpdate ( SimStruct * S , int_T tid ) { int32_T isHit ;
B_Simulation_v2_T * _rtB ; P_Simulation_v2_T * _rtP ; DW_Simulation_v2_T *
_rtDW ; _rtDW = ( ( DW_Simulation_v2_T * ) ssGetRootDWork ( S ) ) ; _rtP = (
( P_Simulation_v2_T * ) ssGetModelRtp ( S ) ) ; _rtB = ( ( B_Simulation_v2_T
* ) _ssGetModelBlockIO ( S ) ) ; _rtDW -> Integrator_IWORK = 0 ; isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtDW -> DelayInput1_DSTATE
= _rtB -> B_120_2_0 ; } if ( _rtDW -> LastMajorTimeA == ( rtInf ) ) { _rtDW
-> LastMajorTimeA = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA = _rtB ->
B_120_25_0 ; } else if ( _rtDW -> LastMajorTimeB == ( rtInf ) ) { _rtDW ->
LastMajorTimeB = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB = _rtB ->
B_120_25_0 ; } else if ( _rtDW -> LastMajorTimeA < _rtDW -> LastMajorTimeB )
{ _rtDW -> LastMajorTimeA = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA = _rtB
-> B_120_25_0 ; } else { _rtDW -> LastMajorTimeB = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYB = _rtB -> B_120_25_0 ; } _rtDW -> vv0at_IWORK = 0 ; { real_T
* * uBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK . TUbufferPtrs [
0 ] ; real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK . Head = ( ( _rtDW -> TransportDelay_IWORK . Head < (
_rtDW -> TransportDelay_IWORK . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK . Head + 1 ) : 0 ) ; if ( _rtDW -> TransportDelay_IWORK
. Head == _rtDW -> TransportDelay_IWORK . Tail ) { if ( !
Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK . CircularBufSize , & _rtDW -> TransportDelay_IWORK .
Tail , & _rtDW -> TransportDelay_IWORK . Head , & _rtDW ->
TransportDelay_IWORK . Last , simTime - _rtP -> P_96 , tBuffer , uBuffer , (
NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK . Head ] = _rtB ->
B_120_17_0 ; } if ( _rtDW -> LastMajorTimeA_b == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_b = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_g = _rtB ->
B_120_51_0 ; } else if ( _rtDW -> LastMajorTimeB_j == ( rtInf ) ) { _rtDW ->
LastMajorTimeB_j = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_b = _rtB ->
B_120_51_0 ; } else if ( _rtDW -> LastMajorTimeA_b < _rtDW ->
LastMajorTimeB_j ) { _rtDW -> LastMajorTimeA_b = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYA_g = _rtB -> B_120_51_0 ; } else { _rtDW -> LastMajorTimeB_j =
ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_b = _rtB -> B_120_51_0 ; } _rtDW ->
vv0at_IWORK_b = 0 ; { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_a . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T *
* ) & _rtDW -> TransportDelay_PWORK_a . TUbufferPtrs [ 1 ] ; real_T simTime =
ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_f . Head = ( ( _rtDW ->
TransportDelay_IWORK_f . Head < ( _rtDW -> TransportDelay_IWORK_f .
CircularBufSize - 1 ) ) ? ( _rtDW -> TransportDelay_IWORK_f . Head + 1 ) : 0
) ; if ( _rtDW -> TransportDelay_IWORK_f . Head == _rtDW ->
TransportDelay_IWORK_f . Tail ) { if ( !
Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_f . CircularBufSize , & _rtDW -> TransportDelay_IWORK_f
. Tail , & _rtDW -> TransportDelay_IWORK_f . Head , & _rtDW ->
TransportDelay_IWORK_f . Last , simTime - _rtP -> P_110 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_f .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_f . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_f . Head ] = _rtB ->
B_120_48_0 ; } if ( _rtDW -> LastMajorTimeA_i == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_i = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_e = _rtB ->
B_120_77_0 ; } else if ( _rtDW -> LastMajorTimeB_m == ( rtInf ) ) { _rtDW ->
LastMajorTimeB_m = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_l = _rtB ->
B_120_77_0 ; } else if ( _rtDW -> LastMajorTimeA_i < _rtDW ->
LastMajorTimeB_m ) { _rtDW -> LastMajorTimeA_i = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYA_e = _rtB -> B_120_77_0 ; } else { _rtDW -> LastMajorTimeB_m =
ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_l = _rtB -> B_120_77_0 ; } _rtDW ->
vv0at_IWORK_h = 0 ; { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_o . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T *
* ) & _rtDW -> TransportDelay_PWORK_o . TUbufferPtrs [ 1 ] ; real_T simTime =
ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_i . Head = ( ( _rtDW ->
TransportDelay_IWORK_i . Head < ( _rtDW -> TransportDelay_IWORK_i .
CircularBufSize - 1 ) ) ? ( _rtDW -> TransportDelay_IWORK_i . Head + 1 ) : 0
) ; if ( _rtDW -> TransportDelay_IWORK_i . Head == _rtDW ->
TransportDelay_IWORK_i . Tail ) { if ( !
Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_i . CircularBufSize , & _rtDW -> TransportDelay_IWORK_i
. Tail , & _rtDW -> TransportDelay_IWORK_i . Head , & _rtDW ->
TransportDelay_IWORK_i . Last , simTime - _rtP -> P_124 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_i .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_i . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_i . Head ] = _rtB ->
B_120_74_0 ; } if ( _rtDW -> LastMajorTimeA_e == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_e = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_j = _rtB ->
B_120_103_0 ; } else if ( _rtDW -> LastMajorTimeB_p == ( rtInf ) ) { _rtDW ->
LastMajorTimeB_p = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_b0 = _rtB ->
B_120_103_0 ; } else if ( _rtDW -> LastMajorTimeA_e < _rtDW ->
LastMajorTimeB_p ) { _rtDW -> LastMajorTimeA_e = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYA_j = _rtB -> B_120_103_0 ; } else { _rtDW -> LastMajorTimeB_p
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_b0 = _rtB -> B_120_103_0 ; }
_rtDW -> vv0at_IWORK_e = 0 ; { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_n . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T *
* ) & _rtDW -> TransportDelay_PWORK_n . TUbufferPtrs [ 1 ] ; real_T simTime =
ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_f2 . Head = ( ( _rtDW ->
TransportDelay_IWORK_f2 . Head < ( _rtDW -> TransportDelay_IWORK_f2 .
CircularBufSize - 1 ) ) ? ( _rtDW -> TransportDelay_IWORK_f2 . Head + 1 ) : 0
) ; if ( _rtDW -> TransportDelay_IWORK_f2 . Head == _rtDW ->
TransportDelay_IWORK_f2 . Tail ) { if ( !
Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_f2 . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_f2 . Tail , & _rtDW -> TransportDelay_IWORK_f2 . Head ,
& _rtDW -> TransportDelay_IWORK_f2 . Last , simTime - _rtP -> P_138 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_f2 . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_f2 . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_f2 . Head ] = _rtB -> B_120_100_0 ; } if ( _rtDW ->
LastMajorTimeA_i2 == ( rtInf ) ) { _rtDW -> LastMajorTimeA_i2 = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_h = _rtB -> B_120_129_0 ; } else if ( _rtDW ->
LastMajorTimeB_c == ( rtInf ) ) { _rtDW -> LastMajorTimeB_c = ssGetTaskTime (
S , 0 ) ; _rtDW -> PrevYB_f = _rtB -> B_120_129_0 ; } else if ( _rtDW ->
LastMajorTimeA_i2 < _rtDW -> LastMajorTimeB_c ) { _rtDW -> LastMajorTimeA_i2
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_h = _rtB -> B_120_129_0 ; } else
{ _rtDW -> LastMajorTimeB_c = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_f =
_rtB -> B_120_129_0 ; } _rtDW -> vv0at_IWORK_c = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_m . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_m . TUbufferPtrs
[ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_k .
Head = ( ( _rtDW -> TransportDelay_IWORK_k . Head < ( _rtDW ->
TransportDelay_IWORK_k . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_k . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_k . Head == _rtDW -> TransportDelay_IWORK_k . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_k . CircularBufSize , & _rtDW -> TransportDelay_IWORK_k
. Tail , & _rtDW -> TransportDelay_IWORK_k . Head , & _rtDW ->
TransportDelay_IWORK_k . Last , simTime - _rtP -> P_152 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_k .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_k . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_k . Head ] = _rtB ->
B_120_126_0 ; } if ( _rtDW -> LastMajorTimeA_c == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_c = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_i = _rtB ->
B_120_155_0 ; } else if ( _rtDW -> LastMajorTimeB_mm == ( rtInf ) ) { _rtDW
-> LastMajorTimeB_mm = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_g = _rtB ->
B_120_155_0 ; } else if ( _rtDW -> LastMajorTimeA_c < _rtDW ->
LastMajorTimeB_mm ) { _rtDW -> LastMajorTimeA_c = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYA_i = _rtB -> B_120_155_0 ; } else { _rtDW -> LastMajorTimeB_mm
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_g = _rtB -> B_120_155_0 ; } _rtDW
-> vv0at_IWORK_m = 0 ; { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_f . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T *
* ) & _rtDW -> TransportDelay_PWORK_f . TUbufferPtrs [ 1 ] ; real_T simTime =
ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_m . Head = ( ( _rtDW ->
TransportDelay_IWORK_m . Head < ( _rtDW -> TransportDelay_IWORK_m .
CircularBufSize - 1 ) ) ? ( _rtDW -> TransportDelay_IWORK_m . Head + 1 ) : 0
) ; if ( _rtDW -> TransportDelay_IWORK_m . Head == _rtDW ->
TransportDelay_IWORK_m . Tail ) { if ( !
Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_m . CircularBufSize , & _rtDW -> TransportDelay_IWORK_m
. Tail , & _rtDW -> TransportDelay_IWORK_m . Head , & _rtDW ->
TransportDelay_IWORK_m . Last , simTime - _rtP -> P_167 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_m .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_m . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_m . Head ] = _rtB ->
B_120_152_0 ; } if ( _rtDW -> LastMajorTimeA_b0 == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_b0 = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_a = _rtB ->
B_120_184_0 ; } else if ( _rtDW -> LastMajorTimeB_e == ( rtInf ) ) { _rtDW ->
LastMajorTimeB_e = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_lz = _rtB ->
B_120_184_0 ; } else if ( _rtDW -> LastMajorTimeA_b0 < _rtDW ->
LastMajorTimeB_e ) { _rtDW -> LastMajorTimeA_b0 = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYA_a = _rtB -> B_120_184_0 ; } else { _rtDW -> LastMajorTimeB_e
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_lz = _rtB -> B_120_184_0 ; }
_rtDW -> vv0at_IWORK_l = 0 ; { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_p . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T *
* ) & _rtDW -> TransportDelay_PWORK_p . TUbufferPtrs [ 1 ] ; real_T simTime =
ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_l . Head = ( ( _rtDW ->
TransportDelay_IWORK_l . Head < ( _rtDW -> TransportDelay_IWORK_l .
CircularBufSize - 1 ) ) ? ( _rtDW -> TransportDelay_IWORK_l . Head + 1 ) : 0
) ; if ( _rtDW -> TransportDelay_IWORK_l . Head == _rtDW ->
TransportDelay_IWORK_l . Tail ) { if ( !
Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_l . CircularBufSize , & _rtDW -> TransportDelay_IWORK_l
. Tail , & _rtDW -> TransportDelay_IWORK_l . Head , & _rtDW ->
TransportDelay_IWORK_l . Last , simTime - _rtP -> P_181 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_l .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_l . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_l . Head ] = _rtB ->
B_120_181_0 ; } if ( _rtDW -> LastMajorTimeA_m == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_m = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_h4 = _rtB ->
B_120_210_0 ; } else if ( _rtDW -> LastMajorTimeB_d == ( rtInf ) ) { _rtDW ->
LastMajorTimeB_d = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_a = _rtB ->
B_120_210_0 ; } else if ( _rtDW -> LastMajorTimeA_m < _rtDW ->
LastMajorTimeB_d ) { _rtDW -> LastMajorTimeA_m = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYA_h4 = _rtB -> B_120_210_0 ; } else { _rtDW -> LastMajorTimeB_d
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_a = _rtB -> B_120_210_0 ; } _rtDW
-> vv0at_IWORK_p = 0 ; { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_c . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T *
* ) & _rtDW -> TransportDelay_PWORK_c . TUbufferPtrs [ 1 ] ; real_T simTime =
ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_g . Head = ( ( _rtDW ->
TransportDelay_IWORK_g . Head < ( _rtDW -> TransportDelay_IWORK_g .
CircularBufSize - 1 ) ) ? ( _rtDW -> TransportDelay_IWORK_g . Head + 1 ) : 0
) ; if ( _rtDW -> TransportDelay_IWORK_g . Head == _rtDW ->
TransportDelay_IWORK_g . Tail ) { if ( !
Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_g . CircularBufSize , & _rtDW -> TransportDelay_IWORK_g
. Tail , & _rtDW -> TransportDelay_IWORK_g . Head , & _rtDW ->
TransportDelay_IWORK_g . Last , simTime - _rtP -> P_195 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_g .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_g . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_g . Head ] = _rtB ->
B_120_207_0 ; } if ( _rtDW -> LastMajorTimeA_mj == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_mj = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_eh = _rtB ->
B_120_236_0 ; } else if ( _rtDW -> LastMajorTimeB_g == ( rtInf ) ) { _rtDW ->
LastMajorTimeB_g = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_p = _rtB ->
B_120_236_0 ; } else if ( _rtDW -> LastMajorTimeA_mj < _rtDW ->
LastMajorTimeB_g ) { _rtDW -> LastMajorTimeA_mj = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYA_eh = _rtB -> B_120_236_0 ; } else { _rtDW -> LastMajorTimeB_g
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_p = _rtB -> B_120_236_0 ; } _rtDW
-> vv0at_IWORK_f = 0 ; { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_j . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T *
* ) & _rtDW -> TransportDelay_PWORK_j . TUbufferPtrs [ 1 ] ; real_T simTime =
ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_j . Head = ( ( _rtDW ->
TransportDelay_IWORK_j . Head < ( _rtDW -> TransportDelay_IWORK_j .
CircularBufSize - 1 ) ) ? ( _rtDW -> TransportDelay_IWORK_j . Head + 1 ) : 0
) ; if ( _rtDW -> TransportDelay_IWORK_j . Head == _rtDW ->
TransportDelay_IWORK_j . Tail ) { if ( !
Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_j . CircularBufSize , & _rtDW -> TransportDelay_IWORK_j
. Tail , & _rtDW -> TransportDelay_IWORK_j . Head , & _rtDW ->
TransportDelay_IWORK_j . Last , simTime - _rtP -> P_209 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_j .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_j . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_j . Head ] = _rtB ->
B_120_233_0 ; } if ( _rtDW -> LastMajorTimeA_a == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_a = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_gw = _rtB ->
B_120_262_0 ; } else if ( _rtDW -> LastMajorTimeB_ms == ( rtInf ) ) { _rtDW
-> LastMajorTimeB_ms = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_gv = _rtB ->
B_120_262_0 ; } else if ( _rtDW -> LastMajorTimeA_a < _rtDW ->
LastMajorTimeB_ms ) { _rtDW -> LastMajorTimeA_a = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYA_gw = _rtB -> B_120_262_0 ; } else { _rtDW ->
LastMajorTimeB_ms = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_gv = _rtB ->
B_120_262_0 ; } _rtDW -> vv0at_IWORK_g = 0 ; { real_T * * uBuffer = ( real_T
* * ) & _rtDW -> TransportDelay_PWORK_d . TUbufferPtrs [ 0 ] ; real_T * *
tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_d . TUbufferPtrs [ 1
] ; real_T simTime = ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_jj . Head =
( ( _rtDW -> TransportDelay_IWORK_jj . Head < ( _rtDW ->
TransportDelay_IWORK_jj . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_jj . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_jj . Head == _rtDW -> TransportDelay_IWORK_jj . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_jj . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_jj . Tail , & _rtDW -> TransportDelay_IWORK_jj . Head ,
& _rtDW -> TransportDelay_IWORK_jj . Last , simTime - _rtP -> P_223 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_jj . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_jj . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_jj . Head ] = _rtB -> B_120_259_0 ; } if ( _rtDW ->
LastMajorTimeA_h == ( rtInf ) ) { _rtDW -> LastMajorTimeA_h = ssGetTaskTime (
S , 0 ) ; _rtDW -> PrevYA_gg = _rtB -> B_120_288_0 ; } else if ( _rtDW ->
LastMajorTimeB_f == ( rtInf ) ) { _rtDW -> LastMajorTimeB_f = ssGetTaskTime (
S , 0 ) ; _rtDW -> PrevYB_n = _rtB -> B_120_288_0 ; } else if ( _rtDW ->
LastMajorTimeA_h < _rtDW -> LastMajorTimeB_f ) { _rtDW -> LastMajorTimeA_h =
ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_gg = _rtB -> B_120_288_0 ; } else {
_rtDW -> LastMajorTimeB_f = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_n =
_rtB -> B_120_288_0 ; } _rtDW -> vv0at_IWORK_l5 = 0 ; { real_T * * uBuffer =
( real_T * * ) & _rtDW -> TransportDelay_PWORK_i . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_i .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_p . Head = ( ( _rtDW -> TransportDelay_IWORK_p . Head <
( _rtDW -> TransportDelay_IWORK_p . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_p . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_p . Head == _rtDW -> TransportDelay_IWORK_p . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_p . CircularBufSize , & _rtDW -> TransportDelay_IWORK_p
. Tail , & _rtDW -> TransportDelay_IWORK_p . Head , & _rtDW ->
TransportDelay_IWORK_p . Last , simTime - _rtP -> P_237 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_p .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_p . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_p . Head ] = _rtB ->
B_120_285_0 ; } if ( _rtDW -> LastMajorTimeA_n == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_n = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_p = _rtB ->
B_120_314_0 ; } else if ( _rtDW -> LastMajorTimeB_i == ( rtInf ) ) { _rtDW ->
LastMajorTimeB_i = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_d = _rtB ->
B_120_314_0 ; } else if ( _rtDW -> LastMajorTimeA_n < _rtDW ->
LastMajorTimeB_i ) { _rtDW -> LastMajorTimeA_n = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYA_p = _rtB -> B_120_314_0 ; } else { _rtDW -> LastMajorTimeB_i
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_d = _rtB -> B_120_314_0 ; } _rtDW
-> vv0at_IWORK_pv = 0 ; { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_l . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T *
* ) & _rtDW -> TransportDelay_PWORK_l . TUbufferPtrs [ 1 ] ; real_T simTime =
ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_o . Head = ( ( _rtDW ->
TransportDelay_IWORK_o . Head < ( _rtDW -> TransportDelay_IWORK_o .
CircularBufSize - 1 ) ) ? ( _rtDW -> TransportDelay_IWORK_o . Head + 1 ) : 0
) ; if ( _rtDW -> TransportDelay_IWORK_o . Head == _rtDW ->
TransportDelay_IWORK_o . Tail ) { if ( !
Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_o . CircularBufSize , & _rtDW -> TransportDelay_IWORK_o
. Tail , & _rtDW -> TransportDelay_IWORK_o . Head , & _rtDW ->
TransportDelay_IWORK_o . Last , simTime - _rtP -> P_251 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_o .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_o . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_o . Head ] = _rtB ->
B_120_311_0 ; } if ( _rtDW -> LastMajorTimeA_d == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_d = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_m = _rtB ->
B_120_340_0 ; } else if ( _rtDW -> LastMajorTimeB_n == ( rtInf ) ) { _rtDW ->
LastMajorTimeB_n = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_j = _rtB ->
B_120_340_0 ; } else if ( _rtDW -> LastMajorTimeA_d < _rtDW ->
LastMajorTimeB_n ) { _rtDW -> LastMajorTimeA_d = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYA_m = _rtB -> B_120_340_0 ; } else { _rtDW -> LastMajorTimeB_n
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_j = _rtB -> B_120_340_0 ; } _rtDW
-> vv0at_IWORK_pu = 0 ; { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_g . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T *
* ) & _rtDW -> TransportDelay_PWORK_g . TUbufferPtrs [ 1 ] ; real_T simTime =
ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_ls . Head = ( ( _rtDW ->
TransportDelay_IWORK_ls . Head < ( _rtDW -> TransportDelay_IWORK_ls .
CircularBufSize - 1 ) ) ? ( _rtDW -> TransportDelay_IWORK_ls . Head + 1 ) : 0
) ; if ( _rtDW -> TransportDelay_IWORK_ls . Head == _rtDW ->
TransportDelay_IWORK_ls . Tail ) { if ( !
Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_ls . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_ls . Tail , & _rtDW -> TransportDelay_IWORK_ls . Head ,
& _rtDW -> TransportDelay_IWORK_ls . Last , simTime - _rtP -> P_265 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_ls . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_ls . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_ls . Head ] = _rtB -> B_120_337_0 ; } if ( _rtDW ->
LastMajorTimeA_f == ( rtInf ) ) { _rtDW -> LastMajorTimeA_f = ssGetTaskTime (
S , 0 ) ; _rtDW -> PrevYA_mj = _rtB -> B_120_366_0 ; } else if ( _rtDW ->
LastMajorTimeB_a == ( rtInf ) ) { _rtDW -> LastMajorTimeB_a = ssGetTaskTime (
S , 0 ) ; _rtDW -> PrevYB_jd = _rtB -> B_120_366_0 ; } else if ( _rtDW ->
LastMajorTimeA_f < _rtDW -> LastMajorTimeB_a ) { _rtDW -> LastMajorTimeA_f =
ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_mj = _rtB -> B_120_366_0 ; } else {
_rtDW -> LastMajorTimeB_a = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_jd =
_rtB -> B_120_366_0 ; } _rtDW -> vv0at_IWORK_o = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_pc . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_pc .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_d . Head = ( ( _rtDW -> TransportDelay_IWORK_d . Head <
( _rtDW -> TransportDelay_IWORK_d . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_d . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_d . Head == _rtDW -> TransportDelay_IWORK_d . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_d . CircularBufSize , & _rtDW -> TransportDelay_IWORK_d
. Tail , & _rtDW -> TransportDelay_IWORK_d . Head , & _rtDW ->
TransportDelay_IWORK_d . Last , simTime - _rtP -> P_279 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_d .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_d . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_d . Head ] = _rtB ->
B_120_363_0 ; } if ( _rtDW -> LastMajorTimeA_hl == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_hl = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_ir = _rtB ->
B_120_392_0 ; } else if ( _rtDW -> LastMajorTimeB_ga == ( rtInf ) ) { _rtDW
-> LastMajorTimeB_ga = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_al = _rtB ->
B_120_392_0 ; } else if ( _rtDW -> LastMajorTimeA_hl < _rtDW ->
LastMajorTimeB_ga ) { _rtDW -> LastMajorTimeA_hl = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYA_ir = _rtB -> B_120_392_0 ; } else { _rtDW ->
LastMajorTimeB_ga = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_al = _rtB ->
B_120_392_0 ; } _rtDW -> vv0at_IWORK_i = 0 ; { real_T * * uBuffer = ( real_T
* * ) & _rtDW -> TransportDelay_PWORK_c2 . TUbufferPtrs [ 0 ] ; real_T * *
tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_c2 . TUbufferPtrs [
1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_a . Head
= ( ( _rtDW -> TransportDelay_IWORK_a . Head < ( _rtDW ->
TransportDelay_IWORK_a . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_a . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_a . Head == _rtDW -> TransportDelay_IWORK_a . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_a . CircularBufSize , & _rtDW -> TransportDelay_IWORK_a
. Tail , & _rtDW -> TransportDelay_IWORK_a . Head , & _rtDW ->
TransportDelay_IWORK_a . Last , simTime - _rtP -> P_293 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_a .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_a . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_a . Head ] = _rtB ->
B_120_389_0 ; } if ( _rtDW -> LastMajorTimeA_ix == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_ix = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_mo = _rtB ->
B_120_418_0 ; } else if ( _rtDW -> LastMajorTimeB_mr == ( rtInf ) ) { _rtDW
-> LastMajorTimeB_mr = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_c = _rtB ->
B_120_418_0 ; } else if ( _rtDW -> LastMajorTimeA_ix < _rtDW ->
LastMajorTimeB_mr ) { _rtDW -> LastMajorTimeA_ix = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYA_mo = _rtB -> B_120_418_0 ; } else { _rtDW ->
LastMajorTimeB_mr = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_c = _rtB ->
B_120_418_0 ; } _rtDW -> vv0at_IWORK_ie = 0 ; { real_T * * uBuffer = ( real_T
* * ) & _rtDW -> TransportDelay_PWORK_je . TUbufferPtrs [ 0 ] ; real_T * *
tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_je . TUbufferPtrs [
1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_dg . Head
= ( ( _rtDW -> TransportDelay_IWORK_dg . Head < ( _rtDW ->
TransportDelay_IWORK_dg . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_dg . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_dg . Head == _rtDW -> TransportDelay_IWORK_dg . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_dg . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_dg . Tail , & _rtDW -> TransportDelay_IWORK_dg . Head ,
& _rtDW -> TransportDelay_IWORK_dg . Last , simTime - _rtP -> P_307 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_dg . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_dg . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_dg . Head ] = _rtB -> B_120_415_0 ; } if ( _rtDW ->
LastMajorTimeA_l == ( rtInf ) ) { _rtDW -> LastMajorTimeA_l = ssGetTaskTime (
S , 0 ) ; _rtDW -> PrevYA_f = _rtB -> B_120_444_0 ; } else if ( _rtDW ->
LastMajorTimeB_fh == ( rtInf ) ) { _rtDW -> LastMajorTimeB_fh = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_k = _rtB -> B_120_444_0 ; } else if ( _rtDW ->
LastMajorTimeA_l < _rtDW -> LastMajorTimeB_fh ) { _rtDW -> LastMajorTimeA_l =
ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_f = _rtB -> B_120_444_0 ; } else {
_rtDW -> LastMajorTimeB_fh = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_k =
_rtB -> B_120_444_0 ; } _rtDW -> vv0at_IWORK_gs = 0 ; { real_T * * uBuffer =
( real_T * * ) & _rtDW -> TransportDelay_PWORK_i5 . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_i5 .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_gj . Head = ( ( _rtDW -> TransportDelay_IWORK_gj . Head
< ( _rtDW -> TransportDelay_IWORK_gj . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_gj . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_gj . Head == _rtDW -> TransportDelay_IWORK_gj . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_gj . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_gj . Tail , & _rtDW -> TransportDelay_IWORK_gj . Head ,
& _rtDW -> TransportDelay_IWORK_gj . Last , simTime - _rtP -> P_321 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_gj . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_gj . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_gj . Head ] = _rtB -> B_120_441_0 ; } if ( _rtDW ->
LastMajorTimeA_nu == ( rtInf ) ) { _rtDW -> LastMajorTimeA_nu = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_im = _rtB -> B_120_470_0 ; } else if ( _rtDW ->
LastMajorTimeB_m5 == ( rtInf ) ) { _rtDW -> LastMajorTimeB_m5 = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_m = _rtB -> B_120_470_0 ; } else if ( _rtDW ->
LastMajorTimeA_nu < _rtDW -> LastMajorTimeB_m5 ) { _rtDW -> LastMajorTimeA_nu
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_im = _rtB -> B_120_470_0 ; } else
{ _rtDW -> LastMajorTimeB_m5 = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_m =
_rtB -> B_120_470_0 ; } _rtDW -> vv0at_IWORK_pa = 0 ; { real_T * * uBuffer =
( real_T * * ) & _rtDW -> TransportDelay_PWORK_nh . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_nh .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_dp . Head = ( ( _rtDW -> TransportDelay_IWORK_dp . Head
< ( _rtDW -> TransportDelay_IWORK_dp . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_dp . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_dp . Head == _rtDW -> TransportDelay_IWORK_dp . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_dp . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_dp . Tail , & _rtDW -> TransportDelay_IWORK_dp . Head ,
& _rtDW -> TransportDelay_IWORK_dp . Last , simTime - _rtP -> P_335 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_dp . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_dp . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_dp . Head ] = _rtB -> B_120_467_0 ; } if ( _rtDW ->
LastMajorTimeA_j == ( rtInf ) ) { _rtDW -> LastMajorTimeA_j = ssGetTaskTime (
S , 0 ) ; _rtDW -> PrevYA_k = _rtB -> B_120_496_0 ; } else if ( _rtDW ->
LastMajorTimeB_as == ( rtInf ) ) { _rtDW -> LastMajorTimeB_as = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_k5 = _rtB -> B_120_496_0 ; } else if ( _rtDW ->
LastMajorTimeA_j < _rtDW -> LastMajorTimeB_as ) { _rtDW -> LastMajorTimeA_j =
ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_k = _rtB -> B_120_496_0 ; } else {
_rtDW -> LastMajorTimeB_as = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_k5 =
_rtB -> B_120_496_0 ; } _rtDW -> vv0at_IWORK_n = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_k . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_k . TUbufferPtrs
[ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_b .
Head = ( ( _rtDW -> TransportDelay_IWORK_b . Head < ( _rtDW ->
TransportDelay_IWORK_b . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_b . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_b . Head == _rtDW -> TransportDelay_IWORK_b . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_b . CircularBufSize , & _rtDW -> TransportDelay_IWORK_b
. Tail , & _rtDW -> TransportDelay_IWORK_b . Head , & _rtDW ->
TransportDelay_IWORK_b . Last , simTime - _rtP -> P_349 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_b .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_b . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_b . Head ] = _rtB ->
B_120_493_0 ; } if ( _rtDW -> LastMajorTimeA_p == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_p = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_kv = _rtB ->
B_120_522_0 ; } else if ( _rtDW -> LastMajorTimeB_l == ( rtInf ) ) { _rtDW ->
LastMajorTimeB_l = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_mx = _rtB ->
B_120_522_0 ; } else if ( _rtDW -> LastMajorTimeA_p < _rtDW ->
LastMajorTimeB_l ) { _rtDW -> LastMajorTimeA_p = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYA_kv = _rtB -> B_120_522_0 ; } else { _rtDW -> LastMajorTimeB_l
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_mx = _rtB -> B_120_522_0 ; }
_rtDW -> vv0at_IWORK_p5 = 0 ; { real_T * * uBuffer = ( real_T * * ) & _rtDW
-> TransportDelay_PWORK_av . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_av . TUbufferPtrs [ 1 ] ; real_T
simTime = ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_ai . Head = ( ( _rtDW
-> TransportDelay_IWORK_ai . Head < ( _rtDW -> TransportDelay_IWORK_ai .
CircularBufSize - 1 ) ) ? ( _rtDW -> TransportDelay_IWORK_ai . Head + 1 ) : 0
) ; if ( _rtDW -> TransportDelay_IWORK_ai . Head == _rtDW ->
TransportDelay_IWORK_ai . Tail ) { if ( !
Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_ai . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_ai . Tail , & _rtDW -> TransportDelay_IWORK_ai . Head ,
& _rtDW -> TransportDelay_IWORK_ai . Last , simTime - _rtP -> P_363 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_ai . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_ai . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_ai . Head ] = _rtB -> B_120_519_0 ; } if ( _rtDW ->
LastMajorTimeA_fl == ( rtInf ) ) { _rtDW -> LastMajorTimeA_fl = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_o = _rtB -> B_120_548_0 ; } else if ( _rtDW ->
LastMajorTimeB_g3 == ( rtInf ) ) { _rtDW -> LastMajorTimeB_g3 = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_ku = _rtB -> B_120_548_0 ; } else if ( _rtDW ->
LastMajorTimeA_fl < _rtDW -> LastMajorTimeB_g3 ) { _rtDW -> LastMajorTimeA_fl
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_o = _rtB -> B_120_548_0 ; } else
{ _rtDW -> LastMajorTimeB_g3 = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_ku =
_rtB -> B_120_548_0 ; } _rtDW -> vv0at_IWORK_d = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_pt . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_pt .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_bw . Head = ( ( _rtDW -> TransportDelay_IWORK_bw . Head
< ( _rtDW -> TransportDelay_IWORK_bw . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_bw . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_bw . Head == _rtDW -> TransportDelay_IWORK_bw . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_bw . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_bw . Tail , & _rtDW -> TransportDelay_IWORK_bw . Head ,
& _rtDW -> TransportDelay_IWORK_bw . Last , simTime - _rtP -> P_377 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_bw . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_bw . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_bw . Head ] = _rtB -> B_120_545_0 ; } if ( _rtDW ->
LastMajorTimeA_l4 == ( rtInf ) ) { _rtDW -> LastMajorTimeA_l4 = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_oa = _rtB -> B_120_574_0 ; } else if ( _rtDW ->
LastMajorTimeB_gh == ( rtInf ) ) { _rtDW -> LastMajorTimeB_gh = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_p3 = _rtB -> B_120_574_0 ; } else if ( _rtDW ->
LastMajorTimeA_l4 < _rtDW -> LastMajorTimeB_gh ) { _rtDW -> LastMajorTimeA_l4
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_oa = _rtB -> B_120_574_0 ; } else
{ _rtDW -> LastMajorTimeB_gh = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_p3 =
_rtB -> B_120_574_0 ; } _rtDW -> vv0at_IWORK_k = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_jo . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_jo .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_lz . Head = ( ( _rtDW -> TransportDelay_IWORK_lz . Head
< ( _rtDW -> TransportDelay_IWORK_lz . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_lz . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_lz . Head == _rtDW -> TransportDelay_IWORK_lz . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_lz . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_lz . Tail , & _rtDW -> TransportDelay_IWORK_lz . Head ,
& _rtDW -> TransportDelay_IWORK_lz . Last , simTime - _rtP -> P_391 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_lz . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_lz . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_lz . Head ] = _rtB -> B_120_571_0 ; } if ( _rtDW ->
LastMajorTimeA_e4 == ( rtInf ) ) { _rtDW -> LastMajorTimeA_e4 = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_oi = _rtB -> B_120_600_0 ; } else if ( _rtDW ->
LastMajorTimeB_lv == ( rtInf ) ) { _rtDW -> LastMajorTimeB_lv = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_i = _rtB -> B_120_600_0 ; } else if ( _rtDW ->
LastMajorTimeA_e4 < _rtDW -> LastMajorTimeB_lv ) { _rtDW -> LastMajorTimeA_e4
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_oi = _rtB -> B_120_600_0 ; } else
{ _rtDW -> LastMajorTimeB_lv = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_i =
_rtB -> B_120_600_0 ; } _rtDW -> vv0at_IWORK_a = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_no . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_no .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_jf . Head = ( ( _rtDW -> TransportDelay_IWORK_jf . Head
< ( _rtDW -> TransportDelay_IWORK_jf . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_jf . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_jf . Head == _rtDW -> TransportDelay_IWORK_jf . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_jf . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_jf . Tail , & _rtDW -> TransportDelay_IWORK_jf . Head ,
& _rtDW -> TransportDelay_IWORK_jf . Last , simTime - _rtP -> P_405 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_jf . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_jf . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_jf . Head ] = _rtB -> B_120_597_0 ; } if ( _rtDW ->
LastMajorTimeA_jf == ( rtInf ) ) { _rtDW -> LastMajorTimeA_jf = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_el = _rtB -> B_120_626_0 ; } else if ( _rtDW ->
LastMajorTimeB_fm == ( rtInf ) ) { _rtDW -> LastMajorTimeB_fm = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_nf = _rtB -> B_120_626_0 ; } else if ( _rtDW ->
LastMajorTimeA_jf < _rtDW -> LastMajorTimeB_fm ) { _rtDW -> LastMajorTimeA_jf
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_el = _rtB -> B_120_626_0 ; } else
{ _rtDW -> LastMajorTimeB_fm = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_nf =
_rtB -> B_120_626_0 ; } _rtDW -> vv0at_IWORK_cz = 0 ; { real_T * * uBuffer =
( real_T * * ) & _rtDW -> TransportDelay_PWORK_mn . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_mn .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_m3 . Head = ( ( _rtDW -> TransportDelay_IWORK_m3 . Head
< ( _rtDW -> TransportDelay_IWORK_m3 . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_m3 . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_m3 . Head == _rtDW -> TransportDelay_IWORK_m3 . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_m3 . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_m3 . Tail , & _rtDW -> TransportDelay_IWORK_m3 . Head ,
& _rtDW -> TransportDelay_IWORK_m3 . Last , simTime - _rtP -> P_419 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_m3 . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_m3 . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_m3 . Head ] = _rtB -> B_120_623_0 ; } if ( _rtDW ->
LastMajorTimeA_k == ( rtInf ) ) { _rtDW -> LastMajorTimeA_k = ssGetTaskTime (
S , 0 ) ; _rtDW -> PrevYA_kr = _rtB -> B_120_652_0 ; } else if ( _rtDW ->
LastMajorTimeB_b == ( rtInf ) ) { _rtDW -> LastMajorTimeB_b = ssGetTaskTime (
S , 0 ) ; _rtDW -> PrevYB_h = _rtB -> B_120_652_0 ; } else if ( _rtDW ->
LastMajorTimeA_k < _rtDW -> LastMajorTimeB_b ) { _rtDW -> LastMajorTimeA_k =
ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_kr = _rtB -> B_120_652_0 ; } else {
_rtDW -> LastMajorTimeB_b = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_h =
_rtB -> B_120_652_0 ; } _rtDW -> vv0at_IWORK_la = 0 ; { real_T * * uBuffer =
( real_T * * ) & _rtDW -> TransportDelay_PWORK_dz . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_dz .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_ga . Head = ( ( _rtDW -> TransportDelay_IWORK_ga . Head
< ( _rtDW -> TransportDelay_IWORK_ga . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_ga . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_ga . Head == _rtDW -> TransportDelay_IWORK_ga . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_ga . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_ga . Tail , & _rtDW -> TransportDelay_IWORK_ga . Head ,
& _rtDW -> TransportDelay_IWORK_ga . Last , simTime - _rtP -> P_433 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_ga . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_ga . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_ga . Head ] = _rtB -> B_120_649_0 ; } if ( _rtDW ->
LastMajorTimeA_io == ( rtInf ) ) { _rtDW -> LastMajorTimeA_io = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_d = _rtB -> B_120_678_0 ; } else if ( _rtDW ->
LastMajorTimeB_mi == ( rtInf ) ) { _rtDW -> LastMajorTimeB_mi = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_cl = _rtB -> B_120_678_0 ; } else if ( _rtDW ->
LastMajorTimeA_io < _rtDW -> LastMajorTimeB_mi ) { _rtDW -> LastMajorTimeA_io
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_d = _rtB -> B_120_678_0 ; } else
{ _rtDW -> LastMajorTimeB_mi = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_cl =
_rtB -> B_120_678_0 ; } _rtDW -> vv0at_IWORK_b2 = 0 ; { real_T * * uBuffer =
( real_T * * ) & _rtDW -> TransportDelay_PWORK_d3 . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_d3 .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_ii . Head = ( ( _rtDW -> TransportDelay_IWORK_ii . Head
< ( _rtDW -> TransportDelay_IWORK_ii . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_ii . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_ii . Head == _rtDW -> TransportDelay_IWORK_ii . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_ii . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_ii . Tail , & _rtDW -> TransportDelay_IWORK_ii . Head ,
& _rtDW -> TransportDelay_IWORK_ii . Last , simTime - _rtP -> P_447 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_ii . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_ii . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_ii . Head ] = _rtB -> B_120_675_0 ; } if ( _rtDW ->
LastMajorTimeA_c3 == ( rtInf ) ) { _rtDW -> LastMajorTimeA_c3 = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_mi = _rtB -> B_120_704_0 ; } else if ( _rtDW ->
LastMajorTimeB_h == ( rtInf ) ) { _rtDW -> LastMajorTimeB_h = ssGetTaskTime (
S , 0 ) ; _rtDW -> PrevYB_ib = _rtB -> B_120_704_0 ; } else if ( _rtDW ->
LastMajorTimeA_c3 < _rtDW -> LastMajorTimeB_h ) { _rtDW -> LastMajorTimeA_c3
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_mi = _rtB -> B_120_704_0 ; } else
{ _rtDW -> LastMajorTimeB_h = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_ib =
_rtB -> B_120_704_0 ; } _rtDW -> vv0at_IWORK_cx = 0 ; { real_T * * uBuffer =
( real_T * * ) & _rtDW -> TransportDelay_PWORK_mz . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_mz .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_fs . Head = ( ( _rtDW -> TransportDelay_IWORK_fs . Head
< ( _rtDW -> TransportDelay_IWORK_fs . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_fs . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_fs . Head == _rtDW -> TransportDelay_IWORK_fs . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_fs . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_fs . Tail , & _rtDW -> TransportDelay_IWORK_fs . Head ,
& _rtDW -> TransportDelay_IWORK_fs . Last , simTime - _rtP -> P_461 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_fs . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_fs . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_fs . Head ] = _rtB -> B_120_701_0 ; } if ( _rtDW ->
LastMajorTimeA_px == ( rtInf ) ) { _rtDW -> LastMajorTimeA_px = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_c = _rtB -> B_120_730_0 ; } else if ( _rtDW ->
LastMajorTimeB_fi == ( rtInf ) ) { _rtDW -> LastMajorTimeB_fi = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_j1 = _rtB -> B_120_730_0 ; } else if ( _rtDW ->
LastMajorTimeA_px < _rtDW -> LastMajorTimeB_fi ) { _rtDW -> LastMajorTimeA_px
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_c = _rtB -> B_120_730_0 ; } else
{ _rtDW -> LastMajorTimeB_fi = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_j1 =
_rtB -> B_120_730_0 ; } _rtDW -> vv0at_IWORK_lc = 0 ; { real_T * * uBuffer =
( real_T * * ) & _rtDW -> TransportDelay_PWORK_ix . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_ix .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_kn . Head = ( ( _rtDW -> TransportDelay_IWORK_kn . Head
< ( _rtDW -> TransportDelay_IWORK_kn . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_kn . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_kn . Head == _rtDW -> TransportDelay_IWORK_kn . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_kn . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_kn . Tail , & _rtDW -> TransportDelay_IWORK_kn . Head ,
& _rtDW -> TransportDelay_IWORK_kn . Last , simTime - _rtP -> P_475 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_kn . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_kn . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_kn . Head ] = _rtB -> B_120_727_0 ; } if ( _rtDW ->
LastMajorTimeA_e4x == ( rtInf ) ) { _rtDW -> LastMajorTimeA_e4x =
ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_df = _rtB -> B_120_756_0 ; } else
if ( _rtDW -> LastMajorTimeB_ex == ( rtInf ) ) { _rtDW -> LastMajorTimeB_ex =
ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_jy = _rtB -> B_120_756_0 ; } else
if ( _rtDW -> LastMajorTimeA_e4x < _rtDW -> LastMajorTimeB_ex ) { _rtDW ->
LastMajorTimeA_e4x = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_df = _rtB ->
B_120_756_0 ; } else { _rtDW -> LastMajorTimeB_ex = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYB_jy = _rtB -> B_120_756_0 ; } _rtDW -> vv0at_IWORK_oj = 0 ; {
real_T * * uBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_d0 .
TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_d0 . TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S )
; _rtDW -> TransportDelay_IWORK_lq . Head = ( ( _rtDW ->
TransportDelay_IWORK_lq . Head < ( _rtDW -> TransportDelay_IWORK_lq .
CircularBufSize - 1 ) ) ? ( _rtDW -> TransportDelay_IWORK_lq . Head + 1 ) : 0
) ; if ( _rtDW -> TransportDelay_IWORK_lq . Head == _rtDW ->
TransportDelay_IWORK_lq . Tail ) { if ( !
Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_lq . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_lq . Tail , & _rtDW -> TransportDelay_IWORK_lq . Head ,
& _rtDW -> TransportDelay_IWORK_lq . Last , simTime - _rtP -> P_489 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_lq . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_lq . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_lq . Head ] = _rtB -> B_120_753_0 ; } if ( _rtDW ->
LastMajorTimeA_jn == ( rtInf ) ) { _rtDW -> LastMajorTimeA_jn = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_or = _rtB -> B_120_782_0 ; } else if ( _rtDW ->
LastMajorTimeB_o == ( rtInf ) ) { _rtDW -> LastMajorTimeB_o = ssGetTaskTime (
S , 0 ) ; _rtDW -> PrevYB_hm = _rtB -> B_120_782_0 ; } else if ( _rtDW ->
LastMajorTimeA_jn < _rtDW -> LastMajorTimeB_o ) { _rtDW -> LastMajorTimeA_jn
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_or = _rtB -> B_120_782_0 ; } else
{ _rtDW -> LastMajorTimeB_o = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_hm =
_rtB -> B_120_782_0 ; } _rtDW -> vv0at_IWORK_od = 0 ; { real_T * * uBuffer =
( real_T * * ) & _rtDW -> TransportDelay_PWORK_n3 . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_n3 .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_d2 . Head = ( ( _rtDW -> TransportDelay_IWORK_d2 . Head
< ( _rtDW -> TransportDelay_IWORK_d2 . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_d2 . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_d2 . Head == _rtDW -> TransportDelay_IWORK_d2 . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_d2 . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_d2 . Tail , & _rtDW -> TransportDelay_IWORK_d2 . Head ,
& _rtDW -> TransportDelay_IWORK_d2 . Last , simTime - _rtP -> P_503 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_d2 . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_d2 . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_d2 . Head ] = _rtB -> B_120_779_0 ; } if ( _rtDW ->
LastMajorTimeA_o == ( rtInf ) ) { _rtDW -> LastMajorTimeA_o = ssGetTaskTime (
S , 0 ) ; _rtDW -> PrevYA_au = _rtB -> B_120_808_0 ; } else if ( _rtDW ->
LastMajorTimeB_ju == ( rtInf ) ) { _rtDW -> LastMajorTimeB_ju = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_a5 = _rtB -> B_120_808_0 ; } else if ( _rtDW ->
LastMajorTimeA_o < _rtDW -> LastMajorTimeB_ju ) { _rtDW -> LastMajorTimeA_o =
ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_au = _rtB -> B_120_808_0 ; } else {
_rtDW -> LastMajorTimeB_ju = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_a5 =
_rtB -> B_120_808_0 ; } _rtDW -> vv0at_IWORK_ff = 0 ; { real_T * * uBuffer =
( real_T * * ) & _rtDW -> TransportDelay_PWORK_gf . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_gf .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_c . Head = ( ( _rtDW -> TransportDelay_IWORK_c . Head <
( _rtDW -> TransportDelay_IWORK_c . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_c . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_c . Head == _rtDW -> TransportDelay_IWORK_c . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_c . CircularBufSize , & _rtDW -> TransportDelay_IWORK_c
. Tail , & _rtDW -> TransportDelay_IWORK_c . Head , & _rtDW ->
TransportDelay_IWORK_c . Last , simTime - _rtP -> P_517 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_c .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_c . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_c . Head ] = _rtB ->
B_120_805_0 ; } if ( _rtDW -> LastMajorTimeA_ev == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_ev = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_m3 = _rtB ->
B_120_834_0 ; } else if ( _rtDW -> LastMajorTimeB_h0 == ( rtInf ) ) { _rtDW
-> LastMajorTimeB_h0 = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_gn = _rtB ->
B_120_834_0 ; } else if ( _rtDW -> LastMajorTimeA_ev < _rtDW ->
LastMajorTimeB_h0 ) { _rtDW -> LastMajorTimeA_ev = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYA_m3 = _rtB -> B_120_834_0 ; } else { _rtDW ->
LastMajorTimeB_h0 = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_gn = _rtB ->
B_120_834_0 ; } _rtDW -> vv0at_IWORK_a3 = 0 ; { real_T * * uBuffer = ( real_T
* * ) & _rtDW -> TransportDelay_PWORK_mc . TUbufferPtrs [ 0 ] ; real_T * *
tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_mc . TUbufferPtrs [
1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_h . Head
= ( ( _rtDW -> TransportDelay_IWORK_h . Head < ( _rtDW ->
TransportDelay_IWORK_h . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_h . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_h . Head == _rtDW -> TransportDelay_IWORK_h . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_h . CircularBufSize , & _rtDW -> TransportDelay_IWORK_h
. Tail , & _rtDW -> TransportDelay_IWORK_h . Head , & _rtDW ->
TransportDelay_IWORK_h . Last , simTime - _rtP -> P_531 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_h .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_h . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_h . Head ] = _rtB ->
B_120_831_0 ; } if ( _rtDW -> LastMajorTimeA_bx == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_bx = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_l = _rtB ->
B_120_860_0 ; } else if ( _rtDW -> LastMajorTimeB_pt == ( rtInf ) ) { _rtDW
-> LastMajorTimeB_pt = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_dt = _rtB ->
B_120_860_0 ; } else if ( _rtDW -> LastMajorTimeA_bx < _rtDW ->
LastMajorTimeB_pt ) { _rtDW -> LastMajorTimeA_bx = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYA_l = _rtB -> B_120_860_0 ; } else { _rtDW -> LastMajorTimeB_pt
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_dt = _rtB -> B_120_860_0 ; }
_rtDW -> vv0at_IWORK_fk = 0 ; { real_T * * uBuffer = ( real_T * * ) & _rtDW
-> TransportDelay_PWORK_km . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_km . TUbufferPtrs [ 1 ] ; real_T
simTime = ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_p5 . Head = ( ( _rtDW
-> TransportDelay_IWORK_p5 . Head < ( _rtDW -> TransportDelay_IWORK_p5 .
CircularBufSize - 1 ) ) ? ( _rtDW -> TransportDelay_IWORK_p5 . Head + 1 ) : 0
) ; if ( _rtDW -> TransportDelay_IWORK_p5 . Head == _rtDW ->
TransportDelay_IWORK_p5 . Tail ) { if ( !
Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_p5 . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_p5 . Tail , & _rtDW -> TransportDelay_IWORK_p5 . Head ,
& _rtDW -> TransportDelay_IWORK_p5 . Last , simTime - _rtP -> P_545 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_p5 . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_p5 . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_p5 . Head ] = _rtB -> B_120_857_0 ; } if ( _rtDW ->
LastMajorTimeA_hd == ( rtInf ) ) { _rtDW -> LastMajorTimeA_hd = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_n = _rtB -> B_120_886_0 ; } else if ( _rtDW ->
LastMajorTimeB_ho == ( rtInf ) ) { _rtDW -> LastMajorTimeB_ho = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_n4 = _rtB -> B_120_886_0 ; } else if ( _rtDW ->
LastMajorTimeA_hd < _rtDW -> LastMajorTimeB_ho ) { _rtDW -> LastMajorTimeA_hd
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_n = _rtB -> B_120_886_0 ; } else
{ _rtDW -> LastMajorTimeB_ho = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_n4 =
_rtB -> B_120_886_0 ; } _rtDW -> vv0at_IWORK_dk = 0 ; { real_T * * uBuffer =
( real_T * * ) & _rtDW -> TransportDelay_PWORK_h . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_h .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_il . Head = ( ( _rtDW -> TransportDelay_IWORK_il . Head
< ( _rtDW -> TransportDelay_IWORK_il . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_il . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_il . Head == _rtDW -> TransportDelay_IWORK_il . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_il . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_il . Tail , & _rtDW -> TransportDelay_IWORK_il . Head ,
& _rtDW -> TransportDelay_IWORK_il . Last , simTime - _rtP -> P_559 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_il . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_il . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_il . Head ] = _rtB -> B_120_883_0 ; } if ( _rtDW ->
LastMajorTimeA_g == ( rtInf ) ) { _rtDW -> LastMajorTimeA_g = ssGetTaskTime (
S , 0 ) ; _rtDW -> PrevYA_pd = _rtB -> B_120_912_0 ; } else if ( _rtDW ->
LastMajorTimeB_b5 == ( rtInf ) ) { _rtDW -> LastMajorTimeB_b5 = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_gy = _rtB -> B_120_912_0 ; } else if ( _rtDW ->
LastMajorTimeA_g < _rtDW -> LastMajorTimeB_b5 ) { _rtDW -> LastMajorTimeA_g =
ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_pd = _rtB -> B_120_912_0 ; } else {
_rtDW -> LastMajorTimeB_b5 = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_gy =
_rtB -> B_120_912_0 ; } _rtDW -> vv0at_IWORK_lx = 0 ; { real_T * * uBuffer =
( real_T * * ) & _rtDW -> TransportDelay_PWORK_ln . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_ln .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_a4 . Head = ( ( _rtDW -> TransportDelay_IWORK_a4 . Head
< ( _rtDW -> TransportDelay_IWORK_a4 . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_a4 . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_a4 . Head == _rtDW -> TransportDelay_IWORK_a4 . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_a4 . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_a4 . Tail , & _rtDW -> TransportDelay_IWORK_a4 . Head ,
& _rtDW -> TransportDelay_IWORK_a4 . Last , simTime - _rtP -> P_573 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_a4 . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_a4 . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_a4 . Head ] = _rtB -> B_120_909_0 ; } if ( _rtDW ->
LastMajorTimeA_ec == ( rtInf ) ) { _rtDW -> LastMajorTimeA_ec = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_il = _rtB -> B_120_938_0 ; } else if ( _rtDW ->
LastMajorTimeB_i4 == ( rtInf ) ) { _rtDW -> LastMajorTimeB_i4 = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_am = _rtB -> B_120_938_0 ; } else if ( _rtDW ->
LastMajorTimeA_ec < _rtDW -> LastMajorTimeB_i4 ) { _rtDW -> LastMajorTimeA_ec
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_il = _rtB -> B_120_938_0 ; } else
{ _rtDW -> LastMajorTimeB_i4 = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_am =
_rtB -> B_120_938_0 ; } _rtDW -> vv0at_IWORK_ng = 0 ; { real_T * * uBuffer =
( real_T * * ) & _rtDW -> TransportDelay_PWORK_h5 . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_h5 .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_e . Head = ( ( _rtDW -> TransportDelay_IWORK_e . Head <
( _rtDW -> TransportDelay_IWORK_e . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_e . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_e . Head == _rtDW -> TransportDelay_IWORK_e . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_e . CircularBufSize , & _rtDW -> TransportDelay_IWORK_e
. Tail , & _rtDW -> TransportDelay_IWORK_e . Head , & _rtDW ->
TransportDelay_IWORK_e . Last , simTime - _rtP -> P_587 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_e .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_e . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_e . Head ] = _rtB ->
B_120_935_0 ; } if ( _rtDW -> LastMajorTimeA_lp == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_lp = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_ag = _rtB ->
B_120_964_0 ; } else if ( _rtDW -> LastMajorTimeB_jq == ( rtInf ) ) { _rtDW
-> LastMajorTimeB_jq = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_o = _rtB ->
B_120_964_0 ; } else if ( _rtDW -> LastMajorTimeA_lp < _rtDW ->
LastMajorTimeB_jq ) { _rtDW -> LastMajorTimeA_lp = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYA_ag = _rtB -> B_120_964_0 ; } else { _rtDW ->
LastMajorTimeB_jq = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_o = _rtB ->
B_120_964_0 ; } _rtDW -> vv0at_IWORK_hk = 0 ; { real_T * * uBuffer = ( real_T
* * ) & _rtDW -> TransportDelay_PWORK_b . TUbufferPtrs [ 0 ] ; real_T * *
tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_b . TUbufferPtrs [ 1
] ; real_T simTime = ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_m2 . Head =
( ( _rtDW -> TransportDelay_IWORK_m2 . Head < ( _rtDW ->
TransportDelay_IWORK_m2 . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_m2 . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_m2 . Head == _rtDW -> TransportDelay_IWORK_m2 . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_m2 . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_m2 . Tail , & _rtDW -> TransportDelay_IWORK_m2 . Head ,
& _rtDW -> TransportDelay_IWORK_m2 . Last , simTime - _rtP -> P_601 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_m2 . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_m2 . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_m2 . Head ] = _rtB -> B_120_961_0 ; } if ( _rtDW ->
LastMajorTimeA_ae == ( rtInf ) ) { _rtDW -> LastMajorTimeA_ae = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_at = _rtB -> B_120_990_0 ; } else if ( _rtDW ->
LastMajorTimeB_hb == ( rtInf ) ) { _rtDW -> LastMajorTimeB_hb = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_f3 = _rtB -> B_120_990_0 ; } else if ( _rtDW ->
LastMajorTimeA_ae < _rtDW -> LastMajorTimeB_hb ) { _rtDW -> LastMajorTimeA_ae
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_at = _rtB -> B_120_990_0 ; } else
{ _rtDW -> LastMajorTimeB_hb = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_f3 =
_rtB -> B_120_990_0 ; } _rtDW -> vv0at_IWORK_fv = 0 ; { real_T * * uBuffer =
( real_T * * ) & _rtDW -> TransportDelay_PWORK_jj . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_jj .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_l0 . Head = ( ( _rtDW -> TransportDelay_IWORK_l0 . Head
< ( _rtDW -> TransportDelay_IWORK_l0 . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_l0 . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_l0 . Head == _rtDW -> TransportDelay_IWORK_l0 . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_l0 . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_l0 . Tail , & _rtDW -> TransportDelay_IWORK_l0 . Head ,
& _rtDW -> TransportDelay_IWORK_l0 . Last , simTime - _rtP -> P_615 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_l0 . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_l0 . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_l0 . Head ] = _rtB -> B_120_987_0 ; } if ( _rtDW ->
LastMajorTimeA_cl == ( rtInf ) ) { _rtDW -> LastMajorTimeA_cl = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_jv = _rtB -> B_120_1016_0 ; } else if ( _rtDW ->
LastMajorTimeB_l3 == ( rtInf ) ) { _rtDW -> LastMajorTimeB_l3 = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_e = _rtB -> B_120_1016_0 ; } else if ( _rtDW ->
LastMajorTimeA_cl < _rtDW -> LastMajorTimeB_l3 ) { _rtDW -> LastMajorTimeA_cl
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_jv = _rtB -> B_120_1016_0 ; }
else { _rtDW -> LastMajorTimeB_l3 = ssGetTaskTime ( S , 0 ) ; _rtDW ->
PrevYB_e = _rtB -> B_120_1016_0 ; } _rtDW -> vv0at_IWORK_n1 = 0 ; { real_T *
* uBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_e . TUbufferPtrs [
0 ] ; real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_e .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_pc . Head = ( ( _rtDW -> TransportDelay_IWORK_pc . Head
< ( _rtDW -> TransportDelay_IWORK_pc . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_pc . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_pc . Head == _rtDW -> TransportDelay_IWORK_pc . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_pc . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_pc . Tail , & _rtDW -> TransportDelay_IWORK_pc . Head ,
& _rtDW -> TransportDelay_IWORK_pc . Last , simTime - _rtP -> P_629 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_pc . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_pc . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_pc . Head ] = _rtB -> B_120_1013_0 ; } if ( _rtDW ->
LastMajorTimeA_nb == ( rtInf ) ) { _rtDW -> LastMajorTimeA_nb = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_h1 = _rtB -> B_120_1042_0 ; } else if ( _rtDW ->
LastMajorTimeB_o2 == ( rtInf ) ) { _rtDW -> LastMajorTimeB_o2 = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_hs = _rtB -> B_120_1042_0 ; } else if ( _rtDW ->
LastMajorTimeA_nb < _rtDW -> LastMajorTimeB_o2 ) { _rtDW -> LastMajorTimeA_nb
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_h1 = _rtB -> B_120_1042_0 ; }
else { _rtDW -> LastMajorTimeB_o2 = ssGetTaskTime ( S , 0 ) ; _rtDW ->
PrevYB_hs = _rtB -> B_120_1042_0 ; } _rtDW -> vv0at_IWORK_j = 0 ; { real_T *
* uBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_ev . TUbufferPtrs
[ 0 ] ; real_T * * tBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_ev . TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S )
; _rtDW -> TransportDelay_IWORK_n . Head = ( ( _rtDW ->
TransportDelay_IWORK_n . Head < ( _rtDW -> TransportDelay_IWORK_n .
CircularBufSize - 1 ) ) ? ( _rtDW -> TransportDelay_IWORK_n . Head + 1 ) : 0
) ; if ( _rtDW -> TransportDelay_IWORK_n . Head == _rtDW ->
TransportDelay_IWORK_n . Tail ) { if ( !
Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_n . CircularBufSize , & _rtDW -> TransportDelay_IWORK_n
. Tail , & _rtDW -> TransportDelay_IWORK_n . Head , & _rtDW ->
TransportDelay_IWORK_n . Last , simTime - _rtP -> P_643 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_n .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_n . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_n . Head ] = _rtB ->
B_120_1039_0 ; } UNUSED_PARAMETER ( tid ) ; }
#define MDL_UPDATE
static void mdlUpdateTID3 ( SimStruct * S , int_T tid ) { UNUSED_PARAMETER (
tid ) ; }
#define MDL_DERIVATIVES
static void mdlDerivatives ( SimStruct * S ) { B_Simulation_v2_T * _rtB ;
P_Simulation_v2_T * _rtP ; X_Simulation_v2_T * _rtX ; XDot_Simulation_v2_T *
_rtXdot ; _rtXdot = ( ( XDot_Simulation_v2_T * ) ssGetdX ( S ) ) ; _rtX = ( (
X_Simulation_v2_T * ) ssGetContStates ( S ) ) ; _rtP = ( ( P_Simulation_v2_T
* ) ssGetModelRtp ( S ) ) ; _rtB = ( ( B_Simulation_v2_T * )
_ssGetModelBlockIO ( S ) ) ; _rtXdot -> Integrator_CSTATE = _rtB ->
B_120_1298_0 ; _rtXdot -> vv0at_CSTATE = _rtB -> B_120_1067_0 ; _rtXdot ->
TransferFcn_CSTATE = 0.0 ; _rtXdot -> TransferFcn_CSTATE += _rtP -> P_98 *
_rtX -> TransferFcn_CSTATE ; _rtXdot -> TransferFcn_CSTATE += _rtB ->
B_120_45_0 ; _rtXdot -> vv0at_CSTATE_k = _rtB -> B_120_1070_0 ; _rtXdot ->
TransferFcn_CSTATE_m = 0.0 ; _rtXdot -> TransferFcn_CSTATE_m += _rtP -> P_112
* _rtX -> TransferFcn_CSTATE_m ; _rtXdot -> TransferFcn_CSTATE_m += _rtB ->
B_120_71_0 ; _rtXdot -> vv0at_CSTATE_m = _rtB -> B_120_1073_0 ; _rtXdot ->
TransferFcn_CSTATE_h = 0.0 ; _rtXdot -> TransferFcn_CSTATE_h += _rtP -> P_126
* _rtX -> TransferFcn_CSTATE_h ; _rtXdot -> TransferFcn_CSTATE_h += _rtB ->
B_120_97_0 ; _rtXdot -> vv0at_CSTATE_e = _rtB -> B_120_1076_0 ; _rtXdot ->
TransferFcn_CSTATE_l = 0.0 ; _rtXdot -> TransferFcn_CSTATE_l += _rtP -> P_140
* _rtX -> TransferFcn_CSTATE_l ; _rtXdot -> TransferFcn_CSTATE_l += _rtB ->
B_120_123_0 ; _rtXdot -> vv0at_CSTATE_d = _rtB -> B_120_1079_0 ; _rtXdot ->
TransferFcn_CSTATE_k = 0.0 ; _rtXdot -> TransferFcn_CSTATE_k += _rtP -> P_154
* _rtX -> TransferFcn_CSTATE_k ; _rtXdot -> TransferFcn_CSTATE_k += _rtB ->
B_120_149_0 ; _rtXdot -> vv0at_CSTATE_h = _rtB -> B_120_1081_0 ; _rtXdot ->
TransferFcn_CSTATE_f = 0.0 ; _rtXdot -> TransferFcn_CSTATE_f += _rtP -> P_169
* _rtX -> TransferFcn_CSTATE_f ; _rtXdot -> TransferFcn_CSTATE_f += _rtB ->
B_120_178_0 ; _rtXdot -> vv0at_CSTATE_o = _rtB -> B_120_1084_0 ; _rtXdot ->
TransferFcn_CSTATE_ln = 0.0 ; _rtXdot -> TransferFcn_CSTATE_ln += _rtP ->
P_183 * _rtX -> TransferFcn_CSTATE_ln ; _rtXdot -> TransferFcn_CSTATE_ln +=
_rtB -> B_120_204_0 ; _rtXdot -> vv0at_CSTATE_dm = _rtB -> B_120_1087_0 ;
_rtXdot -> TransferFcn_CSTATE_p = 0.0 ; _rtXdot -> TransferFcn_CSTATE_p +=
_rtP -> P_197 * _rtX -> TransferFcn_CSTATE_p ; _rtXdot ->
TransferFcn_CSTATE_p += _rtB -> B_120_230_0 ; _rtXdot -> vv0at_CSTATE_e3 =
_rtB -> B_120_1090_0 ; _rtXdot -> TransferFcn_CSTATE_fx = 0.0 ; _rtXdot ->
TransferFcn_CSTATE_fx += _rtP -> P_211 * _rtX -> TransferFcn_CSTATE_fx ;
_rtXdot -> TransferFcn_CSTATE_fx += _rtB -> B_120_256_0 ; _rtXdot ->
vv0at_CSTATE_m2 = _rtB -> B_120_1093_0 ; _rtXdot -> TransferFcn_CSTATE_j =
0.0 ; _rtXdot -> TransferFcn_CSTATE_j += _rtP -> P_225 * _rtX ->
TransferFcn_CSTATE_j ; _rtXdot -> TransferFcn_CSTATE_j += _rtB -> B_120_282_0
; _rtXdot -> vv0at_CSTATE_f = _rtB -> B_120_1096_0 ; _rtXdot ->
TransferFcn_CSTATE_hm = 0.0 ; _rtXdot -> TransferFcn_CSTATE_hm += _rtP ->
P_239 * _rtX -> TransferFcn_CSTATE_hm ; _rtXdot -> TransferFcn_CSTATE_hm +=
_rtB -> B_120_308_0 ; _rtXdot -> vv0at_CSTATE_j = _rtB -> B_120_1099_0 ;
_rtXdot -> TransferFcn_CSTATE_mg = 0.0 ; _rtXdot -> TransferFcn_CSTATE_mg +=
_rtP -> P_253 * _rtX -> TransferFcn_CSTATE_mg ; _rtXdot ->
TransferFcn_CSTATE_mg += _rtB -> B_120_334_0 ; _rtXdot -> vv0at_CSTATE_g =
_rtB -> B_120_1102_0 ; _rtXdot -> TransferFcn_CSTATE_mf = 0.0 ; _rtXdot ->
TransferFcn_CSTATE_mf += _rtP -> P_267 * _rtX -> TransferFcn_CSTATE_mf ;
_rtXdot -> TransferFcn_CSTATE_mf += _rtB -> B_120_360_0 ; _rtXdot ->
vv0at_CSTATE_p = _rtB -> B_120_1105_0 ; _rtXdot -> TransferFcn_CSTATE_l3 =
0.0 ; _rtXdot -> TransferFcn_CSTATE_l3 += _rtP -> P_281 * _rtX ->
TransferFcn_CSTATE_l3 ; _rtXdot -> TransferFcn_CSTATE_l3 += _rtB ->
B_120_386_0 ; _rtXdot -> vv0at_CSTATE_h3 = _rtB -> B_120_1108_0 ; _rtXdot ->
TransferFcn_CSTATE_i = 0.0 ; _rtXdot -> TransferFcn_CSTATE_i += _rtP -> P_295
* _rtX -> TransferFcn_CSTATE_i ; _rtXdot -> TransferFcn_CSTATE_i += _rtB ->
B_120_412_0 ; _rtXdot -> vv0at_CSTATE_l = _rtB -> B_120_1111_0 ; _rtXdot ->
TransferFcn_CSTATE_c = 0.0 ; _rtXdot -> TransferFcn_CSTATE_c += _rtP -> P_309
* _rtX -> TransferFcn_CSTATE_c ; _rtXdot -> TransferFcn_CSTATE_c += _rtB ->
B_120_438_0 ; _rtXdot -> vv0at_CSTATE_fc = _rtB -> B_120_1114_0 ; _rtXdot ->
TransferFcn_CSTATE_b = 0.0 ; _rtXdot -> TransferFcn_CSTATE_b += _rtP -> P_323
* _rtX -> TransferFcn_CSTATE_b ; _rtXdot -> TransferFcn_CSTATE_b += _rtB ->
B_120_464_0 ; _rtXdot -> vv0at_CSTATE_a = _rtB -> B_120_1117_0 ; _rtXdot ->
TransferFcn_CSTATE_m1 = 0.0 ; _rtXdot -> TransferFcn_CSTATE_m1 += _rtP ->
P_337 * _rtX -> TransferFcn_CSTATE_m1 ; _rtXdot -> TransferFcn_CSTATE_m1 +=
_rtB -> B_120_490_0 ; _rtXdot -> vv0at_CSTATE_h3x = _rtB -> B_120_1120_0 ;
_rtXdot -> TransferFcn_CSTATE_g = 0.0 ; _rtXdot -> TransferFcn_CSTATE_g +=
_rtP -> P_351 * _rtX -> TransferFcn_CSTATE_g ; _rtXdot ->
TransferFcn_CSTATE_g += _rtB -> B_120_516_0 ; _rtXdot -> vv0at_CSTATE_ao =
_rtB -> B_120_1123_0 ; _rtXdot -> TransferFcn_CSTATE_n = 0.0 ; _rtXdot ->
TransferFcn_CSTATE_n += _rtP -> P_365 * _rtX -> TransferFcn_CSTATE_n ;
_rtXdot -> TransferFcn_CSTATE_n += _rtB -> B_120_542_0 ; _rtXdot ->
vv0at_CSTATE_ji = _rtB -> B_120_1126_0 ; _rtXdot -> TransferFcn_CSTATE_lb =
0.0 ; _rtXdot -> TransferFcn_CSTATE_lb += _rtP -> P_379 * _rtX ->
TransferFcn_CSTATE_lb ; _rtXdot -> TransferFcn_CSTATE_lb += _rtB ->
B_120_568_0 ; _rtXdot -> vv0at_CSTATE_ad = _rtB -> B_120_1129_0 ; _rtXdot ->
TransferFcn_CSTATE_m3 = 0.0 ; _rtXdot -> TransferFcn_CSTATE_m3 += _rtP ->
P_393 * _rtX -> TransferFcn_CSTATE_m3 ; _rtXdot -> TransferFcn_CSTATE_m3 +=
_rtB -> B_120_594_0 ; _rtXdot -> vv0at_CSTATE_l2 = _rtB -> B_120_1132_0 ;
_rtXdot -> TransferFcn_CSTATE_o = 0.0 ; _rtXdot -> TransferFcn_CSTATE_o +=
_rtP -> P_407 * _rtX -> TransferFcn_CSTATE_o ; _rtXdot ->
TransferFcn_CSTATE_o += _rtB -> B_120_620_0 ; _rtXdot -> vv0at_CSTATE_dj =
_rtB -> B_120_1135_0 ; _rtXdot -> TransferFcn_CSTATE_mv = 0.0 ; _rtXdot ->
TransferFcn_CSTATE_mv += _rtP -> P_421 * _rtX -> TransferFcn_CSTATE_mv ;
_rtXdot -> TransferFcn_CSTATE_mv += _rtB -> B_120_646_0 ; _rtXdot ->
vv0at_CSTATE_dv = _rtB -> B_120_1138_0 ; _rtXdot -> TransferFcn_CSTATE_gj =
0.0 ; _rtXdot -> TransferFcn_CSTATE_gj += _rtP -> P_435 * _rtX ->
TransferFcn_CSTATE_gj ; _rtXdot -> TransferFcn_CSTATE_gj += _rtB ->
B_120_672_0 ; _rtXdot -> vv0at_CSTATE_b = _rtB -> B_120_1141_0 ; _rtXdot ->
TransferFcn_CSTATE_e = 0.0 ; _rtXdot -> TransferFcn_CSTATE_e += _rtP -> P_449
* _rtX -> TransferFcn_CSTATE_e ; _rtXdot -> TransferFcn_CSTATE_e += _rtB ->
B_120_698_0 ; _rtXdot -> vv0at_CSTATE_ady = _rtB -> B_120_1144_0 ; _rtXdot ->
TransferFcn_CSTATE_cm = 0.0 ; _rtXdot -> TransferFcn_CSTATE_cm += _rtP ->
P_463 * _rtX -> TransferFcn_CSTATE_cm ; _rtXdot -> TransferFcn_CSTATE_cm +=
_rtB -> B_120_724_0 ; _rtXdot -> vv0at_CSTATE_i = _rtB -> B_120_1147_0 ;
_rtXdot -> TransferFcn_CSTATE_a = 0.0 ; _rtXdot -> TransferFcn_CSTATE_a +=
_rtP -> P_477 * _rtX -> TransferFcn_CSTATE_a ; _rtXdot ->
TransferFcn_CSTATE_a += _rtB -> B_120_750_0 ; _rtXdot -> vv0at_CSTATE_pm =
_rtB -> B_120_1150_0 ; _rtXdot -> TransferFcn_CSTATE_o0 = 0.0 ; _rtXdot ->
TransferFcn_CSTATE_o0 += _rtP -> P_491 * _rtX -> TransferFcn_CSTATE_o0 ;
_rtXdot -> TransferFcn_CSTATE_o0 += _rtB -> B_120_776_0 ; _rtXdot ->
vv0at_CSTATE_gb = _rtB -> B_120_1153_0 ; _rtXdot -> TransferFcn_CSTATE_ap =
0.0 ; _rtXdot -> TransferFcn_CSTATE_ap += _rtP -> P_505 * _rtX ->
TransferFcn_CSTATE_ap ; _rtXdot -> TransferFcn_CSTATE_ap += _rtB ->
B_120_802_0 ; _rtXdot -> vv0at_CSTATE_as = _rtB -> B_120_1156_0 ; _rtXdot ->
TransferFcn_CSTATE_fc = 0.0 ; _rtXdot -> TransferFcn_CSTATE_fc += _rtP ->
P_519 * _rtX -> TransferFcn_CSTATE_fc ; _rtXdot -> TransferFcn_CSTATE_fc +=
_rtB -> B_120_828_0 ; _rtXdot -> vv0at_CSTATE_jy = _rtB -> B_120_1159_0 ;
_rtXdot -> TransferFcn_CSTATE_hl = 0.0 ; _rtXdot -> TransferFcn_CSTATE_hl +=
_rtP -> P_533 * _rtX -> TransferFcn_CSTATE_hl ; _rtXdot ->
TransferFcn_CSTATE_hl += _rtB -> B_120_854_0 ; _rtXdot -> vv0at_CSTATE_pa =
_rtB -> B_120_1162_0 ; _rtXdot -> TransferFcn_CSTATE_bw = 0.0 ; _rtXdot ->
TransferFcn_CSTATE_bw += _rtP -> P_547 * _rtX -> TransferFcn_CSTATE_bw ;
_rtXdot -> TransferFcn_CSTATE_bw += _rtB -> B_120_880_0 ; _rtXdot ->
vv0at_CSTATE_gg = _rtB -> B_120_1165_0 ; _rtXdot -> TransferFcn_CSTATE_d =
0.0 ; _rtXdot -> TransferFcn_CSTATE_d += _rtP -> P_561 * _rtX ->
TransferFcn_CSTATE_d ; _rtXdot -> TransferFcn_CSTATE_d += _rtB -> B_120_906_0
; _rtXdot -> vv0at_CSTATE_o5 = _rtB -> B_120_1168_0 ; _rtXdot ->
TransferFcn_CSTATE_h2 = 0.0 ; _rtXdot -> TransferFcn_CSTATE_h2 += _rtP ->
P_575 * _rtX -> TransferFcn_CSTATE_h2 ; _rtXdot -> TransferFcn_CSTATE_h2 +=
_rtB -> B_120_932_0 ; _rtXdot -> vv0at_CSTATE_hm = _rtB -> B_120_1171_0 ;
_rtXdot -> TransferFcn_CSTATE_n1 = 0.0 ; _rtXdot -> TransferFcn_CSTATE_n1 +=
_rtP -> P_589 * _rtX -> TransferFcn_CSTATE_n1 ; _rtXdot ->
TransferFcn_CSTATE_n1 += _rtB -> B_120_958_0 ; _rtXdot -> vv0at_CSTATE_fm =
_rtB -> B_120_1174_0 ; _rtXdot -> TransferFcn_CSTATE_eg = 0.0 ; _rtXdot ->
TransferFcn_CSTATE_eg += _rtP -> P_603 * _rtX -> TransferFcn_CSTATE_eg ;
_rtXdot -> TransferFcn_CSTATE_eg += _rtB -> B_120_984_0 ; _rtXdot ->
vv0at_CSTATE_n = _rtB -> B_120_1177_0 ; _rtXdot -> TransferFcn_CSTATE_aq =
0.0 ; _rtXdot -> TransferFcn_CSTATE_aq += _rtP -> P_617 * _rtX ->
TransferFcn_CSTATE_aq ; _rtXdot -> TransferFcn_CSTATE_aq += _rtB ->
B_120_1010_0 ; _rtXdot -> vv0at_CSTATE_mn = _rtB -> B_120_1180_0 ; _rtXdot ->
TransferFcn_CSTATE_pj = 0.0 ; _rtXdot -> TransferFcn_CSTATE_pj += _rtP ->
P_631 * _rtX -> TransferFcn_CSTATE_pj ; _rtXdot -> TransferFcn_CSTATE_pj +=
_rtB -> B_120_1036_0 ; _rtXdot -> vv0at_CSTATE_lr = _rtB -> B_120_1183_0 ;
_rtXdot -> TransferFcn_CSTATE_bm = 0.0 ; _rtXdot -> TransferFcn_CSTATE_bm +=
_rtP -> P_645 * _rtX -> TransferFcn_CSTATE_bm ; _rtXdot ->
TransferFcn_CSTATE_bm += _rtB -> B_120_1060_0 ; _rtXdot -> Integrator1_CSTATE
= _rtB -> B_120_1_0 ; } static void mdlInitializeSizes ( SimStruct * S ) {
ssSetChecksumVal ( S , 0 , 1953800735U ) ; ssSetChecksumVal ( S , 1 ,
349321728U ) ; ssSetChecksumVal ( S , 2 , 1630960371U ) ; ssSetChecksumVal (
S , 3 , 2113207431U ) ; { mxArray * slVerStructMat = NULL ; mxArray *
slStrMat = mxCreateString ( "simulink" ) ; char slVerChar [ 10 ] ; int status
= mexCallMATLAB ( 1 , & slVerStructMat , 1 , & slStrMat , "ver" ) ; if (
status == 0 ) { mxArray * slVerMat = mxGetField ( slVerStructMat , 0 ,
"Version" ) ; if ( slVerMat == NULL ) { status = 1 ; } else { status =
mxGetString ( slVerMat , slVerChar , 10 ) ; } } mxDestroyArray ( slStrMat ) ;
mxDestroyArray ( slVerStructMat ) ; if ( ( status == 1 ) || ( strcmp (
slVerChar , "10.1" ) != 0 ) ) { return ; } } ssSetOptions ( S ,
SS_OPTION_EXCEPTION_FREE_CODE ) ; if ( ssGetSizeofDWork ( S ) != sizeof (
DW_Simulation_v2_T ) ) { ssSetErrorStatus ( S ,
"Unexpected error: Internal DWork sizes do "
"not match for accelerator mex file." ) ; } if ( ssGetSizeofGlobalBlockIO ( S
) != sizeof ( B_Simulation_v2_T ) ) { ssSetErrorStatus ( S ,
"Unexpected error: Internal BlockIO sizes do "
"not match for accelerator mex file." ) ; } { int ssSizeofParams ;
ssGetSizeofParams ( S , & ssSizeofParams ) ; if ( ssSizeofParams != sizeof (
P_Simulation_v2_T ) ) { static char msg [ 256 ] ; sprintf ( msg ,
"Unexpected error: Internal Parameters sizes do "
"not match for accelerator mex file." ) ; } } _ssSetModelRtp ( S , ( real_T *
) & Simulation_v2_rtDefaultP ) ; _ssSetConstBlockIO ( S , &
Simulation_v2_rtInvariant ) ; rt_InitInfAndNaN ( sizeof ( real_T ) ) ; }
static void mdlInitializeSampleTimes ( SimStruct * S ) { slAccRegPrmChangeFcn
( S , mdlOutputsTID3 ) ; } static void mdlTerminate ( SimStruct * S ) { }
#include "simulink.c"
