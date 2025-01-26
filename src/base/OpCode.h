//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2025 The Minotaur Team.
//

/**
 * \file OpCode.h
 * \brief Declare the OpCodes used in Minotaur.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#ifndef MINOTAUROPCODE_H
#define MINOTAUROPCODE_H

namespace Minotaur {
typedef enum {
  OpAbs,
  OpAcos,
  OpAcosh,
  OpAsin,
  OpAsinh,
  OpAtan,
  OpAtanh,
  OpCeil,
  OpCos,
  OpCosh,
  OpCPow, // k^x where k is constant.
  OpDiv,
  OpExp,
  OpFloor,
  OpInt,
  OpIntDiv,
  OpLog,
  OpLog10,
  OpMinus,
  OpMult,
  OpNone,
  OpNum,
  OpPlus,
  OpPow,  // y^x 
  OpPowK, // x^k, where k is constant. 
  OpRound,
  OpSin,
  OpSinh,
  OpSqr, // x^2, same as OP2POW in AMPL.
  OpSqrt,
  OpSumList,
  OpTan,
  OpTanh,
  OpUMinus,
  OpVar
} OpCode;
}

#endif

