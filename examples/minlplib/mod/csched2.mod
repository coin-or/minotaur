#  MINLP written by GAMS Convert at 01/12/18 13:30:48
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#        138       92        7       39        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#        401       93      308        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#        958      900       58        0


var x1 >= 0;
var x2 >= 0;
var x3 >= 0;
var x4 >= 0;
var x5 >= 0;
var x6 >= 0;
var x7 >= 0;
var x8 >= 0;
var x9 >= 0;
var x10 >= 0;
var x11 >= 0;
var x12 >= 0;
var x13 >= 0;
var x14 >= 0;
var x15 >= 0;
var x16 >= 0;
var x17 >= 0;
var x18 >= 0;
var x19 >= 0;
var x20 >= 0;
var x21 >= 0;
var x22 >= 0;
var x23 >= 0;
var x24 >= 0;
var x25 >= 0;
var x26 >= 0;
var x27 >= 0;
var x28 >= 0;
var x29 := 1, >= 0.01, <= 10;
var x30 := 1, >= 0.01, <= 10;
var x31 := 1, >= 0.01, <= 10;
var x32 := 1, >= 0.01, <= 10;
var x33 := 1, >= 0.01, <= 10;
var x34 := 1, >= 0.01, <= 10;
var x35 := 1, >= 0.01, <= 10;
var x36 := 1, >= 0.01, <= 10;
var x37 := 1, >= 0.01, <= 10;
var x38 := 1, >= 0.01, <= 10;
var x39 := 1, >= 0.01, <= 10;
var x40 := 1, >= 0.01, <= 10;
var x41 := 1, >= 0.01, <= 10;
var x42 := 1, >= 0.01, <= 10;
var x43 := 1, >= 0.01, <= 10;
var x44 := 1, >= 0.01, <= 10;
var x45 := 1, >= 0.01, <= 10;
var x46 := 1, >= 0.01, <= 10;
var x47 := 1, >= 0.01, <= 10;
var x48 := 1, >= 0.01, <= 10;
var x49 := 1, >= 0.01, <= 10;
var x50 := 1, >= 0.01, <= 10;
var x51 := 1, >= 0.01, <= 10;
var x52 := 1, >= 0.01, <= 10;
var x53 := 1, >= 0.01, <= 10;
var x54 := 1, >= 0.01, <= 10;
var x55 := 1, >= 0.01, <= 10;
var x56 := 1, >= 0.01, <= 10;
var x57 >= 0;
var x58 >= 0;
var x59 >= 0;
var x60 >= 0;
var x61 >= 0;
var x62 >= 0;
var x63 >= 0;
var x64 >= 0;
var x65 >= 0;
var x66 >= 0;
var x67 >= 0;
var x68 >= 0;
var x69 >= 0;
var x70 >= 0;
var x71 >= 0;
var x72 >= 0;
var x73 >= 0;
var x74 >= 0;
var x75 >= 0;
var x76 >= 0;
var x77 >= 0;
var x78 >= 0;
var x79 >= 0;
var x80 >= 0;
var x81 >= 0;
var x82 >= 0;
var x83 >= 0;
var x84 >= 0;
var x85 >= 0;
var x86 >= 0;
var x87 >= 0;
var x88 >= 0;
var x89 >= 0;
var x90 >= 0;
var x91 >= 0;
var x92 := 100, >= 0;
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
var b151 binary >= 0, <= 1;
var b152 binary >= 0, <= 1;
var b153 binary >= 0, <= 1;
var b154 binary >= 0, <= 1;
var b155 binary >= 0, <= 1;
var b156 binary >= 0, <= 1;
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
var b181 binary >= 0, <= 1;
var b182 binary >= 0, <= 1;
var b183 binary >= 0, <= 1;
var b184 binary >= 0, <= 1;
var b185 binary >= 0, <= 1;
var b186 binary >= 0, <= 1;
var b187 binary >= 0, <= 1;
var b188 binary >= 0, <= 1;
var b189 binary >= 0, <= 1;
var b190 binary >= 0, <= 1;
var b191 binary >= 0, <= 1;
var b192 binary >= 0, <= 1;
var b193 binary >= 0, <= 1;
var b194 binary >= 0, <= 1;
var b195 binary >= 0, <= 1;
var b196 binary >= 0, <= 1;
var b197 binary >= 0, <= 1;
var b198 binary >= 0, <= 1;
var b199 binary >= 0, <= 1;
var b200 binary >= 0, <= 1;
var b201 binary >= 0, <= 1;
var b202 binary >= 0, <= 1;
var b203 binary >= 0, <= 1;
var b204 binary >= 0, <= 1;
var b205 binary >= 0, <= 1;
var b206 binary >= 0, <= 1;
var b207 binary >= 0, <= 1;
var b208 binary >= 0, <= 1;
var b209 binary >= 0, <= 1;
var b210 binary >= 0, <= 1;
var b211 binary >= 0, <= 1;
var b212 binary >= 0, <= 1;
var b213 binary >= 0, <= 1;
var b214 binary >= 0, <= 1;
var b215 binary >= 0, <= 1;
var b216 binary >= 0, <= 1;
var b217 binary >= 0, <= 1;
var b218 binary >= 0, <= 1;
var b219 binary >= 0, <= 1;
var b220 binary >= 0, <= 1;
var b221 binary >= 0, <= 1;
var b222 binary >= 0, <= 1;
var b223 binary >= 0, <= 1;
var b224 binary >= 0, <= 1;
var b225 binary >= 0, <= 1;
var b226 binary >= 0, <= 1;
var b227 binary >= 0, <= 1;
var b228 binary >= 0, <= 1;
var b229 binary >= 0, <= 1;
var b230 binary >= 0, <= 1;
var b231 binary >= 0, <= 1;
var b232 binary >= 0, <= 1;
var b233 binary >= 0, <= 1;
var b234 binary >= 0, <= 1;
var b235 binary >= 0, <= 1;
var b236 binary >= 0, <= 1;
var b237 binary >= 0, <= 1;
var b238 binary >= 0, <= 1;
var b239 binary >= 0, <= 1;
var b240 binary >= 0, <= 1;
var b241 binary >= 0, <= 1;
var b242 binary >= 0, <= 1;
var b243 binary >= 0, <= 1;
var b244 binary >= 0, <= 1;
var b245 binary >= 0, <= 1;
var b246 binary >= 0, <= 1;
var b247 binary >= 0, <= 1;
var b248 binary >= 0, <= 1;
var b249 binary >= 0, <= 1;
var b250 binary >= 0, <= 1;
var b251 binary >= 0, <= 1;
var b252 binary >= 0, <= 1;
var b253 binary >= 0, <= 1;
var b254 binary >= 0, <= 1;
var b255 binary >= 0, <= 1;
var b256 binary >= 0, <= 1;
var b257 binary >= 0, <= 1;
var b258 binary >= 0, <= 1;
var b259 binary >= 0, <= 1;
var b260 binary >= 0, <= 1;
var b261 binary >= 0, <= 1;
var b262 binary >= 0, <= 1;
var b263 binary >= 0, <= 1;
var b264 binary >= 0, <= 1;
var b265 binary >= 0, <= 1;
var b266 binary >= 0, <= 1;
var b267 binary >= 0, <= 1;
var b268 binary >= 0, <= 1;
var b269 binary >= 0, <= 1;
var b270 binary >= 0, <= 1;
var b271 binary >= 0, <= 1;
var b272 binary >= 0, <= 1;
var b273 binary >= 0, <= 1;
var b274 binary >= 0, <= 1;
var b275 binary >= 0, <= 1;
var b276 binary >= 0, <= 1;
var b277 binary >= 0, <= 1;
var b278 binary >= 0, <= 1;
var b279 binary >= 0, <= 1;
var b280 binary >= 0, <= 1;
var b281 binary >= 0, <= 1;
var b282 binary >= 0, <= 1;
var b283 binary >= 0, <= 1;
var b284 binary >= 0, <= 1;
var b285 binary >= 0, <= 1;
var b286 binary >= 0, <= 1;
var b287 binary >= 0, <= 1;
var b288 binary >= 0, <= 1;
var b289 binary >= 0, <= 1;
var b290 binary >= 0, <= 1;
var b291 binary >= 0, <= 1;
var b292 binary >= 0, <= 1;
var b293 binary >= 0, <= 1;
var b294 binary >= 0, <= 1;
var b295 binary >= 0, <= 1;
var b296 binary >= 0, <= 1;
var b297 binary >= 0, <= 1;
var b298 binary >= 0, <= 1;
var b299 binary >= 0, <= 1;
var b300 binary >= 0, <= 1;
var b301 binary >= 0, <= 1;
var b302 binary >= 0, <= 1;
var b303 binary >= 0, <= 1;
var b304 binary >= 0, <= 1;
var b305 binary >= 0, <= 1;
var b306 binary >= 0, <= 1;
var b307 binary >= 0, <= 1;
var b308 binary >= 0, <= 1;
var b309 binary >= 0, <= 1;
var b310 binary >= 0, <= 1;
var b311 binary >= 0, <= 1;
var b312 binary >= 0, <= 1;
var b313 binary >= 0, <= 1;
var b314 binary >= 0, <= 1;
var b315 binary >= 0, <= 1;
var b316 binary >= 0, <= 1;
var b317 binary >= 0, <= 1;
var b318 binary >= 0, <= 1;
var b319 binary >= 0, <= 1;
var b320 binary >= 0, <= 1;
var b321 binary >= 0, <= 1;
var b322 binary >= 0, <= 1;
var b323 binary >= 0, <= 1;
var b324 binary >= 0, <= 1;
var b325 binary >= 0, <= 1;
var b326 binary >= 0, <= 1;
var b327 binary >= 0, <= 1;
var b328 binary >= 0, <= 1;
var b329 binary >= 0, <= 1;
var b330 binary >= 0, <= 1;
var b331 binary >= 0, <= 1;
var b332 binary >= 0, <= 1;
var b333 binary >= 0, <= 1;
var b334 binary >= 0, <= 1;
var b335 binary >= 0, <= 1;
var b336 binary >= 0, <= 1;
var b337 binary >= 0, <= 1;
var b338 binary >= 0, <= 1;
var b339 binary >= 0, <= 1;
var b340 binary >= 0, <= 1;
var b341 binary >= 0, <= 1;
var b342 binary >= 0, <= 1;
var b343 binary >= 0, <= 1;
var b344 binary >= 0, <= 1;
var b345 binary >= 0, <= 1;
var b346 binary >= 0, <= 1;
var b347 binary >= 0, <= 1;
var b348 binary >= 0, <= 1;
var b349 binary >= 0, <= 1;
var b350 binary >= 0, <= 1;
var b351 binary >= 0, <= 1;
var b352 binary >= 0, <= 1;
var b353 binary >= 0, <= 1;
var b354 binary >= 0, <= 1;
var b355 binary >= 0, <= 1;
var b356 binary >= 0, <= 1;
var b357 binary >= 0, <= 1;
var b358 binary >= 0, <= 1;
var b359 binary >= 0, <= 1;
var b360 binary >= 0, <= 1;
var b361 binary >= 0, <= 1;
var b362 binary >= 0, <= 1;
var b363 binary >= 0, <= 1;
var b364 binary >= 0, <= 1;
var b365 binary >= 0, <= 1;
var b366 binary >= 0, <= 1;
var b367 binary >= 0, <= 1;
var b368 binary >= 0, <= 1;
var b369 binary >= 0, <= 1;
var b370 binary >= 0, <= 1;
var b371 binary >= 0, <= 1;
var b372 binary >= 0, <= 1;
var b373 binary >= 0, <= 1;
var b374 binary >= 0, <= 1;
var b375 binary >= 0, <= 1;
var b376 binary >= 0, <= 1;
var b377 binary >= 0, <= 1;
var b378 binary >= 0, <= 1;
var b379 binary >= 0, <= 1;
var b380 binary >= 0, <= 1;
var b381 binary >= 0, <= 1;
var b382 binary >= 0, <= 1;
var b383 binary >= 0, <= 1;
var b384 binary >= 0, <= 1;
var b385 binary >= 0, <= 1;
var b386 binary >= 0, <= 1;
var b387 binary >= 0, <= 1;
var b388 binary >= 0, <= 1;
var b389 binary >= 0, <= 1;
var b390 binary >= 0, <= 1;
var b391 binary >= 0, <= 1;
var b392 binary >= 0, <= 1;
var b393 binary >= 0, <= 1;
var b394 binary >= 0, <= 1;
var b395 binary >= 0, <= 1;
var b396 binary >= 0, <= 1;
var b397 binary >= 0, <= 1;
var b398 binary >= 0, <= 1;
var b399 binary >= 0, <= 1;
var b400 binary >= 0, <= 1;
var x401;

minimize obj: x401;

subject to

e1: (-x92*x401) - ((479700 - 479700*exp(-0.1*x1/x29))*x29 + 31980*x1 - 100*x29
     + (252000 - 252000*exp(-0.2*x2/x30))*x30 + 22680*x2 - 90*x30 + (423500 - 
    423500*exp(-0.1*x3/x31))*x31 + 25410*x3 - 80*x31 + (157440 - 157440*exp(-
    0.2*x4/x32))*x32 + 19680*x4 - 75*x32 + (172108.695652174 - 172108.695652174
    *exp(-0.23*x5/x33))*x33 + 40950*x5 - 90*x33 + (33970.5882352941 - 
    33970.5882352941*exp(-0.34*x6/x34))*x34 + 8580*x6 - 93*x34 + (130200 - 
    130200*exp(-0.2*x7/x35))*x35 + 13440*x7 - 78*x35 + (200640 - 200640*exp(-
    0.2*x8/x36))*x36 + 26334*x8 - 80*x36 + (526680 - 526680*exp(-0.1*x9/x37))*
    x37 + 26334*x9 - 85*x37 + (212850 - 212850*exp(-0.2*x10/x38))*x38 + 29670*
    x10 - 75*x38 + (141360 - 141360*exp(-0.25*x11/x39))*x39 + 28500*x11 - 90*
    x39 + (152937.931034483 - 152937.931034483*exp(-0.29*x12/x40))*x40 + 49104*
    x12 - 94*x40 + (76444.4444444444 - 76444.4444444444*exp(-0.27*x13/x41))*x41
     + 13932*x13 - 78*x41 + (76840 - 76840*exp(-0.3*x14/x42))*x42 + 11526*x14
     - 70*x42 + (102300 - 102300*exp(-0.3*x15/x43))*x43 + 18810*x15 - 90*x43 + 
    (170800 - 170800*exp(-0.2*x16/x44))*x44 + 17568*x16 - 90*x44 + (115200 - 
    115200*exp(-0.3*x17/x45))*x45 + 20160*x17 - 90*x45 + (176000 - 176000*exp(-
    0.27*x18/x46))*x46 + 30360*x18 - 85*x46 + (126357.142857143 - 
    126357.142857143*exp(-0.28*x19/x47))*x47 + 36600*x19 - 93*x47 + (
    45931.0344827586 - 45931.0344827586*exp(-0.29*x20/x48))*x48 + 9000*x20 - 92
    *x48 + (123318 - 123318*exp(-0.25*x21/x49))*x49 + 17901*x21 - 75*x49 + (
    223200 - 223200*exp(-0.2*x22/x50))*x50 + 28800*x22 - 80*x50 + (225000 - 
    225000*exp(-0.2*x23/x51))*x51 + 23750*x23 - 90*x51 + (240800 - 240800*exp(-
    0.15*x24/x52))*x52 + 21672*x24 - 85*x52 + (115920 - 115920*exp(-0.25*x25/
    x53))*x53 + 19320*x25 - 80*x53 + (133241.379310345 - 133241.379310345*exp(-
    0.29*x26/x54))*x54 + 42780*x26 - 92*x54 + (90763.6363636364 - 
    90763.6363636364*exp(-0.22*x27/x55))*x55 + 13312*x27 - 85*x55 + (
    78857.1428571429 - 78857.1428571429*exp(-0.28*x28/x56))*x56 + 11730*x28 - 
    72*x56) = 0;

e2:  - 1300*x1 - 1100*x8 - 900*x15 - 1200*x22 + x57 + 300*x92 = 0;

e3:  - 1200*x2 - 1050*x9 - 800*x16 - 1000*x23 + x58 + 400*x92 = 0;

e4:  - 1100*x3 - 1000*x10 - 800*x17 - 800*x24 + x59 + 300*x92 = 0;

e5:  - 800*x4 - 1000*x11 - 1200*x18 - 700*x25 + x60 + 500*x92 = 0;

e6:  - 1300*x5 - 1200*x12 - 1000*x19 - 1200*x26 + x61 + 500*x92 = 0;

e7:  - 300*x6 - 400*x13 - 300*x20 - 400*x27 + x62 + 100*x92 = 0;

e8:  - 700*x7 - 600*x14 - 850*x21 - 600*x28 + x63 + 600*x92 = 0;

e9:    x57 - 300*x92 <= 0;

e10:    x58 - 300*x92 <= 0;

e11:    x59 - 300*x92 <= 0;

e12:    x60 - 300*x92 <= 0;

e13:    x61 - 300*x92 <= 0;

e14:    x62 - 300*x92 <= 0;

e15:    x63 - 300*x92 <= 0;

e16:    x29 - 0.01*b93 - b94 - 2*b95 - 3*b96 - 4*b97 - 5*b98 - 6*b99 - 7*b100
      - 8*b101 - 9*b102 - 10*b103 = 0;

e17:    x30 - 0.01*b104 - b105 - 2*b106 - 3*b107 - 4*b108 - 5*b109 - 6*b110
      - 7*b111 - 8*b112 - 9*b113 - 10*b114 = 0;

e18:    x31 - 0.01*b115 - b116 - 2*b117 - 3*b118 - 4*b119 - 5*b120 - 6*b121
      - 7*b122 - 8*b123 - 9*b124 - 10*b125 = 0;

e19:    x32 - 0.01*b126 - b127 - 2*b128 - 3*b129 - 4*b130 - 5*b131 - 6*b132
      - 7*b133 - 8*b134 - 9*b135 - 10*b136 = 0;

e20:    x33 - 0.01*b137 - b138 - 2*b139 - 3*b140 - 4*b141 - 5*b142 - 6*b143
      - 7*b144 - 8*b145 - 9*b146 - 10*b147 = 0;

e21:    x34 - 0.01*b148 - b149 - 2*b150 - 3*b151 - 4*b152 - 5*b153 - 6*b154
      - 7*b155 - 8*b156 - 9*b157 - 10*b158 = 0;

e22:    x35 - 0.01*b159 - b160 - 2*b161 - 3*b162 - 4*b163 - 5*b164 - 6*b165
      - 7*b166 - 8*b167 - 9*b168 - 10*b169 = 0;

e23:    x36 - 0.01*b170 - b171 - 2*b172 - 3*b173 - 4*b174 - 5*b175 - 6*b176
      - 7*b177 - 8*b178 - 9*b179 - 10*b180 = 0;

e24:    x37 - 0.01*b181 - b182 - 2*b183 - 3*b184 - 4*b185 - 5*b186 - 6*b187
      - 7*b188 - 8*b189 - 9*b190 - 10*b191 = 0;

e25:    x38 - 0.01*b192 - b193 - 2*b194 - 3*b195 - 4*b196 - 5*b197 - 6*b198
      - 7*b199 - 8*b200 - 9*b201 - 10*b202 = 0;

e26:    x39 - 0.01*b203 - b204 - 2*b205 - 3*b206 - 4*b207 - 5*b208 - 6*b209
      - 7*b210 - 8*b211 - 9*b212 - 10*b213 = 0;

e27:    x40 - 0.01*b214 - b215 - 2*b216 - 3*b217 - 4*b218 - 5*b219 - 6*b220
      - 7*b221 - 8*b222 - 9*b223 - 10*b224 = 0;

e28:    x41 - 0.01*b225 - b226 - 2*b227 - 3*b228 - 4*b229 - 5*b230 - 6*b231
      - 7*b232 - 8*b233 - 9*b234 - 10*b235 = 0;

e29:    x42 - 0.01*b236 - b237 - 2*b238 - 3*b239 - 4*b240 - 5*b241 - 6*b242
      - 7*b243 - 8*b244 - 9*b245 - 10*b246 = 0;

e30:    x43 - 0.01*b247 - b248 - 2*b249 - 3*b250 - 4*b251 - 5*b252 - 6*b253
      - 7*b254 - 8*b255 - 9*b256 - 10*b257 = 0;

e31:    x44 - 0.01*b258 - b259 - 2*b260 - 3*b261 - 4*b262 - 5*b263 - 6*b264
      - 7*b265 - 8*b266 - 9*b267 - 10*b268 = 0;

e32:    x45 - 0.01*b269 - b270 - 2*b271 - 3*b272 - 4*b273 - 5*b274 - 6*b275
      - 7*b276 - 8*b277 - 9*b278 - 10*b279 = 0;

e33:    x46 - 0.01*b280 - b281 - 2*b282 - 3*b283 - 4*b284 - 5*b285 - 6*b286
      - 7*b287 - 8*b288 - 9*b289 - 10*b290 = 0;

e34:    x47 - 0.01*b291 - b292 - 2*b293 - 3*b294 - 4*b295 - 5*b296 - 6*b297
      - 7*b298 - 8*b299 - 9*b300 - 10*b301 = 0;

e35:    x48 - 0.01*b302 - b303 - 2*b304 - 3*b305 - 4*b306 - 5*b307 - 6*b308
      - 7*b309 - 8*b310 - 9*b311 - 10*b312 = 0;

e36:    x49 - 0.01*b313 - b314 - 2*b315 - 3*b316 - 4*b317 - 5*b318 - 6*b319
      - 7*b320 - 8*b321 - 9*b322 - 10*b323 = 0;

e37:    x50 - 0.01*b324 - b325 - 2*b326 - 3*b327 - 4*b328 - 5*b329 - 6*b330
      - 7*b331 - 8*b332 - 9*b333 - 10*b334 = 0;

e38:    x51 - 0.01*b335 - b336 - 2*b337 - 3*b338 - 4*b339 - 5*b340 - 6*b341
      - 7*b342 - 8*b343 - 9*b344 - 10*b345 = 0;

e39:    x52 - 0.01*b346 - b347 - 2*b348 - 3*b349 - 4*b350 - 5*b351 - 6*b352
      - 7*b353 - 8*b354 - 9*b355 - 10*b356 = 0;

e40:    x53 - 0.01*b357 - b358 - 2*b359 - 3*b360 - 4*b361 - 5*b362 - 6*b363
      - 7*b364 - 8*b365 - 9*b366 - 10*b367 = 0;

e41:    x54 - 0.01*b368 - b369 - 2*b370 - 3*b371 - 4*b372 - 5*b373 - 6*b374
      - 7*b375 - 8*b376 - 9*b377 - 10*b378 = 0;

e42:    x55 - 0.01*b379 - b380 - 2*b381 - 3*b382 - 4*b383 - 5*b384 - 6*b385
      - 7*b386 - 8*b387 - 9*b388 - 10*b389 = 0;

e43:    x56 - 0.01*b390 - b391 - 2*b392 - 3*b393 - 4*b394 - 5*b395 - 6*b396
      - 7*b397 - 8*b398 - 9*b399 - 10*b400 = 0;

e44:  - b93 - b94 - b95 - b96 - b97 - b98 - b99 - b100 - b101 - b102 - b103
      = -1;

e45:  - b104 - b105 - b106 - b107 - b108 - b109 - b110 - b111 - b112 - b113
      - b114 = -1;

e46:  - b115 - b116 - b117 - b118 - b119 - b120 - b121 - b122 - b123 - b124
      - b125 = -1;

e47:  - b126 - b127 - b128 - b129 - b130 - b131 - b132 - b133 - b134 - b135
      - b136 = -1;

e48:  - b137 - b138 - b139 - b140 - b141 - b142 - b143 - b144 - b145 - b146
      - b147 = -1;

e49:  - b148 - b149 - b150 - b151 - b152 - b153 - b154 - b155 - b156 - b157
      - b158 = -1;

e50:  - b159 - b160 - b161 - b162 - b163 - b164 - b165 - b166 - b167 - b168
      - b169 = -1;

e51:  - b170 - b171 - b172 - b173 - b174 - b175 - b176 - b177 - b178 - b179
      - b180 = -1;

e52:  - b181 - b182 - b183 - b184 - b185 - b186 - b187 - b188 - b189 - b190
      - b191 = -1;

e53:  - b192 - b193 - b194 - b195 - b196 - b197 - b198 - b199 - b200 - b201
      - b202 = -1;

e54:  - b203 - b204 - b205 - b206 - b207 - b208 - b209 - b210 - b211 - b212
      - b213 = -1;

e55:  - b214 - b215 - b216 - b217 - b218 - b219 - b220 - b221 - b222 - b223
      - b224 = -1;

e56:  - b225 - b226 - b227 - b228 - b229 - b230 - b231 - b232 - b233 - b234
      - b235 = -1;

e57:  - b236 - b237 - b238 - b239 - b240 - b241 - b242 - b243 - b244 - b245
      - b246 = -1;

e58:  - b247 - b248 - b249 - b250 - b251 - b252 - b253 - b254 - b255 - b256
      - b257 = -1;

e59:  - b258 - b259 - b260 - b261 - b262 - b263 - b264 - b265 - b266 - b267
      - b268 = -1;

e60:  - b269 - b270 - b271 - b272 - b273 - b274 - b275 - b276 - b277 - b278
      - b279 = -1;

e61:  - b280 - b281 - b282 - b283 - b284 - b285 - b286 - b287 - b288 - b289
      - b290 = -1;

e62:  - b291 - b292 - b293 - b294 - b295 - b296 - b297 - b298 - b299 - b300
      - b301 = -1;

e63:  - b302 - b303 - b304 - b305 - b306 - b307 - b308 - b309 - b310 - b311
      - b312 = -1;

e64:  - b313 - b314 - b315 - b316 - b317 - b318 - b319 - b320 - b321 - b322
      - b323 = -1;

e65:  - b324 - b325 - b326 - b327 - b328 - b329 - b330 - b331 - b332 - b333
      - b334 = -1;

e66:  - b335 - b336 - b337 - b338 - b339 - b340 - b341 - b342 - b343 - b344
      - b345 = -1;

e67:  - b346 - b347 - b348 - b349 - b350 - b351 - b352 - b353 - b354 - b355
      - b356 = -1;

e68:  - b357 - b358 - b359 - b360 - b361 - b362 - b363 - b364 - b365 - b366
      - b367 = -1;

e69:  - b368 - b369 - b370 - b371 - b372 - b373 - b374 - b375 - b376 - b377
      - b378 = -1;

e70:  - b379 - b380 - b381 - b382 - b383 - b384 - b385 - b386 - b387 - b388
      - b389 = -1;

e71:  - b390 - b391 - b392 - b393 - b394 - b395 - b396 - b397 - b398 - b399
      - b400 = -1;

e72:  - x1 - 2*x29 + x64 = 0;

e73:  - x2 - 3*x30 + x65 = 0;

e74:  - x3 - 3*x31 + x66 = 0;

e75:  - x4 - 3*x32 + x67 = 0;

e76:  - x5 - x33 + x68 = 0;

e77:  - x6 - 2*x34 + x69 = 0;

e78:  - x7 - 3*x35 + x70 = 0;

e79:  - x8 - 3*x36 + x71 = 0;

e80:  - x9 - x37 + x72 = 0;

e81:  - x10 - 2*x38 + x73 = 0;

e82:  - x11 - 2*x39 + x74 = 0;

e83:  - x12 - 2*x40 + x75 = 0;

e84:  - x13 - x41 + x76 = 0;

e85:  - x14 - x42 + x77 = 0;

e86:  - x15 - x43 + x78 = 0;

e87:  - x16 - 3*x44 + x79 = 0;

e88:  - x17 - x45 + x80 = 0;

e89:  - x18 - x46 + x81 = 0;

e90:  - x19 - 2*x47 + x82 = 0;

e91:  - x20 - x48 + x83 = 0;

e92:  - x21 - 2*x49 + x84 = 0;

e93:  - x22 - 2*x50 + x85 = 0;

e94:  - x23 - x51 + x86 = 0;

e95:  - x24 - 3*x52 + x87 = 0;

e96:  - x25 - 2*x53 + x88 = 0;

e97:  - x26 - 2*x54 + x89 = 0;

e98:  - x27 - x55 + x90 = 0;

e99:  - x28 - x56 + x91 = 0;

e100:    x64 + x65 + x66 + x67 + x68 + x69 + x70 - x92 <= 0;

e101:    x71 + x72 + x73 + x74 + x75 + x76 + x77 - x92 <= 0;

e102:    x78 + x79 + x80 + x81 + x82 + x83 + x84 - x92 <= 0;

e103:    x85 + x86 + x87 + x88 + x89 + x90 + x91 - x92 <= 0;

e104:    x1 + 100*b93 <= 100;

e105:    x2 + 100*b104 <= 100;

e106:    x3 + 100*b115 <= 100;

e107:    x4 + 100*b126 <= 100;

e108:    x5 + 100*b137 <= 100;

e109:    x6 + 100*b148 <= 100;

e110:    x7 + 100*b159 <= 100;

e111:    x8 + 100*b170 <= 100;

e112:    x9 + 100*b181 <= 100;

e113:    x10 + 100*b192 <= 100;

e114:    x11 + 100*b203 <= 100;

e115:    x12 + 100*b214 <= 100;

e116:    x13 + 100*b225 <= 100;

e117:    x14 + 100*b236 <= 100;

e118:    x15 + 100*b247 <= 100;

e119:    x16 + 100*b258 <= 100;

e120:    x17 + 100*b269 <= 100;

e121:    x18 + 100*b280 <= 100;

e122:    x19 + 100*b291 <= 100;

e123:    x20 + 100*b302 <= 100;

e124:    x21 + 100*b313 <= 100;

e125:    x22 + 100*b324 <= 100;

e126:    x23 + 100*b335 <= 100;

e127:    x24 + 100*b346 <= 100;

e128:    x25 + 100*b357 <= 100;

e129:    x26 + 100*b368 <= 100;

e130:    x27 + 100*b379 <= 100;

e131:    x28 + 100*b390 <= 100;

e132:    x29 + x36 + x43 + x50 >= 1;

e133:    x30 + x37 + x44 + x51 >= 1;

e134:    x31 + x38 + x45 + x52 >= 1;

e135:    x32 + x39 + x46 + x53 >= 1;

e136:    x33 + x40 + x47 + x54 >= 1;

e137:    x34 + x41 + x48 + x55 >= 1;

e138:    x35 + x42 + x49 + x56 >= 1;
