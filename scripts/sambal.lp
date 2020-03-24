\ Equation counts
\     Total        E        G        L        N        X        C        B
\        10       10        0        0        0        0        0        0
\ 
\ Variable counts
\                  x        b        i      s1s      s2s       sc       si
\     Total     cont   binary  integer     sos1     sos2    scont     sint
\        17       17        0        0        0        0        0        0
\ 
\ Nonzero counts
\     Total    const       NL      DLL
\        47       34       13        0
\ 
Minimize
 obj: - 2.000000000000001 x1 - 1.9999999999999978 x2 - 1.9999999999999993 x3 - 2 x4 + 0 x5 + 0 x6 - 2.000000000000001 x7 
  - 1.9999999999999993 x8 - 2 x9 - 2 x10 - 2 x11 - 2.0000000000000018 x12 - 2.0000000000000018 x13 + 0 x14 
  + 0 x15 - 1.9999999999999993 x16 - 1.9999999999999991 x17 + [ .1333333333333334 x1^2 + .666666666666666 x2^2 
  + 1.538461538461538e-2 x3^2 + 2.5e-2 x4^2 + .1333333333333334 x7^2 + 1.538461538461538e-2 x8^2 + .1 x9^2 
  + 8e-2 x10^2 + 5e-2 x11^2 + 3.63636363636364e-2 x12^2 + 9.0909090909091e-3 x13^2 + 1.052631578947368e-2 x16^2 
  + 1.904761904761904e-2 x17^2 ]/2 + objconstant

Subject To
 e1: - x1 - x2 - x3 - x4 + x13 = 0
 e2: - x5 + x14 = 0
 e3: - x6 + x15 = 0
 e4: - x7 - x8 - x9 + x16 = 0
 e5: - x10 - x11 - x12 + x17 = 0
 e6: - x5 - x6 + x13 = 0
 e7: - x1 - x7 - x10 + x14 = 0
 e8: - x2 - x8 - x11 + x15 = 0
 e9: - x3 - x12 + x16 = 0
 e10: - x4 - x9 + x17 = 0

Bounds
 x1 Free
 x2 Free
 x3 Free
 x4 Free
 x5 Free
 x6 Free
 x7 Free
 x8 Free
 x9 Free
 x10 Free
 x11 Free
 x12 Free
 x13 Free
 x14 Free
 x15 Free
 x16 Free
 x17 Free
 objconstant = 1028.0000000000002

End
