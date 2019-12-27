#include "__cf_Simulation_v2.h"
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
rateLimiterRate ; real_T riseValLimit ; boolean_T rtb_B_40_2_0 ; real_T
rtb_B_40_3_0 ; int32_T isHit ; B_Simulation_v2_T * _rtB ; P_Simulation_v2_T *
_rtP ; X_Simulation_v2_T * _rtX ; DW_Simulation_v2_T * _rtDW ; _rtDW = ( (
DW_Simulation_v2_T * ) ssGetRootDWork ( S ) ) ; _rtX = ( ( X_Simulation_v2_T
* ) ssGetContStates ( S ) ) ; _rtP = ( ( P_Simulation_v2_T * ) ssGetModelRtp
( S ) ) ; _rtB = ( ( B_Simulation_v2_T * ) _ssGetModelBlockIO ( S ) ) ;
ssCallAccelRunBlock ( S , 40 , 0 , SS_CALL_MDL_OUTPUTS ) ; isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB -> B_40_1_0 = _rtDW ->
DelayInput1_DSTATE ; } rtb_B_40_2_0 = ( _rtB -> B_40_0_0 < _rtB -> B_40_1_0 )
; rtb_B_40_3_0 = _rtB -> B_40_0_0 * ( real_T ) rtb_B_40_2_0 ;
vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Calculation of braking pressure/Data Store Read" , 0 ) ; _rtB
-> B_40_7_0 = ( real_T ) ! rtb_B_40_2_0 * _rtDW -> v0 + rtb_B_40_3_0 ; _rtDW
-> v0 = _rtB -> B_40_7_0 ; vm_WriteLocalDSMNoIdx ( S , _rtDW -> dsmIdx , (
char_T * ) "Simulation_v2/Calculation of braking pressure/Data Store Write" ,
0 ) ; ssCallAccelRunBlock ( S , 40 , 9 , SS_CALL_MDL_OUTPUTS ) ;
vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Subsystem/Data Store Read" , 0 ) ; _rtB -> B_40_10_0 = _rtDW
-> v0 ; if ( _rtDW -> Integrator_IWORK != 0 ) { _rtX -> Integrator_CSTATE =
_rtB -> B_40_10_0 ; } _rtB -> B_40_11_0 = _rtX -> Integrator_CSTATE ; _rtB ->
B_40_12_0 = _rtB -> B_40_11_0 - _rtB -> B_40_0_0 ; ssCallAccelRunBlock ( S ,
40 , 13 , SS_CALL_MDL_OUTPUTS ) ; _rtB -> B_40_14_0 = look1_binlxpw ( _rtB ->
B_40_12_0 , _rtP -> P_4 , _rtP -> P_3 , 7U ) ; ssCallAccelRunBlock ( S , 40 ,
15 , SS_CALL_MDL_OUTPUTS ) ; _rtB -> B_40_17_0 = ( real_T ) ( _rtB ->
B_40_11_0 > _rtB -> B_40_0_0 ) * _rtB -> B_40_14_0 ; _rtB -> B_40_18_0 = _rtB
-> B_40_0_0_m - _rtB -> B_40_17_0 ; _rtB -> B_40_19_0 = _rtB -> B_40_18_0 +
_rtP -> P_5 ; _rtB -> B_40_20_0 = _rtP -> P_6 * _rtB -> B_40_19_0 ; if ( _rtB
-> B_40_20_0 > _rtP -> P_7 ) { _rtB -> B_40_21_0 = _rtP -> P_7 ; } else if (
_rtB -> B_40_20_0 < _rtP -> P_8 ) { _rtB -> B_40_21_0 = _rtP -> P_8 ; } else
{ _rtB -> B_40_21_0 = _rtB -> B_40_20_0 ; } if ( ( _rtDW -> LastMajorTimeA >=
ssGetTaskTime ( S , 0 ) ) && ( _rtDW -> LastMajorTimeB >= ssGetTaskTime ( S ,
0 ) ) ) { _rtB -> B_40_22_0 = _rtB -> B_40_21_0 ; } else { if ( ( ( _rtDW ->
LastMajorTimeA < _rtDW -> LastMajorTimeB ) && ( _rtDW -> LastMajorTimeB <
ssGetTaskTime ( S , 0 ) ) ) || ( ( _rtDW -> LastMajorTimeA >= _rtDW ->
LastMajorTimeB ) && ( _rtDW -> LastMajorTimeA >= ssGetTaskTime ( S , 0 ) ) )
) { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB ; rtb_B_40_3_0
= _rtDW -> PrevYB ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA ; rtb_B_40_3_0 = _rtDW -> PrevYA ; } riseValLimit = deltaT *
_rtP -> P_9 ; rateLimiterRate = _rtB -> B_40_21_0 - rtb_B_40_3_0 ; if (
rateLimiterRate > riseValLimit ) { _rtB -> B_40_22_0 = rtb_B_40_3_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_10 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_40_22_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB ->
B_40_22_0 = _rtB -> B_40_21_0 ; } } } _rtB -> B_40_23_0 = _rtP -> P_11 * _rtB
-> B_40_22_0 ; _rtB -> B_40_24_0 = _rtP -> P_12 * _rtB -> B_40_23_0 ;
vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem/Wagon model/Data Store Read" , 0
) ; _rtB -> B_40_25_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK != 0 ) { _rtX
-> vv0at_CSTATE = _rtB -> B_40_25_0 ; } _rtB -> B_40_26_0 = _rtX ->
vv0at_CSTATE ; _rtB -> B_40_27_0 = muDoubleScalarAbs ( _rtB -> B_40_26_0 ) ;
_rtB -> B_40_28_0 = _rtP -> P_13 [ 0 ] ; _rtB -> B_40_28_0 = _rtB ->
B_40_28_0 * _rtB -> B_40_27_0 + _rtP -> P_13 [ 1 ] ; _rtB -> B_40_28_0 = _rtB
-> B_40_28_0 * _rtB -> B_40_27_0 + _rtP -> P_13 [ 2 ] ; _rtB -> B_40_29_0 =
_rtP -> P_14 * _rtB -> B_40_28_0 ; _rtB -> B_40_30_0 = _rtB -> B_40_2_0 *
_rtB -> B_40_29_0 ; _rtB -> B_40_31_0 = _rtB -> B_40_24_0 - _rtB -> B_40_30_0
; if ( _rtB -> B_40_31_0 > _rtB -> B_40_52_0_j ) { _rtB -> B_40_34_0 = _rtB
-> B_40_52_0_j ; } else { if ( _rtB -> B_40_31_0 < _rtB -> B_40_53_0_f ) {
_rtB -> B_0_1_0 = _rtB -> B_40_53_0_f ; } else { _rtB -> B_0_1_0 = _rtB ->
B_40_31_0 ; } _rtB -> B_40_34_0 = _rtB -> B_0_1_0 ; } { real_T * * uBuffer =
( real_T * * ) & _rtDW -> TransportDelay_PWORK . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK . TUbufferPtrs [
1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay = simTime - _rtP ->
P_15 ; _rtB -> B_40_35_0 = Simulation_v2_acc_rt_TDelayInterpolate (
tMinusDelay , 0.0 , * tBuffer , * uBuffer , _rtDW -> TransportDelay_IWORK .
CircularBufSize , & _rtDW -> TransportDelay_IWORK . Last , _rtDW ->
TransportDelay_IWORK . Tail , _rtDW -> TransportDelay_IWORK . Head , _rtP ->
P_16 , 0 , ( boolean_T ) ( ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput
( S ) == ssGetT ( S ) ) ) ) ; } _rtB -> B_40_36_0 = 0.0 ; _rtB -> B_40_36_0
+= _rtP -> P_18 * _rtX -> TransferFcn_CSTATE ; _rtB -> B_40_36_0 += _rtP ->
P_19 * _rtB -> B_40_35_0 ; _rtB -> B_40_37_0 = _rtP -> P_20 * _rtB ->
B_40_36_0 ; if ( _rtB -> B_40_37_0 > _rtP -> P_21 ) { _rtB -> B_40_38_0 =
_rtP -> P_21 ; } else if ( _rtB -> B_40_37_0 < _rtP -> P_22 ) { _rtB ->
B_40_38_0 = _rtP -> P_22 ; } else { _rtB -> B_40_38_0 = _rtB -> B_40_37_0 ; }
if ( ( _rtDW -> LastMajorTimeA_m >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW ->
LastMajorTimeB_a >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_40_39_0 = _rtB ->
B_40_38_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_m < _rtDW ->
LastMajorTimeB_a ) && ( _rtDW -> LastMajorTimeB_a < ssGetTaskTime ( S , 0 ) )
) || ( ( _rtDW -> LastMajorTimeA_m >= _rtDW -> LastMajorTimeB_a ) && ( _rtDW
-> LastMajorTimeA_m >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime
( S , 0 ) - _rtDW -> LastMajorTimeB_a ; rtb_B_40_3_0 = _rtDW -> PrevYB_f ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_m ;
rtb_B_40_3_0 = _rtDW -> PrevYA_d ; } riseValLimit = deltaT * _rtP -> P_23 ;
rateLimiterRate = _rtB -> B_40_38_0 - rtb_B_40_3_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_40_39_0 = rtb_B_40_3_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_24 ; if ( rateLimiterRate < deltaT ) { _rtB -> B_40_39_0
= rtb_B_40_3_0 + deltaT ; } else { _rtB -> B_40_39_0 = _rtB -> B_40_38_0 ; }
} } _rtB -> B_40_40_0 = _rtP -> P_25 * _rtB -> B_40_39_0 ; _rtB -> B_40_41_0
= _rtP -> P_26 * _rtB -> B_40_40_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW ->
dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem/Wagon model1/Data Store Read" , 0
) ; _rtB -> B_40_42_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_o != 0 ) {
_rtX -> vv0at_CSTATE_k = _rtB -> B_40_42_0 ; } _rtB -> B_40_43_0 = _rtX ->
vv0at_CSTATE_k ; _rtB -> B_40_44_0 = muDoubleScalarAbs ( _rtB -> B_40_43_0 )
; _rtB -> B_40_45_0 = _rtP -> P_27 [ 0 ] ; _rtB -> B_40_45_0 = _rtB ->
B_40_45_0 * _rtB -> B_40_44_0 + _rtP -> P_27 [ 1 ] ; _rtB -> B_40_45_0 = _rtB
-> B_40_45_0 * _rtB -> B_40_44_0 + _rtP -> P_27 [ 2 ] ; _rtB -> B_40_46_0 =
_rtP -> P_28 * _rtB -> B_40_45_0 ; _rtB -> B_40_47_0 = _rtB -> B_40_3_0 *
_rtB -> B_40_46_0 ; _rtB -> B_40_48_0 = _rtB -> B_40_41_0 - _rtB -> B_40_47_0
; if ( _rtB -> B_40_48_0 > _rtB -> B_40_55_0_a ) { _rtB -> B_40_51_0 = _rtB
-> B_40_55_0_a ; } else { if ( _rtB -> B_40_48_0 < _rtB -> B_40_56_0_j ) {
_rtB -> B_1_1_0 = _rtB -> B_40_56_0_j ; } else { _rtB -> B_1_1_0 = _rtB ->
B_40_48_0 ; } _rtB -> B_40_51_0 = _rtB -> B_1_1_0 ; } { real_T * * uBuffer =
( real_T * * ) & _rtDW -> TransportDelay_PWORK_d . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_d .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_29 ; _rtB -> B_40_52_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_k . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_k . Last , _rtDW -> TransportDelay_IWORK_k . Tail ,
_rtDW -> TransportDelay_IWORK_k . Head , _rtP -> P_30 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_40_53_0 = 0.0 ; _rtB -> B_40_53_0 += _rtP -> P_32 * _rtX ->
TransferFcn_CSTATE_c ; _rtB -> B_40_53_0 += _rtP -> P_33 * _rtB -> B_40_52_0
; _rtB -> B_40_54_0 = _rtP -> P_34 * _rtB -> B_40_53_0 ; if ( _rtB ->
B_40_54_0 > _rtP -> P_35 ) { _rtB -> B_40_55_0 = _rtP -> P_35 ; } else if (
_rtB -> B_40_54_0 < _rtP -> P_36 ) { _rtB -> B_40_55_0 = _rtP -> P_36 ; }
else { _rtB -> B_40_55_0 = _rtB -> B_40_54_0 ; } if ( ( _rtDW ->
LastMajorTimeA_g >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW -> LastMajorTimeB_h
>= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_40_56_0 = _rtB -> B_40_55_0 ; }
else { if ( ( ( _rtDW -> LastMajorTimeA_g < _rtDW -> LastMajorTimeB_h ) && (
_rtDW -> LastMajorTimeB_h < ssGetTaskTime ( S , 0 ) ) ) || ( ( _rtDW ->
LastMajorTimeA_g >= _rtDW -> LastMajorTimeB_h ) && ( _rtDW ->
LastMajorTimeA_g >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime (
S , 0 ) - _rtDW -> LastMajorTimeB_h ; rtb_B_40_3_0 = _rtDW -> PrevYB_g ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_g ;
rtb_B_40_3_0 = _rtDW -> PrevYA_e ; } riseValLimit = deltaT * _rtP -> P_37 ;
rateLimiterRate = _rtB -> B_40_55_0 - rtb_B_40_3_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_40_56_0 = rtb_B_40_3_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_38 ; if ( rateLimiterRate < deltaT ) { _rtB -> B_40_56_0
= rtb_B_40_3_0 + deltaT ; } else { _rtB -> B_40_56_0 = _rtB -> B_40_55_0 ; }
} } _rtB -> B_40_57_0 = _rtP -> P_39 * _rtB -> B_40_56_0 ; _rtB -> B_40_58_0
= _rtP -> P_40 * _rtB -> B_40_57_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW ->
dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem/Wagon model2/Data Store Read" , 0
) ; _rtB -> B_40_59_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_l != 0 ) {
_rtX -> vv0at_CSTATE_o = _rtB -> B_40_59_0 ; } _rtB -> B_40_60_0 = _rtX ->
vv0at_CSTATE_o ; _rtB -> B_40_61_0 = muDoubleScalarAbs ( _rtB -> B_40_60_0 )
; _rtB -> B_40_62_0 = _rtP -> P_41 [ 0 ] ; _rtB -> B_40_62_0 = _rtB ->
B_40_62_0 * _rtB -> B_40_61_0 + _rtP -> P_41 [ 1 ] ; _rtB -> B_40_62_0 = _rtB
-> B_40_62_0 * _rtB -> B_40_61_0 + _rtP -> P_41 [ 2 ] ; _rtB -> B_40_63_0 =
_rtP -> P_42 * _rtB -> B_40_62_0 ; _rtB -> B_40_64_0 = _rtB -> B_40_4_0 *
_rtB -> B_40_63_0 ; _rtB -> B_40_65_0 = _rtB -> B_40_58_0 - _rtB -> B_40_64_0
; if ( _rtB -> B_40_65_0 > _rtB -> B_40_58_0_j ) { _rtB -> B_40_68_0 = _rtB
-> B_40_58_0_j ; } else { if ( _rtB -> B_40_65_0 < _rtB -> B_40_59_0_o ) {
_rtB -> B_2_1_0 = _rtB -> B_40_59_0_o ; } else { _rtB -> B_2_1_0 = _rtB ->
B_40_65_0 ; } _rtB -> B_40_68_0 = _rtB -> B_2_1_0 ; } { real_T * * uBuffer =
( real_T * * ) & _rtDW -> TransportDelay_PWORK_m . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_m .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_43 ; _rtB -> B_40_69_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_b . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_b . Last , _rtDW -> TransportDelay_IWORK_b . Tail ,
_rtDW -> TransportDelay_IWORK_b . Head , _rtP -> P_44 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_40_70_0 = 0.0 ; _rtB -> B_40_70_0 += _rtP -> P_46 * _rtX ->
TransferFcn_CSTATE_n ; _rtB -> B_40_70_0 += _rtP -> P_47 * _rtB -> B_40_69_0
; _rtB -> B_40_71_0 = _rtP -> P_48 * _rtB -> B_40_70_0 ; if ( _rtB ->
B_40_71_0 > _rtP -> P_49 ) { _rtB -> B_40_72_0 = _rtP -> P_49 ; } else if (
_rtB -> B_40_71_0 < _rtP -> P_50 ) { _rtB -> B_40_72_0 = _rtP -> P_50 ; }
else { _rtB -> B_40_72_0 = _rtB -> B_40_71_0 ; } if ( ( _rtDW ->
LastMajorTimeA_i >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW -> LastMajorTimeB_m
>= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_40_73_0 = _rtB -> B_40_72_0 ; }
else { if ( ( ( _rtDW -> LastMajorTimeA_i < _rtDW -> LastMajorTimeB_m ) && (
_rtDW -> LastMajorTimeB_m < ssGetTaskTime ( S , 0 ) ) ) || ( ( _rtDW ->
LastMajorTimeA_i >= _rtDW -> LastMajorTimeB_m ) && ( _rtDW ->
LastMajorTimeA_i >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime (
S , 0 ) - _rtDW -> LastMajorTimeB_m ; rtb_B_40_3_0 = _rtDW -> PrevYB_c ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_i ;
rtb_B_40_3_0 = _rtDW -> PrevYA_e4 ; } riseValLimit = deltaT * _rtP -> P_51 ;
rateLimiterRate = _rtB -> B_40_72_0 - rtb_B_40_3_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_40_73_0 = rtb_B_40_3_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_52 ; if ( rateLimiterRate < deltaT ) { _rtB -> B_40_73_0
= rtb_B_40_3_0 + deltaT ; } else { _rtB -> B_40_73_0 = _rtB -> B_40_72_0 ; }
} } _rtB -> B_40_74_0 = _rtP -> P_53 * _rtB -> B_40_73_0 ; _rtB -> B_40_75_0
= _rtP -> P_54 * _rtB -> B_40_74_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW ->
dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem/Wagon model3/Data Store Read" , 0
) ; _rtB -> B_40_76_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_h != 0 ) {
_rtX -> vv0at_CSTATE_h = _rtB -> B_40_76_0 ; } _rtB -> B_40_77_0 = _rtX ->
vv0at_CSTATE_h ; _rtB -> B_40_78_0 = muDoubleScalarAbs ( _rtB -> B_40_77_0 )
; _rtB -> B_40_79_0 = _rtP -> P_55 [ 0 ] ; _rtB -> B_40_79_0 = _rtB ->
B_40_79_0 * _rtB -> B_40_78_0 + _rtP -> P_55 [ 1 ] ; _rtB -> B_40_79_0 = _rtB
-> B_40_79_0 * _rtB -> B_40_78_0 + _rtP -> P_55 [ 2 ] ; _rtB -> B_40_80_0 =
_rtP -> P_56 * _rtB -> B_40_79_0 ; _rtB -> B_40_81_0 = _rtB -> B_40_5_0 *
_rtB -> B_40_80_0 ; _rtB -> B_40_82_0 = _rtB -> B_40_75_0 - _rtB -> B_40_81_0
; if ( _rtB -> B_40_82_0 > _rtB -> B_40_61_0_n ) { _rtB -> B_40_85_0 = _rtB
-> B_40_61_0_n ; } else { if ( _rtB -> B_40_82_0 < _rtB -> B_40_62_0_i ) {
_rtB -> B_3_1_0 = _rtB -> B_40_62_0_i ; } else { _rtB -> B_3_1_0 = _rtB ->
B_40_82_0 ; } _rtB -> B_40_85_0 = _rtB -> B_3_1_0 ; } { real_T * * uBuffer =
( real_T * * ) & _rtDW -> TransportDelay_PWORK_a . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_a .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_57 ; _rtB -> B_40_86_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_bc . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_bc . Last , _rtDW -> TransportDelay_IWORK_bc . Tail ,
_rtDW -> TransportDelay_IWORK_bc . Head , _rtP -> P_58 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_40_87_0 = 0.0 ; _rtB -> B_40_87_0 += _rtP -> P_60 * _rtX ->
TransferFcn_CSTATE_b ; _rtB -> B_40_87_0 += _rtP -> P_61 * _rtB -> B_40_86_0
; _rtB -> B_40_88_0 = _rtP -> P_62 * _rtB -> B_40_87_0 ; if ( _rtB ->
B_40_88_0 > _rtP -> P_63 ) { _rtB -> B_40_89_0 = _rtP -> P_63 ; } else if (
_rtB -> B_40_88_0 < _rtP -> P_64 ) { _rtB -> B_40_89_0 = _rtP -> P_64 ; }
else { _rtB -> B_40_89_0 = _rtB -> B_40_88_0 ; } if ( ( _rtDW ->
LastMajorTimeA_j >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW -> LastMajorTimeB_c
>= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_40_90_0 = _rtB -> B_40_89_0 ; }
else { if ( ( ( _rtDW -> LastMajorTimeA_j < _rtDW -> LastMajorTimeB_c ) && (
_rtDW -> LastMajorTimeB_c < ssGetTaskTime ( S , 0 ) ) ) || ( ( _rtDW ->
LastMajorTimeA_j >= _rtDW -> LastMajorTimeB_c ) && ( _rtDW ->
LastMajorTimeA_j >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime (
S , 0 ) - _rtDW -> LastMajorTimeB_c ; rtb_B_40_3_0 = _rtDW -> PrevYB_h ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_j ;
rtb_B_40_3_0 = _rtDW -> PrevYA_n ; } riseValLimit = deltaT * _rtP -> P_65 ;
rateLimiterRate = _rtB -> B_40_89_0 - rtb_B_40_3_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_40_90_0 = rtb_B_40_3_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_66 ; if ( rateLimiterRate < deltaT ) { _rtB -> B_40_90_0
= rtb_B_40_3_0 + deltaT ; } else { _rtB -> B_40_90_0 = _rtB -> B_40_89_0 ; }
} } _rtB -> B_40_91_0 = _rtP -> P_67 * _rtB -> B_40_90_0 ; _rtB -> B_40_92_0
= _rtP -> P_68 * _rtB -> B_40_91_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW ->
dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem/Wagon model4/Data Store Read" , 0
) ; _rtB -> B_40_93_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_hs != 0 ) {
_rtX -> vv0at_CSTATE_kq = _rtB -> B_40_93_0 ; } _rtB -> B_40_94_0 = _rtX ->
vv0at_CSTATE_kq ; _rtB -> B_40_95_0 = muDoubleScalarAbs ( _rtB -> B_40_94_0 )
; _rtB -> B_40_96_0 = _rtP -> P_69 [ 0 ] ; _rtB -> B_40_96_0 = _rtB ->
B_40_96_0 * _rtB -> B_40_95_0 + _rtP -> P_69 [ 1 ] ; _rtB -> B_40_96_0 = _rtB
-> B_40_96_0 * _rtB -> B_40_95_0 + _rtP -> P_69 [ 2 ] ; _rtB -> B_40_97_0 =
_rtP -> P_70 * _rtB -> B_40_96_0 ; _rtB -> B_40_98_0 = _rtB -> B_40_6_0 *
_rtB -> B_40_97_0 ; _rtB -> B_40_99_0 = _rtB -> B_40_92_0 - _rtB -> B_40_98_0
; if ( _rtB -> B_40_99_0 > _rtB -> B_40_64_0_o ) { _rtB -> B_40_102_0 = _rtB
-> B_40_64_0_o ; } else { if ( _rtB -> B_40_99_0 < _rtB -> B_40_65_0_n ) {
_rtB -> B_4_1_0 = _rtB -> B_40_65_0_n ; } else { _rtB -> B_4_1_0 = _rtB ->
B_40_99_0 ; } _rtB -> B_40_102_0 = _rtB -> B_4_1_0 ; } { real_T * * uBuffer =
( real_T * * ) & _rtDW -> TransportDelay_PWORK_l . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_l .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_71 ; _rtB -> B_40_103_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_p . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_p . Last , _rtDW -> TransportDelay_IWORK_p . Tail ,
_rtDW -> TransportDelay_IWORK_p . Head , _rtP -> P_72 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_40_104_0 = 0.0 ; _rtB -> B_40_104_0 += _rtP -> P_74 * _rtX ->
TransferFcn_CSTATE_b0 ; _rtB -> B_40_104_0 += _rtP -> P_75 * _rtB ->
B_40_103_0 ; _rtB -> B_40_105_0 = _rtP -> P_76 * _rtB -> B_40_104_0 ; if (
_rtB -> B_40_105_0 > _rtP -> P_77 ) { _rtB -> B_40_106_0 = _rtP -> P_77 ; }
else if ( _rtB -> B_40_105_0 < _rtP -> P_78 ) { _rtB -> B_40_106_0 = _rtP ->
P_78 ; } else { _rtB -> B_40_106_0 = _rtB -> B_40_105_0 ; } if ( ( _rtDW ->
LastMajorTimeA_d >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW -> LastMajorTimeB_k
>= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_40_107_0 = _rtB -> B_40_106_0 ; }
else { if ( ( ( _rtDW -> LastMajorTimeA_d < _rtDW -> LastMajorTimeB_k ) && (
_rtDW -> LastMajorTimeB_k < ssGetTaskTime ( S , 0 ) ) ) || ( ( _rtDW ->
LastMajorTimeA_d >= _rtDW -> LastMajorTimeB_k ) && ( _rtDW ->
LastMajorTimeA_d >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime (
S , 0 ) - _rtDW -> LastMajorTimeB_k ; rtb_B_40_3_0 = _rtDW -> PrevYB_n ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_d ;
rtb_B_40_3_0 = _rtDW -> PrevYA_f ; } riseValLimit = deltaT * _rtP -> P_79 ;
rateLimiterRate = _rtB -> B_40_106_0 - rtb_B_40_3_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_40_107_0 = rtb_B_40_3_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_80 ; if ( rateLimiterRate < deltaT ) { _rtB -> B_40_107_0
= rtb_B_40_3_0 + deltaT ; } else { _rtB -> B_40_107_0 = _rtB -> B_40_106_0 ;
} } } _rtB -> B_40_108_0 = _rtP -> P_81 * _rtB -> B_40_107_0 ; _rtB ->
B_40_109_0 = _rtP -> P_82 * _rtB -> B_40_108_0 ; vm_ReadLocalDSMNoIdx ( S ,
_rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model/Data Store Read" , 0
) ; _rtB -> B_40_110_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_i != 0 ) {
_rtX -> vv0at_CSTATE_p = _rtB -> B_40_110_0 ; } _rtB -> B_40_111_0 = _rtX ->
vv0at_CSTATE_p ; _rtB -> B_40_112_0 = muDoubleScalarAbs ( _rtB -> B_40_111_0
) ; _rtB -> B_40_113_0 = _rtP -> P_83 [ 0 ] ; _rtB -> B_40_113_0 = _rtB ->
B_40_113_0 * _rtB -> B_40_112_0 + _rtP -> P_83 [ 1 ] ; _rtB -> B_40_113_0 =
_rtB -> B_40_113_0 * _rtB -> B_40_112_0 + _rtP -> P_83 [ 2 ] ; _rtB ->
B_40_114_0 = _rtP -> P_84 * _rtB -> B_40_113_0 ; _rtB -> B_40_115_0 = _rtB ->
B_40_8_0 * _rtB -> B_40_114_0 ; _rtB -> B_40_116_0 = _rtB -> B_40_109_0 -
_rtB -> B_40_115_0 ; if ( _rtB -> B_40_116_0 > _rtB -> B_40_67_0 ) { _rtB ->
B_40_119_0 = _rtB -> B_40_67_0 ; } else { if ( _rtB -> B_40_116_0 < _rtB ->
B_40_68_0_m ) { _rtB -> B_5_1_0 = _rtB -> B_40_68_0_m ; } else { _rtB ->
B_5_1_0 = _rtB -> B_40_116_0 ; } _rtB -> B_40_119_0 = _rtB -> B_5_1_0 ; } {
real_T * * uBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_g .
TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_g . TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ;
real_T tMinusDelay = simTime - _rtP -> P_85 ; _rtB -> B_40_120_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_n . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_n . Last , _rtDW -> TransportDelay_IWORK_n . Tail ,
_rtDW -> TransportDelay_IWORK_n . Head , _rtP -> P_86 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_40_121_0 = 0.0 ; _rtB -> B_40_121_0 += _rtP -> P_88 * _rtX ->
TransferFcn_CSTATE_e ; _rtB -> B_40_121_0 += _rtP -> P_89 * _rtB ->
B_40_120_0 ; _rtB -> B_40_122_0 = _rtP -> P_90 * _rtB -> B_40_121_0 ; if (
_rtB -> B_40_122_0 > _rtP -> P_91 ) { _rtB -> B_40_123_0 = _rtP -> P_91 ; }
else if ( _rtB -> B_40_122_0 < _rtP -> P_92 ) { _rtB -> B_40_123_0 = _rtP ->
P_92 ; } else { _rtB -> B_40_123_0 = _rtB -> B_40_122_0 ; } if ( ( _rtDW ->
LastMajorTimeA_b >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW -> LastMajorTimeB_e
>= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_40_124_0 = _rtB -> B_40_123_0 ; }
else { if ( ( ( _rtDW -> LastMajorTimeA_b < _rtDW -> LastMajorTimeB_e ) && (
_rtDW -> LastMajorTimeB_e < ssGetTaskTime ( S , 0 ) ) ) || ( ( _rtDW ->
LastMajorTimeA_b >= _rtDW -> LastMajorTimeB_e ) && ( _rtDW ->
LastMajorTimeA_b >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime (
S , 0 ) - _rtDW -> LastMajorTimeB_e ; rtb_B_40_3_0 = _rtDW -> PrevYB_l ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_b ;
rtb_B_40_3_0 = _rtDW -> PrevYA_o ; } riseValLimit = deltaT * _rtP -> P_93 ;
rateLimiterRate = _rtB -> B_40_123_0 - rtb_B_40_3_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_40_124_0 = rtb_B_40_3_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_94 ; if ( rateLimiterRate < deltaT ) { _rtB -> B_40_124_0
= rtb_B_40_3_0 + deltaT ; } else { _rtB -> B_40_124_0 = _rtB -> B_40_123_0 ;
} } } _rtB -> B_40_125_0 = _rtP -> P_95 * _rtB -> B_40_124_0 ; _rtB ->
B_40_126_0 = _rtP -> P_96 * _rtB -> B_40_125_0 ; vm_ReadLocalDSMNoIdx ( S ,
_rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model1/Data Store Read" ,
0 ) ; _rtB -> B_40_127_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_k != 0 ) {
_rtX -> vv0at_CSTATE_p1 = _rtB -> B_40_127_0 ; } _rtB -> B_40_128_0 = _rtX ->
vv0at_CSTATE_p1 ; _rtB -> B_40_129_0 = muDoubleScalarAbs ( _rtB -> B_40_128_0
) ; _rtB -> B_40_130_0 = _rtP -> P_97 [ 0 ] ; _rtB -> B_40_130_0 = _rtB ->
B_40_130_0 * _rtB -> B_40_129_0 + _rtP -> P_97 [ 1 ] ; _rtB -> B_40_130_0 =
_rtB -> B_40_130_0 * _rtB -> B_40_129_0 + _rtP -> P_97 [ 2 ] ; _rtB ->
B_40_131_0 = _rtP -> P_98 * _rtB -> B_40_130_0 ; _rtB -> B_40_132_0 = _rtB ->
B_40_9_0 * _rtB -> B_40_131_0 ; _rtB -> B_40_133_0 = _rtB -> B_40_126_0 -
_rtB -> B_40_132_0 ; if ( _rtB -> B_40_133_0 > _rtB -> B_40_70_0_c ) { _rtB
-> B_40_136_0 = _rtB -> B_40_70_0_c ; } else { if ( _rtB -> B_40_133_0 < _rtB
-> B_40_71_0_m ) { _rtB -> B_6_1_0 = _rtB -> B_40_71_0_m ; } else { _rtB ->
B_6_1_0 = _rtB -> B_40_133_0 ; } _rtB -> B_40_136_0 = _rtB -> B_6_1_0 ; } {
real_T * * uBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_aa .
TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_aa . TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S )
; real_T tMinusDelay = simTime - _rtP -> P_99 ; _rtB -> B_40_137_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_p4 . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_p4 . Last , _rtDW -> TransportDelay_IWORK_p4 . Tail ,
_rtDW -> TransportDelay_IWORK_p4 . Head , _rtP -> P_100 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_40_138_0 = 0.0 ; _rtB -> B_40_138_0 += _rtP -> P_102 * _rtX
-> TransferFcn_CSTATE_ec ; _rtB -> B_40_138_0 += _rtP -> P_103 * _rtB ->
B_40_137_0 ; _rtB -> B_40_139_0 = _rtP -> P_104 * _rtB -> B_40_138_0 ; if (
_rtB -> B_40_139_0 > _rtP -> P_105 ) { _rtB -> B_40_140_0 = _rtP -> P_105 ; }
else if ( _rtB -> B_40_139_0 < _rtP -> P_106 ) { _rtB -> B_40_140_0 = _rtP ->
P_106 ; } else { _rtB -> B_40_140_0 = _rtB -> B_40_139_0 ; } if ( ( _rtDW ->
LastMajorTimeA_l >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW -> LastMajorTimeB_l
>= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_40_141_0 = _rtB -> B_40_140_0 ; }
else { if ( ( ( _rtDW -> LastMajorTimeA_l < _rtDW -> LastMajorTimeB_l ) && (
_rtDW -> LastMajorTimeB_l < ssGetTaskTime ( S , 0 ) ) ) || ( ( _rtDW ->
LastMajorTimeA_l >= _rtDW -> LastMajorTimeB_l ) && ( _rtDW ->
LastMajorTimeA_l >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime (
S , 0 ) - _rtDW -> LastMajorTimeB_l ; rtb_B_40_3_0 = _rtDW -> PrevYB_go ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_l ;
rtb_B_40_3_0 = _rtDW -> PrevYA_m ; } riseValLimit = deltaT * _rtP -> P_107 ;
rateLimiterRate = _rtB -> B_40_140_0 - rtb_B_40_3_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_40_141_0 = rtb_B_40_3_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_108 ; if ( rateLimiterRate < deltaT ) { _rtB ->
B_40_141_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB -> B_40_141_0 = _rtB ->
B_40_140_0 ; } } } _rtB -> B_40_142_0 = _rtP -> P_109 * _rtB -> B_40_141_0 ;
_rtB -> B_40_143_0 = _rtP -> P_110 * _rtB -> B_40_142_0 ;
vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model2/Data Store Read" ,
0 ) ; _rtB -> B_40_144_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_d != 0 ) {
_rtX -> vv0at_CSTATE_l = _rtB -> B_40_144_0 ; } _rtB -> B_40_145_0 = _rtX ->
vv0at_CSTATE_l ; _rtB -> B_40_146_0 = muDoubleScalarAbs ( _rtB -> B_40_145_0
) ; _rtB -> B_40_147_0 = _rtP -> P_111 [ 0 ] ; _rtB -> B_40_147_0 = _rtB ->
B_40_147_0 * _rtB -> B_40_146_0 + _rtP -> P_111 [ 1 ] ; _rtB -> B_40_147_0 =
_rtB -> B_40_147_0 * _rtB -> B_40_146_0 + _rtP -> P_111 [ 2 ] ; _rtB ->
B_40_148_0 = _rtP -> P_112 * _rtB -> B_40_147_0 ; _rtB -> B_40_149_0 = _rtB
-> B_40_10_0_k * _rtB -> B_40_148_0 ; _rtB -> B_40_150_0 = _rtB -> B_40_143_0
- _rtB -> B_40_149_0 ; if ( _rtB -> B_40_150_0 > _rtB -> B_40_73_0_m ) { _rtB
-> B_40_153_0 = _rtB -> B_40_73_0_m ; } else { if ( _rtB -> B_40_150_0 < _rtB
-> B_40_74_0_j ) { _rtB -> B_7_1_0 = _rtB -> B_40_74_0_j ; } else { _rtB ->
B_7_1_0 = _rtB -> B_40_150_0 ; } _rtB -> B_40_153_0 = _rtB -> B_7_1_0 ; } {
real_T * * uBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_l0 .
TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_l0 . TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S )
; real_T tMinusDelay = simTime - _rtP -> P_113 ; _rtB -> B_40_154_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_c . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_c . Last , _rtDW -> TransportDelay_IWORK_c . Tail ,
_rtDW -> TransportDelay_IWORK_c . Head , _rtP -> P_114 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_40_155_0 = 0.0 ; _rtB -> B_40_155_0 += _rtP -> P_116 * _rtX
-> TransferFcn_CSTATE_k ; _rtB -> B_40_155_0 += _rtP -> P_117 * _rtB ->
B_40_154_0 ; _rtB -> B_40_156_0 = _rtP -> P_118 * _rtB -> B_40_155_0 ; if (
_rtB -> B_40_156_0 > _rtP -> P_119 ) { _rtB -> B_40_157_0 = _rtP -> P_119 ; }
else if ( _rtB -> B_40_156_0 < _rtP -> P_120 ) { _rtB -> B_40_157_0 = _rtP ->
P_120 ; } else { _rtB -> B_40_157_0 = _rtB -> B_40_156_0 ; } if ( ( _rtDW ->
LastMajorTimeA_dv >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW -> LastMajorTimeB_g
>= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_40_158_0 = _rtB -> B_40_157_0 ; }
else { if ( ( ( _rtDW -> LastMajorTimeA_dv < _rtDW -> LastMajorTimeB_g ) && (
_rtDW -> LastMajorTimeB_g < ssGetTaskTime ( S , 0 ) ) ) || ( ( _rtDW ->
LastMajorTimeA_dv >= _rtDW -> LastMajorTimeB_g ) && ( _rtDW ->
LastMajorTimeA_dv >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime (
S , 0 ) - _rtDW -> LastMajorTimeB_g ; rtb_B_40_3_0 = _rtDW -> PrevYB_i ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_dv ;
rtb_B_40_3_0 = _rtDW -> PrevYA_b ; } riseValLimit = deltaT * _rtP -> P_121 ;
rateLimiterRate = _rtB -> B_40_157_0 - rtb_B_40_3_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_40_158_0 = rtb_B_40_3_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_122 ; if ( rateLimiterRate < deltaT ) { _rtB ->
B_40_158_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB -> B_40_158_0 = _rtB ->
B_40_157_0 ; } } } _rtB -> B_40_159_0 = _rtP -> P_123 * _rtB -> B_40_158_0 ;
_rtB -> B_40_160_0 = _rtP -> P_124 * _rtB -> B_40_159_0 ;
vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model3/Data Store Read" ,
0 ) ; _rtB -> B_40_161_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_j != 0 ) {
_rtX -> vv0at_CSTATE_ly = _rtB -> B_40_161_0 ; } _rtB -> B_40_162_0 = _rtX ->
vv0at_CSTATE_ly ; _rtB -> B_40_163_0 = muDoubleScalarAbs ( _rtB -> B_40_162_0
) ; _rtB -> B_40_164_0 = _rtP -> P_125 [ 0 ] ; _rtB -> B_40_164_0 = _rtB ->
B_40_164_0 * _rtB -> B_40_163_0 + _rtP -> P_125 [ 1 ] ; _rtB -> B_40_164_0 =
_rtB -> B_40_164_0 * _rtB -> B_40_163_0 + _rtP -> P_125 [ 2 ] ; _rtB ->
B_40_165_0 = _rtP -> P_126 * _rtB -> B_40_164_0 ; _rtB -> B_40_166_0 = _rtB
-> B_40_11_0_c * _rtB -> B_40_165_0 ; _rtB -> B_40_167_0 = _rtB -> B_40_160_0
- _rtB -> B_40_166_0 ; if ( _rtB -> B_40_167_0 > _rtB -> B_40_76_0_h ) { _rtB
-> B_40_170_0 = _rtB -> B_40_76_0_h ; } else { if ( _rtB -> B_40_167_0 < _rtB
-> B_40_77_0_c ) { _rtB -> B_8_1_0 = _rtB -> B_40_77_0_c ; } else { _rtB ->
B_8_1_0 = _rtB -> B_40_167_0 ; } _rtB -> B_40_170_0 = _rtB -> B_8_1_0 ; } {
real_T * * uBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_p .
TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_p . TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ;
real_T tMinusDelay = simTime - _rtP -> P_127 ; _rtB -> B_40_171_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_d . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_d . Last , _rtDW -> TransportDelay_IWORK_d . Tail ,
_rtDW -> TransportDelay_IWORK_d . Head , _rtP -> P_128 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_40_172_0 = 0.0 ; _rtB -> B_40_172_0 += _rtP -> P_130 * _rtX
-> TransferFcn_CSTATE_m ; _rtB -> B_40_172_0 += _rtP -> P_131 * _rtB ->
B_40_171_0 ; _rtB -> B_40_173_0 = _rtP -> P_132 * _rtB -> B_40_172_0 ; if (
_rtB -> B_40_173_0 > _rtP -> P_133 ) { _rtB -> B_40_174_0 = _rtP -> P_133 ; }
else if ( _rtB -> B_40_173_0 < _rtP -> P_134 ) { _rtB -> B_40_174_0 = _rtP ->
P_134 ; } else { _rtB -> B_40_174_0 = _rtB -> B_40_173_0 ; } if ( ( _rtDW ->
LastMajorTimeA_gv >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW ->
LastMajorTimeB_e4 >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_40_175_0 = _rtB
-> B_40_174_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_gv < _rtDW ->
LastMajorTimeB_e4 ) && ( _rtDW -> LastMajorTimeB_e4 < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_gv >= _rtDW -> LastMajorTimeB_e4 ) && (
_rtDW -> LastMajorTimeA_gv >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_e4 ; rtb_B_40_3_0 = _rtDW
-> PrevYB_a ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_gv ; rtb_B_40_3_0 = _rtDW -> PrevYA_b5 ; } riseValLimit =
deltaT * _rtP -> P_135 ; rateLimiterRate = _rtB -> B_40_174_0 - rtb_B_40_3_0
; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_40_175_0 = rtb_B_40_3_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_136 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_40_175_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB ->
B_40_175_0 = _rtB -> B_40_174_0 ; } } } _rtB -> B_40_176_0 = _rtP -> P_137 *
_rtB -> B_40_175_0 ; _rtB -> B_40_177_0 = _rtP -> P_138 * _rtB -> B_40_176_0
; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model4/Data Store Read" ,
0 ) ; _rtB -> B_40_178_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_e != 0 ) {
_rtX -> vv0at_CSTATE_a = _rtB -> B_40_178_0 ; } _rtB -> B_40_179_0 = _rtX ->
vv0at_CSTATE_a ; _rtB -> B_40_180_0 = muDoubleScalarAbs ( _rtB -> B_40_179_0
) ; _rtB -> B_40_181_0 = _rtP -> P_139 [ 0 ] ; _rtB -> B_40_181_0 = _rtB ->
B_40_181_0 * _rtB -> B_40_180_0 + _rtP -> P_139 [ 1 ] ; _rtB -> B_40_181_0 =
_rtB -> B_40_181_0 * _rtB -> B_40_180_0 + _rtP -> P_139 [ 2 ] ; _rtB ->
B_40_182_0 = _rtP -> P_140 * _rtB -> B_40_181_0 ; _rtB -> B_40_183_0 = _rtB
-> B_40_12_0_b * _rtB -> B_40_182_0 ; _rtB -> B_40_184_0 = _rtB -> B_40_177_0
- _rtB -> B_40_183_0 ; if ( _rtB -> B_40_184_0 > _rtB -> B_40_79_0_c ) { _rtB
-> B_40_187_0 = _rtB -> B_40_79_0_c ; } else { if ( _rtB -> B_40_184_0 < _rtB
-> B_40_80_0_p ) { _rtB -> B_9_1_0 = _rtB -> B_40_80_0_p ; } else { _rtB ->
B_9_1_0 = _rtB -> B_40_184_0 ; } _rtB -> B_40_187_0 = _rtB -> B_9_1_0 ; } {
real_T * * uBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_o .
TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_o . TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ;
real_T tMinusDelay = simTime - _rtP -> P_141 ; _rtB -> B_40_188_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_cu . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_cu . Last , _rtDW -> TransportDelay_IWORK_cu . Tail ,
_rtDW -> TransportDelay_IWORK_cu . Head , _rtP -> P_142 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_40_189_0 = 0.0 ; _rtB -> B_40_189_0 += _rtP -> P_144 * _rtX
-> TransferFcn_CSTATE_f ; _rtB -> B_40_189_0 += _rtP -> P_145 * _rtB ->
B_40_188_0 ; _rtB -> B_40_190_0 = _rtP -> P_146 * _rtB -> B_40_189_0 ; if (
_rtB -> B_40_190_0 > _rtP -> P_147 ) { _rtB -> B_40_191_0 = _rtP -> P_147 ; }
else if ( _rtB -> B_40_190_0 < _rtP -> P_148 ) { _rtB -> B_40_191_0 = _rtP ->
P_148 ; } else { _rtB -> B_40_191_0 = _rtB -> B_40_190_0 ; } if ( ( _rtDW ->
LastMajorTimeA_p >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW -> LastMajorTimeB_hn
>= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_40_192_0 = _rtB -> B_40_191_0 ; }
else { if ( ( ( _rtDW -> LastMajorTimeA_p < _rtDW -> LastMajorTimeB_hn ) && (
_rtDW -> LastMajorTimeB_hn < ssGetTaskTime ( S , 0 ) ) ) || ( ( _rtDW ->
LastMajorTimeA_p >= _rtDW -> LastMajorTimeB_hn ) && ( _rtDW ->
LastMajorTimeA_p >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime (
S , 0 ) - _rtDW -> LastMajorTimeB_hn ; rtb_B_40_3_0 = _rtDW -> PrevYB_gc ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_p ;
rtb_B_40_3_0 = _rtDW -> PrevYA_i ; } riseValLimit = deltaT * _rtP -> P_149 ;
rateLimiterRate = _rtB -> B_40_191_0 - rtb_B_40_3_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_40_192_0 = rtb_B_40_3_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_150 ; if ( rateLimiterRate < deltaT ) { _rtB ->
B_40_192_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB -> B_40_192_0 = _rtB ->
B_40_191_0 ; } } } _rtB -> B_40_193_0 = _rtP -> P_151 * _rtB -> B_40_192_0 ;
_rtB -> B_40_194_0 = _rtP -> P_152 * _rtB -> B_40_193_0 ;
vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model/Data Store Read" , 0
) ; _rtB -> B_40_195_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_n != 0 ) {
_rtX -> vv0at_CSTATE_n = _rtB -> B_40_195_0 ; } _rtB -> B_40_196_0 = _rtX ->
vv0at_CSTATE_n ; _rtB -> B_40_197_0 = muDoubleScalarAbs ( _rtB -> B_40_196_0
) ; _rtB -> B_40_198_0 = _rtP -> P_153 [ 0 ] ; _rtB -> B_40_198_0 = _rtB ->
B_40_198_0 * _rtB -> B_40_197_0 + _rtP -> P_153 [ 1 ] ; _rtB -> B_40_198_0 =
_rtB -> B_40_198_0 * _rtB -> B_40_197_0 + _rtP -> P_153 [ 2 ] ; _rtB ->
B_40_199_0 = _rtP -> P_154 * _rtB -> B_40_198_0 ; _rtB -> B_40_200_0 = _rtB
-> B_40_14_0_p * _rtB -> B_40_199_0 ; _rtB -> B_40_201_0 = _rtB -> B_40_194_0
- _rtB -> B_40_200_0 ; if ( _rtB -> B_40_201_0 > _rtB -> B_40_82_0_p ) { _rtB
-> B_40_204_0 = _rtB -> B_40_82_0_p ; } else { if ( _rtB -> B_40_201_0 < _rtB
-> B_40_83_0 ) { _rtB -> B_10_1_0 = _rtB -> B_40_83_0 ; } else { _rtB ->
B_10_1_0 = _rtB -> B_40_201_0 ; } _rtB -> B_40_204_0 = _rtB -> B_10_1_0 ; } {
real_T * * uBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_dz .
TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_dz . TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S )
; real_T tMinusDelay = simTime - _rtP -> P_155 ; _rtB -> B_40_205_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_j . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_j . Last , _rtDW -> TransportDelay_IWORK_j . Tail ,
_rtDW -> TransportDelay_IWORK_j . Head , _rtP -> P_156 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_40_206_0 = 0.0 ; _rtB -> B_40_206_0 += _rtP -> P_158 * _rtX
-> TransferFcn_CSTATE_o ; _rtB -> B_40_206_0 += _rtP -> P_159 * _rtB ->
B_40_205_0 ; _rtB -> B_40_207_0 = _rtP -> P_160 * _rtB -> B_40_206_0 ; if (
_rtB -> B_40_207_0 > _rtP -> P_161 ) { _rtB -> B_40_208_0 = _rtP -> P_161 ; }
else if ( _rtB -> B_40_207_0 < _rtP -> P_162 ) { _rtB -> B_40_208_0 = _rtP ->
P_162 ; } else { _rtB -> B_40_208_0 = _rtB -> B_40_207_0 ; } if ( ( _rtDW ->
LastMajorTimeA_o >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW -> LastMajorTimeB_gq
>= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_40_209_0 = _rtB -> B_40_208_0 ; }
else { if ( ( ( _rtDW -> LastMajorTimeA_o < _rtDW -> LastMajorTimeB_gq ) && (
_rtDW -> LastMajorTimeB_gq < ssGetTaskTime ( S , 0 ) ) ) || ( ( _rtDW ->
LastMajorTimeA_o >= _rtDW -> LastMajorTimeB_gq ) && ( _rtDW ->
LastMajorTimeA_o >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime (
S , 0 ) - _rtDW -> LastMajorTimeB_gq ; rtb_B_40_3_0 = _rtDW -> PrevYB_g2 ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_o ;
rtb_B_40_3_0 = _rtDW -> PrevYA_j ; } riseValLimit = deltaT * _rtP -> P_163 ;
rateLimiterRate = _rtB -> B_40_208_0 - rtb_B_40_3_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_40_209_0 = rtb_B_40_3_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_164 ; if ( rateLimiterRate < deltaT ) { _rtB ->
B_40_209_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB -> B_40_209_0 = _rtB ->
B_40_208_0 ; } } } _rtB -> B_40_210_0 = _rtP -> P_165 * _rtB -> B_40_209_0 ;
_rtB -> B_40_211_0 = _rtP -> P_166 * _rtB -> B_40_210_0 ;
vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model1/Data Store Read" ,
0 ) ; _rtB -> B_40_212_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_f != 0 ) {
_rtX -> vv0at_CSTATE_j = _rtB -> B_40_212_0 ; } _rtB -> B_40_213_0 = _rtX ->
vv0at_CSTATE_j ; _rtB -> B_40_214_0 = muDoubleScalarAbs ( _rtB -> B_40_213_0
) ; _rtB -> B_40_215_0 = _rtP -> P_167 [ 0 ] ; _rtB -> B_40_215_0 = _rtB ->
B_40_215_0 * _rtB -> B_40_214_0 + _rtP -> P_167 [ 1 ] ; _rtB -> B_40_215_0 =
_rtB -> B_40_215_0 * _rtB -> B_40_214_0 + _rtP -> P_167 [ 2 ] ; _rtB ->
B_40_216_0 = _rtP -> P_168 * _rtB -> B_40_215_0 ; _rtB -> B_40_217_0 = _rtB
-> B_40_15_0 * _rtB -> B_40_216_0 ; _rtB -> B_40_218_0 = _rtB -> B_40_211_0 -
_rtB -> B_40_217_0 ; if ( _rtB -> B_40_218_0 > _rtB -> B_40_85_0_a ) { _rtB
-> B_40_221_0 = _rtB -> B_40_85_0_a ; } else { if ( _rtB -> B_40_218_0 < _rtB
-> B_40_86_0_e ) { _rtB -> B_11_1_0 = _rtB -> B_40_86_0_e ; } else { _rtB ->
B_11_1_0 = _rtB -> B_40_218_0 ; } _rtB -> B_40_221_0 = _rtB -> B_11_1_0 ; } {
real_T * * uBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_mf .
TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_mf . TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S )
; real_T tMinusDelay = simTime - _rtP -> P_169 ; _rtB -> B_40_222_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_bo . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_bo . Last , _rtDW -> TransportDelay_IWORK_bo . Tail ,
_rtDW -> TransportDelay_IWORK_bo . Head , _rtP -> P_170 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_40_223_0 = 0.0 ; _rtB -> B_40_223_0 += _rtP -> P_172 * _rtX
-> TransferFcn_CSTATE_oj ; _rtB -> B_40_223_0 += _rtP -> P_173 * _rtB ->
B_40_222_0 ; _rtB -> B_40_224_0 = _rtP -> P_174 * _rtB -> B_40_223_0 ; if (
_rtB -> B_40_224_0 > _rtP -> P_175 ) { _rtB -> B_40_225_0 = _rtP -> P_175 ; }
else if ( _rtB -> B_40_224_0 < _rtP -> P_176 ) { _rtB -> B_40_225_0 = _rtP ->
P_176 ; } else { _rtB -> B_40_225_0 = _rtB -> B_40_224_0 ; } if ( ( _rtDW ->
LastMajorTimeA_c >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW -> LastMajorTimeB_i
>= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_40_226_0 = _rtB -> B_40_225_0 ; }
else { if ( ( ( _rtDW -> LastMajorTimeA_c < _rtDW -> LastMajorTimeB_i ) && (
_rtDW -> LastMajorTimeB_i < ssGetTaskTime ( S , 0 ) ) ) || ( ( _rtDW ->
LastMajorTimeA_c >= _rtDW -> LastMajorTimeB_i ) && ( _rtDW ->
LastMajorTimeA_c >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime (
S , 0 ) - _rtDW -> LastMajorTimeB_i ; rtb_B_40_3_0 = _rtDW -> PrevYB_p ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_c ;
rtb_B_40_3_0 = _rtDW -> PrevYA_jn ; } riseValLimit = deltaT * _rtP -> P_177 ;
rateLimiterRate = _rtB -> B_40_225_0 - rtb_B_40_3_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_40_226_0 = rtb_B_40_3_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_178 ; if ( rateLimiterRate < deltaT ) { _rtB ->
B_40_226_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB -> B_40_226_0 = _rtB ->
B_40_225_0 ; } } } _rtB -> B_40_227_0 = _rtP -> P_179 * _rtB -> B_40_226_0 ;
_rtB -> B_40_228_0 = _rtP -> P_180 * _rtB -> B_40_227_0 ;
vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model2/Data Store Read" ,
0 ) ; _rtB -> B_40_229_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_g != 0 ) {
_rtX -> vv0at_CSTATE_of = _rtB -> B_40_229_0 ; } _rtB -> B_40_230_0 = _rtX ->
vv0at_CSTATE_of ; _rtB -> B_40_231_0 = muDoubleScalarAbs ( _rtB -> B_40_230_0
) ; _rtB -> B_40_232_0 = _rtP -> P_181 [ 0 ] ; _rtB -> B_40_232_0 = _rtB ->
B_40_232_0 * _rtB -> B_40_231_0 + _rtP -> P_181 [ 1 ] ; _rtB -> B_40_232_0 =
_rtB -> B_40_232_0 * _rtB -> B_40_231_0 + _rtP -> P_181 [ 2 ] ; _rtB ->
B_40_233_0 = _rtP -> P_182 * _rtB -> B_40_232_0 ; _rtB -> B_40_234_0 = _rtB
-> B_40_16_0 * _rtB -> B_40_233_0 ; _rtB -> B_40_235_0 = _rtB -> B_40_228_0 -
_rtB -> B_40_234_0 ; if ( _rtB -> B_40_235_0 > _rtB -> B_40_88_0_a ) { _rtB
-> B_40_238_0 = _rtB -> B_40_88_0_a ; } else { if ( _rtB -> B_40_235_0 < _rtB
-> B_40_89_0_a ) { _rtB -> B_12_1_0 = _rtB -> B_40_89_0_a ; } else { _rtB ->
B_12_1_0 = _rtB -> B_40_235_0 ; } _rtB -> B_40_238_0 = _rtB -> B_12_1_0 ; } {
real_T * * uBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_mc .
TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_mc . TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S )
; real_T tMinusDelay = simTime - _rtP -> P_183 ; _rtB -> B_40_239_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_ko . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_ko . Last , _rtDW -> TransportDelay_IWORK_ko . Tail ,
_rtDW -> TransportDelay_IWORK_ko . Head , _rtP -> P_184 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_40_240_0 = 0.0 ; _rtB -> B_40_240_0 += _rtP -> P_186 * _rtX
-> TransferFcn_CSTATE_p ; _rtB -> B_40_240_0 += _rtP -> P_187 * _rtB ->
B_40_239_0 ; _rtB -> B_40_241_0 = _rtP -> P_188 * _rtB -> B_40_240_0 ; if (
_rtB -> B_40_241_0 > _rtP -> P_189 ) { _rtB -> B_40_242_0 = _rtP -> P_189 ; }
else if ( _rtB -> B_40_241_0 < _rtP -> P_190 ) { _rtB -> B_40_242_0 = _rtP ->
P_190 ; } else { _rtB -> B_40_242_0 = _rtB -> B_40_241_0 ; } if ( ( _rtDW ->
LastMajorTimeA_dm >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW -> LastMajorTimeB_o
>= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_40_243_0 = _rtB -> B_40_242_0 ; }
else { if ( ( ( _rtDW -> LastMajorTimeA_dm < _rtDW -> LastMajorTimeB_o ) && (
_rtDW -> LastMajorTimeB_o < ssGetTaskTime ( S , 0 ) ) ) || ( ( _rtDW ->
LastMajorTimeA_dm >= _rtDW -> LastMajorTimeB_o ) && ( _rtDW ->
LastMajorTimeA_dm >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime (
S , 0 ) - _rtDW -> LastMajorTimeB_o ; rtb_B_40_3_0 = _rtDW -> PrevYB_h0 ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_dm ;
rtb_B_40_3_0 = _rtDW -> PrevYA_od ; } riseValLimit = deltaT * _rtP -> P_191 ;
rateLimiterRate = _rtB -> B_40_242_0 - rtb_B_40_3_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_40_243_0 = rtb_B_40_3_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_192 ; if ( rateLimiterRate < deltaT ) { _rtB ->
B_40_243_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB -> B_40_243_0 = _rtB ->
B_40_242_0 ; } } } _rtB -> B_40_244_0 = _rtP -> P_193 * _rtB -> B_40_243_0 ;
_rtB -> B_40_245_0 = _rtP -> P_194 * _rtB -> B_40_244_0 ;
vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model3/Data Store Read" ,
0 ) ; _rtB -> B_40_246_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_a != 0 ) {
_rtX -> vv0at_CSTATE_b = _rtB -> B_40_246_0 ; } _rtB -> B_40_247_0 = _rtX ->
vv0at_CSTATE_b ; _rtB -> B_40_248_0 = muDoubleScalarAbs ( _rtB -> B_40_247_0
) ; _rtB -> B_40_249_0 = _rtP -> P_195 [ 0 ] ; _rtB -> B_40_249_0 = _rtB ->
B_40_249_0 * _rtB -> B_40_248_0 + _rtP -> P_195 [ 1 ] ; _rtB -> B_40_249_0 =
_rtB -> B_40_249_0 * _rtB -> B_40_248_0 + _rtP -> P_195 [ 2 ] ; _rtB ->
B_40_250_0 = _rtP -> P_196 * _rtB -> B_40_249_0 ; _rtB -> B_40_251_0 = _rtB
-> B_40_17_0_c * _rtB -> B_40_250_0 ; _rtB -> B_40_252_0 = _rtB -> B_40_245_0
- _rtB -> B_40_251_0 ; if ( _rtB -> B_40_252_0 > _rtB -> B_40_91_0_i ) { _rtB
-> B_40_255_0 = _rtB -> B_40_91_0_i ; } else { if ( _rtB -> B_40_252_0 < _rtB
-> B_40_92_0_l ) { _rtB -> B_13_1_0 = _rtB -> B_40_92_0_l ; } else { _rtB ->
B_13_1_0 = _rtB -> B_40_252_0 ; } _rtB -> B_40_255_0 = _rtB -> B_13_1_0 ; } {
real_T * * uBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_l0w .
TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_l0w . TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S )
; real_T tMinusDelay = simTime - _rtP -> P_197 ; _rtB -> B_40_256_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_g . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_g . Last , _rtDW -> TransportDelay_IWORK_g . Tail ,
_rtDW -> TransportDelay_IWORK_g . Head , _rtP -> P_198 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_40_257_0 = 0.0 ; _rtB -> B_40_257_0 += _rtP -> P_200 * _rtX
-> TransferFcn_CSTATE_l ; _rtB -> B_40_257_0 += _rtP -> P_201 * _rtB ->
B_40_256_0 ; _rtB -> B_40_258_0 = _rtP -> P_202 * _rtB -> B_40_257_0 ; if (
_rtB -> B_40_258_0 > _rtP -> P_203 ) { _rtB -> B_40_259_0 = _rtP -> P_203 ; }
else if ( _rtB -> B_40_258_0 < _rtP -> P_204 ) { _rtB -> B_40_259_0 = _rtP ->
P_204 ; } else { _rtB -> B_40_259_0 = _rtB -> B_40_258_0 ; } if ( ( _rtDW ->
LastMajorTimeA_ga >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW -> LastMajorTimeB_b
>= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_40_260_0 = _rtB -> B_40_259_0 ; }
else { if ( ( ( _rtDW -> LastMajorTimeA_ga < _rtDW -> LastMajorTimeB_b ) && (
_rtDW -> LastMajorTimeB_b < ssGetTaskTime ( S , 0 ) ) ) || ( ( _rtDW ->
LastMajorTimeA_ga >= _rtDW -> LastMajorTimeB_b ) && ( _rtDW ->
LastMajorTimeA_ga >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime (
S , 0 ) - _rtDW -> LastMajorTimeB_b ; rtb_B_40_3_0 = _rtDW -> PrevYB_m ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_ga ;
rtb_B_40_3_0 = _rtDW -> PrevYA_e1 ; } riseValLimit = deltaT * _rtP -> P_205 ;
rateLimiterRate = _rtB -> B_40_259_0 - rtb_B_40_3_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_40_260_0 = rtb_B_40_3_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_206 ; if ( rateLimiterRate < deltaT ) { _rtB ->
B_40_260_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB -> B_40_260_0 = _rtB ->
B_40_259_0 ; } } } _rtB -> B_40_261_0 = _rtP -> P_207 * _rtB -> B_40_260_0 ;
_rtB -> B_40_262_0 = _rtP -> P_208 * _rtB -> B_40_261_0 ;
vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model4/Data Store Read" ,
0 ) ; _rtB -> B_40_263_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_gr != 0 )
{ _rtX -> vv0at_CSTATE_n3 = _rtB -> B_40_263_0 ; } _rtB -> B_40_264_0 = _rtX
-> vv0at_CSTATE_n3 ; _rtB -> B_40_265_0 = muDoubleScalarAbs ( _rtB ->
B_40_264_0 ) ; _rtB -> B_40_266_0 = _rtP -> P_209 [ 0 ] ; _rtB -> B_40_266_0
= _rtB -> B_40_266_0 * _rtB -> B_40_265_0 + _rtP -> P_209 [ 1 ] ; _rtB ->
B_40_266_0 = _rtB -> B_40_266_0 * _rtB -> B_40_265_0 + _rtP -> P_209 [ 2 ] ;
_rtB -> B_40_267_0 = _rtP -> P_210 * _rtB -> B_40_266_0 ; _rtB -> B_40_268_0
= _rtB -> B_40_18_0_f * _rtB -> B_40_267_0 ; _rtB -> B_40_269_0 = _rtB ->
B_40_262_0 - _rtB -> B_40_268_0 ; if ( _rtB -> B_40_269_0 > _rtB ->
B_40_94_0_o ) { _rtB -> B_40_272_0 = _rtB -> B_40_94_0_o ; } else { if ( _rtB
-> B_40_269_0 < _rtB -> B_40_95_0_o ) { _rtB -> B_14_1_0 = _rtB ->
B_40_95_0_o ; } else { _rtB -> B_14_1_0 = _rtB -> B_40_269_0 ; } _rtB ->
B_40_272_0 = _rtB -> B_14_1_0 ; } { real_T * * uBuffer = ( real_T * * ) &
_rtDW -> TransportDelay_PWORK_gu . TUbufferPtrs [ 0 ] ; real_T * * tBuffer =
( real_T * * ) & _rtDW -> TransportDelay_PWORK_gu . TUbufferPtrs [ 1 ] ;
real_T simTime = ssGetT ( S ) ; real_T tMinusDelay = simTime - _rtP -> P_211
; _rtB -> B_40_273_0 = Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay ,
0.0 , * tBuffer , * uBuffer , _rtDW -> TransportDelay_IWORK_pq .
CircularBufSize , & _rtDW -> TransportDelay_IWORK_pq . Last , _rtDW ->
TransportDelay_IWORK_pq . Tail , _rtDW -> TransportDelay_IWORK_pq . Head ,
_rtP -> P_212 , 0 , ( boolean_T ) ( ssIsMinorTimeStep ( S ) && (
ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) ) ) ; } _rtB -> B_40_274_0 =
0.0 ; _rtB -> B_40_274_0 += _rtP -> P_214 * _rtX -> TransferFcn_CSTATE_ou ;
_rtB -> B_40_274_0 += _rtP -> P_215 * _rtB -> B_40_273_0 ; _rtB -> B_40_275_0
= _rtP -> P_216 * _rtB -> B_40_274_0 ; if ( _rtB -> B_40_275_0 > _rtP ->
P_217 ) { _rtB -> B_40_276_0 = _rtP -> P_217 ; } else if ( _rtB -> B_40_275_0
< _rtP -> P_218 ) { _rtB -> B_40_276_0 = _rtP -> P_218 ; } else { _rtB ->
B_40_276_0 = _rtB -> B_40_275_0 ; } if ( ( _rtDW -> LastMajorTimeA_cs >=
ssGetTaskTime ( S , 0 ) ) && ( _rtDW -> LastMajorTimeB_ms >= ssGetTaskTime (
S , 0 ) ) ) { _rtB -> B_40_277_0 = _rtB -> B_40_276_0 ; } else { if ( ( (
_rtDW -> LastMajorTimeA_cs < _rtDW -> LastMajorTimeB_ms ) && ( _rtDW ->
LastMajorTimeB_ms < ssGetTaskTime ( S , 0 ) ) ) || ( ( _rtDW ->
LastMajorTimeA_cs >= _rtDW -> LastMajorTimeB_ms ) && ( _rtDW ->
LastMajorTimeA_cs >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime (
S , 0 ) - _rtDW -> LastMajorTimeB_ms ; rtb_B_40_3_0 = _rtDW -> PrevYB_c3 ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_cs ;
rtb_B_40_3_0 = _rtDW -> PrevYA_l ; } riseValLimit = deltaT * _rtP -> P_219 ;
rateLimiterRate = _rtB -> B_40_276_0 - rtb_B_40_3_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_40_277_0 = rtb_B_40_3_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_220 ; if ( rateLimiterRate < deltaT ) { _rtB ->
B_40_277_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB -> B_40_277_0 = _rtB ->
B_40_276_0 ; } } } _rtB -> B_40_278_0 = _rtP -> P_221 * _rtB -> B_40_277_0 ;
_rtB -> B_40_279_0 = _rtP -> P_222 * _rtB -> B_40_278_0 ;
vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model/Data Store Read" , 0
) ; _rtB -> B_40_280_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_m != 0 ) {
_rtX -> vv0at_CSTATE_c = _rtB -> B_40_280_0 ; } _rtB -> B_40_281_0 = _rtX ->
vv0at_CSTATE_c ; _rtB -> B_40_282_0 = muDoubleScalarAbs ( _rtB -> B_40_281_0
) ; _rtB -> B_40_283_0 = _rtP -> P_223 [ 0 ] ; _rtB -> B_40_283_0 = _rtB ->
B_40_283_0 * _rtB -> B_40_282_0 + _rtP -> P_223 [ 1 ] ; _rtB -> B_40_283_0 =
_rtB -> B_40_283_0 * _rtB -> B_40_282_0 + _rtP -> P_223 [ 2 ] ; _rtB ->
B_40_284_0 = _rtP -> P_224 * _rtB -> B_40_283_0 ; _rtB -> B_40_285_0 = _rtB
-> B_40_20_0_g * _rtB -> B_40_284_0 ; _rtB -> B_40_286_0 = _rtB -> B_40_279_0
- _rtB -> B_40_285_0 ; if ( _rtB -> B_40_286_0 > _rtB -> B_40_97_0_i ) { _rtB
-> B_40_289_0 = _rtB -> B_40_97_0_i ; } else { if ( _rtB -> B_40_286_0 < _rtB
-> B_40_98_0_f ) { _rtB -> B_15_1_0 = _rtB -> B_40_98_0_f ; } else { _rtB ->
B_15_1_0 = _rtB -> B_40_286_0 ; } _rtB -> B_40_289_0 = _rtB -> B_15_1_0 ; } {
real_T * * uBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_c .
TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_c . TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ;
real_T tMinusDelay = simTime - _rtP -> P_225 ; _rtB -> B_40_290_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_i . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_i . Last , _rtDW -> TransportDelay_IWORK_i . Tail ,
_rtDW -> TransportDelay_IWORK_i . Head , _rtP -> P_226 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_40_291_0 = 0.0 ; _rtB -> B_40_291_0 += _rtP -> P_228 * _rtX
-> TransferFcn_CSTATE_kc ; _rtB -> B_40_291_0 += _rtP -> P_229 * _rtB ->
B_40_290_0 ; _rtB -> B_40_292_0 = _rtP -> P_230 * _rtB -> B_40_291_0 ; if (
_rtB -> B_40_292_0 > _rtP -> P_231 ) { _rtB -> B_40_293_0 = _rtP -> P_231 ; }
else if ( _rtB -> B_40_292_0 < _rtP -> P_232 ) { _rtB -> B_40_293_0 = _rtP ->
P_232 ; } else { _rtB -> B_40_293_0 = _rtB -> B_40_292_0 ; } if ( ( _rtDW ->
LastMajorTimeA_dvp >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW ->
LastMajorTimeB_ej >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_40_294_0 = _rtB
-> B_40_293_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_dvp < _rtDW ->
LastMajorTimeB_ej ) && ( _rtDW -> LastMajorTimeB_ej < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_dvp >= _rtDW -> LastMajorTimeB_ej ) && (
_rtDW -> LastMajorTimeA_dvp >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_ej ; rtb_B_40_3_0 = _rtDW
-> PrevYB_mu ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_dvp ; rtb_B_40_3_0 = _rtDW -> PrevYA_oq ; } riseValLimit =
deltaT * _rtP -> P_233 ; rateLimiterRate = _rtB -> B_40_293_0 - rtb_B_40_3_0
; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_40_294_0 = rtb_B_40_3_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_234 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_40_294_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB ->
B_40_294_0 = _rtB -> B_40_293_0 ; } } } _rtB -> B_40_295_0 = _rtP -> P_235 *
_rtB -> B_40_294_0 ; _rtB -> B_40_296_0 = _rtP -> P_236 * _rtB -> B_40_295_0
; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model1/Data Store Read" ,
0 ) ; _rtB -> B_40_297_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_eh != 0 )
{ _rtX -> vv0at_CSTATE_ch = _rtB -> B_40_297_0 ; } _rtB -> B_40_298_0 = _rtX
-> vv0at_CSTATE_ch ; _rtB -> B_40_299_0 = muDoubleScalarAbs ( _rtB ->
B_40_298_0 ) ; _rtB -> B_40_300_0 = _rtP -> P_237 [ 0 ] ; _rtB -> B_40_300_0
= _rtB -> B_40_300_0 * _rtB -> B_40_299_0 + _rtP -> P_237 [ 1 ] ; _rtB ->
B_40_300_0 = _rtB -> B_40_300_0 * _rtB -> B_40_299_0 + _rtP -> P_237 [ 2 ] ;
_rtB -> B_40_301_0 = _rtP -> P_238 * _rtB -> B_40_300_0 ; _rtB -> B_40_302_0
= _rtB -> B_40_21_0_g * _rtB -> B_40_301_0 ; _rtB -> B_40_303_0 = _rtB ->
B_40_296_0 - _rtB -> B_40_302_0 ; if ( _rtB -> B_40_303_0 > _rtB ->
B_40_100_0 ) { _rtB -> B_40_306_0 = _rtB -> B_40_100_0 ; } else { if ( _rtB
-> B_40_303_0 < _rtB -> B_40_101_0 ) { _rtB -> B_16_1_0 = _rtB -> B_40_101_0
; } else { _rtB -> B_16_1_0 = _rtB -> B_40_303_0 ; } _rtB -> B_40_306_0 =
_rtB -> B_16_1_0 ; } { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_k . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T *
* ) & _rtDW -> TransportDelay_PWORK_k . TUbufferPtrs [ 1 ] ; real_T simTime =
ssGetT ( S ) ; real_T tMinusDelay = simTime - _rtP -> P_239 ; _rtB ->
B_40_307_0 = Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , *
tBuffer , * uBuffer , _rtDW -> TransportDelay_IWORK_p0 . CircularBufSize , &
_rtDW -> TransportDelay_IWORK_p0 . Last , _rtDW -> TransportDelay_IWORK_p0 .
Tail , _rtDW -> TransportDelay_IWORK_p0 . Head , _rtP -> P_240 , 0 , (
boolean_T ) ( ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) ==
ssGetT ( S ) ) ) ) ; } _rtB -> B_40_308_0 = 0.0 ; _rtB -> B_40_308_0 += _rtP
-> P_242 * _rtX -> TransferFcn_CSTATE_i ; _rtB -> B_40_308_0 += _rtP -> P_243
* _rtB -> B_40_307_0 ; _rtB -> B_40_309_0 = _rtP -> P_244 * _rtB ->
B_40_308_0 ; if ( _rtB -> B_40_309_0 > _rtP -> P_245 ) { _rtB -> B_40_310_0 =
_rtP -> P_245 ; } else if ( _rtB -> B_40_309_0 < _rtP -> P_246 ) { _rtB ->
B_40_310_0 = _rtP -> P_246 ; } else { _rtB -> B_40_310_0 = _rtB -> B_40_309_0
; } if ( ( _rtDW -> LastMajorTimeA_in >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW
-> LastMajorTimeB_ha >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_40_311_0 =
_rtB -> B_40_310_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_in < _rtDW ->
LastMajorTimeB_ha ) && ( _rtDW -> LastMajorTimeB_ha < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_in >= _rtDW -> LastMajorTimeB_ha ) && (
_rtDW -> LastMajorTimeA_in >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_ha ; rtb_B_40_3_0 = _rtDW
-> PrevYB_hj ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_in ; rtb_B_40_3_0 = _rtDW -> PrevYA_lw ; } riseValLimit =
deltaT * _rtP -> P_247 ; rateLimiterRate = _rtB -> B_40_310_0 - rtb_B_40_3_0
; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_40_311_0 = rtb_B_40_3_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_248 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_40_311_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB ->
B_40_311_0 = _rtB -> B_40_310_0 ; } } } _rtB -> B_40_312_0 = _rtP -> P_249 *
_rtB -> B_40_311_0 ; _rtB -> B_40_313_0 = _rtP -> P_250 * _rtB -> B_40_312_0
; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model2/Data Store Read" ,
0 ) ; _rtB -> B_40_314_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_km != 0 )
{ _rtX -> vv0at_CSTATE_bd = _rtB -> B_40_314_0 ; } _rtB -> B_40_315_0 = _rtX
-> vv0at_CSTATE_bd ; _rtB -> B_40_316_0 = muDoubleScalarAbs ( _rtB ->
B_40_315_0 ) ; _rtB -> B_40_317_0 = _rtP -> P_251 [ 0 ] ; _rtB -> B_40_317_0
= _rtB -> B_40_317_0 * _rtB -> B_40_316_0 + _rtP -> P_251 [ 1 ] ; _rtB ->
B_40_317_0 = _rtB -> B_40_317_0 * _rtB -> B_40_316_0 + _rtP -> P_251 [ 2 ] ;
_rtB -> B_40_318_0 = _rtP -> P_252 * _rtB -> B_40_317_0 ; _rtB -> B_40_319_0
= _rtB -> B_40_22_0_m * _rtB -> B_40_318_0 ; _rtB -> B_40_320_0 = _rtB ->
B_40_313_0 - _rtB -> B_40_319_0 ; if ( _rtB -> B_40_320_0 > _rtB ->
B_40_103_0_i ) { _rtB -> B_40_323_0 = _rtB -> B_40_103_0_i ; } else { if (
_rtB -> B_40_320_0 < _rtB -> B_40_104_0_f ) { _rtB -> B_17_1_0 = _rtB ->
B_40_104_0_f ; } else { _rtB -> B_17_1_0 = _rtB -> B_40_320_0 ; } _rtB ->
B_40_323_0 = _rtB -> B_17_1_0 ; } { real_T * * uBuffer = ( real_T * * ) &
_rtDW -> TransportDelay_PWORK_i . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_i . TUbufferPtrs [ 1 ] ; real_T
simTime = ssGetT ( S ) ; real_T tMinusDelay = simTime - _rtP -> P_253 ; _rtB
-> B_40_324_0 = Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 ,
* tBuffer , * uBuffer , _rtDW -> TransportDelay_IWORK_o . CircularBufSize , &
_rtDW -> TransportDelay_IWORK_o . Last , _rtDW -> TransportDelay_IWORK_o .
Tail , _rtDW -> TransportDelay_IWORK_o . Head , _rtP -> P_254 , 0 , (
boolean_T ) ( ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) ==
ssGetT ( S ) ) ) ) ; } _rtB -> B_40_325_0 = 0.0 ; _rtB -> B_40_325_0 += _rtP
-> P_256 * _rtX -> TransferFcn_CSTATE_me ; _rtB -> B_40_325_0 += _rtP ->
P_257 * _rtB -> B_40_324_0 ; _rtB -> B_40_326_0 = _rtP -> P_258 * _rtB ->
B_40_325_0 ; if ( _rtB -> B_40_326_0 > _rtP -> P_259 ) { _rtB -> B_40_327_0 =
_rtP -> P_259 ; } else if ( _rtB -> B_40_326_0 < _rtP -> P_260 ) { _rtB ->
B_40_327_0 = _rtP -> P_260 ; } else { _rtB -> B_40_327_0 = _rtB -> B_40_326_0
; } if ( ( _rtDW -> LastMajorTimeA_iy >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW
-> LastMajorTimeB_ij >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_40_328_0 =
_rtB -> B_40_327_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_iy < _rtDW ->
LastMajorTimeB_ij ) && ( _rtDW -> LastMajorTimeB_ij < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_iy >= _rtDW -> LastMajorTimeB_ij ) && (
_rtDW -> LastMajorTimeA_iy >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_ij ; rtb_B_40_3_0 = _rtDW
-> PrevYB_d ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_iy ; rtb_B_40_3_0 = _rtDW -> PrevYA_l3 ; } riseValLimit =
deltaT * _rtP -> P_261 ; rateLimiterRate = _rtB -> B_40_327_0 - rtb_B_40_3_0
; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_40_328_0 = rtb_B_40_3_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_262 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_40_328_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB ->
B_40_328_0 = _rtB -> B_40_327_0 ; } } } _rtB -> B_40_329_0 = _rtP -> P_263 *
_rtB -> B_40_328_0 ; _rtB -> B_40_330_0 = _rtP -> P_264 * _rtB -> B_40_329_0
; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model3/Data Store Read" ,
0 ) ; _rtB -> B_40_331_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_oo != 0 )
{ _rtX -> vv0at_CSTATE_i = _rtB -> B_40_331_0 ; } _rtB -> B_40_332_0 = _rtX
-> vv0at_CSTATE_i ; _rtB -> B_40_333_0 = muDoubleScalarAbs ( _rtB ->
B_40_332_0 ) ; _rtB -> B_40_334_0 = _rtP -> P_265 [ 0 ] ; _rtB -> B_40_334_0
= _rtB -> B_40_334_0 * _rtB -> B_40_333_0 + _rtP -> P_265 [ 1 ] ; _rtB ->
B_40_334_0 = _rtB -> B_40_334_0 * _rtB -> B_40_333_0 + _rtP -> P_265 [ 2 ] ;
_rtB -> B_40_335_0 = _rtP -> P_266 * _rtB -> B_40_334_0 ; _rtB -> B_40_336_0
= _rtB -> B_40_23_0_n * _rtB -> B_40_335_0 ; _rtB -> B_40_337_0 = _rtB ->
B_40_330_0 - _rtB -> B_40_336_0 ; if ( _rtB -> B_40_337_0 > _rtB ->
B_40_106_0_g ) { _rtB -> B_40_340_0 = _rtB -> B_40_106_0_g ; } else { if (
_rtB -> B_40_337_0 < _rtB -> B_40_107_0_c ) { _rtB -> B_18_1_0 = _rtB ->
B_40_107_0_c ; } else { _rtB -> B_18_1_0 = _rtB -> B_40_337_0 ; } _rtB ->
B_40_340_0 = _rtB -> B_18_1_0 ; } { real_T * * uBuffer = ( real_T * * ) &
_rtDW -> TransportDelay_PWORK_mfn . TUbufferPtrs [ 0 ] ; real_T * * tBuffer =
( real_T * * ) & _rtDW -> TransportDelay_PWORK_mfn . TUbufferPtrs [ 1 ] ;
real_T simTime = ssGetT ( S ) ; real_T tMinusDelay = simTime - _rtP -> P_267
; _rtB -> B_40_341_0 = Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay ,
0.0 , * tBuffer , * uBuffer , _rtDW -> TransportDelay_IWORK_co .
CircularBufSize , & _rtDW -> TransportDelay_IWORK_co . Last , _rtDW ->
TransportDelay_IWORK_co . Tail , _rtDW -> TransportDelay_IWORK_co . Head ,
_rtP -> P_268 , 0 , ( boolean_T ) ( ssIsMinorTimeStep ( S ) && (
ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) ) ) ; } _rtB -> B_40_342_0 =
0.0 ; _rtB -> B_40_342_0 += _rtP -> P_270 * _rtX -> TransferFcn_CSTATE_lw ;
_rtB -> B_40_342_0 += _rtP -> P_271 * _rtB -> B_40_341_0 ; _rtB -> B_40_343_0
= _rtP -> P_272 * _rtB -> B_40_342_0 ; if ( _rtB -> B_40_343_0 > _rtP ->
P_273 ) { _rtB -> B_40_344_0 = _rtP -> P_273 ; } else if ( _rtB -> B_40_343_0
< _rtP -> P_274 ) { _rtB -> B_40_344_0 = _rtP -> P_274 ; } else { _rtB ->
B_40_344_0 = _rtB -> B_40_343_0 ; } if ( ( _rtDW -> LastMajorTimeA_k >=
ssGetTaskTime ( S , 0 ) ) && ( _rtDW -> LastMajorTimeB_oh >= ssGetTaskTime (
S , 0 ) ) ) { _rtB -> B_40_345_0 = _rtB -> B_40_344_0 ; } else { if ( ( (
_rtDW -> LastMajorTimeA_k < _rtDW -> LastMajorTimeB_oh ) && ( _rtDW ->
LastMajorTimeB_oh < ssGetTaskTime ( S , 0 ) ) ) || ( ( _rtDW ->
LastMajorTimeA_k >= _rtDW -> LastMajorTimeB_oh ) && ( _rtDW ->
LastMajorTimeA_k >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime (
S , 0 ) - _rtDW -> LastMajorTimeB_oh ; rtb_B_40_3_0 = _rtDW -> PrevYB_b ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_k ;
rtb_B_40_3_0 = _rtDW -> PrevYA_bz ; } riseValLimit = deltaT * _rtP -> P_275 ;
rateLimiterRate = _rtB -> B_40_344_0 - rtb_B_40_3_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_40_345_0 = rtb_B_40_3_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_276 ; if ( rateLimiterRate < deltaT ) { _rtB ->
B_40_345_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB -> B_40_345_0 = _rtB ->
B_40_344_0 ; } } } _rtB -> B_40_346_0 = _rtP -> P_277 * _rtB -> B_40_345_0 ;
_rtB -> B_40_347_0 = _rtP -> P_278 * _rtB -> B_40_346_0 ;
vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model4/Data Store Read" ,
0 ) ; _rtB -> B_40_348_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_e5 != 0 )
{ _rtX -> vv0at_CSTATE_le = _rtB -> B_40_348_0 ; } _rtB -> B_40_349_0 = _rtX
-> vv0at_CSTATE_le ; _rtB -> B_40_350_0 = muDoubleScalarAbs ( _rtB ->
B_40_349_0 ) ; _rtB -> B_40_351_0 = _rtP -> P_279 [ 0 ] ; _rtB -> B_40_351_0
= _rtB -> B_40_351_0 * _rtB -> B_40_350_0 + _rtP -> P_279 [ 1 ] ; _rtB ->
B_40_351_0 = _rtB -> B_40_351_0 * _rtB -> B_40_350_0 + _rtP -> P_279 [ 2 ] ;
_rtB -> B_40_352_0 = _rtP -> P_280 * _rtB -> B_40_351_0 ; _rtB -> B_40_353_0
= _rtB -> B_40_24_0_p * _rtB -> B_40_352_0 ; _rtB -> B_40_354_0 = _rtB ->
B_40_347_0 - _rtB -> B_40_353_0 ; if ( _rtB -> B_40_354_0 > _rtB ->
B_40_109_0_o ) { _rtB -> B_40_357_0 = _rtB -> B_40_109_0_o ; } else { if (
_rtB -> B_40_354_0 < _rtB -> B_40_110_0_l ) { _rtB -> B_19_1_0 = _rtB ->
B_40_110_0_l ; } else { _rtB -> B_19_1_0 = _rtB -> B_40_354_0 ; } _rtB ->
B_40_357_0 = _rtB -> B_19_1_0 ; } { real_T * * uBuffer = ( real_T * * ) &
_rtDW -> TransportDelay_PWORK_b . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_b . TUbufferPtrs [ 1 ] ; real_T
simTime = ssGetT ( S ) ; real_T tMinusDelay = simTime - _rtP -> P_281 ; _rtB
-> B_40_358_0 = Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 ,
* tBuffer , * uBuffer , _rtDW -> TransportDelay_IWORK_gp . CircularBufSize ,
& _rtDW -> TransportDelay_IWORK_gp . Last , _rtDW -> TransportDelay_IWORK_gp
. Tail , _rtDW -> TransportDelay_IWORK_gp . Head , _rtP -> P_282 , 0 , (
boolean_T ) ( ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) ==
ssGetT ( S ) ) ) ) ; } _rtB -> B_40_359_0 = 0.0 ; _rtB -> B_40_359_0 += _rtP
-> P_284 * _rtX -> TransferFcn_CSTATE_ll ; _rtB -> B_40_359_0 += _rtP ->
P_285 * _rtB -> B_40_358_0 ; _rtB -> B_40_360_0 = _rtP -> P_286 * _rtB ->
B_40_359_0 ; if ( _rtB -> B_40_360_0 > _rtP -> P_287 ) { _rtB -> B_40_361_0 =
_rtP -> P_287 ; } else if ( _rtB -> B_40_360_0 < _rtP -> P_288 ) { _rtB ->
B_40_361_0 = _rtP -> P_288 ; } else { _rtB -> B_40_361_0 = _rtB -> B_40_360_0
; } if ( ( _rtDW -> LastMajorTimeA_bd >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW
-> LastMajorTimeB_lw >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_40_362_0 =
_rtB -> B_40_361_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_bd < _rtDW ->
LastMajorTimeB_lw ) && ( _rtDW -> LastMajorTimeB_lw < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_bd >= _rtDW -> LastMajorTimeB_lw ) && (
_rtDW -> LastMajorTimeA_bd >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_lw ; rtb_B_40_3_0 = _rtDW
-> PrevYB_an ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_bd ; rtb_B_40_3_0 = _rtDW -> PrevYA_ir ; } riseValLimit =
deltaT * _rtP -> P_289 ; rateLimiterRate = _rtB -> B_40_361_0 - rtb_B_40_3_0
; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_40_362_0 = rtb_B_40_3_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_290 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_40_362_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB ->
B_40_362_0 = _rtB -> B_40_361_0 ; } } } _rtB -> B_40_363_0 = _rtP -> P_291 *
_rtB -> B_40_362_0 ; _rtB -> B_40_364_0 = _rtP -> P_292 * _rtB -> B_40_363_0
; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model/Data Store Read" , 0
) ; _rtB -> B_40_365_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_oe != 0 ) {
_rtX -> vv0at_CSTATE_e = _rtB -> B_40_365_0 ; } _rtB -> B_40_366_0 = _rtX ->
vv0at_CSTATE_e ; _rtB -> B_40_367_0 = muDoubleScalarAbs ( _rtB -> B_40_366_0
) ; _rtB -> B_40_368_0 = _rtP -> P_293 [ 0 ] ; _rtB -> B_40_368_0 = _rtB ->
B_40_368_0 * _rtB -> B_40_367_0 + _rtP -> P_293 [ 1 ] ; _rtB -> B_40_368_0 =
_rtB -> B_40_368_0 * _rtB -> B_40_367_0 + _rtP -> P_293 [ 2 ] ; _rtB ->
B_40_369_0 = _rtP -> P_294 * _rtB -> B_40_368_0 ; _rtB -> B_40_370_0 = _rtB
-> B_40_26_0_l * _rtB -> B_40_369_0 ; _rtB -> B_40_371_0 = _rtB -> B_40_364_0
- _rtB -> B_40_370_0 ; if ( _rtB -> B_40_371_0 > _rtB -> B_40_112_0_m ) {
_rtB -> B_40_374_0 = _rtB -> B_40_112_0_m ; } else { if ( _rtB -> B_40_371_0
< _rtB -> B_40_113_0_m ) { _rtB -> B_20_1_0 = _rtB -> B_40_113_0_m ; } else {
_rtB -> B_20_1_0 = _rtB -> B_40_371_0 ; } _rtB -> B_40_374_0 = _rtB ->
B_20_1_0 ; } { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_by . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T
* * ) & _rtDW -> TransportDelay_PWORK_by . TUbufferPtrs [ 1 ] ; real_T
simTime = ssGetT ( S ) ; real_T tMinusDelay = simTime - _rtP -> P_295 ; _rtB
-> B_40_375_0 = Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 ,
* tBuffer , * uBuffer , _rtDW -> TransportDelay_IWORK_e . CircularBufSize , &
_rtDW -> TransportDelay_IWORK_e . Last , _rtDW -> TransportDelay_IWORK_e .
Tail , _rtDW -> TransportDelay_IWORK_e . Head , _rtP -> P_296 , 0 , (
boolean_T ) ( ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) ==
ssGetT ( S ) ) ) ) ; } _rtB -> B_40_376_0 = 0.0 ; _rtB -> B_40_376_0 += _rtP
-> P_298 * _rtX -> TransferFcn_CSTATE_p1 ; _rtB -> B_40_376_0 += _rtP ->
P_299 * _rtB -> B_40_375_0 ; _rtB -> B_40_377_0 = _rtP -> P_300 * _rtB ->
B_40_376_0 ; if ( _rtB -> B_40_377_0 > _rtP -> P_301 ) { _rtB -> B_40_378_0 =
_rtP -> P_301 ; } else if ( _rtB -> B_40_377_0 < _rtP -> P_302 ) { _rtB ->
B_40_378_0 = _rtP -> P_302 ; } else { _rtB -> B_40_378_0 = _rtB -> B_40_377_0
; } if ( ( _rtDW -> LastMajorTimeA_f >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW
-> LastMajorTimeB_gm >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_40_379_0 =
_rtB -> B_40_378_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_f < _rtDW ->
LastMajorTimeB_gm ) && ( _rtDW -> LastMajorTimeB_gm < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_f >= _rtDW -> LastMajorTimeB_gm ) && (
_rtDW -> LastMajorTimeA_f >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_gm ; rtb_B_40_3_0 = _rtDW
-> PrevYB_fh ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_f ; rtb_B_40_3_0 = _rtDW -> PrevYA_m1 ; } riseValLimit =
deltaT * _rtP -> P_303 ; rateLimiterRate = _rtB -> B_40_378_0 - rtb_B_40_3_0
; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_40_379_0 = rtb_B_40_3_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_304 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_40_379_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB ->
B_40_379_0 = _rtB -> B_40_378_0 ; } } } _rtB -> B_40_380_0 = _rtP -> P_305 *
_rtB -> B_40_379_0 ; _rtB -> B_40_381_0 = _rtP -> P_306 * _rtB -> B_40_380_0
; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model1/Data Store Read" ,
0 ) ; _rtB -> B_40_382_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_et != 0 )
{ _rtX -> vv0at_CSTATE_oy = _rtB -> B_40_382_0 ; } _rtB -> B_40_383_0 = _rtX
-> vv0at_CSTATE_oy ; _rtB -> B_40_384_0 = muDoubleScalarAbs ( _rtB ->
B_40_383_0 ) ; _rtB -> B_40_385_0 = _rtP -> P_307 [ 0 ] ; _rtB -> B_40_385_0
= _rtB -> B_40_385_0 * _rtB -> B_40_384_0 + _rtP -> P_307 [ 1 ] ; _rtB ->
B_40_385_0 = _rtB -> B_40_385_0 * _rtB -> B_40_384_0 + _rtP -> P_307 [ 2 ] ;
_rtB -> B_40_386_0 = _rtP -> P_308 * _rtB -> B_40_385_0 ; _rtB -> B_40_387_0
= _rtB -> B_40_27_0_j * _rtB -> B_40_386_0 ; _rtB -> B_40_388_0 = _rtB ->
B_40_381_0 - _rtB -> B_40_387_0 ; if ( _rtB -> B_40_388_0 > _rtB ->
B_40_115_0_c ) { _rtB -> B_40_391_0 = _rtB -> B_40_115_0_c ; } else { if (
_rtB -> B_40_388_0 < _rtB -> B_40_116_0_f ) { _rtB -> B_21_1_0 = _rtB ->
B_40_116_0_f ; } else { _rtB -> B_21_1_0 = _rtB -> B_40_388_0 ; } _rtB ->
B_40_391_0 = _rtB -> B_21_1_0 ; } { real_T * * uBuffer = ( real_T * * ) &
_rtDW -> TransportDelay_PWORK_j . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_j . TUbufferPtrs [ 1 ] ; real_T
simTime = ssGetT ( S ) ; real_T tMinusDelay = simTime - _rtP -> P_309 ; _rtB
-> B_40_392_0 = Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 ,
* tBuffer , * uBuffer , _rtDW -> TransportDelay_IWORK_nc . CircularBufSize ,
& _rtDW -> TransportDelay_IWORK_nc . Last , _rtDW -> TransportDelay_IWORK_nc
. Tail , _rtDW -> TransportDelay_IWORK_nc . Head , _rtP -> P_310 , 0 , (
boolean_T ) ( ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) ==
ssGetT ( S ) ) ) ) ; } _rtB -> B_40_393_0 = 0.0 ; _rtB -> B_40_393_0 += _rtP
-> P_312 * _rtX -> TransferFcn_CSTATE_o4 ; _rtB -> B_40_393_0 += _rtP ->
P_313 * _rtB -> B_40_392_0 ; _rtB -> B_40_394_0 = _rtP -> P_314 * _rtB ->
B_40_393_0 ; if ( _rtB -> B_40_394_0 > _rtP -> P_315 ) { _rtB -> B_40_395_0 =
_rtP -> P_315 ; } else if ( _rtB -> B_40_394_0 < _rtP -> P_316 ) { _rtB ->
B_40_395_0 = _rtP -> P_316 ; } else { _rtB -> B_40_395_0 = _rtB -> B_40_394_0
; } if ( ( _rtDW -> LastMajorTimeA_ch >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW
-> LastMajorTimeB_ar >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_40_396_0 =
_rtB -> B_40_395_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_ch < _rtDW ->
LastMajorTimeB_ar ) && ( _rtDW -> LastMajorTimeB_ar < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_ch >= _rtDW -> LastMajorTimeB_ar ) && (
_rtDW -> LastMajorTimeA_ch >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_ar ; rtb_B_40_3_0 = _rtDW
-> PrevYB_gl ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_ch ; rtb_B_40_3_0 = _rtDW -> PrevYA_g ; } riseValLimit =
deltaT * _rtP -> P_317 ; rateLimiterRate = _rtB -> B_40_395_0 - rtb_B_40_3_0
; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_40_396_0 = rtb_B_40_3_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_318 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_40_396_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB ->
B_40_396_0 = _rtB -> B_40_395_0 ; } } } _rtB -> B_40_397_0 = _rtP -> P_319 *
_rtB -> B_40_396_0 ; _rtB -> B_40_398_0 = _rtP -> P_320 * _rtB -> B_40_397_0
; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model2/Data Store Read" ,
0 ) ; _rtB -> B_40_399_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_je != 0 )
{ _rtX -> vv0at_CSTATE_jb = _rtB -> B_40_399_0 ; } _rtB -> B_40_400_0 = _rtX
-> vv0at_CSTATE_jb ; _rtB -> B_40_401_0 = muDoubleScalarAbs ( _rtB ->
B_40_400_0 ) ; _rtB -> B_40_402_0 = _rtP -> P_321 [ 0 ] ; _rtB -> B_40_402_0
= _rtB -> B_40_402_0 * _rtB -> B_40_401_0 + _rtP -> P_321 [ 1 ] ; _rtB ->
B_40_402_0 = _rtB -> B_40_402_0 * _rtB -> B_40_401_0 + _rtP -> P_321 [ 2 ] ;
_rtB -> B_40_403_0 = _rtP -> P_322 * _rtB -> B_40_402_0 ; _rtB -> B_40_404_0
= _rtB -> B_40_28_0_d * _rtB -> B_40_403_0 ; _rtB -> B_40_405_0 = _rtB ->
B_40_398_0 - _rtB -> B_40_404_0 ; if ( _rtB -> B_40_405_0 > _rtB ->
B_40_118_0 ) { _rtB -> B_40_408_0 = _rtB -> B_40_118_0 ; } else { if ( _rtB
-> B_40_405_0 < _rtB -> B_40_119_0_p ) { _rtB -> B_22_1_0 = _rtB ->
B_40_119_0_p ; } else { _rtB -> B_22_1_0 = _rtB -> B_40_405_0 ; } _rtB ->
B_40_408_0 = _rtB -> B_22_1_0 ; } { real_T * * uBuffer = ( real_T * * ) &
_rtDW -> TransportDelay_PWORK_dm . TUbufferPtrs [ 0 ] ; real_T * * tBuffer =
( real_T * * ) & _rtDW -> TransportDelay_PWORK_dm . TUbufferPtrs [ 1 ] ;
real_T simTime = ssGetT ( S ) ; real_T tMinusDelay = simTime - _rtP -> P_323
; _rtB -> B_40_409_0 = Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay ,
0.0 , * tBuffer , * uBuffer , _rtDW -> TransportDelay_IWORK_di .
CircularBufSize , & _rtDW -> TransportDelay_IWORK_di . Last , _rtDW ->
TransportDelay_IWORK_di . Tail , _rtDW -> TransportDelay_IWORK_di . Head ,
_rtP -> P_324 , 0 , ( boolean_T ) ( ssIsMinorTimeStep ( S ) && (
ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) ) ) ; } _rtB -> B_40_410_0 =
0.0 ; _rtB -> B_40_410_0 += _rtP -> P_326 * _rtX -> TransferFcn_CSTATE_g ;
_rtB -> B_40_410_0 += _rtP -> P_327 * _rtB -> B_40_409_0 ; _rtB -> B_40_411_0
= _rtP -> P_328 * _rtB -> B_40_410_0 ; if ( _rtB -> B_40_411_0 > _rtP ->
P_329 ) { _rtB -> B_40_412_0 = _rtP -> P_329 ; } else if ( _rtB -> B_40_411_0
< _rtP -> P_330 ) { _rtB -> B_40_412_0 = _rtP -> P_330 ; } else { _rtB ->
B_40_412_0 = _rtB -> B_40_411_0 ; } if ( ( _rtDW -> LastMajorTimeA_l2 >=
ssGetTaskTime ( S , 0 ) ) && ( _rtDW -> LastMajorTimeB_la >= ssGetTaskTime (
S , 0 ) ) ) { _rtB -> B_40_413_0 = _rtB -> B_40_412_0 ; } else { if ( ( (
_rtDW -> LastMajorTimeA_l2 < _rtDW -> LastMajorTimeB_la ) && ( _rtDW ->
LastMajorTimeB_la < ssGetTaskTime ( S , 0 ) ) ) || ( ( _rtDW ->
LastMajorTimeA_l2 >= _rtDW -> LastMajorTimeB_la ) && ( _rtDW ->
LastMajorTimeA_l2 >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime (
S , 0 ) - _rtDW -> LastMajorTimeB_la ; rtb_B_40_3_0 = _rtDW -> PrevYB_ar ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_l2 ;
rtb_B_40_3_0 = _rtDW -> PrevYA_ey ; } riseValLimit = deltaT * _rtP -> P_331 ;
rateLimiterRate = _rtB -> B_40_412_0 - rtb_B_40_3_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_40_413_0 = rtb_B_40_3_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_332 ; if ( rateLimiterRate < deltaT ) { _rtB ->
B_40_413_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB -> B_40_413_0 = _rtB ->
B_40_412_0 ; } } } _rtB -> B_40_414_0 = _rtP -> P_333 * _rtB -> B_40_413_0 ;
_rtB -> B_40_415_0 = _rtP -> P_334 * _rtB -> B_40_414_0 ;
vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model3/Data Store Read" ,
0 ) ; _rtB -> B_40_416_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_hz != 0 )
{ _rtX -> vv0at_CSTATE_hb = _rtB -> B_40_416_0 ; } _rtB -> B_40_417_0 = _rtX
-> vv0at_CSTATE_hb ; _rtB -> B_40_418_0 = muDoubleScalarAbs ( _rtB ->
B_40_417_0 ) ; _rtB -> B_40_419_0 = _rtP -> P_335 [ 0 ] ; _rtB -> B_40_419_0
= _rtB -> B_40_419_0 * _rtB -> B_40_418_0 + _rtP -> P_335 [ 1 ] ; _rtB ->
B_40_419_0 = _rtB -> B_40_419_0 * _rtB -> B_40_418_0 + _rtP -> P_335 [ 2 ] ;
_rtB -> B_40_420_0 = _rtP -> P_336 * _rtB -> B_40_419_0 ; _rtB -> B_40_421_0
= _rtB -> B_40_29_0_g * _rtB -> B_40_420_0 ; _rtB -> B_40_422_0 = _rtB ->
B_40_415_0 - _rtB -> B_40_421_0 ; if ( _rtB -> B_40_422_0 > _rtB ->
B_40_121_0_e ) { _rtB -> B_40_425_0 = _rtB -> B_40_121_0_e ; } else { if (
_rtB -> B_40_422_0 < _rtB -> B_40_122_0_o ) { _rtB -> B_23_1_0 = _rtB ->
B_40_122_0_o ; } else { _rtB -> B_23_1_0 = _rtB -> B_40_422_0 ; } _rtB ->
B_40_425_0 = _rtB -> B_23_1_0 ; } { real_T * * uBuffer = ( real_T * * ) &
_rtDW -> TransportDelay_PWORK_a1 . TUbufferPtrs [ 0 ] ; real_T * * tBuffer =
( real_T * * ) & _rtDW -> TransportDelay_PWORK_a1 . TUbufferPtrs [ 1 ] ;
real_T simTime = ssGetT ( S ) ; real_T tMinusDelay = simTime - _rtP -> P_337
; _rtB -> B_40_426_0 = Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay ,
0.0 , * tBuffer , * uBuffer , _rtDW -> TransportDelay_IWORK_kv .
CircularBufSize , & _rtDW -> TransportDelay_IWORK_kv . Last , _rtDW ->
TransportDelay_IWORK_kv . Tail , _rtDW -> TransportDelay_IWORK_kv . Head ,
_rtP -> P_338 , 0 , ( boolean_T ) ( ssIsMinorTimeStep ( S ) && (
ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) ) ) ; } _rtB -> B_40_427_0 =
0.0 ; _rtB -> B_40_427_0 += _rtP -> P_340 * _rtX -> TransferFcn_CSTATE_j ;
_rtB -> B_40_427_0 += _rtP -> P_341 * _rtB -> B_40_426_0 ; _rtB -> B_40_428_0
= _rtP -> P_342 * _rtB -> B_40_427_0 ; if ( _rtB -> B_40_428_0 > _rtP ->
P_343 ) { _rtB -> B_40_429_0 = _rtP -> P_343 ; } else if ( _rtB -> B_40_428_0
< _rtP -> P_344 ) { _rtB -> B_40_429_0 = _rtP -> P_344 ; } else { _rtB ->
B_40_429_0 = _rtB -> B_40_428_0 ; } if ( ( _rtDW -> LastMajorTimeA_iyq >=
ssGetTaskTime ( S , 0 ) ) && ( _rtDW -> LastMajorTimeB_id >= ssGetTaskTime (
S , 0 ) ) ) { _rtB -> B_40_430_0 = _rtB -> B_40_429_0 ; } else { if ( ( (
_rtDW -> LastMajorTimeA_iyq < _rtDW -> LastMajorTimeB_id ) && ( _rtDW ->
LastMajorTimeB_id < ssGetTaskTime ( S , 0 ) ) ) || ( ( _rtDW ->
LastMajorTimeA_iyq >= _rtDW -> LastMajorTimeB_id ) && ( _rtDW ->
LastMajorTimeA_iyq >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime
( S , 0 ) - _rtDW -> LastMajorTimeB_id ; rtb_B_40_3_0 = _rtDW -> PrevYB_bg ;
} else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_iyq ;
rtb_B_40_3_0 = _rtDW -> PrevYA_i3 ; } riseValLimit = deltaT * _rtP -> P_345 ;
rateLimiterRate = _rtB -> B_40_429_0 - rtb_B_40_3_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_40_430_0 = rtb_B_40_3_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_346 ; if ( rateLimiterRate < deltaT ) { _rtB ->
B_40_430_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB -> B_40_430_0 = _rtB ->
B_40_429_0 ; } } } _rtB -> B_40_431_0 = _rtP -> P_347 * _rtB -> B_40_430_0 ;
_rtB -> B_40_432_0 = _rtP -> P_348 * _rtB -> B_40_431_0 ;
vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model4/Data Store Read" ,
0 ) ; _rtB -> B_40_433_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_k4 != 0 )
{ _rtX -> vv0at_CSTATE_f = _rtB -> B_40_433_0 ; } _rtB -> B_40_434_0 = _rtX
-> vv0at_CSTATE_f ; _rtB -> B_40_435_0 = muDoubleScalarAbs ( _rtB ->
B_40_434_0 ) ; _rtB -> B_40_436_0 = _rtP -> P_349 [ 0 ] ; _rtB -> B_40_436_0
= _rtB -> B_40_436_0 * _rtB -> B_40_435_0 + _rtP -> P_349 [ 1 ] ; _rtB ->
B_40_436_0 = _rtB -> B_40_436_0 * _rtB -> B_40_435_0 + _rtP -> P_349 [ 2 ] ;
_rtB -> B_40_437_0 = _rtP -> P_350 * _rtB -> B_40_436_0 ; _rtB -> B_40_438_0
= _rtB -> B_40_30_0_l * _rtB -> B_40_437_0 ; _rtB -> B_40_439_0 = _rtB ->
B_40_432_0 - _rtB -> B_40_438_0 ; if ( _rtB -> B_40_439_0 > _rtB ->
B_40_124_0_h ) { _rtB -> B_40_442_0 = _rtB -> B_40_124_0_h ; } else { if (
_rtB -> B_40_439_0 < _rtB -> B_40_125_0_l ) { _rtB -> B_24_1_0 = _rtB ->
B_40_125_0_l ; } else { _rtB -> B_24_1_0 = _rtB -> B_40_439_0 ; } _rtB ->
B_40_442_0 = _rtB -> B_24_1_0 ; } { real_T * * uBuffer = ( real_T * * ) &
_rtDW -> TransportDelay_PWORK_n . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_n . TUbufferPtrs [ 1 ] ; real_T
simTime = ssGetT ( S ) ; real_T tMinusDelay = simTime - _rtP -> P_351 ; _rtB
-> B_40_443_0 = Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 ,
* tBuffer , * uBuffer , _rtDW -> TransportDelay_IWORK_ec . CircularBufSize ,
& _rtDW -> TransportDelay_IWORK_ec . Last , _rtDW -> TransportDelay_IWORK_ec
. Tail , _rtDW -> TransportDelay_IWORK_ec . Head , _rtP -> P_352 , 0 , (
boolean_T ) ( ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) ==
ssGetT ( S ) ) ) ) ; } _rtB -> B_40_444_0 = 0.0 ; _rtB -> B_40_444_0 += _rtP
-> P_354 * _rtX -> TransferFcn_CSTATE_h ; _rtB -> B_40_444_0 += _rtP -> P_355
* _rtB -> B_40_443_0 ; _rtB -> B_40_445_0 = _rtP -> P_356 * _rtB ->
B_40_444_0 ; if ( _rtB -> B_40_445_0 > _rtP -> P_357 ) { _rtB -> B_40_446_0 =
_rtP -> P_357 ; } else if ( _rtB -> B_40_445_0 < _rtP -> P_358 ) { _rtB ->
B_40_446_0 = _rtP -> P_358 ; } else { _rtB -> B_40_446_0 = _rtB -> B_40_445_0
; } if ( ( _rtDW -> LastMajorTimeA_k2 >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW
-> LastMajorTimeB_ko >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_40_447_0 =
_rtB -> B_40_446_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_k2 < _rtDW ->
LastMajorTimeB_ko ) && ( _rtDW -> LastMajorTimeB_ko < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_k2 >= _rtDW -> LastMajorTimeB_ko ) && (
_rtDW -> LastMajorTimeA_k2 >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_ko ; rtb_B_40_3_0 = _rtDW
-> PrevYB_av ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_k2 ; rtb_B_40_3_0 = _rtDW -> PrevYA_c ; } riseValLimit =
deltaT * _rtP -> P_359 ; rateLimiterRate = _rtB -> B_40_446_0 - rtb_B_40_3_0
; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_40_447_0 = rtb_B_40_3_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_360 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_40_447_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB ->
B_40_447_0 = _rtB -> B_40_446_0 ; } } } _rtB -> B_40_448_0 = _rtP -> P_361 *
_rtB -> B_40_447_0 ; _rtB -> B_40_449_0 = _rtP -> P_362 * _rtB -> B_40_448_0
; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model/Data Store Read" , 0
) ; _rtB -> B_40_450_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_j4 != 0 ) {
_rtX -> vv0at_CSTATE_lye = _rtB -> B_40_450_0 ; } _rtB -> B_40_451_0 = _rtX
-> vv0at_CSTATE_lye ; _rtB -> B_40_452_0 = muDoubleScalarAbs ( _rtB ->
B_40_451_0 ) ; _rtB -> B_40_453_0 = _rtP -> P_363 [ 0 ] ; _rtB -> B_40_453_0
= _rtB -> B_40_453_0 * _rtB -> B_40_452_0 + _rtP -> P_363 [ 1 ] ; _rtB ->
B_40_453_0 = _rtB -> B_40_453_0 * _rtB -> B_40_452_0 + _rtP -> P_363 [ 2 ] ;
_rtB -> B_40_454_0 = _rtP -> P_364 * _rtB -> B_40_453_0 ; _rtB -> B_40_455_0
= _rtB -> B_40_32_0 * _rtB -> B_40_454_0 ; _rtB -> B_40_456_0 = _rtB ->
B_40_449_0 - _rtB -> B_40_455_0 ; if ( _rtB -> B_40_456_0 > _rtB ->
B_40_127_0_h ) { _rtB -> B_40_459_0 = _rtB -> B_40_127_0_h ; } else { if (
_rtB -> B_40_456_0 < _rtB -> B_40_128_0_m ) { _rtB -> B_25_1_0 = _rtB ->
B_40_128_0_m ; } else { _rtB -> B_25_1_0 = _rtB -> B_40_456_0 ; } _rtB ->
B_40_459_0 = _rtB -> B_25_1_0 ; } { real_T * * uBuffer = ( real_T * * ) &
_rtDW -> TransportDelay_PWORK_gj . TUbufferPtrs [ 0 ] ; real_T * * tBuffer =
( real_T * * ) & _rtDW -> TransportDelay_PWORK_gj . TUbufferPtrs [ 1 ] ;
real_T simTime = ssGetT ( S ) ; real_T tMinusDelay = simTime - _rtP -> P_365
; _rtB -> B_40_460_0 = Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay ,
0.0 , * tBuffer , * uBuffer , _rtDW -> TransportDelay_IWORK_j1 .
CircularBufSize , & _rtDW -> TransportDelay_IWORK_j1 . Last , _rtDW ->
TransportDelay_IWORK_j1 . Tail , _rtDW -> TransportDelay_IWORK_j1 . Head ,
_rtP -> P_366 , 0 , ( boolean_T ) ( ssIsMinorTimeStep ( S ) && (
ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) ) ) ; } _rtB -> B_40_461_0 =
0.0 ; _rtB -> B_40_461_0 += _rtP -> P_368 * _rtX -> TransferFcn_CSTATE_jd ;
_rtB -> B_40_461_0 += _rtP -> P_369 * _rtB -> B_40_460_0 ; _rtB -> B_40_462_0
= _rtP -> P_370 * _rtB -> B_40_461_0 ; if ( _rtB -> B_40_462_0 > _rtP ->
P_371 ) { _rtB -> B_40_463_0 = _rtP -> P_371 ; } else if ( _rtB -> B_40_462_0
< _rtP -> P_372 ) { _rtB -> B_40_463_0 = _rtP -> P_372 ; } else { _rtB ->
B_40_463_0 = _rtB -> B_40_462_0 ; } if ( ( _rtDW -> LastMajorTimeA_e >=
ssGetTaskTime ( S , 0 ) ) && ( _rtDW -> LastMajorTimeB_gmz >= ssGetTaskTime (
S , 0 ) ) ) { _rtB -> B_40_464_0 = _rtB -> B_40_463_0 ; } else { if ( ( (
_rtDW -> LastMajorTimeA_e < _rtDW -> LastMajorTimeB_gmz ) && ( _rtDW ->
LastMajorTimeB_gmz < ssGetTaskTime ( S , 0 ) ) ) || ( ( _rtDW ->
LastMajorTimeA_e >= _rtDW -> LastMajorTimeB_gmz ) && ( _rtDW ->
LastMajorTimeA_e >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime (
S , 0 ) - _rtDW -> LastMajorTimeB_gmz ; rtb_B_40_3_0 = _rtDW -> PrevYB_j ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_e ;
rtb_B_40_3_0 = _rtDW -> PrevYA_ey0 ; } riseValLimit = deltaT * _rtP -> P_373
; rateLimiterRate = _rtB -> B_40_463_0 - rtb_B_40_3_0 ; if ( rateLimiterRate
> riseValLimit ) { _rtB -> B_40_464_0 = rtb_B_40_3_0 + riseValLimit ; } else
{ deltaT *= _rtP -> P_374 ; if ( rateLimiterRate < deltaT ) { _rtB ->
B_40_464_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB -> B_40_464_0 = _rtB ->
B_40_463_0 ; } } } _rtB -> B_40_465_0 = _rtP -> P_375 * _rtB -> B_40_464_0 ;
_rtB -> B_40_466_0 = _rtP -> P_376 * _rtB -> B_40_465_0 ;
vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model1/Data Store Read" ,
0 ) ; _rtB -> B_40_467_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_b != 0 ) {
_rtX -> vv0at_CSTATE_ju = _rtB -> B_40_467_0 ; } _rtB -> B_40_468_0 = _rtX ->
vv0at_CSTATE_ju ; _rtB -> B_40_469_0 = muDoubleScalarAbs ( _rtB -> B_40_468_0
) ; _rtB -> B_40_470_0 = _rtP -> P_377 [ 0 ] ; _rtB -> B_40_470_0 = _rtB ->
B_40_470_0 * _rtB -> B_40_469_0 + _rtP -> P_377 [ 1 ] ; _rtB -> B_40_470_0 =
_rtB -> B_40_470_0 * _rtB -> B_40_469_0 + _rtP -> P_377 [ 2 ] ; _rtB ->
B_40_471_0 = _rtP -> P_378 * _rtB -> B_40_470_0 ; _rtB -> B_40_472_0 = _rtB
-> B_40_33_0 * _rtB -> B_40_471_0 ; _rtB -> B_40_473_0 = _rtB -> B_40_466_0 -
_rtB -> B_40_472_0 ; if ( _rtB -> B_40_473_0 > _rtB -> B_40_130_0_m ) { _rtB
-> B_40_476_0 = _rtB -> B_40_130_0_m ; } else { if ( _rtB -> B_40_473_0 <
_rtB -> B_40_131_0_h ) { _rtB -> B_26_1_0 = _rtB -> B_40_131_0_h ; } else {
_rtB -> B_26_1_0 = _rtB -> B_40_473_0 ; } _rtB -> B_40_476_0 = _rtB ->
B_26_1_0 ; } { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_e . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T *
* ) & _rtDW -> TransportDelay_PWORK_e . TUbufferPtrs [ 1 ] ; real_T simTime =
ssGetT ( S ) ; real_T tMinusDelay = simTime - _rtP -> P_379 ; _rtB ->
B_40_477_0 = Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , *
tBuffer , * uBuffer , _rtDW -> TransportDelay_IWORK_kh . CircularBufSize , &
_rtDW -> TransportDelay_IWORK_kh . Last , _rtDW -> TransportDelay_IWORK_kh .
Tail , _rtDW -> TransportDelay_IWORK_kh . Head , _rtP -> P_380 , 0 , (
boolean_T ) ( ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) ==
ssGetT ( S ) ) ) ) ; } _rtB -> B_40_478_0 = 0.0 ; _rtB -> B_40_478_0 += _rtP
-> P_382 * _rtX -> TransferFcn_CSTATE_jk ; _rtB -> B_40_478_0 += _rtP ->
P_383 * _rtB -> B_40_477_0 ; _rtB -> B_40_479_0 = _rtP -> P_384 * _rtB ->
B_40_478_0 ; if ( _rtB -> B_40_479_0 > _rtP -> P_385 ) { _rtB -> B_40_480_0 =
_rtP -> P_385 ; } else if ( _rtB -> B_40_479_0 < _rtP -> P_386 ) { _rtB ->
B_40_480_0 = _rtP -> P_386 ; } else { _rtB -> B_40_480_0 = _rtB -> B_40_479_0
; } if ( ( _rtDW -> LastMajorTimeA_gp >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW
-> LastMajorTimeB_l5 >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_40_481_0 =
_rtB -> B_40_480_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_gp < _rtDW ->
LastMajorTimeB_l5 ) && ( _rtDW -> LastMajorTimeB_l5 < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_gp >= _rtDW -> LastMajorTimeB_l5 ) && (
_rtDW -> LastMajorTimeA_gp >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_l5 ; rtb_B_40_3_0 = _rtDW
-> PrevYB_l4 ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_gp ; rtb_B_40_3_0 = _rtDW -> PrevYA_b5t ; } riseValLimit =
deltaT * _rtP -> P_387 ; rateLimiterRate = _rtB -> B_40_480_0 - rtb_B_40_3_0
; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_40_481_0 = rtb_B_40_3_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_388 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_40_481_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB ->
B_40_481_0 = _rtB -> B_40_480_0 ; } } } _rtB -> B_40_482_0 = _rtP -> P_389 *
_rtB -> B_40_481_0 ; _rtB -> B_40_483_0 = _rtP -> P_390 * _rtB -> B_40_482_0
; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model2/Data Store Read" ,
0 ) ; _rtB -> B_40_484_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_oi != 0 )
{ _rtX -> vv0at_CSTATE_pt = _rtB -> B_40_484_0 ; } _rtB -> B_40_485_0 = _rtX
-> vv0at_CSTATE_pt ; _rtB -> B_40_486_0 = muDoubleScalarAbs ( _rtB ->
B_40_485_0 ) ; _rtB -> B_40_487_0 = _rtP -> P_391 [ 0 ] ; _rtB -> B_40_487_0
= _rtB -> B_40_487_0 * _rtB -> B_40_486_0 + _rtP -> P_391 [ 1 ] ; _rtB ->
B_40_487_0 = _rtB -> B_40_487_0 * _rtB -> B_40_486_0 + _rtP -> P_391 [ 2 ] ;
_rtB -> B_40_488_0 = _rtP -> P_392 * _rtB -> B_40_487_0 ; _rtB -> B_40_489_0
= _rtB -> B_40_34_0_d * _rtB -> B_40_488_0 ; _rtB -> B_40_490_0 = _rtB ->
B_40_483_0 - _rtB -> B_40_489_0 ; if ( _rtB -> B_40_490_0 > _rtB ->
B_40_133_0_c ) { _rtB -> B_40_493_0 = _rtB -> B_40_133_0_c ; } else { if (
_rtB -> B_40_490_0 < _rtB -> B_40_134_0 ) { _rtB -> B_27_1_0 = _rtB ->
B_40_134_0 ; } else { _rtB -> B_27_1_0 = _rtB -> B_40_490_0 ; } _rtB ->
B_40_493_0 = _rtB -> B_27_1_0 ; } { real_T * * uBuffer = ( real_T * * ) &
_rtDW -> TransportDelay_PWORK_nv . TUbufferPtrs [ 0 ] ; real_T * * tBuffer =
( real_T * * ) & _rtDW -> TransportDelay_PWORK_nv . TUbufferPtrs [ 1 ] ;
real_T simTime = ssGetT ( S ) ; real_T tMinusDelay = simTime - _rtP -> P_393
; _rtB -> B_40_494_0 = Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay ,
0.0 , * tBuffer , * uBuffer , _rtDW -> TransportDelay_IWORK_dt .
CircularBufSize , & _rtDW -> TransportDelay_IWORK_dt . Last , _rtDW ->
TransportDelay_IWORK_dt . Tail , _rtDW -> TransportDelay_IWORK_dt . Head ,
_rtP -> P_394 , 0 , ( boolean_T ) ( ssIsMinorTimeStep ( S ) && (
ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) ) ) ; } _rtB -> B_40_495_0 =
0.0 ; _rtB -> B_40_495_0 += _rtP -> P_396 * _rtX -> TransferFcn_CSTATE_og ;
_rtB -> B_40_495_0 += _rtP -> P_397 * _rtB -> B_40_494_0 ; _rtB -> B_40_496_0
= _rtP -> P_398 * _rtB -> B_40_495_0 ; if ( _rtB -> B_40_496_0 > _rtP ->
P_399 ) { _rtB -> B_40_497_0 = _rtP -> P_399 ; } else if ( _rtB -> B_40_496_0
< _rtP -> P_400 ) { _rtB -> B_40_497_0 = _rtP -> P_400 ; } else { _rtB ->
B_40_497_0 = _rtB -> B_40_496_0 ; } if ( ( _rtDW -> LastMajorTimeA_or >=
ssGetTaskTime ( S , 0 ) ) && ( _rtDW -> LastMajorTimeB_bw >= ssGetTaskTime (
S , 0 ) ) ) { _rtB -> B_40_498_0 = _rtB -> B_40_497_0 ; } else { if ( ( (
_rtDW -> LastMajorTimeA_or < _rtDW -> LastMajorTimeB_bw ) && ( _rtDW ->
LastMajorTimeB_bw < ssGetTaskTime ( S , 0 ) ) ) || ( ( _rtDW ->
LastMajorTimeA_or >= _rtDW -> LastMajorTimeB_bw ) && ( _rtDW ->
LastMajorTimeA_or >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime (
S , 0 ) - _rtDW -> LastMajorTimeB_bw ; rtb_B_40_3_0 = _rtDW -> PrevYB_bs ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_or ;
rtb_B_40_3_0 = _rtDW -> PrevYA_o0 ; } riseValLimit = deltaT * _rtP -> P_401 ;
rateLimiterRate = _rtB -> B_40_497_0 - rtb_B_40_3_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_40_498_0 = rtb_B_40_3_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_402 ; if ( rateLimiterRate < deltaT ) { _rtB ->
B_40_498_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB -> B_40_498_0 = _rtB ->
B_40_497_0 ; } } } _rtB -> B_40_499_0 = _rtP -> P_403 * _rtB -> B_40_498_0 ;
_rtB -> B_40_500_0 = _rtP -> P_404 * _rtB -> B_40_499_0 ;
vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model3/Data Store Read" ,
0 ) ; _rtB -> B_40_501_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_dj != 0 )
{ _rtX -> vv0at_CSTATE_lw = _rtB -> B_40_501_0 ; } _rtB -> B_40_502_0 = _rtX
-> vv0at_CSTATE_lw ; _rtB -> B_40_503_0 = muDoubleScalarAbs ( _rtB ->
B_40_502_0 ) ; _rtB -> B_40_504_0 = _rtP -> P_405 [ 0 ] ; _rtB -> B_40_504_0
= _rtB -> B_40_504_0 * _rtB -> B_40_503_0 + _rtP -> P_405 [ 1 ] ; _rtB ->
B_40_504_0 = _rtB -> B_40_504_0 * _rtB -> B_40_503_0 + _rtP -> P_405 [ 2 ] ;
_rtB -> B_40_505_0 = _rtP -> P_406 * _rtB -> B_40_504_0 ; _rtB -> B_40_506_0
= _rtB -> B_40_35_0_d * _rtB -> B_40_505_0 ; _rtB -> B_40_507_0 = _rtB ->
B_40_500_0 - _rtB -> B_40_506_0 ; if ( _rtB -> B_40_507_0 > _rtB ->
B_40_136_0_k ) { _rtB -> B_40_510_0 = _rtB -> B_40_136_0_k ; } else { if (
_rtB -> B_40_507_0 < _rtB -> B_40_137_0_p ) { _rtB -> B_28_1_0 = _rtB ->
B_40_137_0_p ; } else { _rtB -> B_28_1_0 = _rtB -> B_40_507_0 ; } _rtB ->
B_40_510_0 = _rtB -> B_28_1_0 ; } { real_T * * uBuffer = ( real_T * * ) &
_rtDW -> TransportDelay_PWORK_lj . TUbufferPtrs [ 0 ] ; real_T * * tBuffer =
( real_T * * ) & _rtDW -> TransportDelay_PWORK_lj . TUbufferPtrs [ 1 ] ;
real_T simTime = ssGetT ( S ) ; real_T tMinusDelay = simTime - _rtP -> P_407
; _rtB -> B_40_511_0 = Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay ,
0.0 , * tBuffer , * uBuffer , _rtDW -> TransportDelay_IWORK_f .
CircularBufSize , & _rtDW -> TransportDelay_IWORK_f . Last , _rtDW ->
TransportDelay_IWORK_f . Tail , _rtDW -> TransportDelay_IWORK_f . Head , _rtP
-> P_408 , 0 , ( boolean_T ) ( ssIsMinorTimeStep ( S ) && (
ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) ) ) ; } _rtB -> B_40_512_0 =
0.0 ; _rtB -> B_40_512_0 += _rtP -> P_410 * _rtX -> TransferFcn_CSTATE_na ;
_rtB -> B_40_512_0 += _rtP -> P_411 * _rtB -> B_40_511_0 ; _rtB -> B_40_513_0
= _rtP -> P_412 * _rtB -> B_40_512_0 ; if ( _rtB -> B_40_513_0 > _rtP ->
P_413 ) { _rtB -> B_40_514_0 = _rtP -> P_413 ; } else if ( _rtB -> B_40_513_0
< _rtP -> P_414 ) { _rtB -> B_40_514_0 = _rtP -> P_414 ; } else { _rtB ->
B_40_514_0 = _rtB -> B_40_513_0 ; } if ( ( _rtDW -> LastMajorTimeA_da >=
ssGetTaskTime ( S , 0 ) ) && ( _rtDW -> LastMajorTimeB_en >= ssGetTaskTime (
S , 0 ) ) ) { _rtB -> B_40_515_0 = _rtB -> B_40_514_0 ; } else { if ( ( (
_rtDW -> LastMajorTimeA_da < _rtDW -> LastMajorTimeB_en ) && ( _rtDW ->
LastMajorTimeB_en < ssGetTaskTime ( S , 0 ) ) ) || ( ( _rtDW ->
LastMajorTimeA_da >= _rtDW -> LastMajorTimeB_en ) && ( _rtDW ->
LastMajorTimeA_da >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime (
S , 0 ) - _rtDW -> LastMajorTimeB_en ; rtb_B_40_3_0 = _rtDW -> PrevYB_a0 ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_da ;
rtb_B_40_3_0 = _rtDW -> PrevYA_ek ; } riseValLimit = deltaT * _rtP -> P_415 ;
rateLimiterRate = _rtB -> B_40_514_0 - rtb_B_40_3_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_40_515_0 = rtb_B_40_3_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_416 ; if ( rateLimiterRate < deltaT ) { _rtB ->
B_40_515_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB -> B_40_515_0 = _rtB ->
B_40_514_0 ; } } } _rtB -> B_40_516_0 = _rtP -> P_417 * _rtB -> B_40_515_0 ;
_rtB -> B_40_517_0 = _rtP -> P_418 * _rtB -> B_40_516_0 ;
vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model4/Data Store Read" ,
0 ) ; _rtB -> B_40_518_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_op != 0 )
{ _rtX -> vv0at_CSTATE_d = _rtB -> B_40_518_0 ; } _rtB -> B_40_519_0 = _rtX
-> vv0at_CSTATE_d ; _rtB -> B_40_520_0 = muDoubleScalarAbs ( _rtB ->
B_40_519_0 ) ; _rtB -> B_40_521_0 = _rtP -> P_419 [ 0 ] ; _rtB -> B_40_521_0
= _rtB -> B_40_521_0 * _rtB -> B_40_520_0 + _rtP -> P_419 [ 1 ] ; _rtB ->
B_40_521_0 = _rtB -> B_40_521_0 * _rtB -> B_40_520_0 + _rtP -> P_419 [ 2 ] ;
_rtB -> B_40_522_0 = _rtP -> P_420 * _rtB -> B_40_521_0 ; _rtB -> B_40_523_0
= _rtB -> B_40_36_0_l * _rtB -> B_40_522_0 ; _rtB -> B_40_524_0 = _rtB ->
B_40_517_0 - _rtB -> B_40_523_0 ; if ( _rtB -> B_40_524_0 > _rtB ->
B_40_139_0_p ) { _rtB -> B_40_527_0 = _rtB -> B_40_139_0_p ; } else { if (
_rtB -> B_40_524_0 < _rtB -> B_40_140_0_p ) { _rtB -> B_29_1_0 = _rtB ->
B_40_140_0_p ; } else { _rtB -> B_29_1_0 = _rtB -> B_40_524_0 ; } _rtB ->
B_40_527_0 = _rtB -> B_29_1_0 ; } { real_T * * uBuffer = ( real_T * * ) &
_rtDW -> TransportDelay_PWORK_js . TUbufferPtrs [ 0 ] ; real_T * * tBuffer =
( real_T * * ) & _rtDW -> TransportDelay_PWORK_js . TUbufferPtrs [ 1 ] ;
real_T simTime = ssGetT ( S ) ; real_T tMinusDelay = simTime - _rtP -> P_421
; _rtB -> B_40_528_0 = Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay ,
0.0 , * tBuffer , * uBuffer , _rtDW -> TransportDelay_IWORK_pu .
CircularBufSize , & _rtDW -> TransportDelay_IWORK_pu . Last , _rtDW ->
TransportDelay_IWORK_pu . Tail , _rtDW -> TransportDelay_IWORK_pu . Head ,
_rtP -> P_422 , 0 , ( boolean_T ) ( ssIsMinorTimeStep ( S ) && (
ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) ) ) ; } _rtB -> B_40_529_0 =
0.0 ; _rtB -> B_40_529_0 += _rtP -> P_424 * _rtX -> TransferFcn_CSTATE_o1 ;
_rtB -> B_40_529_0 += _rtP -> P_425 * _rtB -> B_40_528_0 ; _rtB -> B_40_530_0
= _rtP -> P_426 * _rtB -> B_40_529_0 ; if ( _rtB -> B_40_530_0 > _rtP ->
P_427 ) { _rtB -> B_40_531_0 = _rtP -> P_427 ; } else if ( _rtB -> B_40_530_0
< _rtP -> P_428 ) { _rtB -> B_40_531_0 = _rtP -> P_428 ; } else { _rtB ->
B_40_531_0 = _rtB -> B_40_530_0 ; } if ( ( _rtDW -> LastMajorTimeA_p2 >=
ssGetTaskTime ( S , 0 ) ) && ( _rtDW -> LastMajorTimeB_j >= ssGetTaskTime ( S
, 0 ) ) ) { _rtB -> B_40_532_0 = _rtB -> B_40_531_0 ; } else { if ( ( ( _rtDW
-> LastMajorTimeA_p2 < _rtDW -> LastMajorTimeB_j ) && ( _rtDW ->
LastMajorTimeB_j < ssGetTaskTime ( S , 0 ) ) ) || ( ( _rtDW ->
LastMajorTimeA_p2 >= _rtDW -> LastMajorTimeB_j ) && ( _rtDW ->
LastMajorTimeA_p2 >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime (
S , 0 ) - _rtDW -> LastMajorTimeB_j ; rtb_B_40_3_0 = _rtDW -> PrevYB_lk ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_p2 ;
rtb_B_40_3_0 = _rtDW -> PrevYA_cq ; } riseValLimit = deltaT * _rtP -> P_429 ;
rateLimiterRate = _rtB -> B_40_531_0 - rtb_B_40_3_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_40_532_0 = rtb_B_40_3_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_430 ; if ( rateLimiterRate < deltaT ) { _rtB ->
B_40_532_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB -> B_40_532_0 = _rtB ->
B_40_531_0 ; } } } _rtB -> B_40_533_0 = _rtP -> P_431 * _rtB -> B_40_532_0 ;
_rtB -> B_40_534_0 = _rtP -> P_432 * _rtB -> B_40_533_0 ;
vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model/Data Store Read" , 0
) ; _rtB -> B_40_535_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_bo != 0 ) {
_rtX -> vv0at_CSTATE_lt = _rtB -> B_40_535_0 ; } _rtB -> B_40_536_0 = _rtX ->
vv0at_CSTATE_lt ; _rtB -> B_40_537_0 = muDoubleScalarAbs ( _rtB -> B_40_536_0
) ; _rtB -> B_40_538_0 = _rtP -> P_433 [ 0 ] ; _rtB -> B_40_538_0 = _rtB ->
B_40_538_0 * _rtB -> B_40_537_0 + _rtP -> P_433 [ 1 ] ; _rtB -> B_40_538_0 =
_rtB -> B_40_538_0 * _rtB -> B_40_537_0 + _rtP -> P_433 [ 2 ] ; _rtB ->
B_40_539_0 = _rtP -> P_434 * _rtB -> B_40_538_0 ; _rtB -> B_40_540_0 = _rtB
-> B_40_38_0_o * _rtB -> B_40_539_0 ; _rtB -> B_40_541_0 = _rtB -> B_40_534_0
- _rtB -> B_40_540_0 ; if ( _rtB -> B_40_541_0 > _rtB -> B_40_142_0_a ) {
_rtB -> B_40_544_0 = _rtB -> B_40_142_0_a ; } else { if ( _rtB -> B_40_541_0
< _rtB -> B_40_143_0_j ) { _rtB -> B_30_1_0 = _rtB -> B_40_143_0_j ; } else {
_rtB -> B_30_1_0 = _rtB -> B_40_541_0 ; } _rtB -> B_40_544_0 = _rtB ->
B_30_1_0 ; } { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_mg . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T
* * ) & _rtDW -> TransportDelay_PWORK_mg . TUbufferPtrs [ 1 ] ; real_T
simTime = ssGetT ( S ) ; real_T tMinusDelay = simTime - _rtP -> P_435 ; _rtB
-> B_40_545_0 = Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 ,
* tBuffer , * uBuffer , _rtDW -> TransportDelay_IWORK_a . CircularBufSize , &
_rtDW -> TransportDelay_IWORK_a . Last , _rtDW -> TransportDelay_IWORK_a .
Tail , _rtDW -> TransportDelay_IWORK_a . Head , _rtP -> P_436 , 0 , (
boolean_T ) ( ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) ==
ssGetT ( S ) ) ) ) ; } _rtB -> B_40_546_0 = 0.0 ; _rtB -> B_40_546_0 += _rtP
-> P_438 * _rtX -> TransferFcn_CSTATE_fy ; _rtB -> B_40_546_0 += _rtP ->
P_439 * _rtB -> B_40_545_0 ; _rtB -> B_40_547_0 = _rtP -> P_440 * _rtB ->
B_40_546_0 ; if ( _rtB -> B_40_547_0 > _rtP -> P_441 ) { _rtB -> B_40_548_0 =
_rtP -> P_441 ; } else if ( _rtB -> B_40_547_0 < _rtP -> P_442 ) { _rtB ->
B_40_548_0 = _rtP -> P_442 ; } else { _rtB -> B_40_548_0 = _rtB -> B_40_547_0
; } if ( ( _rtDW -> LastMajorTimeA_n >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW
-> LastMajorTimeB_kq >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_40_549_0 =
_rtB -> B_40_548_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_n < _rtDW ->
LastMajorTimeB_kq ) && ( _rtDW -> LastMajorTimeB_kq < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_n >= _rtDW -> LastMajorTimeB_kq ) && (
_rtDW -> LastMajorTimeA_n >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_kq ; rtb_B_40_3_0 = _rtDW
-> PrevYB_al ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_n ; rtb_B_40_3_0 = _rtDW -> PrevYA_k ; } riseValLimit = deltaT
* _rtP -> P_443 ; rateLimiterRate = _rtB -> B_40_548_0 - rtb_B_40_3_0 ; if (
rateLimiterRate > riseValLimit ) { _rtB -> B_40_549_0 = rtb_B_40_3_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_444 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_40_549_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB ->
B_40_549_0 = _rtB -> B_40_548_0 ; } } } _rtB -> B_40_550_0 = _rtP -> P_445 *
_rtB -> B_40_549_0 ; _rtB -> B_40_551_0 = _rtP -> P_446 * _rtB -> B_40_550_0
; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model1/Data Store Read" ,
0 ) ; _rtB -> B_40_552_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_f2 != 0 )
{ _rtX -> vv0at_CSTATE_h2 = _rtB -> B_40_552_0 ; } _rtB -> B_40_553_0 = _rtX
-> vv0at_CSTATE_h2 ; _rtB -> B_40_554_0 = muDoubleScalarAbs ( _rtB ->
B_40_553_0 ) ; _rtB -> B_40_555_0 = _rtP -> P_447 [ 0 ] ; _rtB -> B_40_555_0
= _rtB -> B_40_555_0 * _rtB -> B_40_554_0 + _rtP -> P_447 [ 1 ] ; _rtB ->
B_40_555_0 = _rtB -> B_40_555_0 * _rtB -> B_40_554_0 + _rtP -> P_447 [ 2 ] ;
_rtB -> B_40_556_0 = _rtP -> P_448 * _rtB -> B_40_555_0 ; _rtB -> B_40_557_0
= _rtB -> B_40_39_0_b * _rtB -> B_40_556_0 ; _rtB -> B_40_558_0 = _rtB ->
B_40_551_0 - _rtB -> B_40_557_0 ; if ( _rtB -> B_40_558_0 > _rtB ->
B_40_145_0_e ) { _rtB -> B_40_561_0 = _rtB -> B_40_145_0_e ; } else { if (
_rtB -> B_40_558_0 < _rtB -> B_40_146_0_o ) { _rtB -> B_31_1_0 = _rtB ->
B_40_146_0_o ; } else { _rtB -> B_31_1_0 = _rtB -> B_40_558_0 ; } _rtB ->
B_40_561_0 = _rtB -> B_31_1_0 ; } { real_T * * uBuffer = ( real_T * * ) &
_rtDW -> TransportDelay_PWORK_f . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_f . TUbufferPtrs [ 1 ] ; real_T
simTime = ssGetT ( S ) ; real_T tMinusDelay = simTime - _rtP -> P_449 ; _rtB
-> B_40_562_0 = Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 ,
* tBuffer , * uBuffer , _rtDW -> TransportDelay_IWORK_i3 . CircularBufSize ,
& _rtDW -> TransportDelay_IWORK_i3 . Last , _rtDW -> TransportDelay_IWORK_i3
. Tail , _rtDW -> TransportDelay_IWORK_i3 . Head , _rtP -> P_450 , 0 , (
boolean_T ) ( ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) ==
ssGetT ( S ) ) ) ) ; } _rtB -> B_40_563_0 = 0.0 ; _rtB -> B_40_563_0 += _rtP
-> P_452 * _rtX -> TransferFcn_CSTATE_pk ; _rtB -> B_40_563_0 += _rtP ->
P_453 * _rtB -> B_40_562_0 ; _rtB -> B_40_564_0 = _rtP -> P_454 * _rtB ->
B_40_563_0 ; if ( _rtB -> B_40_564_0 > _rtP -> P_455 ) { _rtB -> B_40_565_0 =
_rtP -> P_455 ; } else if ( _rtB -> B_40_564_0 < _rtP -> P_456 ) { _rtB ->
B_40_565_0 = _rtP -> P_456 ; } else { _rtB -> B_40_565_0 = _rtB -> B_40_564_0
; } if ( ( _rtDW -> LastMajorTimeA_jq >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW
-> LastMajorTimeB_lj >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_40_566_0 =
_rtB -> B_40_565_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_jq < _rtDW ->
LastMajorTimeB_lj ) && ( _rtDW -> LastMajorTimeB_lj < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_jq >= _rtDW -> LastMajorTimeB_lj ) && (
_rtDW -> LastMajorTimeA_jq >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_lj ; rtb_B_40_3_0 = _rtDW
-> PrevYB_pq ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_jq ; rtb_B_40_3_0 = _rtDW -> PrevYA_ce ; } riseValLimit =
deltaT * _rtP -> P_457 ; rateLimiterRate = _rtB -> B_40_565_0 - rtb_B_40_3_0
; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_40_566_0 = rtb_B_40_3_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_458 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_40_566_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB ->
B_40_566_0 = _rtB -> B_40_565_0 ; } } } _rtB -> B_40_567_0 = _rtP -> P_459 *
_rtB -> B_40_566_0 ; _rtB -> B_40_568_0 = _rtP -> P_460 * _rtB -> B_40_567_0
; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model2/Data Store Read" ,
0 ) ; _rtB -> B_40_569_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_aw != 0 )
{ _rtX -> vv0at_CSTATE_hi = _rtB -> B_40_569_0 ; } _rtB -> B_40_570_0 = _rtX
-> vv0at_CSTATE_hi ; _rtB -> B_40_571_0 = muDoubleScalarAbs ( _rtB ->
B_40_570_0 ) ; _rtB -> B_40_572_0 = _rtP -> P_461 [ 0 ] ; _rtB -> B_40_572_0
= _rtB -> B_40_572_0 * _rtB -> B_40_571_0 + _rtP -> P_461 [ 1 ] ; _rtB ->
B_40_572_0 = _rtB -> B_40_572_0 * _rtB -> B_40_571_0 + _rtP -> P_461 [ 2 ] ;
_rtB -> B_40_573_0 = _rtP -> P_462 * _rtB -> B_40_572_0 ; _rtB -> B_40_574_0
= _rtB -> B_40_40_0_n * _rtB -> B_40_573_0 ; _rtB -> B_40_575_0 = _rtB ->
B_40_568_0 - _rtB -> B_40_574_0 ; if ( _rtB -> B_40_575_0 > _rtB ->
B_40_148_0_b ) { _rtB -> B_40_578_0 = _rtB -> B_40_148_0_b ; } else { if (
_rtB -> B_40_575_0 < _rtB -> B_40_149_0_a ) { _rtB -> B_32_1_0 = _rtB ->
B_40_149_0_a ; } else { _rtB -> B_32_1_0 = _rtB -> B_40_575_0 ; } _rtB ->
B_40_578_0 = _rtB -> B_32_1_0 ; } { real_T * * uBuffer = ( real_T * * ) &
_rtDW -> TransportDelay_PWORK_eb . TUbufferPtrs [ 0 ] ; real_T * * tBuffer =
( real_T * * ) & _rtDW -> TransportDelay_PWORK_eb . TUbufferPtrs [ 1 ] ;
real_T simTime = ssGetT ( S ) ; real_T tMinusDelay = simTime - _rtP -> P_463
; _rtB -> B_40_579_0 = Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay ,
0.0 , * tBuffer , * uBuffer , _rtDW -> TransportDelay_IWORK_h .
CircularBufSize , & _rtDW -> TransportDelay_IWORK_h . Last , _rtDW ->
TransportDelay_IWORK_h . Tail , _rtDW -> TransportDelay_IWORK_h . Head , _rtP
-> P_464 , 0 , ( boolean_T ) ( ssIsMinorTimeStep ( S ) && (
ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) ) ) ; } _rtB -> B_40_580_0 =
0.0 ; _rtB -> B_40_580_0 += _rtP -> P_466 * _rtX -> TransferFcn_CSTATE_hj ;
_rtB -> B_40_580_0 += _rtP -> P_467 * _rtB -> B_40_579_0 ; _rtB -> B_40_581_0
= _rtP -> P_468 * _rtB -> B_40_580_0 ; if ( _rtB -> B_40_581_0 > _rtP ->
P_469 ) { _rtB -> B_40_582_0 = _rtP -> P_469 ; } else if ( _rtB -> B_40_581_0
< _rtP -> P_470 ) { _rtB -> B_40_582_0 = _rtP -> P_470 ; } else { _rtB ->
B_40_582_0 = _rtB -> B_40_581_0 ; } if ( ( _rtDW -> LastMajorTimeA_h >=
ssGetTaskTime ( S , 0 ) ) && ( _rtDW -> LastMajorTimeB_oj >= ssGetTaskTime (
S , 0 ) ) ) { _rtB -> B_40_583_0 = _rtB -> B_40_582_0 ; } else { if ( ( (
_rtDW -> LastMajorTimeA_h < _rtDW -> LastMajorTimeB_oj ) && ( _rtDW ->
LastMajorTimeB_oj < ssGetTaskTime ( S , 0 ) ) ) || ( ( _rtDW ->
LastMajorTimeA_h >= _rtDW -> LastMajorTimeB_oj ) && ( _rtDW ->
LastMajorTimeA_h >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime (
S , 0 ) - _rtDW -> LastMajorTimeB_oj ; rtb_B_40_3_0 = _rtDW -> PrevYB_p0 ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_h ;
rtb_B_40_3_0 = _rtDW -> PrevYA_da ; } riseValLimit = deltaT * _rtP -> P_471 ;
rateLimiterRate = _rtB -> B_40_582_0 - rtb_B_40_3_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_40_583_0 = rtb_B_40_3_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_472 ; if ( rateLimiterRate < deltaT ) { _rtB ->
B_40_583_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB -> B_40_583_0 = _rtB ->
B_40_582_0 ; } } } _rtB -> B_40_584_0 = _rtP -> P_473 * _rtB -> B_40_583_0 ;
_rtB -> B_40_585_0 = _rtP -> P_474 * _rtB -> B_40_584_0 ;
vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model3/Data Store Read" ,
0 ) ; _rtB -> B_40_586_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_o3 != 0 )
{ _rtX -> vv0at_CSTATE_b5 = _rtB -> B_40_586_0 ; } _rtB -> B_40_587_0 = _rtX
-> vv0at_CSTATE_b5 ; _rtB -> B_40_588_0 = muDoubleScalarAbs ( _rtB ->
B_40_587_0 ) ; _rtB -> B_40_589_0 = _rtP -> P_475 [ 0 ] ; _rtB -> B_40_589_0
= _rtB -> B_40_589_0 * _rtB -> B_40_588_0 + _rtP -> P_475 [ 1 ] ; _rtB ->
B_40_589_0 = _rtB -> B_40_589_0 * _rtB -> B_40_588_0 + _rtP -> P_475 [ 2 ] ;
_rtB -> B_40_590_0 = _rtP -> P_476 * _rtB -> B_40_589_0 ; _rtB -> B_40_591_0
= _rtB -> B_40_41_0_b * _rtB -> B_40_590_0 ; _rtB -> B_40_592_0 = _rtB ->
B_40_585_0 - _rtB -> B_40_591_0 ; if ( _rtB -> B_40_592_0 > _rtB ->
B_40_151_0 ) { _rtB -> B_40_595_0 = _rtB -> B_40_151_0 ; } else { if ( _rtB
-> B_40_592_0 < _rtB -> B_40_152_0 ) { _rtB -> B_33_1_0 = _rtB -> B_40_152_0
; } else { _rtB -> B_33_1_0 = _rtB -> B_40_592_0 ; } _rtB -> B_40_595_0 =
_rtB -> B_33_1_0 ; } { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_ks . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T
* * ) & _rtDW -> TransportDelay_PWORK_ks . TUbufferPtrs [ 1 ] ; real_T
simTime = ssGetT ( S ) ; real_T tMinusDelay = simTime - _rtP -> P_477 ; _rtB
-> B_40_596_0 = Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 ,
* tBuffer , * uBuffer , _rtDW -> TransportDelay_IWORK_o4 . CircularBufSize ,
& _rtDW -> TransportDelay_IWORK_o4 . Last , _rtDW -> TransportDelay_IWORK_o4
. Tail , _rtDW -> TransportDelay_IWORK_o4 . Head , _rtP -> P_478 , 0 , (
boolean_T ) ( ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) ==
ssGetT ( S ) ) ) ) ; } _rtB -> B_40_597_0 = 0.0 ; _rtB -> B_40_597_0 += _rtP
-> P_480 * _rtX -> TransferFcn_CSTATE_of ; _rtB -> B_40_597_0 += _rtP ->
P_481 * _rtB -> B_40_596_0 ; _rtB -> B_40_598_0 = _rtP -> P_482 * _rtB ->
B_40_597_0 ; if ( _rtB -> B_40_598_0 > _rtP -> P_483 ) { _rtB -> B_40_599_0 =
_rtP -> P_483 ; } else if ( _rtB -> B_40_598_0 < _rtP -> P_484 ) { _rtB ->
B_40_599_0 = _rtP -> P_484 ; } else { _rtB -> B_40_599_0 = _rtB -> B_40_598_0
; } if ( ( _rtDW -> LastMajorTimeA_e1 >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW
-> LastMajorTimeB_ie >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_40_600_0 =
_rtB -> B_40_599_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_e1 < _rtDW ->
LastMajorTimeB_ie ) && ( _rtDW -> LastMajorTimeB_ie < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_e1 >= _rtDW -> LastMajorTimeB_ie ) && (
_rtDW -> LastMajorTimeA_e1 >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_ie ; rtb_B_40_3_0 = _rtDW
-> PrevYB_jf ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_e1 ; rtb_B_40_3_0 = _rtDW -> PrevYA_cy ; } riseValLimit =
deltaT * _rtP -> P_485 ; rateLimiterRate = _rtB -> B_40_599_0 - rtb_B_40_3_0
; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_40_600_0 = rtb_B_40_3_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_486 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_40_600_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB ->
B_40_600_0 = _rtB -> B_40_599_0 ; } } } _rtB -> B_40_601_0 = _rtP -> P_487 *
_rtB -> B_40_600_0 ; _rtB -> B_40_602_0 = _rtP -> P_488 * _rtB -> B_40_601_0
; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model4/Data Store Read" ,
0 ) ; _rtB -> B_40_603_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_gf != 0 )
{ _rtX -> vv0at_CSTATE_b3 = _rtB -> B_40_603_0 ; } _rtB -> B_40_604_0 = _rtX
-> vv0at_CSTATE_b3 ; _rtB -> B_40_605_0 = muDoubleScalarAbs ( _rtB ->
B_40_604_0 ) ; _rtB -> B_40_606_0 = _rtP -> P_489 [ 0 ] ; _rtB -> B_40_606_0
= _rtB -> B_40_606_0 * _rtB -> B_40_605_0 + _rtP -> P_489 [ 1 ] ; _rtB ->
B_40_606_0 = _rtB -> B_40_606_0 * _rtB -> B_40_605_0 + _rtP -> P_489 [ 2 ] ;
_rtB -> B_40_607_0 = _rtP -> P_490 * _rtB -> B_40_606_0 ; _rtB -> B_40_608_0
= _rtB -> B_40_42_0_l * _rtB -> B_40_607_0 ; _rtB -> B_40_609_0 = _rtB ->
B_40_602_0 - _rtB -> B_40_608_0 ; if ( _rtB -> B_40_609_0 > _rtB ->
B_40_154_0_g ) { _rtB -> B_40_612_0 = _rtB -> B_40_154_0_g ; } else { if (
_rtB -> B_40_609_0 < _rtB -> B_40_155_0_e ) { _rtB -> B_34_1_0 = _rtB ->
B_40_155_0_e ; } else { _rtB -> B_34_1_0 = _rtB -> B_40_609_0 ; } _rtB ->
B_40_612_0 = _rtB -> B_34_1_0 ; } { real_T * * uBuffer = ( real_T * * ) &
_rtDW -> TransportDelay_PWORK_b5 . TUbufferPtrs [ 0 ] ; real_T * * tBuffer =
( real_T * * ) & _rtDW -> TransportDelay_PWORK_b5 . TUbufferPtrs [ 1 ] ;
real_T simTime = ssGetT ( S ) ; real_T tMinusDelay = simTime - _rtP -> P_491
; _rtB -> B_40_613_0 = Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay ,
0.0 , * tBuffer , * uBuffer , _rtDW -> TransportDelay_IWORK_dy .
CircularBufSize , & _rtDW -> TransportDelay_IWORK_dy . Last , _rtDW ->
TransportDelay_IWORK_dy . Tail , _rtDW -> TransportDelay_IWORK_dy . Head ,
_rtP -> P_492 , 0 , ( boolean_T ) ( ssIsMinorTimeStep ( S ) && (
ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) ) ) ; } _rtB -> B_40_614_0 =
0.0 ; _rtB -> B_40_614_0 += _rtP -> P_494 * _rtX -> TransferFcn_CSTATE_g4 ;
_rtB -> B_40_614_0 += _rtP -> P_495 * _rtB -> B_40_613_0 ; _rtB -> B_40_615_0
= _rtP -> P_496 * _rtB -> B_40_614_0 ; if ( _rtB -> B_40_615_0 > _rtP ->
P_497 ) { _rtB -> B_40_616_0 = _rtP -> P_497 ; } else if ( _rtB -> B_40_615_0
< _rtP -> P_498 ) { _rtB -> B_40_616_0 = _rtP -> P_498 ; } else { _rtB ->
B_40_616_0 = _rtB -> B_40_615_0 ; } if ( ( _rtDW -> LastMajorTimeA_a >=
ssGetTaskTime ( S , 0 ) ) && ( _rtDW -> LastMajorTimeB_ih >= ssGetTaskTime (
S , 0 ) ) ) { _rtB -> B_40_617_0 = _rtB -> B_40_616_0 ; } else { if ( ( (
_rtDW -> LastMajorTimeA_a < _rtDW -> LastMajorTimeB_ih ) && ( _rtDW ->
LastMajorTimeB_ih < ssGetTaskTime ( S , 0 ) ) ) || ( ( _rtDW ->
LastMajorTimeA_a >= _rtDW -> LastMajorTimeB_ih ) && ( _rtDW ->
LastMajorTimeA_a >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime (
S , 0 ) - _rtDW -> LastMajorTimeB_ih ; rtb_B_40_3_0 = _rtDW -> PrevYB_cc ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_a ;
rtb_B_40_3_0 = _rtDW -> PrevYA_gg ; } riseValLimit = deltaT * _rtP -> P_499 ;
rateLimiterRate = _rtB -> B_40_616_0 - rtb_B_40_3_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_40_617_0 = rtb_B_40_3_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_500 ; if ( rateLimiterRate < deltaT ) { _rtB ->
B_40_617_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB -> B_40_617_0 = _rtB ->
B_40_616_0 ; } } } _rtB -> B_40_618_0 = _rtP -> P_501 * _rtB -> B_40_617_0 ;
_rtB -> B_40_619_0 = _rtP -> P_502 * _rtB -> B_40_618_0 ;
vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model/Data Store Read" , 0
) ; _rtB -> B_40_620_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_j0 != 0 ) {
_rtX -> vv0at_CSTATE_bz = _rtB -> B_40_620_0 ; } _rtB -> B_40_621_0 = _rtX ->
vv0at_CSTATE_bz ; _rtB -> B_40_622_0 = muDoubleScalarAbs ( _rtB -> B_40_621_0
) ; _rtB -> B_40_623_0 = _rtP -> P_503 [ 0 ] ; _rtB -> B_40_623_0 = _rtB ->
B_40_623_0 * _rtB -> B_40_622_0 + _rtP -> P_503 [ 1 ] ; _rtB -> B_40_623_0 =
_rtB -> B_40_623_0 * _rtB -> B_40_622_0 + _rtP -> P_503 [ 2 ] ; _rtB ->
B_40_624_0 = _rtP -> P_504 * _rtB -> B_40_623_0 ; _rtB -> B_40_625_0 = _rtB
-> B_40_44_0_h * _rtB -> B_40_624_0 ; _rtB -> B_40_626_0 = _rtB -> B_40_619_0
- _rtB -> B_40_625_0 ; if ( _rtB -> B_40_626_0 > _rtB -> B_40_157_0_f ) {
_rtB -> B_40_629_0 = _rtB -> B_40_157_0_f ; } else { if ( _rtB -> B_40_626_0
< _rtB -> B_40_158_0_h ) { _rtB -> B_35_1_0 = _rtB -> B_40_158_0_h ; } else {
_rtB -> B_35_1_0 = _rtB -> B_40_626_0 ; } _rtB -> B_40_629_0 = _rtB ->
B_35_1_0 ; } { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_i5 . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T
* * ) & _rtDW -> TransportDelay_PWORK_i5 . TUbufferPtrs [ 1 ] ; real_T
simTime = ssGetT ( S ) ; real_T tMinusDelay = simTime - _rtP -> P_505 ; _rtB
-> B_40_630_0 = Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 ,
* tBuffer , * uBuffer , _rtDW -> TransportDelay_IWORK_ek . CircularBufSize ,
& _rtDW -> TransportDelay_IWORK_ek . Last , _rtDW -> TransportDelay_IWORK_ek
. Tail , _rtDW -> TransportDelay_IWORK_ek . Head , _rtP -> P_506 , 0 , (
boolean_T ) ( ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) ==
ssGetT ( S ) ) ) ) ; } _rtB -> B_40_631_0 = 0.0 ; _rtB -> B_40_631_0 += _rtP
-> P_508 * _rtX -> TransferFcn_CSTATE_a ; _rtB -> B_40_631_0 += _rtP -> P_509
* _rtB -> B_40_630_0 ; _rtB -> B_40_632_0 = _rtP -> P_510 * _rtB ->
B_40_631_0 ; if ( _rtB -> B_40_632_0 > _rtP -> P_511 ) { _rtB -> B_40_633_0 =
_rtP -> P_511 ; } else if ( _rtB -> B_40_632_0 < _rtP -> P_512 ) { _rtB ->
B_40_633_0 = _rtP -> P_512 ; } else { _rtB -> B_40_633_0 = _rtB -> B_40_632_0
; } if ( ( _rtDW -> LastMajorTimeA_chy >= ssGetTaskTime ( S , 0 ) ) && (
_rtDW -> LastMajorTimeB_msx >= ssGetTaskTime ( S , 0 ) ) ) { _rtB ->
B_40_634_0 = _rtB -> B_40_633_0 ; } else { if ( ( ( _rtDW ->
LastMajorTimeA_chy < _rtDW -> LastMajorTimeB_msx ) && ( _rtDW ->
LastMajorTimeB_msx < ssGetTaskTime ( S , 0 ) ) ) || ( ( _rtDW ->
LastMajorTimeA_chy >= _rtDW -> LastMajorTimeB_msx ) && ( _rtDW ->
LastMajorTimeA_chy >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime
( S , 0 ) - _rtDW -> LastMajorTimeB_msx ; rtb_B_40_3_0 = _rtDW -> PrevYB_h5 ;
} else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_chy ;
rtb_B_40_3_0 = _rtDW -> PrevYA_dp ; } riseValLimit = deltaT * _rtP -> P_513 ;
rateLimiterRate = _rtB -> B_40_633_0 - rtb_B_40_3_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_40_634_0 = rtb_B_40_3_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_514 ; if ( rateLimiterRate < deltaT ) { _rtB ->
B_40_634_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB -> B_40_634_0 = _rtB ->
B_40_633_0 ; } } } _rtB -> B_40_635_0 = _rtP -> P_515 * _rtB -> B_40_634_0 ;
_rtB -> B_40_636_0 = _rtP -> P_516 * _rtB -> B_40_635_0 ;
vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model1/Data Store Read" ,
0 ) ; _rtB -> B_40_637_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_og != 0 )
{ _rtX -> vv0at_CSTATE_cc = _rtB -> B_40_637_0 ; } _rtB -> B_40_638_0 = _rtX
-> vv0at_CSTATE_cc ; _rtB -> B_40_639_0 = muDoubleScalarAbs ( _rtB ->
B_40_638_0 ) ; _rtB -> B_40_640_0 = _rtP -> P_517 [ 0 ] ; _rtB -> B_40_640_0
= _rtB -> B_40_640_0 * _rtB -> B_40_639_0 + _rtP -> P_517 [ 1 ] ; _rtB ->
B_40_640_0 = _rtB -> B_40_640_0 * _rtB -> B_40_639_0 + _rtP -> P_517 [ 2 ] ;
_rtB -> B_40_641_0 = _rtP -> P_518 * _rtB -> B_40_640_0 ; _rtB -> B_40_642_0
= _rtB -> B_40_45_0_b * _rtB -> B_40_641_0 ; _rtB -> B_40_643_0 = _rtB ->
B_40_636_0 - _rtB -> B_40_642_0 ; if ( _rtB -> B_40_643_0 > _rtB ->
B_40_160_0_e ) { _rtB -> B_40_646_0 = _rtB -> B_40_160_0_e ; } else { if (
_rtB -> B_40_643_0 < _rtB -> B_40_161_0_c ) { _rtB -> B_36_1_0 = _rtB ->
B_40_161_0_c ; } else { _rtB -> B_36_1_0 = _rtB -> B_40_643_0 ; } _rtB ->
B_40_646_0 = _rtB -> B_36_1_0 ; } { real_T * * uBuffer = ( real_T * * ) &
_rtDW -> TransportDelay_PWORK_am . TUbufferPtrs [ 0 ] ; real_T * * tBuffer =
( real_T * * ) & _rtDW -> TransportDelay_PWORK_am . TUbufferPtrs [ 1 ] ;
real_T simTime = ssGetT ( S ) ; real_T tMinusDelay = simTime - _rtP -> P_519
; _rtB -> B_40_647_0 = Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay ,
0.0 , * tBuffer , * uBuffer , _rtDW -> TransportDelay_IWORK_cus .
CircularBufSize , & _rtDW -> TransportDelay_IWORK_cus . Last , _rtDW ->
TransportDelay_IWORK_cus . Tail , _rtDW -> TransportDelay_IWORK_cus . Head ,
_rtP -> P_520 , 0 , ( boolean_T ) ( ssIsMinorTimeStep ( S ) && (
ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) ) ) ; } _rtB -> B_40_648_0 =
0.0 ; _rtB -> B_40_648_0 += _rtP -> P_522 * _rtX -> TransferFcn_CSTATE_lp ;
_rtB -> B_40_648_0 += _rtP -> P_523 * _rtB -> B_40_647_0 ; _rtB -> B_40_649_0
= _rtP -> P_524 * _rtB -> B_40_648_0 ; if ( _rtB -> B_40_649_0 > _rtP ->
P_525 ) { _rtB -> B_40_650_0 = _rtP -> P_525 ; } else if ( _rtB -> B_40_649_0
< _rtP -> P_526 ) { _rtB -> B_40_650_0 = _rtP -> P_526 ; } else { _rtB ->
B_40_650_0 = _rtB -> B_40_649_0 ; } if ( ( _rtDW -> LastMajorTimeA_fk >=
ssGetTaskTime ( S , 0 ) ) && ( _rtDW -> LastMajorTimeB_kn >= ssGetTaskTime (
S , 0 ) ) ) { _rtB -> B_40_651_0 = _rtB -> B_40_650_0 ; } else { if ( ( (
_rtDW -> LastMajorTimeA_fk < _rtDW -> LastMajorTimeB_kn ) && ( _rtDW ->
LastMajorTimeB_kn < ssGetTaskTime ( S , 0 ) ) ) || ( ( _rtDW ->
LastMajorTimeA_fk >= _rtDW -> LastMajorTimeB_kn ) && ( _rtDW ->
LastMajorTimeA_fk >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime (
S , 0 ) - _rtDW -> LastMajorTimeB_kn ; rtb_B_40_3_0 = _rtDW -> PrevYB_bt ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_fk ;
rtb_B_40_3_0 = _rtDW -> PrevYA_bq ; } riseValLimit = deltaT * _rtP -> P_527 ;
rateLimiterRate = _rtB -> B_40_650_0 - rtb_B_40_3_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_40_651_0 = rtb_B_40_3_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_528 ; if ( rateLimiterRate < deltaT ) { _rtB ->
B_40_651_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB -> B_40_651_0 = _rtB ->
B_40_650_0 ; } } } _rtB -> B_40_652_0 = _rtP -> P_529 * _rtB -> B_40_651_0 ;
_rtB -> B_40_653_0 = _rtP -> P_530 * _rtB -> B_40_652_0 ;
vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model2/Data Store Read" ,
0 ) ; _rtB -> B_40_654_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_ia != 0 )
{ _rtX -> vv0at_CSTATE_hl = _rtB -> B_40_654_0 ; } _rtB -> B_40_655_0 = _rtX
-> vv0at_CSTATE_hl ; _rtB -> B_40_656_0 = muDoubleScalarAbs ( _rtB ->
B_40_655_0 ) ; _rtB -> B_40_657_0 = _rtP -> P_531 [ 0 ] ; _rtB -> B_40_657_0
= _rtB -> B_40_657_0 * _rtB -> B_40_656_0 + _rtP -> P_531 [ 1 ] ; _rtB ->
B_40_657_0 = _rtB -> B_40_657_0 * _rtB -> B_40_656_0 + _rtP -> P_531 [ 2 ] ;
_rtB -> B_40_658_0 = _rtP -> P_532 * _rtB -> B_40_657_0 ; _rtB -> B_40_659_0
= _rtB -> B_40_46_0_d * _rtB -> B_40_658_0 ; _rtB -> B_40_660_0 = _rtB ->
B_40_653_0 - _rtB -> B_40_659_0 ; if ( _rtB -> B_40_660_0 > _rtB ->
B_40_163_0_a ) { _rtB -> B_40_663_0 = _rtB -> B_40_163_0_a ; } else { if (
_rtB -> B_40_660_0 < _rtB -> B_40_164_0_d ) { _rtB -> B_37_1_0 = _rtB ->
B_40_164_0_d ; } else { _rtB -> B_37_1_0 = _rtB -> B_40_660_0 ; } _rtB ->
B_40_663_0 = _rtB -> B_37_1_0 ; } { real_T * * uBuffer = ( real_T * * ) &
_rtDW -> TransportDelay_PWORK_pb . TUbufferPtrs [ 0 ] ; real_T * * tBuffer =
( real_T * * ) & _rtDW -> TransportDelay_PWORK_pb . TUbufferPtrs [ 1 ] ;
real_T simTime = ssGetT ( S ) ; real_T tMinusDelay = simTime - _rtP -> P_533
; _rtB -> B_40_664_0 = Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay ,
0.0 , * tBuffer , * uBuffer , _rtDW -> TransportDelay_IWORK_bob .
CircularBufSize , & _rtDW -> TransportDelay_IWORK_bob . Last , _rtDW ->
TransportDelay_IWORK_bob . Tail , _rtDW -> TransportDelay_IWORK_bob . Head ,
_rtP -> P_534 , 0 , ( boolean_T ) ( ssIsMinorTimeStep ( S ) && (
ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) ) ) ; } _rtB -> B_40_665_0 =
0.0 ; _rtB -> B_40_665_0 += _rtP -> P_536 * _rtX -> TransferFcn_CSTATE_lt ;
_rtB -> B_40_665_0 += _rtP -> P_537 * _rtB -> B_40_664_0 ; _rtB -> B_40_666_0
= _rtP -> P_538 * _rtB -> B_40_665_0 ; if ( _rtB -> B_40_666_0 > _rtP ->
P_539 ) { _rtB -> B_40_667_0 = _rtP -> P_539 ; } else if ( _rtB -> B_40_666_0
< _rtP -> P_540 ) { _rtB -> B_40_667_0 = _rtP -> P_540 ; } else { _rtB ->
B_40_667_0 = _rtB -> B_40_666_0 ; } if ( ( _rtDW -> LastMajorTimeA_n5 >=
ssGetTaskTime ( S , 0 ) ) && ( _rtDW -> LastMajorTimeB_mq >= ssGetTaskTime (
S , 0 ) ) ) { _rtB -> B_40_668_0 = _rtB -> B_40_667_0 ; } else { if ( ( (
_rtDW -> LastMajorTimeA_n5 < _rtDW -> LastMajorTimeB_mq ) && ( _rtDW ->
LastMajorTimeB_mq < ssGetTaskTime ( S , 0 ) ) ) || ( ( _rtDW ->
LastMajorTimeA_n5 >= _rtDW -> LastMajorTimeB_mq ) && ( _rtDW ->
LastMajorTimeA_n5 >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime (
S , 0 ) - _rtDW -> LastMajorTimeB_mq ; rtb_B_40_3_0 = _rtDW -> PrevYB_mq ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_n5 ;
rtb_B_40_3_0 = _rtDW -> PrevYA_ls ; } riseValLimit = deltaT * _rtP -> P_541 ;
rateLimiterRate = _rtB -> B_40_667_0 - rtb_B_40_3_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_40_668_0 = rtb_B_40_3_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_542 ; if ( rateLimiterRate < deltaT ) { _rtB ->
B_40_668_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB -> B_40_668_0 = _rtB ->
B_40_667_0 ; } } } _rtB -> B_40_669_0 = _rtP -> P_543 * _rtB -> B_40_668_0 ;
_rtB -> B_40_670_0 = _rtP -> P_544 * _rtB -> B_40_669_0 ;
vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model3/Data Store Read" ,
0 ) ; _rtB -> B_40_671_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_id != 0 )
{ _rtX -> vv0at_CSTATE_m = _rtB -> B_40_671_0 ; } _rtB -> B_40_672_0 = _rtX
-> vv0at_CSTATE_m ; _rtB -> B_40_673_0 = muDoubleScalarAbs ( _rtB ->
B_40_672_0 ) ; _rtB -> B_40_674_0 = _rtP -> P_545 [ 0 ] ; _rtB -> B_40_674_0
= _rtB -> B_40_674_0 * _rtB -> B_40_673_0 + _rtP -> P_545 [ 1 ] ; _rtB ->
B_40_674_0 = _rtB -> B_40_674_0 * _rtB -> B_40_673_0 + _rtP -> P_545 [ 2 ] ;
_rtB -> B_40_675_0 = _rtP -> P_546 * _rtB -> B_40_674_0 ; _rtB -> B_40_676_0
= _rtB -> B_40_47_0_e * _rtB -> B_40_675_0 ; _rtB -> B_40_677_0 = _rtB ->
B_40_670_0 - _rtB -> B_40_676_0 ; if ( _rtB -> B_40_677_0 > _rtB ->
B_40_166_0_a ) { _rtB -> B_40_680_0 = _rtB -> B_40_166_0_a ; } else { if (
_rtB -> B_40_677_0 < _rtB -> B_40_167_0_p ) { _rtB -> B_38_1_0 = _rtB ->
B_40_167_0_p ; } else { _rtB -> B_38_1_0 = _rtB -> B_40_677_0 ; } _rtB ->
B_40_680_0 = _rtB -> B_38_1_0 ; } { real_T * * uBuffer = ( real_T * * ) &
_rtDW -> TransportDelay_PWORK_dp . TUbufferPtrs [ 0 ] ; real_T * * tBuffer =
( real_T * * ) & _rtDW -> TransportDelay_PWORK_dp . TUbufferPtrs [ 1 ] ;
real_T simTime = ssGetT ( S ) ; real_T tMinusDelay = simTime - _rtP -> P_547
; _rtB -> B_40_681_0 = Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay ,
0.0 , * tBuffer , * uBuffer , _rtDW -> TransportDelay_IWORK_ep .
CircularBufSize , & _rtDW -> TransportDelay_IWORK_ep . Last , _rtDW ->
TransportDelay_IWORK_ep . Tail , _rtDW -> TransportDelay_IWORK_ep . Head ,
_rtP -> P_548 , 0 , ( boolean_T ) ( ssIsMinorTimeStep ( S ) && (
ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) ) ) ; } _rtB -> B_40_682_0 =
0.0 ; _rtB -> B_40_682_0 += _rtP -> P_550 * _rtX -> TransferFcn_CSTATE_nw ;
_rtB -> B_40_682_0 += _rtP -> P_551 * _rtB -> B_40_681_0 ; _rtB -> B_40_683_0
= _rtP -> P_552 * _rtB -> B_40_682_0 ; if ( _rtB -> B_40_683_0 > _rtP ->
P_553 ) { _rtB -> B_40_684_0 = _rtP -> P_553 ; } else if ( _rtB -> B_40_683_0
< _rtP -> P_554 ) { _rtB -> B_40_684_0 = _rtP -> P_554 ; } else { _rtB ->
B_40_684_0 = _rtB -> B_40_683_0 ; } if ( ( _rtDW -> LastMajorTimeA_kh >=
ssGetTaskTime ( S , 0 ) ) && ( _rtDW -> LastMajorTimeB_og >= ssGetTaskTime (
S , 0 ) ) ) { _rtB -> B_40_685_0 = _rtB -> B_40_684_0 ; } else { if ( ( (
_rtDW -> LastMajorTimeA_kh < _rtDW -> LastMajorTimeB_og ) && ( _rtDW ->
LastMajorTimeB_og < ssGetTaskTime ( S , 0 ) ) ) || ( ( _rtDW ->
LastMajorTimeA_kh >= _rtDW -> LastMajorTimeB_og ) && ( _rtDW ->
LastMajorTimeA_kh >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime (
S , 0 ) - _rtDW -> LastMajorTimeB_og ; rtb_B_40_3_0 = _rtDW -> PrevYB_lj ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_kh ;
rtb_B_40_3_0 = _rtDW -> PrevYA_mm ; } riseValLimit = deltaT * _rtP -> P_555 ;
rateLimiterRate = _rtB -> B_40_684_0 - rtb_B_40_3_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_40_685_0 = rtb_B_40_3_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_556 ; if ( rateLimiterRate < deltaT ) { _rtB ->
B_40_685_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB -> B_40_685_0 = _rtB ->
B_40_684_0 ; } } } _rtB -> B_40_686_0 = _rtP -> P_557 * _rtB -> B_40_685_0 ;
_rtB -> B_40_687_0 = _rtP -> P_558 * _rtB -> B_40_686_0 ;
vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model4/Data Store Read" ,
0 ) ; _rtB -> B_40_688_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_j1 != 0 )
{ _rtX -> vv0at_CSTATE_ne = _rtB -> B_40_688_0 ; } _rtB -> B_40_689_0 = _rtX
-> vv0at_CSTATE_ne ; _rtB -> B_40_690_0 = muDoubleScalarAbs ( _rtB ->
B_40_689_0 ) ; _rtB -> B_40_691_0 = _rtP -> P_559 [ 0 ] ; _rtB -> B_40_691_0
= _rtB -> B_40_691_0 * _rtB -> B_40_690_0 + _rtP -> P_559 [ 1 ] ; _rtB ->
B_40_691_0 = _rtB -> B_40_691_0 * _rtB -> B_40_690_0 + _rtP -> P_559 [ 2 ] ;
_rtB -> B_40_692_0 = _rtP -> P_560 * _rtB -> B_40_691_0 ; _rtB -> B_40_693_0
= _rtB -> B_40_48_0_b * _rtB -> B_40_692_0 ; _rtB -> B_40_694_0 = _rtB ->
B_40_687_0 - _rtB -> B_40_693_0 ; if ( _rtB -> B_40_694_0 > _rtB ->
B_40_169_0 ) { _rtB -> B_40_697_0 = _rtB -> B_40_169_0 ; } else { if ( _rtB
-> B_40_694_0 < _rtB -> B_40_170_0_m ) { _rtB -> B_39_1_0 = _rtB ->
B_40_170_0_m ; } else { _rtB -> B_39_1_0 = _rtB -> B_40_694_0 ; } _rtB ->
B_40_697_0 = _rtB -> B_39_1_0 ; } _rtB -> B_40_698_0 [ 0 ] = _rtB ->
B_40_34_0 ; _rtB -> B_40_698_0 [ 1 ] = _rtB -> B_40_51_0 ; _rtB -> B_40_698_0
[ 2 ] = _rtB -> B_40_68_0 ; _rtB -> B_40_698_0 [ 3 ] = _rtB -> B_40_85_0 ;
_rtB -> B_40_698_0 [ 4 ] = _rtB -> B_40_102_0 ; _rtB -> B_40_698_0 [ 5 ] =
_rtB -> B_40_119_0 ; _rtB -> B_40_698_0 [ 6 ] = _rtB -> B_40_136_0 ; _rtB ->
B_40_698_0 [ 7 ] = _rtB -> B_40_153_0 ; _rtB -> B_40_698_0 [ 8 ] = _rtB ->
B_40_170_0 ; _rtB -> B_40_698_0 [ 9 ] = _rtB -> B_40_187_0 ; _rtB ->
B_40_698_0 [ 10 ] = _rtB -> B_40_204_0 ; _rtB -> B_40_698_0 [ 11 ] = _rtB ->
B_40_221_0 ; _rtB -> B_40_698_0 [ 12 ] = _rtB -> B_40_238_0 ; _rtB ->
B_40_698_0 [ 13 ] = _rtB -> B_40_255_0 ; _rtB -> B_40_698_0 [ 14 ] = _rtB ->
B_40_272_0 ; _rtB -> B_40_698_0 [ 15 ] = _rtB -> B_40_289_0 ; _rtB ->
B_40_698_0 [ 16 ] = _rtB -> B_40_306_0 ; _rtB -> B_40_698_0 [ 17 ] = _rtB ->
B_40_323_0 ; _rtB -> B_40_698_0 [ 18 ] = _rtB -> B_40_340_0 ; _rtB ->
B_40_698_0 [ 19 ] = _rtB -> B_40_357_0 ; _rtB -> B_40_698_0 [ 20 ] = _rtB ->
B_40_374_0 ; _rtB -> B_40_698_0 [ 21 ] = _rtB -> B_40_391_0 ; _rtB ->
B_40_698_0 [ 22 ] = _rtB -> B_40_408_0 ; _rtB -> B_40_698_0 [ 23 ] = _rtB ->
B_40_425_0 ; _rtB -> B_40_698_0 [ 24 ] = _rtB -> B_40_442_0 ; _rtB ->
B_40_698_0 [ 25 ] = _rtB -> B_40_459_0 ; _rtB -> B_40_698_0 [ 26 ] = _rtB ->
B_40_476_0 ; _rtB -> B_40_698_0 [ 27 ] = _rtB -> B_40_493_0 ; _rtB ->
B_40_698_0 [ 28 ] = _rtB -> B_40_510_0 ; _rtB -> B_40_698_0 [ 29 ] = _rtB ->
B_40_527_0 ; _rtB -> B_40_698_0 [ 30 ] = _rtB -> B_40_544_0 ; _rtB ->
B_40_698_0 [ 31 ] = _rtB -> B_40_561_0 ; _rtB -> B_40_698_0 [ 32 ] = _rtB ->
B_40_578_0 ; _rtB -> B_40_698_0 [ 33 ] = _rtB -> B_40_595_0 ; _rtB ->
B_40_698_0 [ 34 ] = _rtB -> B_40_612_0 ; _rtB -> B_40_698_0 [ 35 ] = _rtB ->
B_40_629_0 ; _rtB -> B_40_698_0 [ 36 ] = _rtB -> B_40_646_0 ; _rtB ->
B_40_698_0 [ 37 ] = _rtB -> B_40_663_0 ; _rtB -> B_40_698_0 [ 38 ] = _rtB ->
B_40_680_0 ; _rtB -> B_40_698_0 [ 39 ] = _rtB -> B_40_697_0 ;
ssCallAccelRunBlock ( S , 40 , 699 , SS_CALL_MDL_OUTPUTS ) ; { real_T * *
uBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_ct . TUbufferPtrs [
0 ] ; real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_ct
. TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_561 ; _rtB -> B_40_700_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_au . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_au . Last , _rtDW -> TransportDelay_IWORK_au . Tail ,
_rtDW -> TransportDelay_IWORK_au . Head , _rtP -> P_562 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_40_701_0 = 0.0 ; _rtB -> B_40_701_0 += _rtP -> P_564 * _rtX
-> TransferFcn_CSTATE_a3 ; _rtB -> B_40_701_0 += _rtP -> P_565 * _rtB ->
B_40_700_0 ; _rtB -> B_40_698_0 [ 0 ] = _rtB -> B_40_36_0 ; _rtB ->
B_40_698_0 [ 1 ] = _rtB -> B_40_53_0 ; _rtB -> B_40_698_0 [ 2 ] = _rtB ->
B_40_70_0 ; _rtB -> B_40_698_0 [ 3 ] = _rtB -> B_40_87_0 ; _rtB -> B_40_698_0
[ 4 ] = _rtB -> B_40_104_0 ; _rtB -> B_40_698_0 [ 5 ] = _rtB -> B_40_121_0 ;
_rtB -> B_40_698_0 [ 6 ] = _rtB -> B_40_138_0 ; _rtB -> B_40_698_0 [ 7 ] =
_rtB -> B_40_155_0 ; _rtB -> B_40_698_0 [ 8 ] = _rtB -> B_40_172_0 ; _rtB ->
B_40_698_0 [ 9 ] = _rtB -> B_40_189_0 ; _rtB -> B_40_698_0 [ 10 ] = _rtB ->
B_40_206_0 ; _rtB -> B_40_698_0 [ 11 ] = _rtB -> B_40_223_0 ; _rtB ->
B_40_698_0 [ 12 ] = _rtB -> B_40_240_0 ; _rtB -> B_40_698_0 [ 13 ] = _rtB ->
B_40_257_0 ; _rtB -> B_40_698_0 [ 14 ] = _rtB -> B_40_274_0 ; _rtB ->
B_40_698_0 [ 15 ] = _rtB -> B_40_291_0 ; _rtB -> B_40_698_0 [ 16 ] = _rtB ->
B_40_308_0 ; _rtB -> B_40_698_0 [ 17 ] = _rtB -> B_40_325_0 ; _rtB ->
B_40_698_0 [ 18 ] = _rtB -> B_40_342_0 ; _rtB -> B_40_698_0 [ 19 ] = _rtB ->
B_40_359_0 ; _rtB -> B_40_698_0 [ 20 ] = _rtB -> B_40_376_0 ; _rtB ->
B_40_698_0 [ 21 ] = _rtB -> B_40_393_0 ; _rtB -> B_40_698_0 [ 22 ] = _rtB ->
B_40_410_0 ; _rtB -> B_40_698_0 [ 23 ] = _rtB -> B_40_427_0 ; _rtB ->
B_40_698_0 [ 24 ] = _rtB -> B_40_444_0 ; _rtB -> B_40_698_0 [ 25 ] = _rtB ->
B_40_461_0 ; _rtB -> B_40_698_0 [ 26 ] = _rtB -> B_40_478_0 ; _rtB ->
B_40_698_0 [ 27 ] = _rtB -> B_40_495_0 ; _rtB -> B_40_698_0 [ 28 ] = _rtB ->
B_40_512_0 ; _rtB -> B_40_698_0 [ 29 ] = _rtB -> B_40_529_0 ; _rtB ->
B_40_698_0 [ 30 ] = _rtB -> B_40_546_0 ; _rtB -> B_40_698_0 [ 31 ] = _rtB ->
B_40_563_0 ; _rtB -> B_40_698_0 [ 32 ] = _rtB -> B_40_580_0 ; _rtB ->
B_40_698_0 [ 33 ] = _rtB -> B_40_597_0 ; _rtB -> B_40_698_0 [ 34 ] = _rtB ->
B_40_614_0 ; _rtB -> B_40_698_0 [ 35 ] = _rtB -> B_40_631_0 ; _rtB ->
B_40_698_0 [ 36 ] = _rtB -> B_40_648_0 ; _rtB -> B_40_698_0 [ 37 ] = _rtB ->
B_40_665_0 ; _rtB -> B_40_698_0 [ 38 ] = _rtB -> B_40_682_0 ; _rtB ->
B_40_698_0 [ 39 ] = _rtB -> B_40_701_0 ; ssCallAccelRunBlock ( S , 40 , 703 ,
SS_CALL_MDL_OUTPUTS ) ; _rtB -> B_40_704_0 = _rtB -> B_40_34_0 / _rtB ->
B_40_2_0 ; _rtB -> B_40_705_0 = _rtB -> B_40_704_0 + _rtP -> P_566 ; _rtB ->
B_40_706_0 = _rtB -> B_40_51_0 / _rtB -> B_40_3_0 ; _rtB -> B_40_707_0 = _rtB
-> B_40_706_0 + _rtP -> P_567 ; _rtB -> B_40_708_0 = _rtB -> B_40_68_0 / _rtB
-> B_40_4_0 ; _rtB -> B_40_709_0 = _rtB -> B_40_708_0 + _rtP -> P_568 ; _rtB
-> B_40_710_0 = _rtB -> B_40_85_0 / _rtB -> B_40_5_0 ; _rtB -> B_40_711_0 =
_rtB -> B_40_710_0 + _rtP -> P_569 ; _rtB -> B_40_712_0 = _rtB -> B_40_102_0
/ _rtB -> B_40_6_0 ; _rtB -> B_40_713_0 = _rtB -> B_40_712_0 + _rtP -> P_570
; _rtB -> B_40_714_0 = _rtB -> B_40_119_0 / _rtB -> B_40_8_0 ; _rtB ->
B_40_715_0 = _rtB -> B_40_714_0 + _rtP -> P_571 ; _rtB -> B_40_716_0 = _rtB
-> B_40_136_0 / _rtB -> B_40_9_0 ; _rtB -> B_40_717_0 = _rtB -> B_40_716_0 +
_rtP -> P_572 ; _rtB -> B_40_718_0 = _rtB -> B_40_153_0 / _rtB -> B_40_10_0_k
; _rtB -> B_40_719_0 = _rtB -> B_40_718_0 + _rtP -> P_573 ; _rtB ->
B_40_720_0 = _rtB -> B_40_170_0 / _rtB -> B_40_11_0_c ; _rtB -> B_40_721_0 =
_rtB -> B_40_720_0 + _rtP -> P_574 ; _rtB -> B_40_722_0 = _rtB -> B_40_187_0
/ _rtB -> B_40_12_0_b ; _rtB -> B_40_723_0 = _rtB -> B_40_722_0 + _rtP ->
P_575 ; _rtB -> B_40_724_0 = _rtB -> B_40_204_0 / _rtB -> B_40_14_0_p ; _rtB
-> B_40_725_0 = _rtB -> B_40_724_0 + _rtP -> P_576 ; _rtB -> B_40_726_0 =
_rtB -> B_40_221_0 / _rtB -> B_40_15_0 ; _rtB -> B_40_727_0 = _rtB ->
B_40_726_0 + _rtP -> P_577 ; _rtB -> B_40_728_0 = _rtB -> B_40_238_0 / _rtB
-> B_40_16_0 ; _rtB -> B_40_729_0 = _rtB -> B_40_728_0 + _rtP -> P_578 ; _rtB
-> B_40_730_0 = _rtB -> B_40_255_0 / _rtB -> B_40_17_0_c ; _rtB -> B_40_731_0
= _rtB -> B_40_730_0 + _rtP -> P_579 ; _rtB -> B_40_732_0 = _rtB ->
B_40_272_0 / _rtB -> B_40_18_0_f ; _rtB -> B_40_733_0 = _rtB -> B_40_732_0 +
_rtP -> P_580 ; _rtB -> B_40_734_0 = _rtB -> B_40_289_0 / _rtB -> B_40_20_0_g
; _rtB -> B_40_735_0 = _rtB -> B_40_734_0 + _rtP -> P_581 ; _rtB ->
B_40_736_0 = _rtB -> B_40_306_0 / _rtB -> B_40_21_0_g ; _rtB -> B_40_737_0 =
_rtB -> B_40_736_0 + _rtP -> P_582 ; _rtB -> B_40_738_0 = _rtB -> B_40_323_0
/ _rtB -> B_40_22_0_m ; _rtB -> B_40_739_0 = _rtB -> B_40_738_0 + _rtP ->
P_583 ; _rtB -> B_40_740_0 = _rtB -> B_40_340_0 / _rtB -> B_40_23_0_n ; _rtB
-> B_40_741_0 = _rtB -> B_40_740_0 + _rtP -> P_584 ; _rtB -> B_40_742_0 =
_rtB -> B_40_357_0 / _rtB -> B_40_24_0_p ; _rtB -> B_40_743_0 = _rtB ->
B_40_742_0 + _rtP -> P_585 ; _rtB -> B_40_744_0 = _rtB -> B_40_374_0 / _rtB
-> B_40_26_0_l ; _rtB -> B_40_745_0 = _rtB -> B_40_744_0 + _rtP -> P_586 ;
_rtB -> B_40_746_0 = _rtB -> B_40_391_0 / _rtB -> B_40_27_0_j ; _rtB ->
B_40_747_0 = _rtB -> B_40_746_0 + _rtP -> P_587 ; _rtB -> B_40_748_0 = _rtB
-> B_40_408_0 / _rtB -> B_40_28_0_d ; _rtB -> B_40_749_0 = _rtB -> B_40_748_0
+ _rtP -> P_588 ; _rtB -> B_40_750_0 = _rtB -> B_40_425_0 / _rtB ->
B_40_29_0_g ; _rtB -> B_40_751_0 = _rtB -> B_40_750_0 + _rtP -> P_589 ; _rtB
-> B_40_752_0 = _rtB -> B_40_442_0 / _rtB -> B_40_30_0_l ; _rtB -> B_40_753_0
= _rtB -> B_40_752_0 + _rtP -> P_590 ; _rtB -> B_40_754_0 = _rtB ->
B_40_459_0 / _rtB -> B_40_32_0 ; _rtB -> B_40_755_0 = _rtB -> B_40_754_0 +
_rtP -> P_591 ; _rtB -> B_40_756_0 = _rtB -> B_40_476_0 / _rtB -> B_40_33_0 ;
_rtB -> B_40_757_0 = _rtB -> B_40_756_0 + _rtP -> P_592 ; _rtB -> B_40_758_0
= _rtB -> B_40_493_0 / _rtB -> B_40_34_0_d ; _rtB -> B_40_759_0 = _rtB ->
B_40_758_0 + _rtP -> P_593 ; _rtB -> B_40_760_0 = _rtB -> B_40_510_0 / _rtB
-> B_40_35_0_d ; _rtB -> B_40_761_0 = _rtB -> B_40_760_0 + _rtP -> P_594 ;
_rtB -> B_40_762_0 = _rtB -> B_40_527_0 / _rtB -> B_40_36_0_l ; _rtB ->
B_40_763_0 = _rtB -> B_40_762_0 + _rtP -> P_595 ; _rtB -> B_40_764_0 = _rtB
-> B_40_544_0 / _rtB -> B_40_38_0_o ; _rtB -> B_40_765_0 = _rtB -> B_40_764_0
+ _rtP -> P_596 ; _rtB -> B_40_766_0 = _rtB -> B_40_561_0 / _rtB ->
B_40_39_0_b ; _rtB -> B_40_767_0 = _rtB -> B_40_766_0 + _rtP -> P_597 ; _rtB
-> B_40_768_0 = _rtB -> B_40_578_0 / _rtB -> B_40_40_0_n ; _rtB -> B_40_769_0
= _rtB -> B_40_768_0 + _rtP -> P_598 ; _rtB -> B_40_770_0 = _rtB ->
B_40_595_0 / _rtB -> B_40_41_0_b ; _rtB -> B_40_771_0 = _rtB -> B_40_770_0 +
_rtP -> P_599 ; _rtB -> B_40_772_0 = _rtB -> B_40_612_0 / _rtB -> B_40_42_0_l
; _rtB -> B_40_773_0 = _rtB -> B_40_772_0 + _rtP -> P_600 ; _rtB ->
B_40_774_0 = _rtB -> B_40_629_0 / _rtB -> B_40_44_0_h ; _rtB -> B_40_775_0 =
_rtB -> B_40_774_0 + _rtP -> P_601 ; _rtB -> B_40_776_0 = _rtB -> B_40_646_0
/ _rtB -> B_40_45_0_b ; _rtB -> B_40_777_0 = _rtB -> B_40_776_0 + _rtP ->
P_602 ; _rtB -> B_40_778_0 = _rtB -> B_40_663_0 / _rtB -> B_40_46_0_d ; _rtB
-> B_40_779_0 = _rtB -> B_40_778_0 + _rtP -> P_603 ; _rtB -> B_40_780_0 =
_rtB -> B_40_680_0 / _rtB -> B_40_47_0_e ; _rtB -> B_40_781_0 = _rtB ->
B_40_780_0 + _rtP -> P_604 ; _rtB -> B_40_782_0 = _rtB -> B_40_697_0 / _rtB
-> B_40_48_0_b ; _rtB -> B_40_783_0 = _rtB -> B_40_782_0 + _rtP -> P_605 ;
_rtB -> B_40_698_0 [ 0 ] = _rtB -> B_40_705_0 ; _rtB -> B_40_698_0 [ 1 ] =
_rtB -> B_40_707_0 ; _rtB -> B_40_698_0 [ 2 ] = _rtB -> B_40_709_0 ; _rtB ->
B_40_698_0 [ 3 ] = _rtB -> B_40_711_0 ; _rtB -> B_40_698_0 [ 4 ] = _rtB ->
B_40_713_0 ; _rtB -> B_40_698_0 [ 5 ] = _rtB -> B_40_715_0 ; _rtB ->
B_40_698_0 [ 6 ] = _rtB -> B_40_717_0 ; _rtB -> B_40_698_0 [ 7 ] = _rtB ->
B_40_719_0 ; _rtB -> B_40_698_0 [ 8 ] = _rtB -> B_40_721_0 ; _rtB ->
B_40_698_0 [ 9 ] = _rtB -> B_40_723_0 ; _rtB -> B_40_698_0 [ 10 ] = _rtB ->
B_40_725_0 ; _rtB -> B_40_698_0 [ 11 ] = _rtB -> B_40_727_0 ; _rtB ->
B_40_698_0 [ 12 ] = _rtB -> B_40_729_0 ; _rtB -> B_40_698_0 [ 13 ] = _rtB ->
B_40_731_0 ; _rtB -> B_40_698_0 [ 14 ] = _rtB -> B_40_733_0 ; _rtB ->
B_40_698_0 [ 15 ] = _rtB -> B_40_735_0 ; _rtB -> B_40_698_0 [ 16 ] = _rtB ->
B_40_737_0 ; _rtB -> B_40_698_0 [ 17 ] = _rtB -> B_40_739_0 ; _rtB ->
B_40_698_0 [ 18 ] = _rtB -> B_40_741_0 ; _rtB -> B_40_698_0 [ 19 ] = _rtB ->
B_40_743_0 ; _rtB -> B_40_698_0 [ 20 ] = _rtB -> B_40_745_0 ; _rtB ->
B_40_698_0 [ 21 ] = _rtB -> B_40_747_0 ; _rtB -> B_40_698_0 [ 22 ] = _rtB ->
B_40_749_0 ; _rtB -> B_40_698_0 [ 23 ] = _rtB -> B_40_751_0 ; _rtB ->
B_40_698_0 [ 24 ] = _rtB -> B_40_753_0 ; _rtB -> B_40_698_0 [ 25 ] = _rtB ->
B_40_755_0 ; _rtB -> B_40_698_0 [ 26 ] = _rtB -> B_40_757_0 ; _rtB ->
B_40_698_0 [ 27 ] = _rtB -> B_40_759_0 ; _rtB -> B_40_698_0 [ 28 ] = _rtB ->
B_40_761_0 ; _rtB -> B_40_698_0 [ 29 ] = _rtB -> B_40_763_0 ; _rtB ->
B_40_698_0 [ 30 ] = _rtB -> B_40_765_0 ; _rtB -> B_40_698_0 [ 31 ] = _rtB ->
B_40_767_0 ; _rtB -> B_40_698_0 [ 32 ] = _rtB -> B_40_769_0 ; _rtB ->
B_40_698_0 [ 33 ] = _rtB -> B_40_771_0 ; _rtB -> B_40_698_0 [ 34 ] = _rtB ->
B_40_773_0 ; _rtB -> B_40_698_0 [ 35 ] = _rtB -> B_40_775_0 ; _rtB ->
B_40_698_0 [ 36 ] = _rtB -> B_40_777_0 ; _rtB -> B_40_698_0 [ 37 ] = _rtB ->
B_40_779_0 ; _rtB -> B_40_698_0 [ 38 ] = _rtB -> B_40_781_0 ; _rtB ->
B_40_698_0 [ 39 ] = _rtB -> B_40_783_0 ; ssCallAccelRunBlock ( S , 40 , 785 ,
SS_CALL_MDL_OUTPUTS ) ; ssCallAccelRunBlock ( S , 40 , 786 ,
SS_CALL_MDL_OUTPUTS ) ; _rtB -> B_40_787_0 = _rtX -> Integrator1_CSTATE ;
ssCallAccelRunBlock ( S , 40 , 788 , SS_CALL_MDL_OUTPUTS ) ;
ssCallAccelRunBlock ( S , 40 , 789 , SS_CALL_MDL_OUTPUTS ) ; _rtB ->
B_40_790_0 = ( ( ( _rtB -> B_40_34_0 + _rtB -> B_40_51_0 ) + _rtB ->
B_40_68_0 ) + _rtB -> B_40_85_0 ) + _rtB -> B_40_102_0 ; _rtB -> B_40_791_0 =
( ( ( _rtB -> B_40_119_0 + _rtB -> B_40_136_0 ) + _rtB -> B_40_153_0 ) + _rtB
-> B_40_170_0 ) + _rtB -> B_40_187_0 ; _rtB -> B_40_792_0 = ( ( ( _rtB ->
B_40_204_0 + _rtB -> B_40_221_0 ) + _rtB -> B_40_238_0 ) + _rtB -> B_40_255_0
) + _rtB -> B_40_272_0 ; _rtB -> B_40_793_0 = ( ( ( _rtB -> B_40_289_0 + _rtB
-> B_40_306_0 ) + _rtB -> B_40_323_0 ) + _rtB -> B_40_340_0 ) + _rtB ->
B_40_357_0 ; _rtB -> B_40_794_0 = ( ( ( _rtB -> B_40_374_0 + _rtB ->
B_40_391_0 ) + _rtB -> B_40_408_0 ) + _rtB -> B_40_425_0 ) + _rtB ->
B_40_442_0 ; _rtB -> B_40_795_0 = ( ( ( _rtB -> B_40_459_0 + _rtB ->
B_40_476_0 ) + _rtB -> B_40_493_0 ) + _rtB -> B_40_510_0 ) + _rtB ->
B_40_527_0 ; _rtB -> B_40_796_0 = ( ( ( _rtB -> B_40_544_0 + _rtB ->
B_40_561_0 ) + _rtB -> B_40_578_0 ) + _rtB -> B_40_595_0 ) + _rtB ->
B_40_612_0 ; _rtB -> B_40_797_0 = ( ( ( _rtB -> B_40_629_0 + _rtB ->
B_40_646_0 ) + _rtB -> B_40_663_0 ) + _rtB -> B_40_680_0 ) + _rtB ->
B_40_697_0 ; _rtB -> B_40_798_0 = ( ( ( ( ( ( _rtB -> B_40_790_0 + _rtB ->
B_40_791_0 ) + _rtB -> B_40_792_0 ) + _rtB -> B_40_793_0 ) + _rtB ->
B_40_794_0 ) + _rtB -> B_40_795_0 ) + _rtB -> B_40_796_0 ) + _rtB ->
B_40_797_0 ; if ( _rtB -> B_40_798_0 > _rtP -> P_608 ) { _rtB -> B_40_799_0 =
_rtB -> B_40_798_0 - _rtP -> P_608 ; } else if ( _rtB -> B_40_798_0 >= _rtP
-> P_607 ) { _rtB -> B_40_799_0 = 0.0 ; } else { _rtB -> B_40_799_0 = _rtB ->
B_40_798_0 - _rtP -> P_607 ; } _rtB -> B_40_801_0 = ( real_T ) ( _rtB ->
B_40_799_0 == Simulation_v2_rtC ( S ) -> B_40_1_0 ) * _rtB -> B_40_1_0_c ;
_rtB -> B_40_803_0 = ( real_T ) ( _rtB -> B_40_799_0 != Simulation_v2_rtC ( S
) -> B_40_0_0 ) * _rtB -> B_40_799_0 ; _rtB -> B_40_804_0 = _rtB ->
B_40_801_0 + _rtB -> B_40_803_0 ; ssCallAccelRunBlock ( S , 40 , 805 ,
SS_CALL_MDL_OUTPUTS ) ; _rtB -> B_40_806_0 = _rtB -> B_40_804_0 / _rtB ->
B_40_50_0 ; ssCallAccelRunBlock ( S , 40 , 807 , SS_CALL_MDL_OUTPUTS ) ;
UNUSED_PARAMETER ( tid ) ; } static void mdlOutputsTID3 ( SimStruct * S ,
int_T tid ) { B_Simulation_v2_T * _rtB ; P_Simulation_v2_T * _rtP ; _rtP = (
( P_Simulation_v2_T * ) ssGetModelRtp ( S ) ) ; _rtB = ( ( B_Simulation_v2_T
* ) _ssGetModelBlockIO ( S ) ) ; _rtB -> B_40_0_0_m = _rtP -> P_610 ; _rtB ->
B_40_1_0_c = _rtP -> P_611 ; _rtB -> B_40_2_0 = _rtP -> P_612 ; _rtB ->
B_40_3_0 = _rtP -> P_613 ; _rtB -> B_40_4_0 = _rtP -> P_614 ; _rtB ->
B_40_5_0 = _rtP -> P_615 ; _rtB -> B_40_6_0 = _rtP -> P_616 ; _rtB ->
B_40_8_0 = _rtP -> P_617 ; _rtB -> B_40_9_0 = _rtP -> P_618 ; _rtB ->
B_40_10_0_k = _rtP -> P_619 ; _rtB -> B_40_11_0_c = _rtP -> P_620 ; _rtB ->
B_40_12_0_b = _rtP -> P_621 ; _rtB -> B_40_14_0_p = _rtP -> P_622 ; _rtB ->
B_40_15_0 = _rtP -> P_623 ; _rtB -> B_40_16_0 = _rtP -> P_624 ; _rtB ->
B_40_17_0_c = _rtP -> P_625 ; _rtB -> B_40_18_0_f = _rtP -> P_626 ; _rtB ->
B_40_20_0_g = _rtP -> P_627 ; _rtB -> B_40_21_0_g = _rtP -> P_628 ; _rtB ->
B_40_22_0_m = _rtP -> P_629 ; _rtB -> B_40_23_0_n = _rtP -> P_630 ; _rtB ->
B_40_24_0_p = _rtP -> P_631 ; _rtB -> B_40_26_0_l = _rtP -> P_632 ; _rtB ->
B_40_27_0_j = _rtP -> P_633 ; _rtB -> B_40_28_0_d = _rtP -> P_634 ; _rtB ->
B_40_29_0_g = _rtP -> P_635 ; _rtB -> B_40_30_0_l = _rtP -> P_636 ; _rtB ->
B_40_32_0 = _rtP -> P_637 ; _rtB -> B_40_33_0 = _rtP -> P_638 ; _rtB ->
B_40_34_0_d = _rtP -> P_639 ; _rtB -> B_40_35_0_d = _rtP -> P_640 ; _rtB ->
B_40_36_0_l = _rtP -> P_641 ; _rtB -> B_40_38_0_o = _rtP -> P_642 ; _rtB ->
B_40_39_0_b = _rtP -> P_643 ; _rtB -> B_40_40_0_n = _rtP -> P_644 ; _rtB ->
B_40_41_0_b = _rtP -> P_645 ; _rtB -> B_40_42_0_l = _rtP -> P_646 ; _rtB ->
B_40_44_0_h = _rtP -> P_647 ; _rtB -> B_40_45_0_b = _rtP -> P_648 ; _rtB ->
B_40_46_0_d = _rtP -> P_649 ; _rtB -> B_40_47_0_e = _rtP -> P_650 ; _rtB ->
B_40_48_0_b = _rtP -> P_651 ; _rtB -> B_40_50_0 = ( ( ( ( ( ( ( ( ( ( _rtB ->
B_40_2_0 + _rtB -> B_40_3_0 ) + _rtB -> B_40_4_0 ) + _rtB -> B_40_5_0 ) +
_rtB -> B_40_6_0 ) + ( ( ( ( _rtB -> B_40_8_0 + _rtB -> B_40_9_0 ) + _rtB ->
B_40_10_0_k ) + _rtB -> B_40_11_0_c ) + _rtB -> B_40_12_0_b ) ) + ( ( ( (
_rtB -> B_40_14_0_p + _rtB -> B_40_15_0 ) + _rtB -> B_40_16_0 ) + _rtB ->
B_40_17_0_c ) + _rtB -> B_40_18_0_f ) ) + ( ( ( ( _rtB -> B_40_20_0_g + _rtB
-> B_40_21_0_g ) + _rtB -> B_40_22_0_m ) + _rtB -> B_40_23_0_n ) + _rtB ->
B_40_24_0_p ) ) + ( ( ( ( _rtB -> B_40_26_0_l + _rtB -> B_40_27_0_j ) + _rtB
-> B_40_28_0_d ) + _rtB -> B_40_29_0_g ) + _rtB -> B_40_30_0_l ) ) + ( ( ( (
_rtB -> B_40_32_0 + _rtB -> B_40_33_0 ) + _rtB -> B_40_34_0_d ) + _rtB ->
B_40_35_0_d ) + _rtB -> B_40_36_0_l ) ) + ( ( ( ( _rtB -> B_40_38_0_o + _rtB
-> B_40_39_0_b ) + _rtB -> B_40_40_0_n ) + _rtB -> B_40_41_0_b ) + _rtB ->
B_40_42_0_l ) ) + ( ( ( ( _rtB -> B_40_44_0_h + _rtB -> B_40_45_0_b ) + _rtB
-> B_40_46_0_d ) + _rtB -> B_40_47_0_e ) + _rtB -> B_40_48_0_b ) ; _rtB ->
B_40_52_0_j = _rtP -> P_652 * _rtB -> B_40_2_0 ; _rtB -> B_40_53_0_f = _rtP
-> P_653 * _rtB -> B_40_52_0_j ; _rtB -> B_40_55_0_a = _rtP -> P_654 * _rtB
-> B_40_3_0 ; _rtB -> B_40_56_0_j = _rtP -> P_655 * _rtB -> B_40_55_0_a ;
_rtB -> B_40_58_0_j = _rtP -> P_656 * _rtB -> B_40_4_0 ; _rtB -> B_40_59_0_o
= _rtP -> P_657 * _rtB -> B_40_58_0_j ; _rtB -> B_40_61_0_n = _rtP -> P_658 *
_rtB -> B_40_5_0 ; _rtB -> B_40_62_0_i = _rtP -> P_659 * _rtB -> B_40_61_0_n
; _rtB -> B_40_64_0_o = _rtP -> P_660 * _rtB -> B_40_6_0 ; _rtB ->
B_40_65_0_n = _rtP -> P_661 * _rtB -> B_40_64_0_o ; _rtB -> B_40_67_0 = _rtP
-> P_662 * _rtB -> B_40_8_0 ; _rtB -> B_40_68_0_m = _rtP -> P_663 * _rtB ->
B_40_67_0 ; _rtB -> B_40_70_0_c = _rtP -> P_664 * _rtB -> B_40_9_0 ; _rtB ->
B_40_71_0_m = _rtP -> P_665 * _rtB -> B_40_70_0_c ; _rtB -> B_40_73_0_m =
_rtP -> P_666 * _rtB -> B_40_10_0_k ; _rtB -> B_40_74_0_j = _rtP -> P_667 *
_rtB -> B_40_73_0_m ; _rtB -> B_40_76_0_h = _rtP -> P_668 * _rtB ->
B_40_11_0_c ; _rtB -> B_40_77_0_c = _rtP -> P_669 * _rtB -> B_40_76_0_h ;
_rtB -> B_40_79_0_c = _rtP -> P_670 * _rtB -> B_40_12_0_b ; _rtB ->
B_40_80_0_p = _rtP -> P_671 * _rtB -> B_40_79_0_c ; _rtB -> B_40_82_0_p =
_rtP -> P_672 * _rtB -> B_40_14_0_p ; _rtB -> B_40_83_0 = _rtP -> P_673 *
_rtB -> B_40_82_0_p ; _rtB -> B_40_85_0_a = _rtP -> P_674 * _rtB -> B_40_15_0
; _rtB -> B_40_86_0_e = _rtP -> P_675 * _rtB -> B_40_85_0_a ; _rtB ->
B_40_88_0_a = _rtP -> P_676 * _rtB -> B_40_16_0 ; _rtB -> B_40_89_0_a = _rtP
-> P_677 * _rtB -> B_40_88_0_a ; _rtB -> B_40_91_0_i = _rtP -> P_678 * _rtB
-> B_40_17_0_c ; _rtB -> B_40_92_0_l = _rtP -> P_679 * _rtB -> B_40_91_0_i ;
_rtB -> B_40_94_0_o = _rtP -> P_680 * _rtB -> B_40_18_0_f ; _rtB ->
B_40_95_0_o = _rtP -> P_681 * _rtB -> B_40_94_0_o ; _rtB -> B_40_97_0_i =
_rtP -> P_682 * _rtB -> B_40_20_0_g ; _rtB -> B_40_98_0_f = _rtP -> P_683 *
_rtB -> B_40_97_0_i ; _rtB -> B_40_100_0 = _rtP -> P_684 * _rtB ->
B_40_21_0_g ; _rtB -> B_40_101_0 = _rtP -> P_685 * _rtB -> B_40_100_0 ; _rtB
-> B_40_103_0_i = _rtP -> P_686 * _rtB -> B_40_22_0_m ; _rtB -> B_40_104_0_f
= _rtP -> P_687 * _rtB -> B_40_103_0_i ; _rtB -> B_40_106_0_g = _rtP -> P_688
* _rtB -> B_40_23_0_n ; _rtB -> B_40_107_0_c = _rtP -> P_689 * _rtB ->
B_40_106_0_g ; _rtB -> B_40_109_0_o = _rtP -> P_690 * _rtB -> B_40_24_0_p ;
_rtB -> B_40_110_0_l = _rtP -> P_691 * _rtB -> B_40_109_0_o ; _rtB ->
B_40_112_0_m = _rtP -> P_692 * _rtB -> B_40_26_0_l ; _rtB -> B_40_113_0_m =
_rtP -> P_693 * _rtB -> B_40_112_0_m ; _rtB -> B_40_115_0_c = _rtP -> P_694 *
_rtB -> B_40_27_0_j ; _rtB -> B_40_116_0_f = _rtP -> P_695 * _rtB ->
B_40_115_0_c ; _rtB -> B_40_118_0 = _rtP -> P_696 * _rtB -> B_40_28_0_d ;
_rtB -> B_40_119_0_p = _rtP -> P_697 * _rtB -> B_40_118_0 ; _rtB ->
B_40_121_0_e = _rtP -> P_698 * _rtB -> B_40_29_0_g ; _rtB -> B_40_122_0_o =
_rtP -> P_699 * _rtB -> B_40_121_0_e ; _rtB -> B_40_124_0_h = _rtP -> P_700 *
_rtB -> B_40_30_0_l ; _rtB -> B_40_125_0_l = _rtP -> P_701 * _rtB ->
B_40_124_0_h ; _rtB -> B_40_127_0_h = _rtP -> P_702 * _rtB -> B_40_32_0 ;
_rtB -> B_40_128_0_m = _rtP -> P_703 * _rtB -> B_40_127_0_h ; _rtB ->
B_40_130_0_m = _rtP -> P_704 * _rtB -> B_40_33_0 ; _rtB -> B_40_131_0_h =
_rtP -> P_705 * _rtB -> B_40_130_0_m ; _rtB -> B_40_133_0_c = _rtP -> P_706 *
_rtB -> B_40_34_0_d ; _rtB -> B_40_134_0 = _rtP -> P_707 * _rtB ->
B_40_133_0_c ; _rtB -> B_40_136_0_k = _rtP -> P_708 * _rtB -> B_40_35_0_d ;
_rtB -> B_40_137_0_p = _rtP -> P_709 * _rtB -> B_40_136_0_k ; _rtB ->
B_40_139_0_p = _rtP -> P_710 * _rtB -> B_40_36_0_l ; _rtB -> B_40_140_0_p =
_rtP -> P_711 * _rtB -> B_40_139_0_p ; _rtB -> B_40_142_0_a = _rtP -> P_712 *
_rtB -> B_40_38_0_o ; _rtB -> B_40_143_0_j = _rtP -> P_713 * _rtB ->
B_40_142_0_a ; _rtB -> B_40_145_0_e = _rtP -> P_714 * _rtB -> B_40_39_0_b ;
_rtB -> B_40_146_0_o = _rtP -> P_715 * _rtB -> B_40_145_0_e ; _rtB ->
B_40_148_0_b = _rtP -> P_716 * _rtB -> B_40_40_0_n ; _rtB -> B_40_149_0_a =
_rtP -> P_717 * _rtB -> B_40_148_0_b ; _rtB -> B_40_151_0 = _rtP -> P_718 *
_rtB -> B_40_41_0_b ; _rtB -> B_40_152_0 = _rtP -> P_719 * _rtB -> B_40_151_0
; _rtB -> B_40_154_0_g = _rtP -> P_720 * _rtB -> B_40_42_0_l ; _rtB ->
B_40_155_0_e = _rtP -> P_721 * _rtB -> B_40_154_0_g ; _rtB -> B_40_157_0_f =
_rtP -> P_722 * _rtB -> B_40_44_0_h ; _rtB -> B_40_158_0_h = _rtP -> P_723 *
_rtB -> B_40_157_0_f ; _rtB -> B_40_160_0_e = _rtP -> P_724 * _rtB ->
B_40_45_0_b ; _rtB -> B_40_161_0_c = _rtP -> P_725 * _rtB -> B_40_160_0_e ;
_rtB -> B_40_163_0_a = _rtP -> P_726 * _rtB -> B_40_46_0_d ; _rtB ->
B_40_164_0_d = _rtP -> P_727 * _rtB -> B_40_163_0_a ; _rtB -> B_40_166_0_a =
_rtP -> P_728 * _rtB -> B_40_47_0_e ; _rtB -> B_40_167_0_p = _rtP -> P_729 *
_rtB -> B_40_166_0_a ; _rtB -> B_40_169_0 = _rtP -> P_730 * _rtB ->
B_40_48_0_b ; _rtB -> B_40_170_0_m = _rtP -> P_731 * _rtB -> B_40_169_0 ;
UNUSED_PARAMETER ( tid ) ; }
#define MDL_UPDATE
static void mdlUpdate ( SimStruct * S , int_T tid ) { int32_T isHit ;
B_Simulation_v2_T * _rtB ; P_Simulation_v2_T * _rtP ; DW_Simulation_v2_T *
_rtDW ; _rtDW = ( ( DW_Simulation_v2_T * ) ssGetRootDWork ( S ) ) ; _rtP = (
( P_Simulation_v2_T * ) ssGetModelRtp ( S ) ) ; _rtB = ( ( B_Simulation_v2_T
* ) _ssGetModelBlockIO ( S ) ) ; isHit = ssIsSampleHit ( S , 2 , 0 ) ; if (
isHit != 0 ) { _rtDW -> DelayInput1_DSTATE = _rtB -> B_40_0_0 ; } _rtDW ->
Integrator_IWORK = 0 ; if ( _rtDW -> LastMajorTimeA == ( rtInf ) ) { _rtDW ->
LastMajorTimeA = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA = _rtB ->
B_40_22_0 ; } else if ( _rtDW -> LastMajorTimeB == ( rtInf ) ) { _rtDW ->
LastMajorTimeB = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB = _rtB ->
B_40_22_0 ; } else if ( _rtDW -> LastMajorTimeA < _rtDW -> LastMajorTimeB ) {
_rtDW -> LastMajorTimeA = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA = _rtB ->
B_40_22_0 ; } else { _rtDW -> LastMajorTimeB = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYB = _rtB -> B_40_22_0 ; } _rtDW -> vv0at_IWORK = 0 ; { real_T *
* uBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK . TUbufferPtrs [ 0
] ; real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK . Head = ( ( _rtDW -> TransportDelay_IWORK . Head < (
_rtDW -> TransportDelay_IWORK . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK . Head + 1 ) : 0 ) ; if ( _rtDW -> TransportDelay_IWORK
. Head == _rtDW -> TransportDelay_IWORK . Tail ) { if ( !
Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK . CircularBufSize , & _rtDW -> TransportDelay_IWORK .
Tail , & _rtDW -> TransportDelay_IWORK . Head , & _rtDW ->
TransportDelay_IWORK . Last , simTime - _rtP -> P_15 , tBuffer , uBuffer , (
NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK . Head ] = _rtB ->
B_40_19_0 ; } if ( _rtDW -> LastMajorTimeA_m == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_m = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_d = _rtB ->
B_40_39_0 ; } else if ( _rtDW -> LastMajorTimeB_a == ( rtInf ) ) { _rtDW ->
LastMajorTimeB_a = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_f = _rtB ->
B_40_39_0 ; } else if ( _rtDW -> LastMajorTimeA_m < _rtDW -> LastMajorTimeB_a
) { _rtDW -> LastMajorTimeA_m = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_d =
_rtB -> B_40_39_0 ; } else { _rtDW -> LastMajorTimeB_a = ssGetTaskTime ( S ,
0 ) ; _rtDW -> PrevYB_f = _rtB -> B_40_39_0 ; } _rtDW -> vv0at_IWORK_o = 0 ;
{ real_T * * uBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_d .
TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_d . TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ;
_rtDW -> TransportDelay_IWORK_k . Head = ( ( _rtDW -> TransportDelay_IWORK_k
. Head < ( _rtDW -> TransportDelay_IWORK_k . CircularBufSize - 1 ) ) ? (
_rtDW -> TransportDelay_IWORK_k . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_k . Head == _rtDW -> TransportDelay_IWORK_k . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_k . CircularBufSize , & _rtDW -> TransportDelay_IWORK_k
. Tail , & _rtDW -> TransportDelay_IWORK_k . Head , & _rtDW ->
TransportDelay_IWORK_k . Last , simTime - _rtP -> P_29 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_k .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_k . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_k . Head ] = _rtB ->
B_40_36_0 ; } if ( _rtDW -> LastMajorTimeA_g == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_g = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_e = _rtB ->
B_40_56_0 ; } else if ( _rtDW -> LastMajorTimeB_h == ( rtInf ) ) { _rtDW ->
LastMajorTimeB_h = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_g = _rtB ->
B_40_56_0 ; } else if ( _rtDW -> LastMajorTimeA_g < _rtDW -> LastMajorTimeB_h
) { _rtDW -> LastMajorTimeA_g = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_e =
_rtB -> B_40_56_0 ; } else { _rtDW -> LastMajorTimeB_h = ssGetTaskTime ( S ,
0 ) ; _rtDW -> PrevYB_g = _rtB -> B_40_56_0 ; } _rtDW -> vv0at_IWORK_l = 0 ;
{ real_T * * uBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_m .
TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_m . TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ;
_rtDW -> TransportDelay_IWORK_b . Head = ( ( _rtDW -> TransportDelay_IWORK_b
. Head < ( _rtDW -> TransportDelay_IWORK_b . CircularBufSize - 1 ) ) ? (
_rtDW -> TransportDelay_IWORK_b . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_b . Head == _rtDW -> TransportDelay_IWORK_b . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_b . CircularBufSize , & _rtDW -> TransportDelay_IWORK_b
. Tail , & _rtDW -> TransportDelay_IWORK_b . Head , & _rtDW ->
TransportDelay_IWORK_b . Last , simTime - _rtP -> P_43 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_b .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_b . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_b . Head ] = _rtB ->
B_40_53_0 ; } if ( _rtDW -> LastMajorTimeA_i == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_i = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_e4 = _rtB ->
B_40_73_0 ; } else if ( _rtDW -> LastMajorTimeB_m == ( rtInf ) ) { _rtDW ->
LastMajorTimeB_m = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_c = _rtB ->
B_40_73_0 ; } else if ( _rtDW -> LastMajorTimeA_i < _rtDW -> LastMajorTimeB_m
) { _rtDW -> LastMajorTimeA_i = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_e4
= _rtB -> B_40_73_0 ; } else { _rtDW -> LastMajorTimeB_m = ssGetTaskTime ( S
, 0 ) ; _rtDW -> PrevYB_c = _rtB -> B_40_73_0 ; } _rtDW -> vv0at_IWORK_h = 0
; { real_T * * uBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_a .
TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_a . TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ;
_rtDW -> TransportDelay_IWORK_bc . Head = ( ( _rtDW ->
TransportDelay_IWORK_bc . Head < ( _rtDW -> TransportDelay_IWORK_bc .
CircularBufSize - 1 ) ) ? ( _rtDW -> TransportDelay_IWORK_bc . Head + 1 ) : 0
) ; if ( _rtDW -> TransportDelay_IWORK_bc . Head == _rtDW ->
TransportDelay_IWORK_bc . Tail ) { if ( !
Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_bc . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_bc . Tail , & _rtDW -> TransportDelay_IWORK_bc . Head ,
& _rtDW -> TransportDelay_IWORK_bc . Last , simTime - _rtP -> P_57 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_bc . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_bc . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_bc . Head ] = _rtB -> B_40_70_0 ; } if ( _rtDW ->
LastMajorTimeA_j == ( rtInf ) ) { _rtDW -> LastMajorTimeA_j = ssGetTaskTime (
S , 0 ) ; _rtDW -> PrevYA_n = _rtB -> B_40_90_0 ; } else if ( _rtDW ->
LastMajorTimeB_c == ( rtInf ) ) { _rtDW -> LastMajorTimeB_c = ssGetTaskTime (
S , 0 ) ; _rtDW -> PrevYB_h = _rtB -> B_40_90_0 ; } else if ( _rtDW ->
LastMajorTimeA_j < _rtDW -> LastMajorTimeB_c ) { _rtDW -> LastMajorTimeA_j =
ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_n = _rtB -> B_40_90_0 ; } else {
_rtDW -> LastMajorTimeB_c = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_h =
_rtB -> B_40_90_0 ; } _rtDW -> vv0at_IWORK_hs = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_l . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_l . TUbufferPtrs
[ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_p .
Head = ( ( _rtDW -> TransportDelay_IWORK_p . Head < ( _rtDW ->
TransportDelay_IWORK_p . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_p . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_p . Head == _rtDW -> TransportDelay_IWORK_p . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_p . CircularBufSize , & _rtDW -> TransportDelay_IWORK_p
. Tail , & _rtDW -> TransportDelay_IWORK_p . Head , & _rtDW ->
TransportDelay_IWORK_p . Last , simTime - _rtP -> P_71 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_p .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_p . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_p . Head ] = _rtB ->
B_40_87_0 ; } if ( _rtDW -> LastMajorTimeA_d == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_d = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_f = _rtB ->
B_40_107_0 ; } else if ( _rtDW -> LastMajorTimeB_k == ( rtInf ) ) { _rtDW ->
LastMajorTimeB_k = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_n = _rtB ->
B_40_107_0 ; } else if ( _rtDW -> LastMajorTimeA_d < _rtDW ->
LastMajorTimeB_k ) { _rtDW -> LastMajorTimeA_d = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYA_f = _rtB -> B_40_107_0 ; } else { _rtDW -> LastMajorTimeB_k =
ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_n = _rtB -> B_40_107_0 ; } _rtDW ->
vv0at_IWORK_i = 0 ; { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_g . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T *
* ) & _rtDW -> TransportDelay_PWORK_g . TUbufferPtrs [ 1 ] ; real_T simTime =
ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_n . Head = ( ( _rtDW ->
TransportDelay_IWORK_n . Head < ( _rtDW -> TransportDelay_IWORK_n .
CircularBufSize - 1 ) ) ? ( _rtDW -> TransportDelay_IWORK_n . Head + 1 ) : 0
) ; if ( _rtDW -> TransportDelay_IWORK_n . Head == _rtDW ->
TransportDelay_IWORK_n . Tail ) { if ( !
Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_n . CircularBufSize , & _rtDW -> TransportDelay_IWORK_n
. Tail , & _rtDW -> TransportDelay_IWORK_n . Head , & _rtDW ->
TransportDelay_IWORK_n . Last , simTime - _rtP -> P_85 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_n .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_n . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_n . Head ] = _rtB ->
B_40_104_0 ; } if ( _rtDW -> LastMajorTimeA_b == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_b = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_o = _rtB ->
B_40_124_0 ; } else if ( _rtDW -> LastMajorTimeB_e == ( rtInf ) ) { _rtDW ->
LastMajorTimeB_e = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_l = _rtB ->
B_40_124_0 ; } else if ( _rtDW -> LastMajorTimeA_b < _rtDW ->
LastMajorTimeB_e ) { _rtDW -> LastMajorTimeA_b = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYA_o = _rtB -> B_40_124_0 ; } else { _rtDW -> LastMajorTimeB_e =
ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_l = _rtB -> B_40_124_0 ; } _rtDW ->
vv0at_IWORK_k = 0 ; { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_aa . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T
* * ) & _rtDW -> TransportDelay_PWORK_aa . TUbufferPtrs [ 1 ] ; real_T
simTime = ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_p4 . Head = ( ( _rtDW
-> TransportDelay_IWORK_p4 . Head < ( _rtDW -> TransportDelay_IWORK_p4 .
CircularBufSize - 1 ) ) ? ( _rtDW -> TransportDelay_IWORK_p4 . Head + 1 ) : 0
) ; if ( _rtDW -> TransportDelay_IWORK_p4 . Head == _rtDW ->
TransportDelay_IWORK_p4 . Tail ) { if ( !
Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_p4 . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_p4 . Tail , & _rtDW -> TransportDelay_IWORK_p4 . Head ,
& _rtDW -> TransportDelay_IWORK_p4 . Last , simTime - _rtP -> P_99 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_p4 . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_p4 . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_p4 . Head ] = _rtB -> B_40_121_0 ; } if ( _rtDW ->
LastMajorTimeA_l == ( rtInf ) ) { _rtDW -> LastMajorTimeA_l = ssGetTaskTime (
S , 0 ) ; _rtDW -> PrevYA_m = _rtB -> B_40_141_0 ; } else if ( _rtDW ->
LastMajorTimeB_l == ( rtInf ) ) { _rtDW -> LastMajorTimeB_l = ssGetTaskTime (
S , 0 ) ; _rtDW -> PrevYB_go = _rtB -> B_40_141_0 ; } else if ( _rtDW ->
LastMajorTimeA_l < _rtDW -> LastMajorTimeB_l ) { _rtDW -> LastMajorTimeA_l =
ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_m = _rtB -> B_40_141_0 ; } else {
_rtDW -> LastMajorTimeB_l = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_go =
_rtB -> B_40_141_0 ; } _rtDW -> vv0at_IWORK_d = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_l0 . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_l0 .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_c . Head = ( ( _rtDW -> TransportDelay_IWORK_c . Head <
( _rtDW -> TransportDelay_IWORK_c . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_c . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_c . Head == _rtDW -> TransportDelay_IWORK_c . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_c . CircularBufSize , & _rtDW -> TransportDelay_IWORK_c
. Tail , & _rtDW -> TransportDelay_IWORK_c . Head , & _rtDW ->
TransportDelay_IWORK_c . Last , simTime - _rtP -> P_113 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_c .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_c . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_c . Head ] = _rtB ->
B_40_138_0 ; } if ( _rtDW -> LastMajorTimeA_dv == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_dv = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_b = _rtB ->
B_40_158_0 ; } else if ( _rtDW -> LastMajorTimeB_g == ( rtInf ) ) { _rtDW ->
LastMajorTimeB_g = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_i = _rtB ->
B_40_158_0 ; } else if ( _rtDW -> LastMajorTimeA_dv < _rtDW ->
LastMajorTimeB_g ) { _rtDW -> LastMajorTimeA_dv = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYA_b = _rtB -> B_40_158_0 ; } else { _rtDW -> LastMajorTimeB_g =
ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_i = _rtB -> B_40_158_0 ; } _rtDW ->
vv0at_IWORK_j = 0 ; { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_p . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T *
* ) & _rtDW -> TransportDelay_PWORK_p . TUbufferPtrs [ 1 ] ; real_T simTime =
ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_d . Head = ( ( _rtDW ->
TransportDelay_IWORK_d . Head < ( _rtDW -> TransportDelay_IWORK_d .
CircularBufSize - 1 ) ) ? ( _rtDW -> TransportDelay_IWORK_d . Head + 1 ) : 0
) ; if ( _rtDW -> TransportDelay_IWORK_d . Head == _rtDW ->
TransportDelay_IWORK_d . Tail ) { if ( !
Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_d . CircularBufSize , & _rtDW -> TransportDelay_IWORK_d
. Tail , & _rtDW -> TransportDelay_IWORK_d . Head , & _rtDW ->
TransportDelay_IWORK_d . Last , simTime - _rtP -> P_127 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_d .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_d . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_d . Head ] = _rtB ->
B_40_155_0 ; } if ( _rtDW -> LastMajorTimeA_gv == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_gv = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_b5 = _rtB ->
B_40_175_0 ; } else if ( _rtDW -> LastMajorTimeB_e4 == ( rtInf ) ) { _rtDW ->
LastMajorTimeB_e4 = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_a = _rtB ->
B_40_175_0 ; } else if ( _rtDW -> LastMajorTimeA_gv < _rtDW ->
LastMajorTimeB_e4 ) { _rtDW -> LastMajorTimeA_gv = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYA_b5 = _rtB -> B_40_175_0 ; } else { _rtDW -> LastMajorTimeB_e4
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_a = _rtB -> B_40_175_0 ; } _rtDW
-> vv0at_IWORK_e = 0 ; { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_o . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T *
* ) & _rtDW -> TransportDelay_PWORK_o . TUbufferPtrs [ 1 ] ; real_T simTime =
ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_cu . Head = ( ( _rtDW ->
TransportDelay_IWORK_cu . Head < ( _rtDW -> TransportDelay_IWORK_cu .
CircularBufSize - 1 ) ) ? ( _rtDW -> TransportDelay_IWORK_cu . Head + 1 ) : 0
) ; if ( _rtDW -> TransportDelay_IWORK_cu . Head == _rtDW ->
TransportDelay_IWORK_cu . Tail ) { if ( !
Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_cu . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_cu . Tail , & _rtDW -> TransportDelay_IWORK_cu . Head ,
& _rtDW -> TransportDelay_IWORK_cu . Last , simTime - _rtP -> P_141 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_cu . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_cu . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_cu . Head ] = _rtB -> B_40_172_0 ; } if ( _rtDW ->
LastMajorTimeA_p == ( rtInf ) ) { _rtDW -> LastMajorTimeA_p = ssGetTaskTime (
S , 0 ) ; _rtDW -> PrevYA_i = _rtB -> B_40_192_0 ; } else if ( _rtDW ->
LastMajorTimeB_hn == ( rtInf ) ) { _rtDW -> LastMajorTimeB_hn = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_gc = _rtB -> B_40_192_0 ; } else if ( _rtDW ->
LastMajorTimeA_p < _rtDW -> LastMajorTimeB_hn ) { _rtDW -> LastMajorTimeA_p =
ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_i = _rtB -> B_40_192_0 ; } else {
_rtDW -> LastMajorTimeB_hn = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_gc =
_rtB -> B_40_192_0 ; } _rtDW -> vv0at_IWORK_n = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_dz . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_dz .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_j . Head = ( ( _rtDW -> TransportDelay_IWORK_j . Head <
( _rtDW -> TransportDelay_IWORK_j . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_j . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_j . Head == _rtDW -> TransportDelay_IWORK_j . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_j . CircularBufSize , & _rtDW -> TransportDelay_IWORK_j
. Tail , & _rtDW -> TransportDelay_IWORK_j . Head , & _rtDW ->
TransportDelay_IWORK_j . Last , simTime - _rtP -> P_155 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_j .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_j . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_j . Head ] = _rtB ->
B_40_189_0 ; } if ( _rtDW -> LastMajorTimeA_o == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_o = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_j = _rtB ->
B_40_209_0 ; } else if ( _rtDW -> LastMajorTimeB_gq == ( rtInf ) ) { _rtDW ->
LastMajorTimeB_gq = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_g2 = _rtB ->
B_40_209_0 ; } else if ( _rtDW -> LastMajorTimeA_o < _rtDW ->
LastMajorTimeB_gq ) { _rtDW -> LastMajorTimeA_o = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYA_j = _rtB -> B_40_209_0 ; } else { _rtDW -> LastMajorTimeB_gq
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_g2 = _rtB -> B_40_209_0 ; } _rtDW
-> vv0at_IWORK_f = 0 ; { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_mf . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T
* * ) & _rtDW -> TransportDelay_PWORK_mf . TUbufferPtrs [ 1 ] ; real_T
simTime = ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_bo . Head = ( ( _rtDW
-> TransportDelay_IWORK_bo . Head < ( _rtDW -> TransportDelay_IWORK_bo .
CircularBufSize - 1 ) ) ? ( _rtDW -> TransportDelay_IWORK_bo . Head + 1 ) : 0
) ; if ( _rtDW -> TransportDelay_IWORK_bo . Head == _rtDW ->
TransportDelay_IWORK_bo . Tail ) { if ( !
Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_bo . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_bo . Tail , & _rtDW -> TransportDelay_IWORK_bo . Head ,
& _rtDW -> TransportDelay_IWORK_bo . Last , simTime - _rtP -> P_169 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_bo . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_bo . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_bo . Head ] = _rtB -> B_40_206_0 ; } if ( _rtDW ->
LastMajorTimeA_c == ( rtInf ) ) { _rtDW -> LastMajorTimeA_c = ssGetTaskTime (
S , 0 ) ; _rtDW -> PrevYA_jn = _rtB -> B_40_226_0 ; } else if ( _rtDW ->
LastMajorTimeB_i == ( rtInf ) ) { _rtDW -> LastMajorTimeB_i = ssGetTaskTime (
S , 0 ) ; _rtDW -> PrevYB_p = _rtB -> B_40_226_0 ; } else if ( _rtDW ->
LastMajorTimeA_c < _rtDW -> LastMajorTimeB_i ) { _rtDW -> LastMajorTimeA_c =
ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_jn = _rtB -> B_40_226_0 ; } else {
_rtDW -> LastMajorTimeB_i = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_p =
_rtB -> B_40_226_0 ; } _rtDW -> vv0at_IWORK_g = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_mc . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_mc .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_ko . Head = ( ( _rtDW -> TransportDelay_IWORK_ko . Head
< ( _rtDW -> TransportDelay_IWORK_ko . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_ko . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_ko . Head == _rtDW -> TransportDelay_IWORK_ko . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_ko . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_ko . Tail , & _rtDW -> TransportDelay_IWORK_ko . Head ,
& _rtDW -> TransportDelay_IWORK_ko . Last , simTime - _rtP -> P_183 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_ko . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_ko . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_ko . Head ] = _rtB -> B_40_223_0 ; } if ( _rtDW ->
LastMajorTimeA_dm == ( rtInf ) ) { _rtDW -> LastMajorTimeA_dm = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_od = _rtB -> B_40_243_0 ; } else if ( _rtDW ->
LastMajorTimeB_o == ( rtInf ) ) { _rtDW -> LastMajorTimeB_o = ssGetTaskTime (
S , 0 ) ; _rtDW -> PrevYB_h0 = _rtB -> B_40_243_0 ; } else if ( _rtDW ->
LastMajorTimeA_dm < _rtDW -> LastMajorTimeB_o ) { _rtDW -> LastMajorTimeA_dm
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_od = _rtB -> B_40_243_0 ; } else
{ _rtDW -> LastMajorTimeB_o = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_h0 =
_rtB -> B_40_243_0 ; } _rtDW -> vv0at_IWORK_a = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_l0w . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_l0w .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_g . Head = ( ( _rtDW -> TransportDelay_IWORK_g . Head <
( _rtDW -> TransportDelay_IWORK_g . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_g . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_g . Head == _rtDW -> TransportDelay_IWORK_g . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_g . CircularBufSize , & _rtDW -> TransportDelay_IWORK_g
. Tail , & _rtDW -> TransportDelay_IWORK_g . Head , & _rtDW ->
TransportDelay_IWORK_g . Last , simTime - _rtP -> P_197 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_g .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_g . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_g . Head ] = _rtB ->
B_40_240_0 ; } if ( _rtDW -> LastMajorTimeA_ga == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_ga = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_e1 = _rtB ->
B_40_260_0 ; } else if ( _rtDW -> LastMajorTimeB_b == ( rtInf ) ) { _rtDW ->
LastMajorTimeB_b = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_m = _rtB ->
B_40_260_0 ; } else if ( _rtDW -> LastMajorTimeA_ga < _rtDW ->
LastMajorTimeB_b ) { _rtDW -> LastMajorTimeA_ga = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYA_e1 = _rtB -> B_40_260_0 ; } else { _rtDW -> LastMajorTimeB_b
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_m = _rtB -> B_40_260_0 ; } _rtDW
-> vv0at_IWORK_gr = 0 ; { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_gu . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T
* * ) & _rtDW -> TransportDelay_PWORK_gu . TUbufferPtrs [ 1 ] ; real_T
simTime = ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_pq . Head = ( ( _rtDW
-> TransportDelay_IWORK_pq . Head < ( _rtDW -> TransportDelay_IWORK_pq .
CircularBufSize - 1 ) ) ? ( _rtDW -> TransportDelay_IWORK_pq . Head + 1 ) : 0
) ; if ( _rtDW -> TransportDelay_IWORK_pq . Head == _rtDW ->
TransportDelay_IWORK_pq . Tail ) { if ( !
Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_pq . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_pq . Tail , & _rtDW -> TransportDelay_IWORK_pq . Head ,
& _rtDW -> TransportDelay_IWORK_pq . Last , simTime - _rtP -> P_211 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_pq . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_pq . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_pq . Head ] = _rtB -> B_40_257_0 ; } if ( _rtDW ->
LastMajorTimeA_cs == ( rtInf ) ) { _rtDW -> LastMajorTimeA_cs = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_l = _rtB -> B_40_277_0 ; } else if ( _rtDW ->
LastMajorTimeB_ms == ( rtInf ) ) { _rtDW -> LastMajorTimeB_ms = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_c3 = _rtB -> B_40_277_0 ; } else if ( _rtDW ->
LastMajorTimeA_cs < _rtDW -> LastMajorTimeB_ms ) { _rtDW -> LastMajorTimeA_cs
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_l = _rtB -> B_40_277_0 ; } else {
_rtDW -> LastMajorTimeB_ms = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_c3 =
_rtB -> B_40_277_0 ; } _rtDW -> vv0at_IWORK_m = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_c . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_c . TUbufferPtrs
[ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_i .
Head = ( ( _rtDW -> TransportDelay_IWORK_i . Head < ( _rtDW ->
TransportDelay_IWORK_i . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_i . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_i . Head == _rtDW -> TransportDelay_IWORK_i . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_i . CircularBufSize , & _rtDW -> TransportDelay_IWORK_i
. Tail , & _rtDW -> TransportDelay_IWORK_i . Head , & _rtDW ->
TransportDelay_IWORK_i . Last , simTime - _rtP -> P_225 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_i .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_i . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_i . Head ] = _rtB ->
B_40_274_0 ; } if ( _rtDW -> LastMajorTimeA_dvp == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_dvp = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_oq = _rtB ->
B_40_294_0 ; } else if ( _rtDW -> LastMajorTimeB_ej == ( rtInf ) ) { _rtDW ->
LastMajorTimeB_ej = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_mu = _rtB ->
B_40_294_0 ; } else if ( _rtDW -> LastMajorTimeA_dvp < _rtDW ->
LastMajorTimeB_ej ) { _rtDW -> LastMajorTimeA_dvp = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYA_oq = _rtB -> B_40_294_0 ; } else { _rtDW -> LastMajorTimeB_ej
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_mu = _rtB -> B_40_294_0 ; } _rtDW
-> vv0at_IWORK_eh = 0 ; { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_k . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T *
* ) & _rtDW -> TransportDelay_PWORK_k . TUbufferPtrs [ 1 ] ; real_T simTime =
ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_p0 . Head = ( ( _rtDW ->
TransportDelay_IWORK_p0 . Head < ( _rtDW -> TransportDelay_IWORK_p0 .
CircularBufSize - 1 ) ) ? ( _rtDW -> TransportDelay_IWORK_p0 . Head + 1 ) : 0
) ; if ( _rtDW -> TransportDelay_IWORK_p0 . Head == _rtDW ->
TransportDelay_IWORK_p0 . Tail ) { if ( !
Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_p0 . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_p0 . Tail , & _rtDW -> TransportDelay_IWORK_p0 . Head ,
& _rtDW -> TransportDelay_IWORK_p0 . Last , simTime - _rtP -> P_239 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_p0 . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_p0 . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_p0 . Head ] = _rtB -> B_40_291_0 ; } if ( _rtDW ->
LastMajorTimeA_in == ( rtInf ) ) { _rtDW -> LastMajorTimeA_in = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_lw = _rtB -> B_40_311_0 ; } else if ( _rtDW ->
LastMajorTimeB_ha == ( rtInf ) ) { _rtDW -> LastMajorTimeB_ha = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_hj = _rtB -> B_40_311_0 ; } else if ( _rtDW ->
LastMajorTimeA_in < _rtDW -> LastMajorTimeB_ha ) { _rtDW -> LastMajorTimeA_in
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_lw = _rtB -> B_40_311_0 ; } else
{ _rtDW -> LastMajorTimeB_ha = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_hj =
_rtB -> B_40_311_0 ; } _rtDW -> vv0at_IWORK_km = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_i . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_i . TUbufferPtrs
[ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_o .
Head = ( ( _rtDW -> TransportDelay_IWORK_o . Head < ( _rtDW ->
TransportDelay_IWORK_o . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_o . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_o . Head == _rtDW -> TransportDelay_IWORK_o . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_o . CircularBufSize , & _rtDW -> TransportDelay_IWORK_o
. Tail , & _rtDW -> TransportDelay_IWORK_o . Head , & _rtDW ->
TransportDelay_IWORK_o . Last , simTime - _rtP -> P_253 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_o .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_o . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_o . Head ] = _rtB ->
B_40_308_0 ; } if ( _rtDW -> LastMajorTimeA_iy == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_iy = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_l3 = _rtB ->
B_40_328_0 ; } else if ( _rtDW -> LastMajorTimeB_ij == ( rtInf ) ) { _rtDW ->
LastMajorTimeB_ij = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_d = _rtB ->
B_40_328_0 ; } else if ( _rtDW -> LastMajorTimeA_iy < _rtDW ->
LastMajorTimeB_ij ) { _rtDW -> LastMajorTimeA_iy = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYA_l3 = _rtB -> B_40_328_0 ; } else { _rtDW -> LastMajorTimeB_ij
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_d = _rtB -> B_40_328_0 ; } _rtDW
-> vv0at_IWORK_oo = 0 ; { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_mfn . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T
* * ) & _rtDW -> TransportDelay_PWORK_mfn . TUbufferPtrs [ 1 ] ; real_T
simTime = ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_co . Head = ( ( _rtDW
-> TransportDelay_IWORK_co . Head < ( _rtDW -> TransportDelay_IWORK_co .
CircularBufSize - 1 ) ) ? ( _rtDW -> TransportDelay_IWORK_co . Head + 1 ) : 0
) ; if ( _rtDW -> TransportDelay_IWORK_co . Head == _rtDW ->
TransportDelay_IWORK_co . Tail ) { if ( !
Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_co . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_co . Tail , & _rtDW -> TransportDelay_IWORK_co . Head ,
& _rtDW -> TransportDelay_IWORK_co . Last , simTime - _rtP -> P_267 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_co . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_co . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_co . Head ] = _rtB -> B_40_325_0 ; } if ( _rtDW ->
LastMajorTimeA_k == ( rtInf ) ) { _rtDW -> LastMajorTimeA_k = ssGetTaskTime (
S , 0 ) ; _rtDW -> PrevYA_bz = _rtB -> B_40_345_0 ; } else if ( _rtDW ->
LastMajorTimeB_oh == ( rtInf ) ) { _rtDW -> LastMajorTimeB_oh = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_b = _rtB -> B_40_345_0 ; } else if ( _rtDW ->
LastMajorTimeA_k < _rtDW -> LastMajorTimeB_oh ) { _rtDW -> LastMajorTimeA_k =
ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_bz = _rtB -> B_40_345_0 ; } else {
_rtDW -> LastMajorTimeB_oh = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_b =
_rtB -> B_40_345_0 ; } _rtDW -> vv0at_IWORK_e5 = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_b . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_b . TUbufferPtrs
[ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_gp .
Head = ( ( _rtDW -> TransportDelay_IWORK_gp . Head < ( _rtDW ->
TransportDelay_IWORK_gp . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_gp . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_gp . Head == _rtDW -> TransportDelay_IWORK_gp . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_gp . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_gp . Tail , & _rtDW -> TransportDelay_IWORK_gp . Head ,
& _rtDW -> TransportDelay_IWORK_gp . Last , simTime - _rtP -> P_281 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_gp . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_gp . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_gp . Head ] = _rtB -> B_40_342_0 ; } if ( _rtDW ->
LastMajorTimeA_bd == ( rtInf ) ) { _rtDW -> LastMajorTimeA_bd = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_ir = _rtB -> B_40_362_0 ; } else if ( _rtDW ->
LastMajorTimeB_lw == ( rtInf ) ) { _rtDW -> LastMajorTimeB_lw = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_an = _rtB -> B_40_362_0 ; } else if ( _rtDW ->
LastMajorTimeA_bd < _rtDW -> LastMajorTimeB_lw ) { _rtDW -> LastMajorTimeA_bd
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_ir = _rtB -> B_40_362_0 ; } else
{ _rtDW -> LastMajorTimeB_lw = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_an =
_rtB -> B_40_362_0 ; } _rtDW -> vv0at_IWORK_oe = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_by . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_by .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_e . Head = ( ( _rtDW -> TransportDelay_IWORK_e . Head <
( _rtDW -> TransportDelay_IWORK_e . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_e . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_e . Head == _rtDW -> TransportDelay_IWORK_e . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_e . CircularBufSize , & _rtDW -> TransportDelay_IWORK_e
. Tail , & _rtDW -> TransportDelay_IWORK_e . Head , & _rtDW ->
TransportDelay_IWORK_e . Last , simTime - _rtP -> P_295 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_e .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_e . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_e . Head ] = _rtB ->
B_40_359_0 ; } if ( _rtDW -> LastMajorTimeA_f == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_f = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_m1 = _rtB ->
B_40_379_0 ; } else if ( _rtDW -> LastMajorTimeB_gm == ( rtInf ) ) { _rtDW ->
LastMajorTimeB_gm = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_fh = _rtB ->
B_40_379_0 ; } else if ( _rtDW -> LastMajorTimeA_f < _rtDW ->
LastMajorTimeB_gm ) { _rtDW -> LastMajorTimeA_f = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYA_m1 = _rtB -> B_40_379_0 ; } else { _rtDW -> LastMajorTimeB_gm
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_fh = _rtB -> B_40_379_0 ; } _rtDW
-> vv0at_IWORK_et = 0 ; { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_j . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T *
* ) & _rtDW -> TransportDelay_PWORK_j . TUbufferPtrs [ 1 ] ; real_T simTime =
ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_nc . Head = ( ( _rtDW ->
TransportDelay_IWORK_nc . Head < ( _rtDW -> TransportDelay_IWORK_nc .
CircularBufSize - 1 ) ) ? ( _rtDW -> TransportDelay_IWORK_nc . Head + 1 ) : 0
) ; if ( _rtDW -> TransportDelay_IWORK_nc . Head == _rtDW ->
TransportDelay_IWORK_nc . Tail ) { if ( !
Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_nc . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_nc . Tail , & _rtDW -> TransportDelay_IWORK_nc . Head ,
& _rtDW -> TransportDelay_IWORK_nc . Last , simTime - _rtP -> P_309 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_nc . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_nc . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_nc . Head ] = _rtB -> B_40_376_0 ; } if ( _rtDW ->
LastMajorTimeA_ch == ( rtInf ) ) { _rtDW -> LastMajorTimeA_ch = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_g = _rtB -> B_40_396_0 ; } else if ( _rtDW ->
LastMajorTimeB_ar == ( rtInf ) ) { _rtDW -> LastMajorTimeB_ar = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_gl = _rtB -> B_40_396_0 ; } else if ( _rtDW ->
LastMajorTimeA_ch < _rtDW -> LastMajorTimeB_ar ) { _rtDW -> LastMajorTimeA_ch
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_g = _rtB -> B_40_396_0 ; } else {
_rtDW -> LastMajorTimeB_ar = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_gl =
_rtB -> B_40_396_0 ; } _rtDW -> vv0at_IWORK_je = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_dm . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_dm .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_di . Head = ( ( _rtDW -> TransportDelay_IWORK_di . Head
< ( _rtDW -> TransportDelay_IWORK_di . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_di . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_di . Head == _rtDW -> TransportDelay_IWORK_di . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_di . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_di . Tail , & _rtDW -> TransportDelay_IWORK_di . Head ,
& _rtDW -> TransportDelay_IWORK_di . Last , simTime - _rtP -> P_323 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_di . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_di . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_di . Head ] = _rtB -> B_40_393_0 ; } if ( _rtDW ->
LastMajorTimeA_l2 == ( rtInf ) ) { _rtDW -> LastMajorTimeA_l2 = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_ey = _rtB -> B_40_413_0 ; } else if ( _rtDW ->
LastMajorTimeB_la == ( rtInf ) ) { _rtDW -> LastMajorTimeB_la = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_ar = _rtB -> B_40_413_0 ; } else if ( _rtDW ->
LastMajorTimeA_l2 < _rtDW -> LastMajorTimeB_la ) { _rtDW -> LastMajorTimeA_l2
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_ey = _rtB -> B_40_413_0 ; } else
{ _rtDW -> LastMajorTimeB_la = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_ar =
_rtB -> B_40_413_0 ; } _rtDW -> vv0at_IWORK_hz = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_a1 . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_a1 .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_kv . Head = ( ( _rtDW -> TransportDelay_IWORK_kv . Head
< ( _rtDW -> TransportDelay_IWORK_kv . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_kv . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_kv . Head == _rtDW -> TransportDelay_IWORK_kv . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_kv . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_kv . Tail , & _rtDW -> TransportDelay_IWORK_kv . Head ,
& _rtDW -> TransportDelay_IWORK_kv . Last , simTime - _rtP -> P_337 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_kv . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_kv . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_kv . Head ] = _rtB -> B_40_410_0 ; } if ( _rtDW ->
LastMajorTimeA_iyq == ( rtInf ) ) { _rtDW -> LastMajorTimeA_iyq =
ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_i3 = _rtB -> B_40_430_0 ; } else if
( _rtDW -> LastMajorTimeB_id == ( rtInf ) ) { _rtDW -> LastMajorTimeB_id =
ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_bg = _rtB -> B_40_430_0 ; } else if
( _rtDW -> LastMajorTimeA_iyq < _rtDW -> LastMajorTimeB_id ) { _rtDW ->
LastMajorTimeA_iyq = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_i3 = _rtB ->
B_40_430_0 ; } else { _rtDW -> LastMajorTimeB_id = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYB_bg = _rtB -> B_40_430_0 ; } _rtDW -> vv0at_IWORK_k4 = 0 ; {
real_T * * uBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_n .
TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_n . TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ;
_rtDW -> TransportDelay_IWORK_ec . Head = ( ( _rtDW ->
TransportDelay_IWORK_ec . Head < ( _rtDW -> TransportDelay_IWORK_ec .
CircularBufSize - 1 ) ) ? ( _rtDW -> TransportDelay_IWORK_ec . Head + 1 ) : 0
) ; if ( _rtDW -> TransportDelay_IWORK_ec . Head == _rtDW ->
TransportDelay_IWORK_ec . Tail ) { if ( !
Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_ec . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_ec . Tail , & _rtDW -> TransportDelay_IWORK_ec . Head ,
& _rtDW -> TransportDelay_IWORK_ec . Last , simTime - _rtP -> P_351 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_ec . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_ec . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_ec . Head ] = _rtB -> B_40_427_0 ; } if ( _rtDW ->
LastMajorTimeA_k2 == ( rtInf ) ) { _rtDW -> LastMajorTimeA_k2 = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_c = _rtB -> B_40_447_0 ; } else if ( _rtDW ->
LastMajorTimeB_ko == ( rtInf ) ) { _rtDW -> LastMajorTimeB_ko = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_av = _rtB -> B_40_447_0 ; } else if ( _rtDW ->
LastMajorTimeA_k2 < _rtDW -> LastMajorTimeB_ko ) { _rtDW -> LastMajorTimeA_k2
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_c = _rtB -> B_40_447_0 ; } else {
_rtDW -> LastMajorTimeB_ko = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_av =
_rtB -> B_40_447_0 ; } _rtDW -> vv0at_IWORK_j4 = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_gj . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_gj .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_j1 . Head = ( ( _rtDW -> TransportDelay_IWORK_j1 . Head
< ( _rtDW -> TransportDelay_IWORK_j1 . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_j1 . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_j1 . Head == _rtDW -> TransportDelay_IWORK_j1 . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_j1 . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_j1 . Tail , & _rtDW -> TransportDelay_IWORK_j1 . Head ,
& _rtDW -> TransportDelay_IWORK_j1 . Last , simTime - _rtP -> P_365 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_j1 . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_j1 . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_j1 . Head ] = _rtB -> B_40_444_0 ; } if ( _rtDW ->
LastMajorTimeA_e == ( rtInf ) ) { _rtDW -> LastMajorTimeA_e = ssGetTaskTime (
S , 0 ) ; _rtDW -> PrevYA_ey0 = _rtB -> B_40_464_0 ; } else if ( _rtDW ->
LastMajorTimeB_gmz == ( rtInf ) ) { _rtDW -> LastMajorTimeB_gmz =
ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_j = _rtB -> B_40_464_0 ; } else if
( _rtDW -> LastMajorTimeA_e < _rtDW -> LastMajorTimeB_gmz ) { _rtDW ->
LastMajorTimeA_e = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_ey0 = _rtB ->
B_40_464_0 ; } else { _rtDW -> LastMajorTimeB_gmz = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYB_j = _rtB -> B_40_464_0 ; } _rtDW -> vv0at_IWORK_b = 0 ; {
real_T * * uBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_e .
TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_e . TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ;
_rtDW -> TransportDelay_IWORK_kh . Head = ( ( _rtDW ->
TransportDelay_IWORK_kh . Head < ( _rtDW -> TransportDelay_IWORK_kh .
CircularBufSize - 1 ) ) ? ( _rtDW -> TransportDelay_IWORK_kh . Head + 1 ) : 0
) ; if ( _rtDW -> TransportDelay_IWORK_kh . Head == _rtDW ->
TransportDelay_IWORK_kh . Tail ) { if ( !
Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_kh . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_kh . Tail , & _rtDW -> TransportDelay_IWORK_kh . Head ,
& _rtDW -> TransportDelay_IWORK_kh . Last , simTime - _rtP -> P_379 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_kh . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_kh . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_kh . Head ] = _rtB -> B_40_461_0 ; } if ( _rtDW ->
LastMajorTimeA_gp == ( rtInf ) ) { _rtDW -> LastMajorTimeA_gp = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_b5t = _rtB -> B_40_481_0 ; } else if ( _rtDW ->
LastMajorTimeB_l5 == ( rtInf ) ) { _rtDW -> LastMajorTimeB_l5 = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_l4 = _rtB -> B_40_481_0 ; } else if ( _rtDW ->
LastMajorTimeA_gp < _rtDW -> LastMajorTimeB_l5 ) { _rtDW -> LastMajorTimeA_gp
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_b5t = _rtB -> B_40_481_0 ; } else
{ _rtDW -> LastMajorTimeB_l5 = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_l4 =
_rtB -> B_40_481_0 ; } _rtDW -> vv0at_IWORK_oi = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_nv . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_nv .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_dt . Head = ( ( _rtDW -> TransportDelay_IWORK_dt . Head
< ( _rtDW -> TransportDelay_IWORK_dt . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_dt . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_dt . Head == _rtDW -> TransportDelay_IWORK_dt . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_dt . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_dt . Tail , & _rtDW -> TransportDelay_IWORK_dt . Head ,
& _rtDW -> TransportDelay_IWORK_dt . Last , simTime - _rtP -> P_393 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_dt . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_dt . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_dt . Head ] = _rtB -> B_40_478_0 ; } if ( _rtDW ->
LastMajorTimeA_or == ( rtInf ) ) { _rtDW -> LastMajorTimeA_or = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_o0 = _rtB -> B_40_498_0 ; } else if ( _rtDW ->
LastMajorTimeB_bw == ( rtInf ) ) { _rtDW -> LastMajorTimeB_bw = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_bs = _rtB -> B_40_498_0 ; } else if ( _rtDW ->
LastMajorTimeA_or < _rtDW -> LastMajorTimeB_bw ) { _rtDW -> LastMajorTimeA_or
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_o0 = _rtB -> B_40_498_0 ; } else
{ _rtDW -> LastMajorTimeB_bw = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_bs =
_rtB -> B_40_498_0 ; } _rtDW -> vv0at_IWORK_dj = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_lj . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_lj .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_f . Head = ( ( _rtDW -> TransportDelay_IWORK_f . Head <
( _rtDW -> TransportDelay_IWORK_f . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_f . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_f . Head == _rtDW -> TransportDelay_IWORK_f . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_f . CircularBufSize , & _rtDW -> TransportDelay_IWORK_f
. Tail , & _rtDW -> TransportDelay_IWORK_f . Head , & _rtDW ->
TransportDelay_IWORK_f . Last , simTime - _rtP -> P_407 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_f .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_f . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_f . Head ] = _rtB ->
B_40_495_0 ; } if ( _rtDW -> LastMajorTimeA_da == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_da = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_ek = _rtB ->
B_40_515_0 ; } else if ( _rtDW -> LastMajorTimeB_en == ( rtInf ) ) { _rtDW ->
LastMajorTimeB_en = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_a0 = _rtB ->
B_40_515_0 ; } else if ( _rtDW -> LastMajorTimeA_da < _rtDW ->
LastMajorTimeB_en ) { _rtDW -> LastMajorTimeA_da = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYA_ek = _rtB -> B_40_515_0 ; } else { _rtDW -> LastMajorTimeB_en
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_a0 = _rtB -> B_40_515_0 ; } _rtDW
-> vv0at_IWORK_op = 0 ; { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_js . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T
* * ) & _rtDW -> TransportDelay_PWORK_js . TUbufferPtrs [ 1 ] ; real_T
simTime = ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_pu . Head = ( ( _rtDW
-> TransportDelay_IWORK_pu . Head < ( _rtDW -> TransportDelay_IWORK_pu .
CircularBufSize - 1 ) ) ? ( _rtDW -> TransportDelay_IWORK_pu . Head + 1 ) : 0
) ; if ( _rtDW -> TransportDelay_IWORK_pu . Head == _rtDW ->
TransportDelay_IWORK_pu . Tail ) { if ( !
Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_pu . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_pu . Tail , & _rtDW -> TransportDelay_IWORK_pu . Head ,
& _rtDW -> TransportDelay_IWORK_pu . Last , simTime - _rtP -> P_421 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_pu . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_pu . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_pu . Head ] = _rtB -> B_40_512_0 ; } if ( _rtDW ->
LastMajorTimeA_p2 == ( rtInf ) ) { _rtDW -> LastMajorTimeA_p2 = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_cq = _rtB -> B_40_532_0 ; } else if ( _rtDW ->
LastMajorTimeB_j == ( rtInf ) ) { _rtDW -> LastMajorTimeB_j = ssGetTaskTime (
S , 0 ) ; _rtDW -> PrevYB_lk = _rtB -> B_40_532_0 ; } else if ( _rtDW ->
LastMajorTimeA_p2 < _rtDW -> LastMajorTimeB_j ) { _rtDW -> LastMajorTimeA_p2
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_cq = _rtB -> B_40_532_0 ; } else
{ _rtDW -> LastMajorTimeB_j = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_lk =
_rtB -> B_40_532_0 ; } _rtDW -> vv0at_IWORK_bo = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_mg . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_mg .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_a . Head = ( ( _rtDW -> TransportDelay_IWORK_a . Head <
( _rtDW -> TransportDelay_IWORK_a . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_a . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_a . Head == _rtDW -> TransportDelay_IWORK_a . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_a . CircularBufSize , & _rtDW -> TransportDelay_IWORK_a
. Tail , & _rtDW -> TransportDelay_IWORK_a . Head , & _rtDW ->
TransportDelay_IWORK_a . Last , simTime - _rtP -> P_435 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_a .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_a . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_a . Head ] = _rtB ->
B_40_529_0 ; } if ( _rtDW -> LastMajorTimeA_n == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_n = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_k = _rtB ->
B_40_549_0 ; } else if ( _rtDW -> LastMajorTimeB_kq == ( rtInf ) ) { _rtDW ->
LastMajorTimeB_kq = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_al = _rtB ->
B_40_549_0 ; } else if ( _rtDW -> LastMajorTimeA_n < _rtDW ->
LastMajorTimeB_kq ) { _rtDW -> LastMajorTimeA_n = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYA_k = _rtB -> B_40_549_0 ; } else { _rtDW -> LastMajorTimeB_kq
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_al = _rtB -> B_40_549_0 ; } _rtDW
-> vv0at_IWORK_f2 = 0 ; { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_f . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T *
* ) & _rtDW -> TransportDelay_PWORK_f . TUbufferPtrs [ 1 ] ; real_T simTime =
ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_i3 . Head = ( ( _rtDW ->
TransportDelay_IWORK_i3 . Head < ( _rtDW -> TransportDelay_IWORK_i3 .
CircularBufSize - 1 ) ) ? ( _rtDW -> TransportDelay_IWORK_i3 . Head + 1 ) : 0
) ; if ( _rtDW -> TransportDelay_IWORK_i3 . Head == _rtDW ->
TransportDelay_IWORK_i3 . Tail ) { if ( !
Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_i3 . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_i3 . Tail , & _rtDW -> TransportDelay_IWORK_i3 . Head ,
& _rtDW -> TransportDelay_IWORK_i3 . Last , simTime - _rtP -> P_449 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_i3 . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_i3 . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_i3 . Head ] = _rtB -> B_40_546_0 ; } if ( _rtDW ->
LastMajorTimeA_jq == ( rtInf ) ) { _rtDW -> LastMajorTimeA_jq = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_ce = _rtB -> B_40_566_0 ; } else if ( _rtDW ->
LastMajorTimeB_lj == ( rtInf ) ) { _rtDW -> LastMajorTimeB_lj = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_pq = _rtB -> B_40_566_0 ; } else if ( _rtDW ->
LastMajorTimeA_jq < _rtDW -> LastMajorTimeB_lj ) { _rtDW -> LastMajorTimeA_jq
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_ce = _rtB -> B_40_566_0 ; } else
{ _rtDW -> LastMajorTimeB_lj = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_pq =
_rtB -> B_40_566_0 ; } _rtDW -> vv0at_IWORK_aw = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_eb . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_eb .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_h . Head = ( ( _rtDW -> TransportDelay_IWORK_h . Head <
( _rtDW -> TransportDelay_IWORK_h . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_h . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_h . Head == _rtDW -> TransportDelay_IWORK_h . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_h . CircularBufSize , & _rtDW -> TransportDelay_IWORK_h
. Tail , & _rtDW -> TransportDelay_IWORK_h . Head , & _rtDW ->
TransportDelay_IWORK_h . Last , simTime - _rtP -> P_463 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_h .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_h . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_h . Head ] = _rtB ->
B_40_563_0 ; } if ( _rtDW -> LastMajorTimeA_h == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_h = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_da = _rtB ->
B_40_583_0 ; } else if ( _rtDW -> LastMajorTimeB_oj == ( rtInf ) ) { _rtDW ->
LastMajorTimeB_oj = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_p0 = _rtB ->
B_40_583_0 ; } else if ( _rtDW -> LastMajorTimeA_h < _rtDW ->
LastMajorTimeB_oj ) { _rtDW -> LastMajorTimeA_h = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYA_da = _rtB -> B_40_583_0 ; } else { _rtDW -> LastMajorTimeB_oj
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_p0 = _rtB -> B_40_583_0 ; } _rtDW
-> vv0at_IWORK_o3 = 0 ; { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_ks . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T
* * ) & _rtDW -> TransportDelay_PWORK_ks . TUbufferPtrs [ 1 ] ; real_T
simTime = ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_o4 . Head = ( ( _rtDW
-> TransportDelay_IWORK_o4 . Head < ( _rtDW -> TransportDelay_IWORK_o4 .
CircularBufSize - 1 ) ) ? ( _rtDW -> TransportDelay_IWORK_o4 . Head + 1 ) : 0
) ; if ( _rtDW -> TransportDelay_IWORK_o4 . Head == _rtDW ->
TransportDelay_IWORK_o4 . Tail ) { if ( !
Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_o4 . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_o4 . Tail , & _rtDW -> TransportDelay_IWORK_o4 . Head ,
& _rtDW -> TransportDelay_IWORK_o4 . Last , simTime - _rtP -> P_477 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_o4 . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_o4 . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_o4 . Head ] = _rtB -> B_40_580_0 ; } if ( _rtDW ->
LastMajorTimeA_e1 == ( rtInf ) ) { _rtDW -> LastMajorTimeA_e1 = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_cy = _rtB -> B_40_600_0 ; } else if ( _rtDW ->
LastMajorTimeB_ie == ( rtInf ) ) { _rtDW -> LastMajorTimeB_ie = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_jf = _rtB -> B_40_600_0 ; } else if ( _rtDW ->
LastMajorTimeA_e1 < _rtDW -> LastMajorTimeB_ie ) { _rtDW -> LastMajorTimeA_e1
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_cy = _rtB -> B_40_600_0 ; } else
{ _rtDW -> LastMajorTimeB_ie = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_jf =
_rtB -> B_40_600_0 ; } _rtDW -> vv0at_IWORK_gf = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_b5 . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_b5 .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_dy . Head = ( ( _rtDW -> TransportDelay_IWORK_dy . Head
< ( _rtDW -> TransportDelay_IWORK_dy . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_dy . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_dy . Head == _rtDW -> TransportDelay_IWORK_dy . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_dy . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_dy . Tail , & _rtDW -> TransportDelay_IWORK_dy . Head ,
& _rtDW -> TransportDelay_IWORK_dy . Last , simTime - _rtP -> P_491 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_dy . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_dy . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_dy . Head ] = _rtB -> B_40_597_0 ; } if ( _rtDW ->
LastMajorTimeA_a == ( rtInf ) ) { _rtDW -> LastMajorTimeA_a = ssGetTaskTime (
S , 0 ) ; _rtDW -> PrevYA_gg = _rtB -> B_40_617_0 ; } else if ( _rtDW ->
LastMajorTimeB_ih == ( rtInf ) ) { _rtDW -> LastMajorTimeB_ih = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_cc = _rtB -> B_40_617_0 ; } else if ( _rtDW ->
LastMajorTimeA_a < _rtDW -> LastMajorTimeB_ih ) { _rtDW -> LastMajorTimeA_a =
ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_gg = _rtB -> B_40_617_0 ; } else {
_rtDW -> LastMajorTimeB_ih = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_cc =
_rtB -> B_40_617_0 ; } _rtDW -> vv0at_IWORK_j0 = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_i5 . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_i5 .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_ek . Head = ( ( _rtDW -> TransportDelay_IWORK_ek . Head
< ( _rtDW -> TransportDelay_IWORK_ek . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_ek . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_ek . Head == _rtDW -> TransportDelay_IWORK_ek . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_ek . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_ek . Tail , & _rtDW -> TransportDelay_IWORK_ek . Head ,
& _rtDW -> TransportDelay_IWORK_ek . Last , simTime - _rtP -> P_505 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_ek . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_ek . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_ek . Head ] = _rtB -> B_40_614_0 ; } if ( _rtDW ->
LastMajorTimeA_chy == ( rtInf ) ) { _rtDW -> LastMajorTimeA_chy =
ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_dp = _rtB -> B_40_634_0 ; } else if
( _rtDW -> LastMajorTimeB_msx == ( rtInf ) ) { _rtDW -> LastMajorTimeB_msx =
ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_h5 = _rtB -> B_40_634_0 ; } else if
( _rtDW -> LastMajorTimeA_chy < _rtDW -> LastMajorTimeB_msx ) { _rtDW ->
LastMajorTimeA_chy = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_dp = _rtB ->
B_40_634_0 ; } else { _rtDW -> LastMajorTimeB_msx = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYB_h5 = _rtB -> B_40_634_0 ; } _rtDW -> vv0at_IWORK_og = 0 ; {
real_T * * uBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_am .
TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_am . TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S )
; _rtDW -> TransportDelay_IWORK_cus . Head = ( ( _rtDW ->
TransportDelay_IWORK_cus . Head < ( _rtDW -> TransportDelay_IWORK_cus .
CircularBufSize - 1 ) ) ? ( _rtDW -> TransportDelay_IWORK_cus . Head + 1 ) :
0 ) ; if ( _rtDW -> TransportDelay_IWORK_cus . Head == _rtDW ->
TransportDelay_IWORK_cus . Tail ) { if ( !
Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_cus . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_cus . Tail , & _rtDW -> TransportDelay_IWORK_cus . Head
, & _rtDW -> TransportDelay_IWORK_cus . Last , simTime - _rtP -> P_519 ,
tBuffer , uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_cus . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_cus . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_cus . Head ] = _rtB -> B_40_631_0 ; } if ( _rtDW ->
LastMajorTimeA_fk == ( rtInf ) ) { _rtDW -> LastMajorTimeA_fk = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_bq = _rtB -> B_40_651_0 ; } else if ( _rtDW ->
LastMajorTimeB_kn == ( rtInf ) ) { _rtDW -> LastMajorTimeB_kn = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_bt = _rtB -> B_40_651_0 ; } else if ( _rtDW ->
LastMajorTimeA_fk < _rtDW -> LastMajorTimeB_kn ) { _rtDW -> LastMajorTimeA_fk
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_bq = _rtB -> B_40_651_0 ; } else
{ _rtDW -> LastMajorTimeB_kn = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_bt =
_rtB -> B_40_651_0 ; } _rtDW -> vv0at_IWORK_ia = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_pb . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_pb .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_bob . Head = ( ( _rtDW -> TransportDelay_IWORK_bob .
Head < ( _rtDW -> TransportDelay_IWORK_bob . CircularBufSize - 1 ) ) ? (
_rtDW -> TransportDelay_IWORK_bob . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_bob . Head == _rtDW -> TransportDelay_IWORK_bob . Tail )
{ if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_bob . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_bob . Tail , & _rtDW -> TransportDelay_IWORK_bob . Head
, & _rtDW -> TransportDelay_IWORK_bob . Last , simTime - _rtP -> P_533 ,
tBuffer , uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_bob . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_bob . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_bob . Head ] = _rtB -> B_40_648_0 ; } if ( _rtDW ->
LastMajorTimeA_n5 == ( rtInf ) ) { _rtDW -> LastMajorTimeA_n5 = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_ls = _rtB -> B_40_668_0 ; } else if ( _rtDW ->
LastMajorTimeB_mq == ( rtInf ) ) { _rtDW -> LastMajorTimeB_mq = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_mq = _rtB -> B_40_668_0 ; } else if ( _rtDW ->
LastMajorTimeA_n5 < _rtDW -> LastMajorTimeB_mq ) { _rtDW -> LastMajorTimeA_n5
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_ls = _rtB -> B_40_668_0 ; } else
{ _rtDW -> LastMajorTimeB_mq = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_mq =
_rtB -> B_40_668_0 ; } _rtDW -> vv0at_IWORK_id = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_dp . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_dp .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_ep . Head = ( ( _rtDW -> TransportDelay_IWORK_ep . Head
< ( _rtDW -> TransportDelay_IWORK_ep . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_ep . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_ep . Head == _rtDW -> TransportDelay_IWORK_ep . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_ep . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_ep . Tail , & _rtDW -> TransportDelay_IWORK_ep . Head ,
& _rtDW -> TransportDelay_IWORK_ep . Last , simTime - _rtP -> P_547 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_ep . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_ep . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_ep . Head ] = _rtB -> B_40_665_0 ; } if ( _rtDW ->
LastMajorTimeA_kh == ( rtInf ) ) { _rtDW -> LastMajorTimeA_kh = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_mm = _rtB -> B_40_685_0 ; } else if ( _rtDW ->
LastMajorTimeB_og == ( rtInf ) ) { _rtDW -> LastMajorTimeB_og = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_lj = _rtB -> B_40_685_0 ; } else if ( _rtDW ->
LastMajorTimeA_kh < _rtDW -> LastMajorTimeB_og ) { _rtDW -> LastMajorTimeA_kh
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_mm = _rtB -> B_40_685_0 ; } else
{ _rtDW -> LastMajorTimeB_og = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_lj =
_rtB -> B_40_685_0 ; } _rtDW -> vv0at_IWORK_j1 = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_ct . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_ct .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_au . Head = ( ( _rtDW -> TransportDelay_IWORK_au . Head
< ( _rtDW -> TransportDelay_IWORK_au . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_au . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_au . Head == _rtDW -> TransportDelay_IWORK_au . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_au . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_au . Tail , & _rtDW -> TransportDelay_IWORK_au . Head ,
& _rtDW -> TransportDelay_IWORK_au . Last , simTime - _rtP -> P_561 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_au . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_au . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_au . Head ] = _rtB -> B_40_682_0 ; } UNUSED_PARAMETER (
tid ) ; }
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
B_40_806_0 ; _rtXdot -> vv0at_CSTATE = _rtB -> B_40_705_0 ; _rtXdot ->
TransferFcn_CSTATE = 0.0 ; _rtXdot -> TransferFcn_CSTATE += _rtP -> P_17 *
_rtX -> TransferFcn_CSTATE ; _rtXdot -> TransferFcn_CSTATE += _rtB ->
B_40_35_0 ; _rtXdot -> vv0at_CSTATE_k = _rtB -> B_40_707_0 ; _rtXdot ->
TransferFcn_CSTATE_c = 0.0 ; _rtXdot -> TransferFcn_CSTATE_c += _rtP -> P_31
* _rtX -> TransferFcn_CSTATE_c ; _rtXdot -> TransferFcn_CSTATE_c += _rtB ->
B_40_52_0 ; _rtXdot -> vv0at_CSTATE_o = _rtB -> B_40_709_0 ; _rtXdot ->
TransferFcn_CSTATE_n = 0.0 ; _rtXdot -> TransferFcn_CSTATE_n += _rtP -> P_45
* _rtX -> TransferFcn_CSTATE_n ; _rtXdot -> TransferFcn_CSTATE_n += _rtB ->
B_40_69_0 ; _rtXdot -> vv0at_CSTATE_h = _rtB -> B_40_711_0 ; _rtXdot ->
TransferFcn_CSTATE_b = 0.0 ; _rtXdot -> TransferFcn_CSTATE_b += _rtP -> P_59
* _rtX -> TransferFcn_CSTATE_b ; _rtXdot -> TransferFcn_CSTATE_b += _rtB ->
B_40_86_0 ; _rtXdot -> vv0at_CSTATE_kq = _rtB -> B_40_713_0 ; _rtXdot ->
TransferFcn_CSTATE_b0 = 0.0 ; _rtXdot -> TransferFcn_CSTATE_b0 += _rtP ->
P_73 * _rtX -> TransferFcn_CSTATE_b0 ; _rtXdot -> TransferFcn_CSTATE_b0 +=
_rtB -> B_40_103_0 ; _rtXdot -> vv0at_CSTATE_p = _rtB -> B_40_715_0 ; _rtXdot
-> TransferFcn_CSTATE_e = 0.0 ; _rtXdot -> TransferFcn_CSTATE_e += _rtP ->
P_87 * _rtX -> TransferFcn_CSTATE_e ; _rtXdot -> TransferFcn_CSTATE_e += _rtB
-> B_40_120_0 ; _rtXdot -> vv0at_CSTATE_p1 = _rtB -> B_40_717_0 ; _rtXdot ->
TransferFcn_CSTATE_ec = 0.0 ; _rtXdot -> TransferFcn_CSTATE_ec += _rtP ->
P_101 * _rtX -> TransferFcn_CSTATE_ec ; _rtXdot -> TransferFcn_CSTATE_ec +=
_rtB -> B_40_137_0 ; _rtXdot -> vv0at_CSTATE_l = _rtB -> B_40_719_0 ; _rtXdot
-> TransferFcn_CSTATE_k = 0.0 ; _rtXdot -> TransferFcn_CSTATE_k += _rtP ->
P_115 * _rtX -> TransferFcn_CSTATE_k ; _rtXdot -> TransferFcn_CSTATE_k +=
_rtB -> B_40_154_0 ; _rtXdot -> vv0at_CSTATE_ly = _rtB -> B_40_721_0 ;
_rtXdot -> TransferFcn_CSTATE_m = 0.0 ; _rtXdot -> TransferFcn_CSTATE_m +=
_rtP -> P_129 * _rtX -> TransferFcn_CSTATE_m ; _rtXdot ->
TransferFcn_CSTATE_m += _rtB -> B_40_171_0 ; _rtXdot -> vv0at_CSTATE_a = _rtB
-> B_40_723_0 ; _rtXdot -> TransferFcn_CSTATE_f = 0.0 ; _rtXdot ->
TransferFcn_CSTATE_f += _rtP -> P_143 * _rtX -> TransferFcn_CSTATE_f ;
_rtXdot -> TransferFcn_CSTATE_f += _rtB -> B_40_188_0 ; _rtXdot ->
vv0at_CSTATE_n = _rtB -> B_40_725_0 ; _rtXdot -> TransferFcn_CSTATE_o = 0.0 ;
_rtXdot -> TransferFcn_CSTATE_o += _rtP -> P_157 * _rtX ->
TransferFcn_CSTATE_o ; _rtXdot -> TransferFcn_CSTATE_o += _rtB -> B_40_205_0
; _rtXdot -> vv0at_CSTATE_j = _rtB -> B_40_727_0 ; _rtXdot ->
TransferFcn_CSTATE_oj = 0.0 ; _rtXdot -> TransferFcn_CSTATE_oj += _rtP ->
P_171 * _rtX -> TransferFcn_CSTATE_oj ; _rtXdot -> TransferFcn_CSTATE_oj +=
_rtB -> B_40_222_0 ; _rtXdot -> vv0at_CSTATE_of = _rtB -> B_40_729_0 ;
_rtXdot -> TransferFcn_CSTATE_p = 0.0 ; _rtXdot -> TransferFcn_CSTATE_p +=
_rtP -> P_185 * _rtX -> TransferFcn_CSTATE_p ; _rtXdot ->
TransferFcn_CSTATE_p += _rtB -> B_40_239_0 ; _rtXdot -> vv0at_CSTATE_b = _rtB
-> B_40_731_0 ; _rtXdot -> TransferFcn_CSTATE_l = 0.0 ; _rtXdot ->
TransferFcn_CSTATE_l += _rtP -> P_199 * _rtX -> TransferFcn_CSTATE_l ;
_rtXdot -> TransferFcn_CSTATE_l += _rtB -> B_40_256_0 ; _rtXdot ->
vv0at_CSTATE_n3 = _rtB -> B_40_733_0 ; _rtXdot -> TransferFcn_CSTATE_ou = 0.0
; _rtXdot -> TransferFcn_CSTATE_ou += _rtP -> P_213 * _rtX ->
TransferFcn_CSTATE_ou ; _rtXdot -> TransferFcn_CSTATE_ou += _rtB ->
B_40_273_0 ; _rtXdot -> vv0at_CSTATE_c = _rtB -> B_40_735_0 ; _rtXdot ->
TransferFcn_CSTATE_kc = 0.0 ; _rtXdot -> TransferFcn_CSTATE_kc += _rtP ->
P_227 * _rtX -> TransferFcn_CSTATE_kc ; _rtXdot -> TransferFcn_CSTATE_kc +=
_rtB -> B_40_290_0 ; _rtXdot -> vv0at_CSTATE_ch = _rtB -> B_40_737_0 ;
_rtXdot -> TransferFcn_CSTATE_i = 0.0 ; _rtXdot -> TransferFcn_CSTATE_i +=
_rtP -> P_241 * _rtX -> TransferFcn_CSTATE_i ; _rtXdot ->
TransferFcn_CSTATE_i += _rtB -> B_40_307_0 ; _rtXdot -> vv0at_CSTATE_bd =
_rtB -> B_40_739_0 ; _rtXdot -> TransferFcn_CSTATE_me = 0.0 ; _rtXdot ->
TransferFcn_CSTATE_me += _rtP -> P_255 * _rtX -> TransferFcn_CSTATE_me ;
_rtXdot -> TransferFcn_CSTATE_me += _rtB -> B_40_324_0 ; _rtXdot ->
vv0at_CSTATE_i = _rtB -> B_40_741_0 ; _rtXdot -> TransferFcn_CSTATE_lw = 0.0
; _rtXdot -> TransferFcn_CSTATE_lw += _rtP -> P_269 * _rtX ->
TransferFcn_CSTATE_lw ; _rtXdot -> TransferFcn_CSTATE_lw += _rtB ->
B_40_341_0 ; _rtXdot -> vv0at_CSTATE_le = _rtB -> B_40_743_0 ; _rtXdot ->
TransferFcn_CSTATE_ll = 0.0 ; _rtXdot -> TransferFcn_CSTATE_ll += _rtP ->
P_283 * _rtX -> TransferFcn_CSTATE_ll ; _rtXdot -> TransferFcn_CSTATE_ll +=
_rtB -> B_40_358_0 ; _rtXdot -> vv0at_CSTATE_e = _rtB -> B_40_745_0 ; _rtXdot
-> TransferFcn_CSTATE_p1 = 0.0 ; _rtXdot -> TransferFcn_CSTATE_p1 += _rtP ->
P_297 * _rtX -> TransferFcn_CSTATE_p1 ; _rtXdot -> TransferFcn_CSTATE_p1 +=
_rtB -> B_40_375_0 ; _rtXdot -> vv0at_CSTATE_oy = _rtB -> B_40_747_0 ;
_rtXdot -> TransferFcn_CSTATE_o4 = 0.0 ; _rtXdot -> TransferFcn_CSTATE_o4 +=
_rtP -> P_311 * _rtX -> TransferFcn_CSTATE_o4 ; _rtXdot ->
TransferFcn_CSTATE_o4 += _rtB -> B_40_392_0 ; _rtXdot -> vv0at_CSTATE_jb =
_rtB -> B_40_749_0 ; _rtXdot -> TransferFcn_CSTATE_g = 0.0 ; _rtXdot ->
TransferFcn_CSTATE_g += _rtP -> P_325 * _rtX -> TransferFcn_CSTATE_g ;
_rtXdot -> TransferFcn_CSTATE_g += _rtB -> B_40_409_0 ; _rtXdot ->
vv0at_CSTATE_hb = _rtB -> B_40_751_0 ; _rtXdot -> TransferFcn_CSTATE_j = 0.0
; _rtXdot -> TransferFcn_CSTATE_j += _rtP -> P_339 * _rtX ->
TransferFcn_CSTATE_j ; _rtXdot -> TransferFcn_CSTATE_j += _rtB -> B_40_426_0
; _rtXdot -> vv0at_CSTATE_f = _rtB -> B_40_753_0 ; _rtXdot ->
TransferFcn_CSTATE_h = 0.0 ; _rtXdot -> TransferFcn_CSTATE_h += _rtP -> P_353
* _rtX -> TransferFcn_CSTATE_h ; _rtXdot -> TransferFcn_CSTATE_h += _rtB ->
B_40_443_0 ; _rtXdot -> vv0at_CSTATE_lye = _rtB -> B_40_755_0 ; _rtXdot ->
TransferFcn_CSTATE_jd = 0.0 ; _rtXdot -> TransferFcn_CSTATE_jd += _rtP ->
P_367 * _rtX -> TransferFcn_CSTATE_jd ; _rtXdot -> TransferFcn_CSTATE_jd +=
_rtB -> B_40_460_0 ; _rtXdot -> vv0at_CSTATE_ju = _rtB -> B_40_757_0 ;
_rtXdot -> TransferFcn_CSTATE_jk = 0.0 ; _rtXdot -> TransferFcn_CSTATE_jk +=
_rtP -> P_381 * _rtX -> TransferFcn_CSTATE_jk ; _rtXdot ->
TransferFcn_CSTATE_jk += _rtB -> B_40_477_0 ; _rtXdot -> vv0at_CSTATE_pt =
_rtB -> B_40_759_0 ; _rtXdot -> TransferFcn_CSTATE_og = 0.0 ; _rtXdot ->
TransferFcn_CSTATE_og += _rtP -> P_395 * _rtX -> TransferFcn_CSTATE_og ;
_rtXdot -> TransferFcn_CSTATE_og += _rtB -> B_40_494_0 ; _rtXdot ->
vv0at_CSTATE_lw = _rtB -> B_40_761_0 ; _rtXdot -> TransferFcn_CSTATE_na = 0.0
; _rtXdot -> TransferFcn_CSTATE_na += _rtP -> P_409 * _rtX ->
TransferFcn_CSTATE_na ; _rtXdot -> TransferFcn_CSTATE_na += _rtB ->
B_40_511_0 ; _rtXdot -> vv0at_CSTATE_d = _rtB -> B_40_763_0 ; _rtXdot ->
TransferFcn_CSTATE_o1 = 0.0 ; _rtXdot -> TransferFcn_CSTATE_o1 += _rtP ->
P_423 * _rtX -> TransferFcn_CSTATE_o1 ; _rtXdot -> TransferFcn_CSTATE_o1 +=
_rtB -> B_40_528_0 ; _rtXdot -> vv0at_CSTATE_lt = _rtB -> B_40_765_0 ;
_rtXdot -> TransferFcn_CSTATE_fy = 0.0 ; _rtXdot -> TransferFcn_CSTATE_fy +=
_rtP -> P_437 * _rtX -> TransferFcn_CSTATE_fy ; _rtXdot ->
TransferFcn_CSTATE_fy += _rtB -> B_40_545_0 ; _rtXdot -> vv0at_CSTATE_h2 =
_rtB -> B_40_767_0 ; _rtXdot -> TransferFcn_CSTATE_pk = 0.0 ; _rtXdot ->
TransferFcn_CSTATE_pk += _rtP -> P_451 * _rtX -> TransferFcn_CSTATE_pk ;
_rtXdot -> TransferFcn_CSTATE_pk += _rtB -> B_40_562_0 ; _rtXdot ->
vv0at_CSTATE_hi = _rtB -> B_40_769_0 ; _rtXdot -> TransferFcn_CSTATE_hj = 0.0
; _rtXdot -> TransferFcn_CSTATE_hj += _rtP -> P_465 * _rtX ->
TransferFcn_CSTATE_hj ; _rtXdot -> TransferFcn_CSTATE_hj += _rtB ->
B_40_579_0 ; _rtXdot -> vv0at_CSTATE_b5 = _rtB -> B_40_771_0 ; _rtXdot ->
TransferFcn_CSTATE_of = 0.0 ; _rtXdot -> TransferFcn_CSTATE_of += _rtP ->
P_479 * _rtX -> TransferFcn_CSTATE_of ; _rtXdot -> TransferFcn_CSTATE_of +=
_rtB -> B_40_596_0 ; _rtXdot -> vv0at_CSTATE_b3 = _rtB -> B_40_773_0 ;
_rtXdot -> TransferFcn_CSTATE_g4 = 0.0 ; _rtXdot -> TransferFcn_CSTATE_g4 +=
_rtP -> P_493 * _rtX -> TransferFcn_CSTATE_g4 ; _rtXdot ->
TransferFcn_CSTATE_g4 += _rtB -> B_40_613_0 ; _rtXdot -> vv0at_CSTATE_bz =
_rtB -> B_40_775_0 ; _rtXdot -> TransferFcn_CSTATE_a = 0.0 ; _rtXdot ->
TransferFcn_CSTATE_a += _rtP -> P_507 * _rtX -> TransferFcn_CSTATE_a ;
_rtXdot -> TransferFcn_CSTATE_a += _rtB -> B_40_630_0 ; _rtXdot ->
vv0at_CSTATE_cc = _rtB -> B_40_777_0 ; _rtXdot -> TransferFcn_CSTATE_lp = 0.0
; _rtXdot -> TransferFcn_CSTATE_lp += _rtP -> P_521 * _rtX ->
TransferFcn_CSTATE_lp ; _rtXdot -> TransferFcn_CSTATE_lp += _rtB ->
B_40_647_0 ; _rtXdot -> vv0at_CSTATE_hl = _rtB -> B_40_779_0 ; _rtXdot ->
TransferFcn_CSTATE_lt = 0.0 ; _rtXdot -> TransferFcn_CSTATE_lt += _rtP ->
P_535 * _rtX -> TransferFcn_CSTATE_lt ; _rtXdot -> TransferFcn_CSTATE_lt +=
_rtB -> B_40_664_0 ; _rtXdot -> vv0at_CSTATE_m = _rtB -> B_40_781_0 ; _rtXdot
-> TransferFcn_CSTATE_nw = 0.0 ; _rtXdot -> TransferFcn_CSTATE_nw += _rtP ->
P_549 * _rtX -> TransferFcn_CSTATE_nw ; _rtXdot -> TransferFcn_CSTATE_nw +=
_rtB -> B_40_681_0 ; _rtXdot -> vv0at_CSTATE_ne = _rtB -> B_40_783_0 ;
_rtXdot -> TransferFcn_CSTATE_a3 = 0.0 ; _rtXdot -> TransferFcn_CSTATE_a3 +=
_rtP -> P_563 * _rtX -> TransferFcn_CSTATE_a3 ; _rtXdot ->
TransferFcn_CSTATE_a3 += _rtB -> B_40_700_0 ; _rtXdot -> Integrator1_CSTATE =
_rtB -> B_40_11_0 ; } static void mdlInitializeSizes ( SimStruct * S ) {
ssSetChecksumVal ( S , 0 , 1291466530U ) ; ssSetChecksumVal ( S , 1 ,
3667458797U ) ; ssSetChecksumVal ( S , 2 , 4186582689U ) ; ssSetChecksumVal (
S , 3 , 3807152458U ) ; { mxArray * slVerStructMat = NULL ; mxArray *
slStrMat = mxCreateString ( "simulink" ) ; char slVerChar [ 10 ] ; int status
= mexCallMATLAB ( 1 , & slVerStructMat , 1 , & slStrMat , "ver" ) ; if (
status == 0 ) { mxArray * slVerMat = mxGetField ( slVerStructMat , 0 ,
"Version" ) ; if ( slVerMat == NULL ) { status = 1 ; } else { status =
mxGetString ( slVerMat , slVerChar , 10 ) ; } } mxDestroyArray ( slStrMat ) ;
mxDestroyArray ( slVerStructMat ) ; if ( ( status == 1 ) || ( strcmp (
slVerChar , "10.0" ) != 0 ) ) { return ; } } ssSetOptions ( S ,
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
