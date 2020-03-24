#  MINLP written by GAMS Convert at 01/12/18 13:43:30
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         98       97        0        1        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#        481       49      432        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#       1009      961       48        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 := 3, >= 3, <= 156;
var x2 := 3, >= 3, <= 156;
var x3 := 3, >= 3, <= 156;
var x4 := 3, >= 3, <= 156;
var x5 := 3, >= 3, <= 156;
var x6 := 3, >= 3, <= 156;
var x7 := 3, >= 3, <= 156;
var x8 := 3, >= 3, <= 156;
var x9 := 3, >= 3, <= 156;
var x10 := 3, >= 3, <= 156;
var x11 := 3, >= 3, <= 156;
var x12 := 3, >= 3, <= 156;
var x13 := 3, >= 3, <= 156;
var x14 := 3, >= 3, <= 156;
var x15 := 3, >= 3, <= 156;
var x16 := 3, >= 3, <= 156;
var x17 := 3, >= 3, <= 156;
var x18 := 3, >= 3, <= 156;
var x19 := 3, >= 3, <= 156;
var x20 := 3, >= 3, <= 156;
var x21 := 3, >= 3, <= 156;
var x22 := 3, >= 3, <= 156;
var x23 := 3, >= 3, <= 156;
var x24 := 3, >= 3, <= 156;
var x25 := 3, >= 3, <= 156;
var x26 := 3, >= 3, <= 156;
var x27 := 3, >= 3, <= 156;
var x28 := 3, >= 3, <= 156;
var x29 := 3, >= 3, <= 156;
var x30 := 3, >= 3, <= 156;
var x31 := 3, >= 3, <= 156;
var x32 := 3, >= 3, <= 156;
var x33 := 3, >= 3, <= 156;
var x34 := 3, >= 3, <= 156;
var x35 := 3, >= 3, <= 156;
var x36 := 3, >= 3, <= 156;
var x37 := 3, >= 3, <= 156;
var x38 := 3, >= 3, <= 156;
var x39 := 3, >= 3, <= 156;
var x40 := 3, >= 3, <= 156;
var x41 := 3, >= 3, <= 156;
var x42 := 3, >= 3, <= 156;
var x43 := 3, >= 3, <= 156;
var x44 := 3, >= 3, <= 156;
var x45 := 3, >= 3, <= 156;
var x46 := 3, >= 3, <= 156;
var x47 := 3, >= 3, <= 156;
var x48 := 3, >= 3, <= 156;
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
var b401 binary >= 0, <= 1;
var b402 binary >= 0, <= 1;
var b403 binary >= 0, <= 1;
var b404 binary >= 0, <= 1;
var b405 binary >= 0, <= 1;
var b406 binary >= 0, <= 1;
var b407 binary >= 0, <= 1;
var b408 binary >= 0, <= 1;
var b409 binary >= 0, <= 1;
var b410 binary >= 0, <= 1;
var b411 binary >= 0, <= 1;
var b412 binary >= 0, <= 1;
var b413 binary >= 0, <= 1;
var b414 binary >= 0, <= 1;
var b415 binary >= 0, <= 1;
var b416 binary >= 0, <= 1;
var b417 binary >= 0, <= 1;
var b418 binary >= 0, <= 1;
var b419 binary >= 0, <= 1;
var b420 binary >= 0, <= 1;
var b421 binary >= 0, <= 1;
var b422 binary >= 0, <= 1;
var b423 binary >= 0, <= 1;
var b424 binary >= 0, <= 1;
var b425 binary >= 0, <= 1;
var b426 binary >= 0, <= 1;
var b427 binary >= 0, <= 1;
var b428 binary >= 0, <= 1;
var b429 binary >= 0, <= 1;
var b430 binary >= 0, <= 1;
var b431 binary >= 0, <= 1;
var b432 binary >= 0, <= 1;
var b433 binary >= 0, <= 1;
var b434 binary >= 0, <= 1;
var b435 binary >= 0, <= 1;
var b436 binary >= 0, <= 1;
var b437 binary >= 0, <= 1;
var b438 binary >= 0, <= 1;
var b439 binary >= 0, <= 1;
var b440 binary >= 0, <= 1;
var b441 binary >= 0, <= 1;
var b442 binary >= 0, <= 1;
var b443 binary >= 0, <= 1;
var b444 binary >= 0, <= 1;
var b445 binary >= 0, <= 1;
var b446 binary >= 0, <= 1;
var b447 binary >= 0, <= 1;
var b448 binary >= 0, <= 1;
var b449 binary >= 0, <= 1;
var b450 binary >= 0, <= 1;
var b451 binary >= 0, <= 1;
var b452 binary >= 0, <= 1;
var b453 binary >= 0, <= 1;
var b454 binary >= 0, <= 1;
var b455 binary >= 0, <= 1;
var b456 binary >= 0, <= 1;
var b457 binary >= 0, <= 1;
var b458 binary >= 0, <= 1;
var b459 binary >= 0, <= 1;
var b460 binary >= 0, <= 1;
var b461 binary >= 0, <= 1;
var b462 binary >= 0, <= 1;
var b463 binary >= 0, <= 1;
var b464 binary >= 0, <= 1;
var b465 binary >= 0, <= 1;
var b466 binary >= 0, <= 1;
var b467 binary >= 0, <= 1;
var b468 binary >= 0, <= 1;
var b469 binary >= 0, <= 1;
var b470 binary >= 0, <= 1;
var b471 binary >= 0, <= 1;
var b472 binary >= 0, <= 1;
var b473 binary >= 0, <= 1;
var b474 binary >= 0, <= 1;
var b475 binary >= 0, <= 1;
var b476 binary >= 0, <= 1;
var b477 binary >= 0, <= 1;
var b478 binary >= 0, <= 1;
var b479 binary >= 0, <= 1;
var b480 binary >= 0, <= 1;

minimize obj: 30.06/x1 + 32.58/x2 + 56.88/x3 + 81.18/x4 + 92.7/x5 + 121.86/x6
     + 141.3/x7 + 179.1/x8 + 257.4/x9 + 313.2/x10 + 622.905/x11 + 705.345/x12
     + 1818/x13 + 2089.8/x14 + 2244.6/x15 + 2400/x16 + 2553.6/x17 + 2571.75/x18
     + 2805.75/x19 + 2966.7/x20 + 3970.68/x21 + 4715.73/x22 + 6259.5/x23 + 
    6521.67/x24 + 7375.5/x25 + 7572.45/x26 + 8095.8/x27 + 10038.6/x28 + 10256.4
    /x29 + 12004.65/x30 + 12674.25/x31 + 13728/x32 + 19854.45/x33 + 20955/x34
     + 22991.4/x35 + 24552/x36 + 29647.5/x37 + 30705.45/x38 + 34773.3/x39 + 
    37539/x40 + 47513.4/x41 + 85101.3/x42 + 103560.6/x43 + 112788/x44 + 
    145599.75/x45 + 149704.8/x46 + 158976/x47 + 159697.5/x48;

subject to

e2:    x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8 + x9 + x10 + x11 + x12 + x13 + x14
     + x15 + x16 + x17 + x18 + x19 + x20 + x21 + x22 + x23 + x24 + x25 + x26
     + x27 + x28 + x29 + x30 + x31 + x32 + x33 + x34 + x35 + x36 + x37 + x38
     + x39 + x40 + x41 + x42 + x43 + x44 + x45 + x46 + x47 + x48 <= 300;

e3:  - x1 + 3*b49 + 6*b50 + 9*b51 + 12*b52 + 18*b53 + 36*b54 + 52*b55 + 78*b56
     + 156*b57 = 0;

e4:  - x2 + 3*b58 + 6*b59 + 9*b60 + 12*b61 + 18*b62 + 36*b63 + 52*b64 + 78*b65
     + 156*b66 = 0;

e5:  - x3 + 3*b67 + 6*b68 + 9*b69 + 12*b70 + 18*b71 + 36*b72 + 52*b73 + 78*b74
     + 156*b75 = 0;

e6:  - x4 + 3*b76 + 6*b77 + 9*b78 + 12*b79 + 18*b80 + 36*b81 + 52*b82 + 78*b83
     + 156*b84 = 0;

e7:  - x5 + 3*b85 + 6*b86 + 9*b87 + 12*b88 + 18*b89 + 36*b90 + 52*b91 + 78*b92
     + 156*b93 = 0;

e8:  - x6 + 3*b94 + 6*b95 + 9*b96 + 12*b97 + 18*b98 + 36*b99 + 52*b100
     + 78*b101 + 156*b102 = 0;

e9:  - x7 + 3*b103 + 6*b104 + 9*b105 + 12*b106 + 18*b107 + 36*b108 + 52*b109
     + 78*b110 + 156*b111 = 0;

e10:  - x8 + 3*b112 + 6*b113 + 9*b114 + 12*b115 + 18*b116 + 36*b117 + 52*b118
      + 78*b119 + 156*b120 = 0;

e11:  - x9 + 3*b121 + 6*b122 + 9*b123 + 12*b124 + 18*b125 + 36*b126 + 52*b127
      + 78*b128 + 156*b129 = 0;

e12:  - x10 + 3*b130 + 6*b131 + 9*b132 + 12*b133 + 18*b134 + 36*b135 + 52*b136
      + 78*b137 + 156*b138 = 0;

e13:  - x11 + 3*b139 + 6*b140 + 9*b141 + 12*b142 + 18*b143 + 36*b144 + 52*b145
      + 78*b146 + 156*b147 = 0;

e14:  - x12 + 3*b148 + 6*b149 + 9*b150 + 12*b151 + 18*b152 + 36*b153 + 52*b154
      + 78*b155 + 156*b156 = 0;

e15:  - x13 + 3*b157 + 6*b158 + 9*b159 + 12*b160 + 18*b161 + 36*b162 + 52*b163
      + 78*b164 + 156*b165 = 0;

e16:  - x14 + 3*b166 + 6*b167 + 9*b168 + 12*b169 + 18*b170 + 36*b171 + 52*b172
      + 78*b173 + 156*b174 = 0;

e17:  - x15 + 3*b175 + 6*b176 + 9*b177 + 12*b178 + 18*b179 + 36*b180 + 52*b181
      + 78*b182 + 156*b183 = 0;

e18:  - x16 + 3*b184 + 6*b185 + 9*b186 + 12*b187 + 18*b188 + 36*b189 + 52*b190
      + 78*b191 + 156*b192 = 0;

e19:  - x17 + 3*b193 + 6*b194 + 9*b195 + 12*b196 + 18*b197 + 36*b198 + 52*b199
      + 78*b200 + 156*b201 = 0;

e20:  - x18 + 3*b202 + 6*b203 + 9*b204 + 12*b205 + 18*b206 + 36*b207 + 52*b208
      + 78*b209 + 156*b210 = 0;

e21:  - x19 + 3*b211 + 6*b212 + 9*b213 + 12*b214 + 18*b215 + 36*b216 + 52*b217
      + 78*b218 + 156*b219 = 0;

e22:  - x20 + 3*b220 + 6*b221 + 9*b222 + 12*b223 + 18*b224 + 36*b225 + 52*b226
      + 78*b227 + 156*b228 = 0;

e23:  - x21 + 3*b229 + 6*b230 + 9*b231 + 12*b232 + 18*b233 + 36*b234 + 52*b235
      + 78*b236 + 156*b237 = 0;

e24:  - x22 + 3*b238 + 6*b239 + 9*b240 + 12*b241 + 18*b242 + 36*b243 + 52*b244
      + 78*b245 + 156*b246 = 0;

e25:  - x23 + 3*b247 + 6*b248 + 9*b249 + 12*b250 + 18*b251 + 36*b252 + 52*b253
      + 78*b254 + 156*b255 = 0;

e26:  - x24 + 3*b256 + 6*b257 + 9*b258 + 12*b259 + 18*b260 + 36*b261 + 52*b262
      + 78*b263 + 156*b264 = 0;

e27:  - x25 + 3*b265 + 6*b266 + 9*b267 + 12*b268 + 18*b269 + 36*b270 + 52*b271
      + 78*b272 + 156*b273 = 0;

e28:  - x26 + 3*b274 + 6*b275 + 9*b276 + 12*b277 + 18*b278 + 36*b279 + 52*b280
      + 78*b281 + 156*b282 = 0;

e29:  - x27 + 3*b283 + 6*b284 + 9*b285 + 12*b286 + 18*b287 + 36*b288 + 52*b289
      + 78*b290 + 156*b291 = 0;

e30:  - x28 + 3*b292 + 6*b293 + 9*b294 + 12*b295 + 18*b296 + 36*b297 + 52*b298
      + 78*b299 + 156*b300 = 0;

e31:  - x29 + 3*b301 + 6*b302 + 9*b303 + 12*b304 + 18*b305 + 36*b306 + 52*b307
      + 78*b308 + 156*b309 = 0;

e32:  - x30 + 3*b310 + 6*b311 + 9*b312 + 12*b313 + 18*b314 + 36*b315 + 52*b316
      + 78*b317 + 156*b318 = 0;

e33:  - x31 + 3*b319 + 6*b320 + 9*b321 + 12*b322 + 18*b323 + 36*b324 + 52*b325
      + 78*b326 + 156*b327 = 0;

e34:  - x32 + 3*b328 + 6*b329 + 9*b330 + 12*b331 + 18*b332 + 36*b333 + 52*b334
      + 78*b335 + 156*b336 = 0;

e35:  - x33 + 3*b337 + 6*b338 + 9*b339 + 12*b340 + 18*b341 + 36*b342 + 52*b343
      + 78*b344 + 156*b345 = 0;

e36:  - x34 + 3*b346 + 6*b347 + 9*b348 + 12*b349 + 18*b350 + 36*b351 + 52*b352
      + 78*b353 + 156*b354 = 0;

e37:  - x35 + 3*b355 + 6*b356 + 9*b357 + 12*b358 + 18*b359 + 36*b360 + 52*b361
      + 78*b362 + 156*b363 = 0;

e38:  - x36 + 3*b364 + 6*b365 + 9*b366 + 12*b367 + 18*b368 + 36*b369 + 52*b370
      + 78*b371 + 156*b372 = 0;

e39:  - x37 + 3*b373 + 6*b374 + 9*b375 + 12*b376 + 18*b377 + 36*b378 + 52*b379
      + 78*b380 + 156*b381 = 0;

e40:  - x38 + 3*b382 + 6*b383 + 9*b384 + 12*b385 + 18*b386 + 36*b387 + 52*b388
      + 78*b389 + 156*b390 = 0;

e41:  - x39 + 3*b391 + 6*b392 + 9*b393 + 12*b394 + 18*b395 + 36*b396 + 52*b397
      + 78*b398 + 156*b399 = 0;

e42:  - x40 + 3*b400 + 6*b401 + 9*b402 + 12*b403 + 18*b404 + 36*b405 + 52*b406
      + 78*b407 + 156*b408 = 0;

e43:  - x41 + 3*b409 + 6*b410 + 9*b411 + 12*b412 + 18*b413 + 36*b414 + 52*b415
      + 78*b416 + 156*b417 = 0;

e44:  - x42 + 3*b418 + 6*b419 + 9*b420 + 12*b421 + 18*b422 + 36*b423 + 52*b424
      + 78*b425 + 156*b426 = 0;

e45:  - x43 + 3*b427 + 6*b428 + 9*b429 + 12*b430 + 18*b431 + 36*b432 + 52*b433
      + 78*b434 + 156*b435 = 0;

e46:  - x44 + 3*b436 + 6*b437 + 9*b438 + 12*b439 + 18*b440 + 36*b441 + 52*b442
      + 78*b443 + 156*b444 = 0;

e47:  - x45 + 3*b445 + 6*b446 + 9*b447 + 12*b448 + 18*b449 + 36*b450 + 52*b451
      + 78*b452 + 156*b453 = 0;

e48:  - x46 + 3*b454 + 6*b455 + 9*b456 + 12*b457 + 18*b458 + 36*b459 + 52*b460
      + 78*b461 + 156*b462 = 0;

e49:  - x47 + 3*b463 + 6*b464 + 9*b465 + 12*b466 + 18*b467 + 36*b468 + 52*b469
      + 78*b470 + 156*b471 = 0;

e50:  - x48 + 3*b472 + 6*b473 + 9*b474 + 12*b475 + 18*b476 + 36*b477 + 52*b478
      + 78*b479 + 156*b480 = 0;

e51:    b49 + b50 + b51 + b52 + b53 + b54 + b55 + b56 + b57 = 1;

e52:    b58 + b59 + b60 + b61 + b62 + b63 + b64 + b65 + b66 = 1;

e53:    b67 + b68 + b69 + b70 + b71 + b72 + b73 + b74 + b75 = 1;

e54:    b76 + b77 + b78 + b79 + b80 + b81 + b82 + b83 + b84 = 1;

e55:    b85 + b86 + b87 + b88 + b89 + b90 + b91 + b92 + b93 = 1;

e56:    b94 + b95 + b96 + b97 + b98 + b99 + b100 + b101 + b102 = 1;

e57:    b103 + b104 + b105 + b106 + b107 + b108 + b109 + b110 + b111 = 1;

e58:    b112 + b113 + b114 + b115 + b116 + b117 + b118 + b119 + b120 = 1;

e59:    b121 + b122 + b123 + b124 + b125 + b126 + b127 + b128 + b129 = 1;

e60:    b130 + b131 + b132 + b133 + b134 + b135 + b136 + b137 + b138 = 1;

e61:    b139 + b140 + b141 + b142 + b143 + b144 + b145 + b146 + b147 = 1;

e62:    b148 + b149 + b150 + b151 + b152 + b153 + b154 + b155 + b156 = 1;

e63:    b157 + b158 + b159 + b160 + b161 + b162 + b163 + b164 + b165 = 1;

e64:    b166 + b167 + b168 + b169 + b170 + b171 + b172 + b173 + b174 = 1;

e65:    b175 + b176 + b177 + b178 + b179 + b180 + b181 + b182 + b183 = 1;

e66:    b184 + b185 + b186 + b187 + b188 + b189 + b190 + b191 + b192 = 1;

e67:    b193 + b194 + b195 + b196 + b197 + b198 + b199 + b200 + b201 = 1;

e68:    b202 + b203 + b204 + b205 + b206 + b207 + b208 + b209 + b210 = 1;

e69:    b211 + b212 + b213 + b214 + b215 + b216 + b217 + b218 + b219 = 1;

e70:    b220 + b221 + b222 + b223 + b224 + b225 + b226 + b227 + b228 = 1;

e71:    b229 + b230 + b231 + b232 + b233 + b234 + b235 + b236 + b237 = 1;

e72:    b238 + b239 + b240 + b241 + b242 + b243 + b244 + b245 + b246 = 1;

e73:    b247 + b248 + b249 + b250 + b251 + b252 + b253 + b254 + b255 = 1;

e74:    b256 + b257 + b258 + b259 + b260 + b261 + b262 + b263 + b264 = 1;

e75:    b265 + b266 + b267 + b268 + b269 + b270 + b271 + b272 + b273 = 1;

e76:    b274 + b275 + b276 + b277 + b278 + b279 + b280 + b281 + b282 = 1;

e77:    b283 + b284 + b285 + b286 + b287 + b288 + b289 + b290 + b291 = 1;

e78:    b292 + b293 + b294 + b295 + b296 + b297 + b298 + b299 + b300 = 1;

e79:    b301 + b302 + b303 + b304 + b305 + b306 + b307 + b308 + b309 = 1;

e80:    b310 + b311 + b312 + b313 + b314 + b315 + b316 + b317 + b318 = 1;

e81:    b319 + b320 + b321 + b322 + b323 + b324 + b325 + b326 + b327 = 1;

e82:    b328 + b329 + b330 + b331 + b332 + b333 + b334 + b335 + b336 = 1;

e83:    b337 + b338 + b339 + b340 + b341 + b342 + b343 + b344 + b345 = 1;

e84:    b346 + b347 + b348 + b349 + b350 + b351 + b352 + b353 + b354 = 1;

e85:    b355 + b356 + b357 + b358 + b359 + b360 + b361 + b362 + b363 = 1;

e86:    b364 + b365 + b366 + b367 + b368 + b369 + b370 + b371 + b372 = 1;

e87:    b373 + b374 + b375 + b376 + b377 + b378 + b379 + b380 + b381 = 1;

e88:    b382 + b383 + b384 + b385 + b386 + b387 + b388 + b389 + b390 = 1;

e89:    b391 + b392 + b393 + b394 + b395 + b396 + b397 + b398 + b399 = 1;

e90:    b400 + b401 + b402 + b403 + b404 + b405 + b406 + b407 + b408 = 1;

e91:    b409 + b410 + b411 + b412 + b413 + b414 + b415 + b416 + b417 = 1;

e92:    b418 + b419 + b420 + b421 + b422 + b423 + b424 + b425 + b426 = 1;

e93:    b427 + b428 + b429 + b430 + b431 + b432 + b433 + b434 + b435 = 1;

e94:    b436 + b437 + b438 + b439 + b440 + b441 + b442 + b443 + b444 = 1;

e95:    b445 + b446 + b447 + b448 + b449 + b450 + b451 + b452 + b453 = 1;

e96:    b454 + b455 + b456 + b457 + b458 + b459 + b460 + b461 + b462 = 1;

e97:    b463 + b464 + b465 + b466 + b467 + b468 + b469 + b470 + b471 = 1;

e98:    b472 + b473 + b474 + b475 + b476 + b477 + b478 + b479 + b480 = 1;
