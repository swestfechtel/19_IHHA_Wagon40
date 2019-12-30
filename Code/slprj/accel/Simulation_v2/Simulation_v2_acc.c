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
_rtP -> P_5 ; _rtB -> B_40_20_0 = _rtP -> P_6 * _rtB -> B_40_12_0 ; _rtB ->
B_40_21_0 = _rtP -> P_7 * _rtB -> B_40_19_0 ; if ( _rtB -> B_40_21_0 > _rtP
-> P_8 ) { _rtB -> B_40_22_0 = _rtP -> P_8 ; } else if ( _rtB -> B_40_21_0 <
_rtP -> P_9 ) { _rtB -> B_40_22_0 = _rtP -> P_9 ; } else { _rtB -> B_40_22_0
= _rtB -> B_40_21_0 ; } if ( ( _rtDW -> LastMajorTimeA >= ssGetTaskTime ( S ,
0 ) ) && ( _rtDW -> LastMajorTimeB >= ssGetTaskTime ( S , 0 ) ) ) { _rtB ->
B_40_23_0 = _rtB -> B_40_22_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA <
_rtDW -> LastMajorTimeB ) && ( _rtDW -> LastMajorTimeB < ssGetTaskTime ( S ,
0 ) ) ) || ( ( _rtDW -> LastMajorTimeA >= _rtDW -> LastMajorTimeB ) && (
_rtDW -> LastMajorTimeA >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB ; rtb_B_40_3_0 = _rtDW ->
PrevYB ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA
; rtb_B_40_3_0 = _rtDW -> PrevYA ; } riseValLimit = deltaT * _rtP -> P_10 ;
rateLimiterRate = _rtB -> B_40_22_0 - rtb_B_40_3_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_40_23_0 = rtb_B_40_3_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_11 ; if ( rateLimiterRate < deltaT ) { _rtB -> B_40_23_0
= rtb_B_40_3_0 + deltaT ; } else { _rtB -> B_40_23_0 = _rtB -> B_40_22_0 ; }
} } _rtB -> B_40_24_0 = _rtP -> P_12 * _rtB -> B_40_23_0 ; _rtB -> B_40_25_0
= _rtP -> P_13 * _rtB -> B_40_24_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW ->
dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem/Wagon model/Data Store Read" , 0
) ; _rtB -> B_40_26_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK != 0 ) { _rtX
-> vv0at_CSTATE = _rtB -> B_40_26_0 ; } _rtB -> B_40_27_0 = _rtX ->
vv0at_CSTATE ; _rtB -> B_40_28_0 = muDoubleScalarAbs ( _rtB -> B_40_27_0 ) ;
_rtB -> B_40_29_0 = _rtP -> P_14 [ 0 ] ; _rtB -> B_40_29_0 = _rtB ->
B_40_29_0 * _rtB -> B_40_28_0 + _rtP -> P_14 [ 1 ] ; _rtB -> B_40_29_0 = _rtB
-> B_40_29_0 * _rtB -> B_40_28_0 + _rtP -> P_14 [ 2 ] ; _rtB -> B_40_30_0 =
_rtP -> P_15 * _rtB -> B_40_29_0 ; _rtB -> B_40_31_0 = _rtB -> B_40_1_0_c *
_rtB -> B_40_30_0 ; _rtB -> B_40_32_0 = _rtB -> B_40_25_0 - _rtB -> B_40_31_0
; if ( _rtB -> B_40_32_0 > _rtB -> B_40_51_0 ) { _rtB -> B_40_35_0 = _rtB ->
B_40_51_0 ; } else { if ( _rtB -> B_40_32_0 < _rtB -> B_40_52_0 ) { _rtB ->
B_0_1_0 = _rtB -> B_40_52_0 ; } else { _rtB -> B_0_1_0 = _rtB -> B_40_32_0 ;
} _rtB -> B_40_35_0 = _rtB -> B_0_1_0 ; } { real_T * * uBuffer = ( real_T * *
) & _rtDW -> TransportDelay_PWORK . TUbufferPtrs [ 0 ] ; real_T * * tBuffer =
( real_T * * ) & _rtDW -> TransportDelay_PWORK . TUbufferPtrs [ 1 ] ; real_T
simTime = ssGetT ( S ) ; real_T tMinusDelay = simTime - _rtP -> P_16 ; _rtB
-> B_40_36_0 = Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , *
tBuffer , * uBuffer , _rtDW -> TransportDelay_IWORK . CircularBufSize , &
_rtDW -> TransportDelay_IWORK . Last , _rtDW -> TransportDelay_IWORK . Tail ,
_rtDW -> TransportDelay_IWORK . Head , _rtP -> P_17 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_40_37_0 = 0.0 ; _rtB -> B_40_37_0 += _rtP -> P_19 * _rtX ->
TransferFcn_CSTATE ; _rtB -> B_40_37_0 += _rtP -> P_20 * _rtB -> B_40_36_0 ;
if ( _rtB -> B_40_37_0 > _rtP -> P_22 ) { _rtB -> B_40_38_0 = _rtB ->
B_40_37_0 - _rtP -> P_22 ; } else if ( _rtB -> B_40_37_0 >= _rtP -> P_21 ) {
_rtB -> B_40_38_0 = 0.0 ; } else { _rtB -> B_40_38_0 = _rtB -> B_40_37_0 -
_rtP -> P_21 ; } _rtB -> B_40_39_0 = _rtP -> P_23 * _rtB -> B_40_38_0 ; if (
_rtB -> B_40_39_0 > _rtP -> P_24 ) { _rtB -> B_40_40_0 = _rtP -> P_24 ; }
else if ( _rtB -> B_40_39_0 < _rtP -> P_25 ) { _rtB -> B_40_40_0 = _rtP ->
P_25 ; } else { _rtB -> B_40_40_0 = _rtB -> B_40_39_0 ; } if ( ( _rtDW ->
LastMajorTimeA_d >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW -> LastMajorTimeB_m
>= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_40_41_0 = _rtB -> B_40_40_0 ; }
else { if ( ( ( _rtDW -> LastMajorTimeA_d < _rtDW -> LastMajorTimeB_m ) && (
_rtDW -> LastMajorTimeB_m < ssGetTaskTime ( S , 0 ) ) ) || ( ( _rtDW ->
LastMajorTimeA_d >= _rtDW -> LastMajorTimeB_m ) && ( _rtDW ->
LastMajorTimeA_d >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime (
S , 0 ) - _rtDW -> LastMajorTimeB_m ; rtb_B_40_3_0 = _rtDW -> PrevYB_b ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_d ;
rtb_B_40_3_0 = _rtDW -> PrevYA_p ; } riseValLimit = deltaT * _rtP -> P_26 ;
rateLimiterRate = _rtB -> B_40_40_0 - rtb_B_40_3_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_40_41_0 = rtb_B_40_3_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_27 ; if ( rateLimiterRate < deltaT ) { _rtB -> B_40_41_0
= rtb_B_40_3_0 + deltaT ; } else { _rtB -> B_40_41_0 = _rtB -> B_40_40_0 ; }
} } _rtB -> B_40_42_0 = _rtP -> P_28 * _rtB -> B_40_41_0 ; _rtB -> B_40_43_0
= _rtP -> P_29 * _rtB -> B_40_42_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW ->
dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem/Wagon model1/Data Store Read" , 0
) ; _rtB -> B_40_44_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_c != 0 ) {
_rtX -> vv0at_CSTATE_f = _rtB -> B_40_44_0 ; } _rtB -> B_40_45_0 = _rtX ->
vv0at_CSTATE_f ; _rtB -> B_40_46_0 = muDoubleScalarAbs ( _rtB -> B_40_45_0 )
; _rtB -> B_40_47_0 = _rtP -> P_30 [ 0 ] ; _rtB -> B_40_47_0 = _rtB ->
B_40_47_0 * _rtB -> B_40_46_0 + _rtP -> P_30 [ 1 ] ; _rtB -> B_40_47_0 = _rtB
-> B_40_47_0 * _rtB -> B_40_46_0 + _rtP -> P_30 [ 2 ] ; _rtB -> B_40_48_0 =
_rtP -> P_31 * _rtB -> B_40_47_0 ; _rtB -> B_40_49_0 = _rtB -> B_40_2_0 *
_rtB -> B_40_48_0 ; _rtB -> B_40_50_0 = _rtB -> B_40_43_0 - _rtB -> B_40_49_0
; if ( _rtB -> B_40_50_0 > _rtB -> B_40_54_0_j ) { _rtB -> B_40_53_0 = _rtB
-> B_40_54_0_j ; } else { if ( _rtB -> B_40_50_0 < _rtB -> B_40_55_0_f ) {
_rtB -> B_1_1_0 = _rtB -> B_40_55_0_f ; } else { _rtB -> B_1_1_0 = _rtB ->
B_40_50_0 ; } _rtB -> B_40_53_0 = _rtB -> B_1_1_0 ; } { real_T * * uBuffer =
( real_T * * ) & _rtDW -> TransportDelay_PWORK_b . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_b .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_32 ; _rtB -> B_40_54_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_j . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_j . Last , _rtDW -> TransportDelay_IWORK_j . Tail ,
_rtDW -> TransportDelay_IWORK_j . Head , _rtP -> P_33 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_40_55_0 = 0.0 ; _rtB -> B_40_55_0 += _rtP -> P_35 * _rtX ->
TransferFcn_CSTATE_m ; _rtB -> B_40_55_0 += _rtP -> P_36 * _rtB -> B_40_54_0
; if ( _rtB -> B_40_55_0 > _rtP -> P_38 ) { _rtB -> B_40_56_0 = _rtB ->
B_40_55_0 - _rtP -> P_38 ; } else if ( _rtB -> B_40_55_0 >= _rtP -> P_37 ) {
_rtB -> B_40_56_0 = 0.0 ; } else { _rtB -> B_40_56_0 = _rtB -> B_40_55_0 -
_rtP -> P_37 ; } _rtB -> B_40_57_0 = _rtP -> P_39 * _rtB -> B_40_56_0 ; if (
_rtB -> B_40_57_0 > _rtP -> P_40 ) { _rtB -> B_40_58_0 = _rtP -> P_40 ; }
else if ( _rtB -> B_40_57_0 < _rtP -> P_41 ) { _rtB -> B_40_58_0 = _rtP ->
P_41 ; } else { _rtB -> B_40_58_0 = _rtB -> B_40_57_0 ; } if ( ( _rtDW ->
LastMajorTimeA_l >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW -> LastMajorTimeB_c
>= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_40_59_0 = _rtB -> B_40_58_0 ; }
else { if ( ( ( _rtDW -> LastMajorTimeA_l < _rtDW -> LastMajorTimeB_c ) && (
_rtDW -> LastMajorTimeB_c < ssGetTaskTime ( S , 0 ) ) ) || ( ( _rtDW ->
LastMajorTimeA_l >= _rtDW -> LastMajorTimeB_c ) && ( _rtDW ->
LastMajorTimeA_l >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime (
S , 0 ) - _rtDW -> LastMajorTimeB_c ; rtb_B_40_3_0 = _rtDW -> PrevYB_c ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_l ;
rtb_B_40_3_0 = _rtDW -> PrevYA_k ; } riseValLimit = deltaT * _rtP -> P_42 ;
rateLimiterRate = _rtB -> B_40_58_0 - rtb_B_40_3_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_40_59_0 = rtb_B_40_3_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_43 ; if ( rateLimiterRate < deltaT ) { _rtB -> B_40_59_0
= rtb_B_40_3_0 + deltaT ; } else { _rtB -> B_40_59_0 = _rtB -> B_40_58_0 ; }
} } _rtB -> B_40_60_0 = _rtP -> P_44 * _rtB -> B_40_59_0 ; _rtB -> B_40_61_0
= _rtP -> P_45 * _rtB -> B_40_60_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW ->
dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem/Wagon model2/Data Store Read" , 0
) ; _rtB -> B_40_62_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_a != 0 ) {
_rtX -> vv0at_CSTATE_b = _rtB -> B_40_62_0 ; } _rtB -> B_40_63_0 = _rtX ->
vv0at_CSTATE_b ; _rtB -> B_40_64_0 = muDoubleScalarAbs ( _rtB -> B_40_63_0 )
; _rtB -> B_40_65_0 = _rtP -> P_46 [ 0 ] ; _rtB -> B_40_65_0 = _rtB ->
B_40_65_0 * _rtB -> B_40_64_0 + _rtP -> P_46 [ 1 ] ; _rtB -> B_40_65_0 = _rtB
-> B_40_65_0 * _rtB -> B_40_64_0 + _rtP -> P_46 [ 2 ] ; _rtB -> B_40_66_0 =
_rtP -> P_47 * _rtB -> B_40_65_0 ; _rtB -> B_40_67_0 = _rtB -> B_40_3_0 *
_rtB -> B_40_66_0 ; _rtB -> B_40_68_0 = _rtB -> B_40_61_0 - _rtB -> B_40_67_0
; if ( _rtB -> B_40_68_0 > _rtB -> B_40_57_0_a ) { _rtB -> B_40_71_0 = _rtB
-> B_40_57_0_a ; } else { if ( _rtB -> B_40_68_0 < _rtB -> B_40_58_0_j ) {
_rtB -> B_2_1_0 = _rtB -> B_40_58_0_j ; } else { _rtB -> B_2_1_0 = _rtB ->
B_40_68_0 ; } _rtB -> B_40_71_0 = _rtB -> B_2_1_0 ; } { real_T * * uBuffer =
( real_T * * ) & _rtDW -> TransportDelay_PWORK_m . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_m .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_48 ; _rtB -> B_40_72_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_b . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_b . Last , _rtDW -> TransportDelay_IWORK_b . Tail ,
_rtDW -> TransportDelay_IWORK_b . Head , _rtP -> P_49 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_40_73_0 = 0.0 ; _rtB -> B_40_73_0 += _rtP -> P_51 * _rtX ->
TransferFcn_CSTATE_d ; _rtB -> B_40_73_0 += _rtP -> P_52 * _rtB -> B_40_72_0
; if ( _rtB -> B_40_73_0 > _rtP -> P_54 ) { _rtB -> B_40_74_0 = _rtB ->
B_40_73_0 - _rtP -> P_54 ; } else if ( _rtB -> B_40_73_0 >= _rtP -> P_53 ) {
_rtB -> B_40_74_0 = 0.0 ; } else { _rtB -> B_40_74_0 = _rtB -> B_40_73_0 -
_rtP -> P_53 ; } _rtB -> B_40_75_0 = _rtP -> P_55 * _rtB -> B_40_74_0 ; if (
_rtB -> B_40_75_0 > _rtP -> P_56 ) { _rtB -> B_40_76_0 = _rtP -> P_56 ; }
else if ( _rtB -> B_40_75_0 < _rtP -> P_57 ) { _rtB -> B_40_76_0 = _rtP ->
P_57 ; } else { _rtB -> B_40_76_0 = _rtB -> B_40_75_0 ; } if ( ( _rtDW ->
LastMajorTimeA_g >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW -> LastMajorTimeB_e
>= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_40_77_0 = _rtB -> B_40_76_0 ; }
else { if ( ( ( _rtDW -> LastMajorTimeA_g < _rtDW -> LastMajorTimeB_e ) && (
_rtDW -> LastMajorTimeB_e < ssGetTaskTime ( S , 0 ) ) ) || ( ( _rtDW ->
LastMajorTimeA_g >= _rtDW -> LastMajorTimeB_e ) && ( _rtDW ->
LastMajorTimeA_g >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime (
S , 0 ) - _rtDW -> LastMajorTimeB_e ; rtb_B_40_3_0 = _rtDW -> PrevYB_e ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_g ;
rtb_B_40_3_0 = _rtDW -> PrevYA_o ; } riseValLimit = deltaT * _rtP -> P_58 ;
rateLimiterRate = _rtB -> B_40_76_0 - rtb_B_40_3_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_40_77_0 = rtb_B_40_3_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_59 ; if ( rateLimiterRate < deltaT ) { _rtB -> B_40_77_0
= rtb_B_40_3_0 + deltaT ; } else { _rtB -> B_40_77_0 = _rtB -> B_40_76_0 ; }
} } _rtB -> B_40_78_0 = _rtP -> P_60 * _rtB -> B_40_77_0 ; _rtB -> B_40_79_0
= _rtP -> P_61 * _rtB -> B_40_78_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW ->
dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem/Wagon model3/Data Store Read" , 0
) ; _rtB -> B_40_80_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_j != 0 ) {
_rtX -> vv0at_CSTATE_m = _rtB -> B_40_80_0 ; } _rtB -> B_40_81_0 = _rtX ->
vv0at_CSTATE_m ; _rtB -> B_40_82_0 = muDoubleScalarAbs ( _rtB -> B_40_81_0 )
; _rtB -> B_40_83_0 = _rtP -> P_62 [ 0 ] ; _rtB -> B_40_83_0 = _rtB ->
B_40_83_0 * _rtB -> B_40_82_0 + _rtP -> P_62 [ 1 ] ; _rtB -> B_40_83_0 = _rtB
-> B_40_83_0 * _rtB -> B_40_82_0 + _rtP -> P_62 [ 2 ] ; _rtB -> B_40_84_0 =
_rtP -> P_63 * _rtB -> B_40_83_0 ; _rtB -> B_40_85_0 = _rtB -> B_40_4_0 *
_rtB -> B_40_84_0 ; _rtB -> B_40_86_0 = _rtB -> B_40_79_0 - _rtB -> B_40_85_0
; if ( _rtB -> B_40_86_0 > _rtB -> B_40_60_0_j ) { _rtB -> B_40_89_0 = _rtB
-> B_40_60_0_j ; } else { if ( _rtB -> B_40_86_0 < _rtB -> B_40_61_0_o ) {
_rtB -> B_3_1_0 = _rtB -> B_40_61_0_o ; } else { _rtB -> B_3_1_0 = _rtB ->
B_40_86_0 ; } _rtB -> B_40_89_0 = _rtB -> B_3_1_0 ; } { real_T * * uBuffer =
( real_T * * ) & _rtDW -> TransportDelay_PWORK_j . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_j .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_64 ; _rtB -> B_40_90_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_f . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_f . Last , _rtDW -> TransportDelay_IWORK_f . Tail ,
_rtDW -> TransportDelay_IWORK_f . Head , _rtP -> P_65 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_40_91_0 = 0.0 ; _rtB -> B_40_91_0 += _rtP -> P_67 * _rtX ->
TransferFcn_CSTATE_k ; _rtB -> B_40_91_0 += _rtP -> P_68 * _rtB -> B_40_90_0
; if ( _rtB -> B_40_91_0 > _rtP -> P_70 ) { _rtB -> B_40_92_0 = _rtB ->
B_40_91_0 - _rtP -> P_70 ; } else if ( _rtB -> B_40_91_0 >= _rtP -> P_69 ) {
_rtB -> B_40_92_0 = 0.0 ; } else { _rtB -> B_40_92_0 = _rtB -> B_40_91_0 -
_rtP -> P_69 ; } _rtB -> B_40_93_0 = _rtP -> P_71 * _rtB -> B_40_92_0 ; if (
_rtB -> B_40_93_0 > _rtP -> P_72 ) { _rtB -> B_40_94_0 = _rtP -> P_72 ; }
else if ( _rtB -> B_40_93_0 < _rtP -> P_73 ) { _rtB -> B_40_94_0 = _rtP ->
P_73 ; } else { _rtB -> B_40_94_0 = _rtB -> B_40_93_0 ; } if ( ( _rtDW ->
LastMajorTimeA_p >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW -> LastMajorTimeB_o
>= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_40_95_0 = _rtB -> B_40_94_0 ; }
else { if ( ( ( _rtDW -> LastMajorTimeA_p < _rtDW -> LastMajorTimeB_o ) && (
_rtDW -> LastMajorTimeB_o < ssGetTaskTime ( S , 0 ) ) ) || ( ( _rtDW ->
LastMajorTimeA_p >= _rtDW -> LastMajorTimeB_o ) && ( _rtDW ->
LastMajorTimeA_p >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime (
S , 0 ) - _rtDW -> LastMajorTimeB_o ; rtb_B_40_3_0 = _rtDW -> PrevYB_l ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_p ;
rtb_B_40_3_0 = _rtDW -> PrevYA_kx ; } riseValLimit = deltaT * _rtP -> P_74 ;
rateLimiterRate = _rtB -> B_40_94_0 - rtb_B_40_3_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_40_95_0 = rtb_B_40_3_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_75 ; if ( rateLimiterRate < deltaT ) { _rtB -> B_40_95_0
= rtb_B_40_3_0 + deltaT ; } else { _rtB -> B_40_95_0 = _rtB -> B_40_94_0 ; }
} } _rtB -> B_40_96_0 = _rtP -> P_76 * _rtB -> B_40_95_0 ; _rtB -> B_40_97_0
= _rtP -> P_77 * _rtB -> B_40_96_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW ->
dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem/Wagon model4/Data Store Read" , 0
) ; _rtB -> B_40_98_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_e != 0 ) {
_rtX -> vv0at_CSTATE_j = _rtB -> B_40_98_0 ; } _rtB -> B_40_99_0 = _rtX ->
vv0at_CSTATE_j ; _rtB -> B_40_100_0 = muDoubleScalarAbs ( _rtB -> B_40_99_0 )
; _rtB -> B_40_101_0 = _rtP -> P_78 [ 0 ] ; _rtB -> B_40_101_0 = _rtB ->
B_40_101_0 * _rtB -> B_40_100_0 + _rtP -> P_78 [ 1 ] ; _rtB -> B_40_101_0 =
_rtB -> B_40_101_0 * _rtB -> B_40_100_0 + _rtP -> P_78 [ 2 ] ; _rtB ->
B_40_102_0 = _rtP -> P_79 * _rtB -> B_40_101_0 ; _rtB -> B_40_103_0 = _rtB ->
B_40_5_0 * _rtB -> B_40_102_0 ; _rtB -> B_40_104_0 = _rtB -> B_40_97_0 - _rtB
-> B_40_103_0 ; if ( _rtB -> B_40_104_0 > _rtB -> B_40_63_0_n ) { _rtB ->
B_40_107_0 = _rtB -> B_40_63_0_n ; } else { if ( _rtB -> B_40_104_0 < _rtB ->
B_40_64_0_i ) { _rtB -> B_4_1_0 = _rtB -> B_40_64_0_i ; } else { _rtB ->
B_4_1_0 = _rtB -> B_40_104_0 ; } _rtB -> B_40_107_0 = _rtB -> B_4_1_0 ; } {
real_T * * uBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_ba .
TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_ba . TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S )
; real_T tMinusDelay = simTime - _rtP -> P_80 ; _rtB -> B_40_108_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_p . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_p . Last , _rtDW -> TransportDelay_IWORK_p . Tail ,
_rtDW -> TransportDelay_IWORK_p . Head , _rtP -> P_81 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_40_109_0 = 0.0 ; _rtB -> B_40_109_0 += _rtP -> P_83 * _rtX ->
TransferFcn_CSTATE_j ; _rtB -> B_40_109_0 += _rtP -> P_84 * _rtB ->
B_40_108_0 ; if ( _rtB -> B_40_109_0 > _rtP -> P_86 ) { _rtB -> B_40_110_0 =
_rtB -> B_40_109_0 - _rtP -> P_86 ; } else if ( _rtB -> B_40_109_0 >= _rtP ->
P_85 ) { _rtB -> B_40_110_0 = 0.0 ; } else { _rtB -> B_40_110_0 = _rtB ->
B_40_109_0 - _rtP -> P_85 ; } _rtB -> B_40_111_0 = _rtP -> P_87 * _rtB ->
B_40_110_0 ; if ( _rtB -> B_40_111_0 > _rtP -> P_88 ) { _rtB -> B_40_112_0 =
_rtP -> P_88 ; } else if ( _rtB -> B_40_111_0 < _rtP -> P_89 ) { _rtB ->
B_40_112_0 = _rtP -> P_89 ; } else { _rtB -> B_40_112_0 = _rtB -> B_40_111_0
; } if ( ( _rtDW -> LastMajorTimeA_i >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW
-> LastMajorTimeB_b >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_40_113_0 =
_rtB -> B_40_112_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_i < _rtDW ->
LastMajorTimeB_b ) && ( _rtDW -> LastMajorTimeB_b < ssGetTaskTime ( S , 0 ) )
) || ( ( _rtDW -> LastMajorTimeA_i >= _rtDW -> LastMajorTimeB_b ) && ( _rtDW
-> LastMajorTimeA_i >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime
( S , 0 ) - _rtDW -> LastMajorTimeB_b ; rtb_B_40_3_0 = _rtDW -> PrevYB_i ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_i ;
rtb_B_40_3_0 = _rtDW -> PrevYA_c ; } riseValLimit = deltaT * _rtP -> P_90 ;
rateLimiterRate = _rtB -> B_40_112_0 - rtb_B_40_3_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_40_113_0 = rtb_B_40_3_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_91 ; if ( rateLimiterRate < deltaT ) { _rtB -> B_40_113_0
= rtb_B_40_3_0 + deltaT ; } else { _rtB -> B_40_113_0 = _rtB -> B_40_112_0 ;
} } } _rtB -> B_40_114_0 = _rtP -> P_92 * _rtB -> B_40_113_0 ; _rtB ->
B_40_115_0 = _rtP -> P_93 * _rtB -> B_40_114_0 ; vm_ReadLocalDSMNoIdx ( S ,
_rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model/Data Store Read" , 0
) ; _rtB -> B_40_116_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_cm != 0 ) {
_rtX -> vv0at_CSTATE_fz = _rtB -> B_40_116_0 ; } _rtB -> B_40_117_0 = _rtX ->
vv0at_CSTATE_fz ; _rtB -> B_40_118_0 = muDoubleScalarAbs ( _rtB -> B_40_117_0
) ; _rtB -> B_40_119_0 = _rtP -> P_94 [ 0 ] ; _rtB -> B_40_119_0 = _rtB ->
B_40_119_0 * _rtB -> B_40_118_0 + _rtP -> P_94 [ 1 ] ; _rtB -> B_40_119_0 =
_rtB -> B_40_119_0 * _rtB -> B_40_118_0 + _rtP -> P_94 [ 2 ] ; _rtB ->
B_40_120_0 = _rtP -> P_95 * _rtB -> B_40_119_0 ; _rtB -> B_40_121_0 = _rtB ->
B_40_7_0_k * _rtB -> B_40_120_0 ; _rtB -> B_40_122_0 = _rtB -> B_40_115_0 -
_rtB -> B_40_121_0 ; if ( _rtB -> B_40_122_0 > _rtB -> B_40_66_0_o ) { _rtB
-> B_40_125_0 = _rtB -> B_40_66_0_o ; } else { if ( _rtB -> B_40_122_0 < _rtB
-> B_40_67_0_n ) { _rtB -> B_5_1_0 = _rtB -> B_40_67_0_n ; } else { _rtB ->
B_5_1_0 = _rtB -> B_40_122_0 ; } _rtB -> B_40_125_0 = _rtB -> B_5_1_0 ; } {
real_T * * uBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_n .
TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_n . TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ;
real_T tMinusDelay = simTime - _rtP -> P_96 ; _rtB -> B_40_126_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_l . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_l . Last , _rtDW -> TransportDelay_IWORK_l . Tail ,
_rtDW -> TransportDelay_IWORK_l . Head , _rtP -> P_97 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_40_127_0 = 0.0 ; _rtB -> B_40_127_0 += _rtP -> P_99 * _rtX ->
TransferFcn_CSTATE_p ; _rtB -> B_40_127_0 += _rtP -> P_100 * _rtB ->
B_40_126_0 ; if ( _rtB -> B_40_127_0 > _rtP -> P_102 ) { _rtB -> B_40_128_0 =
_rtB -> B_40_127_0 - _rtP -> P_102 ; } else if ( _rtB -> B_40_127_0 >= _rtP
-> P_101 ) { _rtB -> B_40_128_0 = 0.0 ; } else { _rtB -> B_40_128_0 = _rtB ->
B_40_127_0 - _rtP -> P_101 ; } _rtB -> B_40_129_0 = _rtP -> P_103 * _rtB ->
B_40_128_0 ; if ( _rtB -> B_40_129_0 > _rtP -> P_104 ) { _rtB -> B_40_130_0 =
_rtP -> P_104 ; } else if ( _rtB -> B_40_129_0 < _rtP -> P_105 ) { _rtB ->
B_40_130_0 = _rtP -> P_105 ; } else { _rtB -> B_40_130_0 = _rtB -> B_40_129_0
; } if ( ( _rtDW -> LastMajorTimeA_e >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW
-> LastMajorTimeB_g >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_40_131_0 =
_rtB -> B_40_130_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_e < _rtDW ->
LastMajorTimeB_g ) && ( _rtDW -> LastMajorTimeB_g < ssGetTaskTime ( S , 0 ) )
) || ( ( _rtDW -> LastMajorTimeA_e >= _rtDW -> LastMajorTimeB_g ) && ( _rtDW
-> LastMajorTimeA_e >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime
( S , 0 ) - _rtDW -> LastMajorTimeB_g ; rtb_B_40_3_0 = _rtDW -> PrevYB_bm ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_e ;
rtb_B_40_3_0 = _rtDW -> PrevYA_i ; } riseValLimit = deltaT * _rtP -> P_106 ;
rateLimiterRate = _rtB -> B_40_130_0 - rtb_B_40_3_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_40_131_0 = rtb_B_40_3_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_107 ; if ( rateLimiterRate < deltaT ) { _rtB ->
B_40_131_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB -> B_40_131_0 = _rtB ->
B_40_130_0 ; } } } _rtB -> B_40_132_0 = _rtP -> P_108 * _rtB -> B_40_131_0 ;
_rtB -> B_40_133_0 = _rtP -> P_109 * _rtB -> B_40_132_0 ;
vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model1/Data Store Read" ,
0 ) ; _rtB -> B_40_134_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_f != 0 ) {
_rtX -> vv0at_CSTATE_n = _rtB -> B_40_134_0 ; } _rtB -> B_40_135_0 = _rtX ->
vv0at_CSTATE_n ; _rtB -> B_40_136_0 = muDoubleScalarAbs ( _rtB -> B_40_135_0
) ; _rtB -> B_40_137_0 = _rtP -> P_110 [ 0 ] ; _rtB -> B_40_137_0 = _rtB ->
B_40_137_0 * _rtB -> B_40_136_0 + _rtP -> P_110 [ 1 ] ; _rtB -> B_40_137_0 =
_rtB -> B_40_137_0 * _rtB -> B_40_136_0 + _rtP -> P_110 [ 2 ] ; _rtB ->
B_40_138_0 = _rtP -> P_111 * _rtB -> B_40_137_0 ; _rtB -> B_40_139_0 = _rtB
-> B_40_8_0 * _rtB -> B_40_138_0 ; _rtB -> B_40_140_0 = _rtB -> B_40_133_0 -
_rtB -> B_40_139_0 ; if ( _rtB -> B_40_140_0 > _rtB -> B_40_69_0 ) { _rtB ->
B_40_143_0 = _rtB -> B_40_69_0 ; } else { if ( _rtB -> B_40_140_0 < _rtB ->
B_40_70_0 ) { _rtB -> B_6_1_0 = _rtB -> B_40_70_0 ; } else { _rtB -> B_6_1_0
= _rtB -> B_40_140_0 ; } _rtB -> B_40_143_0 = _rtB -> B_6_1_0 ; } { real_T *
* uBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_jr . TUbufferPtrs
[ 0 ] ; real_T * * tBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_jr . TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S )
; real_T tMinusDelay = simTime - _rtP -> P_112 ; _rtB -> B_40_144_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_o . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_o . Last , _rtDW -> TransportDelay_IWORK_o . Tail ,
_rtDW -> TransportDelay_IWORK_o . Head , _rtP -> P_113 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_40_145_0 = 0.0 ; _rtB -> B_40_145_0 += _rtP -> P_115 * _rtX
-> TransferFcn_CSTATE_mr ; _rtB -> B_40_145_0 += _rtP -> P_116 * _rtB ->
B_40_144_0 ; if ( _rtB -> B_40_145_0 > _rtP -> P_118 ) { _rtB -> B_40_146_0 =
_rtB -> B_40_145_0 - _rtP -> P_118 ; } else if ( _rtB -> B_40_145_0 >= _rtP
-> P_117 ) { _rtB -> B_40_146_0 = 0.0 ; } else { _rtB -> B_40_146_0 = _rtB ->
B_40_145_0 - _rtP -> P_117 ; } _rtB -> B_40_147_0 = _rtP -> P_119 * _rtB ->
B_40_146_0 ; if ( _rtB -> B_40_147_0 > _rtP -> P_120 ) { _rtB -> B_40_148_0 =
_rtP -> P_120 ; } else if ( _rtB -> B_40_147_0 < _rtP -> P_121 ) { _rtB ->
B_40_148_0 = _rtP -> P_121 ; } else { _rtB -> B_40_148_0 = _rtB -> B_40_147_0
; } if ( ( _rtDW -> LastMajorTimeA_g5 >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW
-> LastMajorTimeB_h >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_40_149_0 =
_rtB -> B_40_148_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_g5 < _rtDW ->
LastMajorTimeB_h ) && ( _rtDW -> LastMajorTimeB_h < ssGetTaskTime ( S , 0 ) )
) || ( ( _rtDW -> LastMajorTimeA_g5 >= _rtDW -> LastMajorTimeB_h ) && ( _rtDW
-> LastMajorTimeA_g5 >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_h ; rtb_B_40_3_0 = _rtDW ->
PrevYB_eb ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_g5 ; rtb_B_40_3_0 = _rtDW -> PrevYA_p0 ; } riseValLimit =
deltaT * _rtP -> P_122 ; rateLimiterRate = _rtB -> B_40_148_0 - rtb_B_40_3_0
; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_40_149_0 = rtb_B_40_3_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_123 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_40_149_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB ->
B_40_149_0 = _rtB -> B_40_148_0 ; } } } _rtB -> B_40_150_0 = _rtP -> P_124 *
_rtB -> B_40_149_0 ; _rtB -> B_40_151_0 = _rtP -> P_125 * _rtB -> B_40_150_0
; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model2/Data Store Read" ,
0 ) ; _rtB -> B_40_152_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_fh != 0 )
{ _rtX -> vv0at_CSTATE_g = _rtB -> B_40_152_0 ; } _rtB -> B_40_153_0 = _rtX
-> vv0at_CSTATE_g ; _rtB -> B_40_154_0 = muDoubleScalarAbs ( _rtB ->
B_40_153_0 ) ; _rtB -> B_40_155_0 = _rtP -> P_126 [ 0 ] ; _rtB -> B_40_155_0
= _rtB -> B_40_155_0 * _rtB -> B_40_154_0 + _rtP -> P_126 [ 1 ] ; _rtB ->
B_40_155_0 = _rtB -> B_40_155_0 * _rtB -> B_40_154_0 + _rtP -> P_126 [ 2 ] ;
_rtB -> B_40_156_0 = _rtP -> P_127 * _rtB -> B_40_155_0 ; _rtB -> B_40_157_0
= _rtB -> B_40_9_0 * _rtB -> B_40_156_0 ; _rtB -> B_40_158_0 = _rtB ->
B_40_151_0 - _rtB -> B_40_157_0 ; if ( _rtB -> B_40_158_0 > _rtB ->
B_40_72_0_m ) { _rtB -> B_40_161_0 = _rtB -> B_40_72_0_m ; } else { if ( _rtB
-> B_40_158_0 < _rtB -> B_40_73_0_c ) { _rtB -> B_7_1_0 = _rtB -> B_40_73_0_c
; } else { _rtB -> B_7_1_0 = _rtB -> B_40_158_0 ; } _rtB -> B_40_161_0 = _rtB
-> B_7_1_0 ; } { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_f . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T *
* ) & _rtDW -> TransportDelay_PWORK_f . TUbufferPtrs [ 1 ] ; real_T simTime =
ssGetT ( S ) ; real_T tMinusDelay = simTime - _rtP -> P_128 ; _rtB ->
B_40_162_0 = Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , *
tBuffer , * uBuffer , _rtDW -> TransportDelay_IWORK_d . CircularBufSize , &
_rtDW -> TransportDelay_IWORK_d . Last , _rtDW -> TransportDelay_IWORK_d .
Tail , _rtDW -> TransportDelay_IWORK_d . Head , _rtP -> P_129 , 0 , (
boolean_T ) ( ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) ==
ssGetT ( S ) ) ) ) ; } _rtB -> B_40_163_0 = 0.0 ; _rtB -> B_40_163_0 += _rtP
-> P_131 * _rtX -> TransferFcn_CSTATE_l ; _rtB -> B_40_163_0 += _rtP -> P_132
* _rtB -> B_40_162_0 ; if ( _rtB -> B_40_163_0 > _rtP -> P_134 ) { _rtB ->
B_40_164_0 = _rtB -> B_40_163_0 - _rtP -> P_134 ; } else if ( _rtB ->
B_40_163_0 >= _rtP -> P_133 ) { _rtB -> B_40_164_0 = 0.0 ; } else { _rtB ->
B_40_164_0 = _rtB -> B_40_163_0 - _rtP -> P_133 ; } _rtB -> B_40_165_0 = _rtP
-> P_135 * _rtB -> B_40_164_0 ; if ( _rtB -> B_40_165_0 > _rtP -> P_136 ) {
_rtB -> B_40_166_0 = _rtP -> P_136 ; } else if ( _rtB -> B_40_165_0 < _rtP ->
P_137 ) { _rtB -> B_40_166_0 = _rtP -> P_137 ; } else { _rtB -> B_40_166_0 =
_rtB -> B_40_165_0 ; } if ( ( _rtDW -> LastMajorTimeA_f >= ssGetTaskTime ( S
, 0 ) ) && ( _rtDW -> LastMajorTimeB_ga >= ssGetTaskTime ( S , 0 ) ) ) { _rtB
-> B_40_167_0 = _rtB -> B_40_166_0 ; } else { if ( ( ( _rtDW ->
LastMajorTimeA_f < _rtDW -> LastMajorTimeB_ga ) && ( _rtDW ->
LastMajorTimeB_ga < ssGetTaskTime ( S , 0 ) ) ) || ( ( _rtDW ->
LastMajorTimeA_f >= _rtDW -> LastMajorTimeB_ga ) && ( _rtDW ->
LastMajorTimeA_f >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime (
S , 0 ) - _rtDW -> LastMajorTimeB_ga ; rtb_B_40_3_0 = _rtDW -> PrevYB_bd ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_f ;
rtb_B_40_3_0 = _rtDW -> PrevYA_ih ; } riseValLimit = deltaT * _rtP -> P_138 ;
rateLimiterRate = _rtB -> B_40_166_0 - rtb_B_40_3_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_40_167_0 = rtb_B_40_3_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_139 ; if ( rateLimiterRate < deltaT ) { _rtB ->
B_40_167_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB -> B_40_167_0 = _rtB ->
B_40_166_0 ; } } } _rtB -> B_40_168_0 = _rtP -> P_140 * _rtB -> B_40_167_0 ;
_rtB -> B_40_169_0 = _rtP -> P_141 * _rtB -> B_40_168_0 ;
vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model3/Data Store Read" ,
0 ) ; _rtB -> B_40_170_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_p != 0 ) {
_rtX -> vv0at_CSTATE_nh = _rtB -> B_40_170_0 ; } _rtB -> B_40_171_0 = _rtX ->
vv0at_CSTATE_nh ; _rtB -> B_40_172_0 = muDoubleScalarAbs ( _rtB -> B_40_171_0
) ; _rtB -> B_40_173_0 = _rtP -> P_142 [ 0 ] ; _rtB -> B_40_173_0 = _rtB ->
B_40_173_0 * _rtB -> B_40_172_0 + _rtP -> P_142 [ 1 ] ; _rtB -> B_40_173_0 =
_rtB -> B_40_173_0 * _rtB -> B_40_172_0 + _rtP -> P_142 [ 2 ] ; _rtB ->
B_40_174_0 = _rtP -> P_143 * _rtB -> B_40_173_0 ; _rtB -> B_40_175_0 = _rtB
-> B_40_10_0_c * _rtB -> B_40_174_0 ; _rtB -> B_40_176_0 = _rtB -> B_40_169_0
- _rtB -> B_40_175_0 ; if ( _rtB -> B_40_176_0 > _rtB -> B_40_75_0_m ) { _rtB
-> B_40_179_0 = _rtB -> B_40_75_0_m ; } else { if ( _rtB -> B_40_176_0 < _rtB
-> B_40_76_0_m ) { _rtB -> B_8_1_0 = _rtB -> B_40_76_0_m ; } else { _rtB ->
B_8_1_0 = _rtB -> B_40_176_0 ; } _rtB -> B_40_179_0 = _rtB -> B_8_1_0 ; } {
real_T * * uBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_p .
TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_p . TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ;
real_T tMinusDelay = simTime - _rtP -> P_144 ; _rtB -> B_40_180_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_ph . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_ph . Last , _rtDW -> TransportDelay_IWORK_ph . Tail ,
_rtDW -> TransportDelay_IWORK_ph . Head , _rtP -> P_145 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_40_181_0 = 0.0 ; _rtB -> B_40_181_0 += _rtP -> P_147 * _rtX
-> TransferFcn_CSTATE_c ; _rtB -> B_40_181_0 += _rtP -> P_148 * _rtB ->
B_40_180_0 ; if ( _rtB -> B_40_181_0 > _rtP -> P_150 ) { _rtB -> B_40_182_0 =
_rtB -> B_40_181_0 - _rtP -> P_150 ; } else if ( _rtB -> B_40_181_0 >= _rtP
-> P_149 ) { _rtB -> B_40_182_0 = 0.0 ; } else { _rtB -> B_40_182_0 = _rtB ->
B_40_181_0 - _rtP -> P_149 ; } _rtB -> B_40_183_0 = _rtP -> P_151 * _rtB ->
B_40_182_0 ; if ( _rtB -> B_40_183_0 > _rtP -> P_152 ) { _rtB -> B_40_184_0 =
_rtP -> P_152 ; } else if ( _rtB -> B_40_183_0 < _rtP -> P_153 ) { _rtB ->
B_40_184_0 = _rtP -> P_153 ; } else { _rtB -> B_40_184_0 = _rtB -> B_40_183_0
; } if ( ( _rtDW -> LastMajorTimeA_h >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW
-> LastMajorTimeB_c1 >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_40_185_0 =
_rtB -> B_40_184_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_h < _rtDW ->
LastMajorTimeB_c1 ) && ( _rtDW -> LastMajorTimeB_c1 < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_h >= _rtDW -> LastMajorTimeB_c1 ) && (
_rtDW -> LastMajorTimeA_h >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_c1 ; rtb_B_40_3_0 = _rtDW
-> PrevYB_k ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_h ; rtb_B_40_3_0 = _rtDW -> PrevYA_a ; } riseValLimit = deltaT
* _rtP -> P_154 ; rateLimiterRate = _rtB -> B_40_184_0 - rtb_B_40_3_0 ; if (
rateLimiterRate > riseValLimit ) { _rtB -> B_40_185_0 = rtb_B_40_3_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_155 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_40_185_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB ->
B_40_185_0 = _rtB -> B_40_184_0 ; } } } _rtB -> B_40_186_0 = _rtP -> P_156 *
_rtB -> B_40_185_0 ; _rtB -> B_40_187_0 = _rtP -> P_157 * _rtB -> B_40_186_0
; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model4/Data Store Read" ,
0 ) ; _rtB -> B_40_188_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_k != 0 ) {
_rtX -> vv0at_CSTATE_p = _rtB -> B_40_188_0 ; } _rtB -> B_40_189_0 = _rtX ->
vv0at_CSTATE_p ; _rtB -> B_40_190_0 = muDoubleScalarAbs ( _rtB -> B_40_189_0
) ; _rtB -> B_40_191_0 = _rtP -> P_158 [ 0 ] ; _rtB -> B_40_191_0 = _rtB ->
B_40_191_0 * _rtB -> B_40_190_0 + _rtP -> P_158 [ 1 ] ; _rtB -> B_40_191_0 =
_rtB -> B_40_191_0 * _rtB -> B_40_190_0 + _rtP -> P_158 [ 2 ] ; _rtB ->
B_40_192_0 = _rtP -> P_159 * _rtB -> B_40_191_0 ; _rtB -> B_40_193_0 = _rtB
-> B_40_11_0_b * _rtB -> B_40_192_0 ; _rtB -> B_40_194_0 = _rtB -> B_40_187_0
- _rtB -> B_40_193_0 ; if ( _rtB -> B_40_194_0 > _rtB -> B_40_78_0_j ) { _rtB
-> B_40_197_0 = _rtB -> B_40_78_0_j ; } else { if ( _rtB -> B_40_194_0 < _rtB
-> B_40_79_0_h ) { _rtB -> B_9_1_0 = _rtB -> B_40_79_0_h ; } else { _rtB ->
B_9_1_0 = _rtB -> B_40_194_0 ; } _rtB -> B_40_197_0 = _rtB -> B_9_1_0 ; } {
real_T * * uBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_mp .
TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_mp . TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S )
; real_T tMinusDelay = simTime - _rtP -> P_160 ; _rtB -> B_40_198_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_pr . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_pr . Last , _rtDW -> TransportDelay_IWORK_pr . Tail ,
_rtDW -> TransportDelay_IWORK_pr . Head , _rtP -> P_161 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_40_199_0 = 0.0 ; _rtB -> B_40_199_0 += _rtP -> P_163 * _rtX
-> TransferFcn_CSTATE_dt ; _rtB -> B_40_199_0 += _rtP -> P_164 * _rtB ->
B_40_198_0 ; if ( _rtB -> B_40_199_0 > _rtP -> P_166 ) { _rtB -> B_40_200_0 =
_rtB -> B_40_199_0 - _rtP -> P_166 ; } else if ( _rtB -> B_40_199_0 >= _rtP
-> P_165 ) { _rtB -> B_40_200_0 = 0.0 ; } else { _rtB -> B_40_200_0 = _rtB ->
B_40_199_0 - _rtP -> P_165 ; } _rtB -> B_40_201_0 = _rtP -> P_167 * _rtB ->
B_40_200_0 ; if ( _rtB -> B_40_201_0 > _rtP -> P_168 ) { _rtB -> B_40_202_0 =
_rtP -> P_168 ; } else if ( _rtB -> B_40_201_0 < _rtP -> P_169 ) { _rtB ->
B_40_202_0 = _rtP -> P_169 ; } else { _rtB -> B_40_202_0 = _rtB -> B_40_201_0
; } if ( ( _rtDW -> LastMajorTimeA_lm >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW
-> LastMajorTimeB_ei >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_40_203_0 =
_rtB -> B_40_202_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_lm < _rtDW ->
LastMajorTimeB_ei ) && ( _rtDW -> LastMajorTimeB_ei < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_lm >= _rtDW -> LastMajorTimeB_ei ) && (
_rtDW -> LastMajorTimeA_lm >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_ei ; rtb_B_40_3_0 = _rtDW
-> PrevYB_g ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_lm ; rtb_B_40_3_0 = _rtDW -> PrevYA_j ; } riseValLimit =
deltaT * _rtP -> P_170 ; rateLimiterRate = _rtB -> B_40_202_0 - rtb_B_40_3_0
; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_40_203_0 = rtb_B_40_3_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_171 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_40_203_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB ->
B_40_203_0 = _rtB -> B_40_202_0 ; } } } _rtB -> B_40_204_0 = _rtP -> P_172 *
_rtB -> B_40_203_0 ; _rtB -> B_40_205_0 = _rtP -> P_173 * _rtB -> B_40_204_0
; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model/Data Store Read" , 0
) ; _rtB -> B_40_206_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_ej != 0 ) {
_rtX -> vv0at_CSTATE_l = _rtB -> B_40_206_0 ; } _rtB -> B_40_207_0 = _rtX ->
vv0at_CSTATE_l ; _rtB -> B_40_208_0 = muDoubleScalarAbs ( _rtB -> B_40_207_0
) ; _rtB -> B_40_209_0 = _rtP -> P_174 [ 0 ] ; _rtB -> B_40_209_0 = _rtB ->
B_40_209_0 * _rtB -> B_40_208_0 + _rtP -> P_174 [ 1 ] ; _rtB -> B_40_209_0 =
_rtB -> B_40_209_0 * _rtB -> B_40_208_0 + _rtP -> P_174 [ 2 ] ; _rtB ->
B_40_210_0 = _rtP -> P_175 * _rtB -> B_40_209_0 ; _rtB -> B_40_211_0 = _rtB
-> B_40_13_0 * _rtB -> B_40_210_0 ; _rtB -> B_40_212_0 = _rtB -> B_40_205_0 -
_rtB -> B_40_211_0 ; if ( _rtB -> B_40_212_0 > _rtB -> B_40_81_0_c ) { _rtB
-> B_40_215_0 = _rtB -> B_40_81_0_c ; } else { if ( _rtB -> B_40_212_0 < _rtB
-> B_40_82_0_c ) { _rtB -> B_10_1_0 = _rtB -> B_40_82_0_c ; } else { _rtB ->
B_10_1_0 = _rtB -> B_40_212_0 ; } _rtB -> B_40_215_0 = _rtB -> B_10_1_0 ; } {
real_T * * uBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_e .
TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_e . TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ;
real_T tMinusDelay = simTime - _rtP -> P_176 ; _rtB -> B_40_216_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_n . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_n . Last , _rtDW -> TransportDelay_IWORK_n . Tail ,
_rtDW -> TransportDelay_IWORK_n . Head , _rtP -> P_177 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_40_217_0 = 0.0 ; _rtB -> B_40_217_0 += _rtP -> P_179 * _rtX
-> TransferFcn_CSTATE_i ; _rtB -> B_40_217_0 += _rtP -> P_180 * _rtB ->
B_40_216_0 ; if ( _rtB -> B_40_217_0 > _rtP -> P_182 ) { _rtB -> B_40_218_0 =
_rtB -> B_40_217_0 - _rtP -> P_182 ; } else if ( _rtB -> B_40_217_0 >= _rtP
-> P_181 ) { _rtB -> B_40_218_0 = 0.0 ; } else { _rtB -> B_40_218_0 = _rtB ->
B_40_217_0 - _rtP -> P_181 ; } _rtB -> B_40_219_0 = _rtP -> P_183 * _rtB ->
B_40_218_0 ; if ( _rtB -> B_40_219_0 > _rtP -> P_184 ) { _rtB -> B_40_220_0 =
_rtP -> P_184 ; } else if ( _rtB -> B_40_219_0 < _rtP -> P_185 ) { _rtB ->
B_40_220_0 = _rtP -> P_185 ; } else { _rtB -> B_40_220_0 = _rtB -> B_40_219_0
; } if ( ( _rtDW -> LastMajorTimeA_n >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW
-> LastMajorTimeB_i >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_40_221_0 =
_rtB -> B_40_220_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_n < _rtDW ->
LastMajorTimeB_i ) && ( _rtDW -> LastMajorTimeB_i < ssGetTaskTime ( S , 0 ) )
) || ( ( _rtDW -> LastMajorTimeA_n >= _rtDW -> LastMajorTimeB_i ) && ( _rtDW
-> LastMajorTimeA_n >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime
( S , 0 ) - _rtDW -> LastMajorTimeB_i ; rtb_B_40_3_0 = _rtDW -> PrevYB_f ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_n ;
rtb_B_40_3_0 = _rtDW -> PrevYA_h ; } riseValLimit = deltaT * _rtP -> P_186 ;
rateLimiterRate = _rtB -> B_40_220_0 - rtb_B_40_3_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_40_221_0 = rtb_B_40_3_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_187 ; if ( rateLimiterRate < deltaT ) { _rtB ->
B_40_221_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB -> B_40_221_0 = _rtB ->
B_40_220_0 ; } } } _rtB -> B_40_222_0 = _rtP -> P_188 * _rtB -> B_40_221_0 ;
_rtB -> B_40_223_0 = _rtP -> P_189 * _rtB -> B_40_222_0 ;
vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model1/Data Store Read" ,
0 ) ; _rtB -> B_40_224_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_n != 0 ) {
_rtX -> vv0at_CSTATE_c = _rtB -> B_40_224_0 ; } _rtB -> B_40_225_0 = _rtX ->
vv0at_CSTATE_c ; _rtB -> B_40_226_0 = muDoubleScalarAbs ( _rtB -> B_40_225_0
) ; _rtB -> B_40_227_0 = _rtP -> P_190 [ 0 ] ; _rtB -> B_40_227_0 = _rtB ->
B_40_227_0 * _rtB -> B_40_226_0 + _rtP -> P_190 [ 1 ] ; _rtB -> B_40_227_0 =
_rtB -> B_40_227_0 * _rtB -> B_40_226_0 + _rtP -> P_190 [ 2 ] ; _rtB ->
B_40_228_0 = _rtP -> P_191 * _rtB -> B_40_227_0 ; _rtB -> B_40_229_0 = _rtB
-> B_40_14_0_p * _rtB -> B_40_228_0 ; _rtB -> B_40_230_0 = _rtB -> B_40_223_0
- _rtB -> B_40_229_0 ; if ( _rtB -> B_40_230_0 > _rtB -> B_40_84_0_p ) { _rtB
-> B_40_233_0 = _rtB -> B_40_84_0_p ; } else { if ( _rtB -> B_40_230_0 < _rtB
-> B_40_85_0_p ) { _rtB -> B_11_1_0 = _rtB -> B_40_85_0_p ; } else { _rtB ->
B_11_1_0 = _rtB -> B_40_230_0 ; } _rtB -> B_40_233_0 = _rtB -> B_11_1_0 ; } {
real_T * * uBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_jx .
TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_jx . TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S )
; real_T tMinusDelay = simTime - _rtP -> P_192 ; _rtB -> B_40_234_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_fm . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_fm . Last , _rtDW -> TransportDelay_IWORK_fm . Tail ,
_rtDW -> TransportDelay_IWORK_fm . Head , _rtP -> P_193 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_40_235_0 = 0.0 ; _rtB -> B_40_235_0 += _rtP -> P_195 * _rtX
-> TransferFcn_CSTATE_n ; _rtB -> B_40_235_0 += _rtP -> P_196 * _rtB ->
B_40_234_0 ; if ( _rtB -> B_40_235_0 > _rtP -> P_198 ) { _rtB -> B_40_236_0 =
_rtB -> B_40_235_0 - _rtP -> P_198 ; } else if ( _rtB -> B_40_235_0 >= _rtP
-> P_197 ) { _rtB -> B_40_236_0 = 0.0 ; } else { _rtB -> B_40_236_0 = _rtB ->
B_40_235_0 - _rtP -> P_197 ; } _rtB -> B_40_237_0 = _rtP -> P_199 * _rtB ->
B_40_236_0 ; if ( _rtB -> B_40_237_0 > _rtP -> P_200 ) { _rtB -> B_40_238_0 =
_rtP -> P_200 ; } else if ( _rtB -> B_40_237_0 < _rtP -> P_201 ) { _rtB ->
B_40_238_0 = _rtP -> P_201 ; } else { _rtB -> B_40_238_0 = _rtB -> B_40_237_0
; } if ( ( _rtDW -> LastMajorTimeA_c >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW
-> LastMajorTimeB_n >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_40_239_0 =
_rtB -> B_40_238_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_c < _rtDW ->
LastMajorTimeB_n ) && ( _rtDW -> LastMajorTimeB_n < ssGetTaskTime ( S , 0 ) )
) || ( ( _rtDW -> LastMajorTimeA_c >= _rtDW -> LastMajorTimeB_n ) && ( _rtDW
-> LastMajorTimeA_c >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime
( S , 0 ) - _rtDW -> LastMajorTimeB_n ; rtb_B_40_3_0 = _rtDW -> PrevYB_il ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_c ;
rtb_B_40_3_0 = _rtDW -> PrevYA_f ; } riseValLimit = deltaT * _rtP -> P_202 ;
rateLimiterRate = _rtB -> B_40_238_0 - rtb_B_40_3_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_40_239_0 = rtb_B_40_3_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_203 ; if ( rateLimiterRate < deltaT ) { _rtB ->
B_40_239_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB -> B_40_239_0 = _rtB ->
B_40_238_0 ; } } } _rtB -> B_40_240_0 = _rtP -> P_204 * _rtB -> B_40_239_0 ;
_rtB -> B_40_241_0 = _rtP -> P_205 * _rtB -> B_40_240_0 ;
vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model2/Data Store Read" ,
0 ) ; _rtB -> B_40_242_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_i != 0 ) {
_rtX -> vv0at_CSTATE_h = _rtB -> B_40_242_0 ; } _rtB -> B_40_243_0 = _rtX ->
vv0at_CSTATE_h ; _rtB -> B_40_244_0 = muDoubleScalarAbs ( _rtB -> B_40_243_0
) ; _rtB -> B_40_245_0 = _rtP -> P_206 [ 0 ] ; _rtB -> B_40_245_0 = _rtB ->
B_40_245_0 * _rtB -> B_40_244_0 + _rtP -> P_206 [ 1 ] ; _rtB -> B_40_245_0 =
_rtB -> B_40_245_0 * _rtB -> B_40_244_0 + _rtP -> P_206 [ 2 ] ; _rtB ->
B_40_246_0 = _rtP -> P_207 * _rtB -> B_40_245_0 ; _rtB -> B_40_247_0 = _rtB
-> B_40_15_0 * _rtB -> B_40_246_0 ; _rtB -> B_40_248_0 = _rtB -> B_40_241_0 -
_rtB -> B_40_247_0 ; if ( _rtB -> B_40_248_0 > _rtB -> B_40_87_0 ) { _rtB ->
B_40_251_0 = _rtB -> B_40_87_0 ; } else { if ( _rtB -> B_40_248_0 < _rtB ->
B_40_88_0 ) { _rtB -> B_12_1_0 = _rtB -> B_40_88_0 ; } else { _rtB ->
B_12_1_0 = _rtB -> B_40_248_0 ; } _rtB -> B_40_251_0 = _rtB -> B_12_1_0 ; } {
real_T * * uBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_eq .
TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_eq . TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S )
; real_T tMinusDelay = simTime - _rtP -> P_208 ; _rtB -> B_40_252_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_a . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_a . Last , _rtDW -> TransportDelay_IWORK_a . Tail ,
_rtDW -> TransportDelay_IWORK_a . Head , _rtP -> P_209 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_40_253_0 = 0.0 ; _rtB -> B_40_253_0 += _rtP -> P_211 * _rtX
-> TransferFcn_CSTATE_md ; _rtB -> B_40_253_0 += _rtP -> P_212 * _rtB ->
B_40_252_0 ; if ( _rtB -> B_40_253_0 > _rtP -> P_214 ) { _rtB -> B_40_254_0 =
_rtB -> B_40_253_0 - _rtP -> P_214 ; } else if ( _rtB -> B_40_253_0 >= _rtP
-> P_213 ) { _rtB -> B_40_254_0 = 0.0 ; } else { _rtB -> B_40_254_0 = _rtB ->
B_40_253_0 - _rtP -> P_213 ; } _rtB -> B_40_255_0 = _rtP -> P_215 * _rtB ->
B_40_254_0 ; if ( _rtB -> B_40_255_0 > _rtP -> P_216 ) { _rtB -> B_40_256_0 =
_rtP -> P_216 ; } else if ( _rtB -> B_40_255_0 < _rtP -> P_217 ) { _rtB ->
B_40_256_0 = _rtP -> P_217 ; } else { _rtB -> B_40_256_0 = _rtB -> B_40_255_0
; } if ( ( _rtDW -> LastMajorTimeA_a >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW
-> LastMajorTimeB_k >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_40_257_0 =
_rtB -> B_40_256_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_a < _rtDW ->
LastMajorTimeB_k ) && ( _rtDW -> LastMajorTimeB_k < ssGetTaskTime ( S , 0 ) )
) || ( ( _rtDW -> LastMajorTimeA_a >= _rtDW -> LastMajorTimeB_k ) && ( _rtDW
-> LastMajorTimeA_a >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime
( S , 0 ) - _rtDW -> LastMajorTimeB_k ; rtb_B_40_3_0 = _rtDW -> PrevYB_o ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_a ;
rtb_B_40_3_0 = _rtDW -> PrevYA_h5 ; } riseValLimit = deltaT * _rtP -> P_218 ;
rateLimiterRate = _rtB -> B_40_256_0 - rtb_B_40_3_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_40_257_0 = rtb_B_40_3_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_219 ; if ( rateLimiterRate < deltaT ) { _rtB ->
B_40_257_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB -> B_40_257_0 = _rtB ->
B_40_256_0 ; } } } _rtB -> B_40_258_0 = _rtP -> P_220 * _rtB -> B_40_257_0 ;
_rtB -> B_40_259_0 = _rtP -> P_221 * _rtB -> B_40_258_0 ;
vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model3/Data Store Read" ,
0 ) ; _rtB -> B_40_260_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_l != 0 ) {
_rtX -> vv0at_CSTATE_k = _rtB -> B_40_260_0 ; } _rtB -> B_40_261_0 = _rtX ->
vv0at_CSTATE_k ; _rtB -> B_40_262_0 = muDoubleScalarAbs ( _rtB -> B_40_261_0
) ; _rtB -> B_40_263_0 = _rtP -> P_222 [ 0 ] ; _rtB -> B_40_263_0 = _rtB ->
B_40_263_0 * _rtB -> B_40_262_0 + _rtP -> P_222 [ 1 ] ; _rtB -> B_40_263_0 =
_rtB -> B_40_263_0 * _rtB -> B_40_262_0 + _rtP -> P_222 [ 2 ] ; _rtB ->
B_40_264_0 = _rtP -> P_223 * _rtB -> B_40_263_0 ; _rtB -> B_40_265_0 = _rtB
-> B_40_16_0 * _rtB -> B_40_264_0 ; _rtB -> B_40_266_0 = _rtB -> B_40_259_0 -
_rtB -> B_40_265_0 ; if ( _rtB -> B_40_266_0 > _rtB -> B_40_90_0_a ) { _rtB
-> B_40_269_0 = _rtB -> B_40_90_0_a ; } else { if ( _rtB -> B_40_266_0 < _rtB
-> B_40_91_0_e ) { _rtB -> B_13_1_0 = _rtB -> B_40_91_0_e ; } else { _rtB ->
B_13_1_0 = _rtB -> B_40_266_0 ; } _rtB -> B_40_269_0 = _rtB -> B_13_1_0 ; } {
real_T * * uBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_ja .
TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_ja . TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S )
; real_T tMinusDelay = simTime - _rtP -> P_224 ; _rtB -> B_40_270_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_ov . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_ov . Last , _rtDW -> TransportDelay_IWORK_ov . Tail ,
_rtDW -> TransportDelay_IWORK_ov . Head , _rtP -> P_225 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_40_271_0 = 0.0 ; _rtB -> B_40_271_0 += _rtP -> P_227 * _rtX
-> TransferFcn_CSTATE_nd ; _rtB -> B_40_271_0 += _rtP -> P_228 * _rtB ->
B_40_270_0 ; if ( _rtB -> B_40_271_0 > _rtP -> P_230 ) { _rtB -> B_40_272_0 =
_rtB -> B_40_271_0 - _rtP -> P_230 ; } else if ( _rtB -> B_40_271_0 >= _rtP
-> P_229 ) { _rtB -> B_40_272_0 = 0.0 ; } else { _rtB -> B_40_272_0 = _rtB ->
B_40_271_0 - _rtP -> P_229 ; } _rtB -> B_40_273_0 = _rtP -> P_231 * _rtB ->
B_40_272_0 ; if ( _rtB -> B_40_273_0 > _rtP -> P_232 ) { _rtB -> B_40_274_0 =
_rtP -> P_232 ; } else if ( _rtB -> B_40_273_0 < _rtP -> P_233 ) { _rtB ->
B_40_274_0 = _rtP -> P_233 ; } else { _rtB -> B_40_274_0 = _rtB -> B_40_273_0
; } if ( ( _rtDW -> LastMajorTimeA_en >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW
-> LastMajorTimeB_nk >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_40_275_0 =
_rtB -> B_40_274_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_en < _rtDW ->
LastMajorTimeB_nk ) && ( _rtDW -> LastMajorTimeB_nk < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_en >= _rtDW -> LastMajorTimeB_nk ) && (
_rtDW -> LastMajorTimeA_en >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_nk ; rtb_B_40_3_0 = _rtDW
-> PrevYB_la ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_en ; rtb_B_40_3_0 = _rtDW -> PrevYA_e ; } riseValLimit =
deltaT * _rtP -> P_234 ; rateLimiterRate = _rtB -> B_40_274_0 - rtb_B_40_3_0
; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_40_275_0 = rtb_B_40_3_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_235 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_40_275_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB ->
B_40_275_0 = _rtB -> B_40_274_0 ; } } } _rtB -> B_40_276_0 = _rtP -> P_236 *
_rtB -> B_40_275_0 ; _rtB -> B_40_277_0 = _rtP -> P_237 * _rtB -> B_40_276_0
; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model4/Data Store Read" ,
0 ) ; _rtB -> B_40_278_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_fj != 0 )
{ _rtX -> vv0at_CSTATE_l4 = _rtB -> B_40_278_0 ; } _rtB -> B_40_279_0 = _rtX
-> vv0at_CSTATE_l4 ; _rtB -> B_40_280_0 = muDoubleScalarAbs ( _rtB ->
B_40_279_0 ) ; _rtB -> B_40_281_0 = _rtP -> P_238 [ 0 ] ; _rtB -> B_40_281_0
= _rtB -> B_40_281_0 * _rtB -> B_40_280_0 + _rtP -> P_238 [ 1 ] ; _rtB ->
B_40_281_0 = _rtB -> B_40_281_0 * _rtB -> B_40_280_0 + _rtP -> P_238 [ 2 ] ;
_rtB -> B_40_282_0 = _rtP -> P_239 * _rtB -> B_40_281_0 ; _rtB -> B_40_283_0
= _rtB -> B_40_17_0_c * _rtB -> B_40_282_0 ; _rtB -> B_40_284_0 = _rtB ->
B_40_277_0 - _rtB -> B_40_283_0 ; if ( _rtB -> B_40_284_0 > _rtB ->
B_40_93_0_a ) { _rtB -> B_40_287_0 = _rtB -> B_40_93_0_a ; } else { if ( _rtB
-> B_40_284_0 < _rtB -> B_40_94_0_a ) { _rtB -> B_14_1_0 = _rtB ->
B_40_94_0_a ; } else { _rtB -> B_14_1_0 = _rtB -> B_40_284_0 ; } _rtB ->
B_40_287_0 = _rtB -> B_14_1_0 ; } { real_T * * uBuffer = ( real_T * * ) &
_rtDW -> TransportDelay_PWORK_g . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_g . TUbufferPtrs [ 1 ] ; real_T
simTime = ssGetT ( S ) ; real_T tMinusDelay = simTime - _rtP -> P_240 ; _rtB
-> B_40_288_0 = Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 ,
* tBuffer , * uBuffer , _rtDW -> TransportDelay_IWORK_dd . CircularBufSize ,
& _rtDW -> TransportDelay_IWORK_dd . Last , _rtDW -> TransportDelay_IWORK_dd
. Tail , _rtDW -> TransportDelay_IWORK_dd . Head , _rtP -> P_241 , 0 , (
boolean_T ) ( ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) ==
ssGetT ( S ) ) ) ) ; } _rtB -> B_40_289_0 = 0.0 ; _rtB -> B_40_289_0 += _rtP
-> P_243 * _rtX -> TransferFcn_CSTATE_g ; _rtB -> B_40_289_0 += _rtP -> P_244
* _rtB -> B_40_288_0 ; if ( _rtB -> B_40_289_0 > _rtP -> P_246 ) { _rtB ->
B_40_290_0 = _rtB -> B_40_289_0 - _rtP -> P_246 ; } else if ( _rtB ->
B_40_289_0 >= _rtP -> P_245 ) { _rtB -> B_40_290_0 = 0.0 ; } else { _rtB ->
B_40_290_0 = _rtB -> B_40_289_0 - _rtP -> P_245 ; } _rtB -> B_40_291_0 = _rtP
-> P_247 * _rtB -> B_40_290_0 ; if ( _rtB -> B_40_291_0 > _rtP -> P_248 ) {
_rtB -> B_40_292_0 = _rtP -> P_248 ; } else if ( _rtB -> B_40_291_0 < _rtP ->
P_249 ) { _rtB -> B_40_292_0 = _rtP -> P_249 ; } else { _rtB -> B_40_292_0 =
_rtB -> B_40_291_0 ; } if ( ( _rtDW -> LastMajorTimeA_k >= ssGetTaskTime ( S
, 0 ) ) && ( _rtDW -> LastMajorTimeB_oq >= ssGetTaskTime ( S , 0 ) ) ) { _rtB
-> B_40_293_0 = _rtB -> B_40_292_0 ; } else { if ( ( ( _rtDW ->
LastMajorTimeA_k < _rtDW -> LastMajorTimeB_oq ) && ( _rtDW ->
LastMajorTimeB_oq < ssGetTaskTime ( S , 0 ) ) ) || ( ( _rtDW ->
LastMajorTimeA_k >= _rtDW -> LastMajorTimeB_oq ) && ( _rtDW ->
LastMajorTimeA_k >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime (
S , 0 ) - _rtDW -> LastMajorTimeB_oq ; rtb_B_40_3_0 = _rtDW -> PrevYB_ih ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_k ;
rtb_B_40_3_0 = _rtDW -> PrevYA_hx ; } riseValLimit = deltaT * _rtP -> P_250 ;
rateLimiterRate = _rtB -> B_40_292_0 - rtb_B_40_3_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_40_293_0 = rtb_B_40_3_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_251 ; if ( rateLimiterRate < deltaT ) { _rtB ->
B_40_293_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB -> B_40_293_0 = _rtB ->
B_40_292_0 ; } } } _rtB -> B_40_294_0 = _rtP -> P_252 * _rtB -> B_40_293_0 ;
_rtB -> B_40_295_0 = _rtP -> P_253 * _rtB -> B_40_294_0 ;
vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model/Data Store Read" , 0
) ; _rtB -> B_40_296_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_pd != 0 ) {
_rtX -> vv0at_CSTATE_md = _rtB -> B_40_296_0 ; } _rtB -> B_40_297_0 = _rtX ->
vv0at_CSTATE_md ; _rtB -> B_40_298_0 = muDoubleScalarAbs ( _rtB -> B_40_297_0
) ; _rtB -> B_40_299_0 = _rtP -> P_254 [ 0 ] ; _rtB -> B_40_299_0 = _rtB ->
B_40_299_0 * _rtB -> B_40_298_0 + _rtP -> P_254 [ 1 ] ; _rtB -> B_40_299_0 =
_rtB -> B_40_299_0 * _rtB -> B_40_298_0 + _rtP -> P_254 [ 2 ] ; _rtB ->
B_40_300_0 = _rtP -> P_255 * _rtB -> B_40_299_0 ; _rtB -> B_40_301_0 = _rtB
-> B_40_19_0_f * _rtB -> B_40_300_0 ; _rtB -> B_40_302_0 = _rtB -> B_40_295_0
- _rtB -> B_40_301_0 ; if ( _rtB -> B_40_302_0 > _rtB -> B_40_96_0_i ) { _rtB
-> B_40_305_0 = _rtB -> B_40_96_0_i ; } else { if ( _rtB -> B_40_302_0 < _rtB
-> B_40_97_0_l ) { _rtB -> B_15_1_0 = _rtB -> B_40_97_0_l ; } else { _rtB ->
B_15_1_0 = _rtB -> B_40_302_0 ; } _rtB -> B_40_305_0 = _rtB -> B_15_1_0 ; } {
real_T * * uBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_l .
TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_l . TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ;
real_T tMinusDelay = simTime - _rtP -> P_256 ; _rtB -> B_40_306_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_az . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_az . Last , _rtDW -> TransportDelay_IWORK_az . Tail ,
_rtDW -> TransportDelay_IWORK_az . Head , _rtP -> P_257 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_40_307_0 = 0.0 ; _rtB -> B_40_307_0 += _rtP -> P_259 * _rtX
-> TransferFcn_CSTATE_mf ; _rtB -> B_40_307_0 += _rtP -> P_260 * _rtB ->
B_40_306_0 ; if ( _rtB -> B_40_307_0 > _rtP -> P_262 ) { _rtB -> B_40_308_0 =
_rtB -> B_40_307_0 - _rtP -> P_262 ; } else if ( _rtB -> B_40_307_0 >= _rtP
-> P_261 ) { _rtB -> B_40_308_0 = 0.0 ; } else { _rtB -> B_40_308_0 = _rtB ->
B_40_307_0 - _rtP -> P_261 ; } _rtB -> B_40_309_0 = _rtP -> P_263 * _rtB ->
B_40_308_0 ; if ( _rtB -> B_40_309_0 > _rtP -> P_264 ) { _rtB -> B_40_310_0 =
_rtP -> P_264 ; } else if ( _rtB -> B_40_309_0 < _rtP -> P_265 ) { _rtB ->
B_40_310_0 = _rtP -> P_265 ; } else { _rtB -> B_40_310_0 = _rtB -> B_40_309_0
; } if ( ( _rtDW -> LastMajorTimeA_fz >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW
-> LastMajorTimeB_mu >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_40_311_0 =
_rtB -> B_40_310_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_fz < _rtDW ->
LastMajorTimeB_mu ) && ( _rtDW -> LastMajorTimeB_mu < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_fz >= _rtDW -> LastMajorTimeB_mu ) && (
_rtDW -> LastMajorTimeA_fz >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_mu ; rtb_B_40_3_0 = _rtDW
-> PrevYB_cb ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_fz ; rtb_B_40_3_0 = _rtDW -> PrevYA_g ; } riseValLimit =
deltaT * _rtP -> P_266 ; rateLimiterRate = _rtB -> B_40_310_0 - rtb_B_40_3_0
; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_40_311_0 = rtb_B_40_3_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_267 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_40_311_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB ->
B_40_311_0 = _rtB -> B_40_310_0 ; } } } _rtB -> B_40_312_0 = _rtP -> P_268 *
_rtB -> B_40_311_0 ; _rtB -> B_40_313_0 = _rtP -> P_269 * _rtB -> B_40_312_0
; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model1/Data Store Read" ,
0 ) ; _rtB -> B_40_314_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_b != 0 ) {
_rtX -> vv0at_CSTATE_j1 = _rtB -> B_40_314_0 ; } _rtB -> B_40_315_0 = _rtX ->
vv0at_CSTATE_j1 ; _rtB -> B_40_316_0 = muDoubleScalarAbs ( _rtB -> B_40_315_0
) ; _rtB -> B_40_317_0 = _rtP -> P_270 [ 0 ] ; _rtB -> B_40_317_0 = _rtB ->
B_40_317_0 * _rtB -> B_40_316_0 + _rtP -> P_270 [ 1 ] ; _rtB -> B_40_317_0 =
_rtB -> B_40_317_0 * _rtB -> B_40_316_0 + _rtP -> P_270 [ 2 ] ; _rtB ->
B_40_318_0 = _rtP -> P_271 * _rtB -> B_40_317_0 ; _rtB -> B_40_319_0 = _rtB
-> B_40_20_0_g * _rtB -> B_40_318_0 ; _rtB -> B_40_320_0 = _rtB -> B_40_313_0
- _rtB -> B_40_319_0 ; if ( _rtB -> B_40_320_0 > _rtB -> B_40_99_0_o ) { _rtB
-> B_40_323_0 = _rtB -> B_40_99_0_o ; } else { if ( _rtB -> B_40_320_0 < _rtB
-> B_40_100_0_o ) { _rtB -> B_16_1_0 = _rtB -> B_40_100_0_o ; } else { _rtB
-> B_16_1_0 = _rtB -> B_40_320_0 ; } _rtB -> B_40_323_0 = _rtB -> B_16_1_0 ;
} { real_T * * uBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_ez .
TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_ez . TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S )
; real_T tMinusDelay = simTime - _rtP -> P_272 ; _rtB -> B_40_324_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_fy . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_fy . Last , _rtDW -> TransportDelay_IWORK_fy . Tail ,
_rtDW -> TransportDelay_IWORK_fy . Head , _rtP -> P_273 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_40_325_0 = 0.0 ; _rtB -> B_40_325_0 += _rtP -> P_275 * _rtX
-> TransferFcn_CSTATE_m5 ; _rtB -> B_40_325_0 += _rtP -> P_276 * _rtB ->
B_40_324_0 ; if ( _rtB -> B_40_325_0 > _rtP -> P_278 ) { _rtB -> B_40_326_0 =
_rtB -> B_40_325_0 - _rtP -> P_278 ; } else if ( _rtB -> B_40_325_0 >= _rtP
-> P_277 ) { _rtB -> B_40_326_0 = 0.0 ; } else { _rtB -> B_40_326_0 = _rtB ->
B_40_325_0 - _rtP -> P_277 ; } _rtB -> B_40_327_0 = _rtP -> P_279 * _rtB ->
B_40_326_0 ; if ( _rtB -> B_40_327_0 > _rtP -> P_280 ) { _rtB -> B_40_328_0 =
_rtP -> P_280 ; } else if ( _rtB -> B_40_327_0 < _rtP -> P_281 ) { _rtB ->
B_40_328_0 = _rtP -> P_281 ; } else { _rtB -> B_40_328_0 = _rtB -> B_40_327_0
; } if ( ( _rtDW -> LastMajorTimeA_eh >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW
-> LastMajorTimeB_l >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_40_329_0 =
_rtB -> B_40_328_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_eh < _rtDW ->
LastMajorTimeB_l ) && ( _rtDW -> LastMajorTimeB_l < ssGetTaskTime ( S , 0 ) )
) || ( ( _rtDW -> LastMajorTimeA_eh >= _rtDW -> LastMajorTimeB_l ) && ( _rtDW
-> LastMajorTimeA_eh >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_l ; rtb_B_40_3_0 = _rtDW ->
PrevYB_a ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_eh ; rtb_B_40_3_0 = _rtDW -> PrevYA_d ; } riseValLimit =
deltaT * _rtP -> P_282 ; rateLimiterRate = _rtB -> B_40_328_0 - rtb_B_40_3_0
; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_40_329_0 = rtb_B_40_3_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_283 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_40_329_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB ->
B_40_329_0 = _rtB -> B_40_328_0 ; } } } _rtB -> B_40_330_0 = _rtP -> P_284 *
_rtB -> B_40_329_0 ; _rtB -> B_40_331_0 = _rtP -> P_285 * _rtB -> B_40_330_0
; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model2/Data Store Read" ,
0 ) ; _rtB -> B_40_332_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_i1 != 0 )
{ _rtX -> vv0at_CSTATE_o = _rtB -> B_40_332_0 ; } _rtB -> B_40_333_0 = _rtX
-> vv0at_CSTATE_o ; _rtB -> B_40_334_0 = muDoubleScalarAbs ( _rtB ->
B_40_333_0 ) ; _rtB -> B_40_335_0 = _rtP -> P_286 [ 0 ] ; _rtB -> B_40_335_0
= _rtB -> B_40_335_0 * _rtB -> B_40_334_0 + _rtP -> P_286 [ 1 ] ; _rtB ->
B_40_335_0 = _rtB -> B_40_335_0 * _rtB -> B_40_334_0 + _rtP -> P_286 [ 2 ] ;
_rtB -> B_40_336_0 = _rtP -> P_287 * _rtB -> B_40_335_0 ; _rtB -> B_40_337_0
= _rtB -> B_40_21_0_g * _rtB -> B_40_336_0 ; _rtB -> B_40_338_0 = _rtB ->
B_40_331_0 - _rtB -> B_40_337_0 ; if ( _rtB -> B_40_338_0 > _rtB ->
B_40_102_0_i ) { _rtB -> B_40_341_0 = _rtB -> B_40_102_0_i ; } else { if (
_rtB -> B_40_338_0 < _rtB -> B_40_103_0_f ) { _rtB -> B_17_1_0 = _rtB ->
B_40_103_0_f ; } else { _rtB -> B_17_1_0 = _rtB -> B_40_338_0 ; } _rtB ->
B_40_341_0 = _rtB -> B_17_1_0 ; } { real_T * * uBuffer = ( real_T * * ) &
_rtDW -> TransportDelay_PWORK_pe . TUbufferPtrs [ 0 ] ; real_T * * tBuffer =
( real_T * * ) & _rtDW -> TransportDelay_PWORK_pe . TUbufferPtrs [ 1 ] ;
real_T simTime = ssGetT ( S ) ; real_T tMinusDelay = simTime - _rtP -> P_288
; _rtB -> B_40_342_0 = Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay ,
0.0 , * tBuffer , * uBuffer , _rtDW -> TransportDelay_IWORK_i .
CircularBufSize , & _rtDW -> TransportDelay_IWORK_i . Last , _rtDW ->
TransportDelay_IWORK_i . Tail , _rtDW -> TransportDelay_IWORK_i . Head , _rtP
-> P_289 , 0 , ( boolean_T ) ( ssIsMinorTimeStep ( S ) && (
ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) ) ) ; } _rtB -> B_40_343_0 =
0.0 ; _rtB -> B_40_343_0 += _rtP -> P_291 * _rtX -> TransferFcn_CSTATE_e ;
_rtB -> B_40_343_0 += _rtP -> P_292 * _rtB -> B_40_342_0 ; if ( _rtB ->
B_40_343_0 > _rtP -> P_294 ) { _rtB -> B_40_344_0 = _rtB -> B_40_343_0 - _rtP
-> P_294 ; } else if ( _rtB -> B_40_343_0 >= _rtP -> P_293 ) { _rtB ->
B_40_344_0 = 0.0 ; } else { _rtB -> B_40_344_0 = _rtB -> B_40_343_0 - _rtP ->
P_293 ; } _rtB -> B_40_345_0 = _rtP -> P_295 * _rtB -> B_40_344_0 ; if ( _rtB
-> B_40_345_0 > _rtP -> P_296 ) { _rtB -> B_40_346_0 = _rtP -> P_296 ; } else
if ( _rtB -> B_40_345_0 < _rtP -> P_297 ) { _rtB -> B_40_346_0 = _rtP ->
P_297 ; } else { _rtB -> B_40_346_0 = _rtB -> B_40_345_0 ; } if ( ( _rtDW ->
LastMajorTimeA_eo >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW -> LastMajorTimeB_j
>= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_40_347_0 = _rtB -> B_40_346_0 ; }
else { if ( ( ( _rtDW -> LastMajorTimeA_eo < _rtDW -> LastMajorTimeB_j ) && (
_rtDW -> LastMajorTimeB_j < ssGetTaskTime ( S , 0 ) ) ) || ( ( _rtDW ->
LastMajorTimeA_eo >= _rtDW -> LastMajorTimeB_j ) && ( _rtDW ->
LastMajorTimeA_eo >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime (
S , 0 ) - _rtDW -> LastMajorTimeB_j ; rtb_B_40_3_0 = _rtDW -> PrevYB_d ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_eo ;
rtb_B_40_3_0 = _rtDW -> PrevYA_g5 ; } riseValLimit = deltaT * _rtP -> P_298 ;
rateLimiterRate = _rtB -> B_40_346_0 - rtb_B_40_3_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_40_347_0 = rtb_B_40_3_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_299 ; if ( rateLimiterRate < deltaT ) { _rtB ->
B_40_347_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB -> B_40_347_0 = _rtB ->
B_40_346_0 ; } } } _rtB -> B_40_348_0 = _rtP -> P_300 * _rtB -> B_40_347_0 ;
_rtB -> B_40_349_0 = _rtP -> P_301 * _rtB -> B_40_348_0 ;
vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model3/Data Store Read" ,
0 ) ; _rtB -> B_40_350_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_m != 0 ) {
_rtX -> vv0at_CSTATE_d = _rtB -> B_40_350_0 ; } _rtB -> B_40_351_0 = _rtX ->
vv0at_CSTATE_d ; _rtB -> B_40_352_0 = muDoubleScalarAbs ( _rtB -> B_40_351_0
) ; _rtB -> B_40_353_0 = _rtP -> P_302 [ 0 ] ; _rtB -> B_40_353_0 = _rtB ->
B_40_353_0 * _rtB -> B_40_352_0 + _rtP -> P_302 [ 1 ] ; _rtB -> B_40_353_0 =
_rtB -> B_40_353_0 * _rtB -> B_40_352_0 + _rtP -> P_302 [ 2 ] ; _rtB ->
B_40_354_0 = _rtP -> P_303 * _rtB -> B_40_353_0 ; _rtB -> B_40_355_0 = _rtB
-> B_40_22_0_m * _rtB -> B_40_354_0 ; _rtB -> B_40_356_0 = _rtB -> B_40_349_0
- _rtB -> B_40_355_0 ; if ( _rtB -> B_40_356_0 > _rtB -> B_40_105_0 ) { _rtB
-> B_40_359_0 = _rtB -> B_40_105_0 ; } else { if ( _rtB -> B_40_356_0 < _rtB
-> B_40_106_0 ) { _rtB -> B_18_1_0 = _rtB -> B_40_106_0 ; } else { _rtB ->
B_18_1_0 = _rtB -> B_40_356_0 ; } _rtB -> B_40_359_0 = _rtB -> B_18_1_0 ; } {
real_T * * uBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_j4 .
TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_j4 . TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S )
; real_T tMinusDelay = simTime - _rtP -> P_304 ; _rtB -> B_40_360_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_ax . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_ax . Last , _rtDW -> TransportDelay_IWORK_ax . Tail ,
_rtDW -> TransportDelay_IWORK_ax . Head , _rtP -> P_305 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_40_361_0 = 0.0 ; _rtB -> B_40_361_0 += _rtP -> P_307 * _rtX
-> TransferFcn_CSTATE_f ; _rtB -> B_40_361_0 += _rtP -> P_308 * _rtB ->
B_40_360_0 ; if ( _rtB -> B_40_361_0 > _rtP -> P_310 ) { _rtB -> B_40_362_0 =
_rtB -> B_40_361_0 - _rtP -> P_310 ; } else if ( _rtB -> B_40_361_0 >= _rtP
-> P_309 ) { _rtB -> B_40_362_0 = 0.0 ; } else { _rtB -> B_40_362_0 = _rtB ->
B_40_361_0 - _rtP -> P_309 ; } _rtB -> B_40_363_0 = _rtP -> P_311 * _rtB ->
B_40_362_0 ; if ( _rtB -> B_40_363_0 > _rtP -> P_312 ) { _rtB -> B_40_364_0 =
_rtP -> P_312 ; } else if ( _rtB -> B_40_363_0 < _rtP -> P_313 ) { _rtB ->
B_40_364_0 = _rtP -> P_313 ; } else { _rtB -> B_40_364_0 = _rtB -> B_40_363_0
; } if ( ( _rtDW -> LastMajorTimeA_pp >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW
-> LastMajorTimeB_kl >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_40_365_0 =
_rtB -> B_40_364_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_pp < _rtDW ->
LastMajorTimeB_kl ) && ( _rtDW -> LastMajorTimeB_kl < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_pp >= _rtDW -> LastMajorTimeB_kl ) && (
_rtDW -> LastMajorTimeA_pp >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_kl ; rtb_B_40_3_0 = _rtDW
-> PrevYB_m ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_pp ; rtb_B_40_3_0 = _rtDW -> PrevYA_k3 ; } riseValLimit =
deltaT * _rtP -> P_314 ; rateLimiterRate = _rtB -> B_40_364_0 - rtb_B_40_3_0
; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_40_365_0 = rtb_B_40_3_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_315 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_40_365_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB ->
B_40_365_0 = _rtB -> B_40_364_0 ; } } } _rtB -> B_40_366_0 = _rtP -> P_316 *
_rtB -> B_40_365_0 ; _rtB -> B_40_367_0 = _rtP -> P_317 * _rtB -> B_40_366_0
; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model4/Data Store Read" ,
0 ) ; _rtB -> B_40_368_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_kq != 0 )
{ _rtX -> vv0at_CSTATE_kc = _rtB -> B_40_368_0 ; } _rtB -> B_40_369_0 = _rtX
-> vv0at_CSTATE_kc ; _rtB -> B_40_370_0 = muDoubleScalarAbs ( _rtB ->
B_40_369_0 ) ; _rtB -> B_40_371_0 = _rtP -> P_318 [ 0 ] ; _rtB -> B_40_371_0
= _rtB -> B_40_371_0 * _rtB -> B_40_370_0 + _rtP -> P_318 [ 1 ] ; _rtB ->
B_40_371_0 = _rtB -> B_40_371_0 * _rtB -> B_40_370_0 + _rtP -> P_318 [ 2 ] ;
_rtB -> B_40_372_0 = _rtP -> P_319 * _rtB -> B_40_371_0 ; _rtB -> B_40_373_0
= _rtB -> B_40_23_0_n * _rtB -> B_40_372_0 ; _rtB -> B_40_374_0 = _rtB ->
B_40_367_0 - _rtB -> B_40_373_0 ; if ( _rtB -> B_40_374_0 > _rtB ->
B_40_108_0_i ) { _rtB -> B_40_377_0 = _rtB -> B_40_108_0_i ; } else { if (
_rtB -> B_40_374_0 < _rtB -> B_40_109_0_f ) { _rtB -> B_19_1_0 = _rtB ->
B_40_109_0_f ; } else { _rtB -> B_19_1_0 = _rtB -> B_40_374_0 ; } _rtB ->
B_40_377_0 = _rtB -> B_19_1_0 ; } { real_T * * uBuffer = ( real_T * * ) &
_rtDW -> TransportDelay_PWORK_ny . TUbufferPtrs [ 0 ] ; real_T * * tBuffer =
( real_T * * ) & _rtDW -> TransportDelay_PWORK_ny . TUbufferPtrs [ 1 ] ;
real_T simTime = ssGetT ( S ) ; real_T tMinusDelay = simTime - _rtP -> P_320
; _rtB -> B_40_378_0 = Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay ,
0.0 , * tBuffer , * uBuffer , _rtDW -> TransportDelay_IWORK_g .
CircularBufSize , & _rtDW -> TransportDelay_IWORK_g . Last , _rtDW ->
TransportDelay_IWORK_g . Tail , _rtDW -> TransportDelay_IWORK_g . Head , _rtP
-> P_321 , 0 , ( boolean_T ) ( ssIsMinorTimeStep ( S ) && (
ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) ) ) ; } _rtB -> B_40_379_0 =
0.0 ; _rtB -> B_40_379_0 += _rtP -> P_323 * _rtX -> TransferFcn_CSTATE_g1 ;
_rtB -> B_40_379_0 += _rtP -> P_324 * _rtB -> B_40_378_0 ; if ( _rtB ->
B_40_379_0 > _rtP -> P_326 ) { _rtB -> B_40_380_0 = _rtB -> B_40_379_0 - _rtP
-> P_326 ; } else if ( _rtB -> B_40_379_0 >= _rtP -> P_325 ) { _rtB ->
B_40_380_0 = 0.0 ; } else { _rtB -> B_40_380_0 = _rtB -> B_40_379_0 - _rtP ->
P_325 ; } _rtB -> B_40_381_0 = _rtP -> P_327 * _rtB -> B_40_380_0 ; if ( _rtB
-> B_40_381_0 > _rtP -> P_328 ) { _rtB -> B_40_382_0 = _rtP -> P_328 ; } else
if ( _rtB -> B_40_381_0 < _rtP -> P_329 ) { _rtB -> B_40_382_0 = _rtP ->
P_329 ; } else { _rtB -> B_40_382_0 = _rtB -> B_40_381_0 ; } if ( ( _rtDW ->
LastMajorTimeA_b >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW -> LastMajorTimeB_el
>= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_40_383_0 = _rtB -> B_40_382_0 ; }
else { if ( ( ( _rtDW -> LastMajorTimeA_b < _rtDW -> LastMajorTimeB_el ) && (
_rtDW -> LastMajorTimeB_el < ssGetTaskTime ( S , 0 ) ) ) || ( ( _rtDW ->
LastMajorTimeA_b >= _rtDW -> LastMajorTimeB_el ) && ( _rtDW ->
LastMajorTimeA_b >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime (
S , 0 ) - _rtDW -> LastMajorTimeB_el ; rtb_B_40_3_0 = _rtDW -> PrevYB_m5 ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_b ;
rtb_B_40_3_0 = _rtDW -> PrevYA_i0 ; } riseValLimit = deltaT * _rtP -> P_330 ;
rateLimiterRate = _rtB -> B_40_382_0 - rtb_B_40_3_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_40_383_0 = rtb_B_40_3_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_331 ; if ( rateLimiterRate < deltaT ) { _rtB ->
B_40_383_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB -> B_40_383_0 = _rtB ->
B_40_382_0 ; } } } _rtB -> B_40_384_0 = _rtP -> P_332 * _rtB -> B_40_383_0 ;
_rtB -> B_40_385_0 = _rtP -> P_333 * _rtB -> B_40_384_0 ;
vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model/Data Store Read" , 0
) ; _rtB -> B_40_386_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_h != 0 ) {
_rtX -> vv0at_CSTATE_e = _rtB -> B_40_386_0 ; } _rtB -> B_40_387_0 = _rtX ->
vv0at_CSTATE_e ; _rtB -> B_40_388_0 = muDoubleScalarAbs ( _rtB -> B_40_387_0
) ; _rtB -> B_40_389_0 = _rtP -> P_334 [ 0 ] ; _rtB -> B_40_389_0 = _rtB ->
B_40_389_0 * _rtB -> B_40_388_0 + _rtP -> P_334 [ 1 ] ; _rtB -> B_40_389_0 =
_rtB -> B_40_389_0 * _rtB -> B_40_388_0 + _rtP -> P_334 [ 2 ] ; _rtB ->
B_40_390_0 = _rtP -> P_335 * _rtB -> B_40_389_0 ; _rtB -> B_40_391_0 = _rtB
-> B_40_25_0_p * _rtB -> B_40_390_0 ; _rtB -> B_40_392_0 = _rtB -> B_40_385_0
- _rtB -> B_40_391_0 ; if ( _rtB -> B_40_392_0 > _rtB -> B_40_111_0_g ) {
_rtB -> B_40_395_0 = _rtB -> B_40_111_0_g ; } else { if ( _rtB -> B_40_392_0
< _rtB -> B_40_112_0_c ) { _rtB -> B_20_1_0 = _rtB -> B_40_112_0_c ; } else {
_rtB -> B_20_1_0 = _rtB -> B_40_392_0 ; } _rtB -> B_40_395_0 = _rtB ->
B_20_1_0 ; } { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_d . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T *
* ) & _rtDW -> TransportDelay_PWORK_d . TUbufferPtrs [ 1 ] ; real_T simTime =
ssGetT ( S ) ; real_T tMinusDelay = simTime - _rtP -> P_336 ; _rtB ->
B_40_396_0 = Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , *
tBuffer , * uBuffer , _rtDW -> TransportDelay_IWORK_av . CircularBufSize , &
_rtDW -> TransportDelay_IWORK_av . Last , _rtDW -> TransportDelay_IWORK_av .
Tail , _rtDW -> TransportDelay_IWORK_av . Head , _rtP -> P_337 , 0 , (
boolean_T ) ( ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) ==
ssGetT ( S ) ) ) ) ; } _rtB -> B_40_397_0 = 0.0 ; _rtB -> B_40_397_0 += _rtP
-> P_339 * _rtX -> TransferFcn_CSTATE_cx ; _rtB -> B_40_397_0 += _rtP ->
P_340 * _rtB -> B_40_396_0 ; if ( _rtB -> B_40_397_0 > _rtP -> P_342 ) { _rtB
-> B_40_398_0 = _rtB -> B_40_397_0 - _rtP -> P_342 ; } else if ( _rtB ->
B_40_397_0 >= _rtP -> P_341 ) { _rtB -> B_40_398_0 = 0.0 ; } else { _rtB ->
B_40_398_0 = _rtB -> B_40_397_0 - _rtP -> P_341 ; } _rtB -> B_40_399_0 = _rtP
-> P_343 * _rtB -> B_40_398_0 ; if ( _rtB -> B_40_399_0 > _rtP -> P_344 ) {
_rtB -> B_40_400_0 = _rtP -> P_344 ; } else if ( _rtB -> B_40_399_0 < _rtP ->
P_345 ) { _rtB -> B_40_400_0 = _rtP -> P_345 ; } else { _rtB -> B_40_400_0 =
_rtB -> B_40_399_0 ; } if ( ( _rtDW -> LastMajorTimeA_gx >= ssGetTaskTime ( S
, 0 ) ) && ( _rtDW -> LastMajorTimeB_ju >= ssGetTaskTime ( S , 0 ) ) ) { _rtB
-> B_40_401_0 = _rtB -> B_40_400_0 ; } else { if ( ( ( _rtDW ->
LastMajorTimeA_gx < _rtDW -> LastMajorTimeB_ju ) && ( _rtDW ->
LastMajorTimeB_ju < ssGetTaskTime ( S , 0 ) ) ) || ( ( _rtDW ->
LastMajorTimeA_gx >= _rtDW -> LastMajorTimeB_ju ) && ( _rtDW ->
LastMajorTimeA_gx >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime (
S , 0 ) - _rtDW -> LastMajorTimeB_ju ; rtb_B_40_3_0 = _rtDW -> PrevYB_ilg ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_gx ;
rtb_B_40_3_0 = _rtDW -> PrevYA_ex ; } riseValLimit = deltaT * _rtP -> P_346 ;
rateLimiterRate = _rtB -> B_40_400_0 - rtb_B_40_3_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_40_401_0 = rtb_B_40_3_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_347 ; if ( rateLimiterRate < deltaT ) { _rtB ->
B_40_401_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB -> B_40_401_0 = _rtB ->
B_40_400_0 ; } } } _rtB -> B_40_402_0 = _rtP -> P_348 * _rtB -> B_40_401_0 ;
_rtB -> B_40_403_0 = _rtP -> P_349 * _rtB -> B_40_402_0 ;
vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model1/Data Store Read" ,
0 ) ; _rtB -> B_40_404_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_b0 != 0 )
{ _rtX -> vv0at_CSTATE_a = _rtB -> B_40_404_0 ; } _rtB -> B_40_405_0 = _rtX
-> vv0at_CSTATE_a ; _rtB -> B_40_406_0 = muDoubleScalarAbs ( _rtB ->
B_40_405_0 ) ; _rtB -> B_40_407_0 = _rtP -> P_350 [ 0 ] ; _rtB -> B_40_407_0
= _rtB -> B_40_407_0 * _rtB -> B_40_406_0 + _rtP -> P_350 [ 1 ] ; _rtB ->
B_40_407_0 = _rtB -> B_40_407_0 * _rtB -> B_40_406_0 + _rtP -> P_350 [ 2 ] ;
_rtB -> B_40_408_0 = _rtP -> P_351 * _rtB -> B_40_407_0 ; _rtB -> B_40_409_0
= _rtB -> B_40_26_0_l * _rtB -> B_40_408_0 ; _rtB -> B_40_410_0 = _rtB ->
B_40_403_0 - _rtB -> B_40_409_0 ; if ( _rtB -> B_40_410_0 > _rtB ->
B_40_114_0_o ) { _rtB -> B_40_413_0 = _rtB -> B_40_114_0_o ; } else { if (
_rtB -> B_40_410_0 < _rtB -> B_40_115_0_l ) { _rtB -> B_21_1_0 = _rtB ->
B_40_115_0_l ; } else { _rtB -> B_21_1_0 = _rtB -> B_40_410_0 ; } _rtB ->
B_40_413_0 = _rtB -> B_21_1_0 ; } { real_T * * uBuffer = ( real_T * * ) &
_rtDW -> TransportDelay_PWORK_i . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_i . TUbufferPtrs [ 1 ] ; real_T
simTime = ssGetT ( S ) ; real_T tMinusDelay = simTime - _rtP -> P_352 ; _rtB
-> B_40_414_0 = Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 ,
* tBuffer , * uBuffer , _rtDW -> TransportDelay_IWORK_ly . CircularBufSize ,
& _rtDW -> TransportDelay_IWORK_ly . Last , _rtDW -> TransportDelay_IWORK_ly
. Tail , _rtDW -> TransportDelay_IWORK_ly . Head , _rtP -> P_353 , 0 , (
boolean_T ) ( ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) ==
ssGetT ( S ) ) ) ) ; } _rtB -> B_40_415_0 = 0.0 ; _rtB -> B_40_415_0 += _rtP
-> P_355 * _rtX -> TransferFcn_CSTATE_jb ; _rtB -> B_40_415_0 += _rtP ->
P_356 * _rtB -> B_40_414_0 ; if ( _rtB -> B_40_415_0 > _rtP -> P_358 ) { _rtB
-> B_40_416_0 = _rtB -> B_40_415_0 - _rtP -> P_358 ; } else if ( _rtB ->
B_40_415_0 >= _rtP -> P_357 ) { _rtB -> B_40_416_0 = 0.0 ; } else { _rtB ->
B_40_416_0 = _rtB -> B_40_415_0 - _rtP -> P_357 ; } _rtB -> B_40_417_0 = _rtP
-> P_359 * _rtB -> B_40_416_0 ; if ( _rtB -> B_40_417_0 > _rtP -> P_360 ) {
_rtB -> B_40_418_0 = _rtP -> P_360 ; } else if ( _rtB -> B_40_417_0 < _rtP ->
P_361 ) { _rtB -> B_40_418_0 = _rtP -> P_361 ; } else { _rtB -> B_40_418_0 =
_rtB -> B_40_417_0 ; } if ( ( _rtDW -> LastMajorTimeA_hp >= ssGetTaskTime ( S
, 0 ) ) && ( _rtDW -> LastMajorTimeB_ct >= ssGetTaskTime ( S , 0 ) ) ) { _rtB
-> B_40_419_0 = _rtB -> B_40_418_0 ; } else { if ( ( ( _rtDW ->
LastMajorTimeA_hp < _rtDW -> LastMajorTimeB_ct ) && ( _rtDW ->
LastMajorTimeB_ct < ssGetTaskTime ( S , 0 ) ) ) || ( ( _rtDW ->
LastMajorTimeA_hp >= _rtDW -> LastMajorTimeB_ct ) && ( _rtDW ->
LastMajorTimeA_hp >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime (
S , 0 ) - _rtDW -> LastMajorTimeB_ct ; rtb_B_40_3_0 = _rtDW -> PrevYB_au ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_hp ;
rtb_B_40_3_0 = _rtDW -> PrevYA_l ; } riseValLimit = deltaT * _rtP -> P_362 ;
rateLimiterRate = _rtB -> B_40_418_0 - rtb_B_40_3_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_40_419_0 = rtb_B_40_3_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_363 ; if ( rateLimiterRate < deltaT ) { _rtB ->
B_40_419_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB -> B_40_419_0 = _rtB ->
B_40_418_0 ; } } } _rtB -> B_40_420_0 = _rtP -> P_364 * _rtB -> B_40_419_0 ;
_rtB -> B_40_421_0 = _rtP -> P_365 * _rtB -> B_40_420_0 ;
vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model2/Data Store Read" ,
0 ) ; _rtB -> B_40_422_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_la != 0 )
{ _rtX -> vv0at_CSTATE_c4 = _rtB -> B_40_422_0 ; } _rtB -> B_40_423_0 = _rtX
-> vv0at_CSTATE_c4 ; _rtB -> B_40_424_0 = muDoubleScalarAbs ( _rtB ->
B_40_423_0 ) ; _rtB -> B_40_425_0 = _rtP -> P_366 [ 0 ] ; _rtB -> B_40_425_0
= _rtB -> B_40_425_0 * _rtB -> B_40_424_0 + _rtP -> P_366 [ 1 ] ; _rtB ->
B_40_425_0 = _rtB -> B_40_425_0 * _rtB -> B_40_424_0 + _rtP -> P_366 [ 2 ] ;
_rtB -> B_40_426_0 = _rtP -> P_367 * _rtB -> B_40_425_0 ; _rtB -> B_40_427_0
= _rtB -> B_40_27_0_j * _rtB -> B_40_426_0 ; _rtB -> B_40_428_0 = _rtB ->
B_40_421_0 - _rtB -> B_40_427_0 ; if ( _rtB -> B_40_428_0 > _rtB ->
B_40_117_0_m ) { _rtB -> B_40_431_0 = _rtB -> B_40_117_0_m ; } else { if (
_rtB -> B_40_428_0 < _rtB -> B_40_118_0_m ) { _rtB -> B_22_1_0 = _rtB ->
B_40_118_0_m ; } else { _rtB -> B_22_1_0 = _rtB -> B_40_428_0 ; } _rtB ->
B_40_431_0 = _rtB -> B_22_1_0 ; } { real_T * * uBuffer = ( real_T * * ) &
_rtDW -> TransportDelay_PWORK_pz . TUbufferPtrs [ 0 ] ; real_T * * tBuffer =
( real_T * * ) & _rtDW -> TransportDelay_PWORK_pz . TUbufferPtrs [ 1 ] ;
real_T simTime = ssGetT ( S ) ; real_T tMinusDelay = simTime - _rtP -> P_368
; _rtB -> B_40_432_0 = Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay ,
0.0 , * tBuffer , * uBuffer , _rtDW -> TransportDelay_IWORK_f0 .
CircularBufSize , & _rtDW -> TransportDelay_IWORK_f0 . Last , _rtDW ->
TransportDelay_IWORK_f0 . Tail , _rtDW -> TransportDelay_IWORK_f0 . Head ,
_rtP -> P_369 , 0 , ( boolean_T ) ( ssIsMinorTimeStep ( S ) && (
ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) ) ) ; } _rtB -> B_40_433_0 =
0.0 ; _rtB -> B_40_433_0 += _rtP -> P_371 * _rtX -> TransferFcn_CSTATE_g3 ;
_rtB -> B_40_433_0 += _rtP -> P_372 * _rtB -> B_40_432_0 ; if ( _rtB ->
B_40_433_0 > _rtP -> P_374 ) { _rtB -> B_40_434_0 = _rtB -> B_40_433_0 - _rtP
-> P_374 ; } else if ( _rtB -> B_40_433_0 >= _rtP -> P_373 ) { _rtB ->
B_40_434_0 = 0.0 ; } else { _rtB -> B_40_434_0 = _rtB -> B_40_433_0 - _rtP ->
P_373 ; } _rtB -> B_40_435_0 = _rtP -> P_375 * _rtB -> B_40_434_0 ; if ( _rtB
-> B_40_435_0 > _rtP -> P_376 ) { _rtB -> B_40_436_0 = _rtP -> P_376 ; } else
if ( _rtB -> B_40_435_0 < _rtP -> P_377 ) { _rtB -> B_40_436_0 = _rtP ->
P_377 ; } else { _rtB -> B_40_436_0 = _rtB -> B_40_435_0 ; } if ( ( _rtDW ->
LastMajorTimeA_it >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW -> LastMajorTimeB_f
>= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_40_437_0 = _rtB -> B_40_436_0 ; }
else { if ( ( ( _rtDW -> LastMajorTimeA_it < _rtDW -> LastMajorTimeB_f ) && (
_rtDW -> LastMajorTimeB_f < ssGetTaskTime ( S , 0 ) ) ) || ( ( _rtDW ->
LastMajorTimeA_it >= _rtDW -> LastMajorTimeB_f ) && ( _rtDW ->
LastMajorTimeA_it >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime (
S , 0 ) - _rtDW -> LastMajorTimeB_f ; rtb_B_40_3_0 = _rtDW -> PrevYB_fh ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_it ;
rtb_B_40_3_0 = _rtDW -> PrevYA_go ; } riseValLimit = deltaT * _rtP -> P_378 ;
rateLimiterRate = _rtB -> B_40_436_0 - rtb_B_40_3_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_40_437_0 = rtb_B_40_3_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_379 ; if ( rateLimiterRate < deltaT ) { _rtB ->
B_40_437_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB -> B_40_437_0 = _rtB ->
B_40_436_0 ; } } } _rtB -> B_40_438_0 = _rtP -> P_380 * _rtB -> B_40_437_0 ;
_rtB -> B_40_439_0 = _rtP -> P_381 * _rtB -> B_40_438_0 ;
vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model3/Data Store Read" ,
0 ) ; _rtB -> B_40_440_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_c3 != 0 )
{ _rtX -> vv0at_CSTATE_ba = _rtB -> B_40_440_0 ; } _rtB -> B_40_441_0 = _rtX
-> vv0at_CSTATE_ba ; _rtB -> B_40_442_0 = muDoubleScalarAbs ( _rtB ->
B_40_441_0 ) ; _rtB -> B_40_443_0 = _rtP -> P_382 [ 0 ] ; _rtB -> B_40_443_0
= _rtB -> B_40_443_0 * _rtB -> B_40_442_0 + _rtP -> P_382 [ 1 ] ; _rtB ->
B_40_443_0 = _rtB -> B_40_443_0 * _rtB -> B_40_442_0 + _rtP -> P_382 [ 2 ] ;
_rtB -> B_40_444_0 = _rtP -> P_383 * _rtB -> B_40_443_0 ; _rtB -> B_40_445_0
= _rtB -> B_40_28_0_d * _rtB -> B_40_444_0 ; _rtB -> B_40_446_0 = _rtB ->
B_40_439_0 - _rtB -> B_40_445_0 ; if ( _rtB -> B_40_446_0 > _rtB ->
B_40_120_0_c ) { _rtB -> B_40_449_0 = _rtB -> B_40_120_0_c ; } else { if (
_rtB -> B_40_446_0 < _rtB -> B_40_121_0_f ) { _rtB -> B_23_1_0 = _rtB ->
B_40_121_0_f ; } else { _rtB -> B_23_1_0 = _rtB -> B_40_446_0 ; } _rtB ->
B_40_449_0 = _rtB -> B_23_1_0 ; } { real_T * * uBuffer = ( real_T * * ) &
_rtDW -> TransportDelay_PWORK_k . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_k . TUbufferPtrs [ 1 ] ; real_T
simTime = ssGetT ( S ) ; real_T tMinusDelay = simTime - _rtP -> P_384 ; _rtB
-> B_40_450_0 = Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 ,
* tBuffer , * uBuffer , _rtDW -> TransportDelay_IWORK_pz . CircularBufSize ,
& _rtDW -> TransportDelay_IWORK_pz . Last , _rtDW -> TransportDelay_IWORK_pz
. Tail , _rtDW -> TransportDelay_IWORK_pz . Head , _rtP -> P_385 , 0 , (
boolean_T ) ( ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) ==
ssGetT ( S ) ) ) ) ; } _rtB -> B_40_451_0 = 0.0 ; _rtB -> B_40_451_0 += _rtP
-> P_387 * _rtX -> TransferFcn_CSTATE_jw ; _rtB -> B_40_451_0 += _rtP ->
P_388 * _rtB -> B_40_450_0 ; if ( _rtB -> B_40_451_0 > _rtP -> P_390 ) { _rtB
-> B_40_452_0 = _rtB -> B_40_451_0 - _rtP -> P_390 ; } else if ( _rtB ->
B_40_451_0 >= _rtP -> P_389 ) { _rtB -> B_40_452_0 = 0.0 ; } else { _rtB ->
B_40_452_0 = _rtB -> B_40_451_0 - _rtP -> P_389 ; } _rtB -> B_40_453_0 = _rtP
-> P_391 * _rtB -> B_40_452_0 ; if ( _rtB -> B_40_453_0 > _rtP -> P_392 ) {
_rtB -> B_40_454_0 = _rtP -> P_392 ; } else if ( _rtB -> B_40_453_0 < _rtP ->
P_393 ) { _rtB -> B_40_454_0 = _rtP -> P_393 ; } else { _rtB -> B_40_454_0 =
_rtB -> B_40_453_0 ; } if ( ( _rtDW -> LastMajorTimeA_j >= ssGetTaskTime ( S
, 0 ) ) && ( _rtDW -> LastMajorTimeB_b0 >= ssGetTaskTime ( S , 0 ) ) ) { _rtB
-> B_40_455_0 = _rtB -> B_40_454_0 ; } else { if ( ( ( _rtDW ->
LastMajorTimeA_j < _rtDW -> LastMajorTimeB_b0 ) && ( _rtDW ->
LastMajorTimeB_b0 < ssGetTaskTime ( S , 0 ) ) ) || ( ( _rtDW ->
LastMajorTimeA_j >= _rtDW -> LastMajorTimeB_b0 ) && ( _rtDW ->
LastMajorTimeA_j >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime (
S , 0 ) - _rtDW -> LastMajorTimeB_b0 ; rtb_B_40_3_0 = _rtDW -> PrevYB_p ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_j ;
rtb_B_40_3_0 = _rtDW -> PrevYA_n ; } riseValLimit = deltaT * _rtP -> P_394 ;
rateLimiterRate = _rtB -> B_40_454_0 - rtb_B_40_3_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_40_455_0 = rtb_B_40_3_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_395 ; if ( rateLimiterRate < deltaT ) { _rtB ->
B_40_455_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB -> B_40_455_0 = _rtB ->
B_40_454_0 ; } } } _rtB -> B_40_456_0 = _rtP -> P_396 * _rtB -> B_40_455_0 ;
_rtB -> B_40_457_0 = _rtP -> P_397 * _rtB -> B_40_456_0 ;
vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model4/Data Store Read" ,
0 ) ; _rtB -> B_40_458_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_i2 != 0 )
{ _rtX -> vv0at_CSTATE_cm = _rtB -> B_40_458_0 ; } _rtB -> B_40_459_0 = _rtX
-> vv0at_CSTATE_cm ; _rtB -> B_40_460_0 = muDoubleScalarAbs ( _rtB ->
B_40_459_0 ) ; _rtB -> B_40_461_0 = _rtP -> P_398 [ 0 ] ; _rtB -> B_40_461_0
= _rtB -> B_40_461_0 * _rtB -> B_40_460_0 + _rtP -> P_398 [ 1 ] ; _rtB ->
B_40_461_0 = _rtB -> B_40_461_0 * _rtB -> B_40_460_0 + _rtP -> P_398 [ 2 ] ;
_rtB -> B_40_462_0 = _rtP -> P_399 * _rtB -> B_40_461_0 ; _rtB -> B_40_463_0
= _rtB -> B_40_29_0_g * _rtB -> B_40_462_0 ; _rtB -> B_40_464_0 = _rtB ->
B_40_457_0 - _rtB -> B_40_463_0 ; if ( _rtB -> B_40_464_0 > _rtB ->
B_40_123_0 ) { _rtB -> B_40_467_0 = _rtB -> B_40_123_0 ; } else { if ( _rtB
-> B_40_464_0 < _rtB -> B_40_124_0 ) { _rtB -> B_24_1_0 = _rtB -> B_40_124_0
; } else { _rtB -> B_24_1_0 = _rtB -> B_40_464_0 ; } _rtB -> B_40_467_0 =
_rtB -> B_24_1_0 ; } { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_fl . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T
* * ) & _rtDW -> TransportDelay_PWORK_fl . TUbufferPtrs [ 1 ] ; real_T
simTime = ssGetT ( S ) ; real_T tMinusDelay = simTime - _rtP -> P_400 ; _rtB
-> B_40_468_0 = Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 ,
* tBuffer , * uBuffer , _rtDW -> TransportDelay_IWORK_bv . CircularBufSize ,
& _rtDW -> TransportDelay_IWORK_bv . Last , _rtDW -> TransportDelay_IWORK_bv
. Tail , _rtDW -> TransportDelay_IWORK_bv . Head , _rtP -> P_401 , 0 , (
boolean_T ) ( ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) ==
ssGetT ( S ) ) ) ) ; } _rtB -> B_40_469_0 = 0.0 ; _rtB -> B_40_469_0 += _rtP
-> P_403 * _rtX -> TransferFcn_CSTATE_pt ; _rtB -> B_40_469_0 += _rtP ->
P_404 * _rtB -> B_40_468_0 ; if ( _rtB -> B_40_469_0 > _rtP -> P_406 ) { _rtB
-> B_40_470_0 = _rtB -> B_40_469_0 - _rtP -> P_406 ; } else if ( _rtB ->
B_40_469_0 >= _rtP -> P_405 ) { _rtB -> B_40_470_0 = 0.0 ; } else { _rtB ->
B_40_470_0 = _rtB -> B_40_469_0 - _rtP -> P_405 ; } _rtB -> B_40_471_0 = _rtP
-> P_407 * _rtB -> B_40_470_0 ; if ( _rtB -> B_40_471_0 > _rtP -> P_408 ) {
_rtB -> B_40_472_0 = _rtP -> P_408 ; } else if ( _rtB -> B_40_471_0 < _rtP ->
P_409 ) { _rtB -> B_40_472_0 = _rtP -> P_409 ; } else { _rtB -> B_40_472_0 =
_rtB -> B_40_471_0 ; } if ( ( _rtDW -> LastMajorTimeA_c5 >= ssGetTaskTime ( S
, 0 ) ) && ( _rtDW -> LastMajorTimeB_cu >= ssGetTaskTime ( S , 0 ) ) ) { _rtB
-> B_40_473_0 = _rtB -> B_40_472_0 ; } else { if ( ( ( _rtDW ->
LastMajorTimeA_c5 < _rtDW -> LastMajorTimeB_cu ) && ( _rtDW ->
LastMajorTimeB_cu < ssGetTaskTime ( S , 0 ) ) ) || ( ( _rtDW ->
LastMajorTimeA_c5 >= _rtDW -> LastMajorTimeB_cu ) && ( _rtDW ->
LastMajorTimeA_c5 >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime (
S , 0 ) - _rtDW -> LastMajorTimeB_cu ; rtb_B_40_3_0 = _rtDW -> PrevYB_ll ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_c5 ;
rtb_B_40_3_0 = _rtDW -> PrevYA_ad ; } riseValLimit = deltaT * _rtP -> P_410 ;
rateLimiterRate = _rtB -> B_40_472_0 - rtb_B_40_3_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_40_473_0 = rtb_B_40_3_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_411 ; if ( rateLimiterRate < deltaT ) { _rtB ->
B_40_473_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB -> B_40_473_0 = _rtB ->
B_40_472_0 ; } } } _rtB -> B_40_474_0 = _rtP -> P_412 * _rtB -> B_40_473_0 ;
_rtB -> B_40_475_0 = _rtP -> P_413 * _rtB -> B_40_474_0 ;
vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model/Data Store Read" , 0
) ; _rtB -> B_40_476_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_he != 0 ) {
_rtX -> vv0at_CSTATE_lc = _rtB -> B_40_476_0 ; } _rtB -> B_40_477_0 = _rtX ->
vv0at_CSTATE_lc ; _rtB -> B_40_478_0 = muDoubleScalarAbs ( _rtB -> B_40_477_0
) ; _rtB -> B_40_479_0 = _rtP -> P_414 [ 0 ] ; _rtB -> B_40_479_0 = _rtB ->
B_40_479_0 * _rtB -> B_40_478_0 + _rtP -> P_414 [ 1 ] ; _rtB -> B_40_479_0 =
_rtB -> B_40_479_0 * _rtB -> B_40_478_0 + _rtP -> P_414 [ 2 ] ; _rtB ->
B_40_480_0 = _rtP -> P_415 * _rtB -> B_40_479_0 ; _rtB -> B_40_481_0 = _rtB
-> B_40_31_0_l * _rtB -> B_40_480_0 ; _rtB -> B_40_482_0 = _rtB -> B_40_475_0
- _rtB -> B_40_481_0 ; if ( _rtB -> B_40_482_0 > _rtB -> B_40_126_0_p ) {
_rtB -> B_40_485_0 = _rtB -> B_40_126_0_p ; } else { if ( _rtB -> B_40_482_0
< _rtB -> B_40_127_0_e ) { _rtB -> B_25_1_0 = _rtB -> B_40_127_0_e ; } else {
_rtB -> B_25_1_0 = _rtB -> B_40_482_0 ; } _rtB -> B_40_485_0 = _rtB ->
B_25_1_0 ; } { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_iy . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T
* * ) & _rtDW -> TransportDelay_PWORK_iy . TUbufferPtrs [ 1 ] ; real_T
simTime = ssGetT ( S ) ; real_T tMinusDelay = simTime - _rtP -> P_416 ; _rtB
-> B_40_486_0 = Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 ,
* tBuffer , * uBuffer , _rtDW -> TransportDelay_IWORK_m . CircularBufSize , &
_rtDW -> TransportDelay_IWORK_m . Last , _rtDW -> TransportDelay_IWORK_m .
Tail , _rtDW -> TransportDelay_IWORK_m . Head , _rtP -> P_417 , 0 , (
boolean_T ) ( ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) ==
ssGetT ( S ) ) ) ) ; } _rtB -> B_40_487_0 = 0.0 ; _rtB -> B_40_487_0 += _rtP
-> P_419 * _rtX -> TransferFcn_CSTATE_jn ; _rtB -> B_40_487_0 += _rtP ->
P_420 * _rtB -> B_40_486_0 ; if ( _rtB -> B_40_487_0 > _rtP -> P_422 ) { _rtB
-> B_40_488_0 = _rtB -> B_40_487_0 - _rtP -> P_422 ; } else if ( _rtB ->
B_40_487_0 >= _rtP -> P_421 ) { _rtB -> B_40_488_0 = 0.0 ; } else { _rtB ->
B_40_488_0 = _rtB -> B_40_487_0 - _rtP -> P_421 ; } _rtB -> B_40_489_0 = _rtP
-> P_423 * _rtB -> B_40_488_0 ; if ( _rtB -> B_40_489_0 > _rtP -> P_424 ) {
_rtB -> B_40_490_0 = _rtP -> P_424 ; } else if ( _rtB -> B_40_489_0 < _rtP ->
P_425 ) { _rtB -> B_40_490_0 = _rtP -> P_425 ; } else { _rtB -> B_40_490_0 =
_rtB -> B_40_489_0 ; } if ( ( _rtDW -> LastMajorTimeA_if >= ssGetTaskTime ( S
, 0 ) ) && ( _rtDW -> LastMajorTimeB_id >= ssGetTaskTime ( S , 0 ) ) ) { _rtB
-> B_40_491_0 = _rtB -> B_40_490_0 ; } else { if ( ( ( _rtDW ->
LastMajorTimeA_if < _rtDW -> LastMajorTimeB_id ) && ( _rtDW ->
LastMajorTimeB_id < ssGetTaskTime ( S , 0 ) ) ) || ( ( _rtDW ->
LastMajorTimeA_if >= _rtDW -> LastMajorTimeB_id ) && ( _rtDW ->
LastMajorTimeA_if >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime (
S , 0 ) - _rtDW -> LastMajorTimeB_id ; rtb_B_40_3_0 = _rtDW -> PrevYB_e3 ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_if ;
rtb_B_40_3_0 = _rtDW -> PrevYA_e5 ; } riseValLimit = deltaT * _rtP -> P_426 ;
rateLimiterRate = _rtB -> B_40_490_0 - rtb_B_40_3_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_40_491_0 = rtb_B_40_3_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_427 ; if ( rateLimiterRate < deltaT ) { _rtB ->
B_40_491_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB -> B_40_491_0 = _rtB ->
B_40_490_0 ; } } } _rtB -> B_40_492_0 = _rtP -> P_428 * _rtB -> B_40_491_0 ;
_rtB -> B_40_493_0 = _rtP -> P_429 * _rtB -> B_40_492_0 ;
vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model1/Data Store Read" ,
0 ) ; _rtB -> B_40_494_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_hv != 0 )
{ _rtX -> vv0at_CSTATE_n1 = _rtB -> B_40_494_0 ; } _rtB -> B_40_495_0 = _rtX
-> vv0at_CSTATE_n1 ; _rtB -> B_40_496_0 = muDoubleScalarAbs ( _rtB ->
B_40_495_0 ) ; _rtB -> B_40_497_0 = _rtP -> P_430 [ 0 ] ; _rtB -> B_40_497_0
= _rtB -> B_40_497_0 * _rtB -> B_40_496_0 + _rtP -> P_430 [ 1 ] ; _rtB ->
B_40_497_0 = _rtB -> B_40_497_0 * _rtB -> B_40_496_0 + _rtP -> P_430 [ 2 ] ;
_rtB -> B_40_498_0 = _rtP -> P_431 * _rtB -> B_40_497_0 ; _rtB -> B_40_499_0
= _rtB -> B_40_32_0_d * _rtB -> B_40_498_0 ; _rtB -> B_40_500_0 = _rtB ->
B_40_493_0 - _rtB -> B_40_499_0 ; if ( _rtB -> B_40_500_0 > _rtB ->
B_40_129_0_o ) { _rtB -> B_40_503_0 = _rtB -> B_40_129_0_o ; } else { if (
_rtB -> B_40_500_0 < _rtB -> B_40_130_0_h ) { _rtB -> B_26_1_0 = _rtB ->
B_40_130_0_h ; } else { _rtB -> B_26_1_0 = _rtB -> B_40_500_0 ; } _rtB ->
B_40_503_0 = _rtB -> B_26_1_0 ; } { real_T * * uBuffer = ( real_T * * ) &
_rtDW -> TransportDelay_PWORK_iu . TUbufferPtrs [ 0 ] ; real_T * * tBuffer =
( real_T * * ) & _rtDW -> TransportDelay_PWORK_iu . TUbufferPtrs [ 1 ] ;
real_T simTime = ssGetT ( S ) ; real_T tMinusDelay = simTime - _rtP -> P_432
; _rtB -> B_40_504_0 = Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay ,
0.0 , * tBuffer , * uBuffer , _rtDW -> TransportDelay_IWORK_h .
CircularBufSize , & _rtDW -> TransportDelay_IWORK_h . Last , _rtDW ->
TransportDelay_IWORK_h . Tail , _rtDW -> TransportDelay_IWORK_h . Head , _rtP
-> P_433 , 0 , ( boolean_T ) ( ssIsMinorTimeStep ( S ) && (
ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) ) ) ; } _rtB -> B_40_505_0 =
0.0 ; _rtB -> B_40_505_0 += _rtP -> P_435 * _rtX -> TransferFcn_CSTATE_mu ;
_rtB -> B_40_505_0 += _rtP -> P_436 * _rtB -> B_40_504_0 ; if ( _rtB ->
B_40_505_0 > _rtP -> P_438 ) { _rtB -> B_40_506_0 = _rtB -> B_40_505_0 - _rtP
-> P_438 ; } else if ( _rtB -> B_40_505_0 >= _rtP -> P_437 ) { _rtB ->
B_40_506_0 = 0.0 ; } else { _rtB -> B_40_506_0 = _rtB -> B_40_505_0 - _rtP ->
P_437 ; } _rtB -> B_40_507_0 = _rtP -> P_439 * _rtB -> B_40_506_0 ; if ( _rtB
-> B_40_507_0 > _rtP -> P_440 ) { _rtB -> B_40_508_0 = _rtP -> P_440 ; } else
if ( _rtB -> B_40_507_0 < _rtP -> P_441 ) { _rtB -> B_40_508_0 = _rtP ->
P_441 ; } else { _rtB -> B_40_508_0 = _rtB -> B_40_507_0 ; } if ( ( _rtDW ->
LastMajorTimeA_n1 >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW ->
LastMajorTimeB_e5 >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_40_509_0 = _rtB
-> B_40_508_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_n1 < _rtDW ->
LastMajorTimeB_e5 ) && ( _rtDW -> LastMajorTimeB_e5 < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_n1 >= _rtDW -> LastMajorTimeB_e5 ) && (
_rtDW -> LastMajorTimeA_n1 >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_e5 ; rtb_B_40_3_0 = _rtDW
-> PrevYB_j ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_n1 ; rtb_B_40_3_0 = _rtDW -> PrevYA_m ; } riseValLimit =
deltaT * _rtP -> P_442 ; rateLimiterRate = _rtB -> B_40_508_0 - rtb_B_40_3_0
; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_40_509_0 = rtb_B_40_3_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_443 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_40_509_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB ->
B_40_509_0 = _rtB -> B_40_508_0 ; } } } _rtB -> B_40_510_0 = _rtP -> P_444 *
_rtB -> B_40_509_0 ; _rtB -> B_40_511_0 = _rtP -> P_445 * _rtB -> B_40_510_0
; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model2/Data Store Read" ,
0 ) ; _rtB -> B_40_512_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_g != 0 ) {
_rtX -> vv0at_CSTATE_nu = _rtB -> B_40_512_0 ; } _rtB -> B_40_513_0 = _rtX ->
vv0at_CSTATE_nu ; _rtB -> B_40_514_0 = muDoubleScalarAbs ( _rtB -> B_40_513_0
) ; _rtB -> B_40_515_0 = _rtP -> P_446 [ 0 ] ; _rtB -> B_40_515_0 = _rtB ->
B_40_515_0 * _rtB -> B_40_514_0 + _rtP -> P_446 [ 1 ] ; _rtB -> B_40_515_0 =
_rtB -> B_40_515_0 * _rtB -> B_40_514_0 + _rtP -> P_446 [ 2 ] ; _rtB ->
B_40_516_0 = _rtP -> P_447 * _rtB -> B_40_515_0 ; _rtB -> B_40_517_0 = _rtB
-> B_40_33_0 * _rtB -> B_40_516_0 ; _rtB -> B_40_518_0 = _rtB -> B_40_511_0 -
_rtB -> B_40_517_0 ; if ( _rtB -> B_40_518_0 > _rtB -> B_40_132_0_l ) { _rtB
-> B_40_521_0 = _rtB -> B_40_132_0_l ; } else { if ( _rtB -> B_40_518_0 <
_rtB -> B_40_133_0_h ) { _rtB -> B_27_1_0 = _rtB -> B_40_133_0_h ; } else {
_rtB -> B_27_1_0 = _rtB -> B_40_518_0 ; } _rtB -> B_40_521_0 = _rtB ->
B_27_1_0 ; } { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_a . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T *
* ) & _rtDW -> TransportDelay_PWORK_a . TUbufferPtrs [ 1 ] ; real_T simTime =
ssGetT ( S ) ; real_T tMinusDelay = simTime - _rtP -> P_448 ; _rtB ->
B_40_522_0 = Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , *
tBuffer , * uBuffer , _rtDW -> TransportDelay_IWORK_i0 . CircularBufSize , &
_rtDW -> TransportDelay_IWORK_i0 . Last , _rtDW -> TransportDelay_IWORK_i0 .
Tail , _rtDW -> TransportDelay_IWORK_i0 . Head , _rtP -> P_449 , 0 , (
boolean_T ) ( ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) ==
ssGetT ( S ) ) ) ) ; } _rtB -> B_40_523_0 = 0.0 ; _rtB -> B_40_523_0 += _rtP
-> P_451 * _rtX -> TransferFcn_CSTATE_cr ; _rtB -> B_40_523_0 += _rtP ->
P_452 * _rtB -> B_40_522_0 ; if ( _rtB -> B_40_523_0 > _rtP -> P_454 ) { _rtB
-> B_40_524_0 = _rtB -> B_40_523_0 - _rtP -> P_454 ; } else if ( _rtB ->
B_40_523_0 >= _rtP -> P_453 ) { _rtB -> B_40_524_0 = 0.0 ; } else { _rtB ->
B_40_524_0 = _rtB -> B_40_523_0 - _rtP -> P_453 ; } _rtB -> B_40_525_0 = _rtP
-> P_455 * _rtB -> B_40_524_0 ; if ( _rtB -> B_40_525_0 > _rtP -> P_456 ) {
_rtB -> B_40_526_0 = _rtP -> P_456 ; } else if ( _rtB -> B_40_525_0 < _rtP ->
P_457 ) { _rtB -> B_40_526_0 = _rtP -> P_457 ; } else { _rtB -> B_40_526_0 =
_rtB -> B_40_525_0 ; } if ( ( _rtDW -> LastMajorTimeA_hk >= ssGetTaskTime ( S
, 0 ) ) && ( _rtDW -> LastMajorTimeB_nm >= ssGetTaskTime ( S , 0 ) ) ) { _rtB
-> B_40_527_0 = _rtB -> B_40_526_0 ; } else { if ( ( ( _rtDW ->
LastMajorTimeA_hk < _rtDW -> LastMajorTimeB_nm ) && ( _rtDW ->
LastMajorTimeB_nm < ssGetTaskTime ( S , 0 ) ) ) || ( ( _rtDW ->
LastMajorTimeA_hk >= _rtDW -> LastMajorTimeB_nm ) && ( _rtDW ->
LastMajorTimeA_hk >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime (
S , 0 ) - _rtDW -> LastMajorTimeB_nm ; rtb_B_40_3_0 = _rtDW -> PrevYB_d2 ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_hk ;
rtb_B_40_3_0 = _rtDW -> PrevYA_dn ; } riseValLimit = deltaT * _rtP -> P_458 ;
rateLimiterRate = _rtB -> B_40_526_0 - rtb_B_40_3_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_40_527_0 = rtb_B_40_3_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_459 ; if ( rateLimiterRate < deltaT ) { _rtB ->
B_40_527_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB -> B_40_527_0 = _rtB ->
B_40_526_0 ; } } } _rtB -> B_40_528_0 = _rtP -> P_460 * _rtB -> B_40_527_0 ;
_rtB -> B_40_529_0 = _rtP -> P_461 * _rtB -> B_40_528_0 ;
vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model3/Data Store Read" ,
0 ) ; _rtB -> B_40_530_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_fz != 0 )
{ _rtX -> vv0at_CSTATE_ln = _rtB -> B_40_530_0 ; } _rtB -> B_40_531_0 = _rtX
-> vv0at_CSTATE_ln ; _rtB -> B_40_532_0 = muDoubleScalarAbs ( _rtB ->
B_40_531_0 ) ; _rtB -> B_40_533_0 = _rtP -> P_462 [ 0 ] ; _rtB -> B_40_533_0
= _rtB -> B_40_533_0 * _rtB -> B_40_532_0 + _rtP -> P_462 [ 1 ] ; _rtB ->
B_40_533_0 = _rtB -> B_40_533_0 * _rtB -> B_40_532_0 + _rtP -> P_462 [ 2 ] ;
_rtB -> B_40_534_0 = _rtP -> P_463 * _rtB -> B_40_533_0 ; _rtB -> B_40_535_0
= _rtB -> B_40_34_0 * _rtB -> B_40_534_0 ; _rtB -> B_40_536_0 = _rtB ->
B_40_529_0 - _rtB -> B_40_535_0 ; if ( _rtB -> B_40_536_0 > _rtB ->
B_40_135_0_m ) { _rtB -> B_40_539_0 = _rtB -> B_40_135_0_m ; } else { if (
_rtB -> B_40_536_0 < _rtB -> B_40_136_0_m ) { _rtB -> B_28_1_0 = _rtB ->
B_40_136_0_m ; } else { _rtB -> B_28_1_0 = _rtB -> B_40_536_0 ; } _rtB ->
B_40_539_0 = _rtB -> B_28_1_0 ; } { real_T * * uBuffer = ( real_T * * ) &
_rtDW -> TransportDelay_PWORK_nb . TUbufferPtrs [ 0 ] ; real_T * * tBuffer =
( real_T * * ) & _rtDW -> TransportDelay_PWORK_nb . TUbufferPtrs [ 1 ] ;
real_T simTime = ssGetT ( S ) ; real_T tMinusDelay = simTime - _rtP -> P_464
; _rtB -> B_40_540_0 = Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay ,
0.0 , * tBuffer , * uBuffer , _rtDW -> TransportDelay_IWORK_iy .
CircularBufSize , & _rtDW -> TransportDelay_IWORK_iy . Last , _rtDW ->
TransportDelay_IWORK_iy . Tail , _rtDW -> TransportDelay_IWORK_iy . Head ,
_rtP -> P_465 , 0 , ( boolean_T ) ( ssIsMinorTimeStep ( S ) && (
ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) ) ) ; } _rtB -> B_40_541_0 =
0.0 ; _rtB -> B_40_541_0 += _rtP -> P_467 * _rtX -> TransferFcn_CSTATE_h ;
_rtB -> B_40_541_0 += _rtP -> P_468 * _rtB -> B_40_540_0 ; if ( _rtB ->
B_40_541_0 > _rtP -> P_470 ) { _rtB -> B_40_542_0 = _rtB -> B_40_541_0 - _rtP
-> P_470 ; } else if ( _rtB -> B_40_541_0 >= _rtP -> P_469 ) { _rtB ->
B_40_542_0 = 0.0 ; } else { _rtB -> B_40_542_0 = _rtB -> B_40_541_0 - _rtP ->
P_469 ; } _rtB -> B_40_543_0 = _rtP -> P_471 * _rtB -> B_40_542_0 ; if ( _rtB
-> B_40_543_0 > _rtP -> P_472 ) { _rtB -> B_40_544_0 = _rtP -> P_472 ; } else
if ( _rtB -> B_40_543_0 < _rtP -> P_473 ) { _rtB -> B_40_544_0 = _rtP ->
P_473 ; } else { _rtB -> B_40_544_0 = _rtB -> B_40_543_0 ; } if ( ( _rtDW ->
LastMajorTimeA_bx >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW ->
LastMajorTimeB_l1 >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_40_545_0 = _rtB
-> B_40_544_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_bx < _rtDW ->
LastMajorTimeB_l1 ) && ( _rtDW -> LastMajorTimeB_l1 < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_bx >= _rtDW -> LastMajorTimeB_l1 ) && (
_rtDW -> LastMajorTimeA_bx >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_l1 ; rtb_B_40_3_0 = _rtDW
-> PrevYB_m1 ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_bx ; rtb_B_40_3_0 = _rtDW -> PrevYA_ha ; } riseValLimit =
deltaT * _rtP -> P_474 ; rateLimiterRate = _rtB -> B_40_544_0 - rtb_B_40_3_0
; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_40_545_0 = rtb_B_40_3_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_475 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_40_545_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB ->
B_40_545_0 = _rtB -> B_40_544_0 ; } } } _rtB -> B_40_546_0 = _rtP -> P_476 *
_rtB -> B_40_545_0 ; _rtB -> B_40_547_0 = _rtP -> P_477 * _rtB -> B_40_546_0
; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model4/Data Store Read" ,
0 ) ; _rtB -> B_40_548_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_c2 != 0 )
{ _rtX -> vv0at_CSTATE_fh = _rtB -> B_40_548_0 ; } _rtB -> B_40_549_0 = _rtX
-> vv0at_CSTATE_fh ; _rtB -> B_40_550_0 = muDoubleScalarAbs ( _rtB ->
B_40_549_0 ) ; _rtB -> B_40_551_0 = _rtP -> P_478 [ 0 ] ; _rtB -> B_40_551_0
= _rtB -> B_40_551_0 * _rtB -> B_40_550_0 + _rtP -> P_478 [ 1 ] ; _rtB ->
B_40_551_0 = _rtB -> B_40_551_0 * _rtB -> B_40_550_0 + _rtP -> P_478 [ 2 ] ;
_rtB -> B_40_552_0 = _rtP -> P_479 * _rtB -> B_40_551_0 ; _rtB -> B_40_553_0
= _rtB -> B_40_35_0_d * _rtB -> B_40_552_0 ; _rtB -> B_40_554_0 = _rtB ->
B_40_547_0 - _rtB -> B_40_553_0 ; if ( _rtB -> B_40_554_0 > _rtB ->
B_40_138_0_h ) { _rtB -> B_40_557_0 = _rtB -> B_40_138_0_h ; } else { if (
_rtB -> B_40_554_0 < _rtB -> B_40_139_0_c ) { _rtB -> B_29_1_0 = _rtB ->
B_40_139_0_c ; } else { _rtB -> B_29_1_0 = _rtB -> B_40_554_0 ; } _rtB ->
B_40_557_0 = _rtB -> B_29_1_0 ; } { real_T * * uBuffer = ( real_T * * ) &
_rtDW -> TransportDelay_PWORK_o . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_o . TUbufferPtrs [ 1 ] ; real_T
simTime = ssGetT ( S ) ; real_T tMinusDelay = simTime - _rtP -> P_480 ; _rtB
-> B_40_558_0 = Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 ,
* tBuffer , * uBuffer , _rtDW -> TransportDelay_IWORK_if . CircularBufSize ,
& _rtDW -> TransportDelay_IWORK_if . Last , _rtDW -> TransportDelay_IWORK_if
. Tail , _rtDW -> TransportDelay_IWORK_if . Head , _rtP -> P_481 , 0 , (
boolean_T ) ( ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) ==
ssGetT ( S ) ) ) ) ; } _rtB -> B_40_559_0 = 0.0 ; _rtB -> B_40_559_0 += _rtP
-> P_483 * _rtX -> TransferFcn_CSTATE_cp ; _rtB -> B_40_559_0 += _rtP ->
P_484 * _rtB -> B_40_558_0 ; if ( _rtB -> B_40_559_0 > _rtP -> P_486 ) { _rtB
-> B_40_560_0 = _rtB -> B_40_559_0 - _rtP -> P_486 ; } else if ( _rtB ->
B_40_559_0 >= _rtP -> P_485 ) { _rtB -> B_40_560_0 = 0.0 ; } else { _rtB ->
B_40_560_0 = _rtB -> B_40_559_0 - _rtP -> P_485 ; } _rtB -> B_40_561_0 = _rtP
-> P_487 * _rtB -> B_40_560_0 ; if ( _rtB -> B_40_561_0 > _rtP -> P_488 ) {
_rtB -> B_40_562_0 = _rtP -> P_488 ; } else if ( _rtB -> B_40_561_0 < _rtP ->
P_489 ) { _rtB -> B_40_562_0 = _rtP -> P_489 ; } else { _rtB -> B_40_562_0 =
_rtB -> B_40_561_0 ; } if ( ( _rtDW -> LastMajorTimeA_m >= ssGetTaskTime ( S
, 0 ) ) && ( _rtDW -> LastMajorTimeB_hw >= ssGetTaskTime ( S , 0 ) ) ) { _rtB
-> B_40_563_0 = _rtB -> B_40_562_0 ; } else { if ( ( ( _rtDW ->
LastMajorTimeA_m < _rtDW -> LastMajorTimeB_hw ) && ( _rtDW ->
LastMajorTimeB_hw < ssGetTaskTime ( S , 0 ) ) ) || ( ( _rtDW ->
LastMajorTimeA_m >= _rtDW -> LastMajorTimeB_hw ) && ( _rtDW ->
LastMajorTimeA_m >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime (
S , 0 ) - _rtDW -> LastMajorTimeB_hw ; rtb_B_40_3_0 = _rtDW -> PrevYB_ec ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_m ;
rtb_B_40_3_0 = _rtDW -> PrevYA_h2 ; } riseValLimit = deltaT * _rtP -> P_490 ;
rateLimiterRate = _rtB -> B_40_562_0 - rtb_B_40_3_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_40_563_0 = rtb_B_40_3_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_491 ; if ( rateLimiterRate < deltaT ) { _rtB ->
B_40_563_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB -> B_40_563_0 = _rtB ->
B_40_562_0 ; } } } _rtB -> B_40_564_0 = _rtP -> P_492 * _rtB -> B_40_563_0 ;
_rtB -> B_40_565_0 = _rtP -> P_493 * _rtB -> B_40_564_0 ;
vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model/Data Store Read" , 0
) ; _rtB -> B_40_566_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_ee != 0 ) {
_rtX -> vv0at_CSTATE_cx = _rtB -> B_40_566_0 ; } _rtB -> B_40_567_0 = _rtX ->
vv0at_CSTATE_cx ; _rtB -> B_40_568_0 = muDoubleScalarAbs ( _rtB -> B_40_567_0
) ; _rtB -> B_40_569_0 = _rtP -> P_494 [ 0 ] ; _rtB -> B_40_569_0 = _rtB ->
B_40_569_0 * _rtB -> B_40_568_0 + _rtP -> P_494 [ 1 ] ; _rtB -> B_40_569_0 =
_rtB -> B_40_569_0 * _rtB -> B_40_568_0 + _rtP -> P_494 [ 2 ] ; _rtB ->
B_40_570_0 = _rtP -> P_495 * _rtB -> B_40_569_0 ; _rtB -> B_40_571_0 = _rtB
-> B_40_37_0_l * _rtB -> B_40_570_0 ; _rtB -> B_40_572_0 = _rtB -> B_40_565_0
- _rtB -> B_40_571_0 ; if ( _rtB -> B_40_572_0 > _rtB -> B_40_141_0 ) { _rtB
-> B_40_575_0 = _rtB -> B_40_141_0 ; } else { if ( _rtB -> B_40_572_0 < _rtB
-> B_40_142_0 ) { _rtB -> B_30_1_0 = _rtB -> B_40_142_0 ; } else { _rtB ->
B_30_1_0 = _rtB -> B_40_572_0 ; } _rtB -> B_40_575_0 = _rtB -> B_30_1_0 ; } {
real_T * * uBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_a5 .
TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_a5 . TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S )
; real_T tMinusDelay = simTime - _rtP -> P_496 ; _rtB -> B_40_576_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_dl . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_dl . Last , _rtDW -> TransportDelay_IWORK_dl . Tail ,
_rtDW -> TransportDelay_IWORK_dl . Head , _rtP -> P_497 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_40_577_0 = 0.0 ; _rtB -> B_40_577_0 += _rtP -> P_499 * _rtX
-> TransferFcn_CSTATE_ji ; _rtB -> B_40_577_0 += _rtP -> P_500 * _rtB ->
B_40_576_0 ; if ( _rtB -> B_40_577_0 > _rtP -> P_502 ) { _rtB -> B_40_578_0 =
_rtB -> B_40_577_0 - _rtP -> P_502 ; } else if ( _rtB -> B_40_577_0 >= _rtP
-> P_501 ) { _rtB -> B_40_578_0 = 0.0 ; } else { _rtB -> B_40_578_0 = _rtB ->
B_40_577_0 - _rtP -> P_501 ; } _rtB -> B_40_579_0 = _rtP -> P_503 * _rtB ->
B_40_578_0 ; if ( _rtB -> B_40_579_0 > _rtP -> P_504 ) { _rtB -> B_40_580_0 =
_rtP -> P_504 ; } else if ( _rtB -> B_40_579_0 < _rtP -> P_505 ) { _rtB ->
B_40_580_0 = _rtP -> P_505 ; } else { _rtB -> B_40_580_0 = _rtB -> B_40_579_0
; } if ( ( _rtDW -> LastMajorTimeA_bu >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW
-> LastMajorTimeB_bj >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_40_581_0 =
_rtB -> B_40_580_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_bu < _rtDW ->
LastMajorTimeB_bj ) && ( _rtDW -> LastMajorTimeB_bj < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_bu >= _rtDW -> LastMajorTimeB_bj ) && (
_rtDW -> LastMajorTimeA_bu >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_bj ; rtb_B_40_3_0 = _rtDW
-> PrevYB_le ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_bu ; rtb_B_40_3_0 = _rtDW -> PrevYA_oi ; } riseValLimit =
deltaT * _rtP -> P_506 ; rateLimiterRate = _rtB -> B_40_580_0 - rtb_B_40_3_0
; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_40_581_0 = rtb_B_40_3_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_507 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_40_581_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB ->
B_40_581_0 = _rtB -> B_40_580_0 ; } } } _rtB -> B_40_582_0 = _rtP -> P_508 *
_rtB -> B_40_581_0 ; _rtB -> B_40_583_0 = _rtP -> P_509 * _rtB -> B_40_582_0
; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model1/Data Store Read" ,
0 ) ; _rtB -> B_40_584_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_e1 != 0 )
{ _rtX -> vv0at_CSTATE_pp = _rtB -> B_40_584_0 ; } _rtB -> B_40_585_0 = _rtX
-> vv0at_CSTATE_pp ; _rtB -> B_40_586_0 = muDoubleScalarAbs ( _rtB ->
B_40_585_0 ) ; _rtB -> B_40_587_0 = _rtP -> P_510 [ 0 ] ; _rtB -> B_40_587_0
= _rtB -> B_40_587_0 * _rtB -> B_40_586_0 + _rtP -> P_510 [ 1 ] ; _rtB ->
B_40_587_0 = _rtB -> B_40_587_0 * _rtB -> B_40_586_0 + _rtP -> P_510 [ 2 ] ;
_rtB -> B_40_588_0 = _rtP -> P_511 * _rtB -> B_40_587_0 ; _rtB -> B_40_589_0
= _rtB -> B_40_38_0_o * _rtB -> B_40_588_0 ; _rtB -> B_40_590_0 = _rtB ->
B_40_583_0 - _rtB -> B_40_589_0 ; if ( _rtB -> B_40_590_0 > _rtB ->
B_40_144_0_k ) { _rtB -> B_40_593_0 = _rtB -> B_40_144_0_k ; } else { if (
_rtB -> B_40_590_0 < _rtB -> B_40_145_0_p ) { _rtB -> B_31_1_0 = _rtB ->
B_40_145_0_p ; } else { _rtB -> B_31_1_0 = _rtB -> B_40_590_0 ; } _rtB ->
B_40_593_0 = _rtB -> B_31_1_0 ; } { real_T * * uBuffer = ( real_T * * ) &
_rtDW -> TransportDelay_PWORK_ic . TUbufferPtrs [ 0 ] ; real_T * * tBuffer =
( real_T * * ) & _rtDW -> TransportDelay_PWORK_ic . TUbufferPtrs [ 1 ] ;
real_T simTime = ssGetT ( S ) ; real_T tMinusDelay = simTime - _rtP -> P_512
; _rtB -> B_40_594_0 = Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay ,
0.0 , * tBuffer , * uBuffer , _rtDW -> TransportDelay_IWORK_i2 .
CircularBufSize , & _rtDW -> TransportDelay_IWORK_i2 . Last , _rtDW ->
TransportDelay_IWORK_i2 . Tail , _rtDW -> TransportDelay_IWORK_i2 . Head ,
_rtP -> P_513 , 0 , ( boolean_T ) ( ssIsMinorTimeStep ( S ) && (
ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) ) ) ; } _rtB -> B_40_595_0 =
0.0 ; _rtB -> B_40_595_0 += _rtP -> P_515 * _rtX -> TransferFcn_CSTATE_hl ;
_rtB -> B_40_595_0 += _rtP -> P_516 * _rtB -> B_40_594_0 ; if ( _rtB ->
B_40_595_0 > _rtP -> P_518 ) { _rtB -> B_40_596_0 = _rtB -> B_40_595_0 - _rtP
-> P_518 ; } else if ( _rtB -> B_40_595_0 >= _rtP -> P_517 ) { _rtB ->
B_40_596_0 = 0.0 ; } else { _rtB -> B_40_596_0 = _rtB -> B_40_595_0 - _rtP ->
P_517 ; } _rtB -> B_40_597_0 = _rtP -> P_519 * _rtB -> B_40_596_0 ; if ( _rtB
-> B_40_597_0 > _rtP -> P_520 ) { _rtB -> B_40_598_0 = _rtP -> P_520 ; } else
if ( _rtB -> B_40_597_0 < _rtP -> P_521 ) { _rtB -> B_40_598_0 = _rtP ->
P_521 ; } else { _rtB -> B_40_598_0 = _rtB -> B_40_597_0 ; } if ( ( _rtDW ->
LastMajorTimeA_m2 >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW ->
LastMajorTimeB_np >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_40_599_0 = _rtB
-> B_40_598_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_m2 < _rtDW ->
LastMajorTimeB_np ) && ( _rtDW -> LastMajorTimeB_np < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_m2 >= _rtDW -> LastMajorTimeB_np ) && (
_rtDW -> LastMajorTimeA_m2 >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_np ; rtb_B_40_3_0 = _rtDW
-> PrevYB_h ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_m2 ; rtb_B_40_3_0 = _rtDW -> PrevYA_py ; } riseValLimit =
deltaT * _rtP -> P_522 ; rateLimiterRate = _rtB -> B_40_598_0 - rtb_B_40_3_0
; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_40_599_0 = rtb_B_40_3_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_523 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_40_599_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB ->
B_40_599_0 = _rtB -> B_40_598_0 ; } } } _rtB -> B_40_600_0 = _rtP -> P_524 *
_rtB -> B_40_599_0 ; _rtB -> B_40_601_0 = _rtP -> P_525 * _rtB -> B_40_600_0
; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model2/Data Store Read" ,
0 ) ; _rtB -> B_40_602_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_l4 != 0 )
{ _rtX -> vv0at_CSTATE_i = _rtB -> B_40_602_0 ; } _rtB -> B_40_603_0 = _rtX
-> vv0at_CSTATE_i ; _rtB -> B_40_604_0 = muDoubleScalarAbs ( _rtB ->
B_40_603_0 ) ; _rtB -> B_40_605_0 = _rtP -> P_526 [ 0 ] ; _rtB -> B_40_605_0
= _rtB -> B_40_605_0 * _rtB -> B_40_604_0 + _rtP -> P_526 [ 1 ] ; _rtB ->
B_40_605_0 = _rtB -> B_40_605_0 * _rtB -> B_40_604_0 + _rtP -> P_526 [ 2 ] ;
_rtB -> B_40_606_0 = _rtP -> P_527 * _rtB -> B_40_605_0 ; _rtB -> B_40_607_0
= _rtB -> B_40_39_0_b * _rtB -> B_40_606_0 ; _rtB -> B_40_608_0 = _rtB ->
B_40_601_0 - _rtB -> B_40_607_0 ; if ( _rtB -> B_40_608_0 > _rtB ->
B_40_147_0_p ) { _rtB -> B_40_611_0 = _rtB -> B_40_147_0_p ; } else { if (
_rtB -> B_40_608_0 < _rtB -> B_40_148_0_p ) { _rtB -> B_32_1_0 = _rtB ->
B_40_148_0_p ; } else { _rtB -> B_32_1_0 = _rtB -> B_40_608_0 ; } _rtB ->
B_40_611_0 = _rtB -> B_32_1_0 ; } { real_T * * uBuffer = ( real_T * * ) &
_rtDW -> TransportDelay_PWORK_lz . TUbufferPtrs [ 0 ] ; real_T * * tBuffer =
( real_T * * ) & _rtDW -> TransportDelay_PWORK_lz . TUbufferPtrs [ 1 ] ;
real_T simTime = ssGetT ( S ) ; real_T tMinusDelay = simTime - _rtP -> P_528
; _rtB -> B_40_612_0 = Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay ,
0.0 , * tBuffer , * uBuffer , _rtDW -> TransportDelay_IWORK_ab .
CircularBufSize , & _rtDW -> TransportDelay_IWORK_ab . Last , _rtDW ->
TransportDelay_IWORK_ab . Tail , _rtDW -> TransportDelay_IWORK_ab . Head ,
_rtP -> P_529 , 0 , ( boolean_T ) ( ssIsMinorTimeStep ( S ) && (
ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) ) ) ; } _rtB -> B_40_613_0 =
0.0 ; _rtB -> B_40_613_0 += _rtP -> P_531 * _rtX -> TransferFcn_CSTATE_ho ;
_rtB -> B_40_613_0 += _rtP -> P_532 * _rtB -> B_40_612_0 ; if ( _rtB ->
B_40_613_0 > _rtP -> P_534 ) { _rtB -> B_40_614_0 = _rtB -> B_40_613_0 - _rtP
-> P_534 ; } else if ( _rtB -> B_40_613_0 >= _rtP -> P_533 ) { _rtB ->
B_40_614_0 = 0.0 ; } else { _rtB -> B_40_614_0 = _rtB -> B_40_613_0 - _rtP ->
P_533 ; } _rtB -> B_40_615_0 = _rtP -> P_535 * _rtB -> B_40_614_0 ; if ( _rtB
-> B_40_615_0 > _rtP -> P_536 ) { _rtB -> B_40_616_0 = _rtP -> P_536 ; } else
if ( _rtB -> B_40_615_0 < _rtP -> P_537 ) { _rtB -> B_40_616_0 = _rtP ->
P_537 ; } else { _rtB -> B_40_616_0 = _rtB -> B_40_615_0 ; } if ( ( _rtDW ->
LastMajorTimeA_id >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW ->
LastMajorTimeB_oi >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_40_617_0 = _rtB
-> B_40_616_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_id < _rtDW ->
LastMajorTimeB_oi ) && ( _rtDW -> LastMajorTimeB_oi < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_id >= _rtDW -> LastMajorTimeB_oi ) && (
_rtDW -> LastMajorTimeA_id >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_oi ; rtb_B_40_3_0 = _rtDW
-> PrevYB_ai ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_id ; rtb_B_40_3_0 = _rtDW -> PrevYA_f5 ; } riseValLimit =
deltaT * _rtP -> P_538 ; rateLimiterRate = _rtB -> B_40_616_0 - rtb_B_40_3_0
; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_40_617_0 = rtb_B_40_3_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_539 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_40_617_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB ->
B_40_617_0 = _rtB -> B_40_616_0 ; } } } _rtB -> B_40_618_0 = _rtP -> P_540 *
_rtB -> B_40_617_0 ; _rtB -> B_40_619_0 = _rtP -> P_541 * _rtB -> B_40_618_0
; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model3/Data Store Read" ,
0 ) ; _rtB -> B_40_620_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_d != 0 ) {
_rtX -> vv0at_CSTATE_be = _rtB -> B_40_620_0 ; } _rtB -> B_40_621_0 = _rtX ->
vv0at_CSTATE_be ; _rtB -> B_40_622_0 = muDoubleScalarAbs ( _rtB -> B_40_621_0
) ; _rtB -> B_40_623_0 = _rtP -> P_542 [ 0 ] ; _rtB -> B_40_623_0 = _rtB ->
B_40_623_0 * _rtB -> B_40_622_0 + _rtP -> P_542 [ 1 ] ; _rtB -> B_40_623_0 =
_rtB -> B_40_623_0 * _rtB -> B_40_622_0 + _rtP -> P_542 [ 2 ] ; _rtB ->
B_40_624_0 = _rtP -> P_543 * _rtB -> B_40_623_0 ; _rtB -> B_40_625_0 = _rtB
-> B_40_40_0_n * _rtB -> B_40_624_0 ; _rtB -> B_40_626_0 = _rtB -> B_40_619_0
- _rtB -> B_40_625_0 ; if ( _rtB -> B_40_626_0 > _rtB -> B_40_150_0_a ) {
_rtB -> B_40_629_0 = _rtB -> B_40_150_0_a ; } else { if ( _rtB -> B_40_626_0
< _rtB -> B_40_151_0_j ) { _rtB -> B_33_1_0 = _rtB -> B_40_151_0_j ; } else {
_rtB -> B_33_1_0 = _rtB -> B_40_626_0 ; } _rtB -> B_40_629_0 = _rtB ->
B_33_1_0 ; } { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_c . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T *
* ) & _rtDW -> TransportDelay_PWORK_c . TUbufferPtrs [ 1 ] ; real_T simTime =
ssGetT ( S ) ; real_T tMinusDelay = simTime - _rtP -> P_544 ; _rtB ->
B_40_630_0 = Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , *
tBuffer , * uBuffer , _rtDW -> TransportDelay_IWORK_k . CircularBufSize , &
_rtDW -> TransportDelay_IWORK_k . Last , _rtDW -> TransportDelay_IWORK_k .
Tail , _rtDW -> TransportDelay_IWORK_k . Head , _rtP -> P_545 , 0 , (
boolean_T ) ( ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) ==
ssGetT ( S ) ) ) ) ; } _rtB -> B_40_631_0 = 0.0 ; _rtB -> B_40_631_0 += _rtP
-> P_547 * _rtX -> TransferFcn_CSTATE_kb ; _rtB -> B_40_631_0 += _rtP ->
P_548 * _rtB -> B_40_630_0 ; if ( _rtB -> B_40_631_0 > _rtP -> P_550 ) { _rtB
-> B_40_632_0 = _rtB -> B_40_631_0 - _rtP -> P_550 ; } else if ( _rtB ->
B_40_631_0 >= _rtP -> P_549 ) { _rtB -> B_40_632_0 = 0.0 ; } else { _rtB ->
B_40_632_0 = _rtB -> B_40_631_0 - _rtP -> P_549 ; } _rtB -> B_40_633_0 = _rtP
-> P_551 * _rtB -> B_40_632_0 ; if ( _rtB -> B_40_633_0 > _rtP -> P_552 ) {
_rtB -> B_40_634_0 = _rtP -> P_552 ; } else if ( _rtB -> B_40_633_0 < _rtP ->
P_553 ) { _rtB -> B_40_634_0 = _rtP -> P_553 ; } else { _rtB -> B_40_634_0 =
_rtB -> B_40_633_0 ; } if ( ( _rtDW -> LastMajorTimeA_kn >= ssGetTaskTime ( S
, 0 ) ) && ( _rtDW -> LastMajorTimeB_he >= ssGetTaskTime ( S , 0 ) ) ) { _rtB
-> B_40_635_0 = _rtB -> B_40_634_0 ; } else { if ( ( ( _rtDW ->
LastMajorTimeA_kn < _rtDW -> LastMajorTimeB_he ) && ( _rtDW ->
LastMajorTimeB_he < ssGetTaskTime ( S , 0 ) ) ) || ( ( _rtDW ->
LastMajorTimeA_kn >= _rtDW -> LastMajorTimeB_he ) && ( _rtDW ->
LastMajorTimeA_kn >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime (
S , 0 ) - _rtDW -> LastMajorTimeB_he ; rtb_B_40_3_0 = _rtDW -> PrevYB_mp ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_kn ;
rtb_B_40_3_0 = _rtDW -> PrevYA_d5 ; } riseValLimit = deltaT * _rtP -> P_554 ;
rateLimiterRate = _rtB -> B_40_634_0 - rtb_B_40_3_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_40_635_0 = rtb_B_40_3_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_555 ; if ( rateLimiterRate < deltaT ) { _rtB ->
B_40_635_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB -> B_40_635_0 = _rtB ->
B_40_634_0 ; } } } _rtB -> B_40_636_0 = _rtP -> P_556 * _rtB -> B_40_635_0 ;
_rtB -> B_40_637_0 = _rtP -> P_557 * _rtB -> B_40_636_0 ;
vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model4/Data Store Read" ,
0 ) ; _rtB -> B_40_638_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_d1 != 0 )
{ _rtX -> vv0at_CSTATE_l2 = _rtB -> B_40_638_0 ; } _rtB -> B_40_639_0 = _rtX
-> vv0at_CSTATE_l2 ; _rtB -> B_40_640_0 = muDoubleScalarAbs ( _rtB ->
B_40_639_0 ) ; _rtB -> B_40_641_0 = _rtP -> P_558 [ 0 ] ; _rtB -> B_40_641_0
= _rtB -> B_40_641_0 * _rtB -> B_40_640_0 + _rtP -> P_558 [ 1 ] ; _rtB ->
B_40_641_0 = _rtB -> B_40_641_0 * _rtB -> B_40_640_0 + _rtP -> P_558 [ 2 ] ;
_rtB -> B_40_642_0 = _rtP -> P_559 * _rtB -> B_40_641_0 ; _rtB -> B_40_643_0
= _rtB -> B_40_41_0_b * _rtB -> B_40_642_0 ; _rtB -> B_40_644_0 = _rtB ->
B_40_637_0 - _rtB -> B_40_643_0 ; if ( _rtB -> B_40_644_0 > _rtB ->
B_40_153_0_e ) { _rtB -> B_40_647_0 = _rtB -> B_40_153_0_e ; } else { if (
_rtB -> B_40_644_0 < _rtB -> B_40_154_0_o ) { _rtB -> B_34_1_0 = _rtB ->
B_40_154_0_o ; } else { _rtB -> B_34_1_0 = _rtB -> B_40_644_0 ; } _rtB ->
B_40_647_0 = _rtB -> B_34_1_0 ; } { real_T * * uBuffer = ( real_T * * ) &
_rtDW -> TransportDelay_PWORK_or . TUbufferPtrs [ 0 ] ; real_T * * tBuffer =
( real_T * * ) & _rtDW -> TransportDelay_PWORK_or . TUbufferPtrs [ 1 ] ;
real_T simTime = ssGetT ( S ) ; real_T tMinusDelay = simTime - _rtP -> P_560
; _rtB -> B_40_648_0 = Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay ,
0.0 , * tBuffer , * uBuffer , _rtDW -> TransportDelay_IWORK_c .
CircularBufSize , & _rtDW -> TransportDelay_IWORK_c . Last , _rtDW ->
TransportDelay_IWORK_c . Tail , _rtDW -> TransportDelay_IWORK_c . Head , _rtP
-> P_561 , 0 , ( boolean_T ) ( ssIsMinorTimeStep ( S ) && (
ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) ) ) ; } _rtB -> B_40_649_0 =
0.0 ; _rtB -> B_40_649_0 += _rtP -> P_563 * _rtX -> TransferFcn_CSTATE_ke ;
_rtB -> B_40_649_0 += _rtP -> P_564 * _rtB -> B_40_648_0 ; if ( _rtB ->
B_40_649_0 > _rtP -> P_566 ) { _rtB -> B_40_650_0 = _rtB -> B_40_649_0 - _rtP
-> P_566 ; } else if ( _rtB -> B_40_649_0 >= _rtP -> P_565 ) { _rtB ->
B_40_650_0 = 0.0 ; } else { _rtB -> B_40_650_0 = _rtB -> B_40_649_0 - _rtP ->
P_565 ; } _rtB -> B_40_651_0 = _rtP -> P_567 * _rtB -> B_40_650_0 ; if ( _rtB
-> B_40_651_0 > _rtP -> P_568 ) { _rtB -> B_40_652_0 = _rtP -> P_568 ; } else
if ( _rtB -> B_40_651_0 < _rtP -> P_569 ) { _rtB -> B_40_652_0 = _rtP ->
P_569 ; } else { _rtB -> B_40_652_0 = _rtB -> B_40_651_0 ; } if ( ( _rtDW ->
LastMajorTimeA_pu >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW ->
LastMajorTimeB_nmm >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_40_653_0 = _rtB
-> B_40_652_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_pu < _rtDW ->
LastMajorTimeB_nmm ) && ( _rtDW -> LastMajorTimeB_nmm < ssGetTaskTime ( S , 0
) ) ) || ( ( _rtDW -> LastMajorTimeA_pu >= _rtDW -> LastMajorTimeB_nmm ) && (
_rtDW -> LastMajorTimeA_pu >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_nmm ; rtb_B_40_3_0 = _rtDW
-> PrevYB_ic ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_pu ; rtb_B_40_3_0 = _rtDW -> PrevYA_gf ; } riseValLimit =
deltaT * _rtP -> P_570 ; rateLimiterRate = _rtB -> B_40_652_0 - rtb_B_40_3_0
; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_40_653_0 = rtb_B_40_3_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_571 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_40_653_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB ->
B_40_653_0 = _rtB -> B_40_652_0 ; } } } _rtB -> B_40_654_0 = _rtP -> P_572 *
_rtB -> B_40_653_0 ; _rtB -> B_40_655_0 = _rtP -> P_573 * _rtB -> B_40_654_0
; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model/Data Store Read" , 0
) ; _rtB -> B_40_656_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_am != 0 ) {
_rtX -> vv0at_CSTATE_o2 = _rtB -> B_40_656_0 ; } _rtB -> B_40_657_0 = _rtX ->
vv0at_CSTATE_o2 ; _rtB -> B_40_658_0 = muDoubleScalarAbs ( _rtB -> B_40_657_0
) ; _rtB -> B_40_659_0 = _rtP -> P_574 [ 0 ] ; _rtB -> B_40_659_0 = _rtB ->
B_40_659_0 * _rtB -> B_40_658_0 + _rtP -> P_574 [ 1 ] ; _rtB -> B_40_659_0 =
_rtB -> B_40_659_0 * _rtB -> B_40_658_0 + _rtP -> P_574 [ 2 ] ; _rtB ->
B_40_660_0 = _rtP -> P_575 * _rtB -> B_40_659_0 ; _rtB -> B_40_661_0 = _rtB
-> B_40_43_0_l * _rtB -> B_40_660_0 ; _rtB -> B_40_662_0 = _rtB -> B_40_655_0
- _rtB -> B_40_661_0 ; if ( _rtB -> B_40_662_0 > _rtB -> B_40_156_0_b ) {
_rtB -> B_40_665_0 = _rtB -> B_40_156_0_b ; } else { if ( _rtB -> B_40_662_0
< _rtB -> B_40_157_0_a ) { _rtB -> B_35_1_0 = _rtB -> B_40_157_0_a ; } else {
_rtB -> B_35_1_0 = _rtB -> B_40_662_0 ; } _rtB -> B_40_665_0 = _rtB ->
B_35_1_0 ; } { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_m4 . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T
* * ) & _rtDW -> TransportDelay_PWORK_m4 . TUbufferPtrs [ 1 ] ; real_T
simTime = ssGetT ( S ) ; real_T tMinusDelay = simTime - _rtP -> P_576 ; _rtB
-> B_40_666_0 = Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 ,
* tBuffer , * uBuffer , _rtDW -> TransportDelay_IWORK_g4 . CircularBufSize ,
& _rtDW -> TransportDelay_IWORK_g4 . Last , _rtDW -> TransportDelay_IWORK_g4
. Tail , _rtDW -> TransportDelay_IWORK_g4 . Head , _rtP -> P_577 , 0 , (
boolean_T ) ( ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) ==
ssGetT ( S ) ) ) ) ; } _rtB -> B_40_667_0 = 0.0 ; _rtB -> B_40_667_0 += _rtP
-> P_579 * _rtX -> TransferFcn_CSTATE_o ; _rtB -> B_40_667_0 += _rtP -> P_580
* _rtB -> B_40_666_0 ; if ( _rtB -> B_40_667_0 > _rtP -> P_582 ) { _rtB ->
B_40_668_0 = _rtB -> B_40_667_0 - _rtP -> P_582 ; } else if ( _rtB ->
B_40_667_0 >= _rtP -> P_581 ) { _rtB -> B_40_668_0 = 0.0 ; } else { _rtB ->
B_40_668_0 = _rtB -> B_40_667_0 - _rtP -> P_581 ; } _rtB -> B_40_669_0 = _rtP
-> P_583 * _rtB -> B_40_668_0 ; if ( _rtB -> B_40_669_0 > _rtP -> P_584 ) {
_rtB -> B_40_670_0 = _rtP -> P_584 ; } else if ( _rtB -> B_40_669_0 < _rtP ->
P_585 ) { _rtB -> B_40_670_0 = _rtP -> P_585 ; } else { _rtB -> B_40_670_0 =
_rtB -> B_40_669_0 ; } if ( ( _rtDW -> LastMajorTimeA_hx >= ssGetTaskTime ( S
, 0 ) ) && ( _rtDW -> LastMajorTimeB_ho >= ssGetTaskTime ( S , 0 ) ) ) { _rtB
-> B_40_671_0 = _rtB -> B_40_670_0 ; } else { if ( ( ( _rtDW ->
LastMajorTimeA_hx < _rtDW -> LastMajorTimeB_ho ) && ( _rtDW ->
LastMajorTimeB_ho < ssGetTaskTime ( S , 0 ) ) ) || ( ( _rtDW ->
LastMajorTimeA_hx >= _rtDW -> LastMajorTimeB_ho ) && ( _rtDW ->
LastMajorTimeA_hx >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime (
S , 0 ) - _rtDW -> LastMajorTimeB_ho ; rtb_B_40_3_0 = _rtDW -> PrevYB_pl ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_hx ;
rtb_B_40_3_0 = _rtDW -> PrevYA_av ; } riseValLimit = deltaT * _rtP -> P_586 ;
rateLimiterRate = _rtB -> B_40_670_0 - rtb_B_40_3_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_40_671_0 = rtb_B_40_3_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_587 ; if ( rateLimiterRate < deltaT ) { _rtB ->
B_40_671_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB -> B_40_671_0 = _rtB ->
B_40_670_0 ; } } } _rtB -> B_40_672_0 = _rtP -> P_588 * _rtB -> B_40_671_0 ;
_rtB -> B_40_673_0 = _rtP -> P_589 * _rtB -> B_40_672_0 ;
vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model1/Data Store Read" ,
0 ) ; _rtB -> B_40_674_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_ks != 0 )
{ _rtX -> vv0at_CSTATE_kcg = _rtB -> B_40_674_0 ; } _rtB -> B_40_675_0 = _rtX
-> vv0at_CSTATE_kcg ; _rtB -> B_40_676_0 = muDoubleScalarAbs ( _rtB ->
B_40_675_0 ) ; _rtB -> B_40_677_0 = _rtP -> P_590 [ 0 ] ; _rtB -> B_40_677_0
= _rtB -> B_40_677_0 * _rtB -> B_40_676_0 + _rtP -> P_590 [ 1 ] ; _rtB ->
B_40_677_0 = _rtB -> B_40_677_0 * _rtB -> B_40_676_0 + _rtP -> P_590 [ 2 ] ;
_rtB -> B_40_678_0 = _rtP -> P_591 * _rtB -> B_40_677_0 ; _rtB -> B_40_679_0
= _rtB -> B_40_44_0_h * _rtB -> B_40_678_0 ; _rtB -> B_40_680_0 = _rtB ->
B_40_673_0 - _rtB -> B_40_679_0 ; if ( _rtB -> B_40_680_0 > _rtB ->
B_40_159_0 ) { _rtB -> B_40_683_0 = _rtB -> B_40_159_0 ; } else { if ( _rtB
-> B_40_680_0 < _rtB -> B_40_160_0 ) { _rtB -> B_36_1_0 = _rtB -> B_40_160_0
; } else { _rtB -> B_36_1_0 = _rtB -> B_40_680_0 ; } _rtB -> B_40_683_0 =
_rtB -> B_36_1_0 ; } { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_ea . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T
* * ) & _rtDW -> TransportDelay_PWORK_ea . TUbufferPtrs [ 1 ] ; real_T
simTime = ssGetT ( S ) ; real_T tMinusDelay = simTime - _rtP -> P_592 ; _rtB
-> B_40_684_0 = Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 ,
* tBuffer , * uBuffer , _rtDW -> TransportDelay_IWORK_ovh . CircularBufSize ,
& _rtDW -> TransportDelay_IWORK_ovh . Last , _rtDW ->
TransportDelay_IWORK_ovh . Tail , _rtDW -> TransportDelay_IWORK_ovh . Head ,
_rtP -> P_593 , 0 , ( boolean_T ) ( ssIsMinorTimeStep ( S ) && (
ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) ) ) ; } _rtB -> B_40_685_0 =
0.0 ; _rtB -> B_40_685_0 += _rtP -> P_595 * _rtX -> TransferFcn_CSTATE_ge ;
_rtB -> B_40_685_0 += _rtP -> P_596 * _rtB -> B_40_684_0 ; if ( _rtB ->
B_40_685_0 > _rtP -> P_598 ) { _rtB -> B_40_686_0 = _rtB -> B_40_685_0 - _rtP
-> P_598 ; } else if ( _rtB -> B_40_685_0 >= _rtP -> P_597 ) { _rtB ->
B_40_686_0 = 0.0 ; } else { _rtB -> B_40_686_0 = _rtB -> B_40_685_0 - _rtP ->
P_597 ; } _rtB -> B_40_687_0 = _rtP -> P_599 * _rtB -> B_40_686_0 ; if ( _rtB
-> B_40_687_0 > _rtP -> P_600 ) { _rtB -> B_40_688_0 = _rtP -> P_600 ; } else
if ( _rtB -> B_40_687_0 < _rtP -> P_601 ) { _rtB -> B_40_688_0 = _rtP ->
P_601 ; } else { _rtB -> B_40_688_0 = _rtB -> B_40_687_0 ; } if ( ( _rtDW ->
LastMajorTimeA_l1 >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW ->
LastMajorTimeB_en >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_40_689_0 = _rtB
-> B_40_688_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_l1 < _rtDW ->
LastMajorTimeB_en ) && ( _rtDW -> LastMajorTimeB_en < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_l1 >= _rtDW -> LastMajorTimeB_en ) && (
_rtDW -> LastMajorTimeA_l1 >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_en ; rtb_B_40_3_0 = _rtDW
-> PrevYB_ilc ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_l1 ; rtb_B_40_3_0 = _rtDW -> PrevYA_co ; } riseValLimit =
deltaT * _rtP -> P_602 ; rateLimiterRate = _rtB -> B_40_688_0 - rtb_B_40_3_0
; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_40_689_0 = rtb_B_40_3_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_603 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_40_689_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB ->
B_40_689_0 = _rtB -> B_40_688_0 ; } } } _rtB -> B_40_690_0 = _rtP -> P_604 *
_rtB -> B_40_689_0 ; _rtB -> B_40_691_0 = _rtP -> P_605 * _rtB -> B_40_690_0
; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model2/Data Store Read" ,
0 ) ; _rtB -> B_40_692_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_gs != 0 )
{ _rtX -> vv0at_CSTATE_ok = _rtB -> B_40_692_0 ; } _rtB -> B_40_693_0 = _rtX
-> vv0at_CSTATE_ok ; _rtB -> B_40_694_0 = muDoubleScalarAbs ( _rtB ->
B_40_693_0 ) ; _rtB -> B_40_695_0 = _rtP -> P_606 [ 0 ] ; _rtB -> B_40_695_0
= _rtB -> B_40_695_0 * _rtB -> B_40_694_0 + _rtP -> P_606 [ 1 ] ; _rtB ->
B_40_695_0 = _rtB -> B_40_695_0 * _rtB -> B_40_694_0 + _rtP -> P_606 [ 2 ] ;
_rtB -> B_40_696_0 = _rtP -> P_607 * _rtB -> B_40_695_0 ; _rtB -> B_40_697_0
= _rtB -> B_40_45_0_b * _rtB -> B_40_696_0 ; _rtB -> B_40_698_0 = _rtB ->
B_40_691_0 - _rtB -> B_40_697_0 ; if ( _rtB -> B_40_698_0 > _rtB ->
B_40_162_0_g ) { _rtB -> B_40_701_0 = _rtB -> B_40_162_0_g ; } else { if (
_rtB -> B_40_698_0 < _rtB -> B_40_163_0_e ) { _rtB -> B_37_1_0 = _rtB ->
B_40_163_0_e ; } else { _rtB -> B_37_1_0 = _rtB -> B_40_698_0 ; } _rtB ->
B_40_701_0 = _rtB -> B_37_1_0 ; } { real_T * * uBuffer = ( real_T * * ) &
_rtDW -> TransportDelay_PWORK_d2 . TUbufferPtrs [ 0 ] ; real_T * * tBuffer =
( real_T * * ) & _rtDW -> TransportDelay_PWORK_d2 . TUbufferPtrs [ 1 ] ;
real_T simTime = ssGetT ( S ) ; real_T tMinusDelay = simTime - _rtP -> P_608
; _rtB -> B_40_702_0 = Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay ,
0.0 , * tBuffer , * uBuffer , _rtDW -> TransportDelay_IWORK_i5 .
CircularBufSize , & _rtDW -> TransportDelay_IWORK_i5 . Last , _rtDW ->
TransportDelay_IWORK_i5 . Tail , _rtDW -> TransportDelay_IWORK_i5 . Head ,
_rtP -> P_609 , 0 , ( boolean_T ) ( ssIsMinorTimeStep ( S ) && (
ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) ) ) ; } _rtB -> B_40_703_0 =
0.0 ; _rtB -> B_40_703_0 += _rtP -> P_611 * _rtX -> TransferFcn_CSTATE_nc ;
_rtB -> B_40_703_0 += _rtP -> P_612 * _rtB -> B_40_702_0 ; if ( _rtB ->
B_40_703_0 > _rtP -> P_614 ) { _rtB -> B_40_704_0 = _rtB -> B_40_703_0 - _rtP
-> P_614 ; } else if ( _rtB -> B_40_703_0 >= _rtP -> P_613 ) { _rtB ->
B_40_704_0 = 0.0 ; } else { _rtB -> B_40_704_0 = _rtB -> B_40_703_0 - _rtP ->
P_613 ; } _rtB -> B_40_705_0 = _rtP -> P_615 * _rtB -> B_40_704_0 ; if ( _rtB
-> B_40_705_0 > _rtP -> P_616 ) { _rtB -> B_40_706_0 = _rtP -> P_616 ; } else
if ( _rtB -> B_40_705_0 < _rtP -> P_617 ) { _rtB -> B_40_706_0 = _rtP ->
P_617 ; } else { _rtB -> B_40_706_0 = _rtB -> B_40_705_0 ; } if ( ( _rtDW ->
LastMajorTimeA_c3 >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW ->
LastMajorTimeB_oy >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_40_707_0 = _rtB
-> B_40_706_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_c3 < _rtDW ->
LastMajorTimeB_oy ) && ( _rtDW -> LastMajorTimeB_oy < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_c3 >= _rtDW -> LastMajorTimeB_oy ) && (
_rtDW -> LastMajorTimeA_c3 >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_oy ; rtb_B_40_3_0 = _rtDW
-> PrevYB_av ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_c3 ; rtb_B_40_3_0 = _rtDW -> PrevYA_dq ; } riseValLimit =
deltaT * _rtP -> P_618 ; rateLimiterRate = _rtB -> B_40_706_0 - rtb_B_40_3_0
; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_40_707_0 = rtb_B_40_3_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_619 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_40_707_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB ->
B_40_707_0 = _rtB -> B_40_706_0 ; } } } _rtB -> B_40_708_0 = _rtP -> P_620 *
_rtB -> B_40_707_0 ; _rtB -> B_40_709_0 = _rtP -> P_621 * _rtB -> B_40_708_0
; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model3/Data Store Read" ,
0 ) ; _rtB -> B_40_710_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_dy != 0 )
{ _rtX -> vv0at_CSTATE_my = _rtB -> B_40_710_0 ; } _rtB -> B_40_711_0 = _rtX
-> vv0at_CSTATE_my ; _rtB -> B_40_712_0 = muDoubleScalarAbs ( _rtB ->
B_40_711_0 ) ; _rtB -> B_40_713_0 = _rtP -> P_622 [ 0 ] ; _rtB -> B_40_713_0
= _rtB -> B_40_713_0 * _rtB -> B_40_712_0 + _rtP -> P_622 [ 1 ] ; _rtB ->
B_40_713_0 = _rtB -> B_40_713_0 * _rtB -> B_40_712_0 + _rtP -> P_622 [ 2 ] ;
_rtB -> B_40_714_0 = _rtP -> P_623 * _rtB -> B_40_713_0 ; _rtB -> B_40_715_0
= _rtB -> B_40_46_0_d * _rtB -> B_40_714_0 ; _rtB -> B_40_716_0 = _rtB ->
B_40_709_0 - _rtB -> B_40_715_0 ; if ( _rtB -> B_40_716_0 > _rtB ->
B_40_165_0_f ) { _rtB -> B_40_719_0 = _rtB -> B_40_165_0_f ; } else { if (
_rtB -> B_40_716_0 < _rtB -> B_40_166_0_h ) { _rtB -> B_38_1_0 = _rtB ->
B_40_166_0_h ; } else { _rtB -> B_38_1_0 = _rtB -> B_40_716_0 ; } _rtB ->
B_40_719_0 = _rtB -> B_38_1_0 ; } { real_T * * uBuffer = ( real_T * * ) &
_rtDW -> TransportDelay_PWORK_kq . TUbufferPtrs [ 0 ] ; real_T * * tBuffer =
( real_T * * ) & _rtDW -> TransportDelay_PWORK_kq . TUbufferPtrs [ 1 ] ;
real_T simTime = ssGetT ( S ) ; real_T tMinusDelay = simTime - _rtP -> P_624
; _rtB -> B_40_720_0 = Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay ,
0.0 , * tBuffer , * uBuffer , _rtDW -> TransportDelay_IWORK_ns .
CircularBufSize , & _rtDW -> TransportDelay_IWORK_ns . Last , _rtDW ->
TransportDelay_IWORK_ns . Tail , _rtDW -> TransportDelay_IWORK_ns . Head ,
_rtP -> P_625 , 0 , ( boolean_T ) ( ssIsMinorTimeStep ( S ) && (
ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) ) ) ; } _rtB -> B_40_721_0 =
0.0 ; _rtB -> B_40_721_0 += _rtP -> P_627 * _rtX -> TransferFcn_CSTATE_b ;
_rtB -> B_40_721_0 += _rtP -> P_628 * _rtB -> B_40_720_0 ; if ( _rtB ->
B_40_721_0 > _rtP -> P_630 ) { _rtB -> B_40_722_0 = _rtB -> B_40_721_0 - _rtP
-> P_630 ; } else if ( _rtB -> B_40_721_0 >= _rtP -> P_629 ) { _rtB ->
B_40_722_0 = 0.0 ; } else { _rtB -> B_40_722_0 = _rtB -> B_40_721_0 - _rtP ->
P_629 ; } _rtB -> B_40_723_0 = _rtP -> P_631 * _rtB -> B_40_722_0 ; if ( _rtB
-> B_40_723_0 > _rtP -> P_632 ) { _rtB -> B_40_724_0 = _rtP -> P_632 ; } else
if ( _rtB -> B_40_723_0 < _rtP -> P_633 ) { _rtB -> B_40_724_0 = _rtP ->
P_633 ; } else { _rtB -> B_40_724_0 = _rtB -> B_40_723_0 ; } if ( ( _rtDW ->
LastMajorTimeA_la >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW ->
LastMajorTimeB_iq >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_40_725_0 = _rtB
-> B_40_724_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_la < _rtDW ->
LastMajorTimeB_iq ) && ( _rtDW -> LastMajorTimeB_iq < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_la >= _rtDW -> LastMajorTimeB_iq ) && (
_rtDW -> LastMajorTimeA_la >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_iq ; rtb_B_40_3_0 = _rtDW
-> PrevYB_as ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_la ; rtb_B_40_3_0 = _rtDW -> PrevYA_hv ; } riseValLimit =
deltaT * _rtP -> P_634 ; rateLimiterRate = _rtB -> B_40_724_0 - rtb_B_40_3_0
; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_40_725_0 = rtb_B_40_3_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_635 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_40_725_0 = rtb_B_40_3_0 + deltaT ; } else { _rtB ->
B_40_725_0 = _rtB -> B_40_724_0 ; } } } _rtB -> B_40_726_0 = _rtP -> P_636 *
_rtB -> B_40_725_0 ; _rtB -> B_40_727_0 = _rtP -> P_637 * _rtB -> B_40_726_0
; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model4/Data Store Read" ,
0 ) ; _rtB -> B_40_728_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_de != 0 )
{ _rtX -> vv0at_CSTATE_ev = _rtB -> B_40_728_0 ; } _rtB -> B_40_729_0 = _rtX
-> vv0at_CSTATE_ev ; _rtB -> B_40_730_0 = muDoubleScalarAbs ( _rtB ->
B_40_729_0 ) ; _rtB -> B_40_731_0 = _rtP -> P_638 [ 0 ] ; _rtB -> B_40_731_0
= _rtB -> B_40_731_0 * _rtB -> B_40_730_0 + _rtP -> P_638 [ 1 ] ; _rtB ->
B_40_731_0 = _rtB -> B_40_731_0 * _rtB -> B_40_730_0 + _rtP -> P_638 [ 2 ] ;
_rtB -> B_40_732_0 = _rtP -> P_639 * _rtB -> B_40_731_0 ; _rtB -> B_40_733_0
= _rtB -> B_40_47_0_e * _rtB -> B_40_732_0 ; _rtB -> B_40_734_0 = _rtB ->
B_40_727_0 - _rtB -> B_40_733_0 ; if ( _rtB -> B_40_734_0 > _rtB ->
B_40_168_0_e ) { _rtB -> B_40_737_0 = _rtB -> B_40_168_0_e ; } else { if (
_rtB -> B_40_734_0 < _rtB -> B_40_169_0_c ) { _rtB -> B_39_1_0 = _rtB ->
B_40_169_0_c ; } else { _rtB -> B_39_1_0 = _rtB -> B_40_734_0 ; } _rtB ->
B_40_737_0 = _rtB -> B_39_1_0 ; } _rtB -> B_40_738_0 [ 0 ] = _rtB ->
B_40_35_0 ; _rtB -> B_40_738_0 [ 1 ] = _rtB -> B_40_53_0 ; _rtB -> B_40_738_0
[ 2 ] = _rtB -> B_40_71_0 ; _rtB -> B_40_738_0 [ 3 ] = _rtB -> B_40_89_0 ;
_rtB -> B_40_738_0 [ 4 ] = _rtB -> B_40_107_0 ; _rtB -> B_40_738_0 [ 5 ] =
_rtB -> B_40_125_0 ; _rtB -> B_40_738_0 [ 6 ] = _rtB -> B_40_143_0 ; _rtB ->
B_40_738_0 [ 7 ] = _rtB -> B_40_161_0 ; _rtB -> B_40_738_0 [ 8 ] = _rtB ->
B_40_179_0 ; _rtB -> B_40_738_0 [ 9 ] = _rtB -> B_40_197_0 ; _rtB ->
B_40_738_0 [ 10 ] = _rtB -> B_40_215_0 ; _rtB -> B_40_738_0 [ 11 ] = _rtB ->
B_40_233_0 ; _rtB -> B_40_738_0 [ 12 ] = _rtB -> B_40_251_0 ; _rtB ->
B_40_738_0 [ 13 ] = _rtB -> B_40_269_0 ; _rtB -> B_40_738_0 [ 14 ] = _rtB ->
B_40_287_0 ; _rtB -> B_40_738_0 [ 15 ] = _rtB -> B_40_305_0 ; _rtB ->
B_40_738_0 [ 16 ] = _rtB -> B_40_323_0 ; _rtB -> B_40_738_0 [ 17 ] = _rtB ->
B_40_341_0 ; _rtB -> B_40_738_0 [ 18 ] = _rtB -> B_40_359_0 ; _rtB ->
B_40_738_0 [ 19 ] = _rtB -> B_40_377_0 ; _rtB -> B_40_738_0 [ 20 ] = _rtB ->
B_40_395_0 ; _rtB -> B_40_738_0 [ 21 ] = _rtB -> B_40_413_0 ; _rtB ->
B_40_738_0 [ 22 ] = _rtB -> B_40_431_0 ; _rtB -> B_40_738_0 [ 23 ] = _rtB ->
B_40_449_0 ; _rtB -> B_40_738_0 [ 24 ] = _rtB -> B_40_467_0 ; _rtB ->
B_40_738_0 [ 25 ] = _rtB -> B_40_485_0 ; _rtB -> B_40_738_0 [ 26 ] = _rtB ->
B_40_503_0 ; _rtB -> B_40_738_0 [ 27 ] = _rtB -> B_40_521_0 ; _rtB ->
B_40_738_0 [ 28 ] = _rtB -> B_40_539_0 ; _rtB -> B_40_738_0 [ 29 ] = _rtB ->
B_40_557_0 ; _rtB -> B_40_738_0 [ 30 ] = _rtB -> B_40_575_0 ; _rtB ->
B_40_738_0 [ 31 ] = _rtB -> B_40_593_0 ; _rtB -> B_40_738_0 [ 32 ] = _rtB ->
B_40_611_0 ; _rtB -> B_40_738_0 [ 33 ] = _rtB -> B_40_629_0 ; _rtB ->
B_40_738_0 [ 34 ] = _rtB -> B_40_647_0 ; _rtB -> B_40_738_0 [ 35 ] = _rtB ->
B_40_665_0 ; _rtB -> B_40_738_0 [ 36 ] = _rtB -> B_40_683_0 ; _rtB ->
B_40_738_0 [ 37 ] = _rtB -> B_40_701_0 ; _rtB -> B_40_738_0 [ 38 ] = _rtB ->
B_40_719_0 ; _rtB -> B_40_738_0 [ 39 ] = _rtB -> B_40_737_0 ;
ssCallAccelRunBlock ( S , 40 , 739 , SS_CALL_MDL_OUTPUTS ) ; { real_T * *
uBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_ju . TUbufferPtrs [
0 ] ; real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_ju
. TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_640 ; _rtB -> B_40_740_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_gm . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_gm . Last , _rtDW -> TransportDelay_IWORK_gm . Tail ,
_rtDW -> TransportDelay_IWORK_gm . Head , _rtP -> P_641 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_40_741_0 = 0.0 ; _rtB -> B_40_741_0 += _rtP -> P_643 * _rtX
-> TransferFcn_CSTATE_pj ; _rtB -> B_40_741_0 += _rtP -> P_644 * _rtB ->
B_40_740_0 ; _rtB -> B_40_738_0 [ 0 ] = _rtB -> B_40_38_0 ; _rtB ->
B_40_738_0 [ 1 ] = _rtB -> B_40_56_0 ; _rtB -> B_40_738_0 [ 2 ] = _rtB ->
B_40_74_0 ; _rtB -> B_40_738_0 [ 3 ] = _rtB -> B_40_92_0 ; _rtB -> B_40_738_0
[ 4 ] = _rtB -> B_40_110_0 ; _rtB -> B_40_738_0 [ 5 ] = _rtB -> B_40_128_0 ;
_rtB -> B_40_738_0 [ 6 ] = _rtB -> B_40_146_0 ; _rtB -> B_40_738_0 [ 7 ] =
_rtB -> B_40_164_0 ; _rtB -> B_40_738_0 [ 8 ] = _rtB -> B_40_182_0 ; _rtB ->
B_40_738_0 [ 9 ] = _rtB -> B_40_200_0 ; _rtB -> B_40_738_0 [ 10 ] = _rtB ->
B_40_218_0 ; _rtB -> B_40_738_0 [ 11 ] = _rtB -> B_40_236_0 ; _rtB ->
B_40_738_0 [ 12 ] = _rtB -> B_40_254_0 ; _rtB -> B_40_738_0 [ 13 ] = _rtB ->
B_40_272_0 ; _rtB -> B_40_738_0 [ 14 ] = _rtB -> B_40_290_0 ; _rtB ->
B_40_738_0 [ 15 ] = _rtB -> B_40_308_0 ; _rtB -> B_40_738_0 [ 16 ] = _rtB ->
B_40_326_0 ; _rtB -> B_40_738_0 [ 17 ] = _rtB -> B_40_344_0 ; _rtB ->
B_40_738_0 [ 18 ] = _rtB -> B_40_362_0 ; _rtB -> B_40_738_0 [ 19 ] = _rtB ->
B_40_380_0 ; _rtB -> B_40_738_0 [ 20 ] = _rtB -> B_40_398_0 ; _rtB ->
B_40_738_0 [ 21 ] = _rtB -> B_40_416_0 ; _rtB -> B_40_738_0 [ 22 ] = _rtB ->
B_40_434_0 ; _rtB -> B_40_738_0 [ 23 ] = _rtB -> B_40_452_0 ; _rtB ->
B_40_738_0 [ 24 ] = _rtB -> B_40_470_0 ; _rtB -> B_40_738_0 [ 25 ] = _rtB ->
B_40_488_0 ; _rtB -> B_40_738_0 [ 26 ] = _rtB -> B_40_506_0 ; _rtB ->
B_40_738_0 [ 27 ] = _rtB -> B_40_524_0 ; _rtB -> B_40_738_0 [ 28 ] = _rtB ->
B_40_542_0 ; _rtB -> B_40_738_0 [ 29 ] = _rtB -> B_40_560_0 ; _rtB ->
B_40_738_0 [ 30 ] = _rtB -> B_40_578_0 ; _rtB -> B_40_738_0 [ 31 ] = _rtB ->
B_40_596_0 ; _rtB -> B_40_738_0 [ 32 ] = _rtB -> B_40_614_0 ; _rtB ->
B_40_738_0 [ 33 ] = _rtB -> B_40_632_0 ; _rtB -> B_40_738_0 [ 34 ] = _rtB ->
B_40_650_0 ; _rtB -> B_40_738_0 [ 35 ] = _rtB -> B_40_668_0 ; _rtB ->
B_40_738_0 [ 36 ] = _rtB -> B_40_686_0 ; _rtB -> B_40_738_0 [ 37 ] = _rtB ->
B_40_704_0 ; _rtB -> B_40_738_0 [ 38 ] = _rtB -> B_40_722_0 ; if ( _rtB ->
B_40_741_0 > _rtP -> P_646 ) { _rtB -> B_40_738_0 [ 39 ] = _rtB -> B_40_741_0
- _rtP -> P_646 ; } else if ( _rtB -> B_40_741_0 >= _rtP -> P_645 ) { _rtB ->
B_40_738_0 [ 39 ] = 0.0 ; } else { _rtB -> B_40_738_0 [ 39 ] = _rtB ->
B_40_741_0 - _rtP -> P_645 ; } ssCallAccelRunBlock ( S , 40 , 744 ,
SS_CALL_MDL_OUTPUTS ) ; _rtB -> B_40_745_0 = _rtB -> B_40_35_0 / _rtB ->
B_40_1_0_c ; _rtB -> B_40_746_0 = _rtB -> B_40_745_0 + _rtP -> P_647 ; _rtB
-> B_40_747_0 = _rtB -> B_40_53_0 / _rtB -> B_40_2_0 ; _rtB -> B_40_748_0 =
_rtB -> B_40_747_0 + _rtP -> P_648 ; _rtB -> B_40_749_0 = _rtB -> B_40_71_0 /
_rtB -> B_40_3_0 ; _rtB -> B_40_750_0 = _rtB -> B_40_749_0 + _rtP -> P_649 ;
_rtB -> B_40_751_0 = _rtB -> B_40_89_0 / _rtB -> B_40_4_0 ; _rtB ->
B_40_752_0 = _rtB -> B_40_751_0 + _rtP -> P_650 ; _rtB -> B_40_753_0 = _rtB
-> B_40_107_0 / _rtB -> B_40_5_0 ; _rtB -> B_40_754_0 = _rtB -> B_40_753_0 +
_rtP -> P_651 ; _rtB -> B_40_755_0 = _rtB -> B_40_125_0 / _rtB -> B_40_7_0_k
; _rtB -> B_40_756_0 = _rtB -> B_40_755_0 + _rtP -> P_652 ; _rtB ->
B_40_757_0 = _rtB -> B_40_143_0 / _rtB -> B_40_8_0 ; _rtB -> B_40_758_0 =
_rtB -> B_40_757_0 + _rtP -> P_653 ; _rtB -> B_40_759_0 = _rtB -> B_40_161_0
/ _rtB -> B_40_9_0 ; _rtB -> B_40_760_0 = _rtB -> B_40_759_0 + _rtP -> P_654
; _rtB -> B_40_761_0 = _rtB -> B_40_179_0 / _rtB -> B_40_10_0_c ; _rtB ->
B_40_762_0 = _rtB -> B_40_761_0 + _rtP -> P_655 ; _rtB -> B_40_763_0 = _rtB
-> B_40_197_0 / _rtB -> B_40_11_0_b ; _rtB -> B_40_764_0 = _rtB -> B_40_763_0
+ _rtP -> P_656 ; _rtB -> B_40_765_0 = _rtB -> B_40_215_0 / _rtB -> B_40_13_0
; _rtB -> B_40_766_0 = _rtB -> B_40_765_0 + _rtP -> P_657 ; _rtB ->
B_40_767_0 = _rtB -> B_40_233_0 / _rtB -> B_40_14_0_p ; _rtB -> B_40_768_0 =
_rtB -> B_40_767_0 + _rtP -> P_658 ; _rtB -> B_40_769_0 = _rtB -> B_40_251_0
/ _rtB -> B_40_15_0 ; _rtB -> B_40_770_0 = _rtB -> B_40_769_0 + _rtP -> P_659
; _rtB -> B_40_771_0 = _rtB -> B_40_269_0 / _rtB -> B_40_16_0 ; _rtB ->
B_40_772_0 = _rtB -> B_40_771_0 + _rtP -> P_660 ; _rtB -> B_40_773_0 = _rtB
-> B_40_287_0 / _rtB -> B_40_17_0_c ; _rtB -> B_40_774_0 = _rtB -> B_40_773_0
+ _rtP -> P_661 ; _rtB -> B_40_775_0 = _rtB -> B_40_305_0 / _rtB ->
B_40_19_0_f ; _rtB -> B_40_776_0 = _rtB -> B_40_775_0 + _rtP -> P_662 ; _rtB
-> B_40_777_0 = _rtB -> B_40_323_0 / _rtB -> B_40_20_0_g ; _rtB -> B_40_778_0
= _rtB -> B_40_777_0 + _rtP -> P_663 ; _rtB -> B_40_779_0 = _rtB ->
B_40_341_0 / _rtB -> B_40_21_0_g ; _rtB -> B_40_780_0 = _rtB -> B_40_779_0 +
_rtP -> P_664 ; _rtB -> B_40_781_0 = _rtB -> B_40_359_0 / _rtB -> B_40_22_0_m
; _rtB -> B_40_782_0 = _rtB -> B_40_781_0 + _rtP -> P_665 ; _rtB ->
B_40_783_0 = _rtB -> B_40_377_0 / _rtB -> B_40_23_0_n ; _rtB -> B_40_784_0 =
_rtB -> B_40_783_0 + _rtP -> P_666 ; _rtB -> B_40_785_0 = _rtB -> B_40_395_0
/ _rtB -> B_40_25_0_p ; _rtB -> B_40_786_0 = _rtB -> B_40_785_0 + _rtP ->
P_667 ; _rtB -> B_40_787_0 = _rtB -> B_40_413_0 / _rtB -> B_40_26_0_l ; _rtB
-> B_40_788_0 = _rtB -> B_40_787_0 + _rtP -> P_668 ; _rtB -> B_40_789_0 =
_rtB -> B_40_431_0 / _rtB -> B_40_27_0_j ; _rtB -> B_40_790_0 = _rtB ->
B_40_789_0 + _rtP -> P_669 ; _rtB -> B_40_791_0 = _rtB -> B_40_449_0 / _rtB
-> B_40_28_0_d ; _rtB -> B_40_792_0 = _rtB -> B_40_791_0 + _rtP -> P_670 ;
_rtB -> B_40_793_0 = _rtB -> B_40_467_0 / _rtB -> B_40_29_0_g ; _rtB ->
B_40_794_0 = _rtB -> B_40_793_0 + _rtP -> P_671 ; _rtB -> B_40_795_0 = _rtB
-> B_40_485_0 / _rtB -> B_40_31_0_l ; _rtB -> B_40_796_0 = _rtB -> B_40_795_0
+ _rtP -> P_672 ; _rtB -> B_40_797_0 = _rtB -> B_40_503_0 / _rtB ->
B_40_32_0_d ; _rtB -> B_40_798_0 = _rtB -> B_40_797_0 + _rtP -> P_673 ; _rtB
-> B_40_799_0 = _rtB -> B_40_521_0 / _rtB -> B_40_33_0 ; _rtB -> B_40_800_0 =
_rtB -> B_40_799_0 + _rtP -> P_674 ; _rtB -> B_40_801_0 = _rtB -> B_40_539_0
/ _rtB -> B_40_34_0 ; _rtB -> B_40_802_0 = _rtB -> B_40_801_0 + _rtP -> P_675
; _rtB -> B_40_803_0 = _rtB -> B_40_557_0 / _rtB -> B_40_35_0_d ; _rtB ->
B_40_804_0 = _rtB -> B_40_803_0 + _rtP -> P_676 ; _rtB -> B_40_805_0 = _rtB
-> B_40_575_0 / _rtB -> B_40_37_0_l ; _rtB -> B_40_806_0 = _rtB -> B_40_805_0
+ _rtP -> P_677 ; _rtB -> B_40_807_0 = _rtB -> B_40_593_0 / _rtB ->
B_40_38_0_o ; _rtB -> B_40_808_0 = _rtB -> B_40_807_0 + _rtP -> P_678 ; _rtB
-> B_40_809_0 = _rtB -> B_40_611_0 / _rtB -> B_40_39_0_b ; _rtB -> B_40_810_0
= _rtB -> B_40_809_0 + _rtP -> P_679 ; _rtB -> B_40_811_0 = _rtB ->
B_40_629_0 / _rtB -> B_40_40_0_n ; _rtB -> B_40_812_0 = _rtB -> B_40_811_0 +
_rtP -> P_680 ; _rtB -> B_40_813_0 = _rtB -> B_40_647_0 / _rtB -> B_40_41_0_b
; _rtB -> B_40_814_0 = _rtB -> B_40_813_0 + _rtP -> P_681 ; _rtB ->
B_40_815_0 = _rtB -> B_40_665_0 / _rtB -> B_40_43_0_l ; _rtB -> B_40_816_0 =
_rtB -> B_40_815_0 + _rtP -> P_682 ; _rtB -> B_40_817_0 = _rtB -> B_40_683_0
/ _rtB -> B_40_44_0_h ; _rtB -> B_40_818_0 = _rtB -> B_40_817_0 + _rtP ->
P_683 ; _rtB -> B_40_819_0 = _rtB -> B_40_701_0 / _rtB -> B_40_45_0_b ; _rtB
-> B_40_820_0 = _rtB -> B_40_819_0 + _rtP -> P_684 ; _rtB -> B_40_821_0 =
_rtB -> B_40_719_0 / _rtB -> B_40_46_0_d ; _rtB -> B_40_822_0 = _rtB ->
B_40_821_0 + _rtP -> P_685 ; _rtB -> B_40_823_0 = _rtB -> B_40_737_0 / _rtB
-> B_40_47_0_e ; _rtB -> B_40_824_0 = _rtB -> B_40_823_0 + _rtP -> P_686 ;
_rtB -> B_40_738_0 [ 0 ] = _rtB -> B_40_746_0 ; _rtB -> B_40_738_0 [ 1 ] =
_rtB -> B_40_748_0 ; _rtB -> B_40_738_0 [ 2 ] = _rtB -> B_40_750_0 ; _rtB ->
B_40_738_0 [ 3 ] = _rtB -> B_40_752_0 ; _rtB -> B_40_738_0 [ 4 ] = _rtB ->
B_40_754_0 ; _rtB -> B_40_738_0 [ 5 ] = _rtB -> B_40_756_0 ; _rtB ->
B_40_738_0 [ 6 ] = _rtB -> B_40_758_0 ; _rtB -> B_40_738_0 [ 7 ] = _rtB ->
B_40_760_0 ; _rtB -> B_40_738_0 [ 8 ] = _rtB -> B_40_762_0 ; _rtB ->
B_40_738_0 [ 9 ] = _rtB -> B_40_764_0 ; _rtB -> B_40_738_0 [ 10 ] = _rtB ->
B_40_766_0 ; _rtB -> B_40_738_0 [ 11 ] = _rtB -> B_40_768_0 ; _rtB ->
B_40_738_0 [ 12 ] = _rtB -> B_40_770_0 ; _rtB -> B_40_738_0 [ 13 ] = _rtB ->
B_40_772_0 ; _rtB -> B_40_738_0 [ 14 ] = _rtB -> B_40_774_0 ; _rtB ->
B_40_738_0 [ 15 ] = _rtB -> B_40_776_0 ; _rtB -> B_40_738_0 [ 16 ] = _rtB ->
B_40_778_0 ; _rtB -> B_40_738_0 [ 17 ] = _rtB -> B_40_780_0 ; _rtB ->
B_40_738_0 [ 18 ] = _rtB -> B_40_782_0 ; _rtB -> B_40_738_0 [ 19 ] = _rtB ->
B_40_784_0 ; _rtB -> B_40_738_0 [ 20 ] = _rtB -> B_40_786_0 ; _rtB ->
B_40_738_0 [ 21 ] = _rtB -> B_40_788_0 ; _rtB -> B_40_738_0 [ 22 ] = _rtB ->
B_40_790_0 ; _rtB -> B_40_738_0 [ 23 ] = _rtB -> B_40_792_0 ; _rtB ->
B_40_738_0 [ 24 ] = _rtB -> B_40_794_0 ; _rtB -> B_40_738_0 [ 25 ] = _rtB ->
B_40_796_0 ; _rtB -> B_40_738_0 [ 26 ] = _rtB -> B_40_798_0 ; _rtB ->
B_40_738_0 [ 27 ] = _rtB -> B_40_800_0 ; _rtB -> B_40_738_0 [ 28 ] = _rtB ->
B_40_802_0 ; _rtB -> B_40_738_0 [ 29 ] = _rtB -> B_40_804_0 ; _rtB ->
B_40_738_0 [ 30 ] = _rtB -> B_40_806_0 ; _rtB -> B_40_738_0 [ 31 ] = _rtB ->
B_40_808_0 ; _rtB -> B_40_738_0 [ 32 ] = _rtB -> B_40_810_0 ; _rtB ->
B_40_738_0 [ 33 ] = _rtB -> B_40_812_0 ; _rtB -> B_40_738_0 [ 34 ] = _rtB ->
B_40_814_0 ; _rtB -> B_40_738_0 [ 35 ] = _rtB -> B_40_816_0 ; _rtB ->
B_40_738_0 [ 36 ] = _rtB -> B_40_818_0 ; _rtB -> B_40_738_0 [ 37 ] = _rtB ->
B_40_820_0 ; _rtB -> B_40_738_0 [ 38 ] = _rtB -> B_40_822_0 ; _rtB ->
B_40_738_0 [ 39 ] = _rtB -> B_40_824_0 ; ssCallAccelRunBlock ( S , 40 , 826 ,
SS_CALL_MDL_OUTPUTS ) ; ssCallAccelRunBlock ( S , 40 , 827 ,
SS_CALL_MDL_OUTPUTS ) ; _rtB -> B_40_828_0 = _rtX -> Integrator1_CSTATE ;
ssCallAccelRunBlock ( S , 40 , 829 , SS_CALL_MDL_OUTPUTS ) ;
ssCallAccelRunBlock ( S , 40 , 830 , SS_CALL_MDL_OUTPUTS ) ; _rtB ->
B_40_832_0 = ( ( ( _rtB -> B_40_35_0 + _rtB -> B_40_53_0 ) + _rtB ->
B_40_71_0 ) + _rtB -> B_40_89_0 ) + _rtB -> B_40_107_0 ; _rtB -> B_40_833_0 =
( ( ( _rtB -> B_40_125_0 + _rtB -> B_40_143_0 ) + _rtB -> B_40_161_0 ) + _rtB
-> B_40_179_0 ) + _rtB -> B_40_197_0 ; _rtB -> B_40_834_0 = ( ( ( _rtB ->
B_40_215_0 + _rtB -> B_40_233_0 ) + _rtB -> B_40_251_0 ) + _rtB -> B_40_269_0
) + _rtB -> B_40_287_0 ; _rtB -> B_40_835_0 = ( ( ( _rtB -> B_40_305_0 + _rtB
-> B_40_323_0 ) + _rtB -> B_40_341_0 ) + _rtB -> B_40_359_0 ) + _rtB ->
B_40_377_0 ; _rtB -> B_40_836_0 = ( ( ( _rtB -> B_40_395_0 + _rtB ->
B_40_413_0 ) + _rtB -> B_40_431_0 ) + _rtB -> B_40_449_0 ) + _rtB ->
B_40_467_0 ; _rtB -> B_40_837_0 = ( ( ( _rtB -> B_40_485_0 + _rtB ->
B_40_503_0 ) + _rtB -> B_40_521_0 ) + _rtB -> B_40_539_0 ) + _rtB ->
B_40_557_0 ; _rtB -> B_40_838_0 = ( ( ( _rtB -> B_40_575_0 + _rtB ->
B_40_593_0 ) + _rtB -> B_40_611_0 ) + _rtB -> B_40_629_0 ) + _rtB ->
B_40_647_0 ; _rtB -> B_40_839_0 = ( ( ( _rtB -> B_40_665_0 + _rtB ->
B_40_683_0 ) + _rtB -> B_40_701_0 ) + _rtB -> B_40_719_0 ) + _rtB ->
B_40_737_0 ; _rtB -> B_40_840_0 = ( ( ( ( ( ( _rtB -> B_40_832_0 + _rtB ->
B_40_833_0 ) + _rtB -> B_40_834_0 ) + _rtB -> B_40_835_0 ) + _rtB ->
B_40_836_0 ) + _rtB -> B_40_837_0 ) + _rtB -> B_40_838_0 ) + _rtB ->
B_40_839_0 ; if ( _rtB -> B_40_840_0 > _rtP -> P_691 ) { _rtB -> B_40_841_0 =
_rtB -> B_40_840_0 - _rtP -> P_691 ; } else if ( _rtB -> B_40_840_0 >= _rtP
-> P_690 ) { _rtB -> B_40_841_0 = 0.0 ; } else { _rtB -> B_40_841_0 = _rtB ->
B_40_840_0 - _rtP -> P_690 ; } _rtB -> B_40_843_0 = ( real_T ) ( _rtB ->
B_40_841_0 == Simulation_v2_rtC ( S ) -> B_40_1_0 ) * look1_binlxpw ( _rtB ->
B_40_20_0 , _rtP -> P_689 , _rtP -> P_688 , 7U ) ; _rtB -> B_40_845_0 = (
real_T ) ( _rtB -> B_40_841_0 != Simulation_v2_rtC ( S ) -> B_40_0_0 ) * _rtB
-> B_40_841_0 ; _rtB -> B_40_846_0 = _rtB -> B_40_843_0 + _rtB -> B_40_845_0
; ssCallAccelRunBlock ( S , 40 , 847 , SS_CALL_MDL_OUTPUTS ) ; _rtB ->
B_40_848_0 = _rtB -> B_40_846_0 / _rtB -> B_40_49_0_b ; ssCallAccelRunBlock (
S , 40 , 849 , SS_CALL_MDL_OUTPUTS ) ; ssCallAccelRunBlock ( S , 40 , 850 ,
SS_CALL_MDL_OUTPUTS ) ; UNUSED_PARAMETER ( tid ) ; } static void
mdlOutputsTID3 ( SimStruct * S , int_T tid ) { B_Simulation_v2_T * _rtB ;
P_Simulation_v2_T * _rtP ; _rtP = ( ( P_Simulation_v2_T * ) ssGetModelRtp ( S
) ) ; _rtB = ( ( B_Simulation_v2_T * ) _ssGetModelBlockIO ( S ) ) ; _rtB ->
B_40_0_0_m = _rtP -> P_693 ; _rtB -> B_40_1_0_c = _rtP -> P_694 ; _rtB ->
B_40_2_0 = _rtP -> P_695 ; _rtB -> B_40_3_0 = _rtP -> P_696 ; _rtB ->
B_40_4_0 = _rtP -> P_697 ; _rtB -> B_40_5_0 = _rtP -> P_698 ; _rtB ->
B_40_7_0_k = _rtP -> P_699 ; _rtB -> B_40_8_0 = _rtP -> P_700 ; _rtB ->
B_40_9_0 = _rtP -> P_701 ; _rtB -> B_40_10_0_c = _rtP -> P_702 ; _rtB ->
B_40_11_0_b = _rtP -> P_703 ; _rtB -> B_40_13_0 = _rtP -> P_704 ; _rtB ->
B_40_14_0_p = _rtP -> P_705 ; _rtB -> B_40_15_0 = _rtP -> P_706 ; _rtB ->
B_40_16_0 = _rtP -> P_707 ; _rtB -> B_40_17_0_c = _rtP -> P_708 ; _rtB ->
B_40_19_0_f = _rtP -> P_709 ; _rtB -> B_40_20_0_g = _rtP -> P_710 ; _rtB ->
B_40_21_0_g = _rtP -> P_711 ; _rtB -> B_40_22_0_m = _rtP -> P_712 ; _rtB ->
B_40_23_0_n = _rtP -> P_713 ; _rtB -> B_40_25_0_p = _rtP -> P_714 ; _rtB ->
B_40_26_0_l = _rtP -> P_715 ; _rtB -> B_40_27_0_j = _rtP -> P_716 ; _rtB ->
B_40_28_0_d = _rtP -> P_717 ; _rtB -> B_40_29_0_g = _rtP -> P_718 ; _rtB ->
B_40_31_0_l = _rtP -> P_719 ; _rtB -> B_40_32_0_d = _rtP -> P_720 ; _rtB ->
B_40_33_0 = _rtP -> P_721 ; _rtB -> B_40_34_0 = _rtP -> P_722 ; _rtB ->
B_40_35_0_d = _rtP -> P_723 ; _rtB -> B_40_37_0_l = _rtP -> P_724 ; _rtB ->
B_40_38_0_o = _rtP -> P_725 ; _rtB -> B_40_39_0_b = _rtP -> P_726 ; _rtB ->
B_40_40_0_n = _rtP -> P_727 ; _rtB -> B_40_41_0_b = _rtP -> P_728 ; _rtB ->
B_40_43_0_l = _rtP -> P_729 ; _rtB -> B_40_44_0_h = _rtP -> P_730 ; _rtB ->
B_40_45_0_b = _rtP -> P_731 ; _rtB -> B_40_46_0_d = _rtP -> P_732 ; _rtB ->
B_40_47_0_e = _rtP -> P_733 ; _rtB -> B_40_49_0_b = ( ( ( ( ( ( ( ( ( ( _rtB
-> B_40_1_0_c + _rtB -> B_40_2_0 ) + _rtB -> B_40_3_0 ) + _rtB -> B_40_4_0 )
+ _rtB -> B_40_5_0 ) + ( ( ( ( _rtB -> B_40_7_0_k + _rtB -> B_40_8_0 ) + _rtB
-> B_40_9_0 ) + _rtB -> B_40_10_0_c ) + _rtB -> B_40_11_0_b ) ) + ( ( ( (
_rtB -> B_40_13_0 + _rtB -> B_40_14_0_p ) + _rtB -> B_40_15_0 ) + _rtB ->
B_40_16_0 ) + _rtB -> B_40_17_0_c ) ) + ( ( ( ( _rtB -> B_40_19_0_f + _rtB ->
B_40_20_0_g ) + _rtB -> B_40_21_0_g ) + _rtB -> B_40_22_0_m ) + _rtB ->
B_40_23_0_n ) ) + ( ( ( ( _rtB -> B_40_25_0_p + _rtB -> B_40_26_0_l ) + _rtB
-> B_40_27_0_j ) + _rtB -> B_40_28_0_d ) + _rtB -> B_40_29_0_g ) ) + ( ( ( (
_rtB -> B_40_31_0_l + _rtB -> B_40_32_0_d ) + _rtB -> B_40_33_0 ) + _rtB ->
B_40_34_0 ) + _rtB -> B_40_35_0_d ) ) + ( ( ( ( _rtB -> B_40_37_0_l + _rtB ->
B_40_38_0_o ) + _rtB -> B_40_39_0_b ) + _rtB -> B_40_40_0_n ) + _rtB ->
B_40_41_0_b ) ) + ( ( ( ( _rtB -> B_40_43_0_l + _rtB -> B_40_44_0_h ) + _rtB
-> B_40_45_0_b ) + _rtB -> B_40_46_0_d ) + _rtB -> B_40_47_0_e ) ; _rtB ->
B_40_51_0 = _rtP -> P_734 * _rtB -> B_40_1_0_c ; _rtB -> B_40_52_0 = _rtP ->
P_735 * _rtB -> B_40_51_0 ; _rtB -> B_40_54_0_j = _rtP -> P_736 * _rtB ->
B_40_2_0 ; _rtB -> B_40_55_0_f = _rtP -> P_737 * _rtB -> B_40_54_0_j ; _rtB
-> B_40_57_0_a = _rtP -> P_738 * _rtB -> B_40_3_0 ; _rtB -> B_40_58_0_j =
_rtP -> P_739 * _rtB -> B_40_57_0_a ; _rtB -> B_40_60_0_j = _rtP -> P_740 *
_rtB -> B_40_4_0 ; _rtB -> B_40_61_0_o = _rtP -> P_741 * _rtB -> B_40_60_0_j
; _rtB -> B_40_63_0_n = _rtP -> P_742 * _rtB -> B_40_5_0 ; _rtB ->
B_40_64_0_i = _rtP -> P_743 * _rtB -> B_40_63_0_n ; _rtB -> B_40_66_0_o =
_rtP -> P_744 * _rtB -> B_40_7_0_k ; _rtB -> B_40_67_0_n = _rtP -> P_745 *
_rtB -> B_40_66_0_o ; _rtB -> B_40_69_0 = _rtP -> P_746 * _rtB -> B_40_8_0 ;
_rtB -> B_40_70_0 = _rtP -> P_747 * _rtB -> B_40_69_0 ; _rtB -> B_40_72_0_m =
_rtP -> P_748 * _rtB -> B_40_9_0 ; _rtB -> B_40_73_0_c = _rtP -> P_749 * _rtB
-> B_40_72_0_m ; _rtB -> B_40_75_0_m = _rtP -> P_750 * _rtB -> B_40_10_0_c ;
_rtB -> B_40_76_0_m = _rtP -> P_751 * _rtB -> B_40_75_0_m ; _rtB ->
B_40_78_0_j = _rtP -> P_752 * _rtB -> B_40_11_0_b ; _rtB -> B_40_79_0_h =
_rtP -> P_753 * _rtB -> B_40_78_0_j ; _rtB -> B_40_81_0_c = _rtP -> P_754 *
_rtB -> B_40_13_0 ; _rtB -> B_40_82_0_c = _rtP -> P_755 * _rtB -> B_40_81_0_c
; _rtB -> B_40_84_0_p = _rtP -> P_756 * _rtB -> B_40_14_0_p ; _rtB ->
B_40_85_0_p = _rtP -> P_757 * _rtB -> B_40_84_0_p ; _rtB -> B_40_87_0 = _rtP
-> P_758 * _rtB -> B_40_15_0 ; _rtB -> B_40_88_0 = _rtP -> P_759 * _rtB ->
B_40_87_0 ; _rtB -> B_40_90_0_a = _rtP -> P_760 * _rtB -> B_40_16_0 ; _rtB ->
B_40_91_0_e = _rtP -> P_761 * _rtB -> B_40_90_0_a ; _rtB -> B_40_93_0_a =
_rtP -> P_762 * _rtB -> B_40_17_0_c ; _rtB -> B_40_94_0_a = _rtP -> P_763 *
_rtB -> B_40_93_0_a ; _rtB -> B_40_96_0_i = _rtP -> P_764 * _rtB ->
B_40_19_0_f ; _rtB -> B_40_97_0_l = _rtP -> P_765 * _rtB -> B_40_96_0_i ;
_rtB -> B_40_99_0_o = _rtP -> P_766 * _rtB -> B_40_20_0_g ; _rtB ->
B_40_100_0_o = _rtP -> P_767 * _rtB -> B_40_99_0_o ; _rtB -> B_40_102_0_i =
_rtP -> P_768 * _rtB -> B_40_21_0_g ; _rtB -> B_40_103_0_f = _rtP -> P_769 *
_rtB -> B_40_102_0_i ; _rtB -> B_40_105_0 = _rtP -> P_770 * _rtB ->
B_40_22_0_m ; _rtB -> B_40_106_0 = _rtP -> P_771 * _rtB -> B_40_105_0 ; _rtB
-> B_40_108_0_i = _rtP -> P_772 * _rtB -> B_40_23_0_n ; _rtB -> B_40_109_0_f
= _rtP -> P_773 * _rtB -> B_40_108_0_i ; _rtB -> B_40_111_0_g = _rtP -> P_774
* _rtB -> B_40_25_0_p ; _rtB -> B_40_112_0_c = _rtP -> P_775 * _rtB ->
B_40_111_0_g ; _rtB -> B_40_114_0_o = _rtP -> P_776 * _rtB -> B_40_26_0_l ;
_rtB -> B_40_115_0_l = _rtP -> P_777 * _rtB -> B_40_114_0_o ; _rtB ->
B_40_117_0_m = _rtP -> P_778 * _rtB -> B_40_27_0_j ; _rtB -> B_40_118_0_m =
_rtP -> P_779 * _rtB -> B_40_117_0_m ; _rtB -> B_40_120_0_c = _rtP -> P_780 *
_rtB -> B_40_28_0_d ; _rtB -> B_40_121_0_f = _rtP -> P_781 * _rtB ->
B_40_120_0_c ; _rtB -> B_40_123_0 = _rtP -> P_782 * _rtB -> B_40_29_0_g ;
_rtB -> B_40_124_0 = _rtP -> P_783 * _rtB -> B_40_123_0 ; _rtB ->
B_40_126_0_p = _rtP -> P_784 * _rtB -> B_40_31_0_l ; _rtB -> B_40_127_0_e =
_rtP -> P_785 * _rtB -> B_40_126_0_p ; _rtB -> B_40_129_0_o = _rtP -> P_786 *
_rtB -> B_40_32_0_d ; _rtB -> B_40_130_0_h = _rtP -> P_787 * _rtB ->
B_40_129_0_o ; _rtB -> B_40_132_0_l = _rtP -> P_788 * _rtB -> B_40_33_0 ;
_rtB -> B_40_133_0_h = _rtP -> P_789 * _rtB -> B_40_132_0_l ; _rtB ->
B_40_135_0_m = _rtP -> P_790 * _rtB -> B_40_34_0 ; _rtB -> B_40_136_0_m =
_rtP -> P_791 * _rtB -> B_40_135_0_m ; _rtB -> B_40_138_0_h = _rtP -> P_792 *
_rtB -> B_40_35_0_d ; _rtB -> B_40_139_0_c = _rtP -> P_793 * _rtB ->
B_40_138_0_h ; _rtB -> B_40_141_0 = _rtP -> P_794 * _rtB -> B_40_37_0_l ;
_rtB -> B_40_142_0 = _rtP -> P_795 * _rtB -> B_40_141_0 ; _rtB ->
B_40_144_0_k = _rtP -> P_796 * _rtB -> B_40_38_0_o ; _rtB -> B_40_145_0_p =
_rtP -> P_797 * _rtB -> B_40_144_0_k ; _rtB -> B_40_147_0_p = _rtP -> P_798 *
_rtB -> B_40_39_0_b ; _rtB -> B_40_148_0_p = _rtP -> P_799 * _rtB ->
B_40_147_0_p ; _rtB -> B_40_150_0_a = _rtP -> P_800 * _rtB -> B_40_40_0_n ;
_rtB -> B_40_151_0_j = _rtP -> P_801 * _rtB -> B_40_150_0_a ; _rtB ->
B_40_153_0_e = _rtP -> P_802 * _rtB -> B_40_41_0_b ; _rtB -> B_40_154_0_o =
_rtP -> P_803 * _rtB -> B_40_153_0_e ; _rtB -> B_40_156_0_b = _rtP -> P_804 *
_rtB -> B_40_43_0_l ; _rtB -> B_40_157_0_a = _rtP -> P_805 * _rtB ->
B_40_156_0_b ; _rtB -> B_40_159_0 = _rtP -> P_806 * _rtB -> B_40_44_0_h ;
_rtB -> B_40_160_0 = _rtP -> P_807 * _rtB -> B_40_159_0 ; _rtB ->
B_40_162_0_g = _rtP -> P_808 * _rtB -> B_40_45_0_b ; _rtB -> B_40_163_0_e =
_rtP -> P_809 * _rtB -> B_40_162_0_g ; _rtB -> B_40_165_0_f = _rtP -> P_810 *
_rtB -> B_40_46_0_d ; _rtB -> B_40_166_0_h = _rtP -> P_811 * _rtB ->
B_40_165_0_f ; _rtB -> B_40_168_0_e = _rtP -> P_812 * _rtB -> B_40_47_0_e ;
_rtB -> B_40_169_0_c = _rtP -> P_813 * _rtB -> B_40_168_0_e ;
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
B_40_23_0 ; } else if ( _rtDW -> LastMajorTimeB == ( rtInf ) ) { _rtDW ->
LastMajorTimeB = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB = _rtB ->
B_40_23_0 ; } else if ( _rtDW -> LastMajorTimeA < _rtDW -> LastMajorTimeB ) {
_rtDW -> LastMajorTimeA = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA = _rtB ->
B_40_23_0 ; } else { _rtDW -> LastMajorTimeB = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYB = _rtB -> B_40_23_0 ; } _rtDW -> vv0at_IWORK = 0 ; { real_T *
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
TransportDelay_IWORK . Last , simTime - _rtP -> P_16 , tBuffer , uBuffer , (
NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK . Head ] = _rtB ->
B_40_19_0 ; } if ( _rtDW -> LastMajorTimeA_d == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_d = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_p = _rtB ->
B_40_41_0 ; } else if ( _rtDW -> LastMajorTimeB_m == ( rtInf ) ) { _rtDW ->
LastMajorTimeB_m = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_b = _rtB ->
B_40_41_0 ; } else if ( _rtDW -> LastMajorTimeA_d < _rtDW -> LastMajorTimeB_m
) { _rtDW -> LastMajorTimeA_d = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_p =
_rtB -> B_40_41_0 ; } else { _rtDW -> LastMajorTimeB_m = ssGetTaskTime ( S ,
0 ) ; _rtDW -> PrevYB_b = _rtB -> B_40_41_0 ; } _rtDW -> vv0at_IWORK_c = 0 ;
{ real_T * * uBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_b .
TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_b . TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ;
_rtDW -> TransportDelay_IWORK_j . Head = ( ( _rtDW -> TransportDelay_IWORK_j
. Head < ( _rtDW -> TransportDelay_IWORK_j . CircularBufSize - 1 ) ) ? (
_rtDW -> TransportDelay_IWORK_j . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_j . Head == _rtDW -> TransportDelay_IWORK_j . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_j . CircularBufSize , & _rtDW -> TransportDelay_IWORK_j
. Tail , & _rtDW -> TransportDelay_IWORK_j . Head , & _rtDW ->
TransportDelay_IWORK_j . Last , simTime - _rtP -> P_32 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_j .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_j . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_j . Head ] = _rtB ->
B_40_38_0 ; } if ( _rtDW -> LastMajorTimeA_l == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_l = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_k = _rtB ->
B_40_59_0 ; } else if ( _rtDW -> LastMajorTimeB_c == ( rtInf ) ) { _rtDW ->
LastMajorTimeB_c = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_c = _rtB ->
B_40_59_0 ; } else if ( _rtDW -> LastMajorTimeA_l < _rtDW -> LastMajorTimeB_c
) { _rtDW -> LastMajorTimeA_l = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_k =
_rtB -> B_40_59_0 ; } else { _rtDW -> LastMajorTimeB_c = ssGetTaskTime ( S ,
0 ) ; _rtDW -> PrevYB_c = _rtB -> B_40_59_0 ; } _rtDW -> vv0at_IWORK_a = 0 ;
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
TransportDelay_IWORK_b . Last , simTime - _rtP -> P_48 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_b .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_b . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_b . Head ] = _rtB ->
B_40_56_0 ; } if ( _rtDW -> LastMajorTimeA_g == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_g = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_o = _rtB ->
B_40_77_0 ; } else if ( _rtDW -> LastMajorTimeB_e == ( rtInf ) ) { _rtDW ->
LastMajorTimeB_e = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_e = _rtB ->
B_40_77_0 ; } else if ( _rtDW -> LastMajorTimeA_g < _rtDW -> LastMajorTimeB_e
) { _rtDW -> LastMajorTimeA_g = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_o =
_rtB -> B_40_77_0 ; } else { _rtDW -> LastMajorTimeB_e = ssGetTaskTime ( S ,
0 ) ; _rtDW -> PrevYB_e = _rtB -> B_40_77_0 ; } _rtDW -> vv0at_IWORK_j = 0 ;
{ real_T * * uBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_j .
TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_j . TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ;
_rtDW -> TransportDelay_IWORK_f . Head = ( ( _rtDW -> TransportDelay_IWORK_f
. Head < ( _rtDW -> TransportDelay_IWORK_f . CircularBufSize - 1 ) ) ? (
_rtDW -> TransportDelay_IWORK_f . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_f . Head == _rtDW -> TransportDelay_IWORK_f . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_f . CircularBufSize , & _rtDW -> TransportDelay_IWORK_f
. Tail , & _rtDW -> TransportDelay_IWORK_f . Head , & _rtDW ->
TransportDelay_IWORK_f . Last , simTime - _rtP -> P_64 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_f .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_f . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_f . Head ] = _rtB ->
B_40_74_0 ; } if ( _rtDW -> LastMajorTimeA_p == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_p = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_kx = _rtB ->
B_40_95_0 ; } else if ( _rtDW -> LastMajorTimeB_o == ( rtInf ) ) { _rtDW ->
LastMajorTimeB_o = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_l = _rtB ->
B_40_95_0 ; } else if ( _rtDW -> LastMajorTimeA_p < _rtDW -> LastMajorTimeB_o
) { _rtDW -> LastMajorTimeA_p = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_kx
= _rtB -> B_40_95_0 ; } else { _rtDW -> LastMajorTimeB_o = ssGetTaskTime ( S
, 0 ) ; _rtDW -> PrevYB_l = _rtB -> B_40_95_0 ; } _rtDW -> vv0at_IWORK_e = 0
; { real_T * * uBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_ba .
TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_ba . TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S )
; _rtDW -> TransportDelay_IWORK_p . Head = ( ( _rtDW ->
TransportDelay_IWORK_p . Head < ( _rtDW -> TransportDelay_IWORK_p .
CircularBufSize - 1 ) ) ? ( _rtDW -> TransportDelay_IWORK_p . Head + 1 ) : 0
) ; if ( _rtDW -> TransportDelay_IWORK_p . Head == _rtDW ->
TransportDelay_IWORK_p . Tail ) { if ( !
Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_p . CircularBufSize , & _rtDW -> TransportDelay_IWORK_p
. Tail , & _rtDW -> TransportDelay_IWORK_p . Head , & _rtDW ->
TransportDelay_IWORK_p . Last , simTime - _rtP -> P_80 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_p .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_p . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_p . Head ] = _rtB ->
B_40_92_0 ; } if ( _rtDW -> LastMajorTimeA_i == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_i = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_c = _rtB ->
B_40_113_0 ; } else if ( _rtDW -> LastMajorTimeB_b == ( rtInf ) ) { _rtDW ->
LastMajorTimeB_b = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_i = _rtB ->
B_40_113_0 ; } else if ( _rtDW -> LastMajorTimeA_i < _rtDW ->
LastMajorTimeB_b ) { _rtDW -> LastMajorTimeA_i = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYA_c = _rtB -> B_40_113_0 ; } else { _rtDW -> LastMajorTimeB_b =
ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_i = _rtB -> B_40_113_0 ; } _rtDW ->
vv0at_IWORK_cm = 0 ; { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_n . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T *
* ) & _rtDW -> TransportDelay_PWORK_n . TUbufferPtrs [ 1 ] ; real_T simTime =
ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_l . Head = ( ( _rtDW ->
TransportDelay_IWORK_l . Head < ( _rtDW -> TransportDelay_IWORK_l .
CircularBufSize - 1 ) ) ? ( _rtDW -> TransportDelay_IWORK_l . Head + 1 ) : 0
) ; if ( _rtDW -> TransportDelay_IWORK_l . Head == _rtDW ->
TransportDelay_IWORK_l . Tail ) { if ( !
Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_l . CircularBufSize , & _rtDW -> TransportDelay_IWORK_l
. Tail , & _rtDW -> TransportDelay_IWORK_l . Head , & _rtDW ->
TransportDelay_IWORK_l . Last , simTime - _rtP -> P_96 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_l .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_l . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_l . Head ] = _rtB ->
B_40_110_0 ; } if ( _rtDW -> LastMajorTimeA_e == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_e = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_i = _rtB ->
B_40_131_0 ; } else if ( _rtDW -> LastMajorTimeB_g == ( rtInf ) ) { _rtDW ->
LastMajorTimeB_g = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_bm = _rtB ->
B_40_131_0 ; } else if ( _rtDW -> LastMajorTimeA_e < _rtDW ->
LastMajorTimeB_g ) { _rtDW -> LastMajorTimeA_e = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYA_i = _rtB -> B_40_131_0 ; } else { _rtDW -> LastMajorTimeB_g =
ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_bm = _rtB -> B_40_131_0 ; } _rtDW
-> vv0at_IWORK_f = 0 ; { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_jr . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T
* * ) & _rtDW -> TransportDelay_PWORK_jr . TUbufferPtrs [ 1 ] ; real_T
simTime = ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_o . Head = ( ( _rtDW
-> TransportDelay_IWORK_o . Head < ( _rtDW -> TransportDelay_IWORK_o .
CircularBufSize - 1 ) ) ? ( _rtDW -> TransportDelay_IWORK_o . Head + 1 ) : 0
) ; if ( _rtDW -> TransportDelay_IWORK_o . Head == _rtDW ->
TransportDelay_IWORK_o . Tail ) { if ( !
Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_o . CircularBufSize , & _rtDW -> TransportDelay_IWORK_o
. Tail , & _rtDW -> TransportDelay_IWORK_o . Head , & _rtDW ->
TransportDelay_IWORK_o . Last , simTime - _rtP -> P_112 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_o .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_o . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_o . Head ] = _rtB ->
B_40_128_0 ; } if ( _rtDW -> LastMajorTimeA_g5 == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_g5 = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_p0 = _rtB ->
B_40_149_0 ; } else if ( _rtDW -> LastMajorTimeB_h == ( rtInf ) ) { _rtDW ->
LastMajorTimeB_h = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_eb = _rtB ->
B_40_149_0 ; } else if ( _rtDW -> LastMajorTimeA_g5 < _rtDW ->
LastMajorTimeB_h ) { _rtDW -> LastMajorTimeA_g5 = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYA_p0 = _rtB -> B_40_149_0 ; } else { _rtDW -> LastMajorTimeB_h
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_eb = _rtB -> B_40_149_0 ; } _rtDW
-> vv0at_IWORK_fh = 0 ; { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_f . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T *
* ) & _rtDW -> TransportDelay_PWORK_f . TUbufferPtrs [ 1 ] ; real_T simTime =
ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_d . Head = ( ( _rtDW ->
TransportDelay_IWORK_d . Head < ( _rtDW -> TransportDelay_IWORK_d .
CircularBufSize - 1 ) ) ? ( _rtDW -> TransportDelay_IWORK_d . Head + 1 ) : 0
) ; if ( _rtDW -> TransportDelay_IWORK_d . Head == _rtDW ->
TransportDelay_IWORK_d . Tail ) { if ( !
Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_d . CircularBufSize , & _rtDW -> TransportDelay_IWORK_d
. Tail , & _rtDW -> TransportDelay_IWORK_d . Head , & _rtDW ->
TransportDelay_IWORK_d . Last , simTime - _rtP -> P_128 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_d .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_d . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_d . Head ] = _rtB ->
B_40_146_0 ; } if ( _rtDW -> LastMajorTimeA_f == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_f = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_ih = _rtB ->
B_40_167_0 ; } else if ( _rtDW -> LastMajorTimeB_ga == ( rtInf ) ) { _rtDW ->
LastMajorTimeB_ga = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_bd = _rtB ->
B_40_167_0 ; } else if ( _rtDW -> LastMajorTimeA_f < _rtDW ->
LastMajorTimeB_ga ) { _rtDW -> LastMajorTimeA_f = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYA_ih = _rtB -> B_40_167_0 ; } else { _rtDW -> LastMajorTimeB_ga
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_bd = _rtB -> B_40_167_0 ; } _rtDW
-> vv0at_IWORK_p = 0 ; { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_p . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T *
* ) & _rtDW -> TransportDelay_PWORK_p . TUbufferPtrs [ 1 ] ; real_T simTime =
ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_ph . Head = ( ( _rtDW ->
TransportDelay_IWORK_ph . Head < ( _rtDW -> TransportDelay_IWORK_ph .
CircularBufSize - 1 ) ) ? ( _rtDW -> TransportDelay_IWORK_ph . Head + 1 ) : 0
) ; if ( _rtDW -> TransportDelay_IWORK_ph . Head == _rtDW ->
TransportDelay_IWORK_ph . Tail ) { if ( !
Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_ph . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_ph . Tail , & _rtDW -> TransportDelay_IWORK_ph . Head ,
& _rtDW -> TransportDelay_IWORK_ph . Last , simTime - _rtP -> P_144 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_ph . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_ph . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_ph . Head ] = _rtB -> B_40_164_0 ; } if ( _rtDW ->
LastMajorTimeA_h == ( rtInf ) ) { _rtDW -> LastMajorTimeA_h = ssGetTaskTime (
S , 0 ) ; _rtDW -> PrevYA_a = _rtB -> B_40_185_0 ; } else if ( _rtDW ->
LastMajorTimeB_c1 == ( rtInf ) ) { _rtDW -> LastMajorTimeB_c1 = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_k = _rtB -> B_40_185_0 ; } else if ( _rtDW ->
LastMajorTimeA_h < _rtDW -> LastMajorTimeB_c1 ) { _rtDW -> LastMajorTimeA_h =
ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_a = _rtB -> B_40_185_0 ; } else {
_rtDW -> LastMajorTimeB_c1 = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_k =
_rtB -> B_40_185_0 ; } _rtDW -> vv0at_IWORK_k = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_mp . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_mp .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_pr . Head = ( ( _rtDW -> TransportDelay_IWORK_pr . Head
< ( _rtDW -> TransportDelay_IWORK_pr . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_pr . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_pr . Head == _rtDW -> TransportDelay_IWORK_pr . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_pr . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_pr . Tail , & _rtDW -> TransportDelay_IWORK_pr . Head ,
& _rtDW -> TransportDelay_IWORK_pr . Last , simTime - _rtP -> P_160 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_pr . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_pr . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_pr . Head ] = _rtB -> B_40_182_0 ; } if ( _rtDW ->
LastMajorTimeA_lm == ( rtInf ) ) { _rtDW -> LastMajorTimeA_lm = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_j = _rtB -> B_40_203_0 ; } else if ( _rtDW ->
LastMajorTimeB_ei == ( rtInf ) ) { _rtDW -> LastMajorTimeB_ei = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_g = _rtB -> B_40_203_0 ; } else if ( _rtDW ->
LastMajorTimeA_lm < _rtDW -> LastMajorTimeB_ei ) { _rtDW -> LastMajorTimeA_lm
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_j = _rtB -> B_40_203_0 ; } else {
_rtDW -> LastMajorTimeB_ei = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_g =
_rtB -> B_40_203_0 ; } _rtDW -> vv0at_IWORK_ej = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_e . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_e . TUbufferPtrs
[ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_n .
Head = ( ( _rtDW -> TransportDelay_IWORK_n . Head < ( _rtDW ->
TransportDelay_IWORK_n . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_n . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_n . Head == _rtDW -> TransportDelay_IWORK_n . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_n . CircularBufSize , & _rtDW -> TransportDelay_IWORK_n
. Tail , & _rtDW -> TransportDelay_IWORK_n . Head , & _rtDW ->
TransportDelay_IWORK_n . Last , simTime - _rtP -> P_176 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_n .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_n . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_n . Head ] = _rtB ->
B_40_200_0 ; } if ( _rtDW -> LastMajorTimeA_n == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_n = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_h = _rtB ->
B_40_221_0 ; } else if ( _rtDW -> LastMajorTimeB_i == ( rtInf ) ) { _rtDW ->
LastMajorTimeB_i = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_f = _rtB ->
B_40_221_0 ; } else if ( _rtDW -> LastMajorTimeA_n < _rtDW ->
LastMajorTimeB_i ) { _rtDW -> LastMajorTimeA_n = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYA_h = _rtB -> B_40_221_0 ; } else { _rtDW -> LastMajorTimeB_i =
ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_f = _rtB -> B_40_221_0 ; } _rtDW ->
vv0at_IWORK_n = 0 ; { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_jx . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T
* * ) & _rtDW -> TransportDelay_PWORK_jx . TUbufferPtrs [ 1 ] ; real_T
simTime = ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_fm . Head = ( ( _rtDW
-> TransportDelay_IWORK_fm . Head < ( _rtDW -> TransportDelay_IWORK_fm .
CircularBufSize - 1 ) ) ? ( _rtDW -> TransportDelay_IWORK_fm . Head + 1 ) : 0
) ; if ( _rtDW -> TransportDelay_IWORK_fm . Head == _rtDW ->
TransportDelay_IWORK_fm . Tail ) { if ( !
Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_fm . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_fm . Tail , & _rtDW -> TransportDelay_IWORK_fm . Head ,
& _rtDW -> TransportDelay_IWORK_fm . Last , simTime - _rtP -> P_192 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_fm . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_fm . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_fm . Head ] = _rtB -> B_40_218_0 ; } if ( _rtDW ->
LastMajorTimeA_c == ( rtInf ) ) { _rtDW -> LastMajorTimeA_c = ssGetTaskTime (
S , 0 ) ; _rtDW -> PrevYA_f = _rtB -> B_40_239_0 ; } else if ( _rtDW ->
LastMajorTimeB_n == ( rtInf ) ) { _rtDW -> LastMajorTimeB_n = ssGetTaskTime (
S , 0 ) ; _rtDW -> PrevYB_il = _rtB -> B_40_239_0 ; } else if ( _rtDW ->
LastMajorTimeA_c < _rtDW -> LastMajorTimeB_n ) { _rtDW -> LastMajorTimeA_c =
ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_f = _rtB -> B_40_239_0 ; } else {
_rtDW -> LastMajorTimeB_n = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_il =
_rtB -> B_40_239_0 ; } _rtDW -> vv0at_IWORK_i = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_eq . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_eq .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_a . Head = ( ( _rtDW -> TransportDelay_IWORK_a . Head <
( _rtDW -> TransportDelay_IWORK_a . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_a . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_a . Head == _rtDW -> TransportDelay_IWORK_a . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_a . CircularBufSize , & _rtDW -> TransportDelay_IWORK_a
. Tail , & _rtDW -> TransportDelay_IWORK_a . Head , & _rtDW ->
TransportDelay_IWORK_a . Last , simTime - _rtP -> P_208 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_a .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_a . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_a . Head ] = _rtB ->
B_40_236_0 ; } if ( _rtDW -> LastMajorTimeA_a == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_a = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_h5 = _rtB ->
B_40_257_0 ; } else if ( _rtDW -> LastMajorTimeB_k == ( rtInf ) ) { _rtDW ->
LastMajorTimeB_k = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_o = _rtB ->
B_40_257_0 ; } else if ( _rtDW -> LastMajorTimeA_a < _rtDW ->
LastMajorTimeB_k ) { _rtDW -> LastMajorTimeA_a = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYA_h5 = _rtB -> B_40_257_0 ; } else { _rtDW -> LastMajorTimeB_k
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_o = _rtB -> B_40_257_0 ; } _rtDW
-> vv0at_IWORK_l = 0 ; { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_ja . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T
* * ) & _rtDW -> TransportDelay_PWORK_ja . TUbufferPtrs [ 1 ] ; real_T
simTime = ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_ov . Head = ( ( _rtDW
-> TransportDelay_IWORK_ov . Head < ( _rtDW -> TransportDelay_IWORK_ov .
CircularBufSize - 1 ) ) ? ( _rtDW -> TransportDelay_IWORK_ov . Head + 1 ) : 0
) ; if ( _rtDW -> TransportDelay_IWORK_ov . Head == _rtDW ->
TransportDelay_IWORK_ov . Tail ) { if ( !
Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_ov . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_ov . Tail , & _rtDW -> TransportDelay_IWORK_ov . Head ,
& _rtDW -> TransportDelay_IWORK_ov . Last , simTime - _rtP -> P_224 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_ov . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_ov . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_ov . Head ] = _rtB -> B_40_254_0 ; } if ( _rtDW ->
LastMajorTimeA_en == ( rtInf ) ) { _rtDW -> LastMajorTimeA_en = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_e = _rtB -> B_40_275_0 ; } else if ( _rtDW ->
LastMajorTimeB_nk == ( rtInf ) ) { _rtDW -> LastMajorTimeB_nk = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_la = _rtB -> B_40_275_0 ; } else if ( _rtDW ->
LastMajorTimeA_en < _rtDW -> LastMajorTimeB_nk ) { _rtDW -> LastMajorTimeA_en
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_e = _rtB -> B_40_275_0 ; } else {
_rtDW -> LastMajorTimeB_nk = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_la =
_rtB -> B_40_275_0 ; } _rtDW -> vv0at_IWORK_fj = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_g . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_g . TUbufferPtrs
[ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_dd .
Head = ( ( _rtDW -> TransportDelay_IWORK_dd . Head < ( _rtDW ->
TransportDelay_IWORK_dd . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_dd . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_dd . Head == _rtDW -> TransportDelay_IWORK_dd . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_dd . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_dd . Tail , & _rtDW -> TransportDelay_IWORK_dd . Head ,
& _rtDW -> TransportDelay_IWORK_dd . Last , simTime - _rtP -> P_240 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_dd . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_dd . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_dd . Head ] = _rtB -> B_40_272_0 ; } if ( _rtDW ->
LastMajorTimeA_k == ( rtInf ) ) { _rtDW -> LastMajorTimeA_k = ssGetTaskTime (
S , 0 ) ; _rtDW -> PrevYA_hx = _rtB -> B_40_293_0 ; } else if ( _rtDW ->
LastMajorTimeB_oq == ( rtInf ) ) { _rtDW -> LastMajorTimeB_oq = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_ih = _rtB -> B_40_293_0 ; } else if ( _rtDW ->
LastMajorTimeA_k < _rtDW -> LastMajorTimeB_oq ) { _rtDW -> LastMajorTimeA_k =
ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_hx = _rtB -> B_40_293_0 ; } else {
_rtDW -> LastMajorTimeB_oq = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_ih =
_rtB -> B_40_293_0 ; } _rtDW -> vv0at_IWORK_pd = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_l . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_l . TUbufferPtrs
[ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_az .
Head = ( ( _rtDW -> TransportDelay_IWORK_az . Head < ( _rtDW ->
TransportDelay_IWORK_az . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_az . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_az . Head == _rtDW -> TransportDelay_IWORK_az . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_az . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_az . Tail , & _rtDW -> TransportDelay_IWORK_az . Head ,
& _rtDW -> TransportDelay_IWORK_az . Last , simTime - _rtP -> P_256 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_az . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_az . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_az . Head ] = _rtB -> B_40_290_0 ; } if ( _rtDW ->
LastMajorTimeA_fz == ( rtInf ) ) { _rtDW -> LastMajorTimeA_fz = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_g = _rtB -> B_40_311_0 ; } else if ( _rtDW ->
LastMajorTimeB_mu == ( rtInf ) ) { _rtDW -> LastMajorTimeB_mu = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_cb = _rtB -> B_40_311_0 ; } else if ( _rtDW ->
LastMajorTimeA_fz < _rtDW -> LastMajorTimeB_mu ) { _rtDW -> LastMajorTimeA_fz
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_g = _rtB -> B_40_311_0 ; } else {
_rtDW -> LastMajorTimeB_mu = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_cb =
_rtB -> B_40_311_0 ; } _rtDW -> vv0at_IWORK_b = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_ez . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_ez .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_fy . Head = ( ( _rtDW -> TransportDelay_IWORK_fy . Head
< ( _rtDW -> TransportDelay_IWORK_fy . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_fy . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_fy . Head == _rtDW -> TransportDelay_IWORK_fy . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_fy . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_fy . Tail , & _rtDW -> TransportDelay_IWORK_fy . Head ,
& _rtDW -> TransportDelay_IWORK_fy . Last , simTime - _rtP -> P_272 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_fy . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_fy . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_fy . Head ] = _rtB -> B_40_308_0 ; } if ( _rtDW ->
LastMajorTimeA_eh == ( rtInf ) ) { _rtDW -> LastMajorTimeA_eh = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_d = _rtB -> B_40_329_0 ; } else if ( _rtDW ->
LastMajorTimeB_l == ( rtInf ) ) { _rtDW -> LastMajorTimeB_l = ssGetTaskTime (
S , 0 ) ; _rtDW -> PrevYB_a = _rtB -> B_40_329_0 ; } else if ( _rtDW ->
LastMajorTimeA_eh < _rtDW -> LastMajorTimeB_l ) { _rtDW -> LastMajorTimeA_eh
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_d = _rtB -> B_40_329_0 ; } else {
_rtDW -> LastMajorTimeB_l = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_a =
_rtB -> B_40_329_0 ; } _rtDW -> vv0at_IWORK_i1 = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_pe . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_pe .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_i . Head = ( ( _rtDW -> TransportDelay_IWORK_i . Head <
( _rtDW -> TransportDelay_IWORK_i . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_i . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_i . Head == _rtDW -> TransportDelay_IWORK_i . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_i . CircularBufSize , & _rtDW -> TransportDelay_IWORK_i
. Tail , & _rtDW -> TransportDelay_IWORK_i . Head , & _rtDW ->
TransportDelay_IWORK_i . Last , simTime - _rtP -> P_288 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_i .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_i . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_i . Head ] = _rtB ->
B_40_326_0 ; } if ( _rtDW -> LastMajorTimeA_eo == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_eo = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_g5 = _rtB ->
B_40_347_0 ; } else if ( _rtDW -> LastMajorTimeB_j == ( rtInf ) ) { _rtDW ->
LastMajorTimeB_j = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_d = _rtB ->
B_40_347_0 ; } else if ( _rtDW -> LastMajorTimeA_eo < _rtDW ->
LastMajorTimeB_j ) { _rtDW -> LastMajorTimeA_eo = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYA_g5 = _rtB -> B_40_347_0 ; } else { _rtDW -> LastMajorTimeB_j
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_d = _rtB -> B_40_347_0 ; } _rtDW
-> vv0at_IWORK_m = 0 ; { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_j4 . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T
* * ) & _rtDW -> TransportDelay_PWORK_j4 . TUbufferPtrs [ 1 ] ; real_T
simTime = ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_ax . Head = ( ( _rtDW
-> TransportDelay_IWORK_ax . Head < ( _rtDW -> TransportDelay_IWORK_ax .
CircularBufSize - 1 ) ) ? ( _rtDW -> TransportDelay_IWORK_ax . Head + 1 ) : 0
) ; if ( _rtDW -> TransportDelay_IWORK_ax . Head == _rtDW ->
TransportDelay_IWORK_ax . Tail ) { if ( !
Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_ax . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_ax . Tail , & _rtDW -> TransportDelay_IWORK_ax . Head ,
& _rtDW -> TransportDelay_IWORK_ax . Last , simTime - _rtP -> P_304 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_ax . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_ax . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_ax . Head ] = _rtB -> B_40_344_0 ; } if ( _rtDW ->
LastMajorTimeA_pp == ( rtInf ) ) { _rtDW -> LastMajorTimeA_pp = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_k3 = _rtB -> B_40_365_0 ; } else if ( _rtDW ->
LastMajorTimeB_kl == ( rtInf ) ) { _rtDW -> LastMajorTimeB_kl = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_m = _rtB -> B_40_365_0 ; } else if ( _rtDW ->
LastMajorTimeA_pp < _rtDW -> LastMajorTimeB_kl ) { _rtDW -> LastMajorTimeA_pp
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_k3 = _rtB -> B_40_365_0 ; } else
{ _rtDW -> LastMajorTimeB_kl = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_m =
_rtB -> B_40_365_0 ; } _rtDW -> vv0at_IWORK_kq = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_ny . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_ny .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_g . Head = ( ( _rtDW -> TransportDelay_IWORK_g . Head <
( _rtDW -> TransportDelay_IWORK_g . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_g . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_g . Head == _rtDW -> TransportDelay_IWORK_g . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_g . CircularBufSize , & _rtDW -> TransportDelay_IWORK_g
. Tail , & _rtDW -> TransportDelay_IWORK_g . Head , & _rtDW ->
TransportDelay_IWORK_g . Last , simTime - _rtP -> P_320 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_g .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_g . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_g . Head ] = _rtB ->
B_40_362_0 ; } if ( _rtDW -> LastMajorTimeA_b == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_b = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_i0 = _rtB ->
B_40_383_0 ; } else if ( _rtDW -> LastMajorTimeB_el == ( rtInf ) ) { _rtDW ->
LastMajorTimeB_el = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_m5 = _rtB ->
B_40_383_0 ; } else if ( _rtDW -> LastMajorTimeA_b < _rtDW ->
LastMajorTimeB_el ) { _rtDW -> LastMajorTimeA_b = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYA_i0 = _rtB -> B_40_383_0 ; } else { _rtDW -> LastMajorTimeB_el
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_m5 = _rtB -> B_40_383_0 ; } _rtDW
-> vv0at_IWORK_h = 0 ; { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_d . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T *
* ) & _rtDW -> TransportDelay_PWORK_d . TUbufferPtrs [ 1 ] ; real_T simTime =
ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_av . Head = ( ( _rtDW ->
TransportDelay_IWORK_av . Head < ( _rtDW -> TransportDelay_IWORK_av .
CircularBufSize - 1 ) ) ? ( _rtDW -> TransportDelay_IWORK_av . Head + 1 ) : 0
) ; if ( _rtDW -> TransportDelay_IWORK_av . Head == _rtDW ->
TransportDelay_IWORK_av . Tail ) { if ( !
Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_av . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_av . Tail , & _rtDW -> TransportDelay_IWORK_av . Head ,
& _rtDW -> TransportDelay_IWORK_av . Last , simTime - _rtP -> P_336 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_av . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_av . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_av . Head ] = _rtB -> B_40_380_0 ; } if ( _rtDW ->
LastMajorTimeA_gx == ( rtInf ) ) { _rtDW -> LastMajorTimeA_gx = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_ex = _rtB -> B_40_401_0 ; } else if ( _rtDW ->
LastMajorTimeB_ju == ( rtInf ) ) { _rtDW -> LastMajorTimeB_ju = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_ilg = _rtB -> B_40_401_0 ; } else if ( _rtDW ->
LastMajorTimeA_gx < _rtDW -> LastMajorTimeB_ju ) { _rtDW -> LastMajorTimeA_gx
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_ex = _rtB -> B_40_401_0 ; } else
{ _rtDW -> LastMajorTimeB_ju = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_ilg
= _rtB -> B_40_401_0 ; } _rtDW -> vv0at_IWORK_b0 = 0 ; { real_T * * uBuffer =
( real_T * * ) & _rtDW -> TransportDelay_PWORK_i . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_i .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_ly . Head = ( ( _rtDW -> TransportDelay_IWORK_ly . Head
< ( _rtDW -> TransportDelay_IWORK_ly . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_ly . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_ly . Head == _rtDW -> TransportDelay_IWORK_ly . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_ly . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_ly . Tail , & _rtDW -> TransportDelay_IWORK_ly . Head ,
& _rtDW -> TransportDelay_IWORK_ly . Last , simTime - _rtP -> P_352 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_ly . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_ly . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_ly . Head ] = _rtB -> B_40_398_0 ; } if ( _rtDW ->
LastMajorTimeA_hp == ( rtInf ) ) { _rtDW -> LastMajorTimeA_hp = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_l = _rtB -> B_40_419_0 ; } else if ( _rtDW ->
LastMajorTimeB_ct == ( rtInf ) ) { _rtDW -> LastMajorTimeB_ct = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_au = _rtB -> B_40_419_0 ; } else if ( _rtDW ->
LastMajorTimeA_hp < _rtDW -> LastMajorTimeB_ct ) { _rtDW -> LastMajorTimeA_hp
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_l = _rtB -> B_40_419_0 ; } else {
_rtDW -> LastMajorTimeB_ct = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_au =
_rtB -> B_40_419_0 ; } _rtDW -> vv0at_IWORK_la = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_pz . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_pz .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_f0 . Head = ( ( _rtDW -> TransportDelay_IWORK_f0 . Head
< ( _rtDW -> TransportDelay_IWORK_f0 . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_f0 . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_f0 . Head == _rtDW -> TransportDelay_IWORK_f0 . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_f0 . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_f0 . Tail , & _rtDW -> TransportDelay_IWORK_f0 . Head ,
& _rtDW -> TransportDelay_IWORK_f0 . Last , simTime - _rtP -> P_368 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_f0 . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_f0 . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_f0 . Head ] = _rtB -> B_40_416_0 ; } if ( _rtDW ->
LastMajorTimeA_it == ( rtInf ) ) { _rtDW -> LastMajorTimeA_it = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_go = _rtB -> B_40_437_0 ; } else if ( _rtDW ->
LastMajorTimeB_f == ( rtInf ) ) { _rtDW -> LastMajorTimeB_f = ssGetTaskTime (
S , 0 ) ; _rtDW -> PrevYB_fh = _rtB -> B_40_437_0 ; } else if ( _rtDW ->
LastMajorTimeA_it < _rtDW -> LastMajorTimeB_f ) { _rtDW -> LastMajorTimeA_it
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_go = _rtB -> B_40_437_0 ; } else
{ _rtDW -> LastMajorTimeB_f = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_fh =
_rtB -> B_40_437_0 ; } _rtDW -> vv0at_IWORK_c3 = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_k . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_k . TUbufferPtrs
[ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_pz .
Head = ( ( _rtDW -> TransportDelay_IWORK_pz . Head < ( _rtDW ->
TransportDelay_IWORK_pz . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_pz . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_pz . Head == _rtDW -> TransportDelay_IWORK_pz . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_pz . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_pz . Tail , & _rtDW -> TransportDelay_IWORK_pz . Head ,
& _rtDW -> TransportDelay_IWORK_pz . Last , simTime - _rtP -> P_384 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_pz . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_pz . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_pz . Head ] = _rtB -> B_40_434_0 ; } if ( _rtDW ->
LastMajorTimeA_j == ( rtInf ) ) { _rtDW -> LastMajorTimeA_j = ssGetTaskTime (
S , 0 ) ; _rtDW -> PrevYA_n = _rtB -> B_40_455_0 ; } else if ( _rtDW ->
LastMajorTimeB_b0 == ( rtInf ) ) { _rtDW -> LastMajorTimeB_b0 = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_p = _rtB -> B_40_455_0 ; } else if ( _rtDW ->
LastMajorTimeA_j < _rtDW -> LastMajorTimeB_b0 ) { _rtDW -> LastMajorTimeA_j =
ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_n = _rtB -> B_40_455_0 ; } else {
_rtDW -> LastMajorTimeB_b0 = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_p =
_rtB -> B_40_455_0 ; } _rtDW -> vv0at_IWORK_i2 = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_fl . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_fl .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_bv . Head = ( ( _rtDW -> TransportDelay_IWORK_bv . Head
< ( _rtDW -> TransportDelay_IWORK_bv . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_bv . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_bv . Head == _rtDW -> TransportDelay_IWORK_bv . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_bv . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_bv . Tail , & _rtDW -> TransportDelay_IWORK_bv . Head ,
& _rtDW -> TransportDelay_IWORK_bv . Last , simTime - _rtP -> P_400 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_bv . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_bv . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_bv . Head ] = _rtB -> B_40_452_0 ; } if ( _rtDW ->
LastMajorTimeA_c5 == ( rtInf ) ) { _rtDW -> LastMajorTimeA_c5 = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_ad = _rtB -> B_40_473_0 ; } else if ( _rtDW ->
LastMajorTimeB_cu == ( rtInf ) ) { _rtDW -> LastMajorTimeB_cu = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_ll = _rtB -> B_40_473_0 ; } else if ( _rtDW ->
LastMajorTimeA_c5 < _rtDW -> LastMajorTimeB_cu ) { _rtDW -> LastMajorTimeA_c5
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_ad = _rtB -> B_40_473_0 ; } else
{ _rtDW -> LastMajorTimeB_cu = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_ll =
_rtB -> B_40_473_0 ; } _rtDW -> vv0at_IWORK_he = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_iy . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_iy .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_m . Head = ( ( _rtDW -> TransportDelay_IWORK_m . Head <
( _rtDW -> TransportDelay_IWORK_m . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_m . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_m . Head == _rtDW -> TransportDelay_IWORK_m . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_m . CircularBufSize , & _rtDW -> TransportDelay_IWORK_m
. Tail , & _rtDW -> TransportDelay_IWORK_m . Head , & _rtDW ->
TransportDelay_IWORK_m . Last , simTime - _rtP -> P_416 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_m .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_m . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_m . Head ] = _rtB ->
B_40_470_0 ; } if ( _rtDW -> LastMajorTimeA_if == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_if = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_e5 = _rtB ->
B_40_491_0 ; } else if ( _rtDW -> LastMajorTimeB_id == ( rtInf ) ) { _rtDW ->
LastMajorTimeB_id = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_e3 = _rtB ->
B_40_491_0 ; } else if ( _rtDW -> LastMajorTimeA_if < _rtDW ->
LastMajorTimeB_id ) { _rtDW -> LastMajorTimeA_if = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYA_e5 = _rtB -> B_40_491_0 ; } else { _rtDW -> LastMajorTimeB_id
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_e3 = _rtB -> B_40_491_0 ; } _rtDW
-> vv0at_IWORK_hv = 0 ; { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_iu . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T
* * ) & _rtDW -> TransportDelay_PWORK_iu . TUbufferPtrs [ 1 ] ; real_T
simTime = ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_h . Head = ( ( _rtDW
-> TransportDelay_IWORK_h . Head < ( _rtDW -> TransportDelay_IWORK_h .
CircularBufSize - 1 ) ) ? ( _rtDW -> TransportDelay_IWORK_h . Head + 1 ) : 0
) ; if ( _rtDW -> TransportDelay_IWORK_h . Head == _rtDW ->
TransportDelay_IWORK_h . Tail ) { if ( !
Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_h . CircularBufSize , & _rtDW -> TransportDelay_IWORK_h
. Tail , & _rtDW -> TransportDelay_IWORK_h . Head , & _rtDW ->
TransportDelay_IWORK_h . Last , simTime - _rtP -> P_432 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_h .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_h . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_h . Head ] = _rtB ->
B_40_488_0 ; } if ( _rtDW -> LastMajorTimeA_n1 == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_n1 = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_m = _rtB ->
B_40_509_0 ; } else if ( _rtDW -> LastMajorTimeB_e5 == ( rtInf ) ) { _rtDW ->
LastMajorTimeB_e5 = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_j = _rtB ->
B_40_509_0 ; } else if ( _rtDW -> LastMajorTimeA_n1 < _rtDW ->
LastMajorTimeB_e5 ) { _rtDW -> LastMajorTimeA_n1 = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYA_m = _rtB -> B_40_509_0 ; } else { _rtDW -> LastMajorTimeB_e5
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_j = _rtB -> B_40_509_0 ; } _rtDW
-> vv0at_IWORK_g = 0 ; { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_a . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T *
* ) & _rtDW -> TransportDelay_PWORK_a . TUbufferPtrs [ 1 ] ; real_T simTime =
ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_i0 . Head = ( ( _rtDW ->
TransportDelay_IWORK_i0 . Head < ( _rtDW -> TransportDelay_IWORK_i0 .
CircularBufSize - 1 ) ) ? ( _rtDW -> TransportDelay_IWORK_i0 . Head + 1 ) : 0
) ; if ( _rtDW -> TransportDelay_IWORK_i0 . Head == _rtDW ->
TransportDelay_IWORK_i0 . Tail ) { if ( !
Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_i0 . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_i0 . Tail , & _rtDW -> TransportDelay_IWORK_i0 . Head ,
& _rtDW -> TransportDelay_IWORK_i0 . Last , simTime - _rtP -> P_448 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_i0 . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_i0 . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_i0 . Head ] = _rtB -> B_40_506_0 ; } if ( _rtDW ->
LastMajorTimeA_hk == ( rtInf ) ) { _rtDW -> LastMajorTimeA_hk = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_dn = _rtB -> B_40_527_0 ; } else if ( _rtDW ->
LastMajorTimeB_nm == ( rtInf ) ) { _rtDW -> LastMajorTimeB_nm = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_d2 = _rtB -> B_40_527_0 ; } else if ( _rtDW ->
LastMajorTimeA_hk < _rtDW -> LastMajorTimeB_nm ) { _rtDW -> LastMajorTimeA_hk
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_dn = _rtB -> B_40_527_0 ; } else
{ _rtDW -> LastMajorTimeB_nm = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_d2 =
_rtB -> B_40_527_0 ; } _rtDW -> vv0at_IWORK_fz = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_nb . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_nb .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_iy . Head = ( ( _rtDW -> TransportDelay_IWORK_iy . Head
< ( _rtDW -> TransportDelay_IWORK_iy . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_iy . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_iy . Head == _rtDW -> TransportDelay_IWORK_iy . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_iy . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_iy . Tail , & _rtDW -> TransportDelay_IWORK_iy . Head ,
& _rtDW -> TransportDelay_IWORK_iy . Last , simTime - _rtP -> P_464 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_iy . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_iy . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_iy . Head ] = _rtB -> B_40_524_0 ; } if ( _rtDW ->
LastMajorTimeA_bx == ( rtInf ) ) { _rtDW -> LastMajorTimeA_bx = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_ha = _rtB -> B_40_545_0 ; } else if ( _rtDW ->
LastMajorTimeB_l1 == ( rtInf ) ) { _rtDW -> LastMajorTimeB_l1 = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_m1 = _rtB -> B_40_545_0 ; } else if ( _rtDW ->
LastMajorTimeA_bx < _rtDW -> LastMajorTimeB_l1 ) { _rtDW -> LastMajorTimeA_bx
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_ha = _rtB -> B_40_545_0 ; } else
{ _rtDW -> LastMajorTimeB_l1 = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_m1 =
_rtB -> B_40_545_0 ; } _rtDW -> vv0at_IWORK_c2 = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_o . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_o . TUbufferPtrs
[ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_if .
Head = ( ( _rtDW -> TransportDelay_IWORK_if . Head < ( _rtDW ->
TransportDelay_IWORK_if . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_if . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_if . Head == _rtDW -> TransportDelay_IWORK_if . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_if . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_if . Tail , & _rtDW -> TransportDelay_IWORK_if . Head ,
& _rtDW -> TransportDelay_IWORK_if . Last , simTime - _rtP -> P_480 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_if . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_if . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_if . Head ] = _rtB -> B_40_542_0 ; } if ( _rtDW ->
LastMajorTimeA_m == ( rtInf ) ) { _rtDW -> LastMajorTimeA_m = ssGetTaskTime (
S , 0 ) ; _rtDW -> PrevYA_h2 = _rtB -> B_40_563_0 ; } else if ( _rtDW ->
LastMajorTimeB_hw == ( rtInf ) ) { _rtDW -> LastMajorTimeB_hw = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_ec = _rtB -> B_40_563_0 ; } else if ( _rtDW ->
LastMajorTimeA_m < _rtDW -> LastMajorTimeB_hw ) { _rtDW -> LastMajorTimeA_m =
ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_h2 = _rtB -> B_40_563_0 ; } else {
_rtDW -> LastMajorTimeB_hw = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_ec =
_rtB -> B_40_563_0 ; } _rtDW -> vv0at_IWORK_ee = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_a5 . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_a5 .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_dl . Head = ( ( _rtDW -> TransportDelay_IWORK_dl . Head
< ( _rtDW -> TransportDelay_IWORK_dl . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_dl . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_dl . Head == _rtDW -> TransportDelay_IWORK_dl . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_dl . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_dl . Tail , & _rtDW -> TransportDelay_IWORK_dl . Head ,
& _rtDW -> TransportDelay_IWORK_dl . Last , simTime - _rtP -> P_496 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_dl . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_dl . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_dl . Head ] = _rtB -> B_40_560_0 ; } if ( _rtDW ->
LastMajorTimeA_bu == ( rtInf ) ) { _rtDW -> LastMajorTimeA_bu = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_oi = _rtB -> B_40_581_0 ; } else if ( _rtDW ->
LastMajorTimeB_bj == ( rtInf ) ) { _rtDW -> LastMajorTimeB_bj = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_le = _rtB -> B_40_581_0 ; } else if ( _rtDW ->
LastMajorTimeA_bu < _rtDW -> LastMajorTimeB_bj ) { _rtDW -> LastMajorTimeA_bu
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_oi = _rtB -> B_40_581_0 ; } else
{ _rtDW -> LastMajorTimeB_bj = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_le =
_rtB -> B_40_581_0 ; } _rtDW -> vv0at_IWORK_e1 = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_ic . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_ic .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_i2 . Head = ( ( _rtDW -> TransportDelay_IWORK_i2 . Head
< ( _rtDW -> TransportDelay_IWORK_i2 . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_i2 . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_i2 . Head == _rtDW -> TransportDelay_IWORK_i2 . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_i2 . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_i2 . Tail , & _rtDW -> TransportDelay_IWORK_i2 . Head ,
& _rtDW -> TransportDelay_IWORK_i2 . Last , simTime - _rtP -> P_512 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_i2 . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_i2 . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_i2 . Head ] = _rtB -> B_40_578_0 ; } if ( _rtDW ->
LastMajorTimeA_m2 == ( rtInf ) ) { _rtDW -> LastMajorTimeA_m2 = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_py = _rtB -> B_40_599_0 ; } else if ( _rtDW ->
LastMajorTimeB_np == ( rtInf ) ) { _rtDW -> LastMajorTimeB_np = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_h = _rtB -> B_40_599_0 ; } else if ( _rtDW ->
LastMajorTimeA_m2 < _rtDW -> LastMajorTimeB_np ) { _rtDW -> LastMajorTimeA_m2
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_py = _rtB -> B_40_599_0 ; } else
{ _rtDW -> LastMajorTimeB_np = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_h =
_rtB -> B_40_599_0 ; } _rtDW -> vv0at_IWORK_l4 = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_lz . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_lz .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_ab . Head = ( ( _rtDW -> TransportDelay_IWORK_ab . Head
< ( _rtDW -> TransportDelay_IWORK_ab . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_ab . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_ab . Head == _rtDW -> TransportDelay_IWORK_ab . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_ab . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_ab . Tail , & _rtDW -> TransportDelay_IWORK_ab . Head ,
& _rtDW -> TransportDelay_IWORK_ab . Last , simTime - _rtP -> P_528 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_ab . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_ab . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_ab . Head ] = _rtB -> B_40_596_0 ; } if ( _rtDW ->
LastMajorTimeA_id == ( rtInf ) ) { _rtDW -> LastMajorTimeA_id = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_f5 = _rtB -> B_40_617_0 ; } else if ( _rtDW ->
LastMajorTimeB_oi == ( rtInf ) ) { _rtDW -> LastMajorTimeB_oi = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_ai = _rtB -> B_40_617_0 ; } else if ( _rtDW ->
LastMajorTimeA_id < _rtDW -> LastMajorTimeB_oi ) { _rtDW -> LastMajorTimeA_id
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_f5 = _rtB -> B_40_617_0 ; } else
{ _rtDW -> LastMajorTimeB_oi = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_ai =
_rtB -> B_40_617_0 ; } _rtDW -> vv0at_IWORK_d = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_c . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_c . TUbufferPtrs
[ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_k .
Head = ( ( _rtDW -> TransportDelay_IWORK_k . Head < ( _rtDW ->
TransportDelay_IWORK_k . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_k . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_k . Head == _rtDW -> TransportDelay_IWORK_k . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_k . CircularBufSize , & _rtDW -> TransportDelay_IWORK_k
. Tail , & _rtDW -> TransportDelay_IWORK_k . Head , & _rtDW ->
TransportDelay_IWORK_k . Last , simTime - _rtP -> P_544 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_k .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_k . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_k . Head ] = _rtB ->
B_40_614_0 ; } if ( _rtDW -> LastMajorTimeA_kn == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_kn = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_d5 = _rtB ->
B_40_635_0 ; } else if ( _rtDW -> LastMajorTimeB_he == ( rtInf ) ) { _rtDW ->
LastMajorTimeB_he = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_mp = _rtB ->
B_40_635_0 ; } else if ( _rtDW -> LastMajorTimeA_kn < _rtDW ->
LastMajorTimeB_he ) { _rtDW -> LastMajorTimeA_kn = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYA_d5 = _rtB -> B_40_635_0 ; } else { _rtDW -> LastMajorTimeB_he
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_mp = _rtB -> B_40_635_0 ; } _rtDW
-> vv0at_IWORK_d1 = 0 ; { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_or . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T
* * ) & _rtDW -> TransportDelay_PWORK_or . TUbufferPtrs [ 1 ] ; real_T
simTime = ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_c . Head = ( ( _rtDW
-> TransportDelay_IWORK_c . Head < ( _rtDW -> TransportDelay_IWORK_c .
CircularBufSize - 1 ) ) ? ( _rtDW -> TransportDelay_IWORK_c . Head + 1 ) : 0
) ; if ( _rtDW -> TransportDelay_IWORK_c . Head == _rtDW ->
TransportDelay_IWORK_c . Tail ) { if ( !
Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_c . CircularBufSize , & _rtDW -> TransportDelay_IWORK_c
. Tail , & _rtDW -> TransportDelay_IWORK_c . Head , & _rtDW ->
TransportDelay_IWORK_c . Last , simTime - _rtP -> P_560 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_c .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_c . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_c . Head ] = _rtB ->
B_40_632_0 ; } if ( _rtDW -> LastMajorTimeA_pu == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_pu = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_gf = _rtB ->
B_40_653_0 ; } else if ( _rtDW -> LastMajorTimeB_nmm == ( rtInf ) ) { _rtDW
-> LastMajorTimeB_nmm = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_ic = _rtB
-> B_40_653_0 ; } else if ( _rtDW -> LastMajorTimeA_pu < _rtDW ->
LastMajorTimeB_nmm ) { _rtDW -> LastMajorTimeA_pu = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYA_gf = _rtB -> B_40_653_0 ; } else { _rtDW ->
LastMajorTimeB_nmm = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_ic = _rtB ->
B_40_653_0 ; } _rtDW -> vv0at_IWORK_am = 0 ; { real_T * * uBuffer = ( real_T
* * ) & _rtDW -> TransportDelay_PWORK_m4 . TUbufferPtrs [ 0 ] ; real_T * *
tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_m4 . TUbufferPtrs [
1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_g4 . Head
= ( ( _rtDW -> TransportDelay_IWORK_g4 . Head < ( _rtDW ->
TransportDelay_IWORK_g4 . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_g4 . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_g4 . Head == _rtDW -> TransportDelay_IWORK_g4 . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_g4 . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_g4 . Tail , & _rtDW -> TransportDelay_IWORK_g4 . Head ,
& _rtDW -> TransportDelay_IWORK_g4 . Last , simTime - _rtP -> P_576 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_g4 . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_g4 . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_g4 . Head ] = _rtB -> B_40_650_0 ; } if ( _rtDW ->
LastMajorTimeA_hx == ( rtInf ) ) { _rtDW -> LastMajorTimeA_hx = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_av = _rtB -> B_40_671_0 ; } else if ( _rtDW ->
LastMajorTimeB_ho == ( rtInf ) ) { _rtDW -> LastMajorTimeB_ho = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_pl = _rtB -> B_40_671_0 ; } else if ( _rtDW ->
LastMajorTimeA_hx < _rtDW -> LastMajorTimeB_ho ) { _rtDW -> LastMajorTimeA_hx
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_av = _rtB -> B_40_671_0 ; } else
{ _rtDW -> LastMajorTimeB_ho = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_pl =
_rtB -> B_40_671_0 ; } _rtDW -> vv0at_IWORK_ks = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_ea . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_ea .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_ovh . Head = ( ( _rtDW -> TransportDelay_IWORK_ovh .
Head < ( _rtDW -> TransportDelay_IWORK_ovh . CircularBufSize - 1 ) ) ? (
_rtDW -> TransportDelay_IWORK_ovh . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_ovh . Head == _rtDW -> TransportDelay_IWORK_ovh . Tail )
{ if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_ovh . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_ovh . Tail , & _rtDW -> TransportDelay_IWORK_ovh . Head
, & _rtDW -> TransportDelay_IWORK_ovh . Last , simTime - _rtP -> P_592 ,
tBuffer , uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_ovh . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_ovh . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_ovh . Head ] = _rtB -> B_40_668_0 ; } if ( _rtDW ->
LastMajorTimeA_l1 == ( rtInf ) ) { _rtDW -> LastMajorTimeA_l1 = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_co = _rtB -> B_40_689_0 ; } else if ( _rtDW ->
LastMajorTimeB_en == ( rtInf ) ) { _rtDW -> LastMajorTimeB_en = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_ilc = _rtB -> B_40_689_0 ; } else if ( _rtDW ->
LastMajorTimeA_l1 < _rtDW -> LastMajorTimeB_en ) { _rtDW -> LastMajorTimeA_l1
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_co = _rtB -> B_40_689_0 ; } else
{ _rtDW -> LastMajorTimeB_en = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_ilc
= _rtB -> B_40_689_0 ; } _rtDW -> vv0at_IWORK_gs = 0 ; { real_T * * uBuffer =
( real_T * * ) & _rtDW -> TransportDelay_PWORK_d2 . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_d2 .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_i5 . Head = ( ( _rtDW -> TransportDelay_IWORK_i5 . Head
< ( _rtDW -> TransportDelay_IWORK_i5 . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_i5 . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_i5 . Head == _rtDW -> TransportDelay_IWORK_i5 . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_i5 . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_i5 . Tail , & _rtDW -> TransportDelay_IWORK_i5 . Head ,
& _rtDW -> TransportDelay_IWORK_i5 . Last , simTime - _rtP -> P_608 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_i5 . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_i5 . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_i5 . Head ] = _rtB -> B_40_686_0 ; } if ( _rtDW ->
LastMajorTimeA_c3 == ( rtInf ) ) { _rtDW -> LastMajorTimeA_c3 = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_dq = _rtB -> B_40_707_0 ; } else if ( _rtDW ->
LastMajorTimeB_oy == ( rtInf ) ) { _rtDW -> LastMajorTimeB_oy = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_av = _rtB -> B_40_707_0 ; } else if ( _rtDW ->
LastMajorTimeA_c3 < _rtDW -> LastMajorTimeB_oy ) { _rtDW -> LastMajorTimeA_c3
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_dq = _rtB -> B_40_707_0 ; } else
{ _rtDW -> LastMajorTimeB_oy = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_av =
_rtB -> B_40_707_0 ; } _rtDW -> vv0at_IWORK_dy = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_kq . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_kq .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_ns . Head = ( ( _rtDW -> TransportDelay_IWORK_ns . Head
< ( _rtDW -> TransportDelay_IWORK_ns . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_ns . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_ns . Head == _rtDW -> TransportDelay_IWORK_ns . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_ns . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_ns . Tail , & _rtDW -> TransportDelay_IWORK_ns . Head ,
& _rtDW -> TransportDelay_IWORK_ns . Last , simTime - _rtP -> P_624 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_ns . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_ns . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_ns . Head ] = _rtB -> B_40_704_0 ; } if ( _rtDW ->
LastMajorTimeA_la == ( rtInf ) ) { _rtDW -> LastMajorTimeA_la = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_hv = _rtB -> B_40_725_0 ; } else if ( _rtDW ->
LastMajorTimeB_iq == ( rtInf ) ) { _rtDW -> LastMajorTimeB_iq = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_as = _rtB -> B_40_725_0 ; } else if ( _rtDW ->
LastMajorTimeA_la < _rtDW -> LastMajorTimeB_iq ) { _rtDW -> LastMajorTimeA_la
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_hv = _rtB -> B_40_725_0 ; } else
{ _rtDW -> LastMajorTimeB_iq = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_as =
_rtB -> B_40_725_0 ; } _rtDW -> vv0at_IWORK_de = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_ju . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_ju .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_gm . Head = ( ( _rtDW -> TransportDelay_IWORK_gm . Head
< ( _rtDW -> TransportDelay_IWORK_gm . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_gm . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_gm . Head == _rtDW -> TransportDelay_IWORK_gm . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_gm . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_gm . Tail , & _rtDW -> TransportDelay_IWORK_gm . Head ,
& _rtDW -> TransportDelay_IWORK_gm . Last , simTime - _rtP -> P_640 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_gm . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_gm . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_gm . Head ] = _rtB -> B_40_722_0 ; } UNUSED_PARAMETER (
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
B_40_848_0 ; _rtXdot -> vv0at_CSTATE = _rtB -> B_40_746_0 ; _rtXdot ->
TransferFcn_CSTATE = 0.0 ; _rtXdot -> TransferFcn_CSTATE += _rtP -> P_18 *
_rtX -> TransferFcn_CSTATE ; _rtXdot -> TransferFcn_CSTATE += _rtB ->
B_40_36_0 ; _rtXdot -> vv0at_CSTATE_f = _rtB -> B_40_748_0 ; _rtXdot ->
TransferFcn_CSTATE_m = 0.0 ; _rtXdot -> TransferFcn_CSTATE_m += _rtP -> P_34
* _rtX -> TransferFcn_CSTATE_m ; _rtXdot -> TransferFcn_CSTATE_m += _rtB ->
B_40_54_0 ; _rtXdot -> vv0at_CSTATE_b = _rtB -> B_40_750_0 ; _rtXdot ->
TransferFcn_CSTATE_d = 0.0 ; _rtXdot -> TransferFcn_CSTATE_d += _rtP -> P_50
* _rtX -> TransferFcn_CSTATE_d ; _rtXdot -> TransferFcn_CSTATE_d += _rtB ->
B_40_72_0 ; _rtXdot -> vv0at_CSTATE_m = _rtB -> B_40_752_0 ; _rtXdot ->
TransferFcn_CSTATE_k = 0.0 ; _rtXdot -> TransferFcn_CSTATE_k += _rtP -> P_66
* _rtX -> TransferFcn_CSTATE_k ; _rtXdot -> TransferFcn_CSTATE_k += _rtB ->
B_40_90_0 ; _rtXdot -> vv0at_CSTATE_j = _rtB -> B_40_754_0 ; _rtXdot ->
TransferFcn_CSTATE_j = 0.0 ; _rtXdot -> TransferFcn_CSTATE_j += _rtP -> P_82
* _rtX -> TransferFcn_CSTATE_j ; _rtXdot -> TransferFcn_CSTATE_j += _rtB ->
B_40_108_0 ; _rtXdot -> vv0at_CSTATE_fz = _rtB -> B_40_756_0 ; _rtXdot ->
TransferFcn_CSTATE_p = 0.0 ; _rtXdot -> TransferFcn_CSTATE_p += _rtP -> P_98
* _rtX -> TransferFcn_CSTATE_p ; _rtXdot -> TransferFcn_CSTATE_p += _rtB ->
B_40_126_0 ; _rtXdot -> vv0at_CSTATE_n = _rtB -> B_40_758_0 ; _rtXdot ->
TransferFcn_CSTATE_mr = 0.0 ; _rtXdot -> TransferFcn_CSTATE_mr += _rtP ->
P_114 * _rtX -> TransferFcn_CSTATE_mr ; _rtXdot -> TransferFcn_CSTATE_mr +=
_rtB -> B_40_144_0 ; _rtXdot -> vv0at_CSTATE_g = _rtB -> B_40_760_0 ; _rtXdot
-> TransferFcn_CSTATE_l = 0.0 ; _rtXdot -> TransferFcn_CSTATE_l += _rtP ->
P_130 * _rtX -> TransferFcn_CSTATE_l ; _rtXdot -> TransferFcn_CSTATE_l +=
_rtB -> B_40_162_0 ; _rtXdot -> vv0at_CSTATE_nh = _rtB -> B_40_762_0 ;
_rtXdot -> TransferFcn_CSTATE_c = 0.0 ; _rtXdot -> TransferFcn_CSTATE_c +=
_rtP -> P_146 * _rtX -> TransferFcn_CSTATE_c ; _rtXdot ->
TransferFcn_CSTATE_c += _rtB -> B_40_180_0 ; _rtXdot -> vv0at_CSTATE_p = _rtB
-> B_40_764_0 ; _rtXdot -> TransferFcn_CSTATE_dt = 0.0 ; _rtXdot ->
TransferFcn_CSTATE_dt += _rtP -> P_162 * _rtX -> TransferFcn_CSTATE_dt ;
_rtXdot -> TransferFcn_CSTATE_dt += _rtB -> B_40_198_0 ; _rtXdot ->
vv0at_CSTATE_l = _rtB -> B_40_766_0 ; _rtXdot -> TransferFcn_CSTATE_i = 0.0 ;
_rtXdot -> TransferFcn_CSTATE_i += _rtP -> P_178 * _rtX ->
TransferFcn_CSTATE_i ; _rtXdot -> TransferFcn_CSTATE_i += _rtB -> B_40_216_0
; _rtXdot -> vv0at_CSTATE_c = _rtB -> B_40_768_0 ; _rtXdot ->
TransferFcn_CSTATE_n = 0.0 ; _rtXdot -> TransferFcn_CSTATE_n += _rtP -> P_194
* _rtX -> TransferFcn_CSTATE_n ; _rtXdot -> TransferFcn_CSTATE_n += _rtB ->
B_40_234_0 ; _rtXdot -> vv0at_CSTATE_h = _rtB -> B_40_770_0 ; _rtXdot ->
TransferFcn_CSTATE_md = 0.0 ; _rtXdot -> TransferFcn_CSTATE_md += _rtP ->
P_210 * _rtX -> TransferFcn_CSTATE_md ; _rtXdot -> TransferFcn_CSTATE_md +=
_rtB -> B_40_252_0 ; _rtXdot -> vv0at_CSTATE_k = _rtB -> B_40_772_0 ; _rtXdot
-> TransferFcn_CSTATE_nd = 0.0 ; _rtXdot -> TransferFcn_CSTATE_nd += _rtP ->
P_226 * _rtX -> TransferFcn_CSTATE_nd ; _rtXdot -> TransferFcn_CSTATE_nd +=
_rtB -> B_40_270_0 ; _rtXdot -> vv0at_CSTATE_l4 = _rtB -> B_40_774_0 ;
_rtXdot -> TransferFcn_CSTATE_g = 0.0 ; _rtXdot -> TransferFcn_CSTATE_g +=
_rtP -> P_242 * _rtX -> TransferFcn_CSTATE_g ; _rtXdot ->
TransferFcn_CSTATE_g += _rtB -> B_40_288_0 ; _rtXdot -> vv0at_CSTATE_md =
_rtB -> B_40_776_0 ; _rtXdot -> TransferFcn_CSTATE_mf = 0.0 ; _rtXdot ->
TransferFcn_CSTATE_mf += _rtP -> P_258 * _rtX -> TransferFcn_CSTATE_mf ;
_rtXdot -> TransferFcn_CSTATE_mf += _rtB -> B_40_306_0 ; _rtXdot ->
vv0at_CSTATE_j1 = _rtB -> B_40_778_0 ; _rtXdot -> TransferFcn_CSTATE_m5 = 0.0
; _rtXdot -> TransferFcn_CSTATE_m5 += _rtP -> P_274 * _rtX ->
TransferFcn_CSTATE_m5 ; _rtXdot -> TransferFcn_CSTATE_m5 += _rtB ->
B_40_324_0 ; _rtXdot -> vv0at_CSTATE_o = _rtB -> B_40_780_0 ; _rtXdot ->
TransferFcn_CSTATE_e = 0.0 ; _rtXdot -> TransferFcn_CSTATE_e += _rtP -> P_290
* _rtX -> TransferFcn_CSTATE_e ; _rtXdot -> TransferFcn_CSTATE_e += _rtB ->
B_40_342_0 ; _rtXdot -> vv0at_CSTATE_d = _rtB -> B_40_782_0 ; _rtXdot ->
TransferFcn_CSTATE_f = 0.0 ; _rtXdot -> TransferFcn_CSTATE_f += _rtP -> P_306
* _rtX -> TransferFcn_CSTATE_f ; _rtXdot -> TransferFcn_CSTATE_f += _rtB ->
B_40_360_0 ; _rtXdot -> vv0at_CSTATE_kc = _rtB -> B_40_784_0 ; _rtXdot ->
TransferFcn_CSTATE_g1 = 0.0 ; _rtXdot -> TransferFcn_CSTATE_g1 += _rtP ->
P_322 * _rtX -> TransferFcn_CSTATE_g1 ; _rtXdot -> TransferFcn_CSTATE_g1 +=
_rtB -> B_40_378_0 ; _rtXdot -> vv0at_CSTATE_e = _rtB -> B_40_786_0 ; _rtXdot
-> TransferFcn_CSTATE_cx = 0.0 ; _rtXdot -> TransferFcn_CSTATE_cx += _rtP ->
P_338 * _rtX -> TransferFcn_CSTATE_cx ; _rtXdot -> TransferFcn_CSTATE_cx +=
_rtB -> B_40_396_0 ; _rtXdot -> vv0at_CSTATE_a = _rtB -> B_40_788_0 ; _rtXdot
-> TransferFcn_CSTATE_jb = 0.0 ; _rtXdot -> TransferFcn_CSTATE_jb += _rtP ->
P_354 * _rtX -> TransferFcn_CSTATE_jb ; _rtXdot -> TransferFcn_CSTATE_jb +=
_rtB -> B_40_414_0 ; _rtXdot -> vv0at_CSTATE_c4 = _rtB -> B_40_790_0 ;
_rtXdot -> TransferFcn_CSTATE_g3 = 0.0 ; _rtXdot -> TransferFcn_CSTATE_g3 +=
_rtP -> P_370 * _rtX -> TransferFcn_CSTATE_g3 ; _rtXdot ->
TransferFcn_CSTATE_g3 += _rtB -> B_40_432_0 ; _rtXdot -> vv0at_CSTATE_ba =
_rtB -> B_40_792_0 ; _rtXdot -> TransferFcn_CSTATE_jw = 0.0 ; _rtXdot ->
TransferFcn_CSTATE_jw += _rtP -> P_386 * _rtX -> TransferFcn_CSTATE_jw ;
_rtXdot -> TransferFcn_CSTATE_jw += _rtB -> B_40_450_0 ; _rtXdot ->
vv0at_CSTATE_cm = _rtB -> B_40_794_0 ; _rtXdot -> TransferFcn_CSTATE_pt = 0.0
; _rtXdot -> TransferFcn_CSTATE_pt += _rtP -> P_402 * _rtX ->
TransferFcn_CSTATE_pt ; _rtXdot -> TransferFcn_CSTATE_pt += _rtB ->
B_40_468_0 ; _rtXdot -> vv0at_CSTATE_lc = _rtB -> B_40_796_0 ; _rtXdot ->
TransferFcn_CSTATE_jn = 0.0 ; _rtXdot -> TransferFcn_CSTATE_jn += _rtP ->
P_418 * _rtX -> TransferFcn_CSTATE_jn ; _rtXdot -> TransferFcn_CSTATE_jn +=
_rtB -> B_40_486_0 ; _rtXdot -> vv0at_CSTATE_n1 = _rtB -> B_40_798_0 ;
_rtXdot -> TransferFcn_CSTATE_mu = 0.0 ; _rtXdot -> TransferFcn_CSTATE_mu +=
_rtP -> P_434 * _rtX -> TransferFcn_CSTATE_mu ; _rtXdot ->
TransferFcn_CSTATE_mu += _rtB -> B_40_504_0 ; _rtXdot -> vv0at_CSTATE_nu =
_rtB -> B_40_800_0 ; _rtXdot -> TransferFcn_CSTATE_cr = 0.0 ; _rtXdot ->
TransferFcn_CSTATE_cr += _rtP -> P_450 * _rtX -> TransferFcn_CSTATE_cr ;
_rtXdot -> TransferFcn_CSTATE_cr += _rtB -> B_40_522_0 ; _rtXdot ->
vv0at_CSTATE_ln = _rtB -> B_40_802_0 ; _rtXdot -> TransferFcn_CSTATE_h = 0.0
; _rtXdot -> TransferFcn_CSTATE_h += _rtP -> P_466 * _rtX ->
TransferFcn_CSTATE_h ; _rtXdot -> TransferFcn_CSTATE_h += _rtB -> B_40_540_0
; _rtXdot -> vv0at_CSTATE_fh = _rtB -> B_40_804_0 ; _rtXdot ->
TransferFcn_CSTATE_cp = 0.0 ; _rtXdot -> TransferFcn_CSTATE_cp += _rtP ->
P_482 * _rtX -> TransferFcn_CSTATE_cp ; _rtXdot -> TransferFcn_CSTATE_cp +=
_rtB -> B_40_558_0 ; _rtXdot -> vv0at_CSTATE_cx = _rtB -> B_40_806_0 ;
_rtXdot -> TransferFcn_CSTATE_ji = 0.0 ; _rtXdot -> TransferFcn_CSTATE_ji +=
_rtP -> P_498 * _rtX -> TransferFcn_CSTATE_ji ; _rtXdot ->
TransferFcn_CSTATE_ji += _rtB -> B_40_576_0 ; _rtXdot -> vv0at_CSTATE_pp =
_rtB -> B_40_808_0 ; _rtXdot -> TransferFcn_CSTATE_hl = 0.0 ; _rtXdot ->
TransferFcn_CSTATE_hl += _rtP -> P_514 * _rtX -> TransferFcn_CSTATE_hl ;
_rtXdot -> TransferFcn_CSTATE_hl += _rtB -> B_40_594_0 ; _rtXdot ->
vv0at_CSTATE_i = _rtB -> B_40_810_0 ; _rtXdot -> TransferFcn_CSTATE_ho = 0.0
; _rtXdot -> TransferFcn_CSTATE_ho += _rtP -> P_530 * _rtX ->
TransferFcn_CSTATE_ho ; _rtXdot -> TransferFcn_CSTATE_ho += _rtB ->
B_40_612_0 ; _rtXdot -> vv0at_CSTATE_be = _rtB -> B_40_812_0 ; _rtXdot ->
TransferFcn_CSTATE_kb = 0.0 ; _rtXdot -> TransferFcn_CSTATE_kb += _rtP ->
P_546 * _rtX -> TransferFcn_CSTATE_kb ; _rtXdot -> TransferFcn_CSTATE_kb +=
_rtB -> B_40_630_0 ; _rtXdot -> vv0at_CSTATE_l2 = _rtB -> B_40_814_0 ;
_rtXdot -> TransferFcn_CSTATE_ke = 0.0 ; _rtXdot -> TransferFcn_CSTATE_ke +=
_rtP -> P_562 * _rtX -> TransferFcn_CSTATE_ke ; _rtXdot ->
TransferFcn_CSTATE_ke += _rtB -> B_40_648_0 ; _rtXdot -> vv0at_CSTATE_o2 =
_rtB -> B_40_816_0 ; _rtXdot -> TransferFcn_CSTATE_o = 0.0 ; _rtXdot ->
TransferFcn_CSTATE_o += _rtP -> P_578 * _rtX -> TransferFcn_CSTATE_o ;
_rtXdot -> TransferFcn_CSTATE_o += _rtB -> B_40_666_0 ; _rtXdot ->
vv0at_CSTATE_kcg = _rtB -> B_40_818_0 ; _rtXdot -> TransferFcn_CSTATE_ge =
0.0 ; _rtXdot -> TransferFcn_CSTATE_ge += _rtP -> P_594 * _rtX ->
TransferFcn_CSTATE_ge ; _rtXdot -> TransferFcn_CSTATE_ge += _rtB ->
B_40_684_0 ; _rtXdot -> vv0at_CSTATE_ok = _rtB -> B_40_820_0 ; _rtXdot ->
TransferFcn_CSTATE_nc = 0.0 ; _rtXdot -> TransferFcn_CSTATE_nc += _rtP ->
P_610 * _rtX -> TransferFcn_CSTATE_nc ; _rtXdot -> TransferFcn_CSTATE_nc +=
_rtB -> B_40_702_0 ; _rtXdot -> vv0at_CSTATE_my = _rtB -> B_40_822_0 ;
_rtXdot -> TransferFcn_CSTATE_b = 0.0 ; _rtXdot -> TransferFcn_CSTATE_b +=
_rtP -> P_626 * _rtX -> TransferFcn_CSTATE_b ; _rtXdot ->
TransferFcn_CSTATE_b += _rtB -> B_40_720_0 ; _rtXdot -> vv0at_CSTATE_ev =
_rtB -> B_40_824_0 ; _rtXdot -> TransferFcn_CSTATE_pj = 0.0 ; _rtXdot ->
TransferFcn_CSTATE_pj += _rtP -> P_642 * _rtX -> TransferFcn_CSTATE_pj ;
_rtXdot -> TransferFcn_CSTATE_pj += _rtB -> B_40_740_0 ; _rtXdot ->
Integrator1_CSTATE = _rtB -> B_40_11_0 ; } static void mdlInitializeSizes (
SimStruct * S ) { ssSetChecksumVal ( S , 0 , 546203821U ) ; ssSetChecksumVal
( S , 1 , 2808265048U ) ; ssSetChecksumVal ( S , 2 , 716714824U ) ;
ssSetChecksumVal ( S , 3 , 274287231U ) ; { mxArray * slVerStructMat = NULL ;
mxArray * slStrMat = mxCreateString ( "simulink" ) ; char slVerChar [ 10 ] ;
int status = mexCallMATLAB ( 1 , & slVerStructMat , 1 , & slStrMat , "ver" )
; if ( status == 0 ) { mxArray * slVerMat = mxGetField ( slVerStructMat , 0 ,
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
