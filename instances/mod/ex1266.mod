#  MINLP written by GAMS Convert at 01/12/18 13:31:51
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         96       43        7       46        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#        181       43      138        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#        476      404       72        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 := 1, >= 0, <= 5;
var x2 >= 0, <= 5;
var x3 >= 0, <= 5;
var x4 >= 0, <= 5;
var x5 >= 0, <= 5;
var x6 >= 0, <= 5;
var x7 := 2, >= 0, <= 5;
var x8 >= 0, <= 5;
var x9 >= 0, <= 5;
var x10 >= 0, <= 5;
var x11 >= 0, <= 5;
var x12 >= 0, <= 5;
var x13 >= 0, <= 5;
var x14 := 2, >= 0, <= 5;
var x15 >= 0, <= 5;
var x16 >= 0, <= 5;
var x17 >= 0, <= 5;
var x18 >= 0, <= 5;
var x19 >= 0, <= 5;
var x20 := 1, >= 0, <= 5;
var x21 >= 0, <= 5;
var x22 >= 0, <= 5;
var x23 >= 0, <= 5;
var x24 >= 0, <= 5;
var x25 >= 0, <= 5;
var x26 := 2, >= 0, <= 5;
var x27 >= 0, <= 5;
var x28 >= 0, <= 5;
var x29 >= 0, <= 5;
var x30 >= 0, <= 5;
var x31 := 1, >= 0, <= 5;
var x32 >= 0, <= 5;
var x33 >= 0, <= 5;
var x34 >= 0, <= 5;
var x35 >= 0, <= 5;
var x36 >= 0, <= 5;
var b37 binary >= 0, <= 1;
var b38 binary >= 0, <= 1;
var b39 binary >= 0, <= 1;
var b40 binary >= 0, <= 1;
var b41 binary >= 0, <= 1;
var b42 binary >= 0, <= 1;
var b43 binary >= 0, <= 1;
var b44 binary >= 0, <= 1;
var b45 binary >= 0, <= 1;
var b46 binary >= 0, <= 1;
var b47 binary >= 0, <= 1;
var b48 binary >= 0, <= 1;
var b49 binary >= 0, <= 1;
var b50 binary >= 0, <= 1;
var b51 binary >= 0, <= 1;
var b52 binary >= 0, <= 1;
var b53 binary >= 0, <= 1;
var b54 binary >= 0, <= 1;
var b55 binary >= 0, <= 1;
var b56 binary >= 0, <= 1;
var b57 binary >= 0, <= 1;
var b58 binary >= 0, <= 1;
var b59 binary >= 0, <= 1;
var b60 binary >= 0, <= 1;
var b61 binary >= 0, <= 1;
var b62 binary >= 0, <= 1;
var b63 binary >= 0, <= 1;
var b64 binary >= 0, <= 1;
var b65 binary >= 0, <= 1;
var b66 binary >= 0, <= 1;
var b67 binary >= 0, <= 1;
var b68 binary >= 0, <= 1;
var b69 binary >= 0, <= 1;
var b70 binary >= 0, <= 1;
var b71 binary >= 0, <= 1;
var b72 binary >= 0, <= 1;
var b73 binary >= 0, <= 1;
var b74 binary >= 0, <= 1;
var b75 binary >= 0, <= 1;
var b76 binary >= 0, <= 1;
var b77 binary >= 0, <= 1;
var b78 binary >= 0, <= 1;
var b79 binary >= 0, <= 1;
var b80 binary >= 0, <= 1;
var b81 binary >= 0, <= 1;
var b82 binary >= 0, <= 1;
var b83 binary >= 0, <= 1;
var b84 binary >= 0, <= 1;
var b85 binary >= 0, <= 1;
var b86 binary >= 0, <= 1;
var b87 binary >= 0, <= 1;
var b88 binary >= 0, <= 1;
var b89 binary >= 0, <= 1;
var b90 binary >= 0, <= 1;
var b91 binary >= 0, <= 1;
var b92 binary >= 0, <= 1;
var b93 binary >= 0, <= 1;
var b94 binary >= 0, <= 1;
var b95 binary >= 0, <= 1;
var b96 binary >= 0, <= 1;
var b97 binary >= 0, <= 1;
var b98 binary >= 0, <= 1;
var b99 binary >= 0, <= 1;
var b100 binary >= 0, <= 1;
var b101 binary >= 0, <= 1;
var b102 binary >= 0, <= 1;
var b103 binary >= 0, <= 1;
var b104 binary >= 0, <= 1;
var b105 binary >= 0, <= 1;
var b106 binary >= 0, <= 1;
var b107 binary >= 0, <= 1;
var b108 binary >= 0, <= 1;
var b109 binary >= 0, <= 1;
var b110 binary >= 0, <= 1;
var b111 binary >= 0, <= 1;
var b112 binary >= 0, <= 1;
var b113 binary >= 0, <= 1;
var b114 binary >= 0, <= 1;
var b115 binary >= 0, <= 1;
var b116 binary >= 0, <= 1;
var b117 binary >= 0, <= 1;
var b118 binary >= 0, <= 1;
var b119 binary >= 0, <= 1;
var b120 binary >= 0, <= 1;
var b121 binary >= 0, <= 1;
var b122 binary >= 0, <= 1;
var b123 binary >= 0, <= 1;
var b124 binary >= 0, <= 1;
var b125 binary >= 0, <= 1;
var b126 binary >= 0, <= 1;
var b127 binary >= 0, <= 1;
var b128 binary >= 0, <= 1;
var b129 binary >= 0, <= 1;
var b130 binary >= 0, <= 1;
var b131 binary >= 0, <= 1;
var b132 binary >= 0, <= 1;
var b133 binary >= 0, <= 1;
var b134 binary >= 0, <= 1;
var b135 binary >= 0, <= 1;
var b136 binary >= 0, <= 1;
var b137 binary >= 0, <= 1;
var b138 binary >= 0, <= 1;
var b139 binary >= 0, <= 1;
var b140 binary >= 0, <= 1;
var b141 binary >= 0, <= 1;
var b142 binary >= 0, <= 1;
var b143 binary >= 0, <= 1;
var b144 binary >= 0, <= 1;
var b145 binary >= 0, <= 1;
var b146 binary >= 0, <= 1;
var b147 binary >= 0, <= 1;
var b148 binary >= 0, <= 1;
var b149 binary >= 0, <= 1;
var b150 binary >= 0, <= 1;
var x151 := 8, >= 0, <= 15;
var x152 := 7, >= 0, <= 12;
var x153 >= 0, <= 8;
var x154 >= 0, <= 7;
var x155 >= 0, <= 4;
var x156 >= 0, <= 2;
var b157 binary >= 0, <= 1;
var b158 binary >= 0, <= 1;
var b159 binary >= 0, <= 1;
var b160 binary >= 0, <= 1;
var b161 binary >= 0, <= 1;
var b162 binary >= 0, <= 1;
var b163 binary >= 0, <= 1;
var b164 binary >= 0, <= 1;
var b165 binary >= 0, <= 1;
var b166 binary >= 0, <= 1;
var b167 binary >= 0, <= 1;
var b168 binary >= 0, <= 1;
var b169 binary >= 0, <= 1;
var b170 binary >= 0, <= 1;
var b171 binary >= 0, <= 1;
var b172 binary >= 0, <= 1;
var b173 binary >= 0, <= 1;
var b174 binary >= 0, <= 1;
var b175 binary >= 0, <= 1;
var b176 binary >= 0, <= 1;
var b177 binary >= 0, <= 1;
var b178 binary >= 0, <= 1;
var b179 binary >= 0, <= 1;
var b180 binary >= 0, <= 1;

minimize obj:    0.1*b145 + 0.2*b146 + 0.3*b147 + 0.4*b148 + 0.5*b149 + x151
     + x152 + x153 + x154 + x155 + x156;

subject to

e2: x151*x1 + x152*x2 + x153*x3 + x154*x4 + x155*x5 + x156*x6 >= 8;

e3: x151*x7 + x152*x8 + x153*x9 + x154*x10 + x155*x11 + x156*x12 >= 16;

e4: x151*x13 + x152*x14 + x153*x15 + x154*x16 + x155*x17 + x156*x18 >= 12;

e5: x151*x19 + x152*x20 + x153*x21 + x154*x22 + x155*x23 + x156*x24 >= 7;

e6: x151*x25 + x152*x26 + x153*x27 + x154*x28 + x155*x29 + x156*x30 >= 14;

e7: x151*x31 + x152*x32 + x153*x33 + x154*x34 + x155*x35 + x156*x36 >= 16;

e8:  - 330*x1 - 360*x7 - 380*x13 - 430*x19 - 490*x25 - 530*x31 + 2100*b145
     <= 0;

e9:  - 330*x2 - 360*x8 - 380*x14 - 430*x20 - 490*x26 - 530*x32 + 2100*b146
     <= 0;

e10:  - 330*x3 - 360*x9 - 380*x15 - 430*x21 - 490*x27 - 530*x33 + 2100*b147
      <= 0;

e11:  - 330*x4 - 360*x10 - 380*x16 - 430*x22 - 490*x28 - 530*x34 + 2100*b148
      <= 0;

e12:  - 330*x5 - 360*x11 - 380*x17 - 430*x23 - 490*x29 - 530*x35 + 2100*b149
      <= 0;

e13:  - 330*x6 - 360*x12 - 380*x18 - 430*x24 - 490*x30 - 530*x36 + 2100*b150
      <= 0;

e14:    330*x1 + 360*x7 + 380*x13 + 430*x19 + 490*x25 + 530*x31 - 2200*b145
      <= 0;

e15:    330*x2 + 360*x8 + 380*x14 + 430*x20 + 490*x26 + 530*x32 - 2200*b146
      <= 0;

e16:    330*x3 + 360*x9 + 380*x15 + 430*x21 + 490*x27 + 530*x33 - 2200*b147
      <= 0;

e17:    330*x4 + 360*x10 + 380*x16 + 430*x22 + 490*x28 + 530*x34 - 2200*b148
      <= 0;

e18:    330*x5 + 360*x11 + 380*x17 + 430*x23 + 490*x29 + 530*x35 - 2200*b149
      <= 0;

e19:    330*x6 + 360*x12 + 380*x18 + 430*x24 + 490*x30 + 530*x36 - 2200*b150
      <= 0;

e20:  - x1 - x7 - x13 - x19 - x25 - x31 + b145 <= 0;

e21:  - x2 - x8 - x14 - x20 - x26 - x32 + b146 <= 0;

e22:  - x3 - x9 - x15 - x21 - x27 - x33 + b147 <= 0;

e23:  - x4 - x10 - x16 - x22 - x28 - x34 + b148 <= 0;

e24:  - x5 - x11 - x17 - x23 - x29 - x35 + b149 <= 0;

e25:  - x6 - x12 - x18 - x24 - x30 - x36 + b150 <= 0;

e26:    x1 + x7 + x13 + x19 + x25 + x31 - 5*b145 <= 0;

e27:    x2 + x8 + x14 + x20 + x26 + x32 - 5*b146 <= 0;

e28:    x3 + x9 + x15 + x21 + x27 + x33 - 5*b147 <= 0;

e29:    x4 + x10 + x16 + x22 + x28 + x34 - 5*b148 <= 0;

e30:    x5 + x11 + x17 + x23 + x29 + x35 - 5*b149 <= 0;

e31:    x6 + x12 + x18 + x24 + x30 + x36 - 5*b150 <= 0;

e32:    b145 - x151 <= 0;

e33:    b146 - x152 <= 0;

e34:    b147 - x153 <= 0;

e35:    b148 - x154 <= 0;

e36:    b149 - x155 <= 0;

e37:    b150 - x156 <= 0;

e38:  - 15*b145 + x151 <= 0;

e39:  - 12*b146 + x152 <= 0;

e40:  - 8*b147 + x153 <= 0;

e41:  - 7*b148 + x154 <= 0;

e42:  - 4*b149 + x155 <= 0;

e43:  - 2*b150 + x156 <= 0;

e44:    x151 + x152 + x153 + x154 + x155 + x156 >= 16;

e45:  - b145 + b146 <= 0;

e46:  - b146 + b147 <= 0;

e47:  - b147 + b148 <= 0;

e48:  - b148 + b149 <= 0;

e49:  - b149 + b150 <= 0;

e50:  - x151 + x152 <= 0;

e51:  - x152 + x153 <= 0;

e52:  - x153 + x154 <= 0;

e53:  - x154 + x155 <= 0;

e54:  - x155 + x156 <= 0;

e55:    x1 - b37 - 2*b38 - 4*b39 = 0;

e56:    x2 - b40 - 2*b41 - 4*b42 = 0;

e57:    x3 - b43 - 2*b44 - 4*b45 = 0;

e58:    x4 - b46 - 2*b47 - 4*b48 = 0;

e59:    x5 - b49 - 2*b50 - 4*b51 = 0;

e60:    x6 - b52 - 2*b53 - 4*b54 = 0;

e61:    x7 - b55 - 2*b56 - 4*b57 = 0;

e62:    x8 - b58 - 2*b59 - 4*b60 = 0;

e63:    x9 - b61 - 2*b62 - 4*b63 = 0;

e64:    x10 - b64 - 2*b65 - 4*b66 = 0;

e65:    x11 - b67 - 2*b68 - 4*b69 = 0;

e66:    x12 - b70 - 2*b71 - 4*b72 = 0;

e67:    x13 - b73 - 2*b74 - 4*b75 = 0;

e68:    x14 - b76 - 2*b77 - 4*b78 = 0;

e69:    x15 - b79 - 2*b80 - 4*b81 = 0;

e70:    x16 - b82 - 2*b83 - 4*b84 = 0;

e71:    x17 - b85 - 2*b86 - 4*b87 = 0;

e72:    x18 - b88 - 2*b89 - 4*b90 = 0;

e73:    x19 - b91 - 2*b92 - 4*b93 = 0;

e74:    x20 - b94 - 2*b95 - 4*b96 = 0;

e75:    x21 - b97 - 2*b98 - 4*b99 = 0;

e76:    x22 - b100 - 2*b101 - 4*b102 = 0;

e77:    x23 - b103 - 2*b104 - 4*b105 = 0;

e78:    x24 - b106 - 2*b107 - 4*b108 = 0;

e79:    x25 - b109 - 2*b110 - 4*b111 = 0;

e80:    x26 - b112 - 2*b113 - 4*b114 = 0;

e81:    x27 - b115 - 2*b116 - 4*b117 = 0;

e82:    x28 - b118 - 2*b119 - 4*b120 = 0;

e83:    x29 - b121 - 2*b122 - 4*b123 = 0;

e84:    x30 - b124 - 2*b125 - 4*b126 = 0;

e85:    x31 - b127 - 2*b128 - 4*b129 = 0;

e86:    x32 - b130 - 2*b131 - 4*b132 = 0;

e87:    x33 - b133 - 2*b134 - 4*b135 = 0;

e88:    x34 - b136 - 2*b137 - 4*b138 = 0;

e89:    x35 - b139 - 2*b140 - 4*b141 = 0;

e90:    x36 - b142 - 2*b143 - 4*b144 = 0;

e91:    x151 - b157 - 2*b158 - 4*b159 - 8*b160 = 0;

e92:    x152 - b161 - 2*b162 - 4*b163 - 8*b164 = 0;

e93:    x153 - b165 - 2*b166 - 4*b167 - 8*b168 = 0;

e94:    x154 - b169 - 2*b170 - 4*b171 - 8*b172 = 0;

e95:    x155 - b173 - 2*b174 - 4*b175 - 8*b176 = 0;

e96:    x156 - b177 - 2*b178 - 4*b179 - 8*b180 = 0;
