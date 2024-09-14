      program knapsack
c  UInt N = 64;
c  UInt f = 9;
c  double a[] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0};
c  double b[] = {-1.0, -2.0, -1.5, -1.7, -1.2, -1.7, -1.4, -1.2, -1.5};
c  double objval;
c  double sol[f];

      integer N, f
      double precision a(9),b(9),objval,sol(9)

      N = 64
      f = 9
      objval = 0.0
      a(1)=1.0
      a(2)=2.0
      a(3)=3.0
      a(4)=4.0
      a(5)=5.0
      a(6)=6.0
      a(7)=7.0
      a(8)=8.0
      a(9)=9.0
      b(1)=-1.0
      b(2)=-2.0
      b(3)=-1.5
      b(4)=-1.7
      b(5)=-1.2
      b(6)=-1.7
      b(7)=-1.4
      b(8)=-1.2
      b(9)=-1.5

      call solveknap(N, f, a, b, sol, objval)
      write (*,*) 'objval = ', objval
 
      stop
      end


