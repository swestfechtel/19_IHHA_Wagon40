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
rateLimiterRate ; real_T riseValLimit ; boolean_T rtb_B_80_2_0 ; real_T
rtb_B_80_3_0 ; int32_T isHit ; void * diag ; B_Simulation_v2_T * _rtB ;
P_Simulation_v2_T * _rtP ; X_Simulation_v2_T * _rtX ; DW_Simulation_v2_T *
_rtDW ; _rtDW = ( ( DW_Simulation_v2_T * ) ssGetRootDWork ( S ) ) ; _rtX = (
( X_Simulation_v2_T * ) ssGetContStates ( S ) ) ; _rtP = ( (
P_Simulation_v2_T * ) ssGetModelRtp ( S ) ) ; _rtB = ( ( B_Simulation_v2_T *
) _ssGetModelBlockIO ( S ) ) ; ssCallAccelRunBlock ( S , 80 , 0 ,
SS_CALL_MDL_OUTPUTS ) ; isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0
) { _rtB -> B_80_1_0 = _rtDW -> DelayInput1_DSTATE ; } rtb_B_80_2_0 = ( _rtB
-> B_80_0_0 < _rtB -> B_80_1_0 ) ; rtb_B_80_3_0 = _rtB -> B_80_0_0 * ( real_T
) rtb_B_80_2_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Calculation of braking pressure/Data Store Read" , 0 ) ; _rtB
-> B_80_7_0 = ( real_T ) ! rtb_B_80_2_0 * _rtDW -> v0 + rtb_B_80_3_0 ; _rtDW
-> v0 = _rtB -> B_80_7_0 ; vm_WriteLocalDSMNoIdx ( S , _rtDW -> dsmIdx , (
char_T * ) "Simulation_v2/Calculation of braking pressure/Data Store Write" ,
0 ) ; ssCallAccelRunBlock ( S , 80 , 9 , SS_CALL_MDL_OUTPUTS ) ;
vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Subsystem/Data Store Read" , 0 ) ; _rtB -> B_80_10_0 = _rtDW
-> v0 ; if ( _rtDW -> Integrator_IWORK != 0 ) { _rtX -> Integrator_CSTATE =
_rtB -> B_80_10_0 ; } _rtB -> B_80_11_0 = _rtX -> Integrator_CSTATE ; _rtB ->
B_80_12_0 = _rtB -> B_80_11_0 - _rtB -> B_80_0_0 ; ssCallAccelRunBlock ( S ,
80 , 13 , SS_CALL_MDL_OUTPUTS ) ; _rtB -> B_80_14_0 = look1_binlxpw ( _rtB ->
B_80_12_0 , _rtP -> P_4 , _rtP -> P_3 , 7U ) ; ssCallAccelRunBlock ( S , 80 ,
15 , SS_CALL_MDL_OUTPUTS ) ; _rtB -> B_80_17_0 = ( real_T ) ( _rtB ->
B_80_11_0 > _rtB -> B_80_0_0 ) * _rtB -> B_80_14_0 ; _rtB -> B_80_18_0 = _rtB
-> B_80_0_0_m - _rtB -> B_80_17_0 ; _rtB -> B_80_19_0 = _rtB -> B_80_18_0 +
_rtP -> P_5 ; _rtB -> B_80_20_0 = _rtP -> P_6 * _rtB -> B_80_12_0 ; isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { ssCallAccelRunBlock ( S , 0
, 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP -> P_854 != 0 ) && ( ! ( _rtB ->
B_0_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_854 == 1 ) { if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings == 0U ) { diag = CreateDiagnosticAsVoidPtr
( "Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem/Wagon model/lookup wagon mass" )
; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem/Wagon model/lookup wagon mass" )
; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_2_0 ) ; if ( ( _rtP -> P_854 != 0 ) && ( ( ! ( rtb_B_80_3_0 >=
0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_854 == 1 ) { if
( _rtDW -> lookupwagonmass_NumIssuedWarnings == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem/Wagon model/lookup wagon mass" )
; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem/Wagon model/lookup wagon mass" )
; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_2_0 ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB -> B_80_2_0
; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if (
rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_22_0 = _rtB ->
B_80_1_0_c [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_0_0_0 , 499U ) ] ; _rtB -> B_80_23_0 = _rtB
-> B_80_4_0 * _rtB -> B_80_22_0 ; if ( ( _rtP -> P_855 != 0 ) && ( ! ( _rtB
-> B_0_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_855 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem/Wagon model/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem/Wagon model/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_3_0 ) ; if ( ( _rtP -> P_855 != 0 ) && ( ( ! ( rtb_B_80_3_0 >=
0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_855 == 1 ) { if
( _rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem/Wagon model/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem/Wagon model/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_3_0 ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB -> B_80_3_0
; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if (
rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_24_0 = _rtB ->
B_80_1_0_c [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_0_0_0 , 499U ) ] ; } _rtB -> B_80_25_0 =
_rtP -> P_7 * _rtB -> B_80_19_0 ; if ( _rtB -> B_80_25_0 > _rtP -> P_8 ) {
_rtB -> B_80_26_0 = _rtP -> P_8 ; } else if ( _rtB -> B_80_25_0 < _rtP -> P_9
) { _rtB -> B_80_26_0 = _rtP -> P_9 ; } else { _rtB -> B_80_26_0 = _rtB ->
B_80_25_0 ; } if ( ( _rtDW -> LastMajorTimeA >= ssGetTaskTime ( S , 0 ) ) &&
( _rtDW -> LastMajorTimeB >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_80_27_0
= _rtB -> B_80_26_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA < _rtDW ->
LastMajorTimeB ) && ( _rtDW -> LastMajorTimeB < ssGetTaskTime ( S , 0 ) ) )
|| ( ( _rtDW -> LastMajorTimeA >= _rtDW -> LastMajorTimeB ) && ( _rtDW ->
LastMajorTimeA >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime ( S
, 0 ) - _rtDW -> LastMajorTimeB ; rtb_B_80_3_0 = _rtDW -> PrevYB ; } else {
deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA ; rtb_B_80_3_0 =
_rtDW -> PrevYA ; } riseValLimit = deltaT * _rtP -> P_10 ; rateLimiterRate =
_rtB -> B_80_26_0 - rtb_B_80_3_0 ; if ( rateLimiterRate > riseValLimit ) {
_rtB -> B_80_27_0 = rtb_B_80_3_0 + riseValLimit ; } else { deltaT *= _rtP ->
P_11 ; if ( rateLimiterRate < deltaT ) { _rtB -> B_80_27_0 = rtb_B_80_3_0 +
deltaT ; } else { _rtB -> B_80_27_0 = _rtB -> B_80_26_0 ; } } } _rtB ->
B_80_28_0 = _rtP -> P_12 * _rtB -> B_80_27_0 ; _rtB -> B_80_29_0 = _rtB ->
B_80_24_0 * _rtB -> B_80_28_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx ,
( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem/Wagon model/Data Store Read" , 0
) ; _rtB -> B_80_30_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK != 0 ) { _rtX
-> vv0at_CSTATE = _rtB -> B_80_30_0 ; } _rtB -> B_80_31_0 = _rtX ->
vv0at_CSTATE ; _rtB -> B_80_32_0 = muDoubleScalarAbs ( _rtB -> B_80_31_0 ) ;
_rtB -> B_80_33_0 = _rtP -> P_13 [ 0 ] ; _rtB -> B_80_33_0 = _rtB ->
B_80_33_0 * _rtB -> B_80_32_0 + _rtP -> P_13 [ 1 ] ; _rtB -> B_80_33_0 = _rtB
-> B_80_33_0 * _rtB -> B_80_32_0 + _rtP -> P_13 [ 2 ] ; _rtB -> B_80_34_0 =
_rtP -> P_14 * _rtB -> B_80_33_0 ; _rtB -> B_80_35_0 = _rtB -> B_80_22_0 *
_rtB -> B_80_34_0 ; _rtB -> B_80_36_0 = _rtB -> B_80_29_0 - _rtB -> B_80_35_0
; isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB -> B_80_38_0
= _rtP -> P_15 * _rtB -> B_80_23_0 ; } if ( _rtB -> B_80_36_0 > _rtB ->
B_80_23_0 ) { _rtB -> B_80_40_0 = _rtB -> B_80_23_0 ; } else { if ( _rtB ->
B_80_36_0 < _rtB -> B_80_38_0 ) { _rtB -> B_1_1_0 = _rtB -> B_80_38_0 ; }
else { _rtB -> B_1_1_0 = _rtB -> B_80_36_0 ; } _rtB -> B_80_40_0 = _rtB ->
B_1_1_0 ; } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) {
ssCallAccelRunBlock ( S , 2 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP ->
P_856 != 0 ) && ( ! ( _rtB -> B_2_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_856 ==
1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_g == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem/Wagon model1/lookup wagon mass" )
; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_g < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_g ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem/Wagon model1/lookup wagon mass" )
; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_6_0 ) ; if ( ( _rtP -> P_856 != 0 ) && ( ( ! ( rtb_B_80_3_0 >=
0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_856 == 1 ) { if
( _rtDW -> lookupwagonmass_NumIssuedWarnings_g == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem/Wagon model1/lookup wagon mass" )
; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_g < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_g ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem/Wagon model1/lookup wagon mass" )
; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_6_0 ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB -> B_80_6_0
; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if (
rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_42_0 = _rtB ->
B_80_5_0 [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T ) muIntScalarMin_uint32
( _rtB -> B_2_0_0 , 499U ) ] ; _rtB -> B_80_43_0 = _rtB -> B_80_8_0 * _rtB ->
B_80_42_0 ; if ( ( _rtP -> P_857 != 0 ) && ( ! ( _rtB -> B_2_0_0 <= 499.0 ) )
) { if ( _rtP -> P_857 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_m == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_m < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_m ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_7_0_k ) ; if ( ( _rtP -> P_857 != 0 ) && ( ( ! ( rtb_B_80_3_0 >=
0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_857 == 1 ) { if
( _rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_m == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_m < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_m ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_7_0_k ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_7_0_k ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if (
rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_44_0 = _rtB ->
B_80_5_0 [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T ) muIntScalarMin_uint32
( _rtB -> B_2_0_0 , 499U ) ] ; } { real_T * * uBuffer = ( real_T * * ) &
_rtDW -> TransportDelay_PWORK . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK . TUbufferPtrs [ 1 ] ; real_T
simTime = ssGetT ( S ) ; real_T tMinusDelay = simTime - _rtP -> P_16 ; _rtB
-> B_80_45_0 = Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , *
tBuffer , * uBuffer , _rtDW -> TransportDelay_IWORK . CircularBufSize , &
_rtDW -> TransportDelay_IWORK . Last , _rtDW -> TransportDelay_IWORK . Tail ,
_rtDW -> TransportDelay_IWORK . Head , _rtP -> P_17 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_80_46_0 = 0.0 ; _rtB -> B_80_46_0 += _rtP -> P_19 * _rtX ->
TransferFcn_CSTATE ; _rtB -> B_80_46_0 += _rtP -> P_20 * _rtB -> B_80_45_0 ;
if ( _rtB -> B_80_46_0 > _rtP -> P_22 ) { _rtB -> B_80_47_0 = _rtB ->
B_80_46_0 - _rtP -> P_22 ; } else if ( _rtB -> B_80_46_0 >= _rtP -> P_21 ) {
_rtB -> B_80_47_0 = 0.0 ; } else { _rtB -> B_80_47_0 = _rtB -> B_80_46_0 -
_rtP -> P_21 ; } _rtB -> B_80_48_0 = _rtP -> P_23 * _rtB -> B_80_47_0 ; if (
_rtB -> B_80_48_0 > _rtP -> P_24 ) { _rtB -> B_80_49_0 = _rtP -> P_24 ; }
else if ( _rtB -> B_80_48_0 < _rtP -> P_25 ) { _rtB -> B_80_49_0 = _rtP ->
P_25 ; } else { _rtB -> B_80_49_0 = _rtB -> B_80_48_0 ; } if ( ( _rtDW ->
LastMajorTimeA_f >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW -> LastMajorTimeB_m
>= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_80_50_0 = _rtB -> B_80_49_0 ; }
else { if ( ( ( _rtDW -> LastMajorTimeA_f < _rtDW -> LastMajorTimeB_m ) && (
_rtDW -> LastMajorTimeB_m < ssGetTaskTime ( S , 0 ) ) ) || ( ( _rtDW ->
LastMajorTimeA_f >= _rtDW -> LastMajorTimeB_m ) && ( _rtDW ->
LastMajorTimeA_f >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime (
S , 0 ) - _rtDW -> LastMajorTimeB_m ; rtb_B_80_3_0 = _rtDW -> PrevYB_m ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_f ;
rtb_B_80_3_0 = _rtDW -> PrevYA_l ; } riseValLimit = deltaT * _rtP -> P_26 ;
rateLimiterRate = _rtB -> B_80_49_0 - rtb_B_80_3_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_80_50_0 = rtb_B_80_3_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_27 ; if ( rateLimiterRate < deltaT ) { _rtB -> B_80_50_0
= rtb_B_80_3_0 + deltaT ; } else { _rtB -> B_80_50_0 = _rtB -> B_80_49_0 ; }
} } _rtB -> B_80_51_0 = _rtP -> P_28 * _rtB -> B_80_50_0 ; _rtB -> B_80_52_0
= _rtB -> B_80_44_0 * _rtB -> B_80_51_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW ->
dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem/Wagon model1/Data Store Read" , 0
) ; _rtB -> B_80_53_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_b != 0 ) {
_rtX -> vv0at_CSTATE_h = _rtB -> B_80_53_0 ; } _rtB -> B_80_54_0 = _rtX ->
vv0at_CSTATE_h ; _rtB -> B_80_55_0 = muDoubleScalarAbs ( _rtB -> B_80_54_0 )
; _rtB -> B_80_56_0 = _rtP -> P_29 [ 0 ] ; _rtB -> B_80_56_0 = _rtB ->
B_80_56_0 * _rtB -> B_80_55_0 + _rtP -> P_29 [ 1 ] ; _rtB -> B_80_56_0 = _rtB
-> B_80_56_0 * _rtB -> B_80_55_0 + _rtP -> P_29 [ 2 ] ; _rtB -> B_80_57_0 =
_rtP -> P_30 * _rtB -> B_80_56_0 ; _rtB -> B_80_58_0 = _rtB -> B_80_42_0 *
_rtB -> B_80_57_0 ; _rtB -> B_80_59_0 = _rtB -> B_80_52_0 - _rtB -> B_80_58_0
; isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB -> B_80_61_0
= _rtP -> P_31 * _rtB -> B_80_43_0 ; } if ( _rtB -> B_80_59_0 > _rtB ->
B_80_43_0 ) { _rtB -> B_80_63_0 = _rtB -> B_80_43_0 ; } else { if ( _rtB ->
B_80_59_0 < _rtB -> B_80_61_0 ) { _rtB -> B_3_1_0 = _rtB -> B_80_61_0 ; }
else { _rtB -> B_3_1_0 = _rtB -> B_80_59_0 ; } _rtB -> B_80_63_0 = _rtB ->
B_3_1_0 ; } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) {
ssCallAccelRunBlock ( S , 4 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP ->
P_858 != 0 ) && ( ! ( _rtB -> B_4_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_858 ==
1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_e == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem/Wagon model2/lookup wagon mass" )
; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_e < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_e ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem/Wagon model2/lookup wagon mass" )
; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_10_0_c ) ; if ( ( _rtP -> P_858 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_858 == 1 ) {
if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_e == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem/Wagon model2/lookup wagon mass" )
; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_e < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_e ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem/Wagon model2/lookup wagon mass" )
; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_10_0_c ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_10_0_c ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_65_0 = _rtB ->
B_80_9_0 [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T ) muIntScalarMin_uint32
( _rtB -> B_4_0_0 , 499U ) ] ; _rtB -> B_80_66_0 = _rtB -> B_80_12_0_p * _rtB
-> B_80_65_0 ; if ( ( _rtP -> P_859 != 0 ) && ( ! ( _rtB -> B_4_0_0 <= 499.0
) ) ) { if ( _rtP -> P_859 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_b == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_b < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_b ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_11_0_b ) ; if ( ( _rtP -> P_859 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_859 == 1 ) {
if ( _rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_b == 0U ) { diag
= CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_b < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_b ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_11_0_b ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_11_0_b ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_67_0 = _rtB ->
B_80_9_0 [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T ) muIntScalarMin_uint32
( _rtB -> B_4_0_0 , 499U ) ] ; } { real_T * * uBuffer = ( real_T * * ) &
_rtDW -> TransportDelay_PWORK_o . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_o . TUbufferPtrs [ 1 ] ; real_T
simTime = ssGetT ( S ) ; real_T tMinusDelay = simTime - _rtP -> P_32 ; _rtB
-> B_80_68_0 = Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , *
tBuffer , * uBuffer , _rtDW -> TransportDelay_IWORK_c . CircularBufSize , &
_rtDW -> TransportDelay_IWORK_c . Last , _rtDW -> TransportDelay_IWORK_c .
Tail , _rtDW -> TransportDelay_IWORK_c . Head , _rtP -> P_33 , 0 , (
boolean_T ) ( ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) ==
ssGetT ( S ) ) ) ) ; } _rtB -> B_80_69_0 = 0.0 ; _rtB -> B_80_69_0 += _rtP ->
P_35 * _rtX -> TransferFcn_CSTATE_d ; _rtB -> B_80_69_0 += _rtP -> P_36 *
_rtB -> B_80_68_0 ; if ( _rtB -> B_80_69_0 > _rtP -> P_38 ) { _rtB ->
B_80_70_0 = _rtB -> B_80_69_0 - _rtP -> P_38 ; } else if ( _rtB -> B_80_69_0
>= _rtP -> P_37 ) { _rtB -> B_80_70_0 = 0.0 ; } else { _rtB -> B_80_70_0 =
_rtB -> B_80_69_0 - _rtP -> P_37 ; } _rtB -> B_80_71_0 = _rtP -> P_39 * _rtB
-> B_80_70_0 ; if ( _rtB -> B_80_71_0 > _rtP -> P_40 ) { _rtB -> B_80_72_0 =
_rtP -> P_40 ; } else if ( _rtB -> B_80_71_0 < _rtP -> P_41 ) { _rtB ->
B_80_72_0 = _rtP -> P_41 ; } else { _rtB -> B_80_72_0 = _rtB -> B_80_71_0 ; }
if ( ( _rtDW -> LastMajorTimeA_o >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW ->
LastMajorTimeB_o >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_80_73_0 = _rtB ->
B_80_72_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_o < _rtDW ->
LastMajorTimeB_o ) && ( _rtDW -> LastMajorTimeB_o < ssGetTaskTime ( S , 0 ) )
) || ( ( _rtDW -> LastMajorTimeA_o >= _rtDW -> LastMajorTimeB_o ) && ( _rtDW
-> LastMajorTimeA_o >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime
( S , 0 ) - _rtDW -> LastMajorTimeB_o ; rtb_B_80_3_0 = _rtDW -> PrevYB_j ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_o ;
rtb_B_80_3_0 = _rtDW -> PrevYA_d ; } riseValLimit = deltaT * _rtP -> P_42 ;
rateLimiterRate = _rtB -> B_80_72_0 - rtb_B_80_3_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_80_73_0 = rtb_B_80_3_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_43 ; if ( rateLimiterRate < deltaT ) { _rtB -> B_80_73_0
= rtb_B_80_3_0 + deltaT ; } else { _rtB -> B_80_73_0 = _rtB -> B_80_72_0 ; }
} } _rtB -> B_80_74_0 = _rtP -> P_44 * _rtB -> B_80_73_0 ; _rtB -> B_80_75_0
= _rtB -> B_80_67_0 * _rtB -> B_80_74_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW ->
dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem/Wagon model2/Data Store Read" , 0
) ; _rtB -> B_80_76_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_h != 0 ) {
_rtX -> vv0at_CSTATE_f = _rtB -> B_80_76_0 ; } _rtB -> B_80_77_0 = _rtX ->
vv0at_CSTATE_f ; _rtB -> B_80_78_0 = muDoubleScalarAbs ( _rtB -> B_80_77_0 )
; _rtB -> B_80_79_0 = _rtP -> P_45 [ 0 ] ; _rtB -> B_80_79_0 = _rtB ->
B_80_79_0 * _rtB -> B_80_78_0 + _rtP -> P_45 [ 1 ] ; _rtB -> B_80_79_0 = _rtB
-> B_80_79_0 * _rtB -> B_80_78_0 + _rtP -> P_45 [ 2 ] ; _rtB -> B_80_80_0 =
_rtP -> P_46 * _rtB -> B_80_79_0 ; _rtB -> B_80_81_0 = _rtB -> B_80_65_0 *
_rtB -> B_80_80_0 ; _rtB -> B_80_82_0 = _rtB -> B_80_75_0 - _rtB -> B_80_81_0
; isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { _rtB -> B_80_84_0
= _rtP -> P_47 * _rtB -> B_80_66_0 ; } if ( _rtB -> B_80_82_0 > _rtB ->
B_80_66_0 ) { _rtB -> B_80_86_0 = _rtB -> B_80_66_0 ; } else { if ( _rtB ->
B_80_82_0 < _rtB -> B_80_84_0 ) { _rtB -> B_5_1_0 = _rtB -> B_80_84_0 ; }
else { _rtB -> B_5_1_0 = _rtB -> B_80_82_0 ; } _rtB -> B_80_86_0 = _rtB ->
B_5_1_0 ; } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) {
ssCallAccelRunBlock ( S , 6 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP ->
P_860 != 0 ) && ( ! ( _rtB -> B_6_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_860 ==
1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_j == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem/Wagon model3/lookup wagon mass" )
; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_j < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_j ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem/Wagon model3/lookup wagon mass" )
; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_14_0_c ) ; if ( ( _rtP -> P_860 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_860 == 1 ) {
if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_j == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem/Wagon model3/lookup wagon mass" )
; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_j < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_j ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem/Wagon model3/lookup wagon mass" )
; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_14_0_c ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_14_0_c ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_88_0 = _rtB ->
B_80_13_0 [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_6_0_0 , 499U ) ] ; _rtB -> B_80_89_0 = _rtB
-> B_80_16_0 * _rtB -> B_80_88_0 ; if ( ( _rtP -> P_861 != 0 ) && ( ! ( _rtB
-> B_6_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_861 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_a == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_a < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_a ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_15_0 ) ; if ( ( _rtP -> P_861 != 0 ) && ( ( ! ( rtb_B_80_3_0 >=
0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_861 == 1 ) { if
( _rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_a == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_a < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_a ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_15_0 ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_15_0 ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if (
rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_90_0 = _rtB ->
B_80_13_0 [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_6_0_0 , 499U ) ] ; } { real_T * * uBuffer =
( real_T * * ) & _rtDW -> TransportDelay_PWORK_b . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_b .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_48 ; _rtB -> B_80_91_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_f . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_f . Last , _rtDW -> TransportDelay_IWORK_f . Tail ,
_rtDW -> TransportDelay_IWORK_f . Head , _rtP -> P_49 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_80_92_0 = 0.0 ; _rtB -> B_80_92_0 += _rtP -> P_51 * _rtX ->
TransferFcn_CSTATE_h ; _rtB -> B_80_92_0 += _rtP -> P_52 * _rtB -> B_80_91_0
; if ( _rtB -> B_80_92_0 > _rtP -> P_54 ) { _rtB -> B_80_93_0 = _rtB ->
B_80_92_0 - _rtP -> P_54 ; } else if ( _rtB -> B_80_92_0 >= _rtP -> P_53 ) {
_rtB -> B_80_93_0 = 0.0 ; } else { _rtB -> B_80_93_0 = _rtB -> B_80_92_0 -
_rtP -> P_53 ; } _rtB -> B_80_94_0 = _rtP -> P_55 * _rtB -> B_80_93_0 ; if (
_rtB -> B_80_94_0 > _rtP -> P_56 ) { _rtB -> B_80_95_0 = _rtP -> P_56 ; }
else if ( _rtB -> B_80_94_0 < _rtP -> P_57 ) { _rtB -> B_80_95_0 = _rtP ->
P_57 ; } else { _rtB -> B_80_95_0 = _rtB -> B_80_94_0 ; } if ( ( _rtDW ->
LastMajorTimeA_e >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW -> LastMajorTimeB_d
>= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_80_96_0 = _rtB -> B_80_95_0 ; }
else { if ( ( ( _rtDW -> LastMajorTimeA_e < _rtDW -> LastMajorTimeB_d ) && (
_rtDW -> LastMajorTimeB_d < ssGetTaskTime ( S , 0 ) ) ) || ( ( _rtDW ->
LastMajorTimeA_e >= _rtDW -> LastMajorTimeB_d ) && ( _rtDW ->
LastMajorTimeA_e >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime (
S , 0 ) - _rtDW -> LastMajorTimeB_d ; rtb_B_80_3_0 = _rtDW -> PrevYB_g ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_e ;
rtb_B_80_3_0 = _rtDW -> PrevYA_k ; } riseValLimit = deltaT * _rtP -> P_58 ;
rateLimiterRate = _rtB -> B_80_95_0 - rtb_B_80_3_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_80_96_0 = rtb_B_80_3_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_59 ; if ( rateLimiterRate < deltaT ) { _rtB -> B_80_96_0
= rtb_B_80_3_0 + deltaT ; } else { _rtB -> B_80_96_0 = _rtB -> B_80_95_0 ; }
} } _rtB -> B_80_97_0 = _rtP -> P_60 * _rtB -> B_80_96_0 ; _rtB -> B_80_98_0
= _rtB -> B_80_90_0 * _rtB -> B_80_97_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW ->
dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem/Wagon model3/Data Store Read" , 0
) ; _rtB -> B_80_99_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_c != 0 ) {
_rtX -> vv0at_CSTATE_hd = _rtB -> B_80_99_0 ; } _rtB -> B_80_100_0 = _rtX ->
vv0at_CSTATE_hd ; _rtB -> B_80_101_0 = muDoubleScalarAbs ( _rtB -> B_80_100_0
) ; _rtB -> B_80_102_0 = _rtP -> P_61 [ 0 ] ; _rtB -> B_80_102_0 = _rtB ->
B_80_102_0 * _rtB -> B_80_101_0 + _rtP -> P_61 [ 1 ] ; _rtB -> B_80_102_0 =
_rtB -> B_80_102_0 * _rtB -> B_80_101_0 + _rtP -> P_61 [ 2 ] ; _rtB ->
B_80_103_0 = _rtP -> P_62 * _rtB -> B_80_102_0 ; _rtB -> B_80_104_0 = _rtB ->
B_80_88_0 * _rtB -> B_80_103_0 ; _rtB -> B_80_105_0 = _rtB -> B_80_98_0 -
_rtB -> B_80_104_0 ; isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 )
{ _rtB -> B_80_107_0 = _rtP -> P_63 * _rtB -> B_80_89_0 ; } if ( _rtB ->
B_80_105_0 > _rtB -> B_80_89_0 ) { _rtB -> B_80_109_0 = _rtB -> B_80_89_0 ; }
else { if ( _rtB -> B_80_105_0 < _rtB -> B_80_107_0 ) { _rtB -> B_7_1_0 =
_rtB -> B_80_107_0 ; } else { _rtB -> B_7_1_0 = _rtB -> B_80_105_0 ; } _rtB
-> B_80_109_0 = _rtB -> B_7_1_0 ; } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if
( isHit != 0 ) { ssCallAccelRunBlock ( S , 8 , 0 , SS_CALL_MDL_OUTPUTS ) ; if
( ( _rtP -> P_862 != 0 ) && ( ! ( _rtB -> B_8_0_0 <= 499.0 ) ) ) { if ( _rtP
-> P_862 == 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_a == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem/Wagon model4/lookup wagon mass" )
; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_a < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_a ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem/Wagon model4/lookup wagon mass" )
; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_18_0_g ) ; if ( ( _rtP -> P_862 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_862 == 1 ) {
if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_a == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem/Wagon model4/lookup wagon mass" )
; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_a < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_a ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem/Wagon model4/lookup wagon mass" )
; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_18_0_g ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_18_0_g ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_111_0 = _rtB
-> B_80_17_0_f [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_8_0_0 , 499U ) ] ; _rtB -> B_80_112_0 =
_rtB -> B_80_20_0_m * _rtB -> B_80_111_0 ; if ( ( _rtP -> P_863 != 0 ) && ( !
( _rtB -> B_8_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_863 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_o == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_o < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_o ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_19_0_g ) ; if ( ( _rtP -> P_863 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_863 == 1 ) {
if ( _rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_o == 0U ) { diag
= CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_o < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_o ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_19_0_g ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_19_0_g ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_113_0 = _rtB
-> B_80_17_0_f [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_8_0_0 , 499U ) ] ; } { real_T * * uBuffer =
( real_T * * ) & _rtDW -> TransportDelay_PWORK_c . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_c .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_64 ; _rtB -> B_80_114_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_h . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_h . Last , _rtDW -> TransportDelay_IWORK_h . Tail ,
_rtDW -> TransportDelay_IWORK_h . Head , _rtP -> P_65 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_80_115_0 = 0.0 ; _rtB -> B_80_115_0 += _rtP -> P_67 * _rtX ->
TransferFcn_CSTATE_j ; _rtB -> B_80_115_0 += _rtP -> P_68 * _rtB ->
B_80_114_0 ; if ( _rtB -> B_80_115_0 > _rtP -> P_70 ) { _rtB -> B_80_116_0 =
_rtB -> B_80_115_0 - _rtP -> P_70 ; } else if ( _rtB -> B_80_115_0 >= _rtP ->
P_69 ) { _rtB -> B_80_116_0 = 0.0 ; } else { _rtB -> B_80_116_0 = _rtB ->
B_80_115_0 - _rtP -> P_69 ; } _rtB -> B_80_117_0 = _rtP -> P_71 * _rtB ->
B_80_116_0 ; if ( _rtB -> B_80_117_0 > _rtP -> P_72 ) { _rtB -> B_80_118_0 =
_rtP -> P_72 ; } else if ( _rtB -> B_80_117_0 < _rtP -> P_73 ) { _rtB ->
B_80_118_0 = _rtP -> P_73 ; } else { _rtB -> B_80_118_0 = _rtB -> B_80_117_0
; } if ( ( _rtDW -> LastMajorTimeA_b >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW
-> LastMajorTimeB_k >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_80_119_0 =
_rtB -> B_80_118_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_b < _rtDW ->
LastMajorTimeB_k ) && ( _rtDW -> LastMajorTimeB_k < ssGetTaskTime ( S , 0 ) )
) || ( ( _rtDW -> LastMajorTimeA_b >= _rtDW -> LastMajorTimeB_k ) && ( _rtDW
-> LastMajorTimeA_b >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime
( S , 0 ) - _rtDW -> LastMajorTimeB_k ; rtb_B_80_3_0 = _rtDW -> PrevYB_ms ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_b ;
rtb_B_80_3_0 = _rtDW -> PrevYA_p ; } riseValLimit = deltaT * _rtP -> P_74 ;
rateLimiterRate = _rtB -> B_80_118_0 - rtb_B_80_3_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_80_119_0 = rtb_B_80_3_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_75 ; if ( rateLimiterRate < deltaT ) { _rtB -> B_80_119_0
= rtb_B_80_3_0 + deltaT ; } else { _rtB -> B_80_119_0 = _rtB -> B_80_118_0 ;
} } } _rtB -> B_80_120_0 = _rtP -> P_76 * _rtB -> B_80_119_0 ; _rtB ->
B_80_121_0 = _rtB -> B_80_113_0 * _rtB -> B_80_120_0 ; vm_ReadLocalDSMNoIdx (
S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem/Wagon model4/Data Store Read" , 0
) ; _rtB -> B_80_122_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_cz != 0 ) {
_rtX -> vv0at_CSTATE_g = _rtB -> B_80_122_0 ; } _rtB -> B_80_123_0 = _rtX ->
vv0at_CSTATE_g ; _rtB -> B_80_124_0 = muDoubleScalarAbs ( _rtB -> B_80_123_0
) ; _rtB -> B_80_125_0 = _rtP -> P_77 [ 0 ] ; _rtB -> B_80_125_0 = _rtB ->
B_80_125_0 * _rtB -> B_80_124_0 + _rtP -> P_77 [ 1 ] ; _rtB -> B_80_125_0 =
_rtB -> B_80_125_0 * _rtB -> B_80_124_0 + _rtP -> P_77 [ 2 ] ; _rtB ->
B_80_126_0 = _rtP -> P_78 * _rtB -> B_80_125_0 ; _rtB -> B_80_127_0 = _rtB ->
B_80_111_0 * _rtB -> B_80_126_0 ; _rtB -> B_80_128_0 = _rtB -> B_80_121_0 -
_rtB -> B_80_127_0 ; isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 )
{ _rtB -> B_80_130_0 = _rtP -> P_79 * _rtB -> B_80_112_0 ; } if ( _rtB ->
B_80_128_0 > _rtB -> B_80_112_0 ) { _rtB -> B_80_132_0 = _rtB -> B_80_112_0 ;
} else { if ( _rtB -> B_80_128_0 < _rtB -> B_80_130_0 ) { _rtB -> B_9_1_0 =
_rtB -> B_80_130_0 ; } else { _rtB -> B_9_1_0 = _rtB -> B_80_128_0 ; } _rtB
-> B_80_132_0 = _rtB -> B_9_1_0 ; } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if
( isHit != 0 ) { ssCallAccelRunBlock ( S , 10 , 0 , SS_CALL_MDL_OUTPUTS ) ;
if ( ( _rtP -> P_864 != 0 ) && ( ! ( _rtB -> B_10_0_0 <= 499.0 ) ) ) { if (
_rtP -> P_864 == 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_jd ==
0U ) { diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model/lookup wagon mass" )
; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_jd < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_jd ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model/lookup wagon mass" )
; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_22_0_n ) ; if ( ( _rtP -> P_864 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_864 == 1 ) {
if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_jd == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model/lookup wagon mass" )
; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_jd < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_jd ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model/lookup wagon mass" )
; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_22_0_n ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_22_0_n ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_134_0 = _rtB
-> B_80_21_0 [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_10_0_0 , 499U ) ] ; _rtB -> B_80_135_0 =
_rtB -> B_80_24_0_l * _rtB -> B_80_134_0 ; if ( ( _rtP -> P_865 != 0 ) && ( !
( _rtB -> B_10_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_865 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_n == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_n < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_n ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_23_0_p ) ; if ( ( _rtP -> P_865 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_865 == 1 ) {
if ( _rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_n == 0U ) { diag
= CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_n < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_n ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_23_0_p ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_23_0_p ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_136_0 = _rtB
-> B_80_21_0 [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_10_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_n . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_n .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_80 ; _rtB -> B_80_137_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_hd . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_hd . Last , _rtDW -> TransportDelay_IWORK_hd . Tail ,
_rtDW -> TransportDelay_IWORK_hd . Head , _rtP -> P_81 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_80_138_0 = 0.0 ; _rtB -> B_80_138_0 += _rtP -> P_83 * _rtX ->
TransferFcn_CSTATE_e ; _rtB -> B_80_138_0 += _rtP -> P_84 * _rtB ->
B_80_137_0 ; if ( _rtB -> B_80_138_0 > _rtP -> P_86 ) { _rtB -> B_80_139_0 =
_rtB -> B_80_138_0 - _rtP -> P_86 ; } else if ( _rtB -> B_80_138_0 >= _rtP ->
P_85 ) { _rtB -> B_80_139_0 = 0.0 ; } else { _rtB -> B_80_139_0 = _rtB ->
B_80_138_0 - _rtP -> P_85 ; } _rtB -> B_80_140_0 = _rtP -> P_87 * _rtB ->
B_80_139_0 ; if ( _rtB -> B_80_140_0 > _rtP -> P_88 ) { _rtB -> B_80_141_0 =
_rtP -> P_88 ; } else if ( _rtB -> B_80_140_0 < _rtP -> P_89 ) { _rtB ->
B_80_141_0 = _rtP -> P_89 ; } else { _rtB -> B_80_141_0 = _rtB -> B_80_140_0
; } if ( ( _rtDW -> LastMajorTimeA_os >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW
-> LastMajorTimeB_g >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_80_142_0 =
_rtB -> B_80_141_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_os < _rtDW ->
LastMajorTimeB_g ) && ( _rtDW -> LastMajorTimeB_g < ssGetTaskTime ( S , 0 ) )
) || ( ( _rtDW -> LastMajorTimeA_os >= _rtDW -> LastMajorTimeB_g ) && ( _rtDW
-> LastMajorTimeA_os >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_g ; rtb_B_80_3_0 = _rtDW ->
PrevYB_f ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_os ; rtb_B_80_3_0 = _rtDW -> PrevYA_n ; } riseValLimit =
deltaT * _rtP -> P_90 ; rateLimiterRate = _rtB -> B_80_141_0 - rtb_B_80_3_0 ;
if ( rateLimiterRate > riseValLimit ) { _rtB -> B_80_142_0 = rtb_B_80_3_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_91 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_80_142_0 = rtb_B_80_3_0 + deltaT ; } else { _rtB ->
B_80_142_0 = _rtB -> B_80_141_0 ; } } } _rtB -> B_80_143_0 = _rtP -> P_92 *
_rtB -> B_80_142_0 ; _rtB -> B_80_144_0 = _rtB -> B_80_136_0 * _rtB ->
B_80_143_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model/Data Store Read" , 0
) ; _rtB -> B_80_145_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_o != 0 ) {
_rtX -> vv0at_CSTATE_c = _rtB -> B_80_145_0 ; } _rtB -> B_80_146_0 = _rtX ->
vv0at_CSTATE_c ; _rtB -> B_80_147_0 = muDoubleScalarAbs ( _rtB -> B_80_146_0
) ; _rtB -> B_80_148_0 = _rtP -> P_93 [ 0 ] ; _rtB -> B_80_148_0 = _rtB ->
B_80_148_0 * _rtB -> B_80_147_0 + _rtP -> P_93 [ 1 ] ; _rtB -> B_80_148_0 =
_rtB -> B_80_148_0 * _rtB -> B_80_147_0 + _rtP -> P_93 [ 2 ] ; _rtB ->
B_80_149_0 = _rtP -> P_94 * _rtB -> B_80_148_0 ; _rtB -> B_80_150_0 = _rtB ->
B_80_134_0 * _rtB -> B_80_149_0 ; _rtB -> B_80_151_0 = _rtB -> B_80_144_0 -
_rtB -> B_80_150_0 ; isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 )
{ _rtB -> B_80_153_0 = _rtP -> P_95 * _rtB -> B_80_135_0 ; } if ( _rtB ->
B_80_151_0 > _rtB -> B_80_135_0 ) { _rtB -> B_80_155_0 = _rtB -> B_80_135_0 ;
} else { if ( _rtB -> B_80_151_0 < _rtB -> B_80_153_0 ) { _rtB -> B_11_1_0 =
_rtB -> B_80_153_0 ; } else { _rtB -> B_11_1_0 = _rtB -> B_80_151_0 ; } _rtB
-> B_80_155_0 = _rtB -> B_11_1_0 ; } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if
( isHit != 0 ) { ssCallAccelRunBlock ( S , 12 , 0 , SS_CALL_MDL_OUTPUTS ) ;
if ( ( _rtP -> P_866 != 0 ) && ( ! ( _rtB -> B_12_0_0 <= 499.0 ) ) ) { if (
_rtP -> P_866 == 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_j5 ==
0U ) { diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model1/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_j5 < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_j5 ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model1/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_26_0_d ) ; if ( ( _rtP -> P_866 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_866 == 1 ) {
if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_j5 == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model1/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_j5 < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_j5 ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model1/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_26_0_d ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_26_0_d ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_157_0 = _rtB
-> B_80_25_0_j [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_12_0_0 , 499U ) ] ; _rtB -> B_80_158_0 =
_rtB -> B_80_28_0_l * _rtB -> B_80_157_0 ; if ( ( _rtP -> P_867 != 0 ) && ( !
( _rtB -> B_12_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_867 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_g == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_g < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_g ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_27_0_g ) ; if ( ( _rtP -> P_867 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_867 == 1 ) {
if ( _rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_g == 0U ) { diag
= CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_g < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_g ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_27_0_g ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_27_0_g ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_159_0 = _rtB
-> B_80_25_0_j [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_12_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_k . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_k .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_96 ; _rtB -> B_80_160_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_d . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_d . Last , _rtDW -> TransportDelay_IWORK_d . Tail ,
_rtDW -> TransportDelay_IWORK_d . Head , _rtP -> P_97 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_80_161_0 = 0.0 ; _rtB -> B_80_161_0 += _rtP -> P_99 * _rtX ->
TransferFcn_CSTATE_b ; _rtB -> B_80_161_0 += _rtP -> P_100 * _rtB ->
B_80_160_0 ; if ( _rtB -> B_80_161_0 > _rtP -> P_102 ) { _rtB -> B_80_162_0 =
_rtB -> B_80_161_0 - _rtP -> P_102 ; } else if ( _rtB -> B_80_161_0 >= _rtP
-> P_101 ) { _rtB -> B_80_162_0 = 0.0 ; } else { _rtB -> B_80_162_0 = _rtB ->
B_80_161_0 - _rtP -> P_101 ; } _rtB -> B_80_163_0 = _rtP -> P_103 * _rtB ->
B_80_162_0 ; if ( _rtB -> B_80_163_0 > _rtP -> P_104 ) { _rtB -> B_80_164_0 =
_rtP -> P_104 ; } else if ( _rtB -> B_80_163_0 < _rtP -> P_105 ) { _rtB ->
B_80_164_0 = _rtP -> P_105 ; } else { _rtB -> B_80_164_0 = _rtB -> B_80_163_0
; } if ( ( _rtDW -> LastMajorTimeA_n >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW
-> LastMajorTimeB_i >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_80_165_0 =
_rtB -> B_80_164_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_n < _rtDW ->
LastMajorTimeB_i ) && ( _rtDW -> LastMajorTimeB_i < ssGetTaskTime ( S , 0 ) )
) || ( ( _rtDW -> LastMajorTimeA_n >= _rtDW -> LastMajorTimeB_i ) && ( _rtDW
-> LastMajorTimeA_n >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime
( S , 0 ) - _rtDW -> LastMajorTimeB_i ; rtb_B_80_3_0 = _rtDW -> PrevYB_o ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_n ;
rtb_B_80_3_0 = _rtDW -> PrevYA_j ; } riseValLimit = deltaT * _rtP -> P_106 ;
rateLimiterRate = _rtB -> B_80_164_0 - rtb_B_80_3_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_80_165_0 = rtb_B_80_3_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_107 ; if ( rateLimiterRate < deltaT ) { _rtB ->
B_80_165_0 = rtb_B_80_3_0 + deltaT ; } else { _rtB -> B_80_165_0 = _rtB ->
B_80_164_0 ; } } } _rtB -> B_80_166_0 = _rtP -> P_108 * _rtB -> B_80_165_0 ;
_rtB -> B_80_167_0 = _rtB -> B_80_159_0 * _rtB -> B_80_166_0 ;
vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model1/Data Store Read" ,
0 ) ; _rtB -> B_80_168_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_l != 0 ) {
_rtX -> vv0at_CSTATE_m = _rtB -> B_80_168_0 ; } _rtB -> B_80_169_0 = _rtX ->
vv0at_CSTATE_m ; _rtB -> B_80_170_0 = muDoubleScalarAbs ( _rtB -> B_80_169_0
) ; _rtB -> B_80_171_0 = _rtP -> P_109 [ 0 ] ; _rtB -> B_80_171_0 = _rtB ->
B_80_171_0 * _rtB -> B_80_170_0 + _rtP -> P_109 [ 1 ] ; _rtB -> B_80_171_0 =
_rtB -> B_80_171_0 * _rtB -> B_80_170_0 + _rtP -> P_109 [ 2 ] ; _rtB ->
B_80_172_0 = _rtP -> P_110 * _rtB -> B_80_171_0 ; _rtB -> B_80_173_0 = _rtB
-> B_80_157_0 * _rtB -> B_80_172_0 ; _rtB -> B_80_174_0 = _rtB -> B_80_167_0
- _rtB -> B_80_173_0 ; isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0
) { _rtB -> B_80_176_0 = _rtP -> P_111 * _rtB -> B_80_158_0 ; } if ( _rtB ->
B_80_174_0 > _rtB -> B_80_158_0 ) { _rtB -> B_80_178_0 = _rtB -> B_80_158_0 ;
} else { if ( _rtB -> B_80_174_0 < _rtB -> B_80_176_0 ) { _rtB -> B_13_1_0 =
_rtB -> B_80_176_0 ; } else { _rtB -> B_13_1_0 = _rtB -> B_80_174_0 ; } _rtB
-> B_80_178_0 = _rtB -> B_13_1_0 ; } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if
( isHit != 0 ) { ssCallAccelRunBlock ( S , 14 , 0 , SS_CALL_MDL_OUTPUTS ) ;
if ( ( _rtP -> P_868 != 0 ) && ( ! ( _rtB -> B_14_0_0 <= 499.0 ) ) ) { if (
_rtP -> P_868 == 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_h ==
0U ) { diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model2/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_h < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_h ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model2/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_30_0_d ) ; if ( ( _rtP -> P_868 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_868 == 1 ) {
if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_h == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model2/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_h < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_h ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model2/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_30_0_d ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_30_0_d ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_180_0 = _rtB
-> B_80_29_0_d [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_14_0_0 , 499U ) ] ; _rtB -> B_80_181_0 =
_rtB -> B_80_32_0_o * _rtB -> B_80_180_0 ; if ( ( _rtP -> P_869 != 0 ) && ( !
( _rtB -> B_14_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_869 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_aj == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_aj < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_aj ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_31_0_l ) ; if ( ( _rtP -> P_869 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_869 == 1 ) {
if ( _rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_aj == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_aj < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_aj ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_31_0_l ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_31_0_l ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_182_0 = _rtB
-> B_80_29_0_d [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_14_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_ov . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_ov .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_112 ; _rtB -> B_80_183_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_a . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_a . Last , _rtDW -> TransportDelay_IWORK_a . Tail ,
_rtDW -> TransportDelay_IWORK_a . Head , _rtP -> P_113 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_80_184_0 = 0.0 ; _rtB -> B_80_184_0 += _rtP -> P_115 * _rtX
-> TransferFcn_CSTATE_l ; _rtB -> B_80_184_0 += _rtP -> P_116 * _rtB ->
B_80_183_0 ; if ( _rtB -> B_80_184_0 > _rtP -> P_118 ) { _rtB -> B_80_185_0 =
_rtB -> B_80_184_0 - _rtP -> P_118 ; } else if ( _rtB -> B_80_184_0 >= _rtP
-> P_117 ) { _rtB -> B_80_185_0 = 0.0 ; } else { _rtB -> B_80_185_0 = _rtB ->
B_80_184_0 - _rtP -> P_117 ; } _rtB -> B_80_186_0 = _rtP -> P_119 * _rtB ->
B_80_185_0 ; if ( _rtB -> B_80_186_0 > _rtP -> P_120 ) { _rtB -> B_80_187_0 =
_rtP -> P_120 ; } else if ( _rtB -> B_80_186_0 < _rtP -> P_121 ) { _rtB ->
B_80_187_0 = _rtP -> P_121 ; } else { _rtB -> B_80_187_0 = _rtB -> B_80_186_0
; } if ( ( _rtDW -> LastMajorTimeA_i >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW
-> LastMajorTimeB_c >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_80_188_0 =
_rtB -> B_80_187_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_i < _rtDW ->
LastMajorTimeB_c ) && ( _rtDW -> LastMajorTimeB_c < ssGetTaskTime ( S , 0 ) )
) || ( ( _rtDW -> LastMajorTimeA_i >= _rtDW -> LastMajorTimeB_c ) && ( _rtDW
-> LastMajorTimeA_i >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime
( S , 0 ) - _rtDW -> LastMajorTimeB_c ; rtb_B_80_3_0 = _rtDW -> PrevYB_on ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_i ;
rtb_B_80_3_0 = _rtDW -> PrevYA_o ; } riseValLimit = deltaT * _rtP -> P_122 ;
rateLimiterRate = _rtB -> B_80_187_0 - rtb_B_80_3_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_80_188_0 = rtb_B_80_3_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_123 ; if ( rateLimiterRate < deltaT ) { _rtB ->
B_80_188_0 = rtb_B_80_3_0 + deltaT ; } else { _rtB -> B_80_188_0 = _rtB ->
B_80_187_0 ; } } } _rtB -> B_80_189_0 = _rtP -> P_124 * _rtB -> B_80_188_0 ;
_rtB -> B_80_190_0 = _rtB -> B_80_182_0 * _rtB -> B_80_189_0 ;
vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model2/Data Store Read" ,
0 ) ; _rtB -> B_80_191_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_j != 0 ) {
_rtX -> vv0at_CSTATE_i = _rtB -> B_80_191_0 ; } _rtB -> B_80_192_0 = _rtX ->
vv0at_CSTATE_i ; _rtB -> B_80_193_0 = muDoubleScalarAbs ( _rtB -> B_80_192_0
) ; _rtB -> B_80_194_0 = _rtP -> P_125 [ 0 ] ; _rtB -> B_80_194_0 = _rtB ->
B_80_194_0 * _rtB -> B_80_193_0 + _rtP -> P_125 [ 1 ] ; _rtB -> B_80_194_0 =
_rtB -> B_80_194_0 * _rtB -> B_80_193_0 + _rtP -> P_125 [ 2 ] ; _rtB ->
B_80_195_0 = _rtP -> P_126 * _rtB -> B_80_194_0 ; _rtB -> B_80_196_0 = _rtB
-> B_80_180_0 * _rtB -> B_80_195_0 ; _rtB -> B_80_197_0 = _rtB -> B_80_190_0
- _rtB -> B_80_196_0 ; isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0
) { _rtB -> B_80_199_0 = _rtP -> P_127 * _rtB -> B_80_181_0 ; } if ( _rtB ->
B_80_197_0 > _rtB -> B_80_181_0 ) { _rtB -> B_80_201_0 = _rtB -> B_80_181_0 ;
} else { if ( _rtB -> B_80_197_0 < _rtB -> B_80_199_0 ) { _rtB -> B_15_1_0 =
_rtB -> B_80_199_0 ; } else { _rtB -> B_15_1_0 = _rtB -> B_80_197_0 ; } _rtB
-> B_80_201_0 = _rtB -> B_15_1_0 ; } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if
( isHit != 0 ) { ssCallAccelRunBlock ( S , 16 , 0 , SS_CALL_MDL_OUTPUTS ) ;
if ( ( _rtP -> P_870 != 0 ) && ( ! ( _rtB -> B_16_0_0 <= 499.0 ) ) ) { if (
_rtP -> P_870 == 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_n ==
0U ) { diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model3/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_n < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_n ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model3/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_34_0_n ) ; if ( ( _rtP -> P_870 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_870 == 1 ) {
if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_n == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model3/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_n < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_n ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model3/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_34_0_n ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_34_0_n ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_203_0 = _rtB
-> B_80_33_0_b [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_16_0_0 , 499U ) ] ; _rtB -> B_80_204_0 =
_rtB -> B_80_36_0_l * _rtB -> B_80_203_0 ; if ( ( _rtP -> P_871 != 0 ) && ( !
( _rtB -> B_16_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_871 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_h == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_h < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_h ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_35_0_b ) ; if ( ( _rtP -> P_871 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_871 == 1 ) {
if ( _rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_h == 0U ) { diag
= CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_h < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_h ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_35_0_b ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_35_0_b ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_205_0 = _rtB
-> B_80_33_0_b [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_16_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_l . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_l .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_128 ; _rtB -> B_80_206_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_l . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_l . Last , _rtDW -> TransportDelay_IWORK_l . Tail ,
_rtDW -> TransportDelay_IWORK_l . Head , _rtP -> P_129 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_80_207_0 = 0.0 ; _rtB -> B_80_207_0 += _rtP -> P_131 * _rtX
-> TransferFcn_CSTATE_lu ; _rtB -> B_80_207_0 += _rtP -> P_132 * _rtB ->
B_80_206_0 ; if ( _rtB -> B_80_207_0 > _rtP -> P_134 ) { _rtB -> B_80_208_0 =
_rtB -> B_80_207_0 - _rtP -> P_134 ; } else if ( _rtB -> B_80_207_0 >= _rtP
-> P_133 ) { _rtB -> B_80_208_0 = 0.0 ; } else { _rtB -> B_80_208_0 = _rtB ->
B_80_207_0 - _rtP -> P_133 ; } _rtB -> B_80_209_0 = _rtP -> P_135 * _rtB ->
B_80_208_0 ; if ( _rtB -> B_80_209_0 > _rtP -> P_136 ) { _rtB -> B_80_210_0 =
_rtP -> P_136 ; } else if ( _rtB -> B_80_209_0 < _rtP -> P_137 ) { _rtB ->
B_80_210_0 = _rtP -> P_137 ; } else { _rtB -> B_80_210_0 = _rtB -> B_80_209_0
; } if ( ( _rtDW -> LastMajorTimeA_bs >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW
-> LastMajorTimeB_f >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_80_211_0 =
_rtB -> B_80_210_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_bs < _rtDW ->
LastMajorTimeB_f ) && ( _rtDW -> LastMajorTimeB_f < ssGetTaskTime ( S , 0 ) )
) || ( ( _rtDW -> LastMajorTimeA_bs >= _rtDW -> LastMajorTimeB_f ) && ( _rtDW
-> LastMajorTimeA_bs >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_f ; rtb_B_80_3_0 = _rtDW ->
PrevYB_mc ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_bs ; rtb_B_80_3_0 = _rtDW -> PrevYA_dx ; } riseValLimit =
deltaT * _rtP -> P_138 ; rateLimiterRate = _rtB -> B_80_210_0 - rtb_B_80_3_0
; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_80_211_0 = rtb_B_80_3_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_139 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_80_211_0 = rtb_B_80_3_0 + deltaT ; } else { _rtB ->
B_80_211_0 = _rtB -> B_80_210_0 ; } } } _rtB -> B_80_212_0 = _rtP -> P_140 *
_rtB -> B_80_211_0 ; _rtB -> B_80_213_0 = _rtB -> B_80_205_0 * _rtB ->
B_80_212_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model3/Data Store Read" ,
0 ) ; _rtB -> B_80_214_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_k != 0 ) {
_rtX -> vv0at_CSTATE_gg = _rtB -> B_80_214_0 ; } _rtB -> B_80_215_0 = _rtX ->
vv0at_CSTATE_gg ; _rtB -> B_80_216_0 = muDoubleScalarAbs ( _rtB -> B_80_215_0
) ; _rtB -> B_80_217_0 = _rtP -> P_141 [ 0 ] ; _rtB -> B_80_217_0 = _rtB ->
B_80_217_0 * _rtB -> B_80_216_0 + _rtP -> P_141 [ 1 ] ; _rtB -> B_80_217_0 =
_rtB -> B_80_217_0 * _rtB -> B_80_216_0 + _rtP -> P_141 [ 2 ] ; _rtB ->
B_80_218_0 = _rtP -> P_142 * _rtB -> B_80_217_0 ; _rtB -> B_80_219_0 = _rtB
-> B_80_203_0 * _rtB -> B_80_218_0 ; _rtB -> B_80_220_0 = _rtB -> B_80_213_0
- _rtB -> B_80_219_0 ; isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0
) { _rtB -> B_80_222_0 = _rtP -> P_143 * _rtB -> B_80_204_0 ; } if ( _rtB ->
B_80_220_0 > _rtB -> B_80_204_0 ) { _rtB -> B_80_224_0 = _rtB -> B_80_204_0 ;
} else { if ( _rtB -> B_80_220_0 < _rtB -> B_80_222_0 ) { _rtB -> B_17_1_0 =
_rtB -> B_80_222_0 ; } else { _rtB -> B_17_1_0 = _rtB -> B_80_220_0 ; } _rtB
-> B_80_224_0 = _rtB -> B_17_1_0 ; } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if
( isHit != 0 ) { ssCallAccelRunBlock ( S , 18 , 0 , SS_CALL_MDL_OUTPUTS ) ;
if ( ( _rtP -> P_872 != 0 ) && ( ! ( _rtB -> B_18_0_0 <= 499.0 ) ) ) { if (
_rtP -> P_872 == 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_i ==
0U ) { diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model4/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_i < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_i ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model4/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_38_0_h ) ; if ( ( _rtP -> P_872 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_872 == 1 ) {
if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_i == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model4/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_i < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_i ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model4/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_38_0_h ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_38_0_h ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_226_0 = _rtB
-> B_80_37_0 [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_18_0_0 , 499U ) ] ; _rtB -> B_80_227_0 =
_rtB -> B_80_40_0_b * _rtB -> B_80_226_0 ; if ( ( _rtP -> P_873 != 0 ) && ( !
( _rtB -> B_18_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_873 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_np == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_np < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_np ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_39_0 ) ; if ( ( _rtP -> P_873 != 0 ) && ( ( ! ( rtb_B_80_3_0 >=
0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_873 == 1 ) { if
( _rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_np == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_np < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_np ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_39_0 ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_39_0 ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if (
rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_228_0 = _rtB ->
B_80_37_0 [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_18_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_e . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_e .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_144 ; _rtB -> B_80_229_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_i . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_i . Last , _rtDW -> TransportDelay_IWORK_i . Tail ,
_rtDW -> TransportDelay_IWORK_i . Head , _rtP -> P_145 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_80_230_0 = 0.0 ; _rtB -> B_80_230_0 += _rtP -> P_147 * _rtX
-> TransferFcn_CSTATE_n ; _rtB -> B_80_230_0 += _rtP -> P_148 * _rtB ->
B_80_229_0 ; if ( _rtB -> B_80_230_0 > _rtP -> P_150 ) { _rtB -> B_80_231_0 =
_rtB -> B_80_230_0 - _rtP -> P_150 ; } else if ( _rtB -> B_80_230_0 >= _rtP
-> P_149 ) { _rtB -> B_80_231_0 = 0.0 ; } else { _rtB -> B_80_231_0 = _rtB ->
B_80_230_0 - _rtP -> P_149 ; } _rtB -> B_80_232_0 = _rtP -> P_151 * _rtB ->
B_80_231_0 ; if ( _rtB -> B_80_232_0 > _rtP -> P_152 ) { _rtB -> B_80_233_0 =
_rtP -> P_152 ; } else if ( _rtB -> B_80_232_0 < _rtP -> P_153 ) { _rtB ->
B_80_233_0 = _rtP -> P_153 ; } else { _rtB -> B_80_233_0 = _rtB -> B_80_232_0
; } if ( ( _rtDW -> LastMajorTimeA_er >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW
-> LastMajorTimeB_p >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_80_234_0 =
_rtB -> B_80_233_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_er < _rtDW ->
LastMajorTimeB_p ) && ( _rtDW -> LastMajorTimeB_p < ssGetTaskTime ( S , 0 ) )
) || ( ( _rtDW -> LastMajorTimeA_er >= _rtDW -> LastMajorTimeB_p ) && ( _rtDW
-> LastMajorTimeA_er >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_p ; rtb_B_80_3_0 = _rtDW ->
PrevYB_om ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_er ; rtb_B_80_3_0 = _rtDW -> PrevYA_o4 ; } riseValLimit =
deltaT * _rtP -> P_154 ; rateLimiterRate = _rtB -> B_80_233_0 - rtb_B_80_3_0
; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_80_234_0 = rtb_B_80_3_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_155 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_80_234_0 = rtb_B_80_3_0 + deltaT ; } else { _rtB ->
B_80_234_0 = _rtB -> B_80_233_0 ; } } } _rtB -> B_80_235_0 = _rtP -> P_156 *
_rtB -> B_80_234_0 ; _rtB -> B_80_236_0 = _rtB -> B_80_228_0 * _rtB ->
B_80_235_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem1/Wagon model4/Data Store Read" ,
0 ) ; _rtB -> B_80_237_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_f != 0 ) {
_rtX -> vv0at_CSTATE_a = _rtB -> B_80_237_0 ; } _rtB -> B_80_238_0 = _rtX ->
vv0at_CSTATE_a ; _rtB -> B_80_239_0 = muDoubleScalarAbs ( _rtB -> B_80_238_0
) ; _rtB -> B_80_240_0 = _rtP -> P_157 [ 0 ] ; _rtB -> B_80_240_0 = _rtB ->
B_80_240_0 * _rtB -> B_80_239_0 + _rtP -> P_157 [ 1 ] ; _rtB -> B_80_240_0 =
_rtB -> B_80_240_0 * _rtB -> B_80_239_0 + _rtP -> P_157 [ 2 ] ; _rtB ->
B_80_241_0 = _rtP -> P_158 * _rtB -> B_80_240_0 ; _rtB -> B_80_242_0 = _rtB
-> B_80_226_0 * _rtB -> B_80_241_0 ; _rtB -> B_80_243_0 = _rtB -> B_80_236_0
- _rtB -> B_80_242_0 ; isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0
) { _rtB -> B_80_245_0 = _rtP -> P_159 * _rtB -> B_80_227_0 ; } if ( _rtB ->
B_80_243_0 > _rtB -> B_80_227_0 ) { _rtB -> B_80_247_0 = _rtB -> B_80_227_0 ;
} else { if ( _rtB -> B_80_243_0 < _rtB -> B_80_245_0 ) { _rtB -> B_19_1_0 =
_rtB -> B_80_245_0 ; } else { _rtB -> B_19_1_0 = _rtB -> B_80_243_0 ; } _rtB
-> B_80_247_0 = _rtB -> B_19_1_0 ; } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if
( isHit != 0 ) { ssCallAccelRunBlock ( S , 20 , 0 , SS_CALL_MDL_OUTPUTS ) ;
if ( ( _rtP -> P_874 != 0 ) && ( ! ( _rtB -> B_20_0_0 <= 499.0 ) ) ) { if (
_rtP -> P_874 == 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_n3 ==
0U ) { diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model/lookup wagon mass" )
; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_n3 < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_n3 ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model/lookup wagon mass" )
; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_42_0_d ) ; if ( ( _rtP -> P_874 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_874 == 1 ) {
if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_n3 == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model/lookup wagon mass" )
; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_n3 < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_n3 ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model/lookup wagon mass" )
; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_42_0_d ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_42_0_d ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_249_0 = _rtB
-> B_80_41_0 [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_20_0_0 , 499U ) ] ; _rtB -> B_80_250_0 =
_rtB -> B_80_44_0_b * _rtB -> B_80_249_0 ; if ( ( _rtP -> P_875 != 0 ) && ( !
( _rtB -> B_20_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_875 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_p == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_p < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_p ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_43_0_e ) ; if ( ( _rtP -> P_875 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_875 == 1 ) {
if ( _rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_p == 0U ) { diag
= CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_p < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_p ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_43_0_e ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_43_0_e ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_251_0 = _rtB
-> B_80_41_0 [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_20_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_ct . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_ct .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_160 ; _rtB -> B_80_252_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_a4 . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_a4 . Last , _rtDW -> TransportDelay_IWORK_a4 . Tail ,
_rtDW -> TransportDelay_IWORK_a4 . Head , _rtP -> P_161 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_80_253_0 = 0.0 ; _rtB -> B_80_253_0 += _rtP -> P_163 * _rtX
-> TransferFcn_CSTATE_c ; _rtB -> B_80_253_0 += _rtP -> P_164 * _rtB ->
B_80_252_0 ; if ( _rtB -> B_80_253_0 > _rtP -> P_166 ) { _rtB -> B_80_254_0 =
_rtB -> B_80_253_0 - _rtP -> P_166 ; } else if ( _rtB -> B_80_253_0 >= _rtP
-> P_165 ) { _rtB -> B_80_254_0 = 0.0 ; } else { _rtB -> B_80_254_0 = _rtB ->
B_80_253_0 - _rtP -> P_165 ; } _rtB -> B_80_255_0 = _rtP -> P_167 * _rtB ->
B_80_254_0 ; if ( _rtB -> B_80_255_0 > _rtP -> P_168 ) { _rtB -> B_80_256_0 =
_rtP -> P_168 ; } else if ( _rtB -> B_80_255_0 < _rtP -> P_169 ) { _rtB ->
B_80_256_0 = _rtP -> P_169 ; } else { _rtB -> B_80_256_0 = _rtB -> B_80_255_0
; } if ( ( _rtDW -> LastMajorTimeA_d >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW
-> LastMajorTimeB_n >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_80_257_0 =
_rtB -> B_80_256_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_d < _rtDW ->
LastMajorTimeB_n ) && ( _rtDW -> LastMajorTimeB_n < ssGetTaskTime ( S , 0 ) )
) || ( ( _rtDW -> LastMajorTimeA_d >= _rtDW -> LastMajorTimeB_n ) && ( _rtDW
-> LastMajorTimeA_d >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime
( S , 0 ) - _rtDW -> LastMajorTimeB_n ; rtb_B_80_3_0 = _rtDW -> PrevYB_jx ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_d ;
rtb_B_80_3_0 = _rtDW -> PrevYA_nl ; } riseValLimit = deltaT * _rtP -> P_170 ;
rateLimiterRate = _rtB -> B_80_256_0 - rtb_B_80_3_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_80_257_0 = rtb_B_80_3_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_171 ; if ( rateLimiterRate < deltaT ) { _rtB ->
B_80_257_0 = rtb_B_80_3_0 + deltaT ; } else { _rtB -> B_80_257_0 = _rtB ->
B_80_256_0 ; } } } _rtB -> B_80_258_0 = _rtP -> P_172 * _rtB -> B_80_257_0 ;
_rtB -> B_80_259_0 = _rtB -> B_80_251_0 * _rtB -> B_80_258_0 ;
vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model/Data Store Read" , 0
) ; _rtB -> B_80_260_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_p != 0 ) {
_rtX -> vv0at_CSTATE_b = _rtB -> B_80_260_0 ; } _rtB -> B_80_261_0 = _rtX ->
vv0at_CSTATE_b ; _rtB -> B_80_262_0 = muDoubleScalarAbs ( _rtB -> B_80_261_0
) ; _rtB -> B_80_263_0 = _rtP -> P_173 [ 0 ] ; _rtB -> B_80_263_0 = _rtB ->
B_80_263_0 * _rtB -> B_80_262_0 + _rtP -> P_173 [ 1 ] ; _rtB -> B_80_263_0 =
_rtB -> B_80_263_0 * _rtB -> B_80_262_0 + _rtP -> P_173 [ 2 ] ; _rtB ->
B_80_264_0 = _rtP -> P_174 * _rtB -> B_80_263_0 ; _rtB -> B_80_265_0 = _rtB
-> B_80_249_0 * _rtB -> B_80_264_0 ; _rtB -> B_80_266_0 = _rtB -> B_80_259_0
- _rtB -> B_80_265_0 ; isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0
) { _rtB -> B_80_268_0 = _rtP -> P_175 * _rtB -> B_80_250_0 ; } if ( _rtB ->
B_80_266_0 > _rtB -> B_80_250_0 ) { _rtB -> B_80_270_0 = _rtB -> B_80_250_0 ;
} else { if ( _rtB -> B_80_266_0 < _rtB -> B_80_268_0 ) { _rtB -> B_21_1_0 =
_rtB -> B_80_268_0 ; } else { _rtB -> B_21_1_0 = _rtB -> B_80_266_0 ; } _rtB
-> B_80_270_0 = _rtB -> B_21_1_0 ; } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if
( isHit != 0 ) { ssCallAccelRunBlock ( S , 22 , 0 , SS_CALL_MDL_OUTPUTS ) ;
if ( ( _rtP -> P_876 != 0 ) && ( ! ( _rtB -> B_22_0_0 <= 499.0 ) ) ) { if (
_rtP -> P_876 == 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_jy ==
0U ) { diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model1/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_jy < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_jy ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model1/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_46_0_f ) ; if ( ( _rtP -> P_876 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_876 == 1 ) {
if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_jy == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model1/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_jy < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_jy ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model1/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_46_0_f ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_46_0_f ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_272_0 = _rtB
-> B_80_45_0_j [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_22_0_0 , 499U ) ] ; _rtB -> B_80_273_0 =
_rtB -> B_80_48_0_j * _rtB -> B_80_272_0 ; if ( ( _rtP -> P_877 != 0 ) && ( !
( _rtB -> B_22_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_877 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_no == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_no < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_no ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_47_0_a ) ; if ( ( _rtP -> P_877 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_877 == 1 ) {
if ( _rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_no == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_no < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_no ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_47_0_a ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_47_0_a ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_274_0 = _rtB
-> B_80_45_0_j [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_22_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_d . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_d .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_176 ; _rtB -> B_80_275_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_co . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_co . Last , _rtDW -> TransportDelay_IWORK_co . Tail ,
_rtDW -> TransportDelay_IWORK_co . Head , _rtP -> P_177 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_80_276_0 = 0.0 ; _rtB -> B_80_276_0 += _rtP -> P_179 * _rtX
-> TransferFcn_CSTATE_k ; _rtB -> B_80_276_0 += _rtP -> P_180 * _rtB ->
B_80_275_0 ; if ( _rtB -> B_80_276_0 > _rtP -> P_182 ) { _rtB -> B_80_277_0 =
_rtB -> B_80_276_0 - _rtP -> P_182 ; } else if ( _rtB -> B_80_276_0 >= _rtP
-> P_181 ) { _rtB -> B_80_277_0 = 0.0 ; } else { _rtB -> B_80_277_0 = _rtB ->
B_80_276_0 - _rtP -> P_181 ; } _rtB -> B_80_278_0 = _rtP -> P_183 * _rtB ->
B_80_277_0 ; if ( _rtB -> B_80_278_0 > _rtP -> P_184 ) { _rtB -> B_80_279_0 =
_rtP -> P_184 ; } else if ( _rtB -> B_80_278_0 < _rtP -> P_185 ) { _rtB ->
B_80_279_0 = _rtP -> P_185 ; } else { _rtB -> B_80_279_0 = _rtB -> B_80_278_0
; } if ( ( _rtDW -> LastMajorTimeA_c >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW
-> LastMajorTimeB_l >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_80_280_0 =
_rtB -> B_80_279_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_c < _rtDW ->
LastMajorTimeB_l ) && ( _rtDW -> LastMajorTimeB_l < ssGetTaskTime ( S , 0 ) )
) || ( ( _rtDW -> LastMajorTimeA_c >= _rtDW -> LastMajorTimeB_l ) && ( _rtDW
-> LastMajorTimeA_c >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime
( S , 0 ) - _rtDW -> LastMajorTimeB_l ; rtb_B_80_3_0 = _rtDW -> PrevYB_my ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_c ;
rtb_B_80_3_0 = _rtDW -> PrevYA_g ; } riseValLimit = deltaT * _rtP -> P_186 ;
rateLimiterRate = _rtB -> B_80_279_0 - rtb_B_80_3_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_80_280_0 = rtb_B_80_3_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_187 ; if ( rateLimiterRate < deltaT ) { _rtB ->
B_80_280_0 = rtb_B_80_3_0 + deltaT ; } else { _rtB -> B_80_280_0 = _rtB ->
B_80_279_0 ; } } } _rtB -> B_80_281_0 = _rtP -> P_188 * _rtB -> B_80_280_0 ;
_rtB -> B_80_282_0 = _rtB -> B_80_274_0 * _rtB -> B_80_281_0 ;
vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model1/Data Store Read" ,
0 ) ; _rtB -> B_80_283_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_g != 0 ) {
_rtX -> vv0at_CSTATE_o = _rtB -> B_80_283_0 ; } _rtB -> B_80_284_0 = _rtX ->
vv0at_CSTATE_o ; _rtB -> B_80_285_0 = muDoubleScalarAbs ( _rtB -> B_80_284_0
) ; _rtB -> B_80_286_0 = _rtP -> P_189 [ 0 ] ; _rtB -> B_80_286_0 = _rtB ->
B_80_286_0 * _rtB -> B_80_285_0 + _rtP -> P_189 [ 1 ] ; _rtB -> B_80_286_0 =
_rtB -> B_80_286_0 * _rtB -> B_80_285_0 + _rtP -> P_189 [ 2 ] ; _rtB ->
B_80_287_0 = _rtP -> P_190 * _rtB -> B_80_286_0 ; _rtB -> B_80_288_0 = _rtB
-> B_80_272_0 * _rtB -> B_80_287_0 ; _rtB -> B_80_289_0 = _rtB -> B_80_282_0
- _rtB -> B_80_288_0 ; isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0
) { _rtB -> B_80_291_0 = _rtP -> P_191 * _rtB -> B_80_273_0 ; } if ( _rtB ->
B_80_289_0 > _rtB -> B_80_273_0 ) { _rtB -> B_80_293_0 = _rtB -> B_80_273_0 ;
} else { if ( _rtB -> B_80_289_0 < _rtB -> B_80_291_0 ) { _rtB -> B_23_1_0 =
_rtB -> B_80_291_0 ; } else { _rtB -> B_23_1_0 = _rtB -> B_80_289_0 ; } _rtB
-> B_80_293_0 = _rtB -> B_23_1_0 ; } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if
( isHit != 0 ) { ssCallAccelRunBlock ( S , 24 , 0 , SS_CALL_MDL_OUTPUTS ) ;
if ( ( _rtP -> P_878 != 0 ) && ( ! ( _rtB -> B_24_0_0 <= 499.0 ) ) ) { if (
_rtP -> P_878 == 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_jh ==
0U ) { diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model2/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_jh < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_jh ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model2/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_50_0_o ) ; if ( ( _rtP -> P_878 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_878 == 1 ) {
if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_jh == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model2/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_jh < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_jh ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model2/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_50_0_o ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_50_0_o ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_295_0 = _rtB
-> B_80_49_0_j [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_24_0_0 , 499U ) ] ; _rtB -> B_80_296_0 =
_rtB -> B_80_52_0_i * _rtB -> B_80_295_0 ; if ( ( _rtP -> P_879 != 0 ) && ( !
( _rtB -> B_24_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_879 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_bo == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_bo < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_bo ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_51_0_n ) ; if ( ( _rtP -> P_879 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_879 == 1 ) {
if ( _rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_bo == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_bo < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_bo ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_51_0_n ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_51_0_n ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_297_0 = _rtB
-> B_80_49_0_j [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_24_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_la . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_la .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_192 ; _rtB -> B_80_298_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_fv . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_fv . Last , _rtDW -> TransportDelay_IWORK_fv . Tail ,
_rtDW -> TransportDelay_IWORK_fv . Head , _rtP -> P_193 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_80_299_0 = 0.0 ; _rtB -> B_80_299_0 += _rtP -> P_195 * _rtX
-> TransferFcn_CSTATE_dt ; _rtB -> B_80_299_0 += _rtP -> P_196 * _rtB ->
B_80_298_0 ; if ( _rtB -> B_80_299_0 > _rtP -> P_198 ) { _rtB -> B_80_300_0 =
_rtB -> B_80_299_0 - _rtP -> P_198 ; } else if ( _rtB -> B_80_299_0 >= _rtP
-> P_197 ) { _rtB -> B_80_300_0 = 0.0 ; } else { _rtB -> B_80_300_0 = _rtB ->
B_80_299_0 - _rtP -> P_197 ; } _rtB -> B_80_301_0 = _rtP -> P_199 * _rtB ->
B_80_300_0 ; if ( _rtB -> B_80_301_0 > _rtP -> P_200 ) { _rtB -> B_80_302_0 =
_rtP -> P_200 ; } else if ( _rtB -> B_80_301_0 < _rtP -> P_201 ) { _rtB ->
B_80_302_0 = _rtP -> P_201 ; } else { _rtB -> B_80_302_0 = _rtB -> B_80_301_0
; } if ( ( _rtDW -> LastMajorTimeA_l >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW
-> LastMajorTimeB_e >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_80_303_0 =
_rtB -> B_80_302_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_l < _rtDW ->
LastMajorTimeB_e ) && ( _rtDW -> LastMajorTimeB_e < ssGetTaskTime ( S , 0 ) )
) || ( ( _rtDW -> LastMajorTimeA_l >= _rtDW -> LastMajorTimeB_e ) && ( _rtDW
-> LastMajorTimeA_l >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime
( S , 0 ) - _rtDW -> LastMajorTimeB_e ; rtb_B_80_3_0 = _rtDW -> PrevYB_l ; }
else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_l ;
rtb_B_80_3_0 = _rtDW -> PrevYA_gl ; } riseValLimit = deltaT * _rtP -> P_202 ;
rateLimiterRate = _rtB -> B_80_302_0 - rtb_B_80_3_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_80_303_0 = rtb_B_80_3_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_203 ; if ( rateLimiterRate < deltaT ) { _rtB ->
B_80_303_0 = rtb_B_80_3_0 + deltaT ; } else { _rtB -> B_80_303_0 = _rtB ->
B_80_302_0 ; } } } _rtB -> B_80_304_0 = _rtP -> P_204 * _rtB -> B_80_303_0 ;
_rtB -> B_80_305_0 = _rtB -> B_80_297_0 * _rtB -> B_80_304_0 ;
vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model2/Data Store Read" ,
0 ) ; _rtB -> B_80_306_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_bm != 0 )
{ _rtX -> vv0at_CSTATE_om = _rtB -> B_80_306_0 ; } _rtB -> B_80_307_0 = _rtX
-> vv0at_CSTATE_om ; _rtB -> B_80_308_0 = muDoubleScalarAbs ( _rtB ->
B_80_307_0 ) ; _rtB -> B_80_309_0 = _rtP -> P_205 [ 0 ] ; _rtB -> B_80_309_0
= _rtB -> B_80_309_0 * _rtB -> B_80_308_0 + _rtP -> P_205 [ 1 ] ; _rtB ->
B_80_309_0 = _rtB -> B_80_309_0 * _rtB -> B_80_308_0 + _rtP -> P_205 [ 2 ] ;
_rtB -> B_80_310_0 = _rtP -> P_206 * _rtB -> B_80_309_0 ; _rtB -> B_80_311_0
= _rtB -> B_80_295_0 * _rtB -> B_80_310_0 ; _rtB -> B_80_312_0 = _rtB ->
B_80_305_0 - _rtB -> B_80_311_0 ; isHit = ssIsSampleHit ( S , 2 , 0 ) ; if (
isHit != 0 ) { _rtB -> B_80_314_0 = _rtP -> P_207 * _rtB -> B_80_296_0 ; } if
( _rtB -> B_80_312_0 > _rtB -> B_80_296_0 ) { _rtB -> B_80_316_0 = _rtB ->
B_80_296_0 ; } else { if ( _rtB -> B_80_312_0 < _rtB -> B_80_314_0 ) { _rtB
-> B_25_1_0 = _rtB -> B_80_314_0 ; } else { _rtB -> B_25_1_0 = _rtB ->
B_80_312_0 ; } _rtB -> B_80_316_0 = _rtB -> B_25_1_0 ; } isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { ssCallAccelRunBlock ( S ,
26 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP -> P_880 != 0 ) && ( ! ( _rtB ->
B_26_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_880 == 1 ) { if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_o == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model3/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_o < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_o ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model3/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_54_0_n ) ; if ( ( _rtP -> P_880 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_880 == 1 ) {
if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_o == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model3/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_o < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_o ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model3/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_54_0_n ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_54_0_n ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_318_0 = _rtB
-> B_80_53_0_o [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_26_0_0 , 499U ) ] ; _rtB -> B_80_319_0 =
_rtB -> B_80_56_0_c * _rtB -> B_80_318_0 ; if ( ( _rtP -> P_881 != 0 ) && ( !
( _rtB -> B_26_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_881 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_bv == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_bv < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_bv ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_55_0_m ) ; if ( ( _rtP -> P_881 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_881 == 1 ) {
if ( _rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_bv == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_bv < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_bv ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_55_0_m ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_55_0_m ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_320_0 = _rtB
-> B_80_53_0_o [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_26_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_nw . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_nw .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_208 ; _rtB -> B_80_321_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_p . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_p . Last , _rtDW -> TransportDelay_IWORK_p . Tail ,
_rtDW -> TransportDelay_IWORK_p . Head , _rtP -> P_209 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_80_322_0 = 0.0 ; _rtB -> B_80_322_0 += _rtP -> P_211 * _rtX
-> TransferFcn_CSTATE_ez ; _rtB -> B_80_322_0 += _rtP -> P_212 * _rtB ->
B_80_321_0 ; if ( _rtB -> B_80_322_0 > _rtP -> P_214 ) { _rtB -> B_80_323_0 =
_rtB -> B_80_322_0 - _rtP -> P_214 ; } else if ( _rtB -> B_80_322_0 >= _rtP
-> P_213 ) { _rtB -> B_80_323_0 = 0.0 ; } else { _rtB -> B_80_323_0 = _rtB ->
B_80_322_0 - _rtP -> P_213 ; } _rtB -> B_80_324_0 = _rtP -> P_215 * _rtB ->
B_80_323_0 ; if ( _rtB -> B_80_324_0 > _rtP -> P_216 ) { _rtB -> B_80_325_0 =
_rtP -> P_216 ; } else if ( _rtB -> B_80_324_0 < _rtP -> P_217 ) { _rtB ->
B_80_325_0 = _rtP -> P_217 ; } else { _rtB -> B_80_325_0 = _rtB -> B_80_324_0
; } if ( ( _rtDW -> LastMajorTimeA_j >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW
-> LastMajorTimeB_gi >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_80_326_0 =
_rtB -> B_80_325_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_j < _rtDW ->
LastMajorTimeB_gi ) && ( _rtDW -> LastMajorTimeB_gi < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_j >= _rtDW -> LastMajorTimeB_gi ) && (
_rtDW -> LastMajorTimeA_j >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_gi ; rtb_B_80_3_0 = _rtDW
-> PrevYB_a ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_j ; rtb_B_80_3_0 = _rtDW -> PrevYA_dh ; } riseValLimit =
deltaT * _rtP -> P_218 ; rateLimiterRate = _rtB -> B_80_325_0 - rtb_B_80_3_0
; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_80_326_0 = rtb_B_80_3_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_219 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_80_326_0 = rtb_B_80_3_0 + deltaT ; } else { _rtB ->
B_80_326_0 = _rtB -> B_80_325_0 ; } } } _rtB -> B_80_327_0 = _rtP -> P_220 *
_rtB -> B_80_326_0 ; _rtB -> B_80_328_0 = _rtB -> B_80_320_0 * _rtB ->
B_80_327_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model3/Data Store Read" ,
0 ) ; _rtB -> B_80_329_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_e != 0 ) {
_rtX -> vv0at_CSTATE_l = _rtB -> B_80_329_0 ; } _rtB -> B_80_330_0 = _rtX ->
vv0at_CSTATE_l ; _rtB -> B_80_331_0 = muDoubleScalarAbs ( _rtB -> B_80_330_0
) ; _rtB -> B_80_332_0 = _rtP -> P_221 [ 0 ] ; _rtB -> B_80_332_0 = _rtB ->
B_80_332_0 * _rtB -> B_80_331_0 + _rtP -> P_221 [ 1 ] ; _rtB -> B_80_332_0 =
_rtB -> B_80_332_0 * _rtB -> B_80_331_0 + _rtP -> P_221 [ 2 ] ; _rtB ->
B_80_333_0 = _rtP -> P_222 * _rtB -> B_80_332_0 ; _rtB -> B_80_334_0 = _rtB
-> B_80_318_0 * _rtB -> B_80_333_0 ; _rtB -> B_80_335_0 = _rtB -> B_80_328_0
- _rtB -> B_80_334_0 ; isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0
) { _rtB -> B_80_337_0 = _rtP -> P_223 * _rtB -> B_80_319_0 ; } if ( _rtB ->
B_80_335_0 > _rtB -> B_80_319_0 ) { _rtB -> B_80_339_0 = _rtB -> B_80_319_0 ;
} else { if ( _rtB -> B_80_335_0 < _rtB -> B_80_337_0 ) { _rtB -> B_27_1_0 =
_rtB -> B_80_337_0 ; } else { _rtB -> B_27_1_0 = _rtB -> B_80_335_0 ; } _rtB
-> B_80_339_0 = _rtB -> B_27_1_0 ; } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if
( isHit != 0 ) { ssCallAccelRunBlock ( S , 28 , 0 , SS_CALL_MDL_OUTPUTS ) ;
if ( ( _rtP -> P_882 != 0 ) && ( ! ( _rtB -> B_28_0_0 <= 499.0 ) ) ) { if (
_rtP -> P_882 == 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_ei ==
0U ) { diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model4/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_ei < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_ei ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model4/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_58_0_m ) ; if ( ( _rtP -> P_882 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_882 == 1 ) {
if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_ei == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model4/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_ei < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_ei ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model4/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_58_0_m ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_58_0_m ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_341_0 = _rtB
-> B_80_57_0_m [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_28_0_0 , 499U ) ] ; _rtB -> B_80_342_0 =
_rtB -> B_80_60_0 * _rtB -> B_80_341_0 ; if ( ( _rtP -> P_883 != 0 ) && ( ! (
_rtB -> B_28_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_883 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_gn == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_gn < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_gn ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_59_0_j ) ; if ( ( _rtP -> P_883 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_883 == 1 ) {
if ( _rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_gn == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_gn < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_gn ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_59_0_j ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_59_0_j ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_343_0 = _rtB
-> B_80_57_0_m [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_28_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_dl . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_dl .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_224 ; _rtB -> B_80_344_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_hc . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_hc . Last , _rtDW -> TransportDelay_IWORK_hc . Tail ,
_rtDW -> TransportDelay_IWORK_hc . Head , _rtP -> P_225 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_80_345_0 = 0.0 ; _rtB -> B_80_345_0 += _rtP -> P_227 * _rtX
-> TransferFcn_CSTATE_m ; _rtB -> B_80_345_0 += _rtP -> P_228 * _rtB ->
B_80_344_0 ; if ( _rtB -> B_80_345_0 > _rtP -> P_230 ) { _rtB -> B_80_346_0 =
_rtB -> B_80_345_0 - _rtP -> P_230 ; } else if ( _rtB -> B_80_345_0 >= _rtP
-> P_229 ) { _rtB -> B_80_346_0 = 0.0 ; } else { _rtB -> B_80_346_0 = _rtB ->
B_80_345_0 - _rtP -> P_229 ; } _rtB -> B_80_347_0 = _rtP -> P_231 * _rtB ->
B_80_346_0 ; if ( _rtB -> B_80_347_0 > _rtP -> P_232 ) { _rtB -> B_80_348_0 =
_rtP -> P_232 ; } else if ( _rtB -> B_80_347_0 < _rtP -> P_233 ) { _rtB ->
B_80_348_0 = _rtP -> P_233 ; } else { _rtB -> B_80_348_0 = _rtB -> B_80_347_0
; } if ( ( _rtDW -> LastMajorTimeA_ns >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW
-> LastMajorTimeB_a >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_80_349_0 =
_rtB -> B_80_348_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_ns < _rtDW ->
LastMajorTimeB_a ) && ( _rtDW -> LastMajorTimeB_a < ssGetTaskTime ( S , 0 ) )
) || ( ( _rtDW -> LastMajorTimeA_ns >= _rtDW -> LastMajorTimeB_a ) && ( _rtDW
-> LastMajorTimeA_ns >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_a ; rtb_B_80_3_0 = _rtDW ->
PrevYB_d ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_ns ; rtb_B_80_3_0 = _rtDW -> PrevYA_b ; } riseValLimit =
deltaT * _rtP -> P_234 ; rateLimiterRate = _rtB -> B_80_348_0 - rtb_B_80_3_0
; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_80_349_0 = rtb_B_80_3_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_235 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_80_349_0 = rtb_B_80_3_0 + deltaT ; } else { _rtB ->
B_80_349_0 = _rtB -> B_80_348_0 ; } } } _rtB -> B_80_350_0 = _rtP -> P_236 *
_rtB -> B_80_349_0 ; _rtB -> B_80_351_0 = _rtB -> B_80_343_0 * _rtB ->
B_80_350_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem2/Wagon model4/Data Store Read" ,
0 ) ; _rtB -> B_80_352_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_bg != 0 )
{ _rtX -> vv0at_CSTATE_p = _rtB -> B_80_352_0 ; } _rtB -> B_80_353_0 = _rtX
-> vv0at_CSTATE_p ; _rtB -> B_80_354_0 = muDoubleScalarAbs ( _rtB ->
B_80_353_0 ) ; _rtB -> B_80_355_0 = _rtP -> P_237 [ 0 ] ; _rtB -> B_80_355_0
= _rtB -> B_80_355_0 * _rtB -> B_80_354_0 + _rtP -> P_237 [ 1 ] ; _rtB ->
B_80_355_0 = _rtB -> B_80_355_0 * _rtB -> B_80_354_0 + _rtP -> P_237 [ 2 ] ;
_rtB -> B_80_356_0 = _rtP -> P_238 * _rtB -> B_80_355_0 ; _rtB -> B_80_357_0
= _rtB -> B_80_341_0 * _rtB -> B_80_356_0 ; _rtB -> B_80_358_0 = _rtB ->
B_80_351_0 - _rtB -> B_80_357_0 ; isHit = ssIsSampleHit ( S , 2 , 0 ) ; if (
isHit != 0 ) { _rtB -> B_80_360_0 = _rtP -> P_239 * _rtB -> B_80_342_0 ; } if
( _rtB -> B_80_358_0 > _rtB -> B_80_342_0 ) { _rtB -> B_80_362_0 = _rtB ->
B_80_342_0 ; } else { if ( _rtB -> B_80_358_0 < _rtB -> B_80_360_0 ) { _rtB
-> B_29_1_0 = _rtB -> B_80_360_0 ; } else { _rtB -> B_29_1_0 = _rtB ->
B_80_358_0 ; } _rtB -> B_80_362_0 = _rtB -> B_29_1_0 ; } isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { ssCallAccelRunBlock ( S ,
30 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP -> P_884 != 0 ) && ( ! ( _rtB ->
B_30_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_884 == 1 ) { if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_d == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model/lookup wagon mass" )
; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_d < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_d ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model/lookup wagon mass" )
; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_62_0 ) ; if ( ( _rtP -> P_884 != 0 ) && ( ( ! ( rtb_B_80_3_0 >=
0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_884 == 1 ) { if
( _rtDW -> lookupwagonmass_NumIssuedWarnings_d == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model/lookup wagon mass" )
; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_d < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_d ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model/lookup wagon mass" )
; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_62_0 ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_62_0 ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if (
rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_364_0 = _rtB ->
B_80_61_0_h [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_30_0_0 , 499U ) ] ; _rtB -> B_80_365_0 =
_rtB -> B_80_64_0 * _rtB -> B_80_364_0 ; if ( ( _rtP -> P_885 != 0 ) && ( ! (
_rtB -> B_30_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_885 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_p3 == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_p3 < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_p3 ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_63_0_c ) ; if ( ( _rtP -> P_885 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_885 == 1 ) {
if ( _rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_p3 == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_p3 < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_p3 ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_63_0_c ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_63_0_c ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_366_0 = _rtB
-> B_80_61_0_h [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_30_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_m . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_m .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_240 ; _rtB -> B_80_367_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_c0 . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_c0 . Last , _rtDW -> TransportDelay_IWORK_c0 . Tail ,
_rtDW -> TransportDelay_IWORK_c0 . Head , _rtP -> P_241 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_80_368_0 = 0.0 ; _rtB -> B_80_368_0 += _rtP -> P_243 * _rtX
-> TransferFcn_CSTATE_na ; _rtB -> B_80_368_0 += _rtP -> P_244 * _rtB ->
B_80_367_0 ; if ( _rtB -> B_80_368_0 > _rtP -> P_246 ) { _rtB -> B_80_369_0 =
_rtB -> B_80_368_0 - _rtP -> P_246 ; } else if ( _rtB -> B_80_368_0 >= _rtP
-> P_245 ) { _rtB -> B_80_369_0 = 0.0 ; } else { _rtB -> B_80_369_0 = _rtB ->
B_80_368_0 - _rtP -> P_245 ; } _rtB -> B_80_370_0 = _rtP -> P_247 * _rtB ->
B_80_369_0 ; if ( _rtB -> B_80_370_0 > _rtP -> P_248 ) { _rtB -> B_80_371_0 =
_rtP -> P_248 ; } else if ( _rtB -> B_80_370_0 < _rtP -> P_249 ) { _rtB ->
B_80_371_0 = _rtP -> P_249 ; } else { _rtB -> B_80_371_0 = _rtB -> B_80_370_0
; } if ( ( _rtDW -> LastMajorTimeA_fg >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW
-> LastMajorTimeB_ps >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_80_372_0 =
_rtB -> B_80_371_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_fg < _rtDW ->
LastMajorTimeB_ps ) && ( _rtDW -> LastMajorTimeB_ps < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_fg >= _rtDW -> LastMajorTimeB_ps ) && (
_rtDW -> LastMajorTimeA_fg >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_ps ; rtb_B_80_3_0 = _rtDW
-> PrevYB_p ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_fg ; rtb_B_80_3_0 = _rtDW -> PrevYA_e ; } riseValLimit =
deltaT * _rtP -> P_250 ; rateLimiterRate = _rtB -> B_80_371_0 - rtb_B_80_3_0
; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_80_372_0 = rtb_B_80_3_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_251 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_80_372_0 = rtb_B_80_3_0 + deltaT ; } else { _rtB ->
B_80_372_0 = _rtB -> B_80_371_0 ; } } } _rtB -> B_80_373_0 = _rtP -> P_252 *
_rtB -> B_80_372_0 ; _rtB -> B_80_374_0 = _rtB -> B_80_366_0 * _rtB ->
B_80_373_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model/Data Store Read" , 0
) ; _rtB -> B_80_375_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_kl != 0 ) {
_rtX -> vv0at_CSTATE_bi = _rtB -> B_80_375_0 ; } _rtB -> B_80_376_0 = _rtX ->
vv0at_CSTATE_bi ; _rtB -> B_80_377_0 = muDoubleScalarAbs ( _rtB -> B_80_376_0
) ; _rtB -> B_80_378_0 = _rtP -> P_253 [ 0 ] ; _rtB -> B_80_378_0 = _rtB ->
B_80_378_0 * _rtB -> B_80_377_0 + _rtP -> P_253 [ 1 ] ; _rtB -> B_80_378_0 =
_rtB -> B_80_378_0 * _rtB -> B_80_377_0 + _rtP -> P_253 [ 2 ] ; _rtB ->
B_80_379_0 = _rtP -> P_254 * _rtB -> B_80_378_0 ; _rtB -> B_80_380_0 = _rtB
-> B_80_364_0 * _rtB -> B_80_379_0 ; _rtB -> B_80_381_0 = _rtB -> B_80_374_0
- _rtB -> B_80_380_0 ; isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0
) { _rtB -> B_80_383_0 = _rtP -> P_255 * _rtB -> B_80_365_0 ; } if ( _rtB ->
B_80_381_0 > _rtB -> B_80_365_0 ) { _rtB -> B_80_385_0 = _rtB -> B_80_365_0 ;
} else { if ( _rtB -> B_80_381_0 < _rtB -> B_80_383_0 ) { _rtB -> B_31_1_0 =
_rtB -> B_80_383_0 ; } else { _rtB -> B_31_1_0 = _rtB -> B_80_381_0 ; } _rtB
-> B_80_385_0 = _rtB -> B_31_1_0 ; } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if
( isHit != 0 ) { ssCallAccelRunBlock ( S , 32 , 0 , SS_CALL_MDL_OUTPUTS ) ;
if ( ( _rtP -> P_886 != 0 ) && ( ! ( _rtB -> B_32_0_0 <= 499.0 ) ) ) { if (
_rtP -> P_886 == 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_gy ==
0U ) { diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model1/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_gy < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_gy ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model1/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_66_0_p ) ; if ( ( _rtP -> P_886 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_886 == 1 ) {
if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_gy == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model1/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_gy < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_gy ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model1/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_66_0_p ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_66_0_p ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_387_0 = _rtB
-> B_80_65_0_c [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_32_0_0 , 499U ) ] ; _rtB -> B_80_388_0 =
_rtB -> B_80_68_0_a * _rtB -> B_80_387_0 ; if ( ( _rtP -> P_887 != 0 ) && ( !
( _rtB -> B_32_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_887 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_f == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_f < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_f ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_67_0_p ) ; if ( ( _rtP -> P_887 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_887 == 1 ) {
if ( _rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_f == 0U ) { diag
= CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_f < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_f ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_67_0_p ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_67_0_p ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_389_0 = _rtB
-> B_80_65_0_c [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_32_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_ej . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_ej .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_256 ; _rtB -> B_80_390_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_dp . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_dp . Last , _rtDW -> TransportDelay_IWORK_dp . Tail ,
_rtDW -> TransportDelay_IWORK_dp . Head , _rtP -> P_257 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_80_391_0 = 0.0 ; _rtB -> B_80_391_0 += _rtP -> P_259 * _rtX
-> TransferFcn_CSTATE_kl ; _rtB -> B_80_391_0 += _rtP -> P_260 * _rtB ->
B_80_390_0 ; if ( _rtB -> B_80_391_0 > _rtP -> P_262 ) { _rtB -> B_80_392_0 =
_rtB -> B_80_391_0 - _rtP -> P_262 ; } else if ( _rtB -> B_80_391_0 >= _rtP
-> P_261 ) { _rtB -> B_80_392_0 = 0.0 ; } else { _rtB -> B_80_392_0 = _rtB ->
B_80_391_0 - _rtP -> P_261 ; } _rtB -> B_80_393_0 = _rtP -> P_263 * _rtB ->
B_80_392_0 ; if ( _rtB -> B_80_393_0 > _rtP -> P_264 ) { _rtB -> B_80_394_0 =
_rtP -> P_264 ; } else if ( _rtB -> B_80_393_0 < _rtP -> P_265 ) { _rtB ->
B_80_394_0 = _rtP -> P_265 ; } else { _rtB -> B_80_394_0 = _rtB -> B_80_393_0
; } if ( ( _rtDW -> LastMajorTimeA_eq >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW
-> LastMajorTimeB_oc >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_80_395_0 =
_rtB -> B_80_394_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_eq < _rtDW ->
LastMajorTimeB_oc ) && ( _rtDW -> LastMajorTimeB_oc < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_eq >= _rtDW -> LastMajorTimeB_oc ) && (
_rtDW -> LastMajorTimeA_eq >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_oc ; rtb_B_80_3_0 = _rtDW
-> PrevYB_k ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_eq ; rtb_B_80_3_0 = _rtDW -> PrevYA_m ; } riseValLimit =
deltaT * _rtP -> P_266 ; rateLimiterRate = _rtB -> B_80_394_0 - rtb_B_80_3_0
; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_80_395_0 = rtb_B_80_3_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_267 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_80_395_0 = rtb_B_80_3_0 + deltaT ; } else { _rtB ->
B_80_395_0 = _rtB -> B_80_394_0 ; } } } _rtB -> B_80_396_0 = _rtP -> P_268 *
_rtB -> B_80_395_0 ; _rtB -> B_80_397_0 = _rtB -> B_80_389_0 * _rtB ->
B_80_396_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model1/Data Store Read" ,
0 ) ; _rtB -> B_80_398_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_pf != 0 )
{ _rtX -> vv0at_CSTATE_g2 = _rtB -> B_80_398_0 ; } _rtB -> B_80_399_0 = _rtX
-> vv0at_CSTATE_g2 ; _rtB -> B_80_400_0 = muDoubleScalarAbs ( _rtB ->
B_80_399_0 ) ; _rtB -> B_80_401_0 = _rtP -> P_269 [ 0 ] ; _rtB -> B_80_401_0
= _rtB -> B_80_401_0 * _rtB -> B_80_400_0 + _rtP -> P_269 [ 1 ] ; _rtB ->
B_80_401_0 = _rtB -> B_80_401_0 * _rtB -> B_80_400_0 + _rtP -> P_269 [ 2 ] ;
_rtB -> B_80_402_0 = _rtP -> P_270 * _rtB -> B_80_401_0 ; _rtB -> B_80_403_0
= _rtB -> B_80_387_0 * _rtB -> B_80_402_0 ; _rtB -> B_80_404_0 = _rtB ->
B_80_397_0 - _rtB -> B_80_403_0 ; isHit = ssIsSampleHit ( S , 2 , 0 ) ; if (
isHit != 0 ) { _rtB -> B_80_406_0 = _rtP -> P_271 * _rtB -> B_80_388_0 ; } if
( _rtB -> B_80_404_0 > _rtB -> B_80_388_0 ) { _rtB -> B_80_408_0 = _rtB ->
B_80_388_0 ; } else { if ( _rtB -> B_80_404_0 < _rtB -> B_80_406_0 ) { _rtB
-> B_33_1_0 = _rtB -> B_80_406_0 ; } else { _rtB -> B_33_1_0 = _rtB ->
B_80_404_0 ; } _rtB -> B_80_408_0 = _rtB -> B_33_1_0 ; } isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { ssCallAccelRunBlock ( S ,
34 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP -> P_888 != 0 ) && ( ! ( _rtB ->
B_34_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_888 == 1 ) { if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_m == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model2/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_m < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_m ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model2/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_70_0_a ) ; if ( ( _rtP -> P_888 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_888 == 1 ) {
if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_m == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model2/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_m < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_m ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model2/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_70_0_a ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_70_0_a ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_410_0 = _rtB
-> B_80_69_0_e [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_34_0_0 , 499U ) ] ; _rtB -> B_80_411_0 =
_rtB -> B_80_72_0_i * _rtB -> B_80_410_0 ; if ( ( _rtP -> P_889 != 0 ) && ( !
( _rtB -> B_34_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_889 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_e == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_e < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_e ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_71_0_a ) ; if ( ( _rtP -> P_889 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_889 == 1 ) {
if ( _rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_e == 0U ) { diag
= CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_e < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_e ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_71_0_a ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_71_0_a ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_412_0 = _rtB
-> B_80_69_0_e [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_34_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_o4 . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_o4 .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_272 ; _rtB -> B_80_413_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_j . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_j . Last , _rtDW -> TransportDelay_IWORK_j . Tail ,
_rtDW -> TransportDelay_IWORK_j . Head , _rtP -> P_273 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_80_414_0 = 0.0 ; _rtB -> B_80_414_0 += _rtP -> P_275 * _rtX
-> TransferFcn_CSTATE_cj ; _rtB -> B_80_414_0 += _rtP -> P_276 * _rtB ->
B_80_413_0 ; if ( _rtB -> B_80_414_0 > _rtP -> P_278 ) { _rtB -> B_80_415_0 =
_rtB -> B_80_414_0 - _rtP -> P_278 ; } else if ( _rtB -> B_80_414_0 >= _rtP
-> P_277 ) { _rtB -> B_80_415_0 = 0.0 ; } else { _rtB -> B_80_415_0 = _rtB ->
B_80_414_0 - _rtP -> P_277 ; } _rtB -> B_80_416_0 = _rtP -> P_279 * _rtB ->
B_80_415_0 ; if ( _rtB -> B_80_416_0 > _rtP -> P_280 ) { _rtB -> B_80_417_0 =
_rtP -> P_280 ; } else if ( _rtB -> B_80_416_0 < _rtP -> P_281 ) { _rtB ->
B_80_417_0 = _rtP -> P_281 ; } else { _rtB -> B_80_417_0 = _rtB -> B_80_416_0
; } if ( ( _rtDW -> LastMajorTimeA_dq >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW
-> LastMajorTimeB_cb >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_80_418_0 =
_rtB -> B_80_417_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_dq < _rtDW ->
LastMajorTimeB_cb ) && ( _rtDW -> LastMajorTimeB_cb < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_dq >= _rtDW -> LastMajorTimeB_cb ) && (
_rtDW -> LastMajorTimeA_dq >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_cb ; rtb_B_80_3_0 = _rtDW
-> PrevYB_gj ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_dq ; rtb_B_80_3_0 = _rtDW -> PrevYA_a ; } riseValLimit =
deltaT * _rtP -> P_282 ; rateLimiterRate = _rtB -> B_80_417_0 - rtb_B_80_3_0
; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_80_418_0 = rtb_B_80_3_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_283 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_80_418_0 = rtb_B_80_3_0 + deltaT ; } else { _rtB ->
B_80_418_0 = _rtB -> B_80_417_0 ; } } } _rtB -> B_80_419_0 = _rtP -> P_284 *
_rtB -> B_80_418_0 ; _rtB -> B_80_420_0 = _rtB -> B_80_412_0 * _rtB ->
B_80_419_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model2/Data Store Read" ,
0 ) ; _rtB -> B_80_421_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_fs != 0 )
{ _rtX -> vv0at_CSTATE_e = _rtB -> B_80_421_0 ; } _rtB -> B_80_422_0 = _rtX
-> vv0at_CSTATE_e ; _rtB -> B_80_423_0 = muDoubleScalarAbs ( _rtB ->
B_80_422_0 ) ; _rtB -> B_80_424_0 = _rtP -> P_285 [ 0 ] ; _rtB -> B_80_424_0
= _rtB -> B_80_424_0 * _rtB -> B_80_423_0 + _rtP -> P_285 [ 1 ] ; _rtB ->
B_80_424_0 = _rtB -> B_80_424_0 * _rtB -> B_80_423_0 + _rtP -> P_285 [ 2 ] ;
_rtB -> B_80_425_0 = _rtP -> P_286 * _rtB -> B_80_424_0 ; _rtB -> B_80_426_0
= _rtB -> B_80_410_0 * _rtB -> B_80_425_0 ; _rtB -> B_80_427_0 = _rtB ->
B_80_420_0 - _rtB -> B_80_426_0 ; isHit = ssIsSampleHit ( S , 2 , 0 ) ; if (
isHit != 0 ) { _rtB -> B_80_429_0 = _rtP -> P_287 * _rtB -> B_80_411_0 ; } if
( _rtB -> B_80_427_0 > _rtB -> B_80_411_0 ) { _rtB -> B_80_431_0 = _rtB ->
B_80_411_0 ; } else { if ( _rtB -> B_80_427_0 < _rtB -> B_80_429_0 ) { _rtB
-> B_35_1_0 = _rtB -> B_80_429_0 ; } else { _rtB -> B_35_1_0 = _rtB ->
B_80_427_0 ; } _rtB -> B_80_431_0 = _rtB -> B_35_1_0 ; } isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { ssCallAccelRunBlock ( S ,
36 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP -> P_890 != 0 ) && ( ! ( _rtB ->
B_36_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_890 == 1 ) { if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_oh == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model3/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_oh < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_oh ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model3/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_74_0_o ) ; if ( ( _rtP -> P_890 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_890 == 1 ) {
if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_oh == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model3/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_oh < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_oh ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model3/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_74_0_o ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_74_0_o ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_433_0 = _rtB
-> B_80_73_0_l [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_36_0_0 , 499U ) ] ; _rtB -> B_80_434_0 =
_rtB -> B_80_76_0_i * _rtB -> B_80_433_0 ; if ( ( _rtP -> P_891 != 0 ) && ( !
( _rtB -> B_36_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_891 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_hz == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_hz < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_hz ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_75_0_o ) ; if ( ( _rtP -> P_891 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_891 == 1 ) {
if ( _rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_hz == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_hz < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_hz ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_75_0_o ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_75_0_o ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_435_0 = _rtB
-> B_80_73_0_l [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_36_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_a . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_a .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_288 ; _rtB -> B_80_436_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_f1 . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_f1 . Last , _rtDW -> TransportDelay_IWORK_f1 . Tail ,
_rtDW -> TransportDelay_IWORK_f1 . Head , _rtP -> P_289 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_80_437_0 = 0.0 ; _rtB -> B_80_437_0 += _rtP -> P_291 * _rtX
-> TransferFcn_CSTATE_jq ; _rtB -> B_80_437_0 += _rtP -> P_292 * _rtB ->
B_80_436_0 ; if ( _rtB -> B_80_437_0 > _rtP -> P_294 ) { _rtB -> B_80_438_0 =
_rtB -> B_80_437_0 - _rtP -> P_294 ; } else if ( _rtB -> B_80_437_0 >= _rtP
-> P_293 ) { _rtB -> B_80_438_0 = 0.0 ; } else { _rtB -> B_80_438_0 = _rtB ->
B_80_437_0 - _rtP -> P_293 ; } _rtB -> B_80_439_0 = _rtP -> P_295 * _rtB ->
B_80_438_0 ; if ( _rtB -> B_80_439_0 > _rtP -> P_296 ) { _rtB -> B_80_440_0 =
_rtP -> P_296 ; } else if ( _rtB -> B_80_439_0 < _rtP -> P_297 ) { _rtB ->
B_80_440_0 = _rtP -> P_297 ; } else { _rtB -> B_80_440_0 = _rtB -> B_80_439_0
; } if ( ( _rtDW -> LastMajorTimeA_li >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW
-> LastMajorTimeB_f3 >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_80_441_0 =
_rtB -> B_80_440_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_li < _rtDW ->
LastMajorTimeB_f3 ) && ( _rtDW -> LastMajorTimeB_f3 < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_li >= _rtDW -> LastMajorTimeB_f3 ) && (
_rtDW -> LastMajorTimeA_li >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_f3 ; rtb_B_80_3_0 = _rtDW
-> PrevYB_od ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_li ; rtb_B_80_3_0 = _rtDW -> PrevYA_k1 ; } riseValLimit =
deltaT * _rtP -> P_298 ; rateLimiterRate = _rtB -> B_80_440_0 - rtb_B_80_3_0
; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_80_441_0 = rtb_B_80_3_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_299 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_80_441_0 = rtb_B_80_3_0 + deltaT ; } else { _rtB ->
B_80_441_0 = _rtB -> B_80_440_0 ; } } } _rtB -> B_80_442_0 = _rtP -> P_300 *
_rtB -> B_80_441_0 ; _rtB -> B_80_443_0 = _rtB -> B_80_435_0 * _rtB ->
B_80_442_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model3/Data Store Read" ,
0 ) ; _rtB -> B_80_444_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_hd != 0 )
{ _rtX -> vv0at_CSTATE_po = _rtB -> B_80_444_0 ; } _rtB -> B_80_445_0 = _rtX
-> vv0at_CSTATE_po ; _rtB -> B_80_446_0 = muDoubleScalarAbs ( _rtB ->
B_80_445_0 ) ; _rtB -> B_80_447_0 = _rtP -> P_301 [ 0 ] ; _rtB -> B_80_447_0
= _rtB -> B_80_447_0 * _rtB -> B_80_446_0 + _rtP -> P_301 [ 1 ] ; _rtB ->
B_80_447_0 = _rtB -> B_80_447_0 * _rtB -> B_80_446_0 + _rtP -> P_301 [ 2 ] ;
_rtB -> B_80_448_0 = _rtP -> P_302 * _rtB -> B_80_447_0 ; _rtB -> B_80_449_0
= _rtB -> B_80_433_0 * _rtB -> B_80_448_0 ; _rtB -> B_80_450_0 = _rtB ->
B_80_443_0 - _rtB -> B_80_449_0 ; isHit = ssIsSampleHit ( S , 2 , 0 ) ; if (
isHit != 0 ) { _rtB -> B_80_452_0 = _rtP -> P_303 * _rtB -> B_80_434_0 ; } if
( _rtB -> B_80_450_0 > _rtB -> B_80_434_0 ) { _rtB -> B_80_454_0 = _rtB ->
B_80_434_0 ; } else { if ( _rtB -> B_80_450_0 < _rtB -> B_80_452_0 ) { _rtB
-> B_37_1_0 = _rtB -> B_80_452_0 ; } else { _rtB -> B_37_1_0 = _rtB ->
B_80_450_0 ; } _rtB -> B_80_454_0 = _rtB -> B_37_1_0 ; } isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { ssCallAccelRunBlock ( S ,
38 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP -> P_892 != 0 ) && ( ! ( _rtB ->
B_38_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_892 == 1 ) { if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_a5 == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model4/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_a5 < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_a5 ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model4/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_78_0_i ) ; if ( ( _rtP -> P_892 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_892 == 1 ) {
if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_a5 == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model4/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_a5 < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_a5 ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model4/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_78_0_i ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_78_0_i ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_456_0 = _rtB
-> B_80_77_0_f [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_38_0_0 , 499U ) ] ; _rtB -> B_80_457_0 =
_rtB -> B_80_80_0_g * _rtB -> B_80_456_0 ; if ( ( _rtP -> P_893 != 0 ) && ( !
( _rtB -> B_38_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_893 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_b3 == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_b3 < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_b3 ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_79_0_f ) ; if ( ( _rtP -> P_893 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_893 == 1 ) {
if ( _rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_b3 == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_b3 < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_b3 ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_79_0_f ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_79_0_f ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_458_0 = _rtB
-> B_80_77_0_f [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_38_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_g . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_g .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_304 ; _rtB -> B_80_459_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_g . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_g . Last , _rtDW -> TransportDelay_IWORK_g . Tail ,
_rtDW -> TransportDelay_IWORK_g . Head , _rtP -> P_305 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_80_460_0 = 0.0 ; _rtB -> B_80_460_0 += _rtP -> P_307 * _rtX
-> TransferFcn_CSTATE_g ; _rtB -> B_80_460_0 += _rtP -> P_308 * _rtB ->
B_80_459_0 ; if ( _rtB -> B_80_460_0 > _rtP -> P_310 ) { _rtB -> B_80_461_0 =
_rtB -> B_80_460_0 - _rtP -> P_310 ; } else if ( _rtB -> B_80_460_0 >= _rtP
-> P_309 ) { _rtB -> B_80_461_0 = 0.0 ; } else { _rtB -> B_80_461_0 = _rtB ->
B_80_460_0 - _rtP -> P_309 ; } _rtB -> B_80_462_0 = _rtP -> P_311 * _rtB ->
B_80_461_0 ; if ( _rtB -> B_80_462_0 > _rtP -> P_312 ) { _rtB -> B_80_463_0 =
_rtP -> P_312 ; } else if ( _rtB -> B_80_462_0 < _rtP -> P_313 ) { _rtB ->
B_80_463_0 = _rtP -> P_313 ; } else { _rtB -> B_80_463_0 = _rtB -> B_80_462_0
; } if ( ( _rtDW -> LastMajorTimeA_ou >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW
-> LastMajorTimeB_pr >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_80_464_0 =
_rtB -> B_80_463_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_ou < _rtDW ->
LastMajorTimeB_pr ) && ( _rtDW -> LastMajorTimeB_pr < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_ou >= _rtDW -> LastMajorTimeB_pr ) && (
_rtDW -> LastMajorTimeA_ou >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_pr ; rtb_B_80_3_0 = _rtDW
-> PrevYB_af ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_ou ; rtb_B_80_3_0 = _rtDW -> PrevYA_h ; } riseValLimit =
deltaT * _rtP -> P_314 ; rateLimiterRate = _rtB -> B_80_463_0 - rtb_B_80_3_0
; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_80_464_0 = rtb_B_80_3_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_315 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_80_464_0 = rtb_B_80_3_0 + deltaT ; } else { _rtB ->
B_80_464_0 = _rtB -> B_80_463_0 ; } } } _rtB -> B_80_465_0 = _rtP -> P_316 *
_rtB -> B_80_464_0 ; _rtB -> B_80_466_0 = _rtB -> B_80_458_0 * _rtB ->
B_80_465_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem3/Wagon model4/Data Store Read" ,
0 ) ; _rtB -> B_80_467_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_d != 0 ) {
_rtX -> vv0at_CSTATE_al = _rtB -> B_80_467_0 ; } _rtB -> B_80_468_0 = _rtX ->
vv0at_CSTATE_al ; _rtB -> B_80_469_0 = muDoubleScalarAbs ( _rtB -> B_80_468_0
) ; _rtB -> B_80_470_0 = _rtP -> P_317 [ 0 ] ; _rtB -> B_80_470_0 = _rtB ->
B_80_470_0 * _rtB -> B_80_469_0 + _rtP -> P_317 [ 1 ] ; _rtB -> B_80_470_0 =
_rtB -> B_80_470_0 * _rtB -> B_80_469_0 + _rtP -> P_317 [ 2 ] ; _rtB ->
B_80_471_0 = _rtP -> P_318 * _rtB -> B_80_470_0 ; _rtB -> B_80_472_0 = _rtB
-> B_80_456_0 * _rtB -> B_80_471_0 ; _rtB -> B_80_473_0 = _rtB -> B_80_466_0
- _rtB -> B_80_472_0 ; isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0
) { _rtB -> B_80_475_0 = _rtP -> P_319 * _rtB -> B_80_457_0 ; } if ( _rtB ->
B_80_473_0 > _rtB -> B_80_457_0 ) { _rtB -> B_80_477_0 = _rtB -> B_80_457_0 ;
} else { if ( _rtB -> B_80_473_0 < _rtB -> B_80_475_0 ) { _rtB -> B_39_1_0 =
_rtB -> B_80_475_0 ; } else { _rtB -> B_39_1_0 = _rtB -> B_80_473_0 ; } _rtB
-> B_80_477_0 = _rtB -> B_39_1_0 ; } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if
( isHit != 0 ) { ssCallAccelRunBlock ( S , 40 , 0 , SS_CALL_MDL_OUTPUTS ) ;
if ( ( _rtP -> P_894 != 0 ) && ( ! ( _rtB -> B_40_0_0 <= 499.0 ) ) ) { if (
_rtP -> P_894 == 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_i4 ==
0U ) { diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model/lookup wagon mass" )
; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_i4 < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_i4 ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model/lookup wagon mass" )
; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_82_0_o ) ; if ( ( _rtP -> P_894 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_894 == 1 ) {
if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_i4 == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model/lookup wagon mass" )
; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_i4 < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_i4 ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model/lookup wagon mass" )
; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_82_0_o ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_82_0_o ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_479_0 = _rtB
-> B_80_81_0_c [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_40_0_0 , 499U ) ] ; _rtB -> B_80_480_0 =
_rtB -> B_80_84_0_l * _rtB -> B_80_479_0 ; if ( ( _rtP -> P_895 != 0 ) && ( !
( _rtB -> B_40_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_895 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_k == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_k < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_k ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_83_0 ) ; if ( ( _rtP -> P_895 != 0 ) && ( ( ! ( rtb_B_80_3_0 >=
0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_895 == 1 ) { if
( _rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_k == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_k < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_k ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_83_0 ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_83_0 ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if (
rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_481_0 = _rtB ->
B_80_81_0_c [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_40_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_as . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_as .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_320 ; _rtB -> B_80_482_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_b . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_b . Last , _rtDW -> TransportDelay_IWORK_b . Tail ,
_rtDW -> TransportDelay_IWORK_b . Head , _rtP -> P_321 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_80_483_0 = 0.0 ; _rtB -> B_80_483_0 += _rtP -> P_323 * _rtX
-> TransferFcn_CSTATE_ku ; _rtB -> B_80_483_0 += _rtP -> P_324 * _rtB ->
B_80_482_0 ; if ( _rtB -> B_80_483_0 > _rtP -> P_326 ) { _rtB -> B_80_484_0 =
_rtB -> B_80_483_0 - _rtP -> P_326 ; } else if ( _rtB -> B_80_483_0 >= _rtP
-> P_325 ) { _rtB -> B_80_484_0 = 0.0 ; } else { _rtB -> B_80_484_0 = _rtB ->
B_80_483_0 - _rtP -> P_325 ; } _rtB -> B_80_485_0 = _rtP -> P_327 * _rtB ->
B_80_484_0 ; if ( _rtB -> B_80_485_0 > _rtP -> P_328 ) { _rtB -> B_80_486_0 =
_rtP -> P_328 ; } else if ( _rtB -> B_80_485_0 < _rtP -> P_329 ) { _rtB ->
B_80_486_0 = _rtP -> P_329 ; } else { _rtB -> B_80_486_0 = _rtB -> B_80_485_0
; } if ( ( _rtDW -> LastMajorTimeA_eh >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW
-> LastMajorTimeB_no >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_80_487_0 =
_rtB -> B_80_486_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_eh < _rtDW ->
LastMajorTimeB_no ) && ( _rtDW -> LastMajorTimeB_no < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_eh >= _rtDW -> LastMajorTimeB_no ) && (
_rtDW -> LastMajorTimeA_eh >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_no ; rtb_B_80_3_0 = _rtDW
-> PrevYB_e ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_eh ; rtb_B_80_3_0 = _rtDW -> PrevYA_f ; } riseValLimit =
deltaT * _rtP -> P_330 ; rateLimiterRate = _rtB -> B_80_486_0 - rtb_B_80_3_0
; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_80_487_0 = rtb_B_80_3_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_331 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_80_487_0 = rtb_B_80_3_0 + deltaT ; } else { _rtB ->
B_80_487_0 = _rtB -> B_80_486_0 ; } } } _rtB -> B_80_488_0 = _rtP -> P_332 *
_rtB -> B_80_487_0 ; _rtB -> B_80_489_0 = _rtB -> B_80_481_0 * _rtB ->
B_80_488_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model/Data Store Read" , 0
) ; _rtB -> B_80_490_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_fw != 0 ) {
_rtX -> vv0at_CSTATE_j = _rtB -> B_80_490_0 ; } _rtB -> B_80_491_0 = _rtX ->
vv0at_CSTATE_j ; _rtB -> B_80_492_0 = muDoubleScalarAbs ( _rtB -> B_80_491_0
) ; _rtB -> B_80_493_0 = _rtP -> P_333 [ 0 ] ; _rtB -> B_80_493_0 = _rtB ->
B_80_493_0 * _rtB -> B_80_492_0 + _rtP -> P_333 [ 1 ] ; _rtB -> B_80_493_0 =
_rtB -> B_80_493_0 * _rtB -> B_80_492_0 + _rtP -> P_333 [ 2 ] ; _rtB ->
B_80_494_0 = _rtP -> P_334 * _rtB -> B_80_493_0 ; _rtB -> B_80_495_0 = _rtB
-> B_80_479_0 * _rtB -> B_80_494_0 ; _rtB -> B_80_496_0 = _rtB -> B_80_489_0
- _rtB -> B_80_495_0 ; isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0
) { _rtB -> B_80_498_0 = _rtP -> P_335 * _rtB -> B_80_480_0 ; } if ( _rtB ->
B_80_496_0 > _rtB -> B_80_480_0 ) { _rtB -> B_80_500_0 = _rtB -> B_80_480_0 ;
} else { if ( _rtB -> B_80_496_0 < _rtB -> B_80_498_0 ) { _rtB -> B_41_1_0 =
_rtB -> B_80_498_0 ; } else { _rtB -> B_41_1_0 = _rtB -> B_80_496_0 ; } _rtB
-> B_80_500_0 = _rtB -> B_41_1_0 ; } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if
( isHit != 0 ) { ssCallAccelRunBlock ( S , 42 , 0 , SS_CALL_MDL_OUTPUTS ) ;
if ( ( _rtP -> P_896 != 0 ) && ( ! ( _rtB -> B_42_0_0 <= 499.0 ) ) ) { if (
_rtP -> P_896 == 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_l ==
0U ) { diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model1/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_l < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_l ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model1/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_86_0_m ) ; if ( ( _rtP -> P_896 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_896 == 1 ) {
if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_l == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model1/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_l < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_l ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model1/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_86_0_m ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_86_0_m ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_502_0 = _rtB
-> B_80_85_0 [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_42_0_0 , 499U ) ] ; _rtB -> B_80_503_0 =
_rtB -> B_80_88_0_m * _rtB -> B_80_502_0 ; if ( ( _rtP -> P_897 != 0 ) && ( !
( _rtB -> B_42_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_897 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_k5 == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_k5 < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_k5 ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_87_0 ) ; if ( ( _rtP -> P_897 != 0 ) && ( ( ! ( rtb_B_80_3_0 >=
0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_897 == 1 ) { if
( _rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_k5 == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_k5 < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_k5 ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_87_0 ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_87_0 ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if (
rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_504_0 = _rtB ->
B_80_85_0 [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_42_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_j . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_j .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_336 ; _rtB -> B_80_505_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_m . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_m . Last , _rtDW -> TransportDelay_IWORK_m . Tail ,
_rtDW -> TransportDelay_IWORK_m . Head , _rtP -> P_337 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_80_506_0 = 0.0 ; _rtB -> B_80_506_0 += _rtP -> P_339 * _rtX
-> TransferFcn_CSTATE_o ; _rtB -> B_80_506_0 += _rtP -> P_340 * _rtB ->
B_80_505_0 ; if ( _rtB -> B_80_506_0 > _rtP -> P_342 ) { _rtB -> B_80_507_0 =
_rtB -> B_80_506_0 - _rtP -> P_342 ; } else if ( _rtB -> B_80_506_0 >= _rtP
-> P_341 ) { _rtB -> B_80_507_0 = 0.0 ; } else { _rtB -> B_80_507_0 = _rtB ->
B_80_506_0 - _rtP -> P_341 ; } _rtB -> B_80_508_0 = _rtP -> P_343 * _rtB ->
B_80_507_0 ; if ( _rtB -> B_80_508_0 > _rtP -> P_344 ) { _rtB -> B_80_509_0 =
_rtP -> P_344 ; } else if ( _rtB -> B_80_508_0 < _rtP -> P_345 ) { _rtB ->
B_80_509_0 = _rtP -> P_345 ; } else { _rtB -> B_80_509_0 = _rtB -> B_80_508_0
; } if ( ( _rtDW -> LastMajorTimeA_ly >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW
-> LastMajorTimeB_m0 >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_80_510_0 =
_rtB -> B_80_509_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_ly < _rtDW ->
LastMajorTimeB_m0 ) && ( _rtDW -> LastMajorTimeB_m0 < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_ly >= _rtDW -> LastMajorTimeB_m0 ) && (
_rtDW -> LastMajorTimeA_ly >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_m0 ; rtb_B_80_3_0 = _rtDW
-> PrevYB_pd ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_ly ; rtb_B_80_3_0 = _rtDW -> PrevYA_e4 ; } riseValLimit =
deltaT * _rtP -> P_346 ; rateLimiterRate = _rtB -> B_80_509_0 - rtb_B_80_3_0
; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_80_510_0 = rtb_B_80_3_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_347 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_80_510_0 = rtb_B_80_3_0 + deltaT ; } else { _rtB ->
B_80_510_0 = _rtB -> B_80_509_0 ; } } } _rtB -> B_80_511_0 = _rtP -> P_348 *
_rtB -> B_80_510_0 ; _rtB -> B_80_512_0 = _rtB -> B_80_504_0 * _rtB ->
B_80_511_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model1/Data Store Read" ,
0 ) ; _rtB -> B_80_513_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_fz != 0 )
{ _rtX -> vv0at_CSTATE_gl = _rtB -> B_80_513_0 ; } _rtB -> B_80_514_0 = _rtX
-> vv0at_CSTATE_gl ; _rtB -> B_80_515_0 = muDoubleScalarAbs ( _rtB ->
B_80_514_0 ) ; _rtB -> B_80_516_0 = _rtP -> P_349 [ 0 ] ; _rtB -> B_80_516_0
= _rtB -> B_80_516_0 * _rtB -> B_80_515_0 + _rtP -> P_349 [ 1 ] ; _rtB ->
B_80_516_0 = _rtB -> B_80_516_0 * _rtB -> B_80_515_0 + _rtP -> P_349 [ 2 ] ;
_rtB -> B_80_517_0 = _rtP -> P_350 * _rtB -> B_80_516_0 ; _rtB -> B_80_518_0
= _rtB -> B_80_502_0 * _rtB -> B_80_517_0 ; _rtB -> B_80_519_0 = _rtB ->
B_80_512_0 - _rtB -> B_80_518_0 ; isHit = ssIsSampleHit ( S , 2 , 0 ) ; if (
isHit != 0 ) { _rtB -> B_80_521_0 = _rtP -> P_351 * _rtB -> B_80_503_0 ; } if
( _rtB -> B_80_519_0 > _rtB -> B_80_503_0 ) { _rtB -> B_80_523_0 = _rtB ->
B_80_503_0 ; } else { if ( _rtB -> B_80_519_0 < _rtB -> B_80_521_0 ) { _rtB
-> B_43_1_0 = _rtB -> B_80_521_0 ; } else { _rtB -> B_43_1_0 = _rtB ->
B_80_519_0 ; } _rtB -> B_80_523_0 = _rtB -> B_43_1_0 ; } isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { ssCallAccelRunBlock ( S ,
44 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP -> P_898 != 0 ) && ( ! ( _rtB ->
B_44_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_898 == 1 ) { if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_ho == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model2/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_ho < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_ho ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model2/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_90_0_f ) ; if ( ( _rtP -> P_898 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_898 == 1 ) {
if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_ho == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model2/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_ho < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_ho ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model2/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_90_0_f ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_90_0_f ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_525_0 = _rtB
-> B_80_89_0_c [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_44_0_0 , 499U ) ] ; _rtB -> B_80_526_0 =
_rtB -> B_80_92_0_e * _rtB -> B_80_525_0 ; if ( ( _rtP -> P_899 != 0 ) && ( !
( _rtB -> B_44_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_899 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_eh == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_eh < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_eh ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_91_0_p ) ; if ( ( _rtP -> P_899 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_899 == 1 ) {
if ( _rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_eh == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_eh < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_eh ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_91_0_p ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_91_0_p ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_527_0 = _rtB
-> B_80_89_0_c [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_44_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_jw . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_jw .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_352 ; _rtB -> B_80_528_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_ln . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_ln . Last , _rtDW -> TransportDelay_IWORK_ln . Tail ,
_rtDW -> TransportDelay_IWORK_ln . Head , _rtP -> P_353 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_80_529_0 = 0.0 ; _rtB -> B_80_529_0 += _rtP -> P_355 * _rtX
-> TransferFcn_CSTATE_ct ; _rtB -> B_80_529_0 += _rtP -> P_356 * _rtB ->
B_80_528_0 ; if ( _rtB -> B_80_529_0 > _rtP -> P_358 ) { _rtB -> B_80_530_0 =
_rtB -> B_80_529_0 - _rtP -> P_358 ; } else if ( _rtB -> B_80_529_0 >= _rtP
-> P_357 ) { _rtB -> B_80_530_0 = 0.0 ; } else { _rtB -> B_80_530_0 = _rtB ->
B_80_529_0 - _rtP -> P_357 ; } _rtB -> B_80_531_0 = _rtP -> P_359 * _rtB ->
B_80_530_0 ; if ( _rtB -> B_80_531_0 > _rtP -> P_360 ) { _rtB -> B_80_532_0 =
_rtP -> P_360 ; } else if ( _rtB -> B_80_531_0 < _rtP -> P_361 ) { _rtB ->
B_80_532_0 = _rtP -> P_361 ; } else { _rtB -> B_80_532_0 = _rtB -> B_80_531_0
; } if ( ( _rtDW -> LastMajorTimeA_k >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW
-> LastMajorTimeB_lc >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_80_533_0 =
_rtB -> B_80_532_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_k < _rtDW ->
LastMajorTimeB_lc ) && ( _rtDW -> LastMajorTimeB_lc < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_k >= _rtDW -> LastMajorTimeB_lc ) && (
_rtDW -> LastMajorTimeA_k >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_lc ; rtb_B_80_3_0 = _rtDW
-> PrevYB_et ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_k ; rtb_B_80_3_0 = _rtDW -> PrevYA_oo ; } riseValLimit =
deltaT * _rtP -> P_362 ; rateLimiterRate = _rtB -> B_80_532_0 - rtb_B_80_3_0
; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_80_533_0 = rtb_B_80_3_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_363 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_80_533_0 = rtb_B_80_3_0 + deltaT ; } else { _rtB ->
B_80_533_0 = _rtB -> B_80_532_0 ; } } } _rtB -> B_80_534_0 = _rtP -> P_364 *
_rtB -> B_80_533_0 ; _rtB -> B_80_535_0 = _rtB -> B_80_527_0 * _rtB ->
B_80_534_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model2/Data Store Read" ,
0 ) ; _rtB -> B_80_536_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_ek != 0 )
{ _rtX -> vv0at_CSTATE_fc = _rtB -> B_80_536_0 ; } _rtB -> B_80_537_0 = _rtX
-> vv0at_CSTATE_fc ; _rtB -> B_80_538_0 = muDoubleScalarAbs ( _rtB ->
B_80_537_0 ) ; _rtB -> B_80_539_0 = _rtP -> P_365 [ 0 ] ; _rtB -> B_80_539_0
= _rtB -> B_80_539_0 * _rtB -> B_80_538_0 + _rtP -> P_365 [ 1 ] ; _rtB ->
B_80_539_0 = _rtB -> B_80_539_0 * _rtB -> B_80_538_0 + _rtP -> P_365 [ 2 ] ;
_rtB -> B_80_540_0 = _rtP -> P_366 * _rtB -> B_80_539_0 ; _rtB -> B_80_541_0
= _rtB -> B_80_525_0 * _rtB -> B_80_540_0 ; _rtB -> B_80_542_0 = _rtB ->
B_80_535_0 - _rtB -> B_80_541_0 ; isHit = ssIsSampleHit ( S , 2 , 0 ) ; if (
isHit != 0 ) { _rtB -> B_80_544_0 = _rtP -> P_367 * _rtB -> B_80_526_0 ; } if
( _rtB -> B_80_542_0 > _rtB -> B_80_526_0 ) { _rtB -> B_80_546_0 = _rtB ->
B_80_526_0 ; } else { if ( _rtB -> B_80_542_0 < _rtB -> B_80_544_0 ) { _rtB
-> B_45_1_0 = _rtB -> B_80_544_0 ; } else { _rtB -> B_45_1_0 = _rtB ->
B_80_542_0 ; } _rtB -> B_80_546_0 = _rtB -> B_45_1_0 ; } isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { ssCallAccelRunBlock ( S ,
46 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP -> P_900 != 0 ) && ( ! ( _rtB ->
B_46_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_900 == 1 ) { if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_p == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model3/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_p < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_p ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model3/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_94_0_h ) ; if ( ( _rtP -> P_900 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_900 == 1 ) {
if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_p == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model3/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_p < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_p ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model3/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_94_0_h ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_94_0_h ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_548_0 = _rtB
-> B_80_93_0_o [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_46_0_0 , 499U ) ] ; _rtB -> B_80_549_0 =
_rtB -> B_80_96_0_h * _rtB -> B_80_548_0 ; if ( ( _rtP -> P_901 != 0 ) && ( !
( _rtB -> B_46_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_901 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_f4 == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_f4 < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_f4 ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_95_0_l ) ; if ( ( _rtP -> P_901 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_901 == 1 ) {
if ( _rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_f4 == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_f4 < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_f4 ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_95_0_l ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_95_0_l ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_550_0 = _rtB
-> B_80_93_0_o [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_46_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_ey . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_ey .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_368 ; _rtB -> B_80_551_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_m5 . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_m5 . Last , _rtDW -> TransportDelay_IWORK_m5 . Tail ,
_rtDW -> TransportDelay_IWORK_m5 . Head , _rtP -> P_369 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_80_552_0 = 0.0 ; _rtB -> B_80_552_0 += _rtP -> P_371 * _rtX
-> TransferFcn_CSTATE_hw ; _rtB -> B_80_552_0 += _rtP -> P_372 * _rtB ->
B_80_551_0 ; if ( _rtB -> B_80_552_0 > _rtP -> P_374 ) { _rtB -> B_80_553_0 =
_rtB -> B_80_552_0 - _rtP -> P_374 ; } else if ( _rtB -> B_80_552_0 >= _rtP
-> P_373 ) { _rtB -> B_80_553_0 = 0.0 ; } else { _rtB -> B_80_553_0 = _rtB ->
B_80_552_0 - _rtP -> P_373 ; } _rtB -> B_80_554_0 = _rtP -> P_375 * _rtB ->
B_80_553_0 ; if ( _rtB -> B_80_554_0 > _rtP -> P_376 ) { _rtB -> B_80_555_0 =
_rtP -> P_376 ; } else if ( _rtB -> B_80_554_0 < _rtP -> P_377 ) { _rtB ->
B_80_555_0 = _rtP -> P_377 ; } else { _rtB -> B_80_555_0 = _rtB -> B_80_554_0
; } if ( ( _rtDW -> LastMajorTimeA_i1 >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW
-> LastMajorTimeB_mi >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_80_556_0 =
_rtB -> B_80_555_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_i1 < _rtDW ->
LastMajorTimeB_mi ) && ( _rtDW -> LastMajorTimeB_mi < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_i1 >= _rtDW -> LastMajorTimeB_mi ) && (
_rtDW -> LastMajorTimeA_i1 >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_mi ; rtb_B_80_3_0 = _rtDW
-> PrevYB_ka ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_i1 ; rtb_B_80_3_0 = _rtDW -> PrevYA_c ; } riseValLimit =
deltaT * _rtP -> P_378 ; rateLimiterRate = _rtB -> B_80_555_0 - rtb_B_80_3_0
; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_80_556_0 = rtb_B_80_3_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_379 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_80_556_0 = rtb_B_80_3_0 + deltaT ; } else { _rtB ->
B_80_556_0 = _rtB -> B_80_555_0 ; } } } _rtB -> B_80_557_0 = _rtP -> P_380 *
_rtB -> B_80_556_0 ; _rtB -> B_80_558_0 = _rtB -> B_80_550_0 * _rtB ->
B_80_557_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model3/Data Store Read" ,
0 ) ; _rtB -> B_80_559_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_fm != 0 )
{ _rtX -> vv0at_CSTATE_ct = _rtB -> B_80_559_0 ; } _rtB -> B_80_560_0 = _rtX
-> vv0at_CSTATE_ct ; _rtB -> B_80_561_0 = muDoubleScalarAbs ( _rtB ->
B_80_560_0 ) ; _rtB -> B_80_562_0 = _rtP -> P_381 [ 0 ] ; _rtB -> B_80_562_0
= _rtB -> B_80_562_0 * _rtB -> B_80_561_0 + _rtP -> P_381 [ 1 ] ; _rtB ->
B_80_562_0 = _rtB -> B_80_562_0 * _rtB -> B_80_561_0 + _rtP -> P_381 [ 2 ] ;
_rtB -> B_80_563_0 = _rtP -> P_382 * _rtB -> B_80_562_0 ; _rtB -> B_80_564_0
= _rtB -> B_80_548_0 * _rtB -> B_80_563_0 ; _rtB -> B_80_565_0 = _rtB ->
B_80_558_0 - _rtB -> B_80_564_0 ; isHit = ssIsSampleHit ( S , 2 , 0 ) ; if (
isHit != 0 ) { _rtB -> B_80_567_0 = _rtP -> P_383 * _rtB -> B_80_549_0 ; } if
( _rtB -> B_80_565_0 > _rtB -> B_80_549_0 ) { _rtB -> B_80_569_0 = _rtB ->
B_80_549_0 ; } else { if ( _rtB -> B_80_565_0 < _rtB -> B_80_567_0 ) { _rtB
-> B_47_1_0 = _rtB -> B_80_567_0 ; } else { _rtB -> B_47_1_0 = _rtB ->
B_80_565_0 ; } _rtB -> B_80_569_0 = _rtB -> B_47_1_0 ; } isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { ssCallAccelRunBlock ( S ,
48 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP -> P_902 != 0 ) && ( ! ( _rtB ->
B_48_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_902 == 1 ) { if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_ef == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model4/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_ef < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_ef ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model4/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_98_0_m ) ; if ( ( _rtP -> P_902 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_902 == 1 ) {
if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_ef == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model4/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_ef < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_ef ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model4/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_98_0_m ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_98_0_m ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_571_0 = _rtB
-> B_80_97_0_m [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_48_0_0 , 499U ) ] ; _rtB -> B_80_572_0 =
_rtB -> B_80_100_0_c * _rtB -> B_80_571_0 ; if ( ( _rtP -> P_903 != 0 ) && (
! ( _rtB -> B_48_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_903 == 1 ) { if ( _rtDW
-> lookupwagonbrakingefficiency_NumIssuedWarnings_mw == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_mw < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_mw ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_99_0_h ) ; if ( ( _rtP -> P_903 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_903 == 1 ) {
if ( _rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_mw == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_mw < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_mw ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_99_0_h ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_99_0_h ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_573_0 = _rtB
-> B_80_97_0_m [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_48_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_ap . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_ap .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_384 ; _rtB -> B_80_574_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_n . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_n . Last , _rtDW -> TransportDelay_IWORK_n . Tail ,
_rtDW -> TransportDelay_IWORK_n . Head , _rtP -> P_385 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_80_575_0 = 0.0 ; _rtB -> B_80_575_0 += _rtP -> P_387 * _rtX
-> TransferFcn_CSTATE_ez3 ; _rtB -> B_80_575_0 += _rtP -> P_388 * _rtB ->
B_80_574_0 ; if ( _rtB -> B_80_575_0 > _rtP -> P_390 ) { _rtB -> B_80_576_0 =
_rtB -> B_80_575_0 - _rtP -> P_390 ; } else if ( _rtB -> B_80_575_0 >= _rtP
-> P_389 ) { _rtB -> B_80_576_0 = 0.0 ; } else { _rtB -> B_80_576_0 = _rtB ->
B_80_575_0 - _rtP -> P_389 ; } _rtB -> B_80_577_0 = _rtP -> P_391 * _rtB ->
B_80_576_0 ; if ( _rtB -> B_80_577_0 > _rtP -> P_392 ) { _rtB -> B_80_578_0 =
_rtP -> P_392 ; } else if ( _rtB -> B_80_577_0 < _rtP -> P_393 ) { _rtB ->
B_80_578_0 = _rtP -> P_393 ; } else { _rtB -> B_80_578_0 = _rtB -> B_80_577_0
; } if ( ( _rtDW -> LastMajorTimeA_kn >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW
-> LastMajorTimeB_an >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_80_579_0 =
_rtB -> B_80_578_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_kn < _rtDW ->
LastMajorTimeB_an ) && ( _rtDW -> LastMajorTimeB_an < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_kn >= _rtDW -> LastMajorTimeB_an ) && (
_rtDW -> LastMajorTimeA_kn >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_an ; rtb_B_80_3_0 = _rtDW
-> PrevYB_oe ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_kn ; rtb_B_80_3_0 = _rtDW -> PrevYA_lz ; } riseValLimit =
deltaT * _rtP -> P_394 ; rateLimiterRate = _rtB -> B_80_578_0 - rtb_B_80_3_0
; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_80_579_0 = rtb_B_80_3_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_395 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_80_579_0 = rtb_B_80_3_0 + deltaT ; } else { _rtB ->
B_80_579_0 = _rtB -> B_80_578_0 ; } } } _rtB -> B_80_580_0 = _rtP -> P_396 *
_rtB -> B_80_579_0 ; _rtB -> B_80_581_0 = _rtB -> B_80_573_0 * _rtB ->
B_80_580_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem4/Wagon model4/Data Store Read" ,
0 ) ; _rtB -> B_80_582_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_py != 0 )
{ _rtX -> vv0at_CSTATE_iz = _rtB -> B_80_582_0 ; } _rtB -> B_80_583_0 = _rtX
-> vv0at_CSTATE_iz ; _rtB -> B_80_584_0 = muDoubleScalarAbs ( _rtB ->
B_80_583_0 ) ; _rtB -> B_80_585_0 = _rtP -> P_397 [ 0 ] ; _rtB -> B_80_585_0
= _rtB -> B_80_585_0 * _rtB -> B_80_584_0 + _rtP -> P_397 [ 1 ] ; _rtB ->
B_80_585_0 = _rtB -> B_80_585_0 * _rtB -> B_80_584_0 + _rtP -> P_397 [ 2 ] ;
_rtB -> B_80_586_0 = _rtP -> P_398 * _rtB -> B_80_585_0 ; _rtB -> B_80_587_0
= _rtB -> B_80_571_0 * _rtB -> B_80_586_0 ; _rtB -> B_80_588_0 = _rtB ->
B_80_581_0 - _rtB -> B_80_587_0 ; isHit = ssIsSampleHit ( S , 2 , 0 ) ; if (
isHit != 0 ) { _rtB -> B_80_590_0 = _rtP -> P_399 * _rtB -> B_80_572_0 ; } if
( _rtB -> B_80_588_0 > _rtB -> B_80_572_0 ) { _rtB -> B_80_592_0 = _rtB ->
B_80_572_0 ; } else { if ( _rtB -> B_80_588_0 < _rtB -> B_80_590_0 ) { _rtB
-> B_49_1_0 = _rtB -> B_80_590_0 ; } else { _rtB -> B_49_1_0 = _rtB ->
B_80_588_0 ; } _rtB -> B_80_592_0 = _rtB -> B_49_1_0 ; } isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { ssCallAccelRunBlock ( S ,
50 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP -> P_904 != 0 ) && ( ! ( _rtB ->
B_50_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_904 == 1 ) { if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_m1 == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model/lookup wagon mass" )
; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_m1 < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_m1 ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model/lookup wagon mass" )
; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_102_0_p ) ; if ( ( _rtP -> P_904 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_904 == 1 ) {
if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_m1 == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model/lookup wagon mass" )
; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_m1 < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_m1 ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model/lookup wagon mass" )
; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_102_0_p ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_102_0_p ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_594_0 = _rtB
-> B_80_101_0_k [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_50_0_0 , 499U ) ] ; _rtB -> B_80_595_0 =
_rtB -> B_80_104_0_p * _rtB -> B_80_594_0 ; if ( ( _rtP -> P_905 != 0 ) && (
! ( _rtB -> B_50_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_905 == 1 ) { if ( _rtDW
-> lookupwagonbrakingefficiency_NumIssuedWarnings_kv == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_kv < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_kv ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_103_0_p ) ; if ( ( _rtP -> P_905 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_905 == 1 ) {
if ( _rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_kv == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_kv < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_kv ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_103_0_p ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_103_0_p ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_596_0 = _rtB
-> B_80_101_0_k [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_50_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_i . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_i .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_400 ; _rtB -> B_80_597_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_k . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_k . Last , _rtDW -> TransportDelay_IWORK_k . Tail ,
_rtDW -> TransportDelay_IWORK_k . Head , _rtP -> P_401 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_80_598_0 = 0.0 ; _rtB -> B_80_598_0 += _rtP -> P_403 * _rtX
-> TransferFcn_CSTATE_i ; _rtB -> B_80_598_0 += _rtP -> P_404 * _rtB ->
B_80_597_0 ; if ( _rtB -> B_80_598_0 > _rtP -> P_406 ) { _rtB -> B_80_599_0 =
_rtB -> B_80_598_0 - _rtP -> P_406 ; } else if ( _rtB -> B_80_598_0 >= _rtP
-> P_405 ) { _rtB -> B_80_599_0 = 0.0 ; } else { _rtB -> B_80_599_0 = _rtB ->
B_80_598_0 - _rtP -> P_405 ; } _rtB -> B_80_600_0 = _rtP -> P_407 * _rtB ->
B_80_599_0 ; if ( _rtB -> B_80_600_0 > _rtP -> P_408 ) { _rtB -> B_80_601_0 =
_rtP -> P_408 ; } else if ( _rtB -> B_80_600_0 < _rtP -> P_409 ) { _rtB ->
B_80_601_0 = _rtP -> P_409 ; } else { _rtB -> B_80_601_0 = _rtB -> B_80_600_0
; } if ( ( _rtDW -> LastMajorTimeA_bx >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW
-> LastMajorTimeB_b >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_80_602_0 =
_rtB -> B_80_601_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_bx < _rtDW ->
LastMajorTimeB_b ) && ( _rtDW -> LastMajorTimeB_b < ssGetTaskTime ( S , 0 ) )
) || ( ( _rtDW -> LastMajorTimeA_bx >= _rtDW -> LastMajorTimeB_b ) && ( _rtDW
-> LastMajorTimeA_bx >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_b ; rtb_B_80_3_0 = _rtDW ->
PrevYB_gs ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_bx ; rtb_B_80_3_0 = _rtDW -> PrevYA_cn ; } riseValLimit =
deltaT * _rtP -> P_410 ; rateLimiterRate = _rtB -> B_80_601_0 - rtb_B_80_3_0
; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_80_602_0 = rtb_B_80_3_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_411 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_80_602_0 = rtb_B_80_3_0 + deltaT ; } else { _rtB ->
B_80_602_0 = _rtB -> B_80_601_0 ; } } } _rtB -> B_80_603_0 = _rtP -> P_412 *
_rtB -> B_80_602_0 ; _rtB -> B_80_604_0 = _rtB -> B_80_596_0 * _rtB ->
B_80_603_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model/Data Store Read" , 0
) ; _rtB -> B_80_605_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_kw != 0 ) {
_rtX -> vv0at_CSTATE_n = _rtB -> B_80_605_0 ; } _rtB -> B_80_606_0 = _rtX ->
vv0at_CSTATE_n ; _rtB -> B_80_607_0 = muDoubleScalarAbs ( _rtB -> B_80_606_0
) ; _rtB -> B_80_608_0 = _rtP -> P_413 [ 0 ] ; _rtB -> B_80_608_0 = _rtB ->
B_80_608_0 * _rtB -> B_80_607_0 + _rtP -> P_413 [ 1 ] ; _rtB -> B_80_608_0 =
_rtB -> B_80_608_0 * _rtB -> B_80_607_0 + _rtP -> P_413 [ 2 ] ; _rtB ->
B_80_609_0 = _rtP -> P_414 * _rtB -> B_80_608_0 ; _rtB -> B_80_610_0 = _rtB
-> B_80_594_0 * _rtB -> B_80_609_0 ; _rtB -> B_80_611_0 = _rtB -> B_80_604_0
- _rtB -> B_80_610_0 ; isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0
) { _rtB -> B_80_613_0 = _rtP -> P_415 * _rtB -> B_80_595_0 ; } if ( _rtB ->
B_80_611_0 > _rtB -> B_80_595_0 ) { _rtB -> B_80_615_0 = _rtB -> B_80_595_0 ;
} else { if ( _rtB -> B_80_611_0 < _rtB -> B_80_613_0 ) { _rtB -> B_51_1_0 =
_rtB -> B_80_613_0 ; } else { _rtB -> B_51_1_0 = _rtB -> B_80_611_0 ; } _rtB
-> B_80_615_0 = _rtB -> B_51_1_0 ; } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if
( isHit != 0 ) { ssCallAccelRunBlock ( S , 52 , 0 , SS_CALL_MDL_OUTPUTS ) ;
if ( ( _rtP -> P_906 != 0 ) && ( ! ( _rtB -> B_52_0_0 <= 499.0 ) ) ) { if (
_rtP -> P_906 == 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_oc ==
0U ) { diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model1/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_oc < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_oc ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model1/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_106_0 ) ; if ( ( _rtP -> P_906 != 0 ) && ( ( ! ( rtb_B_80_3_0 >=
0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_906 == 1 ) { if
( _rtDW -> lookupwagonmass_NumIssuedWarnings_oc == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model1/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_oc < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_oc ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model1/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_106_0 ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_106_0 ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if (
rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_617_0 = _rtB ->
B_80_105_0_a [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_52_0_0 , 499U ) ] ; _rtB -> B_80_618_0 =
_rtB -> B_80_108_0 * _rtB -> B_80_617_0 ; if ( ( _rtP -> P_907 != 0 ) && ( !
( _rtB -> B_52_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_907 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_bk == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_bk < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_bk ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_107_0_j ) ; if ( ( _rtP -> P_907 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_907 == 1 ) {
if ( _rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_bk == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_bk < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_bk ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_107_0_j ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_107_0_j ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_619_0 = _rtB
-> B_80_105_0_a [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_52_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_ls . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_ls .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_416 ; _rtB -> B_80_620_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_j2 . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_j2 . Last , _rtDW -> TransportDelay_IWORK_j2 . Tail ,
_rtDW -> TransportDelay_IWORK_j2 . Head , _rtP -> P_417 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_80_621_0 = 0.0 ; _rtB -> B_80_621_0 += _rtP -> P_419 * _rtX
-> TransferFcn_CSTATE_p ; _rtB -> B_80_621_0 += _rtP -> P_420 * _rtB ->
B_80_620_0 ; if ( _rtB -> B_80_621_0 > _rtP -> P_422 ) { _rtB -> B_80_622_0 =
_rtB -> B_80_621_0 - _rtP -> P_422 ; } else if ( _rtB -> B_80_621_0 >= _rtP
-> P_421 ) { _rtB -> B_80_622_0 = 0.0 ; } else { _rtB -> B_80_622_0 = _rtB ->
B_80_621_0 - _rtP -> P_421 ; } _rtB -> B_80_623_0 = _rtP -> P_423 * _rtB ->
B_80_622_0 ; if ( _rtB -> B_80_623_0 > _rtP -> P_424 ) { _rtB -> B_80_624_0 =
_rtP -> P_424 ; } else if ( _rtB -> B_80_623_0 < _rtP -> P_425 ) { _rtB ->
B_80_624_0 = _rtP -> P_425 ; } else { _rtB -> B_80_624_0 = _rtB -> B_80_623_0
; } if ( ( _rtDW -> LastMajorTimeA_bb >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW
-> LastMajorTimeB_h >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_80_625_0 =
_rtB -> B_80_624_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_bb < _rtDW ->
LastMajorTimeB_h ) && ( _rtDW -> LastMajorTimeB_h < ssGetTaskTime ( S , 0 ) )
) || ( ( _rtDW -> LastMajorTimeA_bb >= _rtDW -> LastMajorTimeB_h ) && ( _rtDW
-> LastMajorTimeA_bb >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_h ; rtb_B_80_3_0 = _rtDW ->
PrevYB_k4 ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_bb ; rtb_B_80_3_0 = _rtDW -> PrevYA_js ; } riseValLimit =
deltaT * _rtP -> P_426 ; rateLimiterRate = _rtB -> B_80_624_0 - rtb_B_80_3_0
; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_80_625_0 = rtb_B_80_3_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_427 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_80_625_0 = rtb_B_80_3_0 + deltaT ; } else { _rtB ->
B_80_625_0 = _rtB -> B_80_624_0 ; } } } _rtB -> B_80_626_0 = _rtP -> P_428 *
_rtB -> B_80_625_0 ; _rtB -> B_80_627_0 = _rtB -> B_80_619_0 * _rtB ->
B_80_626_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model1/Data Store Read" ,
0 ) ; _rtB -> B_80_628_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_dh != 0 )
{ _rtX -> vv0at_CSTATE_fe = _rtB -> B_80_628_0 ; } _rtB -> B_80_629_0 = _rtX
-> vv0at_CSTATE_fe ; _rtB -> B_80_630_0 = muDoubleScalarAbs ( _rtB ->
B_80_629_0 ) ; _rtB -> B_80_631_0 = _rtP -> P_429 [ 0 ] ; _rtB -> B_80_631_0
= _rtB -> B_80_631_0 * _rtB -> B_80_630_0 + _rtP -> P_429 [ 1 ] ; _rtB ->
B_80_631_0 = _rtB -> B_80_631_0 * _rtB -> B_80_630_0 + _rtP -> P_429 [ 2 ] ;
_rtB -> B_80_632_0 = _rtP -> P_430 * _rtB -> B_80_631_0 ; _rtB -> B_80_633_0
= _rtB -> B_80_617_0 * _rtB -> B_80_632_0 ; _rtB -> B_80_634_0 = _rtB ->
B_80_627_0 - _rtB -> B_80_633_0 ; isHit = ssIsSampleHit ( S , 2 , 0 ) ; if (
isHit != 0 ) { _rtB -> B_80_636_0 = _rtP -> P_431 * _rtB -> B_80_618_0 ; } if
( _rtB -> B_80_634_0 > _rtB -> B_80_618_0 ) { _rtB -> B_80_638_0 = _rtB ->
B_80_618_0 ; } else { if ( _rtB -> B_80_634_0 < _rtB -> B_80_636_0 ) { _rtB
-> B_53_1_0 = _rtB -> B_80_636_0 ; } else { _rtB -> B_53_1_0 = _rtB ->
B_80_634_0 ; } _rtB -> B_80_638_0 = _rtB -> B_53_1_0 ; } isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { ssCallAccelRunBlock ( S ,
54 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP -> P_908 != 0 ) && ( ! ( _rtB ->
B_54_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_908 == 1 ) { if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_a2 == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model2/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_a2 < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_a2 ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model2/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_110_0 ) ; if ( ( _rtP -> P_908 != 0 ) && ( ( ! ( rtb_B_80_3_0 >=
0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_908 == 1 ) { if
( _rtDW -> lookupwagonmass_NumIssuedWarnings_a2 == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model2/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_a2 < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_a2 ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model2/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_110_0 ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_110_0 ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if (
rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_640_0 = _rtB ->
B_80_109_0_e [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_54_0_0 , 499U ) ] ; _rtB -> B_80_641_0 =
_rtB -> B_80_112_0_b * _rtB -> B_80_640_0 ; if ( ( _rtP -> P_909 != 0 ) && (
! ( _rtB -> B_54_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_909 == 1 ) { if ( _rtDW
-> lookupwagonbrakingefficiency_NumIssuedWarnings_p35 == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_p35 < MAX_uint32_T ) { _rtDW
-> lookupwagonbrakingefficiency_NumIssuedWarnings_p35 ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_111_0_o ) ; if ( ( _rtP -> P_909 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_909 == 1 ) {
if ( _rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_p35 == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_p35 < MAX_uint32_T ) { _rtDW
-> lookupwagonbrakingefficiency_NumIssuedWarnings_p35 ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_111_0_o ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_111_0_o ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_642_0 = _rtB
-> B_80_109_0_e [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_54_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_p . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_p .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_432 ; _rtB -> B_80_643_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_nt . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_nt . Last , _rtDW -> TransportDelay_IWORK_nt . Tail ,
_rtDW -> TransportDelay_IWORK_nt . Head , _rtP -> P_433 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_80_644_0 = 0.0 ; _rtB -> B_80_644_0 += _rtP -> P_435 * _rtX
-> TransferFcn_CSTATE_bi ; _rtB -> B_80_644_0 += _rtP -> P_436 * _rtB ->
B_80_643_0 ; if ( _rtB -> B_80_644_0 > _rtP -> P_438 ) { _rtB -> B_80_645_0 =
_rtB -> B_80_644_0 - _rtP -> P_438 ; } else if ( _rtB -> B_80_644_0 >= _rtP
-> P_437 ) { _rtB -> B_80_645_0 = 0.0 ; } else { _rtB -> B_80_645_0 = _rtB ->
B_80_644_0 - _rtP -> P_437 ; } _rtB -> B_80_646_0 = _rtP -> P_439 * _rtB ->
B_80_645_0 ; if ( _rtB -> B_80_646_0 > _rtP -> P_440 ) { _rtB -> B_80_647_0 =
_rtP -> P_440 ; } else if ( _rtB -> B_80_646_0 < _rtP -> P_441 ) { _rtB ->
B_80_647_0 = _rtP -> P_441 ; } else { _rtB -> B_80_647_0 = _rtB -> B_80_646_0
; } if ( ( _rtDW -> LastMajorTimeA_jb >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW
-> LastMajorTimeB_fe >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_80_648_0 =
_rtB -> B_80_647_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_jb < _rtDW ->
LastMajorTimeB_fe ) && ( _rtDW -> LastMajorTimeB_fe < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_jb >= _rtDW -> LastMajorTimeB_fe ) && (
_rtDW -> LastMajorTimeA_jb >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_fe ; rtb_B_80_3_0 = _rtDW
-> PrevYB_oo ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_jb ; rtb_B_80_3_0 = _rtDW -> PrevYA_lt ; } riseValLimit =
deltaT * _rtP -> P_442 ; rateLimiterRate = _rtB -> B_80_647_0 - rtb_B_80_3_0
; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_80_648_0 = rtb_B_80_3_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_443 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_80_648_0 = rtb_B_80_3_0 + deltaT ; } else { _rtB ->
B_80_648_0 = _rtB -> B_80_647_0 ; } } } _rtB -> B_80_649_0 = _rtP -> P_444 *
_rtB -> B_80_648_0 ; _rtB -> B_80_650_0 = _rtB -> B_80_642_0 * _rtB ->
B_80_649_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model2/Data Store Read" ,
0 ) ; _rtB -> B_80_651_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_fa != 0 )
{ _rtX -> vv0at_CSTATE_ge = _rtB -> B_80_651_0 ; } _rtB -> B_80_652_0 = _rtX
-> vv0at_CSTATE_ge ; _rtB -> B_80_653_0 = muDoubleScalarAbs ( _rtB ->
B_80_652_0 ) ; _rtB -> B_80_654_0 = _rtP -> P_445 [ 0 ] ; _rtB -> B_80_654_0
= _rtB -> B_80_654_0 * _rtB -> B_80_653_0 + _rtP -> P_445 [ 1 ] ; _rtB ->
B_80_654_0 = _rtB -> B_80_654_0 * _rtB -> B_80_653_0 + _rtP -> P_445 [ 2 ] ;
_rtB -> B_80_655_0 = _rtP -> P_446 * _rtB -> B_80_654_0 ; _rtB -> B_80_656_0
= _rtB -> B_80_640_0 * _rtB -> B_80_655_0 ; _rtB -> B_80_657_0 = _rtB ->
B_80_650_0 - _rtB -> B_80_656_0 ; isHit = ssIsSampleHit ( S , 2 , 0 ) ; if (
isHit != 0 ) { _rtB -> B_80_659_0 = _rtP -> P_447 * _rtB -> B_80_641_0 ; } if
( _rtB -> B_80_657_0 > _rtB -> B_80_641_0 ) { _rtB -> B_80_661_0 = _rtB ->
B_80_641_0 ; } else { if ( _rtB -> B_80_657_0 < _rtB -> B_80_659_0 ) { _rtB
-> B_55_1_0 = _rtB -> B_80_659_0 ; } else { _rtB -> B_55_1_0 = _rtB ->
B_80_657_0 ; } _rtB -> B_80_661_0 = _rtB -> B_55_1_0 ; } isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { ssCallAccelRunBlock ( S ,
56 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP -> P_910 != 0 ) && ( ! ( _rtB ->
B_56_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_910 == 1 ) { if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_o3 == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model3/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_o3 < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_o3 ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model3/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_114_0_g ) ; if ( ( _rtP -> P_910 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_910 == 1 ) {
if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_o3 == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model3/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_o3 < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_o3 ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model3/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_114_0_g ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_114_0_g ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_663_0 = _rtB
-> B_80_113_0_a [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_56_0_0 , 499U ) ] ; _rtB -> B_80_664_0 =
_rtB -> B_80_116_0_f * _rtB -> B_80_663_0 ; if ( ( _rtP -> P_911 != 0 ) && (
! ( _rtB -> B_56_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_911 == 1 ) { if ( _rtDW
-> lookupwagonbrakingefficiency_NumIssuedWarnings_hn == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_hn < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_hn ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_115_0_e ) ; if ( ( _rtP -> P_911 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_911 == 1 ) {
if ( _rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_hn == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_hn < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_hn ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_115_0_e ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_115_0_e ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_665_0 = _rtB
-> B_80_113_0_a [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_56_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_lm . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_lm .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_448 ; _rtB -> B_80_666_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_ls . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_ls . Last , _rtDW -> TransportDelay_IWORK_ls . Tail ,
_rtDW -> TransportDelay_IWORK_ls . Head , _rtP -> P_449 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_80_667_0 = 0.0 ; _rtB -> B_80_667_0 += _rtP -> P_451 * _rtX
-> TransferFcn_CSTATE_f ; _rtB -> B_80_667_0 += _rtP -> P_452 * _rtB ->
B_80_666_0 ; if ( _rtB -> B_80_667_0 > _rtP -> P_454 ) { _rtB -> B_80_668_0 =
_rtB -> B_80_667_0 - _rtP -> P_454 ; } else if ( _rtB -> B_80_667_0 >= _rtP
-> P_453 ) { _rtB -> B_80_668_0 = 0.0 ; } else { _rtB -> B_80_668_0 = _rtB ->
B_80_667_0 - _rtP -> P_453 ; } _rtB -> B_80_669_0 = _rtP -> P_455 * _rtB ->
B_80_668_0 ; if ( _rtB -> B_80_669_0 > _rtP -> P_456 ) { _rtB -> B_80_670_0 =
_rtP -> P_456 ; } else if ( _rtB -> B_80_669_0 < _rtP -> P_457 ) { _rtB ->
B_80_670_0 = _rtP -> P_457 ; } else { _rtB -> B_80_670_0 = _rtB -> B_80_669_0
; } if ( ( _rtDW -> LastMajorTimeA_ne >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW
-> LastMajorTimeB_a5 >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_80_671_0 =
_rtB -> B_80_670_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_ne < _rtDW ->
LastMajorTimeB_a5 ) && ( _rtDW -> LastMajorTimeB_a5 < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_ne >= _rtDW -> LastMajorTimeB_a5 ) && (
_rtDW -> LastMajorTimeA_ne >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_a5 ; rtb_B_80_3_0 = _rtDW
-> PrevYB_ls ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_ne ; rtb_B_80_3_0 = _rtDW -> PrevYA_p5 ; } riseValLimit =
deltaT * _rtP -> P_458 ; rateLimiterRate = _rtB -> B_80_670_0 - rtb_B_80_3_0
; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_80_671_0 = rtb_B_80_3_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_459 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_80_671_0 = rtb_B_80_3_0 + deltaT ; } else { _rtB ->
B_80_671_0 = _rtB -> B_80_670_0 ; } } } _rtB -> B_80_672_0 = _rtP -> P_460 *
_rtB -> B_80_671_0 ; _rtB -> B_80_673_0 = _rtB -> B_80_665_0 * _rtB ->
B_80_672_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model3/Data Store Read" ,
0 ) ; _rtB -> B_80_674_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_gf != 0 )
{ _rtX -> vv0at_CSTATE_ht = _rtB -> B_80_674_0 ; } _rtB -> B_80_675_0 = _rtX
-> vv0at_CSTATE_ht ; _rtB -> B_80_676_0 = muDoubleScalarAbs ( _rtB ->
B_80_675_0 ) ; _rtB -> B_80_677_0 = _rtP -> P_461 [ 0 ] ; _rtB -> B_80_677_0
= _rtB -> B_80_677_0 * _rtB -> B_80_676_0 + _rtP -> P_461 [ 1 ] ; _rtB ->
B_80_677_0 = _rtB -> B_80_677_0 * _rtB -> B_80_676_0 + _rtP -> P_461 [ 2 ] ;
_rtB -> B_80_678_0 = _rtP -> P_462 * _rtB -> B_80_677_0 ; _rtB -> B_80_679_0
= _rtB -> B_80_663_0 * _rtB -> B_80_678_0 ; _rtB -> B_80_680_0 = _rtB ->
B_80_673_0 - _rtB -> B_80_679_0 ; isHit = ssIsSampleHit ( S , 2 , 0 ) ; if (
isHit != 0 ) { _rtB -> B_80_682_0 = _rtP -> P_463 * _rtB -> B_80_664_0 ; } if
( _rtB -> B_80_680_0 > _rtB -> B_80_664_0 ) { _rtB -> B_80_684_0 = _rtB ->
B_80_664_0 ; } else { if ( _rtB -> B_80_680_0 < _rtB -> B_80_682_0 ) { _rtB
-> B_57_1_0 = _rtB -> B_80_682_0 ; } else { _rtB -> B_57_1_0 = _rtB ->
B_80_680_0 ; } _rtB -> B_80_684_0 = _rtB -> B_57_1_0 ; } isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { ssCallAccelRunBlock ( S ,
58 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP -> P_912 != 0 ) && ( ! ( _rtB ->
B_58_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_912 == 1 ) { if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_d4 == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model4/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_d4 < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_d4 ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model4/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_118_0_e ) ; if ( ( _rtP -> P_912 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_912 == 1 ) {
if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_d4 == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model4/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_d4 < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_d4 ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model4/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_118_0_e ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_118_0_e ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_686_0 = _rtB
-> B_80_117_0_h [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_58_0_0 , 499U ) ] ; _rtB -> B_80_687_0 =
_rtB -> B_80_120_0_a * _rtB -> B_80_686_0 ; if ( ( _rtP -> P_913 != 0 ) && (
! ( _rtB -> B_58_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_913 == 1 ) { if ( _rtDW
-> lookupwagonbrakingefficiency_NumIssuedWarnings_b3z == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_b3z < MAX_uint32_T ) { _rtDW
-> lookupwagonbrakingefficiency_NumIssuedWarnings_b3z ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_119_0_c ) ; if ( ( _rtP -> P_913 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_913 == 1 ) {
if ( _rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_b3z == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_b3z < MAX_uint32_T ) { _rtDW
-> lookupwagonbrakingefficiency_NumIssuedWarnings_b3z ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_119_0_c ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_119_0_c ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_688_0 = _rtB
-> B_80_117_0_h [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_58_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_h . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_h .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_464 ; _rtB -> B_80_689_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_e . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_e . Last , _rtDW -> TransportDelay_IWORK_e . Tail ,
_rtDW -> TransportDelay_IWORK_e . Head , _rtP -> P_465 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_80_690_0 = 0.0 ; _rtB -> B_80_690_0 += _rtP -> P_467 * _rtX
-> TransferFcn_CSTATE_ii ; _rtB -> B_80_690_0 += _rtP -> P_468 * _rtB ->
B_80_689_0 ; if ( _rtB -> B_80_690_0 > _rtP -> P_470 ) { _rtB -> B_80_691_0 =
_rtB -> B_80_690_0 - _rtP -> P_470 ; } else if ( _rtB -> B_80_690_0 >= _rtP
-> P_469 ) { _rtB -> B_80_691_0 = 0.0 ; } else { _rtB -> B_80_691_0 = _rtB ->
B_80_690_0 - _rtP -> P_469 ; } _rtB -> B_80_692_0 = _rtP -> P_471 * _rtB ->
B_80_691_0 ; if ( _rtB -> B_80_692_0 > _rtP -> P_472 ) { _rtB -> B_80_693_0 =
_rtP -> P_472 ; } else if ( _rtB -> B_80_692_0 < _rtP -> P_473 ) { _rtB ->
B_80_693_0 = _rtP -> P_473 ; } else { _rtB -> B_80_693_0 = _rtB -> B_80_692_0
; } if ( ( _rtDW -> LastMajorTimeA_h >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW
-> LastMajorTimeB_au >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_80_694_0 =
_rtB -> B_80_693_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_h < _rtDW ->
LastMajorTimeB_au ) && ( _rtDW -> LastMajorTimeB_au < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_h >= _rtDW -> LastMajorTimeB_au ) && (
_rtDW -> LastMajorTimeA_h >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_au ; rtb_B_80_3_0 = _rtDW
-> PrevYB_c ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_h ; rtb_B_80_3_0 = _rtDW -> PrevYA_mc ; } riseValLimit =
deltaT * _rtP -> P_474 ; rateLimiterRate = _rtB -> B_80_693_0 - rtb_B_80_3_0
; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_80_694_0 = rtb_B_80_3_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_475 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_80_694_0 = rtb_B_80_3_0 + deltaT ; } else { _rtB ->
B_80_694_0 = _rtB -> B_80_693_0 ; } } } _rtB -> B_80_695_0 = _rtP -> P_476 *
_rtB -> B_80_694_0 ; _rtB -> B_80_696_0 = _rtB -> B_80_688_0 * _rtB ->
B_80_695_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem5/Wagon model4/Data Store Read" ,
0 ) ; _rtB -> B_80_697_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_cc != 0 )
{ _rtX -> vv0at_CSTATE_el = _rtB -> B_80_697_0 ; } _rtB -> B_80_698_0 = _rtX
-> vv0at_CSTATE_el ; _rtB -> B_80_699_0 = muDoubleScalarAbs ( _rtB ->
B_80_698_0 ) ; _rtB -> B_80_700_0 = _rtP -> P_477 [ 0 ] ; _rtB -> B_80_700_0
= _rtB -> B_80_700_0 * _rtB -> B_80_699_0 + _rtP -> P_477 [ 1 ] ; _rtB ->
B_80_700_0 = _rtB -> B_80_700_0 * _rtB -> B_80_699_0 + _rtP -> P_477 [ 2 ] ;
_rtB -> B_80_701_0 = _rtP -> P_478 * _rtB -> B_80_700_0 ; _rtB -> B_80_702_0
= _rtB -> B_80_686_0 * _rtB -> B_80_701_0 ; _rtB -> B_80_703_0 = _rtB ->
B_80_696_0 - _rtB -> B_80_702_0 ; isHit = ssIsSampleHit ( S , 2 , 0 ) ; if (
isHit != 0 ) { _rtB -> B_80_705_0 = _rtP -> P_479 * _rtB -> B_80_687_0 ; } if
( _rtB -> B_80_703_0 > _rtB -> B_80_687_0 ) { _rtB -> B_80_707_0 = _rtB ->
B_80_687_0 ; } else { if ( _rtB -> B_80_703_0 < _rtB -> B_80_705_0 ) { _rtB
-> B_59_1_0 = _rtB -> B_80_705_0 ; } else { _rtB -> B_59_1_0 = _rtB ->
B_80_703_0 ; } _rtB -> B_80_707_0 = _rtB -> B_59_1_0 ; } isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { ssCallAccelRunBlock ( S ,
60 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP -> P_914 != 0 ) && ( ! ( _rtB ->
B_60_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_914 == 1 ) { if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_jb == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model/lookup wagon mass" )
; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_jb < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_jb ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model/lookup wagon mass" )
; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_122_0_a ) ; if ( ( _rtP -> P_914 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_914 == 1 ) {
if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_jb == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model/lookup wagon mass" )
; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_jb < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_jb ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model/lookup wagon mass" )
; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_122_0_a ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_122_0_a ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_709_0 = _rtB
-> B_80_121_0_d [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_60_0_0 , 499U ) ] ; _rtB -> B_80_710_0 =
_rtB -> B_80_124_0_m * _rtB -> B_80_709_0 ; if ( ( _rtP -> P_915 != 0 ) && (
! ( _rtB -> B_60_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_915 == 1 ) { if ( _rtDW
-> lookupwagonbrakingefficiency_NumIssuedWarnings_d == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_d < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_d ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_123_0_p ) ; if ( ( _rtP -> P_915 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_915 == 1 ) {
if ( _rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_d == 0U ) { diag
= CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_d < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_d ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_123_0_p ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_123_0_p ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_711_0 = _rtB
-> B_80_121_0_d [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_60_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_pp . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_pp .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_480 ; _rtB -> B_80_712_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_ex . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_ex . Last , _rtDW -> TransportDelay_IWORK_ex . Tail ,
_rtDW -> TransportDelay_IWORK_ex . Head , _rtP -> P_481 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_80_713_0 = 0.0 ; _rtB -> B_80_713_0 += _rtP -> P_483 * _rtX
-> TransferFcn_CSTATE_gd ; _rtB -> B_80_713_0 += _rtP -> P_484 * _rtB ->
B_80_712_0 ; if ( _rtB -> B_80_713_0 > _rtP -> P_486 ) { _rtB -> B_80_714_0 =
_rtB -> B_80_713_0 - _rtP -> P_486 ; } else if ( _rtB -> B_80_713_0 >= _rtP
-> P_485 ) { _rtB -> B_80_714_0 = 0.0 ; } else { _rtB -> B_80_714_0 = _rtB ->
B_80_713_0 - _rtP -> P_485 ; } _rtB -> B_80_715_0 = _rtP -> P_487 * _rtB ->
B_80_714_0 ; if ( _rtB -> B_80_715_0 > _rtP -> P_488 ) { _rtB -> B_80_716_0 =
_rtP -> P_488 ; } else if ( _rtB -> B_80_715_0 < _rtP -> P_489 ) { _rtB ->
B_80_716_0 = _rtP -> P_489 ; } else { _rtB -> B_80_716_0 = _rtB -> B_80_715_0
; } if ( ( _rtDW -> LastMajorTimeA_neb >= ssGetTaskTime ( S , 0 ) ) && (
_rtDW -> LastMajorTimeB_gg >= ssGetTaskTime ( S , 0 ) ) ) { _rtB ->
B_80_717_0 = _rtB -> B_80_716_0 ; } else { if ( ( ( _rtDW ->
LastMajorTimeA_neb < _rtDW -> LastMajorTimeB_gg ) && ( _rtDW ->
LastMajorTimeB_gg < ssGetTaskTime ( S , 0 ) ) ) || ( ( _rtDW ->
LastMajorTimeA_neb >= _rtDW -> LastMajorTimeB_gg ) && ( _rtDW ->
LastMajorTimeA_neb >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT = ssGetTaskTime
( S , 0 ) - _rtDW -> LastMajorTimeB_gg ; rtb_B_80_3_0 = _rtDW -> PrevYB_jb ;
} else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeA_neb ;
rtb_B_80_3_0 = _rtDW -> PrevYA_a2 ; } riseValLimit = deltaT * _rtP -> P_490 ;
rateLimiterRate = _rtB -> B_80_716_0 - rtb_B_80_3_0 ; if ( rateLimiterRate >
riseValLimit ) { _rtB -> B_80_717_0 = rtb_B_80_3_0 + riseValLimit ; } else {
deltaT *= _rtP -> P_491 ; if ( rateLimiterRate < deltaT ) { _rtB ->
B_80_717_0 = rtb_B_80_3_0 + deltaT ; } else { _rtB -> B_80_717_0 = _rtB ->
B_80_716_0 ; } } } _rtB -> B_80_718_0 = _rtP -> P_492 * _rtB -> B_80_717_0 ;
_rtB -> B_80_719_0 = _rtB -> B_80_711_0 * _rtB -> B_80_718_0 ;
vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model/Data Store Read" , 0
) ; _rtB -> B_80_720_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_jq != 0 ) {
_rtX -> vv0at_CSTATE_iy = _rtB -> B_80_720_0 ; } _rtB -> B_80_721_0 = _rtX ->
vv0at_CSTATE_iy ; _rtB -> B_80_722_0 = muDoubleScalarAbs ( _rtB -> B_80_721_0
) ; _rtB -> B_80_723_0 = _rtP -> P_493 [ 0 ] ; _rtB -> B_80_723_0 = _rtB ->
B_80_723_0 * _rtB -> B_80_722_0 + _rtP -> P_493 [ 1 ] ; _rtB -> B_80_723_0 =
_rtB -> B_80_723_0 * _rtB -> B_80_722_0 + _rtP -> P_493 [ 2 ] ; _rtB ->
B_80_724_0 = _rtP -> P_494 * _rtB -> B_80_723_0 ; _rtB -> B_80_725_0 = _rtB
-> B_80_709_0 * _rtB -> B_80_724_0 ; _rtB -> B_80_726_0 = _rtB -> B_80_719_0
- _rtB -> B_80_725_0 ; isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0
) { _rtB -> B_80_728_0 = _rtP -> P_495 * _rtB -> B_80_710_0 ; } if ( _rtB ->
B_80_726_0 > _rtB -> B_80_710_0 ) { _rtB -> B_80_730_0 = _rtB -> B_80_710_0 ;
} else { if ( _rtB -> B_80_726_0 < _rtB -> B_80_728_0 ) { _rtB -> B_61_1_0 =
_rtB -> B_80_728_0 ; } else { _rtB -> B_61_1_0 = _rtB -> B_80_726_0 ; } _rtB
-> B_80_730_0 = _rtB -> B_61_1_0 ; } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if
( isHit != 0 ) { ssCallAccelRunBlock ( S , 62 , 0 , SS_CALL_MDL_OUTPUTS ) ;
if ( ( _rtP -> P_916 != 0 ) && ( ! ( _rtB -> B_62_0_0 <= 499.0 ) ) ) { if (
_rtP -> P_916 == 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_nm ==
0U ) { diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model1/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_nm < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_nm ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model1/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_126_0_n ) ; if ( ( _rtP -> P_916 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_916 == 1 ) {
if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_nm == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model1/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_nm < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_nm ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model1/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_126_0_n ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_126_0_n ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_732_0 = _rtB
-> B_80_125_0_o [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_62_0_0 , 499U ) ] ; _rtB -> B_80_733_0 =
_rtB -> B_80_128_0_p * _rtB -> B_80_732_0 ; if ( ( _rtP -> P_917 != 0 ) && (
! ( _rtB -> B_62_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_917 == 1 ) { if ( _rtDW
-> lookupwagonbrakingefficiency_NumIssuedWarnings_a4 == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_a4 < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_a4 ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_127_0_l ) ; if ( ( _rtP -> P_917 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_917 == 1 ) {
if ( _rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_a4 == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_a4 < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_a4 ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_127_0_l ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_127_0_l ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_734_0 = _rtB
-> B_80_125_0_o [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_62_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_lf . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_lf .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_496 ; _rtB -> B_80_735_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_ir . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_ir . Last , _rtDW -> TransportDelay_IWORK_ir . Tail ,
_rtDW -> TransportDelay_IWORK_ir . Head , _rtP -> P_497 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_80_736_0 = 0.0 ; _rtB -> B_80_736_0 += _rtP -> P_499 * _rtX
-> TransferFcn_CSTATE_k5 ; _rtB -> B_80_736_0 += _rtP -> P_500 * _rtB ->
B_80_735_0 ; if ( _rtB -> B_80_736_0 > _rtP -> P_502 ) { _rtB -> B_80_737_0 =
_rtB -> B_80_736_0 - _rtP -> P_502 ; } else if ( _rtB -> B_80_736_0 >= _rtP
-> P_501 ) { _rtB -> B_80_737_0 = 0.0 ; } else { _rtB -> B_80_737_0 = _rtB ->
B_80_736_0 - _rtP -> P_501 ; } _rtB -> B_80_738_0 = _rtP -> P_503 * _rtB ->
B_80_737_0 ; if ( _rtB -> B_80_738_0 > _rtP -> P_504 ) { _rtB -> B_80_739_0 =
_rtP -> P_504 ; } else if ( _rtB -> B_80_738_0 < _rtP -> P_505 ) { _rtB ->
B_80_739_0 = _rtP -> P_505 ; } else { _rtB -> B_80_739_0 = _rtB -> B_80_738_0
; } if ( ( _rtDW -> LastMajorTimeA_kk >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW
-> LastMajorTimeB_bi >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_80_740_0 =
_rtB -> B_80_739_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_kk < _rtDW ->
LastMajorTimeB_bi ) && ( _rtDW -> LastMajorTimeB_bi < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_kk >= _rtDW -> LastMajorTimeB_bi ) && (
_rtDW -> LastMajorTimeA_kk >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_bi ; rtb_B_80_3_0 = _rtDW
-> PrevYB_lt ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_kk ; rtb_B_80_3_0 = _rtDW -> PrevYA_dy ; } riseValLimit =
deltaT * _rtP -> P_506 ; rateLimiterRate = _rtB -> B_80_739_0 - rtb_B_80_3_0
; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_80_740_0 = rtb_B_80_3_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_507 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_80_740_0 = rtb_B_80_3_0 + deltaT ; } else { _rtB ->
B_80_740_0 = _rtB -> B_80_739_0 ; } } } _rtB -> B_80_741_0 = _rtP -> P_508 *
_rtB -> B_80_740_0 ; _rtB -> B_80_742_0 = _rtB -> B_80_734_0 * _rtB ->
B_80_741_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model1/Data Store Read" ,
0 ) ; _rtB -> B_80_743_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_kz != 0 )
{ _rtX -> vv0at_CSTATE_cs = _rtB -> B_80_743_0 ; } _rtB -> B_80_744_0 = _rtX
-> vv0at_CSTATE_cs ; _rtB -> B_80_745_0 = muDoubleScalarAbs ( _rtB ->
B_80_744_0 ) ; _rtB -> B_80_746_0 = _rtP -> P_509 [ 0 ] ; _rtB -> B_80_746_0
= _rtB -> B_80_746_0 * _rtB -> B_80_745_0 + _rtP -> P_509 [ 1 ] ; _rtB ->
B_80_746_0 = _rtB -> B_80_746_0 * _rtB -> B_80_745_0 + _rtP -> P_509 [ 2 ] ;
_rtB -> B_80_747_0 = _rtP -> P_510 * _rtB -> B_80_746_0 ; _rtB -> B_80_748_0
= _rtB -> B_80_732_0 * _rtB -> B_80_747_0 ; _rtB -> B_80_749_0 = _rtB ->
B_80_742_0 - _rtB -> B_80_748_0 ; isHit = ssIsSampleHit ( S , 2 , 0 ) ; if (
isHit != 0 ) { _rtB -> B_80_751_0 = _rtP -> P_511 * _rtB -> B_80_733_0 ; } if
( _rtB -> B_80_749_0 > _rtB -> B_80_733_0 ) { _rtB -> B_80_753_0 = _rtB ->
B_80_733_0 ; } else { if ( _rtB -> B_80_749_0 < _rtB -> B_80_751_0 ) { _rtB
-> B_63_1_0 = _rtB -> B_80_751_0 ; } else { _rtB -> B_63_1_0 = _rtB ->
B_80_749_0 ; } _rtB -> B_80_753_0 = _rtB -> B_63_1_0 ; } isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { ssCallAccelRunBlock ( S ,
64 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP -> P_918 != 0 ) && ( ! ( _rtB ->
B_64_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_918 == 1 ) { if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_mb == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model2/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_mb < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_mb ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model2/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_130_0_p ) ; if ( ( _rtP -> P_918 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_918 == 1 ) {
if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_mb == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model2/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_mb < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_mb ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model2/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_130_0_p ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_130_0_p ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_755_0 = _rtB
-> B_80_129_0 [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_64_0_0 , 499U ) ] ; _rtB -> B_80_756_0 =
_rtB -> B_80_132_0_f * _rtB -> B_80_755_0 ; if ( ( _rtP -> P_919 != 0 ) && (
! ( _rtB -> B_64_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_919 == 1 ) { if ( _rtDW
-> lookupwagonbrakingefficiency_NumIssuedWarnings_m2 == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_m2 < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_m2 ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_131_0 ) ; if ( ( _rtP -> P_919 != 0 ) && ( ( ! ( rtb_B_80_3_0 >=
0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_919 == 1 ) { if
( _rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_m2 == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_m2 < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_m2 ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_131_0 ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_131_0 ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if (
rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_757_0 = _rtB ->
B_80_129_0 [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_64_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_o2 . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_o2 .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_512 ; _rtB -> B_80_758_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_m3 . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_m3 . Last , _rtDW -> TransportDelay_IWORK_m3 . Tail ,
_rtDW -> TransportDelay_IWORK_m3 . Head , _rtP -> P_513 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_80_759_0 = 0.0 ; _rtB -> B_80_759_0 += _rtP -> P_515 * _rtX
-> TransferFcn_CSTATE_ga ; _rtB -> B_80_759_0 += _rtP -> P_516 * _rtB ->
B_80_758_0 ; if ( _rtB -> B_80_759_0 > _rtP -> P_518 ) { _rtB -> B_80_760_0 =
_rtB -> B_80_759_0 - _rtP -> P_518 ; } else if ( _rtB -> B_80_759_0 >= _rtP
-> P_517 ) { _rtB -> B_80_760_0 = 0.0 ; } else { _rtB -> B_80_760_0 = _rtB ->
B_80_759_0 - _rtP -> P_517 ; } _rtB -> B_80_761_0 = _rtP -> P_519 * _rtB ->
B_80_760_0 ; if ( _rtB -> B_80_761_0 > _rtP -> P_520 ) { _rtB -> B_80_762_0 =
_rtP -> P_520 ; } else if ( _rtB -> B_80_761_0 < _rtP -> P_521 ) { _rtB ->
B_80_762_0 = _rtP -> P_521 ; } else { _rtB -> B_80_762_0 = _rtB -> B_80_761_0
; } if ( ( _rtDW -> LastMajorTimeA_f5 >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW
-> LastMajorTimeB_b3 >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_80_763_0 =
_rtB -> B_80_762_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_f5 < _rtDW ->
LastMajorTimeB_b3 ) && ( _rtDW -> LastMajorTimeB_b3 < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_f5 >= _rtDW -> LastMajorTimeB_b3 ) && (
_rtDW -> LastMajorTimeA_f5 >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_b3 ; rtb_B_80_3_0 = _rtDW
-> PrevYB_ak ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_f5 ; rtb_B_80_3_0 = _rtDW -> PrevYA_bl ; } riseValLimit =
deltaT * _rtP -> P_522 ; rateLimiterRate = _rtB -> B_80_762_0 - rtb_B_80_3_0
; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_80_763_0 = rtb_B_80_3_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_523 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_80_763_0 = rtb_B_80_3_0 + deltaT ; } else { _rtB ->
B_80_763_0 = _rtB -> B_80_762_0 ; } } } _rtB -> B_80_764_0 = _rtP -> P_524 *
_rtB -> B_80_763_0 ; _rtB -> B_80_765_0 = _rtB -> B_80_757_0 * _rtB ->
B_80_764_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model2/Data Store Read" ,
0 ) ; _rtB -> B_80_766_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_dk != 0 )
{ _rtX -> vv0at_CSTATE_bs = _rtB -> B_80_766_0 ; } _rtB -> B_80_767_0 = _rtX
-> vv0at_CSTATE_bs ; _rtB -> B_80_768_0 = muDoubleScalarAbs ( _rtB ->
B_80_767_0 ) ; _rtB -> B_80_769_0 = _rtP -> P_525 [ 0 ] ; _rtB -> B_80_769_0
= _rtB -> B_80_769_0 * _rtB -> B_80_768_0 + _rtP -> P_525 [ 1 ] ; _rtB ->
B_80_769_0 = _rtB -> B_80_769_0 * _rtB -> B_80_768_0 + _rtP -> P_525 [ 2 ] ;
_rtB -> B_80_770_0 = _rtP -> P_526 * _rtB -> B_80_769_0 ; _rtB -> B_80_771_0
= _rtB -> B_80_755_0 * _rtB -> B_80_770_0 ; _rtB -> B_80_772_0 = _rtB ->
B_80_765_0 - _rtB -> B_80_771_0 ; isHit = ssIsSampleHit ( S , 2 , 0 ) ; if (
isHit != 0 ) { _rtB -> B_80_774_0 = _rtP -> P_527 * _rtB -> B_80_756_0 ; } if
( _rtB -> B_80_772_0 > _rtB -> B_80_756_0 ) { _rtB -> B_80_776_0 = _rtB ->
B_80_756_0 ; } else { if ( _rtB -> B_80_772_0 < _rtB -> B_80_774_0 ) { _rtB
-> B_65_1_0 = _rtB -> B_80_774_0 ; } else { _rtB -> B_65_1_0 = _rtB ->
B_80_772_0 ; } _rtB -> B_80_776_0 = _rtB -> B_65_1_0 ; } isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { ssCallAccelRunBlock ( S ,
66 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP -> P_920 != 0 ) && ( ! ( _rtB ->
B_66_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_920 == 1 ) { if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_b == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model3/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_b < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_b ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model3/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_134_0_i ) ; if ( ( _rtP -> P_920 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_920 == 1 ) {
if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_b == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model3/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_b < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_b ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model3/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_134_0_i ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_134_0_i ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_778_0 = _rtB
-> B_80_133_0 [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_66_0_0 , 499U ) ] ; _rtB -> B_80_779_0 =
_rtB -> B_80_136_0_k * _rtB -> B_80_778_0 ; if ( ( _rtP -> P_921 != 0 ) && (
! ( _rtB -> B_66_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_921 == 1 ) { if ( _rtDW
-> lookupwagonbrakingefficiency_NumIssuedWarnings_fd == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_fd < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_fd ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_135_0_o ) ; if ( ( _rtP -> P_921 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_921 == 1 ) {
if ( _rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_fd == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_fd < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_fd ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_135_0_o ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_135_0_o ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_780_0 = _rtB
-> B_80_133_0 [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_66_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_pi . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_pi .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_528 ; _rtB -> B_80_781_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_o . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_o . Last , _rtDW -> TransportDelay_IWORK_o . Tail ,
_rtDW -> TransportDelay_IWORK_o . Head , _rtP -> P_529 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_80_782_0 = 0.0 ; _rtB -> B_80_782_0 += _rtP -> P_531 * _rtX
-> TransferFcn_CSTATE_jw ; _rtB -> B_80_782_0 += _rtP -> P_532 * _rtB ->
B_80_781_0 ; if ( _rtB -> B_80_782_0 > _rtP -> P_534 ) { _rtB -> B_80_783_0 =
_rtB -> B_80_782_0 - _rtP -> P_534 ; } else if ( _rtB -> B_80_782_0 >= _rtP
-> P_533 ) { _rtB -> B_80_783_0 = 0.0 ; } else { _rtB -> B_80_783_0 = _rtB ->
B_80_782_0 - _rtP -> P_533 ; } _rtB -> B_80_784_0 = _rtP -> P_535 * _rtB ->
B_80_783_0 ; if ( _rtB -> B_80_784_0 > _rtP -> P_536 ) { _rtB -> B_80_785_0 =
_rtP -> P_536 ; } else if ( _rtB -> B_80_784_0 < _rtP -> P_537 ) { _rtB ->
B_80_785_0 = _rtP -> P_537 ; } else { _rtB -> B_80_785_0 = _rtB -> B_80_784_0
; } if ( ( _rtDW -> LastMajorTimeA_n0 >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW
-> LastMajorTimeB_b2 >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_80_786_0 =
_rtB -> B_80_785_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_n0 < _rtDW ->
LastMajorTimeB_b2 ) && ( _rtDW -> LastMajorTimeB_b2 < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_n0 >= _rtDW -> LastMajorTimeB_b2 ) && (
_rtDW -> LastMajorTimeA_n0 >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_b2 ; rtb_B_80_3_0 = _rtDW
-> PrevYB_i ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_n0 ; rtb_B_80_3_0 = _rtDW -> PrevYA_an ; } riseValLimit =
deltaT * _rtP -> P_538 ; rateLimiterRate = _rtB -> B_80_785_0 - rtb_B_80_3_0
; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_80_786_0 = rtb_B_80_3_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_539 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_80_786_0 = rtb_B_80_3_0 + deltaT ; } else { _rtB ->
B_80_786_0 = _rtB -> B_80_785_0 ; } } } _rtB -> B_80_787_0 = _rtP -> P_540 *
_rtB -> B_80_786_0 ; _rtB -> B_80_788_0 = _rtB -> B_80_780_0 * _rtB ->
B_80_787_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model3/Data Store Read" ,
0 ) ; _rtB -> B_80_789_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_da != 0 )
{ _rtX -> vv0at_CSTATE_co = _rtB -> B_80_789_0 ; } _rtB -> B_80_790_0 = _rtX
-> vv0at_CSTATE_co ; _rtB -> B_80_791_0 = muDoubleScalarAbs ( _rtB ->
B_80_790_0 ) ; _rtB -> B_80_792_0 = _rtP -> P_541 [ 0 ] ; _rtB -> B_80_792_0
= _rtB -> B_80_792_0 * _rtB -> B_80_791_0 + _rtP -> P_541 [ 1 ] ; _rtB ->
B_80_792_0 = _rtB -> B_80_792_0 * _rtB -> B_80_791_0 + _rtP -> P_541 [ 2 ] ;
_rtB -> B_80_793_0 = _rtP -> P_542 * _rtB -> B_80_792_0 ; _rtB -> B_80_794_0
= _rtB -> B_80_778_0 * _rtB -> B_80_793_0 ; _rtB -> B_80_795_0 = _rtB ->
B_80_788_0 - _rtB -> B_80_794_0 ; isHit = ssIsSampleHit ( S , 2 , 0 ) ; if (
isHit != 0 ) { _rtB -> B_80_797_0 = _rtP -> P_543 * _rtB -> B_80_779_0 ; } if
( _rtB -> B_80_795_0 > _rtB -> B_80_779_0 ) { _rtB -> B_80_799_0 = _rtB ->
B_80_779_0 ; } else { if ( _rtB -> B_80_795_0 < _rtB -> B_80_797_0 ) { _rtB
-> B_67_1_0 = _rtB -> B_80_797_0 ; } else { _rtB -> B_67_1_0 = _rtB ->
B_80_795_0 ; } _rtB -> B_80_799_0 = _rtB -> B_67_1_0 ; } isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { ssCallAccelRunBlock ( S ,
68 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP -> P_922 != 0 ) && ( ! ( _rtB ->
B_68_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_922 == 1 ) { if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_dv == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model4/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_dv < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_dv ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model4/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_138_0_o ) ; if ( ( _rtP -> P_922 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_922 == 1 ) {
if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_dv == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model4/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_dv < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_dv ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model4/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_138_0_o ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_138_0_o ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_801_0 = _rtB
-> B_80_137_0_i [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_68_0_0 , 499U ) ] ; _rtB -> B_80_802_0 =
_rtB -> B_80_140_0_c * _rtB -> B_80_801_0 ; if ( ( _rtP -> P_923 != 0 ) && (
! ( _rtB -> B_68_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_923 == 1 ) { if ( _rtDW
-> lookupwagonbrakingefficiency_NumIssuedWarnings_aq == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_aq < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_aq ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_139_0_m ) ; if ( ( _rtP -> P_923 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_923 == 1 ) {
if ( _rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_aq == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_aq < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_aq ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_139_0_m ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_139_0_m ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_803_0 = _rtB
-> B_80_137_0_i [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_68_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_lr . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_lr .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_544 ; _rtB -> B_80_804_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_gs . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_gs . Last , _rtDW -> TransportDelay_IWORK_gs . Tail ,
_rtDW -> TransportDelay_IWORK_gs . Head , _rtP -> P_545 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_80_805_0 = 0.0 ; _rtB -> B_80_805_0 += _rtP -> P_547 * _rtX
-> TransferFcn_CSTATE_in ; _rtB -> B_80_805_0 += _rtP -> P_548 * _rtB ->
B_80_804_0 ; if ( _rtB -> B_80_805_0 > _rtP -> P_550 ) { _rtB -> B_80_806_0 =
_rtB -> B_80_805_0 - _rtP -> P_550 ; } else if ( _rtB -> B_80_805_0 >= _rtP
-> P_549 ) { _rtB -> B_80_806_0 = 0.0 ; } else { _rtB -> B_80_806_0 = _rtB ->
B_80_805_0 - _rtP -> P_549 ; } _rtB -> B_80_807_0 = _rtP -> P_551 * _rtB ->
B_80_806_0 ; if ( _rtB -> B_80_807_0 > _rtP -> P_552 ) { _rtB -> B_80_808_0 =
_rtP -> P_552 ; } else if ( _rtB -> B_80_807_0 < _rtP -> P_553 ) { _rtB ->
B_80_808_0 = _rtP -> P_553 ; } else { _rtB -> B_80_808_0 = _rtB -> B_80_807_0
; } if ( ( _rtDW -> LastMajorTimeA_id >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW
-> LastMajorTimeB_ei >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_80_809_0 =
_rtB -> B_80_808_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_id < _rtDW ->
LastMajorTimeB_ei ) && ( _rtDW -> LastMajorTimeB_ei < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_id >= _rtDW -> LastMajorTimeB_ei ) && (
_rtDW -> LastMajorTimeA_id >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_ei ; rtb_B_80_3_0 = _rtDW
-> PrevYB_ew ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_id ; rtb_B_80_3_0 = _rtDW -> PrevYA_jo ; } riseValLimit =
deltaT * _rtP -> P_554 ; rateLimiterRate = _rtB -> B_80_808_0 - rtb_B_80_3_0
; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_80_809_0 = rtb_B_80_3_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_555 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_80_809_0 = rtb_B_80_3_0 + deltaT ; } else { _rtB ->
B_80_809_0 = _rtB -> B_80_808_0 ; } } } _rtB -> B_80_810_0 = _rtP -> P_556 *
_rtB -> B_80_809_0 ; _rtB -> B_80_811_0 = _rtB -> B_80_803_0 * _rtB ->
B_80_810_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem6/Wagon model4/Data Store Read" ,
0 ) ; _rtB -> B_80_812_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_oe != 0 )
{ _rtX -> vv0at_CSTATE_i2 = _rtB -> B_80_812_0 ; } _rtB -> B_80_813_0 = _rtX
-> vv0at_CSTATE_i2 ; _rtB -> B_80_814_0 = muDoubleScalarAbs ( _rtB ->
B_80_813_0 ) ; _rtB -> B_80_815_0 = _rtP -> P_557 [ 0 ] ; _rtB -> B_80_815_0
= _rtB -> B_80_815_0 * _rtB -> B_80_814_0 + _rtP -> P_557 [ 1 ] ; _rtB ->
B_80_815_0 = _rtB -> B_80_815_0 * _rtB -> B_80_814_0 + _rtP -> P_557 [ 2 ] ;
_rtB -> B_80_816_0 = _rtP -> P_558 * _rtB -> B_80_815_0 ; _rtB -> B_80_817_0
= _rtB -> B_80_801_0 * _rtB -> B_80_816_0 ; _rtB -> B_80_818_0 = _rtB ->
B_80_811_0 - _rtB -> B_80_817_0 ; isHit = ssIsSampleHit ( S , 2 , 0 ) ; if (
isHit != 0 ) { _rtB -> B_80_820_0 = _rtP -> P_559 * _rtB -> B_80_802_0 ; } if
( _rtB -> B_80_818_0 > _rtB -> B_80_802_0 ) { _rtB -> B_80_822_0 = _rtB ->
B_80_802_0 ; } else { if ( _rtB -> B_80_818_0 < _rtB -> B_80_820_0 ) { _rtB
-> B_69_1_0 = _rtB -> B_80_820_0 ; } else { _rtB -> B_69_1_0 = _rtB ->
B_80_818_0 ; } _rtB -> B_80_822_0 = _rtB -> B_69_1_0 ; } isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { ssCallAccelRunBlock ( S ,
70 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP -> P_924 != 0 ) && ( ! ( _rtB ->
B_70_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_924 == 1 ) { if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_jc == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model/lookup wagon mass" )
; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_jc < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_jc ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model/lookup wagon mass" )
; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_142_0_h ) ; if ( ( _rtP -> P_924 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_924 == 1 ) {
if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_jc == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model/lookup wagon mass" )
; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_jc < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_jc ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model/lookup wagon mass" )
; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_142_0_h ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_142_0_h ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_824_0 = _rtB
-> B_80_141_0_f [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_70_0_0 , 499U ) ] ; _rtB -> B_80_825_0 =
_rtB -> B_80_144_0_a * _rtB -> B_80_824_0 ; if ( ( _rtP -> P_925 != 0 ) && (
! ( _rtB -> B_70_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_925 == 1 ) { if ( _rtDW
-> lookupwagonbrakingefficiency_NumIssuedWarnings_ac == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_ac < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_ac ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_143_0_m ) ; if ( ( _rtP -> P_925 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_925 == 1 ) {
if ( _rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_ac == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_ac < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_ac ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_143_0_m ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_143_0_m ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_826_0 = _rtB
-> B_80_141_0_f [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_70_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_gr . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_gr .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_560 ; _rtB -> B_80_827_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_id . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_id . Last , _rtDW -> TransportDelay_IWORK_id . Tail ,
_rtDW -> TransportDelay_IWORK_id . Head , _rtP -> P_561 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_80_828_0 = 0.0 ; _rtB -> B_80_828_0 += _rtP -> P_563 * _rtX
-> TransferFcn_CSTATE_cy ; _rtB -> B_80_828_0 += _rtP -> P_564 * _rtB ->
B_80_827_0 ; if ( _rtB -> B_80_828_0 > _rtP -> P_566 ) { _rtB -> B_80_829_0 =
_rtB -> B_80_828_0 - _rtP -> P_566 ; } else if ( _rtB -> B_80_828_0 >= _rtP
-> P_565 ) { _rtB -> B_80_829_0 = 0.0 ; } else { _rtB -> B_80_829_0 = _rtB ->
B_80_828_0 - _rtP -> P_565 ; } _rtB -> B_80_830_0 = _rtP -> P_567 * _rtB ->
B_80_829_0 ; if ( _rtB -> B_80_830_0 > _rtP -> P_568 ) { _rtB -> B_80_831_0 =
_rtP -> P_568 ; } else if ( _rtB -> B_80_830_0 < _rtP -> P_569 ) { _rtB ->
B_80_831_0 = _rtP -> P_569 ; } else { _rtB -> B_80_831_0 = _rtB -> B_80_830_0
; } if ( ( _rtDW -> LastMajorTimeA_np >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW
-> LastMajorTimeB_lo >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_80_832_0 =
_rtB -> B_80_831_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_np < _rtDW ->
LastMajorTimeB_lo ) && ( _rtDW -> LastMajorTimeB_lo < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_np >= _rtDW -> LastMajorTimeB_lo ) && (
_rtDW -> LastMajorTimeA_np >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_lo ; rtb_B_80_3_0 = _rtDW
-> PrevYB_k1 ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_np ; rtb_B_80_3_0 = _rtDW -> PrevYA_ei ; } riseValLimit =
deltaT * _rtP -> P_570 ; rateLimiterRate = _rtB -> B_80_831_0 - rtb_B_80_3_0
; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_80_832_0 = rtb_B_80_3_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_571 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_80_832_0 = rtb_B_80_3_0 + deltaT ; } else { _rtB ->
B_80_832_0 = _rtB -> B_80_831_0 ; } } } _rtB -> B_80_833_0 = _rtP -> P_572 *
_rtB -> B_80_832_0 ; _rtB -> B_80_834_0 = _rtB -> B_80_826_0 * _rtB ->
B_80_833_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model/Data Store Read" , 0
) ; _rtB -> B_80_835_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_a != 0 ) {
_rtX -> vv0at_CSTATE_mt = _rtB -> B_80_835_0 ; } _rtB -> B_80_836_0 = _rtX ->
vv0at_CSTATE_mt ; _rtB -> B_80_837_0 = muDoubleScalarAbs ( _rtB -> B_80_836_0
) ; _rtB -> B_80_838_0 = _rtP -> P_573 [ 0 ] ; _rtB -> B_80_838_0 = _rtB ->
B_80_838_0 * _rtB -> B_80_837_0 + _rtP -> P_573 [ 1 ] ; _rtB -> B_80_838_0 =
_rtB -> B_80_838_0 * _rtB -> B_80_837_0 + _rtP -> P_573 [ 2 ] ; _rtB ->
B_80_839_0 = _rtP -> P_574 * _rtB -> B_80_838_0 ; _rtB -> B_80_840_0 = _rtB
-> B_80_824_0 * _rtB -> B_80_839_0 ; _rtB -> B_80_841_0 = _rtB -> B_80_834_0
- _rtB -> B_80_840_0 ; isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0
) { _rtB -> B_80_843_0 = _rtP -> P_575 * _rtB -> B_80_825_0 ; } if ( _rtB ->
B_80_841_0 > _rtB -> B_80_825_0 ) { _rtB -> B_80_845_0 = _rtB -> B_80_825_0 ;
} else { if ( _rtB -> B_80_841_0 < _rtB -> B_80_843_0 ) { _rtB -> B_71_1_0 =
_rtB -> B_80_843_0 ; } else { _rtB -> B_71_1_0 = _rtB -> B_80_841_0 ; } _rtB
-> B_80_845_0 = _rtB -> B_71_1_0 ; } isHit = ssIsSampleHit ( S , 2 , 0 ) ; if
( isHit != 0 ) { ssCallAccelRunBlock ( S , 72 , 0 , SS_CALL_MDL_OUTPUTS ) ;
if ( ( _rtP -> P_926 != 0 ) && ( ! ( _rtB -> B_72_0_0 <= 499.0 ) ) ) { if (
_rtP -> P_926 == 1 ) { if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_e1 ==
0U ) { diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model1/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_e1 < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_e1 ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model1/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_146_0_p ) ; if ( ( _rtP -> P_926 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_926 == 1 ) {
if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_e1 == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model1/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_e1 < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_e1 ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model1/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_146_0_p ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_146_0_p ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_847_0 = _rtB
-> B_80_145_0_k [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_72_0_0 , 499U ) ] ; _rtB -> B_80_848_0 =
_rtB -> B_80_148_0_c * _rtB -> B_80_847_0 ; if ( ( _rtP -> P_927 != 0 ) && (
! ( _rtB -> B_72_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_927 == 1 ) { if ( _rtDW
-> lookupwagonbrakingefficiency_NumIssuedWarnings_nd == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_nd < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_nd ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_147_0_b ) ; if ( ( _rtP -> P_927 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_927 == 1 ) {
if ( _rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_nd == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_nd < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_nd ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model1/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_147_0_b ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_147_0_b ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_849_0 = _rtB
-> B_80_145_0_k [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_72_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_ny . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_ny .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_576 ; _rtB -> B_80_850_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_ch . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_ch . Last , _rtDW -> TransportDelay_IWORK_ch . Tail ,
_rtDW -> TransportDelay_IWORK_ch . Head , _rtP -> P_577 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_80_851_0 = 0.0 ; _rtB -> B_80_851_0 += _rtP -> P_579 * _rtX
-> TransferFcn_CSTATE_h0 ; _rtB -> B_80_851_0 += _rtP -> P_580 * _rtB ->
B_80_850_0 ; if ( _rtB -> B_80_851_0 > _rtP -> P_582 ) { _rtB -> B_80_852_0 =
_rtB -> B_80_851_0 - _rtP -> P_582 ; } else if ( _rtB -> B_80_851_0 >= _rtP
-> P_581 ) { _rtB -> B_80_852_0 = 0.0 ; } else { _rtB -> B_80_852_0 = _rtB ->
B_80_851_0 - _rtP -> P_581 ; } _rtB -> B_80_853_0 = _rtP -> P_583 * _rtB ->
B_80_852_0 ; if ( _rtB -> B_80_853_0 > _rtP -> P_584 ) { _rtB -> B_80_854_0 =
_rtP -> P_584 ; } else if ( _rtB -> B_80_853_0 < _rtP -> P_585 ) { _rtB ->
B_80_854_0 = _rtP -> P_585 ; } else { _rtB -> B_80_854_0 = _rtB -> B_80_853_0
; } if ( ( _rtDW -> LastMajorTimeA_ny >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW
-> LastMajorTimeB_pl >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_80_855_0 =
_rtB -> B_80_854_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_ny < _rtDW ->
LastMajorTimeB_pl ) && ( _rtDW -> LastMajorTimeB_pl < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_ny >= _rtDW -> LastMajorTimeB_pl ) && (
_rtDW -> LastMajorTimeA_ny >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_pl ; rtb_B_80_3_0 = _rtDW
-> PrevYB_fn ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_ny ; rtb_B_80_3_0 = _rtDW -> PrevYA_fn ; } riseValLimit =
deltaT * _rtP -> P_586 ; rateLimiterRate = _rtB -> B_80_854_0 - rtb_B_80_3_0
; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_80_855_0 = rtb_B_80_3_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_587 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_80_855_0 = rtb_B_80_3_0 + deltaT ; } else { _rtB ->
B_80_855_0 = _rtB -> B_80_854_0 ; } } } _rtB -> B_80_856_0 = _rtP -> P_588 *
_rtB -> B_80_855_0 ; _rtB -> B_80_857_0 = _rtB -> B_80_849_0 * _rtB ->
B_80_856_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model1/Data Store Read" ,
0 ) ; _rtB -> B_80_858_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_ae != 0 )
{ _rtX -> vv0at_CSTATE_ps = _rtB -> B_80_858_0 ; } _rtB -> B_80_859_0 = _rtX
-> vv0at_CSTATE_ps ; _rtB -> B_80_860_0 = muDoubleScalarAbs ( _rtB ->
B_80_859_0 ) ; _rtB -> B_80_861_0 = _rtP -> P_589 [ 0 ] ; _rtB -> B_80_861_0
= _rtB -> B_80_861_0 * _rtB -> B_80_860_0 + _rtP -> P_589 [ 1 ] ; _rtB ->
B_80_861_0 = _rtB -> B_80_861_0 * _rtB -> B_80_860_0 + _rtP -> P_589 [ 2 ] ;
_rtB -> B_80_862_0 = _rtP -> P_590 * _rtB -> B_80_861_0 ; _rtB -> B_80_863_0
= _rtB -> B_80_847_0 * _rtB -> B_80_862_0 ; _rtB -> B_80_864_0 = _rtB ->
B_80_857_0 - _rtB -> B_80_863_0 ; isHit = ssIsSampleHit ( S , 2 , 0 ) ; if (
isHit != 0 ) { _rtB -> B_80_866_0 = _rtP -> P_591 * _rtB -> B_80_848_0 ; } if
( _rtB -> B_80_864_0 > _rtB -> B_80_848_0 ) { _rtB -> B_80_868_0 = _rtB ->
B_80_848_0 ; } else { if ( _rtB -> B_80_864_0 < _rtB -> B_80_866_0 ) { _rtB
-> B_73_1_0 = _rtB -> B_80_866_0 ; } else { _rtB -> B_73_1_0 = _rtB ->
B_80_864_0 ; } _rtB -> B_80_868_0 = _rtB -> B_73_1_0 ; } isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { ssCallAccelRunBlock ( S ,
74 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP -> P_928 != 0 ) && ( ! ( _rtB ->
B_74_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_928 == 1 ) { if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_k == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model2/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_k < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_k ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model2/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_150_0_i ) ; if ( ( _rtP -> P_928 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_928 == 1 ) {
if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_k == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model2/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_k < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_k ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model2/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_150_0_i ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_150_0_i ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_870_0 = _rtB
-> B_80_149_0_n [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_74_0_0 , 499U ) ] ; _rtB -> B_80_871_0 =
_rtB -> B_80_152_0 * _rtB -> B_80_870_0 ; if ( ( _rtP -> P_929 != 0 ) && ( !
( _rtB -> B_74_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_929 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_l == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_l < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_l ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_151_0_m ) ; if ( ( _rtP -> P_929 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_929 == 1 ) {
if ( _rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_l == 0U ) { diag
= CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_l < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_l ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model2/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_151_0_m ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_151_0_m ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_872_0 = _rtB
-> B_80_149_0_n [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_74_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_a5 . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_a5 .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_592 ; _rtB -> B_80_873_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_kh . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_kh . Last , _rtDW -> TransportDelay_IWORK_kh . Tail ,
_rtDW -> TransportDelay_IWORK_kh . Head , _rtP -> P_593 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_80_874_0 = 0.0 ; _rtB -> B_80_874_0 += _rtP -> P_595 * _rtX
-> TransferFcn_CSTATE_es ; _rtB -> B_80_874_0 += _rtP -> P_596 * _rtB ->
B_80_873_0 ; if ( _rtB -> B_80_874_0 > _rtP -> P_598 ) { _rtB -> B_80_875_0 =
_rtB -> B_80_874_0 - _rtP -> P_598 ; } else if ( _rtB -> B_80_874_0 >= _rtP
-> P_597 ) { _rtB -> B_80_875_0 = 0.0 ; } else { _rtB -> B_80_875_0 = _rtB ->
B_80_874_0 - _rtP -> P_597 ; } _rtB -> B_80_876_0 = _rtP -> P_599 * _rtB ->
B_80_875_0 ; if ( _rtB -> B_80_876_0 > _rtP -> P_600 ) { _rtB -> B_80_877_0 =
_rtP -> P_600 ; } else if ( _rtB -> B_80_876_0 < _rtP -> P_601 ) { _rtB ->
B_80_877_0 = _rtP -> P_601 ; } else { _rtB -> B_80_877_0 = _rtB -> B_80_876_0
; } if ( ( _rtDW -> LastMajorTimeA_m >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW
-> LastMajorTimeB_mm >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_80_878_0 =
_rtB -> B_80_877_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_m < _rtDW ->
LastMajorTimeB_mm ) && ( _rtDW -> LastMajorTimeB_mm < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_m >= _rtDW -> LastMajorTimeB_mm ) && (
_rtDW -> LastMajorTimeA_m >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_mm ; rtb_B_80_3_0 = _rtDW
-> PrevYB_gv ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_m ; rtb_B_80_3_0 = _rtDW -> PrevYA_dd ; } riseValLimit =
deltaT * _rtP -> P_602 ; rateLimiterRate = _rtB -> B_80_877_0 - rtb_B_80_3_0
; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_80_878_0 = rtb_B_80_3_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_603 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_80_878_0 = rtb_B_80_3_0 + deltaT ; } else { _rtB ->
B_80_878_0 = _rtB -> B_80_877_0 ; } } } _rtB -> B_80_879_0 = _rtP -> P_604 *
_rtB -> B_80_878_0 ; _rtB -> B_80_880_0 = _rtB -> B_80_872_0 * _rtB ->
B_80_879_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model2/Data Store Read" ,
0 ) ; _rtB -> B_80_881_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_hw != 0 )
{ _rtX -> vv0at_CSTATE_i1 = _rtB -> B_80_881_0 ; } _rtB -> B_80_882_0 = _rtX
-> vv0at_CSTATE_i1 ; _rtB -> B_80_883_0 = muDoubleScalarAbs ( _rtB ->
B_80_882_0 ) ; _rtB -> B_80_884_0 = _rtP -> P_605 [ 0 ] ; _rtB -> B_80_884_0
= _rtB -> B_80_884_0 * _rtB -> B_80_883_0 + _rtP -> P_605 [ 1 ] ; _rtB ->
B_80_884_0 = _rtB -> B_80_884_0 * _rtB -> B_80_883_0 + _rtP -> P_605 [ 2 ] ;
_rtB -> B_80_885_0 = _rtP -> P_606 * _rtB -> B_80_884_0 ; _rtB -> B_80_886_0
= _rtB -> B_80_870_0 * _rtB -> B_80_885_0 ; _rtB -> B_80_887_0 = _rtB ->
B_80_880_0 - _rtB -> B_80_886_0 ; isHit = ssIsSampleHit ( S , 2 , 0 ) ; if (
isHit != 0 ) { _rtB -> B_80_889_0 = _rtP -> P_607 * _rtB -> B_80_871_0 ; } if
( _rtB -> B_80_887_0 > _rtB -> B_80_871_0 ) { _rtB -> B_80_891_0 = _rtB ->
B_80_871_0 ; } else { if ( _rtB -> B_80_887_0 < _rtB -> B_80_889_0 ) { _rtB
-> B_75_1_0 = _rtB -> B_80_889_0 ; } else { _rtB -> B_75_1_0 = _rtB ->
B_80_887_0 ; } _rtB -> B_80_891_0 = _rtB -> B_75_1_0 ; } isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { ssCallAccelRunBlock ( S ,
76 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP -> P_930 != 0 ) && ( ! ( _rtB ->
B_76_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_930 == 1 ) { if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_hh == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model3/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_hh < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_hh ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model3/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_154_0 ) ; if ( ( _rtP -> P_930 != 0 ) && ( ( ! ( rtb_B_80_3_0 >=
0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_930 == 1 ) { if
( _rtDW -> lookupwagonmass_NumIssuedWarnings_hh == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model3/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_hh < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_hh ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model3/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_154_0 ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_154_0 ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if (
rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_893_0 = _rtB ->
B_80_153_0_j [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_76_0_0 , 499U ) ] ; _rtB -> B_80_894_0 =
_rtB -> B_80_156_0 * _rtB -> B_80_893_0 ; if ( ( _rtP -> P_931 != 0 ) && ( !
( _rtB -> B_76_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_931 == 1 ) { if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_ma == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_ma < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_ma ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_155_0_e ) ; if ( ( _rtP -> P_931 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_931 == 1 ) {
if ( _rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_ma == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_ma < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_ma ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model3/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_155_0_e ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_155_0_e ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_895_0 = _rtB
-> B_80_153_0_j [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_76_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_do . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_do .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_608 ; _rtB -> B_80_896_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_ne . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_ne . Last , _rtDW -> TransportDelay_IWORK_ne . Tail ,
_rtDW -> TransportDelay_IWORK_ne . Head , _rtP -> P_609 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_80_897_0 = 0.0 ; _rtB -> B_80_897_0 += _rtP -> P_611 * _rtX
-> TransferFcn_CSTATE_fs ; _rtB -> B_80_897_0 += _rtP -> P_612 * _rtB ->
B_80_896_0 ; if ( _rtB -> B_80_897_0 > _rtP -> P_614 ) { _rtB -> B_80_898_0 =
_rtB -> B_80_897_0 - _rtP -> P_614 ; } else if ( _rtB -> B_80_897_0 >= _rtP
-> P_613 ) { _rtB -> B_80_898_0 = 0.0 ; } else { _rtB -> B_80_898_0 = _rtB ->
B_80_897_0 - _rtP -> P_613 ; } _rtB -> B_80_899_0 = _rtP -> P_615 * _rtB ->
B_80_898_0 ; if ( _rtB -> B_80_899_0 > _rtP -> P_616 ) { _rtB -> B_80_900_0 =
_rtP -> P_616 ; } else if ( _rtB -> B_80_899_0 < _rtP -> P_617 ) { _rtB ->
B_80_900_0 = _rtP -> P_617 ; } else { _rtB -> B_80_900_0 = _rtB -> B_80_899_0
; } if ( ( _rtDW -> LastMajorTimeA_hd >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW
-> LastMajorTimeB_n1 >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_80_901_0 =
_rtB -> B_80_900_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_hd < _rtDW ->
LastMajorTimeB_n1 ) && ( _rtDW -> LastMajorTimeB_n1 < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_hd >= _rtDW -> LastMajorTimeB_n1 ) && (
_rtDW -> LastMajorTimeA_hd >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_n1 ; rtb_B_80_3_0 = _rtDW
-> PrevYB_cx ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_hd ; rtb_B_80_3_0 = _rtDW -> PrevYA_e5 ; } riseValLimit =
deltaT * _rtP -> P_618 ; rateLimiterRate = _rtB -> B_80_900_0 - rtb_B_80_3_0
; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_80_901_0 = rtb_B_80_3_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_619 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_80_901_0 = rtb_B_80_3_0 + deltaT ; } else { _rtB ->
B_80_901_0 = _rtB -> B_80_900_0 ; } } } _rtB -> B_80_902_0 = _rtP -> P_620 *
_rtB -> B_80_901_0 ; _rtB -> B_80_903_0 = _rtB -> B_80_895_0 * _rtB ->
B_80_902_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model3/Data Store Read" ,
0 ) ; _rtB -> B_80_904_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_fzr != 0 )
{ _rtX -> vv0at_CSTATE_bu = _rtB -> B_80_904_0 ; } _rtB -> B_80_905_0 = _rtX
-> vv0at_CSTATE_bu ; _rtB -> B_80_906_0 = muDoubleScalarAbs ( _rtB ->
B_80_905_0 ) ; _rtB -> B_80_907_0 = _rtP -> P_621 [ 0 ] ; _rtB -> B_80_907_0
= _rtB -> B_80_907_0 * _rtB -> B_80_906_0 + _rtP -> P_621 [ 1 ] ; _rtB ->
B_80_907_0 = _rtB -> B_80_907_0 * _rtB -> B_80_906_0 + _rtP -> P_621 [ 2 ] ;
_rtB -> B_80_908_0 = _rtP -> P_622 * _rtB -> B_80_907_0 ; _rtB -> B_80_909_0
= _rtB -> B_80_893_0 * _rtB -> B_80_908_0 ; _rtB -> B_80_910_0 = _rtB ->
B_80_903_0 - _rtB -> B_80_909_0 ; isHit = ssIsSampleHit ( S , 2 , 0 ) ; if (
isHit != 0 ) { _rtB -> B_80_912_0 = _rtP -> P_623 * _rtB -> B_80_894_0 ; } if
( _rtB -> B_80_910_0 > _rtB -> B_80_894_0 ) { _rtB -> B_80_914_0 = _rtB ->
B_80_894_0 ; } else { if ( _rtB -> B_80_910_0 < _rtB -> B_80_912_0 ) { _rtB
-> B_77_1_0 = _rtB -> B_80_912_0 ; } else { _rtB -> B_77_1_0 = _rtB ->
B_80_910_0 ; } _rtB -> B_80_914_0 = _rtB -> B_77_1_0 ; } isHit =
ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0 ) { ssCallAccelRunBlock ( S ,
78 , 0 , SS_CALL_MDL_OUTPUTS ) ; if ( ( _rtP -> P_932 != 0 ) && ( ! ( _rtB ->
B_78_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_932 == 1 ) { if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_ds == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model4/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_ds < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_ds ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model4/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_158_0_m ) ; if ( ( _rtP -> P_932 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_932 == 1 ) {
if ( _rtDW -> lookupwagonmass_NumIssuedWarnings_ds == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model4/lookup wagon mass"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonmass_NumIssuedWarnings_ds < MAX_uint32_T ) { _rtDW ->
lookupwagonmass_NumIssuedWarnings_ds ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
"Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model4/lookup wagon mass"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_158_0_m ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_158_0_m ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_916_0 = _rtB
-> B_80_157_0_m [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_78_0_0 , 499U ) ] ; _rtB -> B_80_917_0 =
_rtB -> B_80_160_0_f * _rtB -> B_80_916_0 ; if ( ( _rtP -> P_933 != 0 ) && (
! ( _rtB -> B_78_0_0 <= 499.0 ) ) ) { if ( _rtP -> P_933 == 1 ) { if ( _rtDW
-> lookupwagonbrakingefficiency_NumIssuedWarnings_fw == 0U ) { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_fw < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_fw ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } rtb_B_80_3_0 = muDoubleScalarFloor (
_rtB -> B_80_159_0_j ) ; if ( ( _rtP -> P_933 != 0 ) && ( ( ! ( rtb_B_80_3_0
>= 0.0 ) ) || ( ! ( rtb_B_80_3_0 <= 2.0 ) ) ) ) { if ( _rtP -> P_933 == 1 ) {
if ( _rtDW -> lookupwagonbrakingefficiency_NumIssuedWarnings_fw == 0U ) {
diag = CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeWarnMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssReportDiagnosticAsWarning ( S , diag ) ; } if ( _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_fw < MAX_uint32_T ) { _rtDW ->
lookupwagonbrakingefficiency_NumIssuedWarnings_fw ++ ; } } else { diag =
CreateDiagnosticAsVoidPtr (
"Simulink:blocks:DirectLookupNdBlockIndexOutOfRangeErrorMsg" , 1 , 5 ,
 "Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model4/lookup wagon braking efficiency"
) ; rt_ssSet_slErrMsg ( S , diag ) ; } } if ( muDoubleScalarIsNaN ( _rtB ->
B_80_159_0_j ) ) { rtb_B_80_3_0 = 0.0 ; } else { rtb_B_80_3_0 = _rtB ->
B_80_159_0_j ; } if ( rtb_B_80_3_0 > 2.0 ) { rtb_B_80_3_0 = 2.0 ; } else { if
( rtb_B_80_3_0 < 0.0 ) { rtb_B_80_3_0 = 0.0 ; } } _rtB -> B_80_918_0 = _rtB
-> B_80_157_0_m [ ( int32_T ) rtb_B_80_3_0 * 500 + ( int32_T )
muIntScalarMin_uint32 ( _rtB -> B_78_0_0 , 499U ) ] ; } { real_T * * uBuffer
= ( real_T * * ) & _rtDW -> TransportDelay_PWORK_gu . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_gu .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; real_T tMinusDelay =
simTime - _rtP -> P_624 ; _rtB -> B_80_919_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_a4b . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_a4b . Last , _rtDW -> TransportDelay_IWORK_a4b . Tail ,
_rtDW -> TransportDelay_IWORK_a4b . Head , _rtP -> P_625 , 0 , ( boolean_T )
( ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) )
) ) ; } _rtB -> B_80_920_0 = 0.0 ; _rtB -> B_80_920_0 += _rtP -> P_627 * _rtX
-> TransferFcn_CSTATE_he ; _rtB -> B_80_920_0 += _rtP -> P_628 * _rtB ->
B_80_919_0 ; if ( _rtB -> B_80_920_0 > _rtP -> P_630 ) { _rtB -> B_80_921_0 =
_rtB -> B_80_920_0 - _rtP -> P_630 ; } else if ( _rtB -> B_80_920_0 >= _rtP
-> P_629 ) { _rtB -> B_80_921_0 = 0.0 ; } else { _rtB -> B_80_921_0 = _rtB ->
B_80_920_0 - _rtP -> P_629 ; } _rtB -> B_80_922_0 = _rtP -> P_631 * _rtB ->
B_80_921_0 ; if ( _rtB -> B_80_922_0 > _rtP -> P_632 ) { _rtB -> B_80_923_0 =
_rtP -> P_632 ; } else if ( _rtB -> B_80_922_0 < _rtP -> P_633 ) { _rtB ->
B_80_923_0 = _rtP -> P_633 ; } else { _rtB -> B_80_923_0 = _rtB -> B_80_922_0
; } if ( ( _rtDW -> LastMajorTimeA_dm >= ssGetTaskTime ( S , 0 ) ) && ( _rtDW
-> LastMajorTimeB_ek >= ssGetTaskTime ( S , 0 ) ) ) { _rtB -> B_80_924_0 =
_rtB -> B_80_923_0 ; } else { if ( ( ( _rtDW -> LastMajorTimeA_dm < _rtDW ->
LastMajorTimeB_ek ) && ( _rtDW -> LastMajorTimeB_ek < ssGetTaskTime ( S , 0 )
) ) || ( ( _rtDW -> LastMajorTimeA_dm >= _rtDW -> LastMajorTimeB_ek ) && (
_rtDW -> LastMajorTimeA_dm >= ssGetTaskTime ( S , 0 ) ) ) ) { deltaT =
ssGetTaskTime ( S , 0 ) - _rtDW -> LastMajorTimeB_ek ; rtb_B_80_3_0 = _rtDW
-> PrevYB_jt ; } else { deltaT = ssGetTaskTime ( S , 0 ) - _rtDW ->
LastMajorTimeA_dm ; rtb_B_80_3_0 = _rtDW -> PrevYA_e0 ; } riseValLimit =
deltaT * _rtP -> P_634 ; rateLimiterRate = _rtB -> B_80_923_0 - rtb_B_80_3_0
; if ( rateLimiterRate > riseValLimit ) { _rtB -> B_80_924_0 = rtb_B_80_3_0 +
riseValLimit ; } else { deltaT *= _rtP -> P_635 ; if ( rateLimiterRate <
deltaT ) { _rtB -> B_80_924_0 = rtb_B_80_3_0 + deltaT ; } else { _rtB ->
B_80_924_0 = _rtB -> B_80_923_0 ; } } } _rtB -> B_80_925_0 = _rtP -> P_636 *
_rtB -> B_80_924_0 ; _rtB -> B_80_926_0 = _rtB -> B_80_918_0 * _rtB ->
B_80_925_0 ; vm_ReadLocalDSMNoIdx ( S , _rtDW -> dsmIdx , ( char_T * )
"Simulation_v2/Train of 40 wagons/Subsystem7/Wagon model4/Data Store Read" ,
0 ) ; _rtB -> B_80_927_0 = _rtDW -> v0 ; if ( _rtDW -> vv0at_IWORK_ke != 0 )
{ _rtX -> vv0at_CSTATE_fw = _rtB -> B_80_927_0 ; } _rtB -> B_80_928_0 = _rtX
-> vv0at_CSTATE_fw ; _rtB -> B_80_929_0 = muDoubleScalarAbs ( _rtB ->
B_80_928_0 ) ; _rtB -> B_80_930_0 = _rtP -> P_637 [ 0 ] ; _rtB -> B_80_930_0
= _rtB -> B_80_930_0 * _rtB -> B_80_929_0 + _rtP -> P_637 [ 1 ] ; _rtB ->
B_80_930_0 = _rtB -> B_80_930_0 * _rtB -> B_80_929_0 + _rtP -> P_637 [ 2 ] ;
_rtB -> B_80_931_0 = _rtP -> P_638 * _rtB -> B_80_930_0 ; _rtB -> B_80_932_0
= _rtB -> B_80_916_0 * _rtB -> B_80_931_0 ; _rtB -> B_80_933_0 = _rtB ->
B_80_926_0 - _rtB -> B_80_932_0 ; isHit = ssIsSampleHit ( S , 2 , 0 ) ; if (
isHit != 0 ) { _rtB -> B_80_935_0 = _rtP -> P_639 * _rtB -> B_80_917_0 ; } if
( _rtB -> B_80_933_0 > _rtB -> B_80_917_0 ) { _rtB -> B_80_937_0 = _rtB ->
B_80_917_0 ; } else { if ( _rtB -> B_80_933_0 < _rtB -> B_80_935_0 ) { _rtB
-> B_79_1_0 = _rtB -> B_80_935_0 ; } else { _rtB -> B_79_1_0 = _rtB ->
B_80_933_0 ; } _rtB -> B_80_937_0 = _rtB -> B_79_1_0 ; } _rtB -> B_80_938_0 [
0 ] = _rtB -> B_80_40_0 ; _rtB -> B_80_938_0 [ 1 ] = _rtB -> B_80_63_0 ; _rtB
-> B_80_938_0 [ 2 ] = _rtB -> B_80_86_0 ; _rtB -> B_80_938_0 [ 3 ] = _rtB ->
B_80_109_0 ; _rtB -> B_80_938_0 [ 4 ] = _rtB -> B_80_132_0 ; _rtB ->
B_80_938_0 [ 5 ] = _rtB -> B_80_155_0 ; _rtB -> B_80_938_0 [ 6 ] = _rtB ->
B_80_178_0 ; _rtB -> B_80_938_0 [ 7 ] = _rtB -> B_80_201_0 ; _rtB ->
B_80_938_0 [ 8 ] = _rtB -> B_80_224_0 ; _rtB -> B_80_938_0 [ 9 ] = _rtB ->
B_80_247_0 ; _rtB -> B_80_938_0 [ 10 ] = _rtB -> B_80_270_0 ; _rtB ->
B_80_938_0 [ 11 ] = _rtB -> B_80_293_0 ; _rtB -> B_80_938_0 [ 12 ] = _rtB ->
B_80_316_0 ; _rtB -> B_80_938_0 [ 13 ] = _rtB -> B_80_339_0 ; _rtB ->
B_80_938_0 [ 14 ] = _rtB -> B_80_362_0 ; _rtB -> B_80_938_0 [ 15 ] = _rtB ->
B_80_385_0 ; _rtB -> B_80_938_0 [ 16 ] = _rtB -> B_80_408_0 ; _rtB ->
B_80_938_0 [ 17 ] = _rtB -> B_80_431_0 ; _rtB -> B_80_938_0 [ 18 ] = _rtB ->
B_80_454_0 ; _rtB -> B_80_938_0 [ 19 ] = _rtB -> B_80_477_0 ; _rtB ->
B_80_938_0 [ 20 ] = _rtB -> B_80_500_0 ; _rtB -> B_80_938_0 [ 21 ] = _rtB ->
B_80_523_0 ; _rtB -> B_80_938_0 [ 22 ] = _rtB -> B_80_546_0 ; _rtB ->
B_80_938_0 [ 23 ] = _rtB -> B_80_569_0 ; _rtB -> B_80_938_0 [ 24 ] = _rtB ->
B_80_592_0 ; _rtB -> B_80_938_0 [ 25 ] = _rtB -> B_80_615_0 ; _rtB ->
B_80_938_0 [ 26 ] = _rtB -> B_80_638_0 ; _rtB -> B_80_938_0 [ 27 ] = _rtB ->
B_80_661_0 ; _rtB -> B_80_938_0 [ 28 ] = _rtB -> B_80_684_0 ; _rtB ->
B_80_938_0 [ 29 ] = _rtB -> B_80_707_0 ; _rtB -> B_80_938_0 [ 30 ] = _rtB ->
B_80_730_0 ; _rtB -> B_80_938_0 [ 31 ] = _rtB -> B_80_753_0 ; _rtB ->
B_80_938_0 [ 32 ] = _rtB -> B_80_776_0 ; _rtB -> B_80_938_0 [ 33 ] = _rtB ->
B_80_799_0 ; _rtB -> B_80_938_0 [ 34 ] = _rtB -> B_80_822_0 ; _rtB ->
B_80_938_0 [ 35 ] = _rtB -> B_80_845_0 ; _rtB -> B_80_938_0 [ 36 ] = _rtB ->
B_80_868_0 ; _rtB -> B_80_938_0 [ 37 ] = _rtB -> B_80_891_0 ; _rtB ->
B_80_938_0 [ 38 ] = _rtB -> B_80_914_0 ; _rtB -> B_80_938_0 [ 39 ] = _rtB ->
B_80_937_0 ; ssCallAccelRunBlock ( S , 80 , 939 , SS_CALL_MDL_OUTPUTS ) ; {
real_T * * uBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_ei .
TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_ei . TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S )
; real_T tMinusDelay = simTime - _rtP -> P_640 ; _rtB -> B_80_940_0 =
Simulation_v2_acc_rt_TDelayInterpolate ( tMinusDelay , 0.0 , * tBuffer , *
uBuffer , _rtDW -> TransportDelay_IWORK_cs . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_cs . Last , _rtDW -> TransportDelay_IWORK_cs . Tail ,
_rtDW -> TransportDelay_IWORK_cs . Head , _rtP -> P_641 , 0 , ( boolean_T ) (
ssIsMinorTimeStep ( S ) && ( ssGetTimeOfLastOutput ( S ) == ssGetT ( S ) ) )
) ; } _rtB -> B_80_941_0 = 0.0 ; _rtB -> B_80_941_0 += _rtP -> P_643 * _rtX
-> TransferFcn_CSTATE_a ; _rtB -> B_80_941_0 += _rtP -> P_644 * _rtB ->
B_80_940_0 ; _rtB -> B_80_938_0 [ 0 ] = _rtB -> B_80_47_0 ; _rtB ->
B_80_938_0 [ 1 ] = _rtB -> B_80_70_0 ; _rtB -> B_80_938_0 [ 2 ] = _rtB ->
B_80_93_0 ; _rtB -> B_80_938_0 [ 3 ] = _rtB -> B_80_116_0 ; _rtB ->
B_80_938_0 [ 4 ] = _rtB -> B_80_139_0 ; _rtB -> B_80_938_0 [ 5 ] = _rtB ->
B_80_162_0 ; _rtB -> B_80_938_0 [ 6 ] = _rtB -> B_80_185_0 ; _rtB ->
B_80_938_0 [ 7 ] = _rtB -> B_80_208_0 ; _rtB -> B_80_938_0 [ 8 ] = _rtB ->
B_80_231_0 ; _rtB -> B_80_938_0 [ 9 ] = _rtB -> B_80_254_0 ; _rtB ->
B_80_938_0 [ 10 ] = _rtB -> B_80_277_0 ; _rtB -> B_80_938_0 [ 11 ] = _rtB ->
B_80_300_0 ; _rtB -> B_80_938_0 [ 12 ] = _rtB -> B_80_323_0 ; _rtB ->
B_80_938_0 [ 13 ] = _rtB -> B_80_346_0 ; _rtB -> B_80_938_0 [ 14 ] = _rtB ->
B_80_369_0 ; _rtB -> B_80_938_0 [ 15 ] = _rtB -> B_80_392_0 ; _rtB ->
B_80_938_0 [ 16 ] = _rtB -> B_80_415_0 ; _rtB -> B_80_938_0 [ 17 ] = _rtB ->
B_80_438_0 ; _rtB -> B_80_938_0 [ 18 ] = _rtB -> B_80_461_0 ; _rtB ->
B_80_938_0 [ 19 ] = _rtB -> B_80_484_0 ; _rtB -> B_80_938_0 [ 20 ] = _rtB ->
B_80_507_0 ; _rtB -> B_80_938_0 [ 21 ] = _rtB -> B_80_530_0 ; _rtB ->
B_80_938_0 [ 22 ] = _rtB -> B_80_553_0 ; _rtB -> B_80_938_0 [ 23 ] = _rtB ->
B_80_576_0 ; _rtB -> B_80_938_0 [ 24 ] = _rtB -> B_80_599_0 ; _rtB ->
B_80_938_0 [ 25 ] = _rtB -> B_80_622_0 ; _rtB -> B_80_938_0 [ 26 ] = _rtB ->
B_80_645_0 ; _rtB -> B_80_938_0 [ 27 ] = _rtB -> B_80_668_0 ; _rtB ->
B_80_938_0 [ 28 ] = _rtB -> B_80_691_0 ; _rtB -> B_80_938_0 [ 29 ] = _rtB ->
B_80_714_0 ; _rtB -> B_80_938_0 [ 30 ] = _rtB -> B_80_737_0 ; _rtB ->
B_80_938_0 [ 31 ] = _rtB -> B_80_760_0 ; _rtB -> B_80_938_0 [ 32 ] = _rtB ->
B_80_783_0 ; _rtB -> B_80_938_0 [ 33 ] = _rtB -> B_80_806_0 ; _rtB ->
B_80_938_0 [ 34 ] = _rtB -> B_80_829_0 ; _rtB -> B_80_938_0 [ 35 ] = _rtB ->
B_80_852_0 ; _rtB -> B_80_938_0 [ 36 ] = _rtB -> B_80_875_0 ; _rtB ->
B_80_938_0 [ 37 ] = _rtB -> B_80_898_0 ; _rtB -> B_80_938_0 [ 38 ] = _rtB ->
B_80_921_0 ; if ( _rtB -> B_80_941_0 > _rtP -> P_646 ) { _rtB -> B_80_938_0 [
39 ] = _rtB -> B_80_941_0 - _rtP -> P_646 ; } else if ( _rtB -> B_80_941_0 >=
_rtP -> P_645 ) { _rtB -> B_80_938_0 [ 39 ] = 0.0 ; } else { _rtB ->
B_80_938_0 [ 39 ] = _rtB -> B_80_941_0 - _rtP -> P_645 ; }
ssCallAccelRunBlock ( S , 80 , 944 , SS_CALL_MDL_OUTPUTS ) ; _rtB ->
B_80_945_0 = _rtB -> B_80_40_0 / _rtB -> B_80_22_0 ; _rtB -> B_80_946_0 =
_rtB -> B_80_945_0 + _rtP -> P_647 ; _rtB -> B_80_947_0 = _rtB -> B_80_63_0 /
_rtB -> B_80_42_0 ; _rtB -> B_80_948_0 = _rtB -> B_80_947_0 + _rtP -> P_648 ;
_rtB -> B_80_949_0 = _rtB -> B_80_86_0 / _rtB -> B_80_65_0 ; _rtB ->
B_80_950_0 = _rtB -> B_80_949_0 + _rtP -> P_649 ; _rtB -> B_80_951_0 = _rtB
-> B_80_109_0 / _rtB -> B_80_88_0 ; _rtB -> B_80_952_0 = _rtB -> B_80_951_0 +
_rtP -> P_650 ; _rtB -> B_80_953_0 = _rtB -> B_80_132_0 / _rtB -> B_80_111_0
; _rtB -> B_80_954_0 = _rtB -> B_80_953_0 + _rtP -> P_651 ; _rtB ->
B_80_955_0 = _rtB -> B_80_155_0 / _rtB -> B_80_134_0 ; _rtB -> B_80_956_0 =
_rtB -> B_80_955_0 + _rtP -> P_652 ; _rtB -> B_80_957_0 = _rtB -> B_80_178_0
/ _rtB -> B_80_157_0 ; _rtB -> B_80_958_0 = _rtB -> B_80_957_0 + _rtP ->
P_653 ; _rtB -> B_80_959_0 = _rtB -> B_80_201_0 / _rtB -> B_80_180_0 ; _rtB
-> B_80_960_0 = _rtB -> B_80_959_0 + _rtP -> P_654 ; _rtB -> B_80_961_0 =
_rtB -> B_80_224_0 / _rtB -> B_80_203_0 ; _rtB -> B_80_962_0 = _rtB ->
B_80_961_0 + _rtP -> P_655 ; _rtB -> B_80_963_0 = _rtB -> B_80_247_0 / _rtB
-> B_80_226_0 ; _rtB -> B_80_964_0 = _rtB -> B_80_963_0 + _rtP -> P_656 ;
_rtB -> B_80_965_0 = _rtB -> B_80_270_0 / _rtB -> B_80_249_0 ; _rtB ->
B_80_966_0 = _rtB -> B_80_965_0 + _rtP -> P_657 ; _rtB -> B_80_967_0 = _rtB
-> B_80_293_0 / _rtB -> B_80_272_0 ; _rtB -> B_80_968_0 = _rtB -> B_80_967_0
+ _rtP -> P_658 ; _rtB -> B_80_969_0 = _rtB -> B_80_316_0 / _rtB ->
B_80_295_0 ; _rtB -> B_80_970_0 = _rtB -> B_80_969_0 + _rtP -> P_659 ; _rtB
-> B_80_971_0 = _rtB -> B_80_339_0 / _rtB -> B_80_318_0 ; _rtB -> B_80_972_0
= _rtB -> B_80_971_0 + _rtP -> P_660 ; _rtB -> B_80_973_0 = _rtB ->
B_80_362_0 / _rtB -> B_80_341_0 ; _rtB -> B_80_974_0 = _rtB -> B_80_973_0 +
_rtP -> P_661 ; _rtB -> B_80_975_0 = _rtB -> B_80_385_0 / _rtB -> B_80_364_0
; _rtB -> B_80_976_0 = _rtB -> B_80_975_0 + _rtP -> P_662 ; _rtB ->
B_80_977_0 = _rtB -> B_80_408_0 / _rtB -> B_80_387_0 ; _rtB -> B_80_978_0 =
_rtB -> B_80_977_0 + _rtP -> P_663 ; _rtB -> B_80_979_0 = _rtB -> B_80_431_0
/ _rtB -> B_80_410_0 ; _rtB -> B_80_980_0 = _rtB -> B_80_979_0 + _rtP ->
P_664 ; _rtB -> B_80_981_0 = _rtB -> B_80_454_0 / _rtB -> B_80_433_0 ; _rtB
-> B_80_982_0 = _rtB -> B_80_981_0 + _rtP -> P_665 ; _rtB -> B_80_983_0 =
_rtB -> B_80_477_0 / _rtB -> B_80_456_0 ; _rtB -> B_80_984_0 = _rtB ->
B_80_983_0 + _rtP -> P_666 ; _rtB -> B_80_985_0 = _rtB -> B_80_500_0 / _rtB
-> B_80_479_0 ; _rtB -> B_80_986_0 = _rtB -> B_80_985_0 + _rtP -> P_667 ;
_rtB -> B_80_987_0 = _rtB -> B_80_523_0 / _rtB -> B_80_502_0 ; _rtB ->
B_80_988_0 = _rtB -> B_80_987_0 + _rtP -> P_668 ; _rtB -> B_80_989_0 = _rtB
-> B_80_546_0 / _rtB -> B_80_525_0 ; _rtB -> B_80_990_0 = _rtB -> B_80_989_0
+ _rtP -> P_669 ; _rtB -> B_80_991_0 = _rtB -> B_80_569_0 / _rtB ->
B_80_548_0 ; _rtB -> B_80_992_0 = _rtB -> B_80_991_0 + _rtP -> P_670 ; _rtB
-> B_80_993_0 = _rtB -> B_80_592_0 / _rtB -> B_80_571_0 ; _rtB -> B_80_994_0
= _rtB -> B_80_993_0 + _rtP -> P_671 ; _rtB -> B_80_995_0 = _rtB ->
B_80_615_0 / _rtB -> B_80_594_0 ; _rtB -> B_80_996_0 = _rtB -> B_80_995_0 +
_rtP -> P_672 ; _rtB -> B_80_997_0 = _rtB -> B_80_638_0 / _rtB -> B_80_617_0
; _rtB -> B_80_998_0 = _rtB -> B_80_997_0 + _rtP -> P_673 ; _rtB ->
B_80_999_0 = _rtB -> B_80_661_0 / _rtB -> B_80_640_0 ; _rtB -> B_80_1000_0 =
_rtB -> B_80_999_0 + _rtP -> P_674 ; _rtB -> B_80_1001_0 = _rtB -> B_80_684_0
/ _rtB -> B_80_663_0 ; _rtB -> B_80_1002_0 = _rtB -> B_80_1001_0 + _rtP ->
P_675 ; _rtB -> B_80_1003_0 = _rtB -> B_80_707_0 / _rtB -> B_80_686_0 ; _rtB
-> B_80_1004_0 = _rtB -> B_80_1003_0 + _rtP -> P_676 ; _rtB -> B_80_1005_0 =
_rtB -> B_80_730_0 / _rtB -> B_80_709_0 ; _rtB -> B_80_1006_0 = _rtB ->
B_80_1005_0 + _rtP -> P_677 ; _rtB -> B_80_1007_0 = _rtB -> B_80_753_0 / _rtB
-> B_80_732_0 ; _rtB -> B_80_1008_0 = _rtB -> B_80_1007_0 + _rtP -> P_678 ;
_rtB -> B_80_1009_0 = _rtB -> B_80_776_0 / _rtB -> B_80_755_0 ; _rtB ->
B_80_1010_0 = _rtB -> B_80_1009_0 + _rtP -> P_679 ; _rtB -> B_80_1011_0 =
_rtB -> B_80_799_0 / _rtB -> B_80_778_0 ; _rtB -> B_80_1012_0 = _rtB ->
B_80_1011_0 + _rtP -> P_680 ; _rtB -> B_80_1013_0 = _rtB -> B_80_822_0 / _rtB
-> B_80_801_0 ; _rtB -> B_80_1014_0 = _rtB -> B_80_1013_0 + _rtP -> P_681 ;
_rtB -> B_80_1015_0 = _rtB -> B_80_845_0 / _rtB -> B_80_824_0 ; _rtB ->
B_80_1016_0 = _rtB -> B_80_1015_0 + _rtP -> P_682 ; _rtB -> B_80_1017_0 =
_rtB -> B_80_868_0 / _rtB -> B_80_847_0 ; _rtB -> B_80_1018_0 = _rtB ->
B_80_1017_0 + _rtP -> P_683 ; _rtB -> B_80_1019_0 = _rtB -> B_80_891_0 / _rtB
-> B_80_870_0 ; _rtB -> B_80_1020_0 = _rtB -> B_80_1019_0 + _rtP -> P_684 ;
_rtB -> B_80_1021_0 = _rtB -> B_80_914_0 / _rtB -> B_80_893_0 ; _rtB ->
B_80_1022_0 = _rtB -> B_80_1021_0 + _rtP -> P_685 ; _rtB -> B_80_1023_0 =
_rtB -> B_80_937_0 / _rtB -> B_80_916_0 ; _rtB -> B_80_1024_0 = _rtB ->
B_80_1023_0 + _rtP -> P_686 ; _rtB -> B_80_938_0 [ 0 ] = _rtB -> B_80_946_0 ;
_rtB -> B_80_938_0 [ 1 ] = _rtB -> B_80_948_0 ; _rtB -> B_80_938_0 [ 2 ] =
_rtB -> B_80_950_0 ; _rtB -> B_80_938_0 [ 3 ] = _rtB -> B_80_952_0 ; _rtB ->
B_80_938_0 [ 4 ] = _rtB -> B_80_954_0 ; _rtB -> B_80_938_0 [ 5 ] = _rtB ->
B_80_956_0 ; _rtB -> B_80_938_0 [ 6 ] = _rtB -> B_80_958_0 ; _rtB ->
B_80_938_0 [ 7 ] = _rtB -> B_80_960_0 ; _rtB -> B_80_938_0 [ 8 ] = _rtB ->
B_80_962_0 ; _rtB -> B_80_938_0 [ 9 ] = _rtB -> B_80_964_0 ; _rtB ->
B_80_938_0 [ 10 ] = _rtB -> B_80_966_0 ; _rtB -> B_80_938_0 [ 11 ] = _rtB ->
B_80_968_0 ; _rtB -> B_80_938_0 [ 12 ] = _rtB -> B_80_970_0 ; _rtB ->
B_80_938_0 [ 13 ] = _rtB -> B_80_972_0 ; _rtB -> B_80_938_0 [ 14 ] = _rtB ->
B_80_974_0 ; _rtB -> B_80_938_0 [ 15 ] = _rtB -> B_80_976_0 ; _rtB ->
B_80_938_0 [ 16 ] = _rtB -> B_80_978_0 ; _rtB -> B_80_938_0 [ 17 ] = _rtB ->
B_80_980_0 ; _rtB -> B_80_938_0 [ 18 ] = _rtB -> B_80_982_0 ; _rtB ->
B_80_938_0 [ 19 ] = _rtB -> B_80_984_0 ; _rtB -> B_80_938_0 [ 20 ] = _rtB ->
B_80_986_0 ; _rtB -> B_80_938_0 [ 21 ] = _rtB -> B_80_988_0 ; _rtB ->
B_80_938_0 [ 22 ] = _rtB -> B_80_990_0 ; _rtB -> B_80_938_0 [ 23 ] = _rtB ->
B_80_992_0 ; _rtB -> B_80_938_0 [ 24 ] = _rtB -> B_80_994_0 ; _rtB ->
B_80_938_0 [ 25 ] = _rtB -> B_80_996_0 ; _rtB -> B_80_938_0 [ 26 ] = _rtB ->
B_80_998_0 ; _rtB -> B_80_938_0 [ 27 ] = _rtB -> B_80_1000_0 ; _rtB ->
B_80_938_0 [ 28 ] = _rtB -> B_80_1002_0 ; _rtB -> B_80_938_0 [ 29 ] = _rtB ->
B_80_1004_0 ; _rtB -> B_80_938_0 [ 30 ] = _rtB -> B_80_1006_0 ; _rtB ->
B_80_938_0 [ 31 ] = _rtB -> B_80_1008_0 ; _rtB -> B_80_938_0 [ 32 ] = _rtB ->
B_80_1010_0 ; _rtB -> B_80_938_0 [ 33 ] = _rtB -> B_80_1012_0 ; _rtB ->
B_80_938_0 [ 34 ] = _rtB -> B_80_1014_0 ; _rtB -> B_80_938_0 [ 35 ] = _rtB ->
B_80_1016_0 ; _rtB -> B_80_938_0 [ 36 ] = _rtB -> B_80_1018_0 ; _rtB ->
B_80_938_0 [ 37 ] = _rtB -> B_80_1020_0 ; _rtB -> B_80_938_0 [ 38 ] = _rtB ->
B_80_1022_0 ; _rtB -> B_80_938_0 [ 39 ] = _rtB -> B_80_1024_0 ;
ssCallAccelRunBlock ( S , 80 , 1026 , SS_CALL_MDL_OUTPUTS ) ;
ssCallAccelRunBlock ( S , 80 , 1027 , SS_CALL_MDL_OUTPUTS ) ; _rtB ->
B_80_1028_0 = _rtX -> Integrator1_CSTATE ; ssCallAccelRunBlock ( S , 80 ,
1029 , SS_CALL_MDL_OUTPUTS ) ; ssCallAccelRunBlock ( S , 80 , 1030 ,
SS_CALL_MDL_OUTPUTS ) ; isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0
) { _rtB -> B_80_1031_0 [ 0 ] = _rtB -> B_0_0_0 ; _rtB -> B_80_1031_0 [ 1 ] =
_rtB -> B_2_0_0 ; _rtB -> B_80_1031_0 [ 2 ] = _rtB -> B_4_0_0 ; _rtB ->
B_80_1031_0 [ 3 ] = _rtB -> B_6_0_0 ; _rtB -> B_80_1031_0 [ 4 ] = _rtB ->
B_8_0_0 ; _rtB -> B_80_1031_0 [ 5 ] = _rtB -> B_10_0_0 ; _rtB -> B_80_1031_0
[ 6 ] = _rtB -> B_12_0_0 ; _rtB -> B_80_1031_0 [ 7 ] = _rtB -> B_14_0_0 ;
_rtB -> B_80_1031_0 [ 8 ] = _rtB -> B_16_0_0 ; _rtB -> B_80_1031_0 [ 9 ] =
_rtB -> B_18_0_0 ; _rtB -> B_80_1031_0 [ 10 ] = _rtB -> B_20_0_0 ; _rtB ->
B_80_1031_0 [ 11 ] = _rtB -> B_22_0_0 ; _rtB -> B_80_1031_0 [ 12 ] = _rtB ->
B_24_0_0 ; _rtB -> B_80_1031_0 [ 13 ] = _rtB -> B_26_0_0 ; _rtB ->
B_80_1031_0 [ 14 ] = _rtB -> B_28_0_0 ; _rtB -> B_80_1031_0 [ 15 ] = _rtB ->
B_30_0_0 ; _rtB -> B_80_1031_0 [ 16 ] = _rtB -> B_32_0_0 ; _rtB ->
B_80_1031_0 [ 17 ] = _rtB -> B_34_0_0 ; _rtB -> B_80_1031_0 [ 18 ] = _rtB ->
B_36_0_0 ; _rtB -> B_80_1031_0 [ 19 ] = _rtB -> B_38_0_0 ; _rtB ->
B_80_1031_0 [ 20 ] = _rtB -> B_40_0_0 ; _rtB -> B_80_1031_0 [ 21 ] = _rtB ->
B_42_0_0 ; _rtB -> B_80_1031_0 [ 22 ] = _rtB -> B_44_0_0 ; _rtB ->
B_80_1031_0 [ 23 ] = _rtB -> B_46_0_0 ; _rtB -> B_80_1031_0 [ 24 ] = _rtB ->
B_48_0_0 ; _rtB -> B_80_1031_0 [ 25 ] = _rtB -> B_50_0_0 ; _rtB ->
B_80_1031_0 [ 26 ] = _rtB -> B_52_0_0 ; _rtB -> B_80_1031_0 [ 27 ] = _rtB ->
B_54_0_0 ; _rtB -> B_80_1031_0 [ 28 ] = _rtB -> B_56_0_0 ; _rtB ->
B_80_1031_0 [ 29 ] = _rtB -> B_58_0_0 ; _rtB -> B_80_1031_0 [ 30 ] = _rtB ->
B_60_0_0 ; _rtB -> B_80_1031_0 [ 31 ] = _rtB -> B_62_0_0 ; _rtB ->
B_80_1031_0 [ 32 ] = _rtB -> B_64_0_0 ; _rtB -> B_80_1031_0 [ 33 ] = _rtB ->
B_66_0_0 ; _rtB -> B_80_1031_0 [ 34 ] = _rtB -> B_68_0_0 ; _rtB ->
B_80_1031_0 [ 35 ] = _rtB -> B_70_0_0 ; _rtB -> B_80_1031_0 [ 36 ] = _rtB ->
B_72_0_0 ; _rtB -> B_80_1031_0 [ 37 ] = _rtB -> B_74_0_0 ; _rtB ->
B_80_1031_0 [ 38 ] = _rtB -> B_76_0_0 ; _rtB -> B_80_1031_0 [ 39 ] = _rtB ->
B_78_0_0 ; ssCallAccelRunBlock ( S , 80 , 1032 , SS_CALL_MDL_OUTPUTS ) ; }
_rtB -> B_80_1034_0 = ( ( ( _rtB -> B_80_40_0 + _rtB -> B_80_63_0 ) + _rtB ->
B_80_86_0 ) + _rtB -> B_80_109_0 ) + _rtB -> B_80_132_0 ; _rtB -> B_80_1035_0
= ( ( ( _rtB -> B_80_155_0 + _rtB -> B_80_178_0 ) + _rtB -> B_80_201_0 ) +
_rtB -> B_80_224_0 ) + _rtB -> B_80_247_0 ; _rtB -> B_80_1036_0 = ( ( ( _rtB
-> B_80_270_0 + _rtB -> B_80_293_0 ) + _rtB -> B_80_316_0 ) + _rtB ->
B_80_339_0 ) + _rtB -> B_80_362_0 ; _rtB -> B_80_1037_0 = ( ( ( _rtB ->
B_80_385_0 + _rtB -> B_80_408_0 ) + _rtB -> B_80_431_0 ) + _rtB -> B_80_454_0
) + _rtB -> B_80_477_0 ; _rtB -> B_80_1038_0 = ( ( ( _rtB -> B_80_500_0 +
_rtB -> B_80_523_0 ) + _rtB -> B_80_546_0 ) + _rtB -> B_80_569_0 ) + _rtB ->
B_80_592_0 ; _rtB -> B_80_1039_0 = ( ( ( _rtB -> B_80_615_0 + _rtB ->
B_80_638_0 ) + _rtB -> B_80_661_0 ) + _rtB -> B_80_684_0 ) + _rtB ->
B_80_707_0 ; _rtB -> B_80_1040_0 = ( ( ( _rtB -> B_80_730_0 + _rtB ->
B_80_753_0 ) + _rtB -> B_80_776_0 ) + _rtB -> B_80_799_0 ) + _rtB ->
B_80_822_0 ; _rtB -> B_80_1041_0 = ( ( ( _rtB -> B_80_845_0 + _rtB ->
B_80_868_0 ) + _rtB -> B_80_891_0 ) + _rtB -> B_80_914_0 ) + _rtB ->
B_80_937_0 ; _rtB -> B_80_1042_0 = ( ( ( ( ( ( _rtB -> B_80_1034_0 + _rtB ->
B_80_1035_0 ) + _rtB -> B_80_1036_0 ) + _rtB -> B_80_1037_0 ) + _rtB ->
B_80_1038_0 ) + _rtB -> B_80_1039_0 ) + _rtB -> B_80_1040_0 ) + _rtB ->
B_80_1041_0 ; if ( _rtB -> B_80_1042_0 > _rtP -> P_691 ) { _rtB ->
B_80_1043_0 = _rtB -> B_80_1042_0 - _rtP -> P_691 ; } else if ( _rtB ->
B_80_1042_0 >= _rtP -> P_690 ) { _rtB -> B_80_1043_0 = 0.0 ; } else { _rtB ->
B_80_1043_0 = _rtB -> B_80_1042_0 - _rtP -> P_690 ; } _rtB -> B_80_1045_0 = (
real_T ) ( _rtB -> B_80_1043_0 == Simulation_v2_rtC ( S ) -> B_80_1_0 ) *
look1_binlxpw ( _rtB -> B_80_20_0 , _rtP -> P_689 , _rtP -> P_688 , 7U ) ;
_rtB -> B_80_1047_0 = ( real_T ) ( _rtB -> B_80_1043_0 != Simulation_v2_rtC (
S ) -> B_80_0_0 ) * _rtB -> B_80_1043_0 ; _rtB -> B_80_1048_0 = _rtB ->
B_80_1045_0 + _rtB -> B_80_1047_0 ; ssCallAccelRunBlock ( S , 80 , 1049 ,
SS_CALL_MDL_OUTPUTS ) ; isHit = ssIsSampleHit ( S , 2 , 0 ) ; if ( isHit != 0
) { _rtB -> B_80_1058_0 = ( ( ( ( ( ( ( ( ( ( _rtB -> B_80_22_0 + _rtB ->
B_80_42_0 ) + _rtB -> B_80_65_0 ) + _rtB -> B_80_88_0 ) + _rtB -> B_80_111_0
) + ( ( ( ( _rtB -> B_80_134_0 + _rtB -> B_80_157_0 ) + _rtB -> B_80_180_0 )
+ _rtB -> B_80_203_0 ) + _rtB -> B_80_226_0 ) ) + ( ( ( ( _rtB -> B_80_249_0
+ _rtB -> B_80_272_0 ) + _rtB -> B_80_295_0 ) + _rtB -> B_80_318_0 ) + _rtB
-> B_80_341_0 ) ) + ( ( ( ( _rtB -> B_80_364_0 + _rtB -> B_80_387_0 ) + _rtB
-> B_80_410_0 ) + _rtB -> B_80_433_0 ) + _rtB -> B_80_456_0 ) ) + ( ( ( (
_rtB -> B_80_479_0 + _rtB -> B_80_502_0 ) + _rtB -> B_80_525_0 ) + _rtB ->
B_80_548_0 ) + _rtB -> B_80_571_0 ) ) + ( ( ( ( _rtB -> B_80_594_0 + _rtB ->
B_80_617_0 ) + _rtB -> B_80_640_0 ) + _rtB -> B_80_663_0 ) + _rtB ->
B_80_686_0 ) ) + ( ( ( ( _rtB -> B_80_709_0 + _rtB -> B_80_732_0 ) + _rtB ->
B_80_755_0 ) + _rtB -> B_80_778_0 ) + _rtB -> B_80_801_0 ) ) + ( ( ( ( _rtB
-> B_80_824_0 + _rtB -> B_80_847_0 ) + _rtB -> B_80_870_0 ) + _rtB ->
B_80_893_0 ) + _rtB -> B_80_916_0 ) ; } _rtB -> B_80_1059_0 = _rtB ->
B_80_1048_0 / _rtB -> B_80_1058_0 ; ssCallAccelRunBlock ( S , 80 , 1060 ,
SS_CALL_MDL_OUTPUTS ) ; ssCallAccelRunBlock ( S , 80 , 1061 ,
SS_CALL_MDL_OUTPUTS ) ; UNUSED_PARAMETER ( tid ) ; } static void
mdlOutputsTID3 ( SimStruct * S , int_T tid ) { B_Simulation_v2_T * _rtB ;
P_Simulation_v2_T * _rtP ; _rtP = ( ( P_Simulation_v2_T * ) ssGetModelRtp ( S
) ) ; _rtB = ( ( B_Simulation_v2_T * ) _ssGetModelBlockIO ( S ) ) ; _rtB ->
B_80_0_0_m = _rtP -> P_693 ; _rtB -> B_80_2_0 = _rtP -> P_695 ; _rtB ->
B_80_3_0 = _rtP -> P_696 ; _rtB -> B_80_4_0 = _rtP -> P_697 ; _rtB ->
B_80_6_0 = _rtP -> P_699 ; _rtB -> B_80_7_0_k = _rtP -> P_700 ; _rtB ->
B_80_8_0 = _rtP -> P_701 ; _rtB -> B_80_10_0_c = _rtP -> P_703 ; _rtB ->
B_80_11_0_b = _rtP -> P_704 ; _rtB -> B_80_12_0_p = _rtP -> P_705 ; _rtB ->
B_80_14_0_c = _rtP -> P_707 ; _rtB -> B_80_15_0 = _rtP -> P_708 ; _rtB ->
B_80_16_0 = _rtP -> P_709 ; _rtB -> B_80_18_0_g = _rtP -> P_711 ; _rtB ->
B_80_19_0_g = _rtP -> P_712 ; _rtB -> B_80_20_0_m = _rtP -> P_713 ; _rtB ->
B_80_22_0_n = _rtP -> P_715 ; _rtB -> B_80_23_0_p = _rtP -> P_716 ; _rtB ->
B_80_24_0_l = _rtP -> P_717 ; _rtB -> B_80_26_0_d = _rtP -> P_719 ; _rtB ->
B_80_27_0_g = _rtP -> P_720 ; _rtB -> B_80_28_0_l = _rtP -> P_721 ; _rtB ->
B_80_30_0_d = _rtP -> P_723 ; _rtB -> B_80_31_0_l = _rtP -> P_724 ; _rtB ->
B_80_32_0_o = _rtP -> P_725 ; _rtB -> B_80_34_0_n = _rtP -> P_727 ; _rtB ->
B_80_35_0_b = _rtP -> P_728 ; _rtB -> B_80_36_0_l = _rtP -> P_729 ; _rtB ->
B_80_38_0_h = _rtP -> P_731 ; _rtB -> B_80_39_0 = _rtP -> P_732 ; _rtB ->
B_80_40_0_b = _rtP -> P_733 ; _rtB -> B_80_42_0_d = _rtP -> P_735 ; _rtB ->
B_80_43_0_e = _rtP -> P_736 ; _rtB -> B_80_44_0_b = _rtP -> P_737 ; _rtB ->
B_80_46_0_f = _rtP -> P_739 ; _rtB -> B_80_47_0_a = _rtP -> P_740 ; _rtB ->
B_80_48_0_j = _rtP -> P_741 ; _rtB -> B_80_50_0_o = _rtP -> P_743 ; _rtB ->
B_80_51_0_n = _rtP -> P_744 ; _rtB -> B_80_52_0_i = _rtP -> P_745 ; _rtB ->
B_80_54_0_n = _rtP -> P_747 ; _rtB -> B_80_55_0_m = _rtP -> P_748 ; _rtB ->
B_80_56_0_c = _rtP -> P_749 ; _rtB -> B_80_58_0_m = _rtP -> P_751 ; _rtB ->
B_80_59_0_j = _rtP -> P_752 ; _rtB -> B_80_60_0 = _rtP -> P_753 ; _rtB ->
B_80_62_0 = _rtP -> P_755 ; _rtB -> B_80_63_0_c = _rtP -> P_756 ; _rtB ->
B_80_64_0 = _rtP -> P_757 ; _rtB -> B_80_66_0_p = _rtP -> P_759 ; _rtB ->
B_80_67_0_p = _rtP -> P_760 ; _rtB -> B_80_68_0_a = _rtP -> P_761 ; _rtB ->
B_80_70_0_a = _rtP -> P_763 ; _rtB -> B_80_71_0_a = _rtP -> P_764 ; _rtB ->
B_80_72_0_i = _rtP -> P_765 ; _rtB -> B_80_74_0_o = _rtP -> P_767 ; _rtB ->
B_80_75_0_o = _rtP -> P_768 ; _rtB -> B_80_76_0_i = _rtP -> P_769 ; _rtB ->
B_80_78_0_i = _rtP -> P_771 ; _rtB -> B_80_79_0_f = _rtP -> P_772 ; _rtB ->
B_80_80_0_g = _rtP -> P_773 ; _rtB -> B_80_82_0_o = _rtP -> P_775 ; _rtB ->
B_80_83_0 = _rtP -> P_776 ; _rtB -> B_80_84_0_l = _rtP -> P_777 ; _rtB ->
B_80_86_0_m = _rtP -> P_779 ; _rtB -> B_80_87_0 = _rtP -> P_780 ; _rtB ->
B_80_88_0_m = _rtP -> P_781 ; _rtB -> B_80_90_0_f = _rtP -> P_783 ; _rtB ->
B_80_91_0_p = _rtP -> P_784 ; _rtB -> B_80_92_0_e = _rtP -> P_785 ; _rtB ->
B_80_94_0_h = _rtP -> P_787 ; _rtB -> B_80_95_0_l = _rtP -> P_788 ; _rtB ->
B_80_96_0_h = _rtP -> P_789 ; _rtB -> B_80_98_0_m = _rtP -> P_791 ; _rtB ->
B_80_99_0_h = _rtP -> P_792 ; _rtB -> B_80_100_0_c = _rtP -> P_793 ; _rtB ->
B_80_102_0_p = _rtP -> P_795 ; _rtB -> B_80_103_0_p = _rtP -> P_796 ; _rtB ->
B_80_104_0_p = _rtP -> P_797 ; _rtB -> B_80_106_0 = _rtP -> P_799 ; _rtB ->
B_80_107_0_j = _rtP -> P_800 ; _rtB -> B_80_108_0 = _rtP -> P_801 ; _rtB ->
B_80_110_0 = _rtP -> P_803 ; _rtB -> B_80_111_0_o = _rtP -> P_804 ; _rtB ->
B_80_112_0_b = _rtP -> P_805 ; _rtB -> B_80_114_0_g = _rtP -> P_807 ; _rtB ->
B_80_115_0_e = _rtP -> P_808 ; _rtB -> B_80_116_0_f = _rtP -> P_809 ; _rtB ->
B_80_118_0_e = _rtP -> P_811 ; _rtB -> B_80_119_0_c = _rtP -> P_812 ; _rtB ->
B_80_120_0_a = _rtP -> P_813 ; _rtB -> B_80_122_0_a = _rtP -> P_815 ; _rtB ->
B_80_123_0_p = _rtP -> P_816 ; _rtB -> B_80_124_0_m = _rtP -> P_817 ; _rtB ->
B_80_126_0_n = _rtP -> P_819 ; _rtB -> B_80_127_0_l = _rtP -> P_820 ; _rtB ->
B_80_128_0_p = _rtP -> P_821 ; _rtB -> B_80_130_0_p = _rtP -> P_823 ; _rtB ->
B_80_131_0 = _rtP -> P_824 ; _rtB -> B_80_132_0_f = _rtP -> P_825 ; _rtB ->
B_80_134_0_i = _rtP -> P_827 ; _rtB -> B_80_135_0_o = _rtP -> P_828 ; _rtB ->
B_80_136_0_k = _rtP -> P_829 ; _rtB -> B_80_138_0_o = _rtP -> P_831 ; _rtB ->
B_80_139_0_m = _rtP -> P_832 ; _rtB -> B_80_140_0_c = _rtP -> P_833 ; _rtB ->
B_80_142_0_h = _rtP -> P_835 ; _rtB -> B_80_143_0_m = _rtP -> P_836 ; _rtB ->
B_80_144_0_a = _rtP -> P_837 ; _rtB -> B_80_146_0_p = _rtP -> P_839 ; _rtB ->
B_80_147_0_b = _rtP -> P_840 ; _rtB -> B_80_148_0_c = _rtP -> P_841 ; _rtB ->
B_80_150_0_i = _rtP -> P_843 ; _rtB -> B_80_151_0_m = _rtP -> P_844 ; _rtB ->
B_80_152_0 = _rtP -> P_845 ; _rtB -> B_80_154_0 = _rtP -> P_847 ; _rtB ->
B_80_155_0_e = _rtP -> P_848 ; _rtB -> B_80_156_0 = _rtP -> P_849 ; memcpy (
& _rtB -> B_80_1_0_c [ 0 ] , & _rtP -> P_694 [ 0 ] , 1500U * sizeof ( real_T
) ) ; memcpy ( & _rtB -> B_80_5_0 [ 0 ] , & _rtP -> P_698 [ 0 ] , 1500U *
sizeof ( real_T ) ) ; memcpy ( & _rtB -> B_80_9_0 [ 0 ] , & _rtP -> P_702 [ 0
] , 1500U * sizeof ( real_T ) ) ; memcpy ( & _rtB -> B_80_13_0 [ 0 ] , & _rtP
-> P_706 [ 0 ] , 1500U * sizeof ( real_T ) ) ; memcpy ( & _rtB -> B_80_17_0_f
[ 0 ] , & _rtP -> P_710 [ 0 ] , 1500U * sizeof ( real_T ) ) ; memcpy ( & _rtB
-> B_80_21_0 [ 0 ] , & _rtP -> P_714 [ 0 ] , 1500U * sizeof ( real_T ) ) ;
memcpy ( & _rtB -> B_80_25_0_j [ 0 ] , & _rtP -> P_718 [ 0 ] , 1500U * sizeof
( real_T ) ) ; memcpy ( & _rtB -> B_80_29_0_d [ 0 ] , & _rtP -> P_722 [ 0 ] ,
1500U * sizeof ( real_T ) ) ; memcpy ( & _rtB -> B_80_33_0_b [ 0 ] , & _rtP
-> P_726 [ 0 ] , 1500U * sizeof ( real_T ) ) ; memcpy ( & _rtB -> B_80_37_0 [
0 ] , & _rtP -> P_730 [ 0 ] , 1500U * sizeof ( real_T ) ) ; memcpy ( & _rtB
-> B_80_41_0 [ 0 ] , & _rtP -> P_734 [ 0 ] , 1500U * sizeof ( real_T ) ) ;
memcpy ( & _rtB -> B_80_45_0_j [ 0 ] , & _rtP -> P_738 [ 0 ] , 1500U * sizeof
( real_T ) ) ; memcpy ( & _rtB -> B_80_49_0_j [ 0 ] , & _rtP -> P_742 [ 0 ] ,
1500U * sizeof ( real_T ) ) ; memcpy ( & _rtB -> B_80_53_0_o [ 0 ] , & _rtP
-> P_746 [ 0 ] , 1500U * sizeof ( real_T ) ) ; memcpy ( & _rtB -> B_80_57_0_m
[ 0 ] , & _rtP -> P_750 [ 0 ] , 1500U * sizeof ( real_T ) ) ; memcpy ( & _rtB
-> B_80_61_0_h [ 0 ] , & _rtP -> P_754 [ 0 ] , 1500U * sizeof ( real_T ) ) ;
memcpy ( & _rtB -> B_80_65_0_c [ 0 ] , & _rtP -> P_758 [ 0 ] , 1500U * sizeof
( real_T ) ) ; memcpy ( & _rtB -> B_80_69_0_e [ 0 ] , & _rtP -> P_762 [ 0 ] ,
1500U * sizeof ( real_T ) ) ; memcpy ( & _rtB -> B_80_73_0_l [ 0 ] , & _rtP
-> P_766 [ 0 ] , 1500U * sizeof ( real_T ) ) ; memcpy ( & _rtB -> B_80_77_0_f
[ 0 ] , & _rtP -> P_770 [ 0 ] , 1500U * sizeof ( real_T ) ) ; memcpy ( & _rtB
-> B_80_81_0_c [ 0 ] , & _rtP -> P_774 [ 0 ] , 1500U * sizeof ( real_T ) ) ;
memcpy ( & _rtB -> B_80_85_0 [ 0 ] , & _rtP -> P_778 [ 0 ] , 1500U * sizeof (
real_T ) ) ; memcpy ( & _rtB -> B_80_89_0_c [ 0 ] , & _rtP -> P_782 [ 0 ] ,
1500U * sizeof ( real_T ) ) ; memcpy ( & _rtB -> B_80_93_0_o [ 0 ] , & _rtP
-> P_786 [ 0 ] , 1500U * sizeof ( real_T ) ) ; memcpy ( & _rtB -> B_80_97_0_m
[ 0 ] , & _rtP -> P_790 [ 0 ] , 1500U * sizeof ( real_T ) ) ; memcpy ( & _rtB
-> B_80_101_0_k [ 0 ] , & _rtP -> P_794 [ 0 ] , 1500U * sizeof ( real_T ) ) ;
memcpy ( & _rtB -> B_80_105_0_a [ 0 ] , & _rtP -> P_798 [ 0 ] , 1500U *
sizeof ( real_T ) ) ; memcpy ( & _rtB -> B_80_109_0_e [ 0 ] , & _rtP -> P_802
[ 0 ] , 1500U * sizeof ( real_T ) ) ; memcpy ( & _rtB -> B_80_113_0_a [ 0 ] ,
& _rtP -> P_806 [ 0 ] , 1500U * sizeof ( real_T ) ) ; memcpy ( & _rtB ->
B_80_117_0_h [ 0 ] , & _rtP -> P_810 [ 0 ] , 1500U * sizeof ( real_T ) ) ;
memcpy ( & _rtB -> B_80_121_0_d [ 0 ] , & _rtP -> P_814 [ 0 ] , 1500U *
sizeof ( real_T ) ) ; memcpy ( & _rtB -> B_80_125_0_o [ 0 ] , & _rtP -> P_818
[ 0 ] , 1500U * sizeof ( real_T ) ) ; memcpy ( & _rtB -> B_80_129_0 [ 0 ] , &
_rtP -> P_822 [ 0 ] , 1500U * sizeof ( real_T ) ) ; memcpy ( & _rtB ->
B_80_133_0 [ 0 ] , & _rtP -> P_826 [ 0 ] , 1500U * sizeof ( real_T ) ) ;
memcpy ( & _rtB -> B_80_137_0_i [ 0 ] , & _rtP -> P_830 [ 0 ] , 1500U *
sizeof ( real_T ) ) ; memcpy ( & _rtB -> B_80_141_0_f [ 0 ] , & _rtP -> P_834
[ 0 ] , 1500U * sizeof ( real_T ) ) ; memcpy ( & _rtB -> B_80_145_0_k [ 0 ] ,
& _rtP -> P_838 [ 0 ] , 1500U * sizeof ( real_T ) ) ; memcpy ( & _rtB ->
B_80_149_0_n [ 0 ] , & _rtP -> P_842 [ 0 ] , 1500U * sizeof ( real_T ) ) ;
memcpy ( & _rtB -> B_80_153_0_j [ 0 ] , & _rtP -> P_846 [ 0 ] , 1500U *
sizeof ( real_T ) ) ; memcpy ( & _rtB -> B_80_157_0_m [ 0 ] , & _rtP -> P_850
[ 0 ] , 1500U * sizeof ( real_T ) ) ; _rtB -> B_80_158_0_m = _rtP -> P_851 ;
_rtB -> B_80_159_0_j = _rtP -> P_852 ; _rtB -> B_80_160_0_f = _rtP -> P_853 ;
UNUSED_PARAMETER ( tid ) ; }
#define MDL_UPDATE
static void mdlUpdate ( SimStruct * S , int_T tid ) { int32_T isHit ;
B_Simulation_v2_T * _rtB ; P_Simulation_v2_T * _rtP ; DW_Simulation_v2_T *
_rtDW ; _rtDW = ( ( DW_Simulation_v2_T * ) ssGetRootDWork ( S ) ) ; _rtP = (
( P_Simulation_v2_T * ) ssGetModelRtp ( S ) ) ; _rtB = ( ( B_Simulation_v2_T
* ) _ssGetModelBlockIO ( S ) ) ; isHit = ssIsSampleHit ( S , 2 , 0 ) ; if (
isHit != 0 ) { _rtDW -> DelayInput1_DSTATE = _rtB -> B_80_0_0 ; } _rtDW ->
Integrator_IWORK = 0 ; if ( _rtDW -> LastMajorTimeA == ( rtInf ) ) { _rtDW ->
LastMajorTimeA = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA = _rtB ->
B_80_27_0 ; } else if ( _rtDW -> LastMajorTimeB == ( rtInf ) ) { _rtDW ->
LastMajorTimeB = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB = _rtB ->
B_80_27_0 ; } else if ( _rtDW -> LastMajorTimeA < _rtDW -> LastMajorTimeB ) {
_rtDW -> LastMajorTimeA = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA = _rtB ->
B_80_27_0 ; } else { _rtDW -> LastMajorTimeB = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYB = _rtB -> B_80_27_0 ; } _rtDW -> vv0at_IWORK = 0 ; { real_T *
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
B_80_19_0 ; } if ( _rtDW -> LastMajorTimeA_f == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_f = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_l = _rtB ->
B_80_50_0 ; } else if ( _rtDW -> LastMajorTimeB_m == ( rtInf ) ) { _rtDW ->
LastMajorTimeB_m = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_m = _rtB ->
B_80_50_0 ; } else if ( _rtDW -> LastMajorTimeA_f < _rtDW -> LastMajorTimeB_m
) { _rtDW -> LastMajorTimeA_f = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_l =
_rtB -> B_80_50_0 ; } else { _rtDW -> LastMajorTimeB_m = ssGetTaskTime ( S ,
0 ) ; _rtDW -> PrevYB_m = _rtB -> B_80_50_0 ; } _rtDW -> vv0at_IWORK_b = 0 ;
{ real_T * * uBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_o .
TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_o . TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ;
_rtDW -> TransportDelay_IWORK_c . Head = ( ( _rtDW -> TransportDelay_IWORK_c
. Head < ( _rtDW -> TransportDelay_IWORK_c . CircularBufSize - 1 ) ) ? (
_rtDW -> TransportDelay_IWORK_c . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_c . Head == _rtDW -> TransportDelay_IWORK_c . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_c . CircularBufSize , & _rtDW -> TransportDelay_IWORK_c
. Tail , & _rtDW -> TransportDelay_IWORK_c . Head , & _rtDW ->
TransportDelay_IWORK_c . Last , simTime - _rtP -> P_32 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_c .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_c . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_c . Head ] = _rtB ->
B_80_47_0 ; } if ( _rtDW -> LastMajorTimeA_o == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_o = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_d = _rtB ->
B_80_73_0 ; } else if ( _rtDW -> LastMajorTimeB_o == ( rtInf ) ) { _rtDW ->
LastMajorTimeB_o = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_j = _rtB ->
B_80_73_0 ; } else if ( _rtDW -> LastMajorTimeA_o < _rtDW -> LastMajorTimeB_o
) { _rtDW -> LastMajorTimeA_o = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_d =
_rtB -> B_80_73_0 ; } else { _rtDW -> LastMajorTimeB_o = ssGetTaskTime ( S ,
0 ) ; _rtDW -> PrevYB_j = _rtB -> B_80_73_0 ; } _rtDW -> vv0at_IWORK_h = 0 ;
{ real_T * * uBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_b .
TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_b . TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ;
_rtDW -> TransportDelay_IWORK_f . Head = ( ( _rtDW -> TransportDelay_IWORK_f
. Head < ( _rtDW -> TransportDelay_IWORK_f . CircularBufSize - 1 ) ) ? (
_rtDW -> TransportDelay_IWORK_f . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_f . Head == _rtDW -> TransportDelay_IWORK_f . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_f . CircularBufSize , & _rtDW -> TransportDelay_IWORK_f
. Tail , & _rtDW -> TransportDelay_IWORK_f . Head , & _rtDW ->
TransportDelay_IWORK_f . Last , simTime - _rtP -> P_48 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_f .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_f . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_f . Head ] = _rtB ->
B_80_70_0 ; } if ( _rtDW -> LastMajorTimeA_e == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_e = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_k = _rtB ->
B_80_96_0 ; } else if ( _rtDW -> LastMajorTimeB_d == ( rtInf ) ) { _rtDW ->
LastMajorTimeB_d = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_g = _rtB ->
B_80_96_0 ; } else if ( _rtDW -> LastMajorTimeA_e < _rtDW -> LastMajorTimeB_d
) { _rtDW -> LastMajorTimeA_e = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_k =
_rtB -> B_80_96_0 ; } else { _rtDW -> LastMajorTimeB_d = ssGetTaskTime ( S ,
0 ) ; _rtDW -> PrevYB_g = _rtB -> B_80_96_0 ; } _rtDW -> vv0at_IWORK_c = 0 ;
{ real_T * * uBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_c .
TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_c . TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ;
_rtDW -> TransportDelay_IWORK_h . Head = ( ( _rtDW -> TransportDelay_IWORK_h
. Head < ( _rtDW -> TransportDelay_IWORK_h . CircularBufSize - 1 ) ) ? (
_rtDW -> TransportDelay_IWORK_h . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_h . Head == _rtDW -> TransportDelay_IWORK_h . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_h . CircularBufSize , & _rtDW -> TransportDelay_IWORK_h
. Tail , & _rtDW -> TransportDelay_IWORK_h . Head , & _rtDW ->
TransportDelay_IWORK_h . Last , simTime - _rtP -> P_64 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_h .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_h . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_h . Head ] = _rtB ->
B_80_93_0 ; } if ( _rtDW -> LastMajorTimeA_b == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_b = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_p = _rtB ->
B_80_119_0 ; } else if ( _rtDW -> LastMajorTimeB_k == ( rtInf ) ) { _rtDW ->
LastMajorTimeB_k = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_ms = _rtB ->
B_80_119_0 ; } else if ( _rtDW -> LastMajorTimeA_b < _rtDW ->
LastMajorTimeB_k ) { _rtDW -> LastMajorTimeA_b = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYA_p = _rtB -> B_80_119_0 ; } else { _rtDW -> LastMajorTimeB_k =
ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_ms = _rtB -> B_80_119_0 ; } _rtDW
-> vv0at_IWORK_cz = 0 ; { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_n . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T *
* ) & _rtDW -> TransportDelay_PWORK_n . TUbufferPtrs [ 1 ] ; real_T simTime =
ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_hd . Head = ( ( _rtDW ->
TransportDelay_IWORK_hd . Head < ( _rtDW -> TransportDelay_IWORK_hd .
CircularBufSize - 1 ) ) ? ( _rtDW -> TransportDelay_IWORK_hd . Head + 1 ) : 0
) ; if ( _rtDW -> TransportDelay_IWORK_hd . Head == _rtDW ->
TransportDelay_IWORK_hd . Tail ) { if ( !
Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_hd . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_hd . Tail , & _rtDW -> TransportDelay_IWORK_hd . Head ,
& _rtDW -> TransportDelay_IWORK_hd . Last , simTime - _rtP -> P_80 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_hd . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_hd . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_hd . Head ] = _rtB -> B_80_116_0 ; } if ( _rtDW ->
LastMajorTimeA_os == ( rtInf ) ) { _rtDW -> LastMajorTimeA_os = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_n = _rtB -> B_80_142_0 ; } else if ( _rtDW ->
LastMajorTimeB_g == ( rtInf ) ) { _rtDW -> LastMajorTimeB_g = ssGetTaskTime (
S , 0 ) ; _rtDW -> PrevYB_f = _rtB -> B_80_142_0 ; } else if ( _rtDW ->
LastMajorTimeA_os < _rtDW -> LastMajorTimeB_g ) { _rtDW -> LastMajorTimeA_os
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_n = _rtB -> B_80_142_0 ; } else {
_rtDW -> LastMajorTimeB_g = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_f =
_rtB -> B_80_142_0 ; } _rtDW -> vv0at_IWORK_o = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_k . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_k . TUbufferPtrs
[ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_d .
Head = ( ( _rtDW -> TransportDelay_IWORK_d . Head < ( _rtDW ->
TransportDelay_IWORK_d . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_d . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_d . Head == _rtDW -> TransportDelay_IWORK_d . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_d . CircularBufSize , & _rtDW -> TransportDelay_IWORK_d
. Tail , & _rtDW -> TransportDelay_IWORK_d . Head , & _rtDW ->
TransportDelay_IWORK_d . Last , simTime - _rtP -> P_96 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_d .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_d . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_d . Head ] = _rtB ->
B_80_139_0 ; } if ( _rtDW -> LastMajorTimeA_n == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_n = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_j = _rtB ->
B_80_165_0 ; } else if ( _rtDW -> LastMajorTimeB_i == ( rtInf ) ) { _rtDW ->
LastMajorTimeB_i = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_o = _rtB ->
B_80_165_0 ; } else if ( _rtDW -> LastMajorTimeA_n < _rtDW ->
LastMajorTimeB_i ) { _rtDW -> LastMajorTimeA_n = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYA_j = _rtB -> B_80_165_0 ; } else { _rtDW -> LastMajorTimeB_i =
ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_o = _rtB -> B_80_165_0 ; } _rtDW ->
vv0at_IWORK_l = 0 ; { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_ov . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T
* * ) & _rtDW -> TransportDelay_PWORK_ov . TUbufferPtrs [ 1 ] ; real_T
simTime = ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_a . Head = ( ( _rtDW
-> TransportDelay_IWORK_a . Head < ( _rtDW -> TransportDelay_IWORK_a .
CircularBufSize - 1 ) ) ? ( _rtDW -> TransportDelay_IWORK_a . Head + 1 ) : 0
) ; if ( _rtDW -> TransportDelay_IWORK_a . Head == _rtDW ->
TransportDelay_IWORK_a . Tail ) { if ( !
Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_a . CircularBufSize , & _rtDW -> TransportDelay_IWORK_a
. Tail , & _rtDW -> TransportDelay_IWORK_a . Head , & _rtDW ->
TransportDelay_IWORK_a . Last , simTime - _rtP -> P_112 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_a .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_a . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_a . Head ] = _rtB ->
B_80_162_0 ; } if ( _rtDW -> LastMajorTimeA_i == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_i = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_o = _rtB ->
B_80_188_0 ; } else if ( _rtDW -> LastMajorTimeB_c == ( rtInf ) ) { _rtDW ->
LastMajorTimeB_c = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_on = _rtB ->
B_80_188_0 ; } else if ( _rtDW -> LastMajorTimeA_i < _rtDW ->
LastMajorTimeB_c ) { _rtDW -> LastMajorTimeA_i = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYA_o = _rtB -> B_80_188_0 ; } else { _rtDW -> LastMajorTimeB_c =
ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_on = _rtB -> B_80_188_0 ; } _rtDW
-> vv0at_IWORK_j = 0 ; { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_l . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T *
* ) & _rtDW -> TransportDelay_PWORK_l . TUbufferPtrs [ 1 ] ; real_T simTime =
ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_l . Head = ( ( _rtDW ->
TransportDelay_IWORK_l . Head < ( _rtDW -> TransportDelay_IWORK_l .
CircularBufSize - 1 ) ) ? ( _rtDW -> TransportDelay_IWORK_l . Head + 1 ) : 0
) ; if ( _rtDW -> TransportDelay_IWORK_l . Head == _rtDW ->
TransportDelay_IWORK_l . Tail ) { if ( !
Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_l . CircularBufSize , & _rtDW -> TransportDelay_IWORK_l
. Tail , & _rtDW -> TransportDelay_IWORK_l . Head , & _rtDW ->
TransportDelay_IWORK_l . Last , simTime - _rtP -> P_128 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_l .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_l . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_l . Head ] = _rtB ->
B_80_185_0 ; } if ( _rtDW -> LastMajorTimeA_bs == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_bs = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_dx = _rtB ->
B_80_211_0 ; } else if ( _rtDW -> LastMajorTimeB_f == ( rtInf ) ) { _rtDW ->
LastMajorTimeB_f = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_mc = _rtB ->
B_80_211_0 ; } else if ( _rtDW -> LastMajorTimeA_bs < _rtDW ->
LastMajorTimeB_f ) { _rtDW -> LastMajorTimeA_bs = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYA_dx = _rtB -> B_80_211_0 ; } else { _rtDW -> LastMajorTimeB_f
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_mc = _rtB -> B_80_211_0 ; } _rtDW
-> vv0at_IWORK_k = 0 ; { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_e . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T *
* ) & _rtDW -> TransportDelay_PWORK_e . TUbufferPtrs [ 1 ] ; real_T simTime =
ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_i . Head = ( ( _rtDW ->
TransportDelay_IWORK_i . Head < ( _rtDW -> TransportDelay_IWORK_i .
CircularBufSize - 1 ) ) ? ( _rtDW -> TransportDelay_IWORK_i . Head + 1 ) : 0
) ; if ( _rtDW -> TransportDelay_IWORK_i . Head == _rtDW ->
TransportDelay_IWORK_i . Tail ) { if ( !
Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_i . CircularBufSize , & _rtDW -> TransportDelay_IWORK_i
. Tail , & _rtDW -> TransportDelay_IWORK_i . Head , & _rtDW ->
TransportDelay_IWORK_i . Last , simTime - _rtP -> P_144 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_i .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_i . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_i . Head ] = _rtB ->
B_80_208_0 ; } if ( _rtDW -> LastMajorTimeA_er == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_er = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_o4 = _rtB ->
B_80_234_0 ; } else if ( _rtDW -> LastMajorTimeB_p == ( rtInf ) ) { _rtDW ->
LastMajorTimeB_p = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_om = _rtB ->
B_80_234_0 ; } else if ( _rtDW -> LastMajorTimeA_er < _rtDW ->
LastMajorTimeB_p ) { _rtDW -> LastMajorTimeA_er = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYA_o4 = _rtB -> B_80_234_0 ; } else { _rtDW -> LastMajorTimeB_p
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_om = _rtB -> B_80_234_0 ; } _rtDW
-> vv0at_IWORK_f = 0 ; { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_ct . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T
* * ) & _rtDW -> TransportDelay_PWORK_ct . TUbufferPtrs [ 1 ] ; real_T
simTime = ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_a4 . Head = ( ( _rtDW
-> TransportDelay_IWORK_a4 . Head < ( _rtDW -> TransportDelay_IWORK_a4 .
CircularBufSize - 1 ) ) ? ( _rtDW -> TransportDelay_IWORK_a4 . Head + 1 ) : 0
) ; if ( _rtDW -> TransportDelay_IWORK_a4 . Head == _rtDW ->
TransportDelay_IWORK_a4 . Tail ) { if ( !
Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_a4 . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_a4 . Tail , & _rtDW -> TransportDelay_IWORK_a4 . Head ,
& _rtDW -> TransportDelay_IWORK_a4 . Last , simTime - _rtP -> P_160 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_a4 . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_a4 . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_a4 . Head ] = _rtB -> B_80_231_0 ; } if ( _rtDW ->
LastMajorTimeA_d == ( rtInf ) ) { _rtDW -> LastMajorTimeA_d = ssGetTaskTime (
S , 0 ) ; _rtDW -> PrevYA_nl = _rtB -> B_80_257_0 ; } else if ( _rtDW ->
LastMajorTimeB_n == ( rtInf ) ) { _rtDW -> LastMajorTimeB_n = ssGetTaskTime (
S , 0 ) ; _rtDW -> PrevYB_jx = _rtB -> B_80_257_0 ; } else if ( _rtDW ->
LastMajorTimeA_d < _rtDW -> LastMajorTimeB_n ) { _rtDW -> LastMajorTimeA_d =
ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_nl = _rtB -> B_80_257_0 ; } else {
_rtDW -> LastMajorTimeB_n = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_jx =
_rtB -> B_80_257_0 ; } _rtDW -> vv0at_IWORK_p = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_d . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_d . TUbufferPtrs
[ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_co .
Head = ( ( _rtDW -> TransportDelay_IWORK_co . Head < ( _rtDW ->
TransportDelay_IWORK_co . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_co . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_co . Head == _rtDW -> TransportDelay_IWORK_co . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_co . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_co . Tail , & _rtDW -> TransportDelay_IWORK_co . Head ,
& _rtDW -> TransportDelay_IWORK_co . Last , simTime - _rtP -> P_176 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_co . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_co . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_co . Head ] = _rtB -> B_80_254_0 ; } if ( _rtDW ->
LastMajorTimeA_c == ( rtInf ) ) { _rtDW -> LastMajorTimeA_c = ssGetTaskTime (
S , 0 ) ; _rtDW -> PrevYA_g = _rtB -> B_80_280_0 ; } else if ( _rtDW ->
LastMajorTimeB_l == ( rtInf ) ) { _rtDW -> LastMajorTimeB_l = ssGetTaskTime (
S , 0 ) ; _rtDW -> PrevYB_my = _rtB -> B_80_280_0 ; } else if ( _rtDW ->
LastMajorTimeA_c < _rtDW -> LastMajorTimeB_l ) { _rtDW -> LastMajorTimeA_c =
ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_g = _rtB -> B_80_280_0 ; } else {
_rtDW -> LastMajorTimeB_l = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_my =
_rtB -> B_80_280_0 ; } _rtDW -> vv0at_IWORK_g = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_la . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_la .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_fv . Head = ( ( _rtDW -> TransportDelay_IWORK_fv . Head
< ( _rtDW -> TransportDelay_IWORK_fv . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_fv . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_fv . Head == _rtDW -> TransportDelay_IWORK_fv . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_fv . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_fv . Tail , & _rtDW -> TransportDelay_IWORK_fv . Head ,
& _rtDW -> TransportDelay_IWORK_fv . Last , simTime - _rtP -> P_192 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_fv . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_fv . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_fv . Head ] = _rtB -> B_80_277_0 ; } if ( _rtDW ->
LastMajorTimeA_l == ( rtInf ) ) { _rtDW -> LastMajorTimeA_l = ssGetTaskTime (
S , 0 ) ; _rtDW -> PrevYA_gl = _rtB -> B_80_303_0 ; } else if ( _rtDW ->
LastMajorTimeB_e == ( rtInf ) ) { _rtDW -> LastMajorTimeB_e = ssGetTaskTime (
S , 0 ) ; _rtDW -> PrevYB_l = _rtB -> B_80_303_0 ; } else if ( _rtDW ->
LastMajorTimeA_l < _rtDW -> LastMajorTimeB_e ) { _rtDW -> LastMajorTimeA_l =
ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_gl = _rtB -> B_80_303_0 ; } else {
_rtDW -> LastMajorTimeB_e = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_l =
_rtB -> B_80_303_0 ; } _rtDW -> vv0at_IWORK_bm = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_nw . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_nw .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_p . Head = ( ( _rtDW -> TransportDelay_IWORK_p . Head <
( _rtDW -> TransportDelay_IWORK_p . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_p . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_p . Head == _rtDW -> TransportDelay_IWORK_p . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_p . CircularBufSize , & _rtDW -> TransportDelay_IWORK_p
. Tail , & _rtDW -> TransportDelay_IWORK_p . Head , & _rtDW ->
TransportDelay_IWORK_p . Last , simTime - _rtP -> P_208 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_p .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_p . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_p . Head ] = _rtB ->
B_80_300_0 ; } if ( _rtDW -> LastMajorTimeA_j == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_j = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_dh = _rtB ->
B_80_326_0 ; } else if ( _rtDW -> LastMajorTimeB_gi == ( rtInf ) ) { _rtDW ->
LastMajorTimeB_gi = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_a = _rtB ->
B_80_326_0 ; } else if ( _rtDW -> LastMajorTimeA_j < _rtDW ->
LastMajorTimeB_gi ) { _rtDW -> LastMajorTimeA_j = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYA_dh = _rtB -> B_80_326_0 ; } else { _rtDW -> LastMajorTimeB_gi
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_a = _rtB -> B_80_326_0 ; } _rtDW
-> vv0at_IWORK_e = 0 ; { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_dl . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T
* * ) & _rtDW -> TransportDelay_PWORK_dl . TUbufferPtrs [ 1 ] ; real_T
simTime = ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_hc . Head = ( ( _rtDW
-> TransportDelay_IWORK_hc . Head < ( _rtDW -> TransportDelay_IWORK_hc .
CircularBufSize - 1 ) ) ? ( _rtDW -> TransportDelay_IWORK_hc . Head + 1 ) : 0
) ; if ( _rtDW -> TransportDelay_IWORK_hc . Head == _rtDW ->
TransportDelay_IWORK_hc . Tail ) { if ( !
Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_hc . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_hc . Tail , & _rtDW -> TransportDelay_IWORK_hc . Head ,
& _rtDW -> TransportDelay_IWORK_hc . Last , simTime - _rtP -> P_224 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_hc . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_hc . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_hc . Head ] = _rtB -> B_80_323_0 ; } if ( _rtDW ->
LastMajorTimeA_ns == ( rtInf ) ) { _rtDW -> LastMajorTimeA_ns = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_b = _rtB -> B_80_349_0 ; } else if ( _rtDW ->
LastMajorTimeB_a == ( rtInf ) ) { _rtDW -> LastMajorTimeB_a = ssGetTaskTime (
S , 0 ) ; _rtDW -> PrevYB_d = _rtB -> B_80_349_0 ; } else if ( _rtDW ->
LastMajorTimeA_ns < _rtDW -> LastMajorTimeB_a ) { _rtDW -> LastMajorTimeA_ns
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_b = _rtB -> B_80_349_0 ; } else {
_rtDW -> LastMajorTimeB_a = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_d =
_rtB -> B_80_349_0 ; } _rtDW -> vv0at_IWORK_bg = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_m . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_m . TUbufferPtrs
[ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_c0 .
Head = ( ( _rtDW -> TransportDelay_IWORK_c0 . Head < ( _rtDW ->
TransportDelay_IWORK_c0 . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_c0 . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_c0 . Head == _rtDW -> TransportDelay_IWORK_c0 . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_c0 . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_c0 . Tail , & _rtDW -> TransportDelay_IWORK_c0 . Head ,
& _rtDW -> TransportDelay_IWORK_c0 . Last , simTime - _rtP -> P_240 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_c0 . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_c0 . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_c0 . Head ] = _rtB -> B_80_346_0 ; } if ( _rtDW ->
LastMajorTimeA_fg == ( rtInf ) ) { _rtDW -> LastMajorTimeA_fg = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_e = _rtB -> B_80_372_0 ; } else if ( _rtDW ->
LastMajorTimeB_ps == ( rtInf ) ) { _rtDW -> LastMajorTimeB_ps = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_p = _rtB -> B_80_372_0 ; } else if ( _rtDW ->
LastMajorTimeA_fg < _rtDW -> LastMajorTimeB_ps ) { _rtDW -> LastMajorTimeA_fg
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_e = _rtB -> B_80_372_0 ; } else {
_rtDW -> LastMajorTimeB_ps = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_p =
_rtB -> B_80_372_0 ; } _rtDW -> vv0at_IWORK_kl = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_ej . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_ej .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_dp . Head = ( ( _rtDW -> TransportDelay_IWORK_dp . Head
< ( _rtDW -> TransportDelay_IWORK_dp . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_dp . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_dp . Head == _rtDW -> TransportDelay_IWORK_dp . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_dp . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_dp . Tail , & _rtDW -> TransportDelay_IWORK_dp . Head ,
& _rtDW -> TransportDelay_IWORK_dp . Last , simTime - _rtP -> P_256 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_dp . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_dp . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_dp . Head ] = _rtB -> B_80_369_0 ; } if ( _rtDW ->
LastMajorTimeA_eq == ( rtInf ) ) { _rtDW -> LastMajorTimeA_eq = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_m = _rtB -> B_80_395_0 ; } else if ( _rtDW ->
LastMajorTimeB_oc == ( rtInf ) ) { _rtDW -> LastMajorTimeB_oc = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_k = _rtB -> B_80_395_0 ; } else if ( _rtDW ->
LastMajorTimeA_eq < _rtDW -> LastMajorTimeB_oc ) { _rtDW -> LastMajorTimeA_eq
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_m = _rtB -> B_80_395_0 ; } else {
_rtDW -> LastMajorTimeB_oc = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_k =
_rtB -> B_80_395_0 ; } _rtDW -> vv0at_IWORK_pf = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_o4 . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_o4 .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_j . Head = ( ( _rtDW -> TransportDelay_IWORK_j . Head <
( _rtDW -> TransportDelay_IWORK_j . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_j . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_j . Head == _rtDW -> TransportDelay_IWORK_j . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_j . CircularBufSize , & _rtDW -> TransportDelay_IWORK_j
. Tail , & _rtDW -> TransportDelay_IWORK_j . Head , & _rtDW ->
TransportDelay_IWORK_j . Last , simTime - _rtP -> P_272 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_j .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_j . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_j . Head ] = _rtB ->
B_80_392_0 ; } if ( _rtDW -> LastMajorTimeA_dq == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_dq = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_a = _rtB ->
B_80_418_0 ; } else if ( _rtDW -> LastMajorTimeB_cb == ( rtInf ) ) { _rtDW ->
LastMajorTimeB_cb = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_gj = _rtB ->
B_80_418_0 ; } else if ( _rtDW -> LastMajorTimeA_dq < _rtDW ->
LastMajorTimeB_cb ) { _rtDW -> LastMajorTimeA_dq = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYA_a = _rtB -> B_80_418_0 ; } else { _rtDW -> LastMajorTimeB_cb
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_gj = _rtB -> B_80_418_0 ; } _rtDW
-> vv0at_IWORK_fs = 0 ; { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_a . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T *
* ) & _rtDW -> TransportDelay_PWORK_a . TUbufferPtrs [ 1 ] ; real_T simTime =
ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_f1 . Head = ( ( _rtDW ->
TransportDelay_IWORK_f1 . Head < ( _rtDW -> TransportDelay_IWORK_f1 .
CircularBufSize - 1 ) ) ? ( _rtDW -> TransportDelay_IWORK_f1 . Head + 1 ) : 0
) ; if ( _rtDW -> TransportDelay_IWORK_f1 . Head == _rtDW ->
TransportDelay_IWORK_f1 . Tail ) { if ( !
Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_f1 . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_f1 . Tail , & _rtDW -> TransportDelay_IWORK_f1 . Head ,
& _rtDW -> TransportDelay_IWORK_f1 . Last , simTime - _rtP -> P_288 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_f1 . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_f1 . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_f1 . Head ] = _rtB -> B_80_415_0 ; } if ( _rtDW ->
LastMajorTimeA_li == ( rtInf ) ) { _rtDW -> LastMajorTimeA_li = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_k1 = _rtB -> B_80_441_0 ; } else if ( _rtDW ->
LastMajorTimeB_f3 == ( rtInf ) ) { _rtDW -> LastMajorTimeB_f3 = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_od = _rtB -> B_80_441_0 ; } else if ( _rtDW ->
LastMajorTimeA_li < _rtDW -> LastMajorTimeB_f3 ) { _rtDW -> LastMajorTimeA_li
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_k1 = _rtB -> B_80_441_0 ; } else
{ _rtDW -> LastMajorTimeB_f3 = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_od =
_rtB -> B_80_441_0 ; } _rtDW -> vv0at_IWORK_hd = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_g . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_g . TUbufferPtrs
[ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_g .
Head = ( ( _rtDW -> TransportDelay_IWORK_g . Head < ( _rtDW ->
TransportDelay_IWORK_g . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_g . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_g . Head == _rtDW -> TransportDelay_IWORK_g . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_g . CircularBufSize , & _rtDW -> TransportDelay_IWORK_g
. Tail , & _rtDW -> TransportDelay_IWORK_g . Head , & _rtDW ->
TransportDelay_IWORK_g . Last , simTime - _rtP -> P_304 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_g .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_g . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_g . Head ] = _rtB ->
B_80_438_0 ; } if ( _rtDW -> LastMajorTimeA_ou == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_ou = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_h = _rtB ->
B_80_464_0 ; } else if ( _rtDW -> LastMajorTimeB_pr == ( rtInf ) ) { _rtDW ->
LastMajorTimeB_pr = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_af = _rtB ->
B_80_464_0 ; } else if ( _rtDW -> LastMajorTimeA_ou < _rtDW ->
LastMajorTimeB_pr ) { _rtDW -> LastMajorTimeA_ou = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYA_h = _rtB -> B_80_464_0 ; } else { _rtDW -> LastMajorTimeB_pr
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_af = _rtB -> B_80_464_0 ; } _rtDW
-> vv0at_IWORK_d = 0 ; { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_as . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T
* * ) & _rtDW -> TransportDelay_PWORK_as . TUbufferPtrs [ 1 ] ; real_T
simTime = ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_b . Head = ( ( _rtDW
-> TransportDelay_IWORK_b . Head < ( _rtDW -> TransportDelay_IWORK_b .
CircularBufSize - 1 ) ) ? ( _rtDW -> TransportDelay_IWORK_b . Head + 1 ) : 0
) ; if ( _rtDW -> TransportDelay_IWORK_b . Head == _rtDW ->
TransportDelay_IWORK_b . Tail ) { if ( !
Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_b . CircularBufSize , & _rtDW -> TransportDelay_IWORK_b
. Tail , & _rtDW -> TransportDelay_IWORK_b . Head , & _rtDW ->
TransportDelay_IWORK_b . Last , simTime - _rtP -> P_320 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_b .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_b . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_b . Head ] = _rtB ->
B_80_461_0 ; } if ( _rtDW -> LastMajorTimeA_eh == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_eh = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_f = _rtB ->
B_80_487_0 ; } else if ( _rtDW -> LastMajorTimeB_no == ( rtInf ) ) { _rtDW ->
LastMajorTimeB_no = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_e = _rtB ->
B_80_487_0 ; } else if ( _rtDW -> LastMajorTimeA_eh < _rtDW ->
LastMajorTimeB_no ) { _rtDW -> LastMajorTimeA_eh = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYA_f = _rtB -> B_80_487_0 ; } else { _rtDW -> LastMajorTimeB_no
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_e = _rtB -> B_80_487_0 ; } _rtDW
-> vv0at_IWORK_fw = 0 ; { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_j . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T *
* ) & _rtDW -> TransportDelay_PWORK_j . TUbufferPtrs [ 1 ] ; real_T simTime =
ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_m . Head = ( ( _rtDW ->
TransportDelay_IWORK_m . Head < ( _rtDW -> TransportDelay_IWORK_m .
CircularBufSize - 1 ) ) ? ( _rtDW -> TransportDelay_IWORK_m . Head + 1 ) : 0
) ; if ( _rtDW -> TransportDelay_IWORK_m . Head == _rtDW ->
TransportDelay_IWORK_m . Tail ) { if ( !
Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_m . CircularBufSize , & _rtDW -> TransportDelay_IWORK_m
. Tail , & _rtDW -> TransportDelay_IWORK_m . Head , & _rtDW ->
TransportDelay_IWORK_m . Last , simTime - _rtP -> P_336 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_m .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_m . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_m . Head ] = _rtB ->
B_80_484_0 ; } if ( _rtDW -> LastMajorTimeA_ly == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_ly = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_e4 = _rtB ->
B_80_510_0 ; } else if ( _rtDW -> LastMajorTimeB_m0 == ( rtInf ) ) { _rtDW ->
LastMajorTimeB_m0 = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_pd = _rtB ->
B_80_510_0 ; } else if ( _rtDW -> LastMajorTimeA_ly < _rtDW ->
LastMajorTimeB_m0 ) { _rtDW -> LastMajorTimeA_ly = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYA_e4 = _rtB -> B_80_510_0 ; } else { _rtDW -> LastMajorTimeB_m0
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_pd = _rtB -> B_80_510_0 ; } _rtDW
-> vv0at_IWORK_fz = 0 ; { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_jw . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T
* * ) & _rtDW -> TransportDelay_PWORK_jw . TUbufferPtrs [ 1 ] ; real_T
simTime = ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_ln . Head = ( ( _rtDW
-> TransportDelay_IWORK_ln . Head < ( _rtDW -> TransportDelay_IWORK_ln .
CircularBufSize - 1 ) ) ? ( _rtDW -> TransportDelay_IWORK_ln . Head + 1 ) : 0
) ; if ( _rtDW -> TransportDelay_IWORK_ln . Head == _rtDW ->
TransportDelay_IWORK_ln . Tail ) { if ( !
Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_ln . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_ln . Tail , & _rtDW -> TransportDelay_IWORK_ln . Head ,
& _rtDW -> TransportDelay_IWORK_ln . Last , simTime - _rtP -> P_352 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_ln . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_ln . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_ln . Head ] = _rtB -> B_80_507_0 ; } if ( _rtDW ->
LastMajorTimeA_k == ( rtInf ) ) { _rtDW -> LastMajorTimeA_k = ssGetTaskTime (
S , 0 ) ; _rtDW -> PrevYA_oo = _rtB -> B_80_533_0 ; } else if ( _rtDW ->
LastMajorTimeB_lc == ( rtInf ) ) { _rtDW -> LastMajorTimeB_lc = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_et = _rtB -> B_80_533_0 ; } else if ( _rtDW ->
LastMajorTimeA_k < _rtDW -> LastMajorTimeB_lc ) { _rtDW -> LastMajorTimeA_k =
ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_oo = _rtB -> B_80_533_0 ; } else {
_rtDW -> LastMajorTimeB_lc = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_et =
_rtB -> B_80_533_0 ; } _rtDW -> vv0at_IWORK_ek = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_ey . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_ey .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_m5 . Head = ( ( _rtDW -> TransportDelay_IWORK_m5 . Head
< ( _rtDW -> TransportDelay_IWORK_m5 . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_m5 . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_m5 . Head == _rtDW -> TransportDelay_IWORK_m5 . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_m5 . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_m5 . Tail , & _rtDW -> TransportDelay_IWORK_m5 . Head ,
& _rtDW -> TransportDelay_IWORK_m5 . Last , simTime - _rtP -> P_368 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_m5 . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_m5 . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_m5 . Head ] = _rtB -> B_80_530_0 ; } if ( _rtDW ->
LastMajorTimeA_i1 == ( rtInf ) ) { _rtDW -> LastMajorTimeA_i1 = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_c = _rtB -> B_80_556_0 ; } else if ( _rtDW ->
LastMajorTimeB_mi == ( rtInf ) ) { _rtDW -> LastMajorTimeB_mi = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_ka = _rtB -> B_80_556_0 ; } else if ( _rtDW ->
LastMajorTimeA_i1 < _rtDW -> LastMajorTimeB_mi ) { _rtDW -> LastMajorTimeA_i1
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_c = _rtB -> B_80_556_0 ; } else {
_rtDW -> LastMajorTimeB_mi = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_ka =
_rtB -> B_80_556_0 ; } _rtDW -> vv0at_IWORK_fm = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_ap . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_ap .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_n . Head = ( ( _rtDW -> TransportDelay_IWORK_n . Head <
( _rtDW -> TransportDelay_IWORK_n . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_n . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_n . Head == _rtDW -> TransportDelay_IWORK_n . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_n . CircularBufSize , & _rtDW -> TransportDelay_IWORK_n
. Tail , & _rtDW -> TransportDelay_IWORK_n . Head , & _rtDW ->
TransportDelay_IWORK_n . Last , simTime - _rtP -> P_384 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_n .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_n . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_n . Head ] = _rtB ->
B_80_553_0 ; } if ( _rtDW -> LastMajorTimeA_kn == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_kn = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_lz = _rtB ->
B_80_579_0 ; } else if ( _rtDW -> LastMajorTimeB_an == ( rtInf ) ) { _rtDW ->
LastMajorTimeB_an = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_oe = _rtB ->
B_80_579_0 ; } else if ( _rtDW -> LastMajorTimeA_kn < _rtDW ->
LastMajorTimeB_an ) { _rtDW -> LastMajorTimeA_kn = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYA_lz = _rtB -> B_80_579_0 ; } else { _rtDW -> LastMajorTimeB_an
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_oe = _rtB -> B_80_579_0 ; } _rtDW
-> vv0at_IWORK_py = 0 ; { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_i . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T *
* ) & _rtDW -> TransportDelay_PWORK_i . TUbufferPtrs [ 1 ] ; real_T simTime =
ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_k . Head = ( ( _rtDW ->
TransportDelay_IWORK_k . Head < ( _rtDW -> TransportDelay_IWORK_k .
CircularBufSize - 1 ) ) ? ( _rtDW -> TransportDelay_IWORK_k . Head + 1 ) : 0
) ; if ( _rtDW -> TransportDelay_IWORK_k . Head == _rtDW ->
TransportDelay_IWORK_k . Tail ) { if ( !
Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_k . CircularBufSize , & _rtDW -> TransportDelay_IWORK_k
. Tail , & _rtDW -> TransportDelay_IWORK_k . Head , & _rtDW ->
TransportDelay_IWORK_k . Last , simTime - _rtP -> P_400 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_k .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_k . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_k . Head ] = _rtB ->
B_80_576_0 ; } if ( _rtDW -> LastMajorTimeA_bx == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_bx = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_cn = _rtB ->
B_80_602_0 ; } else if ( _rtDW -> LastMajorTimeB_b == ( rtInf ) ) { _rtDW ->
LastMajorTimeB_b = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_gs = _rtB ->
B_80_602_0 ; } else if ( _rtDW -> LastMajorTimeA_bx < _rtDW ->
LastMajorTimeB_b ) { _rtDW -> LastMajorTimeA_bx = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYA_cn = _rtB -> B_80_602_0 ; } else { _rtDW -> LastMajorTimeB_b
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_gs = _rtB -> B_80_602_0 ; } _rtDW
-> vv0at_IWORK_kw = 0 ; { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_ls . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T
* * ) & _rtDW -> TransportDelay_PWORK_ls . TUbufferPtrs [ 1 ] ; real_T
simTime = ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_j2 . Head = ( ( _rtDW
-> TransportDelay_IWORK_j2 . Head < ( _rtDW -> TransportDelay_IWORK_j2 .
CircularBufSize - 1 ) ) ? ( _rtDW -> TransportDelay_IWORK_j2 . Head + 1 ) : 0
) ; if ( _rtDW -> TransportDelay_IWORK_j2 . Head == _rtDW ->
TransportDelay_IWORK_j2 . Tail ) { if ( !
Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_j2 . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_j2 . Tail , & _rtDW -> TransportDelay_IWORK_j2 . Head ,
& _rtDW -> TransportDelay_IWORK_j2 . Last , simTime - _rtP -> P_416 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_j2 . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_j2 . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_j2 . Head ] = _rtB -> B_80_599_0 ; } if ( _rtDW ->
LastMajorTimeA_bb == ( rtInf ) ) { _rtDW -> LastMajorTimeA_bb = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_js = _rtB -> B_80_625_0 ; } else if ( _rtDW ->
LastMajorTimeB_h == ( rtInf ) ) { _rtDW -> LastMajorTimeB_h = ssGetTaskTime (
S , 0 ) ; _rtDW -> PrevYB_k4 = _rtB -> B_80_625_0 ; } else if ( _rtDW ->
LastMajorTimeA_bb < _rtDW -> LastMajorTimeB_h ) { _rtDW -> LastMajorTimeA_bb
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_js = _rtB -> B_80_625_0 ; } else
{ _rtDW -> LastMajorTimeB_h = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_k4 =
_rtB -> B_80_625_0 ; } _rtDW -> vv0at_IWORK_dh = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_p . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_p . TUbufferPtrs
[ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_nt .
Head = ( ( _rtDW -> TransportDelay_IWORK_nt . Head < ( _rtDW ->
TransportDelay_IWORK_nt . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_nt . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_nt . Head == _rtDW -> TransportDelay_IWORK_nt . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_nt . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_nt . Tail , & _rtDW -> TransportDelay_IWORK_nt . Head ,
& _rtDW -> TransportDelay_IWORK_nt . Last , simTime - _rtP -> P_432 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_nt . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_nt . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_nt . Head ] = _rtB -> B_80_622_0 ; } if ( _rtDW ->
LastMajorTimeA_jb == ( rtInf ) ) { _rtDW -> LastMajorTimeA_jb = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_lt = _rtB -> B_80_648_0 ; } else if ( _rtDW ->
LastMajorTimeB_fe == ( rtInf ) ) { _rtDW -> LastMajorTimeB_fe = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_oo = _rtB -> B_80_648_0 ; } else if ( _rtDW ->
LastMajorTimeA_jb < _rtDW -> LastMajorTimeB_fe ) { _rtDW -> LastMajorTimeA_jb
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_lt = _rtB -> B_80_648_0 ; } else
{ _rtDW -> LastMajorTimeB_fe = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_oo =
_rtB -> B_80_648_0 ; } _rtDW -> vv0at_IWORK_fa = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_lm . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_lm .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_ls . Head = ( ( _rtDW -> TransportDelay_IWORK_ls . Head
< ( _rtDW -> TransportDelay_IWORK_ls . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_ls . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_ls . Head == _rtDW -> TransportDelay_IWORK_ls . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_ls . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_ls . Tail , & _rtDW -> TransportDelay_IWORK_ls . Head ,
& _rtDW -> TransportDelay_IWORK_ls . Last , simTime - _rtP -> P_448 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_ls . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_ls . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_ls . Head ] = _rtB -> B_80_645_0 ; } if ( _rtDW ->
LastMajorTimeA_ne == ( rtInf ) ) { _rtDW -> LastMajorTimeA_ne = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_p5 = _rtB -> B_80_671_0 ; } else if ( _rtDW ->
LastMajorTimeB_a5 == ( rtInf ) ) { _rtDW -> LastMajorTimeB_a5 = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_ls = _rtB -> B_80_671_0 ; } else if ( _rtDW ->
LastMajorTimeA_ne < _rtDW -> LastMajorTimeB_a5 ) { _rtDW -> LastMajorTimeA_ne
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_p5 = _rtB -> B_80_671_0 ; } else
{ _rtDW -> LastMajorTimeB_a5 = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_ls =
_rtB -> B_80_671_0 ; } _rtDW -> vv0at_IWORK_gf = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_h . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_h . TUbufferPtrs
[ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_e .
Head = ( ( _rtDW -> TransportDelay_IWORK_e . Head < ( _rtDW ->
TransportDelay_IWORK_e . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_e . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_e . Head == _rtDW -> TransportDelay_IWORK_e . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_e . CircularBufSize , & _rtDW -> TransportDelay_IWORK_e
. Tail , & _rtDW -> TransportDelay_IWORK_e . Head , & _rtDW ->
TransportDelay_IWORK_e . Last , simTime - _rtP -> P_464 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_e .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_e . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_e . Head ] = _rtB ->
B_80_668_0 ; } if ( _rtDW -> LastMajorTimeA_h == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_h = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_mc = _rtB ->
B_80_694_0 ; } else if ( _rtDW -> LastMajorTimeB_au == ( rtInf ) ) { _rtDW ->
LastMajorTimeB_au = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_c = _rtB ->
B_80_694_0 ; } else if ( _rtDW -> LastMajorTimeA_h < _rtDW ->
LastMajorTimeB_au ) { _rtDW -> LastMajorTimeA_h = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYA_mc = _rtB -> B_80_694_0 ; } else { _rtDW -> LastMajorTimeB_au
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_c = _rtB -> B_80_694_0 ; } _rtDW
-> vv0at_IWORK_cc = 0 ; { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_pp . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T
* * ) & _rtDW -> TransportDelay_PWORK_pp . TUbufferPtrs [ 1 ] ; real_T
simTime = ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_ex . Head = ( ( _rtDW
-> TransportDelay_IWORK_ex . Head < ( _rtDW -> TransportDelay_IWORK_ex .
CircularBufSize - 1 ) ) ? ( _rtDW -> TransportDelay_IWORK_ex . Head + 1 ) : 0
) ; if ( _rtDW -> TransportDelay_IWORK_ex . Head == _rtDW ->
TransportDelay_IWORK_ex . Tail ) { if ( !
Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_ex . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_ex . Tail , & _rtDW -> TransportDelay_IWORK_ex . Head ,
& _rtDW -> TransportDelay_IWORK_ex . Last , simTime - _rtP -> P_480 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_ex . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_ex . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_ex . Head ] = _rtB -> B_80_691_0 ; } if ( _rtDW ->
LastMajorTimeA_neb == ( rtInf ) ) { _rtDW -> LastMajorTimeA_neb =
ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_a2 = _rtB -> B_80_717_0 ; } else if
( _rtDW -> LastMajorTimeB_gg == ( rtInf ) ) { _rtDW -> LastMajorTimeB_gg =
ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_jb = _rtB -> B_80_717_0 ; } else if
( _rtDW -> LastMajorTimeA_neb < _rtDW -> LastMajorTimeB_gg ) { _rtDW ->
LastMajorTimeA_neb = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_a2 = _rtB ->
B_80_717_0 ; } else { _rtDW -> LastMajorTimeB_gg = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYB_jb = _rtB -> B_80_717_0 ; } _rtDW -> vv0at_IWORK_jq = 0 ; {
real_T * * uBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_lf .
TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_lf . TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S )
; _rtDW -> TransportDelay_IWORK_ir . Head = ( ( _rtDW ->
TransportDelay_IWORK_ir . Head < ( _rtDW -> TransportDelay_IWORK_ir .
CircularBufSize - 1 ) ) ? ( _rtDW -> TransportDelay_IWORK_ir . Head + 1 ) : 0
) ; if ( _rtDW -> TransportDelay_IWORK_ir . Head == _rtDW ->
TransportDelay_IWORK_ir . Tail ) { if ( !
Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_ir . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_ir . Tail , & _rtDW -> TransportDelay_IWORK_ir . Head ,
& _rtDW -> TransportDelay_IWORK_ir . Last , simTime - _rtP -> P_496 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_ir . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_ir . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_ir . Head ] = _rtB -> B_80_714_0 ; } if ( _rtDW ->
LastMajorTimeA_kk == ( rtInf ) ) { _rtDW -> LastMajorTimeA_kk = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_dy = _rtB -> B_80_740_0 ; } else if ( _rtDW ->
LastMajorTimeB_bi == ( rtInf ) ) { _rtDW -> LastMajorTimeB_bi = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_lt = _rtB -> B_80_740_0 ; } else if ( _rtDW ->
LastMajorTimeA_kk < _rtDW -> LastMajorTimeB_bi ) { _rtDW -> LastMajorTimeA_kk
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_dy = _rtB -> B_80_740_0 ; } else
{ _rtDW -> LastMajorTimeB_bi = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_lt =
_rtB -> B_80_740_0 ; } _rtDW -> vv0at_IWORK_kz = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_o2 . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_o2 .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_m3 . Head = ( ( _rtDW -> TransportDelay_IWORK_m3 . Head
< ( _rtDW -> TransportDelay_IWORK_m3 . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_m3 . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_m3 . Head == _rtDW -> TransportDelay_IWORK_m3 . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_m3 . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_m3 . Tail , & _rtDW -> TransportDelay_IWORK_m3 . Head ,
& _rtDW -> TransportDelay_IWORK_m3 . Last , simTime - _rtP -> P_512 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_m3 . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_m3 . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_m3 . Head ] = _rtB -> B_80_737_0 ; } if ( _rtDW ->
LastMajorTimeA_f5 == ( rtInf ) ) { _rtDW -> LastMajorTimeA_f5 = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_bl = _rtB -> B_80_763_0 ; } else if ( _rtDW ->
LastMajorTimeB_b3 == ( rtInf ) ) { _rtDW -> LastMajorTimeB_b3 = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_ak = _rtB -> B_80_763_0 ; } else if ( _rtDW ->
LastMajorTimeA_f5 < _rtDW -> LastMajorTimeB_b3 ) { _rtDW -> LastMajorTimeA_f5
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_bl = _rtB -> B_80_763_0 ; } else
{ _rtDW -> LastMajorTimeB_b3 = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_ak =
_rtB -> B_80_763_0 ; } _rtDW -> vv0at_IWORK_dk = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_pi . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_pi .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_o . Head = ( ( _rtDW -> TransportDelay_IWORK_o . Head <
( _rtDW -> TransportDelay_IWORK_o . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_o . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_o . Head == _rtDW -> TransportDelay_IWORK_o . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_o . CircularBufSize , & _rtDW -> TransportDelay_IWORK_o
. Tail , & _rtDW -> TransportDelay_IWORK_o . Head , & _rtDW ->
TransportDelay_IWORK_o . Last , simTime - _rtP -> P_528 , tBuffer , uBuffer ,
( NULL ) , ( boolean_T ) 0 , false , & _rtDW -> TransportDelay_IWORK_o .
MaxNewBufSize ) ) { ssSetErrorStatus ( S , "tdelay memory allocation error" )
; return ; } } ( * tBuffer ) [ _rtDW -> TransportDelay_IWORK_o . Head ] =
simTime ; ( * uBuffer ) [ _rtDW -> TransportDelay_IWORK_o . Head ] = _rtB ->
B_80_760_0 ; } if ( _rtDW -> LastMajorTimeA_n0 == ( rtInf ) ) { _rtDW ->
LastMajorTimeA_n0 = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_an = _rtB ->
B_80_786_0 ; } else if ( _rtDW -> LastMajorTimeB_b2 == ( rtInf ) ) { _rtDW ->
LastMajorTimeB_b2 = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_i = _rtB ->
B_80_786_0 ; } else if ( _rtDW -> LastMajorTimeA_n0 < _rtDW ->
LastMajorTimeB_b2 ) { _rtDW -> LastMajorTimeA_n0 = ssGetTaskTime ( S , 0 ) ;
_rtDW -> PrevYA_an = _rtB -> B_80_786_0 ; } else { _rtDW -> LastMajorTimeB_b2
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_i = _rtB -> B_80_786_0 ; } _rtDW
-> vv0at_IWORK_da = 0 ; { real_T * * uBuffer = ( real_T * * ) & _rtDW ->
TransportDelay_PWORK_lr . TUbufferPtrs [ 0 ] ; real_T * * tBuffer = ( real_T
* * ) & _rtDW -> TransportDelay_PWORK_lr . TUbufferPtrs [ 1 ] ; real_T
simTime = ssGetT ( S ) ; _rtDW -> TransportDelay_IWORK_gs . Head = ( ( _rtDW
-> TransportDelay_IWORK_gs . Head < ( _rtDW -> TransportDelay_IWORK_gs .
CircularBufSize - 1 ) ) ? ( _rtDW -> TransportDelay_IWORK_gs . Head + 1 ) : 0
) ; if ( _rtDW -> TransportDelay_IWORK_gs . Head == _rtDW ->
TransportDelay_IWORK_gs . Tail ) { if ( !
Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_gs . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_gs . Tail , & _rtDW -> TransportDelay_IWORK_gs . Head ,
& _rtDW -> TransportDelay_IWORK_gs . Last , simTime - _rtP -> P_544 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_gs . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_gs . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_gs . Head ] = _rtB -> B_80_783_0 ; } if ( _rtDW ->
LastMajorTimeA_id == ( rtInf ) ) { _rtDW -> LastMajorTimeA_id = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_jo = _rtB -> B_80_809_0 ; } else if ( _rtDW ->
LastMajorTimeB_ei == ( rtInf ) ) { _rtDW -> LastMajorTimeB_ei = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_ew = _rtB -> B_80_809_0 ; } else if ( _rtDW ->
LastMajorTimeA_id < _rtDW -> LastMajorTimeB_ei ) { _rtDW -> LastMajorTimeA_id
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_jo = _rtB -> B_80_809_0 ; } else
{ _rtDW -> LastMajorTimeB_ei = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_ew =
_rtB -> B_80_809_0 ; } _rtDW -> vv0at_IWORK_oe = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_gr . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_gr .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_id . Head = ( ( _rtDW -> TransportDelay_IWORK_id . Head
< ( _rtDW -> TransportDelay_IWORK_id . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_id . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_id . Head == _rtDW -> TransportDelay_IWORK_id . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_id . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_id . Tail , & _rtDW -> TransportDelay_IWORK_id . Head ,
& _rtDW -> TransportDelay_IWORK_id . Last , simTime - _rtP -> P_560 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_id . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_id . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_id . Head ] = _rtB -> B_80_806_0 ; } if ( _rtDW ->
LastMajorTimeA_np == ( rtInf ) ) { _rtDW -> LastMajorTimeA_np = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_ei = _rtB -> B_80_832_0 ; } else if ( _rtDW ->
LastMajorTimeB_lo == ( rtInf ) ) { _rtDW -> LastMajorTimeB_lo = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_k1 = _rtB -> B_80_832_0 ; } else if ( _rtDW ->
LastMajorTimeA_np < _rtDW -> LastMajorTimeB_lo ) { _rtDW -> LastMajorTimeA_np
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_ei = _rtB -> B_80_832_0 ; } else
{ _rtDW -> LastMajorTimeB_lo = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_k1 =
_rtB -> B_80_832_0 ; } _rtDW -> vv0at_IWORK_a = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_ny . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_ny .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_ch . Head = ( ( _rtDW -> TransportDelay_IWORK_ch . Head
< ( _rtDW -> TransportDelay_IWORK_ch . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_ch . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_ch . Head == _rtDW -> TransportDelay_IWORK_ch . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_ch . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_ch . Tail , & _rtDW -> TransportDelay_IWORK_ch . Head ,
& _rtDW -> TransportDelay_IWORK_ch . Last , simTime - _rtP -> P_576 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_ch . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_ch . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_ch . Head ] = _rtB -> B_80_829_0 ; } if ( _rtDW ->
LastMajorTimeA_ny == ( rtInf ) ) { _rtDW -> LastMajorTimeA_ny = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_fn = _rtB -> B_80_855_0 ; } else if ( _rtDW ->
LastMajorTimeB_pl == ( rtInf ) ) { _rtDW -> LastMajorTimeB_pl = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_fn = _rtB -> B_80_855_0 ; } else if ( _rtDW ->
LastMajorTimeA_ny < _rtDW -> LastMajorTimeB_pl ) { _rtDW -> LastMajorTimeA_ny
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_fn = _rtB -> B_80_855_0 ; } else
{ _rtDW -> LastMajorTimeB_pl = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_fn =
_rtB -> B_80_855_0 ; } _rtDW -> vv0at_IWORK_ae = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_a5 . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_a5 .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_kh . Head = ( ( _rtDW -> TransportDelay_IWORK_kh . Head
< ( _rtDW -> TransportDelay_IWORK_kh . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_kh . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_kh . Head == _rtDW -> TransportDelay_IWORK_kh . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_kh . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_kh . Tail , & _rtDW -> TransportDelay_IWORK_kh . Head ,
& _rtDW -> TransportDelay_IWORK_kh . Last , simTime - _rtP -> P_592 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_kh . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_kh . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_kh . Head ] = _rtB -> B_80_852_0 ; } if ( _rtDW ->
LastMajorTimeA_m == ( rtInf ) ) { _rtDW -> LastMajorTimeA_m = ssGetTaskTime (
S , 0 ) ; _rtDW -> PrevYA_dd = _rtB -> B_80_878_0 ; } else if ( _rtDW ->
LastMajorTimeB_mm == ( rtInf ) ) { _rtDW -> LastMajorTimeB_mm = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_gv = _rtB -> B_80_878_0 ; } else if ( _rtDW ->
LastMajorTimeA_m < _rtDW -> LastMajorTimeB_mm ) { _rtDW -> LastMajorTimeA_m =
ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_dd = _rtB -> B_80_878_0 ; } else {
_rtDW -> LastMajorTimeB_mm = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_gv =
_rtB -> B_80_878_0 ; } _rtDW -> vv0at_IWORK_hw = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_do . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_do .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_ne . Head = ( ( _rtDW -> TransportDelay_IWORK_ne . Head
< ( _rtDW -> TransportDelay_IWORK_ne . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_ne . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_ne . Head == _rtDW -> TransportDelay_IWORK_ne . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_ne . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_ne . Tail , & _rtDW -> TransportDelay_IWORK_ne . Head ,
& _rtDW -> TransportDelay_IWORK_ne . Last , simTime - _rtP -> P_608 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_ne . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_ne . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_ne . Head ] = _rtB -> B_80_875_0 ; } if ( _rtDW ->
LastMajorTimeA_hd == ( rtInf ) ) { _rtDW -> LastMajorTimeA_hd = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_e5 = _rtB -> B_80_901_0 ; } else if ( _rtDW ->
LastMajorTimeB_n1 == ( rtInf ) ) { _rtDW -> LastMajorTimeB_n1 = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_cx = _rtB -> B_80_901_0 ; } else if ( _rtDW ->
LastMajorTimeA_hd < _rtDW -> LastMajorTimeB_n1 ) { _rtDW -> LastMajorTimeA_hd
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_e5 = _rtB -> B_80_901_0 ; } else
{ _rtDW -> LastMajorTimeB_n1 = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_cx =
_rtB -> B_80_901_0 ; } _rtDW -> vv0at_IWORK_fzr = 0 ; { real_T * * uBuffer =
( real_T * * ) & _rtDW -> TransportDelay_PWORK_gu . TUbufferPtrs [ 0 ] ;
real_T * * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_gu .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_a4b . Head = ( ( _rtDW -> TransportDelay_IWORK_a4b .
Head < ( _rtDW -> TransportDelay_IWORK_a4b . CircularBufSize - 1 ) ) ? (
_rtDW -> TransportDelay_IWORK_a4b . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_a4b . Head == _rtDW -> TransportDelay_IWORK_a4b . Tail )
{ if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_a4b . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_a4b . Tail , & _rtDW -> TransportDelay_IWORK_a4b . Head
, & _rtDW -> TransportDelay_IWORK_a4b . Last , simTime - _rtP -> P_624 ,
tBuffer , uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_a4b . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_a4b . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_a4b . Head ] = _rtB -> B_80_898_0 ; } if ( _rtDW ->
LastMajorTimeA_dm == ( rtInf ) ) { _rtDW -> LastMajorTimeA_dm = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYA_e0 = _rtB -> B_80_924_0 ; } else if ( _rtDW ->
LastMajorTimeB_ek == ( rtInf ) ) { _rtDW -> LastMajorTimeB_ek = ssGetTaskTime
( S , 0 ) ; _rtDW -> PrevYB_jt = _rtB -> B_80_924_0 ; } else if ( _rtDW ->
LastMajorTimeA_dm < _rtDW -> LastMajorTimeB_ek ) { _rtDW -> LastMajorTimeA_dm
= ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYA_e0 = _rtB -> B_80_924_0 ; } else
{ _rtDW -> LastMajorTimeB_ek = ssGetTaskTime ( S , 0 ) ; _rtDW -> PrevYB_jt =
_rtB -> B_80_924_0 ; } _rtDW -> vv0at_IWORK_ke = 0 ; { real_T * * uBuffer = (
real_T * * ) & _rtDW -> TransportDelay_PWORK_ei . TUbufferPtrs [ 0 ] ; real_T
* * tBuffer = ( real_T * * ) & _rtDW -> TransportDelay_PWORK_ei .
TUbufferPtrs [ 1 ] ; real_T simTime = ssGetT ( S ) ; _rtDW ->
TransportDelay_IWORK_cs . Head = ( ( _rtDW -> TransportDelay_IWORK_cs . Head
< ( _rtDW -> TransportDelay_IWORK_cs . CircularBufSize - 1 ) ) ? ( _rtDW ->
TransportDelay_IWORK_cs . Head + 1 ) : 0 ) ; if ( _rtDW ->
TransportDelay_IWORK_cs . Head == _rtDW -> TransportDelay_IWORK_cs . Tail ) {
if ( ! Simulation_v2_acc_rt_TDelayUpdateTailOrGrowBuf ( & _rtDW ->
TransportDelay_IWORK_cs . CircularBufSize , & _rtDW ->
TransportDelay_IWORK_cs . Tail , & _rtDW -> TransportDelay_IWORK_cs . Head ,
& _rtDW -> TransportDelay_IWORK_cs . Last , simTime - _rtP -> P_640 , tBuffer
, uBuffer , ( NULL ) , ( boolean_T ) 0 , false , & _rtDW ->
TransportDelay_IWORK_cs . MaxNewBufSize ) ) { ssSetErrorStatus ( S ,
"tdelay memory allocation error" ) ; return ; } } ( * tBuffer ) [ _rtDW ->
TransportDelay_IWORK_cs . Head ] = simTime ; ( * uBuffer ) [ _rtDW ->
TransportDelay_IWORK_cs . Head ] = _rtB -> B_80_921_0 ; } UNUSED_PARAMETER (
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
B_80_1059_0 ; _rtXdot -> vv0at_CSTATE = _rtB -> B_80_946_0 ; _rtXdot ->
TransferFcn_CSTATE = 0.0 ; _rtXdot -> TransferFcn_CSTATE += _rtP -> P_18 *
_rtX -> TransferFcn_CSTATE ; _rtXdot -> TransferFcn_CSTATE += _rtB ->
B_80_45_0 ; _rtXdot -> vv0at_CSTATE_h = _rtB -> B_80_948_0 ; _rtXdot ->
TransferFcn_CSTATE_d = 0.0 ; _rtXdot -> TransferFcn_CSTATE_d += _rtP -> P_34
* _rtX -> TransferFcn_CSTATE_d ; _rtXdot -> TransferFcn_CSTATE_d += _rtB ->
B_80_68_0 ; _rtXdot -> vv0at_CSTATE_f = _rtB -> B_80_950_0 ; _rtXdot ->
TransferFcn_CSTATE_h = 0.0 ; _rtXdot -> TransferFcn_CSTATE_h += _rtP -> P_50
* _rtX -> TransferFcn_CSTATE_h ; _rtXdot -> TransferFcn_CSTATE_h += _rtB ->
B_80_91_0 ; _rtXdot -> vv0at_CSTATE_hd = _rtB -> B_80_952_0 ; _rtXdot ->
TransferFcn_CSTATE_j = 0.0 ; _rtXdot -> TransferFcn_CSTATE_j += _rtP -> P_66
* _rtX -> TransferFcn_CSTATE_j ; _rtXdot -> TransferFcn_CSTATE_j += _rtB ->
B_80_114_0 ; _rtXdot -> vv0at_CSTATE_g = _rtB -> B_80_954_0 ; _rtXdot ->
TransferFcn_CSTATE_e = 0.0 ; _rtXdot -> TransferFcn_CSTATE_e += _rtP -> P_82
* _rtX -> TransferFcn_CSTATE_e ; _rtXdot -> TransferFcn_CSTATE_e += _rtB ->
B_80_137_0 ; _rtXdot -> vv0at_CSTATE_c = _rtB -> B_80_956_0 ; _rtXdot ->
TransferFcn_CSTATE_b = 0.0 ; _rtXdot -> TransferFcn_CSTATE_b += _rtP -> P_98
* _rtX -> TransferFcn_CSTATE_b ; _rtXdot -> TransferFcn_CSTATE_b += _rtB ->
B_80_160_0 ; _rtXdot -> vv0at_CSTATE_m = _rtB -> B_80_958_0 ; _rtXdot ->
TransferFcn_CSTATE_l = 0.0 ; _rtXdot -> TransferFcn_CSTATE_l += _rtP -> P_114
* _rtX -> TransferFcn_CSTATE_l ; _rtXdot -> TransferFcn_CSTATE_l += _rtB ->
B_80_183_0 ; _rtXdot -> vv0at_CSTATE_i = _rtB -> B_80_960_0 ; _rtXdot ->
TransferFcn_CSTATE_lu = 0.0 ; _rtXdot -> TransferFcn_CSTATE_lu += _rtP ->
P_130 * _rtX -> TransferFcn_CSTATE_lu ; _rtXdot -> TransferFcn_CSTATE_lu +=
_rtB -> B_80_206_0 ; _rtXdot -> vv0at_CSTATE_gg = _rtB -> B_80_962_0 ;
_rtXdot -> TransferFcn_CSTATE_n = 0.0 ; _rtXdot -> TransferFcn_CSTATE_n +=
_rtP -> P_146 * _rtX -> TransferFcn_CSTATE_n ; _rtXdot ->
TransferFcn_CSTATE_n += _rtB -> B_80_229_0 ; _rtXdot -> vv0at_CSTATE_a = _rtB
-> B_80_964_0 ; _rtXdot -> TransferFcn_CSTATE_c = 0.0 ; _rtXdot ->
TransferFcn_CSTATE_c += _rtP -> P_162 * _rtX -> TransferFcn_CSTATE_c ;
_rtXdot -> TransferFcn_CSTATE_c += _rtB -> B_80_252_0 ; _rtXdot ->
vv0at_CSTATE_b = _rtB -> B_80_966_0 ; _rtXdot -> TransferFcn_CSTATE_k = 0.0 ;
_rtXdot -> TransferFcn_CSTATE_k += _rtP -> P_178 * _rtX ->
TransferFcn_CSTATE_k ; _rtXdot -> TransferFcn_CSTATE_k += _rtB -> B_80_275_0
; _rtXdot -> vv0at_CSTATE_o = _rtB -> B_80_968_0 ; _rtXdot ->
TransferFcn_CSTATE_dt = 0.0 ; _rtXdot -> TransferFcn_CSTATE_dt += _rtP ->
P_194 * _rtX -> TransferFcn_CSTATE_dt ; _rtXdot -> TransferFcn_CSTATE_dt +=
_rtB -> B_80_298_0 ; _rtXdot -> vv0at_CSTATE_om = _rtB -> B_80_970_0 ;
_rtXdot -> TransferFcn_CSTATE_ez = 0.0 ; _rtXdot -> TransferFcn_CSTATE_ez +=
_rtP -> P_210 * _rtX -> TransferFcn_CSTATE_ez ; _rtXdot ->
TransferFcn_CSTATE_ez += _rtB -> B_80_321_0 ; _rtXdot -> vv0at_CSTATE_l =
_rtB -> B_80_972_0 ; _rtXdot -> TransferFcn_CSTATE_m = 0.0 ; _rtXdot ->
TransferFcn_CSTATE_m += _rtP -> P_226 * _rtX -> TransferFcn_CSTATE_m ;
_rtXdot -> TransferFcn_CSTATE_m += _rtB -> B_80_344_0 ; _rtXdot ->
vv0at_CSTATE_p = _rtB -> B_80_974_0 ; _rtXdot -> TransferFcn_CSTATE_na = 0.0
; _rtXdot -> TransferFcn_CSTATE_na += _rtP -> P_242 * _rtX ->
TransferFcn_CSTATE_na ; _rtXdot -> TransferFcn_CSTATE_na += _rtB ->
B_80_367_0 ; _rtXdot -> vv0at_CSTATE_bi = _rtB -> B_80_976_0 ; _rtXdot ->
TransferFcn_CSTATE_kl = 0.0 ; _rtXdot -> TransferFcn_CSTATE_kl += _rtP ->
P_258 * _rtX -> TransferFcn_CSTATE_kl ; _rtXdot -> TransferFcn_CSTATE_kl +=
_rtB -> B_80_390_0 ; _rtXdot -> vv0at_CSTATE_g2 = _rtB -> B_80_978_0 ;
_rtXdot -> TransferFcn_CSTATE_cj = 0.0 ; _rtXdot -> TransferFcn_CSTATE_cj +=
_rtP -> P_274 * _rtX -> TransferFcn_CSTATE_cj ; _rtXdot ->
TransferFcn_CSTATE_cj += _rtB -> B_80_413_0 ; _rtXdot -> vv0at_CSTATE_e =
_rtB -> B_80_980_0 ; _rtXdot -> TransferFcn_CSTATE_jq = 0.0 ; _rtXdot ->
TransferFcn_CSTATE_jq += _rtP -> P_290 * _rtX -> TransferFcn_CSTATE_jq ;
_rtXdot -> TransferFcn_CSTATE_jq += _rtB -> B_80_436_0 ; _rtXdot ->
vv0at_CSTATE_po = _rtB -> B_80_982_0 ; _rtXdot -> TransferFcn_CSTATE_g = 0.0
; _rtXdot -> TransferFcn_CSTATE_g += _rtP -> P_306 * _rtX ->
TransferFcn_CSTATE_g ; _rtXdot -> TransferFcn_CSTATE_g += _rtB -> B_80_459_0
; _rtXdot -> vv0at_CSTATE_al = _rtB -> B_80_984_0 ; _rtXdot ->
TransferFcn_CSTATE_ku = 0.0 ; _rtXdot -> TransferFcn_CSTATE_ku += _rtP ->
P_322 * _rtX -> TransferFcn_CSTATE_ku ; _rtXdot -> TransferFcn_CSTATE_ku +=
_rtB -> B_80_482_0 ; _rtXdot -> vv0at_CSTATE_j = _rtB -> B_80_986_0 ; _rtXdot
-> TransferFcn_CSTATE_o = 0.0 ; _rtXdot -> TransferFcn_CSTATE_o += _rtP ->
P_338 * _rtX -> TransferFcn_CSTATE_o ; _rtXdot -> TransferFcn_CSTATE_o +=
_rtB -> B_80_505_0 ; _rtXdot -> vv0at_CSTATE_gl = _rtB -> B_80_988_0 ;
_rtXdot -> TransferFcn_CSTATE_ct = 0.0 ; _rtXdot -> TransferFcn_CSTATE_ct +=
_rtP -> P_354 * _rtX -> TransferFcn_CSTATE_ct ; _rtXdot ->
TransferFcn_CSTATE_ct += _rtB -> B_80_528_0 ; _rtXdot -> vv0at_CSTATE_fc =
_rtB -> B_80_990_0 ; _rtXdot -> TransferFcn_CSTATE_hw = 0.0 ; _rtXdot ->
TransferFcn_CSTATE_hw += _rtP -> P_370 * _rtX -> TransferFcn_CSTATE_hw ;
_rtXdot -> TransferFcn_CSTATE_hw += _rtB -> B_80_551_0 ; _rtXdot ->
vv0at_CSTATE_ct = _rtB -> B_80_992_0 ; _rtXdot -> TransferFcn_CSTATE_ez3 =
0.0 ; _rtXdot -> TransferFcn_CSTATE_ez3 += _rtP -> P_386 * _rtX ->
TransferFcn_CSTATE_ez3 ; _rtXdot -> TransferFcn_CSTATE_ez3 += _rtB ->
B_80_574_0 ; _rtXdot -> vv0at_CSTATE_iz = _rtB -> B_80_994_0 ; _rtXdot ->
TransferFcn_CSTATE_i = 0.0 ; _rtXdot -> TransferFcn_CSTATE_i += _rtP -> P_402
* _rtX -> TransferFcn_CSTATE_i ; _rtXdot -> TransferFcn_CSTATE_i += _rtB ->
B_80_597_0 ; _rtXdot -> vv0at_CSTATE_n = _rtB -> B_80_996_0 ; _rtXdot ->
TransferFcn_CSTATE_p = 0.0 ; _rtXdot -> TransferFcn_CSTATE_p += _rtP -> P_418
* _rtX -> TransferFcn_CSTATE_p ; _rtXdot -> TransferFcn_CSTATE_p += _rtB ->
B_80_620_0 ; _rtXdot -> vv0at_CSTATE_fe = _rtB -> B_80_998_0 ; _rtXdot ->
TransferFcn_CSTATE_bi = 0.0 ; _rtXdot -> TransferFcn_CSTATE_bi += _rtP ->
P_434 * _rtX -> TransferFcn_CSTATE_bi ; _rtXdot -> TransferFcn_CSTATE_bi +=
_rtB -> B_80_643_0 ; _rtXdot -> vv0at_CSTATE_ge = _rtB -> B_80_1000_0 ;
_rtXdot -> TransferFcn_CSTATE_f = 0.0 ; _rtXdot -> TransferFcn_CSTATE_f +=
_rtP -> P_450 * _rtX -> TransferFcn_CSTATE_f ; _rtXdot ->
TransferFcn_CSTATE_f += _rtB -> B_80_666_0 ; _rtXdot -> vv0at_CSTATE_ht =
_rtB -> B_80_1002_0 ; _rtXdot -> TransferFcn_CSTATE_ii = 0.0 ; _rtXdot ->
TransferFcn_CSTATE_ii += _rtP -> P_466 * _rtX -> TransferFcn_CSTATE_ii ;
_rtXdot -> TransferFcn_CSTATE_ii += _rtB -> B_80_689_0 ; _rtXdot ->
vv0at_CSTATE_el = _rtB -> B_80_1004_0 ; _rtXdot -> TransferFcn_CSTATE_gd =
0.0 ; _rtXdot -> TransferFcn_CSTATE_gd += _rtP -> P_482 * _rtX ->
TransferFcn_CSTATE_gd ; _rtXdot -> TransferFcn_CSTATE_gd += _rtB ->
B_80_712_0 ; _rtXdot -> vv0at_CSTATE_iy = _rtB -> B_80_1006_0 ; _rtXdot ->
TransferFcn_CSTATE_k5 = 0.0 ; _rtXdot -> TransferFcn_CSTATE_k5 += _rtP ->
P_498 * _rtX -> TransferFcn_CSTATE_k5 ; _rtXdot -> TransferFcn_CSTATE_k5 +=
_rtB -> B_80_735_0 ; _rtXdot -> vv0at_CSTATE_cs = _rtB -> B_80_1008_0 ;
_rtXdot -> TransferFcn_CSTATE_ga = 0.0 ; _rtXdot -> TransferFcn_CSTATE_ga +=
_rtP -> P_514 * _rtX -> TransferFcn_CSTATE_ga ; _rtXdot ->
TransferFcn_CSTATE_ga += _rtB -> B_80_758_0 ; _rtXdot -> vv0at_CSTATE_bs =
_rtB -> B_80_1010_0 ; _rtXdot -> TransferFcn_CSTATE_jw = 0.0 ; _rtXdot ->
TransferFcn_CSTATE_jw += _rtP -> P_530 * _rtX -> TransferFcn_CSTATE_jw ;
_rtXdot -> TransferFcn_CSTATE_jw += _rtB -> B_80_781_0 ; _rtXdot ->
vv0at_CSTATE_co = _rtB -> B_80_1012_0 ; _rtXdot -> TransferFcn_CSTATE_in =
0.0 ; _rtXdot -> TransferFcn_CSTATE_in += _rtP -> P_546 * _rtX ->
TransferFcn_CSTATE_in ; _rtXdot -> TransferFcn_CSTATE_in += _rtB ->
B_80_804_0 ; _rtXdot -> vv0at_CSTATE_i2 = _rtB -> B_80_1014_0 ; _rtXdot ->
TransferFcn_CSTATE_cy = 0.0 ; _rtXdot -> TransferFcn_CSTATE_cy += _rtP ->
P_562 * _rtX -> TransferFcn_CSTATE_cy ; _rtXdot -> TransferFcn_CSTATE_cy +=
_rtB -> B_80_827_0 ; _rtXdot -> vv0at_CSTATE_mt = _rtB -> B_80_1016_0 ;
_rtXdot -> TransferFcn_CSTATE_h0 = 0.0 ; _rtXdot -> TransferFcn_CSTATE_h0 +=
_rtP -> P_578 * _rtX -> TransferFcn_CSTATE_h0 ; _rtXdot ->
TransferFcn_CSTATE_h0 += _rtB -> B_80_850_0 ; _rtXdot -> vv0at_CSTATE_ps =
_rtB -> B_80_1018_0 ; _rtXdot -> TransferFcn_CSTATE_es = 0.0 ; _rtXdot ->
TransferFcn_CSTATE_es += _rtP -> P_594 * _rtX -> TransferFcn_CSTATE_es ;
_rtXdot -> TransferFcn_CSTATE_es += _rtB -> B_80_873_0 ; _rtXdot ->
vv0at_CSTATE_i1 = _rtB -> B_80_1020_0 ; _rtXdot -> TransferFcn_CSTATE_fs =
0.0 ; _rtXdot -> TransferFcn_CSTATE_fs += _rtP -> P_610 * _rtX ->
TransferFcn_CSTATE_fs ; _rtXdot -> TransferFcn_CSTATE_fs += _rtB ->
B_80_896_0 ; _rtXdot -> vv0at_CSTATE_bu = _rtB -> B_80_1022_0 ; _rtXdot ->
TransferFcn_CSTATE_he = 0.0 ; _rtXdot -> TransferFcn_CSTATE_he += _rtP ->
P_626 * _rtX -> TransferFcn_CSTATE_he ; _rtXdot -> TransferFcn_CSTATE_he +=
_rtB -> B_80_919_0 ; _rtXdot -> vv0at_CSTATE_fw = _rtB -> B_80_1024_0 ;
_rtXdot -> TransferFcn_CSTATE_a = 0.0 ; _rtXdot -> TransferFcn_CSTATE_a +=
_rtP -> P_642 * _rtX -> TransferFcn_CSTATE_a ; _rtXdot ->
TransferFcn_CSTATE_a += _rtB -> B_80_940_0 ; _rtXdot -> Integrator1_CSTATE =
_rtB -> B_80_11_0 ; } static void mdlInitializeSizes ( SimStruct * S ) {
ssSetChecksumVal ( S , 0 , 2765158758U ) ; ssSetChecksumVal ( S , 1 ,
2702910282U ) ; ssSetChecksumVal ( S , 2 , 1763038455U ) ; ssSetChecksumVal (
S , 3 , 2234154383U ) ; { mxArray * slVerStructMat = NULL ; mxArray *
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
