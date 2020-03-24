\ Equation counts
\     Total        E        G        L        N        X        C        B
\         2        0        0        2        0        0        0        0
\ 
\ Variable counts
\                  x        b        i      s1s      s2s       sc       si
\     Total     cont   binary  integer     sos1     sos2    scont     sint
\         4        4        0        0        0        0        0        0
\ 
\ Nonzero counts
\     Total    const       NL      DLL
\        12        8        4        0
\ 
Minimize
 obj: x1 

Subject To
 e1: - x1 - x2 - x3 - x4 <= -1
 e2: x1 + x2 + x3 + x4 <= 1

Bounds
 x2 Free
 -1e4 <= x3 <= 1e4
 -1e4 <= x4 <= 1e4

End
