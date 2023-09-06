; veneer_f.s - float add/sub/mul/div
;
; Copyright (C) Advanced RISC Machines Limited, 1994. All rights reserved.
;
; RCS $Revision: 1.23 $
; Checkin $Date: 1998/03/19 15:04:12 $
; Revising $Author: wdijkstr $

        GET     fpe.s

a       RN 0
b       RN 1
tmp     RN 12
mask    RN 12
expa    RN 2
expb    RN 3
exp     RN expa
sign    RN expb
shift   RN expb
res     RN expb
guess   RN 14
num     RN b
den     RN a
div     RN res


        [ {ARCHITECTURE} = "3M" :LOR: \
          {ARCHITECTURE} = "4" :LOR: \
          {ARCHITECTURE} = "4T"
LONG_MULTIPLY EQU 1
        |
LONG_MULTIPLY EQU 0
        ]


;===========================================================================
;
; fadd/fsub/frsb
; ==============
;
;   Upon entry the signs are checked, and if they are not equal, control is
;   given to the inverse routine while negating the second operand. I.e.
;   fadd(+A,-B) -> fsub(+A,+B) and fsub(-A,+B) -> fadd(-A,-B). After the
;   check the signs are known to be equal, and we have a magnitude addition
;   or magnitude subtraction to perform.
;   Frsb (B - A) is implemented by negating both signs on entry before
;   falling into fsub. Its use is mainly intended for code size optimization.
;
;   The operands are sorted to ensure that A >= B. This enables many checks
;   to be simplified: (A == 0) || (B == 0) reduces to (B == 0). The
;   calculations are also simpler: only operand B needs to be shifted right 
;   by the exponent difference.
;   Unsigned arithmetic is used to compare the packed numbers, since we want
;   to have the operand with the largest magnitude as operand A.
;
;   The operands are decoded into a fraction part and a sign + exponent part.
;   As the signs of the operands are known to be equal and the operands
;   are ordered, the sign of the result is the sign of the first operand.
;   Since the result exponent cannot change much (by at most 1 in fadd, 
;   and at most 23 (though usually a lot less) in fsub unless the two
;   operands were equal), the sign and exponent are not separated.
;   Instead of decoding the exponent of the second operand, the exponent
;   difference is calculated directly. The exponent difference is the 
;   shift amount for B ( >= 0 since A >= B implies exp A >= exp B).
;
;   Special cases, namely zeros, infinities, denormals and Not-a-Numbers
;   (NaNs) are checked for on entry. If one of the operands is special, a
;   jump is made to handle these cases out of line. This keeps the overhead 
;   for the general case as low as possible. Because the operands are sorted, 
;   only 2 checks need to be made: operand A is checked for NaN/infinity
;   (exp = 255), while operand B is checked for zero/denormal (exp = 0).
;
;   The fractions are decoded by shifting the operands left while ORing
;   the implicit leading bit (fractions are in the high 24 bits).
;
;   Magnitude addition:
;
;   For a magnitude addition, the operands are added with the smallest one
;   shifted right by the exponent difference. The result mantissa might be
;   larger than 2.0, and is renormalized if necessary (max 1 right shift
;   needed). When the exponent is recombined with the mantissa using an
;   addition, the mantissa's units bit will adjust the exponent by +1; to
;   make the overall adjustment come out right, the exponent is first
;   adjusted by -1 (if mantissa was < 2.0) or 0 (if mantissa was >= 2.0).
;
;   Magnitude subtraction:
; 
;   For a magnitude subtraction, operand B is subtracted from A, after being
;   shifted right by the exponent difference. The result cannot be negative
;   since A >= B, but it can result in an unnormalized number (as the high
;   bits of A and B might cancel out).
;
;   The common case results in the exponent being adjusted by 0 or -1, 
;   this is when the MSB of the result is still set, or when the next bit 
;   is set. In the last case underflow can occur if the exponent was 1.
;   These cases require normal rounding (if no underflow occurred).
; 
;   When 2 or more leading bits of the result are clear, the result must be
;   normalized. If the resulting exponent is <= 0, it has underflowed to 
;   a denormal. 
;   The result is exact, because 2 or more leading zeros means that the 
;   exponent difference cannot have been more than 1; since we shifted 
;   left by at least 2 bits, all significant bits of the result must lie 
;   within the 24-bit mantissa, and the round bit is always zero. So no 
;   rounding is required in this case.
;
;   There is one complication in doing the subtraction using a
;   
;   SUB res, a, b, LSR shift
;
;   instruction: some of the bits of b which are shifted out during the 
;   subtraction might be non-zero. This means that the result might be 1 
;   too large. We prove that there is only one case where we need to 
;   correct the 24 bit result by enumeration of all possibilities. 
;   We assume the result is in the high 24 bits (after being normalised
;   by shifting left). Then bit 7 is the round bit, and bit 6:0 are part 
;   of the guard bits. We only consider the case where some nonzero bits 
;   are shifted out of b (otherwise the result was already correct).
;
;   Round Guard bits 6:0
;     0   0   : OK (correction would give (1, > 0) which always rounds up)
;     0   > 0 : OK (subtract of one doesn't alter 24 bit result)
;     1   0   : need to subtract one to prevent round to even
;     1   > 0 : OK (subtract of one doesn't alter 24 bit result)
;
;   So if the round bit is set and the partial guard bits are zero, 
;   the result would be incorrect if normal rounding is applied:
;   the result is rounded to even, while it should be rounded down.
;   Instead of correcting this immediately after the SUB, we can 
;   handle this case in the code that implements the round to even. 
;   At that point we know the round bit is set and the first 6 guard bits 
;   are zero. If the bits shifted out of b are zero too, we round to even as
;   normal, otherwise we decrement the result.
;
;   Rounding:
;
;   In the rounding stage, the exponent is recombined with the fraction
;   which leading bit is still set (if it is normalized). This causes the
;   exponent to increment by one. To compensate, the exponent will normally
;   have been decremented in an earlier stage.
;   If the exponent is to be decremented by 1, the fraction is shifted
;   right using an arithmetic shift. Since its MSB is set, this causes
;   1 to be subtracted from the exponent, rather than added. This is only
;   possible if the leading bit is set.
;   A third approach is used in fsub, where the leading bit is shifted
;   out. This means no exponent adjustment is necessary. 
;
;   The round bit is bit 7, since the result is in the high 24 bits after
;   normalisation. When the result is shifted right to remove the low
;   8 bits, the carry contains the round bit. 
;   Fraction B contains the guard bits, and they can be extracted from 
;   it by shifting left by 25 - exponent difference. Note that it is
;   only necessary to look at the guard bits if the round bit is set:
;   if all guard bits are zero, we need to round to even.
;
;   Unfortunately, calculating the guard bits is expensive: if normalisation
;   is needed (because the result is >= 2.0 or < 1.0), the result is 
;   shifted, and so must the guard bits (either by shifting B or by 
;   adjusting the exponent difference).
;   However, since we have 8 bits extra precision during the calculation, 
;   we can use the low 7 bits as a first approximation. If they are nonzero,
;   we can just round up, if they are zero, the remaining guard bits
;   must be checked using the right shift. Since there is a reasonable
;   chance that the bits 6:0 are non-zero if the round bit is set, we save
;   a few cycles.
;
;   Round to even is implemented by always rounding upwards, and then 
;   clearing the LSB in the infrequent case that all guard bits are zero.
;   Thus if the result is odd, it will be rounded up, while an even result
;   will not.
;
;   While rounding, the fraction may become too large (>= 2.0), at which
;   time the exponent must be incremented and the fraction shifted right.
;   However, this doesn't need extra code, since exponent and fraction are
;   already combined: the overflow from the fraction increments the exponent
;   and the fraction becomes zero as required. Note that this means a
;   denormalized number may become normalized while rounding; also note that
;   a non-overflowed result may overflow during rounding.
;
;   For a magnitude addition, overflow is checked after rounding by adding 1
;   to the exponent of the result. If the result overflowed, the sign bit inverts
;   (overflowed exponent is 255, and 255+1 negates the sign bit). Note that
;   overflow can only occur after renormalization, or during rounding, but
;   not in both (the maximum exponent adjustment is +1 either during rounding
;   or after the magnitude addition). 
;   Overflow cannot occur in a magnitude subtraction.
;
;   Uncommon operands:
;
;   If the largest operand is a NaN, control is passed to fcheck_NaN2 which
;   raises an Invalid Operation Exception if one of the operands is a
;   signalling NaN, or alternatively returns the quiet NaN.
;
;   Infinities are returned unaltered (ie. inf + B = inf, inf - B = inf), 
;   but in fsub an Invalid Operation exception is raised for inf - inf.
;
;   If the smallest operand is a zero, the other operand is returned - i.e.
;   A + 0 -> A, A - 0 -> A. A special case is -0 - -0 -> +0 (since fplib
;   only implements round to nearest/even).
;
;   Denormalized operands:
;
;   Denormalized numbers are handled by decoding an unnormalized fraction
;   with exponent 1. This is to make up for the hidden bit which is clear in
;   denormalized numbers. The smallest non-denormal number is 1.00..00 with
;   exp = 1, while the largest number not greater than this is the largest 
;   denormal: 0.11..11 with exp = 1. Thus the representation of denormals with
;   exp = 1 makes it possible to mix normalised and denormalised numbers.
;
;   Since we have already decoded the (initial) result exponent and the exponent 
;   difference, the adjustments to be made are dependent of whether A is a 
;   denormal as well as B. If both A and B are denormalised, the exponent 
;   difference is correct (zero) but the resulting exponent should be 1. 
;   If only B is denormalised, the resulting exponent is OK, but the exponent 
;   difference is one too large (exp B was zero, but becomes one, so 
;   exp A - exp B must be decremented).   
;
;   The comments generally assume that a is normalized and thus the
;   leading bit of a is set. The code however works if the units bits
;   is clear. For example during rounding it is assumed that the resulting
;   exponent is incremented by the leading bit (after being decremented
;   to make up for this). If two denormals are added, then exp = 1, and will
;   be decremented. The leading bit is either clear, and a denormal with 
;   exp = 0 is returned, or the leading bit has been set (result larger than
;   1.0), thus a normalised number with exp = 1 is returned. 
;   In fsub, subtraction of denormals always results in renormalisation
;   of at least one bit. A special case that checks for exp = 1 bypasses the
;   normal renormalisation, and jumps directly to flt_underflow (thus 
;   speeding up denormal calculations).
;   
;   Denormalised numbers than thus be handled using normal addition or 
;   subtraction. The result can be a denormalized number or a normalized number.
;
;===========================================================================
;
; fadd timing (for SA1.1)
; =======================
; signs A & B unequal   : fsub + 4 clk
; A or B NaN            : 19 clk + invalid operation
; A + inf, inf + B      : 18 clk
; A + 0, 0 + B          : 17 clk
; denorm opnd A or B    : +9 clk
; normal case (fast code)
;   no carry, no round  : 22 clk
;   no carry, round     : 26 clk
;   no carry, round even: 32 clk
;   carry, no round     : 26 clk
;   carry, round        : 26 clk
;   carry, round even   : 35 clk
; normal case (short code)
;   no round            : 25 clk
;   round               : 25 clk
;   round even          : 31 clk
; overflow              : 34/37/33 + overflow
;
; fsub timing (for SA1.1)
; =======================
; sign A & B unequal    : fadd + 4 clk
; A or B NaN, inf-inf   : 19 clk + invalid operation
; A - inf,  inf - B     : 21 clk
; A - 0, 0 - B          : 20 clk
; denorm opnd A or B    : +11 clk
; renormalise 0 bit
;   no round            : 24 clk
;   round               : 26 clk
;   round even          : 32 clk
; renormalise 1 bit
;   no round            : 25 clk
;   round               : 27 clk
;   round even          : 34 clk
; renormalise >= 2 bit
;   renorm 2 .. 9 bit   : 40 clk
;   renorm > 9 bit      : 44 clk
;   underflow (denorm)  : 29 clk + underflow
; zero result           : 28 clk
;
; frsb timing (for SA1.1)
; =======================
; sign A & B unequal    : fadd + 6 clk
; sign A & B equal      : fsub + 2 clk
;
;===========================================================================


        ; *** SUSPECTED INEFFICIENCY ***
        ; Cycle counts for _fsub sign-handling with this implementation:
        ;   Signs same: 2   Signs differ: 6 (non-Thumb), 6 (Thumb)
        ; If conditional EOR inserted in _fsub code with branch directly to
        ; _fadd1, cycle counts become:
        ;   Signs same: 3   Signs differ: 4 (non-Thumb), 4 (Thumb)
        ; The latter is better provided signs differ at least a third of the
        ; time. (And I would expect them to differ close to half the time in
        ; real code - though possibly not in simple benchmarks!)


        [ :DEF: add_s

        CodeArea |FPL$$fadd|

        Export  _fadd
        Export  _fsub
        Export  _frsb
        IMPORT  __fp_exception
        IMPORT  __fp_fcheck_NaN2
        IMPORT  __flt_underflow
        IMPORT  __flt_overflow

        CODE_32

_faddn
        ; We branch to here if _fsub discovers that the signs are different.
        ; This causes operand B's sign to be reversed and the operation to
        ; become an addition. If we are generating the special Thumb entry
        ; points, we branch around them and around _fadd's test of the signs
        ; (which is now certain to find that they are identical). Otherwise,
        ; it is cheaper to fall through into _fadd's test of the signs than
        ; it is to branch around it.
        EOR     b, b, #1 << 31
        [ THUMB
        B       _fadd1
        ]
_fadd   EnterWithLR_16
        ; If the signs are unequal, treat this as a subtraction with second
        ; operand's sign reversed.
        TEQ     a, b
        BMI     _fsubn
_fadd1
        ; If we get here, we're adding operands with equal signs (i.e. a
        ; magnitude addition). First thing to do is put operands in
        ; magnitude order, so that a >= b.
        SUBS    tmp, a, b
        SUBLO   a, a, tmp
        ADDLO   b, b, tmp
        ; Decode result exponent, and calculate the exp difference
        MOV     exp, a, LSR #23         ; exp = sign<<8 + exponent
        SUB     shift, exp, b, LSR #23  ; shift = 0..254 (sign bits cancel)
        ; Filter out uncommon cases
        MOV     tmp, #255 << 24
        TST     tmp, b, LSL #1          ; check for denorm/zero
        TEQNE   tmp, exp, LSL #24       ; check for inf/NaN
        BEQ     fadd_uncommon           ; handle zeros/denorms/infs/NaNs
        ; Decode fractions and OR in the leading ones
        MOV     tmp, #1 << 31
        ORR     a, tmp, a, LSL #8       ; a = 1.frac_a
        ORR     b, tmp, b, LSL #8       ; b = 1.frac_b
fadd_doadd
        ; We get here with:
        ;   Operands known to be numeric rather than zero/infinity/NaN;
        ;   a = mantissa of larger operand (in high 24 bits);
        ;   b = mantissa of smaller operand (in high 24 bits);
        ;   exp = result sign/exponent (in low 9 bits)
        ;   shift = exponent difference;
        ; Start with the alignment shift and addition.
        ADDS    tmp, a, b, LSR shift    ; CS if a >= 2.0

        ; Now do normalization (by 1 bit if addition overflowed), exponent
        ; adjustment (overall by 0 if addition didn't overflow, 1 if it
        ; did), rounding and overflow detection. Two code sequences are
        ; used for this, depending on space/time trade-offs.

        [ CodeSize = 0                  ; CODE OPTIMISED FOR SPEED

        BCS     fadd_carry              ; Split out mantissa overflow case
        ADD     exp, exp, #-1           ; Adjust exp for leading bit
        MOVS    a, tmp, LSR #8          ; Round down, setting C to round bit
        ADC     a, a, exp, LSL #23      ; Combine sign, exp & fraction, and
                                        ;  round (exp adjusted by units bit)
        ReturnToLR CC                   ; If round bit is 0, result is
                                        ;  correct and overflow didn't happen

        ; Round bit is 1 (C=1) - we have rounded up, but need to check guard
        ; bits. We first do a quick check on 7 guard bits, if they are non-
        ; zero and overflow didn't occur, we can return the result.        
        TST     tmp, #127               ; Test 7 guard bits, if Z=0 don't
                                        ;  round to even (Carry bit still set)
        MOV     tmp, a, LSL #1          ; Prepare for overflow check 
        CMPNE   tmp, #255 << 24         ; If not round to even: do overflow
        ReturnToLR CC                   ;  check and return if not overflowed
fadd_roundeven       
        ; Here we've got overflow, round to even or both. We pretend that
        ; we didn't get overflow, and always perform the round to even 
        ; step. This is OK since we don't care about the result if we
        ; overflow (the exception handler will return an infinity).

        ; Here we round to even using the guard bits beyond the first 7.
        RSB     shift, shift, #32       ; Shift to get guard bits
        MOVS    b, b, LSL shift         ;  shifted out of b during add.
        BICEQ   a, a, #1                ; Round to even if guard bits zero
        
        CMP     tmp, #255 << 24         ; Check for overflow
        ReturnToLR CC                   ; Return if not overflowed
        MOV     ip, #OVF_bit :OR: AddFn ; Sign in a is correct
        B       __fp_exception


fadd_carry                              ; Result >= 2.0
        MOV     a, tmp, RRX             ; Restore leading bit
        MOVS    a, a, LSR #8            ; Round down, setting C to round bit
                                        ;  and Z=1 (since result is non-zero)
        ADC     a, a, exp, LSL #23      ; Combine sign, exp & fraction, and
                                        ;  round (exp adjusted by units bit)
        ; Note mantissa cannot have overflowed during rounding: if it is all
        ; 1s before rounding, both operand mantissas must have been all 1s
        ; and the exponent difference 0 - which implies the round bit was 0.
        
        TSTCS   tmp, #255               ; Test 8 guard bits if round bit set
        ; Now Z=1 if we need to round to even                              
        MOV     tmp, a, LSL #1          ; Prepare for overflow check
        CMPNE   tmp, #255 << 24         ; If not round to even: do overflow
        ReturnToLR CC                   ;  check and return if not overflowed
        B       fadd_roundeven
        
        
        |                               ; CODE OPTIMISED FOR SPACE


        MOVCS   tmp, tmp, RRX           ; Renormalize a
        SUBCC   exp, exp, #1            ; Adjust exponent by -1
        MOVS    a, tmp, LSR #8          ; Round down, setting C to round bit
        ADC     a, a, exp, LSL #23      ; Combine sign, exp & fraction, and
                                        ;  round (exp adjusted by units bit)

        TSTCS   tmp, #127               ; Test 7 guard bits if round bit set
        ; Now Z=1 if we need to round to even                              
        MOV     tmp, a, LSL #1          ; Prepare for overflow check
        CMPNE   tmp, #255 << 24         ; If not round to even: do overflow
        ReturnToLR CC                   ;  check and return if not overflowed
        
        ; Here we've got overflow, round to even or both. We pretend that
        ; we didn't get overflow, and always perform the round to even 
        ; step. This is OK since we don't care about the result if we
        ; overflow (the exception handler will return an infinity).

        ; Here we round to even using the guard bits beyond the first 7.
        ; We need to test one extra guard bit, since we have shifted the
        ; result if it was >= 2.0, thus losing a guard bit.
        RSB     shift, shift, #31       ; Shift to get guard bits shifted
                                        ;  out of b plus one extra bit.
        MOVS    b, b, LSL shift         ;  Note that we might test this
                                        ;  bit twice if round bit is clear.
        BICEQ   a, a, #1                ; Round to even if guard bits zero

        CMP     tmp, #255 << 24         ; Check for overflow 
        ReturnToLR CC                   ; Return if not overflowed
        SUB     a, a, #192 << 23        ; Rebias the exponent        
        B       __flt_overflow

        ]   ; CodeSize = 0


fadd_uncommon
        ; Handle zeros, denorms, infinities and NaNs. We arrive here knowing
        ; that a and b have the same sign and are in a >= b magnitude order,
        ; that exp contains the sign and exponent of a in its low 9 bits,
        ; and that tmp == (255 << 24).
        TEQ     tmp, exp, LSL #24       ; filter out NaN and infinities (EQ)
        BEQ     fadd_inf_NaN
        ; Fast check for zeros (b is zero or denormal)
        MOVS    b, b, LSL #8            ; b = 0.frac_b (EQ if zero)
        ReturnToLR EQ                   ; return A + 0 = A
        ; b is denormalized, a might be
        MOV     a, a, LSL #8            ; a = 0.frac_a
        TST     exp, #255               ; a denormalized? (exp == 0 -> EQ)
        ORRNE   a, a, #1 << 31          ; If not, set leading bit of a, and 
        SUBNE   shift, shift, #1        ;  adjust exponent difference
        ADDEQ   exp, exp, #1            ; If so, both operands are denorms
                                        ;  and we need to adjust exp
        B       fadd_doadd


fadd_inf_NaN
        ; Handle infinities and NaNs - a is infinite or NaN, b might be
        MOVS    tmp, a, LSL #9          ; EQ if an infinity, NE if a NaN
        ReturnToLR EQ                   ; infinity + non-NaN = infinity
        MOV     ip, #IVO_bit :OR: AddFn
        B       __fp_fcheck_NaN2


_frsb   EnterWithLR_16
        ; This is implemented by reversing both operands' signs, then using
        ; the _fsub code. For Thumb we branch directly to fadd after negating
        ; a if the signs are equal, otherwise negate b too and jump to fsub1. 
         EOR     a, a, #1 << 31
         [ THUMB
         TEQ     a, b
         BPL     _fadd1
         ]
_fsubn
        ; We branch to here if _fadd discovers that the signs are different.
        ; This causes operand B's sign to be reversed and the operation to
        ; become a subtraction. If we are generating the special Thumb entry
        ; points, we branch around them and around _fsub's test of the signs
        ; (which is now certain to find that they are identical). Otherwise,
        ; it is cheaper to fall through into _fsub's test of the signs than
        ; it is to branch around it.
        EOR     b, b, #1 << 31
        [ THUMB
        B       _fsub1
        ]
_fsub   EnterWithLR_16
        ; If the signs are unequal, treat this as an addition with the
        ; second operand's sign reversed.
        TEQ     a, b
        BMI     _faddn
_fsub1
        ; If we get here, we're subtracting operands with equal signs (i.e.
        ; a magnitude subtration). First thing to do is put operands in
        ; magnitude order, so that a >= b. However, if they are swapped, we
        ; must also negate both of them, since A - B = (-B) - (-A).
        SUBS    tmp, a, b
        EORLO   tmp, tmp, #1 << 31      ; Negate both operands
        SUBLO   a, a, tmp
        ADDLO   b, b, tmp
        ; Decode result exponent, and calculate the exp difference
        MOV     exp, a, LSR #23         ; exp = sign<<8 + exponent
        SUB     shift, exp, b, LSR #23  ; shift = 0..254 (sign bits cancel)
        ; Filter out uncommon cases
        MOV     tmp, #255 << 24
        TST     tmp, b, LSL #1          ; check for denorm/zero
        TEQNE   tmp, exp, LSL #24       ; check for inf/NaN
        BEQ     fsub_uncommon           ; handle zeroes/denorms/infs/NaNs

        ; Decode fractions and OR in the leading ones
        MOV     tmp, #1 << 31
        ORR     a, tmp, a, LSL #8
        ORR     b, tmp, b, LSL #8
        
fsub_dosub
        ; We get here with:
        ;   Operands known to be numeric rather than zero/infinity/NaN;
        ;   a = mantissa of larger operand (in high 24 bits);
        ;   b = mantissa of smaller operand (in high 24 bits);
        ;   exp = result sign/exponent (in low 9 bits)
        ;   shift = exponent difference;
        ; Start with the alignment shift and subtraction - note that
        ; the result might be 1 too large as we might shift out a nonzero
        ; part of b. We correct this during rounding if necessary.
        ; Split into cases according to whether the normalization shift
        ; required is by 0, 1 or >= 2 bits.
        SUBS    tmp, a, b, LSR shift    ; MI if high bit set
        BMI     fsub_renorm_0           ; high bit set -> no renormalisation                             
        ; High bit of result clear - we need at least one bit renormalisation.
        ; Check whether underflow occurred or we need >= 2 bits renormalisation
        TST     exp, #254               ; underflow if exp == 1 (exp nonzero)
                                        ; EQ if underflowed, always PL
        MOVNES  a, tmp, LSL #1          ; check whether 2nd bit is cleared (PL)
        BPL     fsub_renormalise        ; underflow or >= 2 bits renormalisation

fsub_renorm_1
        ; Normalization by one bit is required. We get here with 'a' holding
        ; the result in the high 24 bits (leading bit set), and know that
        ; exp >= 1, so that underflow cannot occur. The exponent is 
        ; decremented by using a signed shift right of a.
        MOVS    a, a, ASR #8            ; Result mantissa now in low 24 bits
                                        ;  of a, with top 9 bits all = 1
        ADC     a, a, exp, LSL #23      ; Combine sign, exp & fraction and
                                        ;  round while decrementing exp
        ReturnToLR CC                   ; Result is OK if rounding down
        TST     tmp, #63                ; Test 6 guard bits
        ReturnToLR NE                   ; No round to even - return                               
        B       fsub_roundeven 


fsub_renorm_0
        ; No normalization is needed. The 24 bit result is in a. Since the
        ; leading bit is set, we need to decrement exp in order to make up
        ; for the leading bit exponent & fraction are recombined.
        MOVS    a, tmp, LSR #8          ; Round down, setting C to round bit
                                        ;  and Z=1 (since result is non-zero)
        SUB     exp, exp, #1            ; adjust exp for leading bit 
        ADC     a, a, exp, LSL #23      ; Combine sign, exp & fraction and
                                        ;  round according to round bit
        ReturnToLR CC                   ; Done if we're rounding down
        TST     tmp, #127               ; Test 7 guard bits
        ReturnToLR NE                   ; No round to even - return
fsub_roundeven        
        RSB     shift, shift, #32       ; Shift to get guard bits
        MOVS    b, b, LSL shift         ;  shifted out of b during add.
        SUBNE   a, a, #1                ; Undo round up
        BICEQ   a, a, #1                ; Round to even if guard bits zero
        ReturnToLR                                  


fsub_renormalise
        ; We arrive here if renormalization by >= 2 bits is required,
        ; or if the result is denormal (exp = 1 and >= 1 bit renormalisation),
        ; or if the result is zero. The Z-flag is set in the last 2 cases.
        ; In all cases, the result is known to be exact. This is trivial for 
        ; zero and denormal results. Otherwise, because of the 2 top bits of 
        ; the result are clear, the exponent difference can only be 0 or 1.
        ; This means that after renormalisation the round bit is always zero.
        ; Note also that because of the small exponent difference there are 
        ; no bits of b shifted out, no special correction is needed.
        ; So we needn't worry about rounding.
        
        MOV     sign, exp, LSR #8       ; For underflow check
        BEQ     fsub_underflow          ; Split off zero/denormals
        ; We need to renormalise by >= 2 bits.
        ; The 22 bit nonzero result is in bit 30:9 of a (with binary point 
        ; between bit 31 and 32), and needs to be normalised so that bit 23 
        ; is set.
        ; During renormalisation the exponent is updated with the amount
        ; shifted: if bit 30 was set, 7 is added to exp. However, this is
        ; the case where only 2 bits need to be renormalised, so the final
        ; exponent adjustment is 7 + 2 + 1 = 10 (1 extra to make up for the
        ; leading bit) - thus causing the final exp to be decremented with 2.
        ; Note that while renormalising we can underflow the exponent,
        ; this is handled as a special case.
   
        MOVS    tmp, a, LSR #23         ; bit 23..30 set?
        BNE     fsub_renorm_small
        MOVEQ   a, a, LSL #8
        SUBEQ   exp, exp, #8
        MOVS    tmp, a, LSR #23         ; bit 23..30 set?
        MOVEQ   a, a, LSL #8
        SUBEQ   exp, exp, #8
fsub_renorm_small        
        TST     a, #15 << 27
        MOVNE   a, a, LSR #4
        ADDNE   exp, exp, #4
        TST     a, #3 << 25
        MOVNE   a, a, LSR #2
        ADDNE   exp, exp, #2
        CMP     a, #1 << 24
        MOVCS   a, a, LSR #1
        ADC     exp, exp, #-10
        ; exp is one too small (because of the units bit compensation). 
        ; So it is negative precisely if the true exponent has dropped below
        ; 1 (causing the sign bit in bit 8 to be inverted).
        TEQ     sign, exp, LSR #8       ; Exponent underflow?
                                        ;  (signs differ if so)
        ADDEQ   a, a, exp, LSL #23      ; No rounding necessary
        ReturnToLR EQ

        ; We underflowed to a denormalized number - use a standard
        ; routine for this purpose. Arguments are almost right at present:
        ;   a (R0) holds normalized mantissa, also interpretable as floating
        ;          point number with correct fraction, +ve sign, minimum
        ;          exponent for normalized floating point number.
        ;   exp (R2) holds minus the denormalization amount in its low 8
        ;          bits, and needs to contain plus the denormalization
        ;          amount in the same bits.
        ;   sign (R3) holds required sign in bit 0.
        RSB     exp, exp, #0
        B       __flt_underflow


fsub_underflow
        ; Result is denormalised or zero. Return +0 if zero, otherwise
        ; report underflow.
        ; Input: exp = 1 + sign<<8, tmp = 24bit fraction with top bit cleared,
        ;     sign = exp >> 8
        MOVS    a, tmp, LSR #7          ; Test whether result is zero
        ReturnToLR EQ                   ; Return +0 if result is zero
        B       __flt_underflow         ; Handle underflow


fsub_uncommon
        ; Handle zeros, denorms, infinities and NaNs. We arrive here knowing
        ; that a and b have the same sign and are in a >= b magnitude order,
        ; that exp contains the sign and exponent of a in its low 9 bits,
        ; and that tmp == (255 << 24).
        TEQ     tmp, exp, LSL #24       ; EQ if NaN/infinity
        BEQ     fsub_inf_NaN
fsub_denorm                             ; b is zero or denormal
        ; Check whether a or b is zero - fast case. At same time, produce
        ; b shifted left in preparation for likely transfer to fsub_dosub.
        MOVS    tmp, a, LSL #1
        MOVEQ   a, #0                   ; -0 - -0 = +0 special case
        MOVS    b, b, LSL #8            ; b = 0.frac_b (EQ if zero)
        ReturnToLR EQ                   ; return a - 0 = a
        ; b is denormalized, a might be
        MOV     a, a, LSL #8            ; a = 0.frac_a
        TST     exp, #255               ; a denormalized? (exp == 0 -> EQ)
        ORRNE   a, a, #1 << 31          ; If not, set leading bit of a, and 
        SUBNE   shift, shift, #1        ;  adjust exponent difference
        ADDEQ   exp, exp, #1            ; If so, both operands are denorms
                                        ;  and we need to adjust exp
        B       fsub_dosub


fsub_inf_NaN
        ; Handle infinities and NaNs - a is infinite or a NaN, b might be
        MOVS    tmp, a, LSL #9          ; a NaN? (NE)
        MOV     ip, #IVO_bit :OR: AddFn
        BNE     __fp_fcheck_NaN2
        CMP     a, b                    ; a is infinite, b too? (EQ)
        ReturnToLR NE                   ; no, return infinite
        B       __fp_exception          ; yes, a & b infinite
                                        ;  => generate IVO

        ]

;===========================================================================
; Fmul / fdiv:
;
;   On entry the special cases inf/NaN/denorm/zero are filtered out while
;   decoding the exponents. The result sign is calculated as the exclusive 
;   OR of the operand signs. The exponents are decoded to bits 16..23 while
;   the result sign is placed in bit 8 of the exponent. This way a register 
;   (to store the sign) is saved, while the exponent and sign are still
;   available seperately. The exponent and sign can be recombined in the 
;   low 9 bits using a single instruction including an optional +1 
;   adjustment to the exponent:
;
;     ADC exp, exp, exp, ASR #16 (signed shifts since exponent is signed).
;
;   Then the fractions are decoded to their explicit leading one form,
;   the exponents are added (fmul) or subtracted (fdiv), and rebiased so
;   that the resulting exponent is one too small. After combination of 
;   exponent and fraction, the exponent is incremented by the leading bit 
;   of the fraction. The exponent range is -171..383 for fmul, and
;   -150..405 for fdiv, so both need overflow and underflow checks.
;
;   Fmul:
;
;   In fmul, there are two algorithms to do the 24x24 bit multiplication,
;   either by a single MULL instruction or using 3 8x24 bit MLAs.
;   In the 8x24 case, the last 8 bits of the 32 bit results are shifted out,
;   with the flags updated if these bits are non-zero. The MLAs can't
;   overflow since the maximum value is 255 * (2^24 - 1) + (2^24 - 1) =
;   2^32 - 256.
;
;   In both cases the end result is a 32 bit number between 1.0 (2^30) and
;   up to 4.0 (2^32), representing the high 32 bits of the 48 bit product.
;   The low bit is set to one if any of the 16 low bits of the product are
;   nonzero. If the result is smaller than 2.0, it is shifted left,
;   otherwise the exponent is increased.
;
;   Fdiv:
;
;   Since both fractions are normalised, it is possible to go beyond the
;   fastest division step of 2 cycles per dividend bit. A combination of
;   Newton Rhapson approximation is used with 2 long division steps. 
;   This allows the algorithm to use only 32 x 32 -> 32 multiplies.
;
;   The first step is to get an initial guess of 1/den, which is used for 
;   the Newton Rhapson iteration which returns a more accurate approximation
;   of 1/den. The high 7 bits of den (range 64..127) are indexed into a 
;   64-entry lookup table, giving a 8 bit guess (range 128..255).
;   The guesses are precalculated using a minimax approximation of the
;   error, giving them slightly more than 6 bits precision.
;
;   The second step is a Newton Rhapson iteration using the basic formula:
;   
;   div_guess = guess * (2.0 - guess * den)
;
;   (where den is the denominator, and guess is the table lookup result)
;   The following form allows for better precision by using the fact
;   that guess * den will be close to 1.0:
;  
;   div_guess = guess + guess * (1.0 - guess * den)
;
;   By carefully choosing the fixed point precision so that 1.0 = 2^32,
;   the 1.0 can be left out (giving guess + guess * (-guess * den) ).
;   After the Newton Rhapson iteration the guess of 1/den is 16 bit
;   (range 32768..65536) while the precision has doubled from 6 bit 
;   to more than 13 bits. 
;   The error of the estimate is smaller than (or equal to) zero: 
;   since the slope of 1/x is negative (derivative -1/x^2), the 
;   result is always too small - even if the initial guess was too 
;   large. Thus: div_guess * den < 1.0.
;   We must however ensure that the error isn't too large to affect
;   the correctness to the successive steps - this is explained below.
;   
;   The third and forth step are long divisions using the guess of
;   1/den. Long division works by multipying the numerator with this 
;   guess to get an estimate of num/den. 
;   We have chosen to calculate the 24 bit divident using 2 12 bit 
;   steps. 
;   The general formula for a long division step is:
;
;   div_approx = (num * div_guess) >> K
;   num = num * 2^N - div_approx * den
;
;   (where num is the numerator, and div_guess the estimate for 1/den,
;   K is the shift to get an N bit division approximation, and N is the
;   number of division result bits produced by each step)
;
;   Since we want to use 32 bit multiplies, and num * guess would be
;   24+16-1 = 39 bits, we multiply by an estimate of the numerator,
;   which is the numerator shifted right until the product fits in 32 
;   bits: num_est = num >> 8. 
;   Because we are using estimates of both the numerator and 1/den, the
;   result cannot be exact: we allow the division approximation
;   to be 1 too small at worst (and never too large). This means that 
;   the numerator may be 25 bits after the first step, so the 2nd 
;   division step must be able to produce 13 bits. We also need to check 
;   the result after the last step and correct an off by one error.
;
;   We now present the long division step and analise its error:
;
;   We assume 0.5 * den < num <= den (if not we can shift num or den), 
;   so that the result will be in (0.5 .. 1.0].
;   Since the division approximation may have an error of 0 or -1,
;   the effective range of the numerator is enlarged by den, giving it 
;   a maximum range of num + den <= 2 * den (which is 25 bits).
;
;   num_est = num >> 8
;   div_approx = (num_est * div_guess) >> 19
;   num = num * 4096 - div_approx * den
;
;   We choose the precision of the numerator estimate to be 17 bits,
;   so that the product of div_guess and num_est fit exactly in
;   32 bits. Since num <= 2 * den, den * div_guess < 1.0,
;   num * div_guess < 2.0. If we assume the fixed point of den lies
;   between bit 22 and 23 of den (thus den in [1.0 .. 2.0), the
;   fixed point of the division guess lies between bit 15 and 16
;   (range [0.5..1.0) ). Multiplying them together results in a
;   16 + 24 - 1 = 39 bit fixed point number (close to but below 1.0),
;   thus losing a result bit.
;   After the multiply we shift the division approximation down to
;   the required 12+1 bits: a shift of 32 - 13 = 19 bits.
; 
;   This step is repeated twice and on exit the numerator is checked
;   to see whether it is larger than den, and if it is, the 
;   denominator is subtracted from it. If all is well, the numerator
;   range is now 0 <= num < den.
;
;   The error of the division approximation may be at most -1, thus
;   the sum of the errors of the multiply must be in (-(2^19)..0].
;   We know the the error of the numerator estimate is at most -1,
;   since it is obtained by shifting the numerator down. We get
;   the following equations:
;
;   -1 <= div_approx_err <= 0
;   -1 <= num_est_err <= 0
;  
;   num_est = num >> 8
;   =>
;   num_est <= (2*den) >> 8 = den >> 7
;
;   div_approx_err = 
;     (div_guess_err * num_est + num_est_err * div_guess) >> 19
;   => (assume worst num_est_err)
;   -(2^19) <= div_guess_err * num_est - div_guess <= 0
;   =>
;   -(2^19) <= div_guess_err * (den >> 7) - div_guess <= 0
;   =>
;   (div_guess - (2^19)) / (den >> 7) <= div_guess_err 
;   div_guess_err <= div_guess / (den >> 7)
;
;   It can be seen that the maximum error of the division guess 
;   for small denominators is -7, while it is -3 for large denominators.
;   The division lookup table is created by checking that the error of
;   the the guess of 1/den after the Newton Rhapson iteration
;   satisfies the above formula for all possible values of den for
;   each chosen entry in the division guess table.
;
;   The resulting division produces a 24 bit result in 26 cycles(!) on a
;   StrongARM1.1 using a 64 byte table and 32 bit multiplies.
;
;   Rounding:
;
;   Finally rounding is done using the round and guard bits in the result.
;   If the result doesn't need to be rounded to even, an approximate 
;   overflow check is made on the exponent before rounding. This check
;   is cautious: it will signal over/underflow on numbers that might
;   overflow during rounding. If the test succeeds, the result is returned, 
;   otherwise it is rounded to even if appropriate, and the overflow
;   check is redone. If it fails again, control is handled to either the
;   overflow or underflow handler (depending on whether the exponent is
;   too large or too small). 
;   The overflow and underflow routines first inspect the result, so
;   that false positives can be filtered out with minimal overhead.
;   Overflow results in an invalid operation exception, while underflow 
;   returns a denormal number. The guard bits are the low 7 bits of the 
;   result for fmul and in the remainder for fdiv (since a non-zero remainder 
;   means non-zero guard bits).
;
;   Uncommon operands:
;
;   If one of the operands is a NaN, fcheck_NaN2 is called to process them.
;
;   Infinities are handled the usual way: Inf * b = Inf, a / Inf = 0,
;   Inf / 0 = Inf. However 0 * Inf, Inf / Inf result in the invalid
;   operation exception.
;
;   If one of the operands is zero, a signed zero is returned, except for
;   0 / 0 (invalid operation) and a / 0 (division by zero).
;
;   Denormalized numbers must be normalized before jumping back into the
;   main code, as the main multiply and divide algorithms assume normalised
;   numbers. This is done using a coroutine, shared by fmul and fdiv. 
;   The code for the usual case may thus rely on the leading bit being set.
;
;
; fmul timing (for SA1.1)
; =======================
; A or B NaN, 0 * inf   : 19 clk + invalid operation
; A * inf, inf * B      : 24 clk
; A * 0, 0 * B          : 18 clk
; denorm A or B         : +32 clk (A or B), +49 clk (A and B)
; normal result
;   slow mul            : 36 clk
;   slow mul, round even: 39 clk
;   fast mul            : 27 clk
;   fast mul, round even: 30 clk
; near infinite result  : +7 clk (43 / 34)
; near denorm result    : +7 clk (43 / 34)
; underflow (denorm)    : +11 clk + underflow
;
; fdiv timing (for SA1.1)
; =======================
; A or B NaN, A / 0     : 19 clk + invalid operation
; A / inf, inf / B      : 24 clk
; 0 / B                 : 25 clk
; denorm A or B         : +41 clk (A or B), +57 clk (A and B)
; normal result
;   round               : 48 clk
;   round even          : 51 clk
; near infinite result  : +7 clk (55)
; near denorm result    : +7 clk (55)
; underflow (denorm)    : +12 clk + underflow
;
;===========================================================================

        [ :DEF: mul_s

        CodeArea |FPL$$fmul|

        Export  _fmul
        IMPORT  __flt_normalise2
        IMPORT  __fp_exception
        IMPORT  __fp_fcheck_NaN2s
        IMPORT  __flt_underflow
        IMPORT  __flt_overflow

        MACRO
        MULL48  $a, $b, $res, $tmp
        
        ; Multiply 2 24 bit numbers a and b (in the high 24 bits) to a 
        ; 48 bit product. The high 32 bits of the product are returned in
        ; res, and the LSB is set if the low 16 bits of the product are
        ; non-zero. The low 8 bits of res can be used as guard bits, and
        ; can be tested using one instruction.
        ; Note that because the MSBs of a and b are set, the result
        ; can have either bit 31 or 30 set, so renormalisation is 
        ; necessary.
        ; There are two versions of the code to support architectures
        ; without 64-bit multiply.
        ; In order to fill a result-delay slot, the exponent bias+1 is
        ; subtracted to obtain a correctly biased result exponent.

        [ LONG_MULTIPLY = 1

        UMULL   $tmp, $res, $a, $b
        SUB     exp, exp, #128 << 16    ; Subtract bias+1 - 0..253 normal
        CMP     $tmp, #0                ; Are low 16 bits of 48 product nonzero?
        ORRNE   $res, $res, #1          ;   yes: set LSB of res

        |

        MOV     $a, $a, LSR #8          ; Shift a and b down to low 24 bits
        MOV     $b, $b, LSR #8
        AND     $tmp, $b, #255          ; Get low 8 bits of b
        MUL     $res, $a, $tmp          ; 8 * 24 bit mul
        MOV     $tmp, $b, LSR #8
        AND     $tmp, $tmp, #255        ; Get middle 8 bits of b
        TST     $res, #255              ; Test low 8 guard bits
        MOV     $res, $res, LSR #8      ; Shift res down to 24 bits
        MLA     $res, $a, $tmp, $res    ; 8 * 24 + 24 bit mul
        MOV     $tmp, $b, LSR #16       ; Get high 8 bits of b
        TSTEQ   $res, #255              ; If EQ test next 8 guard bits
        MOV     $res, $res, LSR #8      ; Shift res down to 24 bits
        MLA     $res, $a, $tmp, $res    ; 8 * 24 + 24 bit mul
        SUB     exp, exp, #128 << 16    ; Subtract bias+1 - 0..253 normal
        ORRNE   $res, $res, #1          ; Set LSB if the low 16 bits of
                                        ;   the 48 product are nonzero

        ]

        MEND



_fmul   EnterWithLR_16
        ; Decode exponents and handle uncommon cases
        MOV     mask, #255 << 16
        ANDS    expa, mask, a, LSR #7   ; Decode exponents into bit 16..23 and
        ANDNES  expb, mask, b, LSR #7   ;   test for denormals/zeroes.
        TEQNE   expa, mask              ; If both nonzero, test for infinities
        TEQNE   expb, mask              ;   and NaNs.
        BEQ     fmul_uncommon           ; handle zeroes/denorms/infs/NaNs
        ; Set result sign
        TEQ     a, b                    ; Result sign = sign(a) EOR sign(b)
        ORRMI   expa, expa, #1 << 8     ; Place result sign in bit 8 
        ; Decode fractions
        MOV     mask, #1 << 31
        ORR     a, mask, a, LSL #8      ; Decode fractions to high 24 bits
        ORR     b, mask, b, LSL #8      ;   with explicit leading bit
fmul_mul
        ; Do the multiply. We arrive here with decoded fractions in a and b,
        ; and the exponents in expa and expb.
        ADD     exp, expa, expb         ; Add the exponents and rebias
        MULL48  a, b, res, tmp          ;   them and calculate result
        CMP     res, #&80000000         ; Result >= 2.0?
        MOVLO   res, res, LSL #1        ; Renormalise result so that MSB is set
        ADC     exp, exp, exp, ASR #16  ; Recombine sign & exponent, and
                                        ;   increment exponent if result was >= 2.0
fmul_round
        ; Round the result
        MOVS    a, res, LSR #8          ; Shift down result (never EQ: leading bit)
        ADC     a, a, exp, LSL #23      ; Recombine fraction and exponent and round
        TSTCS   res, #0x7f              ; If round up: test for round to even (EQ)
        CMPNE   exp, #252 << 16         ; If not round to even: did we overflow?
        ReturnToLR LO                   ; Return if no overflow and no round
                                        ;   to even
        BICEQ   a, a, #1                ; Round to even
        CMP     exp, #252 << 16         ; Check for overflow
        ReturnToLR LO                   ; Return if no overflow
        BPL     fmul_overflow           ; Split into overflow (PL) and underflow (MI)
        
fmul_underflow
        ; The result exp may be in range -171 .. 3,
        ; so we check first whether the result is really underflowed.
        MOV     tmp, a, LSL #1
        SUB     tmp, tmp, #1 << 24
        CMP     tmp, #3 << 24           ; Result exp in 1..3 -> no underflow
        ReturnToLR LO                   ; No underflow - 8 cycles overhead
        MOV     a, res
        MVN     sign, exp, LSR #8       ; Correct sign from underflowed exp
        RSB     exp, exp, #8            ; Set denormalising shift
        B       __flt_underflow         ; Handle underflow
     

fmul_overflow                           
        ; The result result exponent may be in range 253 .. 383, so 
        ; first check whether result is really overflowed. 
        ADD     tmp, a, #1 << 23 
        MOVS    tmp, tmp, LSL #1        ; Exp 252..254 +1 gives MI
        ReturnToLR MI                   ; No overflow - 8 cycles overhead
        SUB     a, a, #192 << 23        ; Rebias and correct sign
        B       __flt_overflow


fmul_uncommon
        ; Handle zeros, denorms, infinities and NaNs. We arrive here knowing
        ; that expa contains the sign and exponent of a in bit 26..23,
        ; and that mask == (255 << 24).
        AND     expb, mask, b, LSR #7   ; Decode exp b to bit 16..23
        ; Set result sign
        TEQ     a, b
        ORRMI   expa, expa, #1 << 8
        ; Filter out infinities and NaNs
        CMP     expa, mask
        CMPLO   expb, mask
        BHS     fmul_inf_NaN
        ; Now a or b is a denorm or zero, first check for zero case
        MOVS    tmp, a, LSL #1          ; a zero? (EQ)
        MOVNES  tmp, b, LSL #1          ; If not, is b zero? (EQ)
        MOVEQ   a, expa, LSL #23        ; a or b zero: return signed zero
        ReturnToLR EQ
        ; Here one or both operands are denormals. We normalise them 
        ; before jumping to back the main thread
        ADR     tmp, fmul_mul
        B       __flt_normalise2


fmul_inf_NaN                            
        ; Handle infinities and NaNs. One of a or b is a NaN or infinite.
        MOV     tmp, #0x01000000
        CMN     tmp, a, LSL #1          ; Check whether a or b are NaNs
        CMNLS   tmp, b, LSL #1
        BHI     __fp_fcheck_NaN2s       ; Yes: handle NaNs
        ; Now a or b is infinite
        ; Check that the other operand is non-zero (inf * 0 signals IVO)
        MOVS    tmp, a, LSL #1          ; a zero?
        MOVNES  tmp, b, LSL #1          ; b zero?
        ORRNE   expa, expa, #255        ; No, create infinite
        MOV     a, expa, LSL #23        ;   with correct sign
        ReturnToLR NE                   ; a & b nonzero, return infinite
        MOV     ip, #IVO_bit :OR: MulFn
        B       __fp_exception          ; Signal IVO exception


        ]

;===========================================================================

        [ :DEF: div_s

        CodeArea |FPL$$fdiv|

        Export  _fdiv
        Export  _frdiv
        IMPORT  __flt_normalise2
        IMPORT  __fp_exception
        IMPORT  __fp_fcheck_NaN2s
        IMPORT  __flt_underflow
        IMPORT  __flt_overflow


        ; 64 entry 16384 / [64..127] lookup table
        DCB 129,130,131,132,133,134,135,136
        DCB 137,139,140,141,142,143,145,146
        DCB 147,149,150,151,153,154,156,157
        DCB 159,160,162,163,165,167,168,170
        DCB 172,174,176,178,179,181,183,185
        DCB 188,190,192,194,197,199,201,204
        DCB 206,209,212,215,217,220,223,226
        DCB 230,233,236,240,243,247,250,254
fdiv_tab

_frdiv  EnterWithLR_16
        ; Reverse divide b / a: swap operands
        EOR     a, a, b
        EOR     b, b, a
        EOR     a, a, b
        [ THUMB
        B       _fdiv1
        ]
_fdiv   EnterWithLR_16
_fdiv1
        ; Decode exponents and handle uncommon cases
        MOV     mask, #255 << 16
        ANDS    expa, mask, a, LSR #7   ; Decode exponents into bit 16..23 and
        ANDNES  expb, mask, b, LSR #7   ;   test for denormals/zeroes.
        TEQNE   expa, mask              ; If both nonzero, test for infinities
        TEQNE   expb, mask              ;   and NaNs.
        BEQ     fdiv_uncommon           ; handle zeroes/denorms/infs/NaNs
        ; Set result sign
        TEQ     a, b                    ; Result sign = sign(a) EOR sign(b)
        ORRMI   expa, expa, #1 << 8     ; Place result sign in bit 8 
        ; Decode fractions
        ORR     tmp, a, #1 << 23        ; decode fractions to low 24 bits, 
        ORR     den, b, #1 << 23        ;   add leading bit and swap a and b
        BIC     num, tmp, #0xFF000000   ;   into num and den.
        BIC     den, den, #0xFF000000
fdiv_div
        STMFD   sp!, {lr}
        ; Calculate result exponent and find leading bit of result
        SUB     exp, expa, expb
        CMP     num, den
        ; This code fills result delay slots
        ;MOVLO   num, num, LSL #1         ; shift so that div >= 1 << 23
        ;ADD     exp, exp, #(127-2) << 16 ; subtract bias (one too small)
        ;ADC     exp, exp, exp, ASR #16   ; calc exp, combine with sign

        ; Lookup first guess of 1/den - use inverted tablelookup
        ADD     tmp, den, #(. + 12 - (fdiv_tab + 63)) << 17
        LDRB    guess, [pc, -tmp, LSR #17]
        RSB     den, den, #0            ; Result delay - negate den for MLA
        MOV     guess, guess, LSL #1

        ; Do one Newton-Rhapson iteration to increase precision to 16 bits
        MUL     tmp, den, guess         ; -(guess * den)
        MOVLO   num, num, LSL #1        ; Result delay - shift so that
                                        ;  div >= 1 << 23
        MOV     tmp, tmp, ASR #4
        MUL     div, tmp, guess         ; guess * (-guess * den)
        MOV     guess, guess, LSL #7
        ADD     guess, guess, div, ASR #21; div_guess += guess * (-guess * den)

        ; Long division - 12 bits
        MOV     tmp, num, LSR #8        ; Calc 17 bit estimate from num
        MUL     tmp, guess, tmp         ; Multiply with 1/den estimate
        MOV     num, num, LSL #11       ; Shift num 11(+1) bits left
        MOV     div, tmp, LSR #20       ; Extract 12 bit division estimate
        MLA     num, den, div, num      ; Update num for next iteration
        ADD     exp, exp, #(127-2) << 16 ; Result delay
                                         ; - subtract bias (one too small)

        ; Long division - 12 bits
        MOV     tmp, num, LSR #8        ; Calc 17 bit estimate from num
        MUL     tmp, guess, tmp         ; Multiply with 1/den estimate
        MOV     num, num, LSL #12       ; Shift num 12 bits left
        MOV     tmp, tmp, LSR #19       ; Extract 12+1 bit division estimate
        MLA     num, den, tmp, num      ; Update num for modulo
        ADC     exp, exp, exp, ASR #16  ; Result delay
                                        ; - calc exp, combine with sign

        ; Correct the divivion result (may be off by one)
        CMN     num, den                ; If num >= den
        ADDHS   num, num, den           ;   subtract den from num
        ADC     div, tmp, div, LSL #12  ; Update final result

        ; Round the result
        LDMFD   sp!, {lr}
        CMN     den, num, LSL #1        ; CS -> round, EQ -> round to even
        ADC     a, div, exp, LSL #23    ; Recombine exponent & fraction, and
                                        ;   increment exponent
        CMPNE   exp, #252 << 16         ; exp < 252 cannot overflow
        ReturnToLR LO                   ; Return if not overflow OR round to even
        BICEQ   a, a, #1                ; Round to even
        CMP     exp, #252 << 16         ; Recheck for overflow
        ReturnToLR LO
        BPL     fdiv_overflow           ; Split into overflow (PL) and underflow (MI)
       
fdiv_underflow
        ; The result exp may be in range -150 .. 3, so first check
        ; whether the result is really underflowed.
        MOV     tmp, a, LSL #1
        SUB     tmp, tmp, #1 << 24
        CMP     tmp, #3 << 24           ; Result exp in 1..3 -> no underflow
        ReturnToLR LO                   ; No underflow - 8 cycles overhead
        CMP     num, #1                 ; If num non-zero, there are nonzero
                                        ;   guard bits
        ADC     a, div, div             ; Add explicit guard bit (if num > 0)
        MVN     sign, exp, LSR #8       ; Set correct sign
        RSB     exp, exp, #1            ; Set denormalising shift
        B       __flt_underflow         ; Handle underflow
        

fdiv_overflow
        ; The result exp may be in range 253 .. 405, so first check
        ; whether the result is really overflowed. 
        MOV     tmp, a, LSL #1
        ADD     tmp, tmp, #1 << 24
        CMP     tmp, #254 << 24         ; Check for exp = 253 or 254
        ReturnToLR HS                   ; No overflow - 9 cycles overhead
        SUB     a, a, #192 << 23        ; Rebias and correct sign
        B       __flt_overflow


fdiv_uncommon
        ; Handle zeros, denorms, infinities and NaNs. We arrive here knowing
        ; that expa contains the sign and exponent of a in bit 26..23,
        ; and that mask == (255 << 24).
        AND     expb, mask, b, LSR #7
        ; Set result sign
        TEQ     a, b
        ORRMI   expa, expa, #1 << 8
        ; Filter out infinities and NaNs
        CMP     expa, mask
        CMPLO   expb, mask
        BHS     fdiv_inf_NaN
        ; Now a or b is a denorm or zero, first check for zero case
        MOVS    tmp, b, LSL #1          ; b zero?
        BEQ     fdiv_divbyzero          ; a / 0 -> division by zero
        MOVS    tmp, a, LSL #1          ; 0 / b -> 0
        MOVEQ   a, expa, LSL #23        ; Return signed zero
        ReturnToLR EQ
        ; Here one or both operands are denormals. We normalise them 
        ; before jumping to back the main thread
        ADR     tmp, fdiv_div1
        B       __flt_normalise2


fdiv_div1  
        ; Exchange a and b into num and den, and place the fractions
        ; in the low 24 bits before jumping back to the main thread.                            
        MOV     tmp, a, LSR #8
        MOV     den, b, LSR #8
        MOV     num, tmp
        B       fdiv_div


fdiv_inf_NaN
        ; Handle infinities and NaNs. One of a or b is a NaN or infinite.
        MOV     tmp, #0x01000000
        CMN     tmp, a, LSL #1          ; Check whether a or b are NaNs
        CMNLS   tmp, b, LSL #1
        BHI     __fp_fcheck_NaN2s       ; Yes: handle NaNs
        ; Now a or b is infinite
        ; Check that they are not both infinite (inf / inf -> IVO)
        CMN     tmp, a, LSL #1          ; a and b inf? (EQ) 
        CMNEQ   tmp, b, LSL #1
        MOVEQ   a, expa, LSL #23        ; Yes - set result sign
        MOVEQ   ip, #IVO_bit :OR: DivFn
        BEQ     __fp_exception          ; Generate IVO exception
        ; Here a or b is an infinity, but not both
        CMN     tmp, b, LSL #1          ; b inf? (EQ)
        MOVEQ   a, #0                   ; a / inf -> signed zero
        BICNE   a, a, #1 << 31          ; inf / b = signed inf (even inf / 0 = inf)
        ORR     a, a, expa, LSL #23     ; Add result sign
        ReturnToLR
        

fdiv_divbyzero                          
        ; Handle division by zero (b is zero). a might be zero too, which
        ; generates an IVO exception.
        MOVS    tmp, a, LSL #1          ; a zero? (EQ)
        MOVEQ   ip, #DivFn :OR: IVO_bit ; 0 / 0 -> IVO
        MOVNE   ip, #DivFn :OR: DVZ_bit ; A / 0 -> DVZ
        MOVNE   a, expa, LSL #23        ; Set result sign (returns signed
                                        ;   infinity)
        B       __fp_exception


        ]

;===========================================================================

        [ :DEF: fnorm2_s

        CodeArea |FPL$$fnorm2|

        EXPORT  __flt_normalise2


        ; Normalise and decode a or b (or both). One of a and b is known
        ; to be a deormalised number.
        ; On entry a or b are floating point numbers, with expa and expb 
        ; the decoded exponents of a and b.
        ; If an operand is denormal, normalise it until the MSB is set, 
        ; and subtract the shift count from the respective exponent.
        ; If an operand is already normalised, decode the fraction to the
        ; high 24 bits, and add an explicit leading bit. The exponent
        ; remains unaltered in this case. 
        ; Return to the address held in tmp (might not be LR).

        CODE_32

__flt_normalise2
        MOV     a, a, LSL #8
        MOV     b, b, LSL #8
        TST     expa, #255 << 16        ; Is a denormalised? (EQ)
        BNE     fnorm_b                 ; No - but b must be
fnorm_a
        ; a denormalised
        CMP     a, #1 << 16
        SUBLO   expa, expa, #16 << 16
        MOVLO   a, a, LSL #16
        TST     a, #255 << 24
        SUBEQ   expa, expa, #8 << 16
        MOVEQ   a, a, LSL #8
        TST     a, #15 << 28
        SUBEQ   expa, expa, #4 << 16
        MOVEQ   a, a, LSL #4
        TST     a, #3 << 30
        SUBEQ   expa, expa, #2 << 16
        MOVEQS  a, a, LSL #2
        MOVPL   a, a, LSL #1
        ADDMI   expa, expa, #1 << 16

        TST     expb, #255 << 16        ; Is b denormalised? (EQ)
        ORRNE   b, b, #1 << 31          ; No - add leading bit and
        MOVNE   pc, tmp                 ;   return
fnorm_b
        ; b denormalised
        ORR     a, a, #1 << 31
        CMP     b, #1 << 16
        SUBLO   expb, expb, #16 << 16
        MOVLO   b, b, LSL #16
        TST     b, #255 << 24
        SUBEQ   expb, expb, #8 << 16
        MOVEQ   b, b, LSL #8
        TST     b, #15 << 28
        SUBEQ   expb, expb, #4 << 16
        MOVEQ   b, b, LSL #4
        TST     b, #3 << 30
        SUBEQ   expb, expb, #2 << 16
        MOVEQS  b, b, LSL #2
        MOVPL   b, b, LSL #1
        ADDMI   expb, expb, #1 << 16

        MOV     pc, tmp

        ]

;===========================================================================

        END
