#  MINLP written by GAMS Convert at 01/12/18 13:45:57
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#        385      157        0      228        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#        813      145      656       12        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#       7205     6917      288        0
# 
#  Reformulation has removed 1 variable and 1 equation


var b1 binary >= 0, <= 1;
var b2 binary >= 0, <= 1;
var b3 binary >= 0, <= 1;
var b4 binary >= 0, <= 1;
var b5 binary >= 0, <= 1;
var b6 binary >= 0, <= 1;
var b7 binary >= 0, <= 1;
var b8 binary >= 0, <= 1;
var b9 binary >= 0, <= 1;
var b10 binary >= 0, <= 1;
var b11 binary >= 0, <= 1;
var b12 binary >= 0, <= 1;
var i13 integer := 1, >= 1, <= 100;
var i14 integer := 1, >= 1, <= 100;
var i15 integer := 1, >= 1, <= 100;
var i16 integer := 1, >= 1, <= 100;
var i17 integer := 1, >= 1, <= 100;
var i18 integer := 1, >= 1, <= 100;
var i19 integer := 1, >= 1, <= 100;
var i20 integer := 1, >= 1, <= 100;
var i21 integer := 1, >= 1, <= 100;
var i22 integer := 1, >= 1, <= 100;
var i23 integer := 1, >= 1, <= 100;
var i24 integer := 1, >= 1, <= 100;
var x25 := 1, >= 1;
var x26 := 1, >= 1;
var x27 := 1, >= 1;
var x28 := 1, >= 1;
var x29 := 1, >= 1;
var x30 := 1, >= 1;
var x31 := 1, >= 1;
var x32 := 1, >= 1;
var x33 := 1, >= 1;
var x34 := 1, >= 1;
var x35 := 1, >= 1;
var x36 := 1, >= 1;
var x37 := 1, >= 1;
var x38 := 1, >= 1;
var x39 := 1, >= 1;
var x40 := 1, >= 1;
var x41 := 1, >= 1;
var x42 := 1, >= 1;
var x43 := 1, >= 1;
var x44 := 1, >= 1;
var x45 := 1, >= 1;
var x46 := 1, >= 1;
var x47 := 1, >= 1;
var x48 := 1, >= 1;
var x49 := 1, >= 1;
var x50 := 1, >= 1;
var x51 := 1, >= 1;
var x52 := 1, >= 1;
var x53 := 1, >= 1;
var x54 := 1, >= 1;
var x55 := 1, >= 1;
var x56 := 1, >= 1;
var x57 := 1, >= 1;
var x58 := 1, >= 1;
var x59 := 1, >= 1;
var x60 := 1, >= 1;
var x61 := 1, >= 1;
var x62 := 1, >= 1;
var x63 := 1, >= 1;
var x64 := 1, >= 1;
var x65 := 1, >= 1;
var x66 := 1, >= 1;
var x67 := 1, >= 1;
var x68 := 1, >= 1;
var x69 := 1, >= 1;
var x70 := 1, >= 1;
var x71 := 1, >= 1;
var x72 := 1, >= 1;
var x73 := 1, >= 1;
var x74 := 1, >= 1;
var x75 := 1, >= 1;
var x76 := 1, >= 1;
var x77 := 1, >= 1;
var x78 := 1, >= 1;
var x79 := 1, >= 1;
var x80 := 1, >= 1;
var x81 := 1, >= 1;
var x82 := 1, >= 1;
var x83 := 1, >= 1;
var x84 := 1, >= 1;
var x85 := 1, >= 1;
var x86 := 1, >= 1;
var x87 := 1, >= 1;
var x88 := 1, >= 1;
var x89 := 1, >= 1;
var x90 := 1, >= 1;
var x91 := 1, >= 1;
var x92 := 1, >= 1;
var x93 := 1, >= 1;
var x94 := 1, >= 1;
var x95 := 1, >= 1;
var x96 := 1, >= 1;
var x97 := 1, >= 1;
var x98 := 1, >= 1;
var x99 := 1, >= 1;
var x100 := 1, >= 1;
var x101 := 1, >= 1;
var x102 := 1, >= 1;
var x103 := 1, >= 1;
var x104 := 1, >= 1;
var x105 := 1, >= 1;
var x106 := 1, >= 1;
var x107 := 1, >= 1;
var x108 := 1, >= 1;
var x109 := 1, >= 1;
var x110 := 1, >= 1;
var x111 := 1, >= 1;
var x112 := 1, >= 1;
var x113 := 1, >= 1;
var x114 := 1, >= 1;
var x115 := 1, >= 1;
var x116 := 1, >= 1;
var x117 := 1, >= 1;
var x118 := 1, >= 1;
var x119 := 1, >= 1;
var x120 := 1, >= 1;
var x121 := 1, >= 1;
var x122 := 1, >= 1;
var x123 := 1, >= 1;
var x124 := 1, >= 1;
var x125 := 1, >= 1;
var x126 := 1, >= 1;
var x127 := 1, >= 1;
var x128 := 1, >= 1;
var x129 := 1, >= 1;
var x130 := 1, >= 1;
var x131 := 1, >= 1;
var x132 := 1, >= 1;
var x133 := 1, >= 1;
var x134 := 1, >= 1;
var x135 := 1, >= 1;
var x136 := 1, >= 1;
var x137 := 1, >= 1;
var x138 := 1, >= 1;
var x139 := 1, >= 1;
var x140 := 1, >= 1;
var x141 := 1, >= 1;
var x142 := 1, >= 1;
var x143 := 1, >= 1;
var x144 := 1, >= 1;
var x145 := 1, >= 1;
var x146 := 1, >= 1;
var x147 := 1, >= 1;
var x148 := 1, >= 1;
var x149 := 1, >= 1;
var x150 := 1, >= 1;
var x151 := 1, >= 1;
var x152 := 1, >= 1;
var x153 := 1, >= 1;
var x154 := 1, >= 1;
var x155 := 1, >= 1;
var x156 := 1, >= 1;
var x157 := 1, >= 1;
var x158 := 1, >= 1;
var x159 := 1, >= 1;
var x160 := 1, >= 1;
var x161 := 1, >= 1;
var x162 := 1, >= 1;
var x163 := 1, >= 1;
var x164 := 1, >= 1;
var x165 := 1, >= 1;
var x166 := 1, >= 1;
var x167 := 1, >= 1;
var x168 := 1, >= 1;
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
var b481 binary >= 0, <= 1;
var b482 binary >= 0, <= 1;
var b483 binary >= 0, <= 1;
var b484 binary >= 0, <= 1;
var b485 binary >= 0, <= 1;
var b486 binary >= 0, <= 1;
var b487 binary >= 0, <= 1;
var b488 binary >= 0, <= 1;
var b489 binary >= 0, <= 1;
var b490 binary >= 0, <= 1;
var b491 binary >= 0, <= 1;
var b492 binary >= 0, <= 1;
var b493 binary >= 0, <= 1;
var b494 binary >= 0, <= 1;
var b495 binary >= 0, <= 1;
var b496 binary >= 0, <= 1;
var b497 binary >= 0, <= 1;
var b498 binary >= 0, <= 1;
var b499 binary >= 0, <= 1;
var b500 binary >= 0, <= 1;
var b501 binary >= 0, <= 1;
var b502 binary >= 0, <= 1;
var b503 binary >= 0, <= 1;
var b504 binary >= 0, <= 1;
var b505 binary >= 0, <= 1;
var b506 binary >= 0, <= 1;
var b507 binary >= 0, <= 1;
var b508 binary >= 0, <= 1;
var b509 binary >= 0, <= 1;
var b510 binary >= 0, <= 1;
var b511 binary >= 0, <= 1;
var b512 binary >= 0, <= 1;
var b513 binary >= 0, <= 1;
var b514 binary >= 0, <= 1;
var b515 binary >= 0, <= 1;
var b516 binary >= 0, <= 1;
var b517 binary >= 0, <= 1;
var b518 binary >= 0, <= 1;
var b519 binary >= 0, <= 1;
var b520 binary >= 0, <= 1;
var b521 binary >= 0, <= 1;
var b522 binary >= 0, <= 1;
var b523 binary >= 0, <= 1;
var b524 binary >= 0, <= 1;
var b525 binary >= 0, <= 1;
var b526 binary >= 0, <= 1;
var b527 binary >= 0, <= 1;
var b528 binary >= 0, <= 1;
var b529 binary >= 0, <= 1;
var b530 binary >= 0, <= 1;
var b531 binary >= 0, <= 1;
var b532 binary >= 0, <= 1;
var b533 binary >= 0, <= 1;
var b534 binary >= 0, <= 1;
var b535 binary >= 0, <= 1;
var b536 binary >= 0, <= 1;
var b537 binary >= 0, <= 1;
var b538 binary >= 0, <= 1;
var b539 binary >= 0, <= 1;
var b540 binary >= 0, <= 1;
var b541 binary >= 0, <= 1;
var b542 binary >= 0, <= 1;
var b543 binary >= 0, <= 1;
var b544 binary >= 0, <= 1;
var b545 binary >= 0, <= 1;
var b546 binary >= 0, <= 1;
var b547 binary >= 0, <= 1;
var b548 binary >= 0, <= 1;
var b549 binary >= 0, <= 1;
var b550 binary >= 0, <= 1;
var b551 binary >= 0, <= 1;
var b552 binary >= 0, <= 1;
var b553 binary >= 0, <= 1;
var b554 binary >= 0, <= 1;
var b555 binary >= 0, <= 1;
var b556 binary >= 0, <= 1;
var b557 binary >= 0, <= 1;
var b558 binary >= 0, <= 1;
var b559 binary >= 0, <= 1;
var b560 binary >= 0, <= 1;
var b561 binary >= 0, <= 1;
var b562 binary >= 0, <= 1;
var b563 binary >= 0, <= 1;
var b564 binary >= 0, <= 1;
var b565 binary >= 0, <= 1;
var b566 binary >= 0, <= 1;
var b567 binary >= 0, <= 1;
var b568 binary >= 0, <= 1;
var b569 binary >= 0, <= 1;
var b570 binary >= 0, <= 1;
var b571 binary >= 0, <= 1;
var b572 binary >= 0, <= 1;
var b573 binary >= 0, <= 1;
var b574 binary >= 0, <= 1;
var b575 binary >= 0, <= 1;
var b576 binary >= 0, <= 1;
var b577 binary >= 0, <= 1;
var b578 binary >= 0, <= 1;
var b579 binary >= 0, <= 1;
var b580 binary >= 0, <= 1;
var b581 binary >= 0, <= 1;
var b582 binary >= 0, <= 1;
var b583 binary >= 0, <= 1;
var b584 binary >= 0, <= 1;
var b585 binary >= 0, <= 1;
var b586 binary >= 0, <= 1;
var b587 binary >= 0, <= 1;
var b588 binary >= 0, <= 1;
var b589 binary >= 0, <= 1;
var b590 binary >= 0, <= 1;
var b591 binary >= 0, <= 1;
var b592 binary >= 0, <= 1;
var b593 binary >= 0, <= 1;
var b594 binary >= 0, <= 1;
var b595 binary >= 0, <= 1;
var b596 binary >= 0, <= 1;
var b597 binary >= 0, <= 1;
var b598 binary >= 0, <= 1;
var b599 binary >= 0, <= 1;
var b600 binary >= 0, <= 1;
var b601 binary >= 0, <= 1;
var b602 binary >= 0, <= 1;
var b603 binary >= 0, <= 1;
var b604 binary >= 0, <= 1;
var b605 binary >= 0, <= 1;
var b606 binary >= 0, <= 1;
var b607 binary >= 0, <= 1;
var b608 binary >= 0, <= 1;
var b609 binary >= 0, <= 1;
var b610 binary >= 0, <= 1;
var b611 binary >= 0, <= 1;
var b612 binary >= 0, <= 1;
var b613 binary >= 0, <= 1;
var b614 binary >= 0, <= 1;
var b615 binary >= 0, <= 1;
var b616 binary >= 0, <= 1;
var b617 binary >= 0, <= 1;
var b618 binary >= 0, <= 1;
var b619 binary >= 0, <= 1;
var b620 binary >= 0, <= 1;
var b621 binary >= 0, <= 1;
var b622 binary >= 0, <= 1;
var b623 binary >= 0, <= 1;
var b624 binary >= 0, <= 1;
var b625 binary >= 0, <= 1;
var b626 binary >= 0, <= 1;
var b627 binary >= 0, <= 1;
var b628 binary >= 0, <= 1;
var b629 binary >= 0, <= 1;
var b630 binary >= 0, <= 1;
var b631 binary >= 0, <= 1;
var b632 binary >= 0, <= 1;
var b633 binary >= 0, <= 1;
var b634 binary >= 0, <= 1;
var b635 binary >= 0, <= 1;
var b636 binary >= 0, <= 1;
var b637 binary >= 0, <= 1;
var b638 binary >= 0, <= 1;
var b639 binary >= 0, <= 1;
var b640 binary >= 0, <= 1;
var b641 binary >= 0, <= 1;
var b642 binary >= 0, <= 1;
var b643 binary >= 0, <= 1;
var b644 binary >= 0, <= 1;
var b645 binary >= 0, <= 1;
var b646 binary >= 0, <= 1;
var b647 binary >= 0, <= 1;
var b648 binary >= 0, <= 1;
var b649 binary >= 0, <= 1;
var b650 binary >= 0, <= 1;
var b651 binary >= 0, <= 1;
var b652 binary >= 0, <= 1;
var b653 binary >= 0, <= 1;
var b654 binary >= 0, <= 1;
var b655 binary >= 0, <= 1;
var b656 binary >= 0, <= 1;
var b657 binary >= 0, <= 1;
var b658 binary >= 0, <= 1;
var b659 binary >= 0, <= 1;
var b660 binary >= 0, <= 1;
var b661 binary >= 0, <= 1;
var b662 binary >= 0, <= 1;
var b663 binary >= 0, <= 1;
var b664 binary >= 0, <= 1;
var b665 binary >= 0, <= 1;
var b666 binary >= 0, <= 1;
var b667 binary >= 0, <= 1;
var b668 binary >= 0, <= 1;
var b669 binary >= 0, <= 1;
var b670 binary >= 0, <= 1;
var b671 binary >= 0, <= 1;
var b672 binary >= 0, <= 1;
var b673 binary >= 0, <= 1;
var b674 binary >= 0, <= 1;
var b675 binary >= 0, <= 1;
var b676 binary >= 0, <= 1;
var b677 binary >= 0, <= 1;
var b678 binary >= 0, <= 1;
var b679 binary >= 0, <= 1;
var b680 binary >= 0, <= 1;
var b681 binary >= 0, <= 1;
var b682 binary >= 0, <= 1;
var b683 binary >= 0, <= 1;
var b684 binary >= 0, <= 1;
var b685 binary >= 0, <= 1;
var b686 binary >= 0, <= 1;
var b687 binary >= 0, <= 1;
var b688 binary >= 0, <= 1;
var b689 binary >= 0, <= 1;
var b690 binary >= 0, <= 1;
var b691 binary >= 0, <= 1;
var b692 binary >= 0, <= 1;
var b693 binary >= 0, <= 1;
var b694 binary >= 0, <= 1;
var b695 binary >= 0, <= 1;
var b696 binary >= 0, <= 1;
var b697 binary >= 0, <= 1;
var b698 binary >= 0, <= 1;
var b699 binary >= 0, <= 1;
var b700 binary >= 0, <= 1;
var b701 binary >= 0, <= 1;
var b702 binary >= 0, <= 1;
var b703 binary >= 0, <= 1;
var b704 binary >= 0, <= 1;
var b705 binary >= 0, <= 1;
var b706 binary >= 0, <= 1;
var b707 binary >= 0, <= 1;
var b708 binary >= 0, <= 1;
var b709 binary >= 0, <= 1;
var b710 binary >= 0, <= 1;
var b711 binary >= 0, <= 1;
var b712 binary >= 0, <= 1;
var b713 binary >= 0, <= 1;
var b714 binary >= 0, <= 1;
var b715 binary >= 0, <= 1;
var b716 binary >= 0, <= 1;
var b717 binary >= 0, <= 1;
var b718 binary >= 0, <= 1;
var b719 binary >= 0, <= 1;
var b720 binary >= 0, <= 1;
var b721 binary >= 0, <= 1;
var b722 binary >= 0, <= 1;
var b723 binary >= 0, <= 1;
var b724 binary >= 0, <= 1;
var b725 binary >= 0, <= 1;
var b726 binary >= 0, <= 1;
var b727 binary >= 0, <= 1;
var b728 binary >= 0, <= 1;
var b729 binary >= 0, <= 1;
var b730 binary >= 0, <= 1;
var b731 binary >= 0, <= 1;
var b732 binary >= 0, <= 1;
var b733 binary >= 0, <= 1;
var b734 binary >= 0, <= 1;
var b735 binary >= 0, <= 1;
var b736 binary >= 0, <= 1;
var b737 binary >= 0, <= 1;
var b738 binary >= 0, <= 1;
var b739 binary >= 0, <= 1;
var b740 binary >= 0, <= 1;
var b741 binary >= 0, <= 1;
var b742 binary >= 0, <= 1;
var b743 binary >= 0, <= 1;
var b744 binary >= 0, <= 1;
var b745 binary >= 0, <= 1;
var b746 binary >= 0, <= 1;
var b747 binary >= 0, <= 1;
var b748 binary >= 0, <= 1;
var b749 binary >= 0, <= 1;
var b750 binary >= 0, <= 1;
var b751 binary >= 0, <= 1;
var b752 binary >= 0, <= 1;
var b753 binary >= 0, <= 1;
var b754 binary >= 0, <= 1;
var b755 binary >= 0, <= 1;
var b756 binary >= 0, <= 1;
var b757 binary >= 0, <= 1;
var b758 binary >= 0, <= 1;
var b759 binary >= 0, <= 1;
var b760 binary >= 0, <= 1;
var b761 binary >= 0, <= 1;
var b762 binary >= 0, <= 1;
var b763 binary >= 0, <= 1;
var b764 binary >= 0, <= 1;
var b765 binary >= 0, <= 1;
var b766 binary >= 0, <= 1;
var b767 binary >= 0, <= 1;
var b768 binary >= 0, <= 1;
var b769 binary >= 0, <= 1;
var b770 binary >= 0, <= 1;
var b771 binary >= 0, <= 1;
var b772 binary >= 0, <= 1;
var b773 binary >= 0, <= 1;
var b774 binary >= 0, <= 1;
var b775 binary >= 0, <= 1;
var b776 binary >= 0, <= 1;
var b777 binary >= 0, <= 1;
var b778 binary >= 0, <= 1;
var b779 binary >= 0, <= 1;
var b780 binary >= 0, <= 1;
var b781 binary >= 0, <= 1;
var b782 binary >= 0, <= 1;
var b783 binary >= 0, <= 1;
var b784 binary >= 0, <= 1;
var b785 binary >= 0, <= 1;
var b786 binary >= 0, <= 1;
var b787 binary >= 0, <= 1;
var b788 binary >= 0, <= 1;
var b789 binary >= 0, <= 1;
var b790 binary >= 0, <= 1;
var b791 binary >= 0, <= 1;
var b792 binary >= 0, <= 1;
var b793 binary >= 0, <= 1;
var b794 binary >= 0, <= 1;
var b795 binary >= 0, <= 1;
var b796 binary >= 0, <= 1;
var b797 binary >= 0, <= 1;
var b798 binary >= 0, <= 1;
var b799 binary >= 0, <= 1;
var b800 binary >= 0, <= 1;
var b801 binary >= 0, <= 1;
var b802 binary >= 0, <= 1;
var b803 binary >= 0, <= 1;
var b804 binary >= 0, <= 1;
var b805 binary >= 0, <= 1;
var b806 binary >= 0, <= 1;
var b807 binary >= 0, <= 1;
var b808 binary >= 0, <= 1;
var b809 binary >= 0, <= 1;
var b810 binary >= 0, <= 1;
var b811 binary >= 0, <= 1;
var b812 binary >= 0, <= 1;

minimize obj:    0.1*b1 + 0.2*b2 + 0.3*b3 + 0.4*b4 + 0.5*b5 + 0.6*b6 + 0.7*b7
     + 0.8*b8 + 0.9*b9 + b10 + 1.1*b11 + 1.2*b12 + b169 + 2*b170 + 3*b171
     + 4*b172 + 5*b173 + 6*b174 + 7*b175 + 8*b176 + 9*b177 + 10*b178 + 11*b179
     + 12*b180 + 13*b181 + 14*b182 + 15*b183 + 16*b184 + 17*b185 + 18*b186
     + 19*b187 + 20*b188 + 21*b189 + 22*b190 + 23*b191 + 24*b192 + 25*b193
     + 26*b194 + 27*b195 + 28*b196 + 29*b197 + 30*b198 + 31*b199 + 32*b200
     + 33*b201 + 34*b202 + 35*b203 + 36*b204 + 37*b205 + 38*b206 + 39*b207
     + 40*b208 + 41*b209 + 42*b210 + 43*b211 + 44*b212 + 45*b213 + 46*b214
     + 47*b215 + 48*b216 + b217 + 2*b218 + 3*b219 + 4*b220 + 5*b221 + 6*b222
     + 7*b223 + 8*b224 + 9*b225 + 10*b226 + 11*b227 + 12*b228 + 13*b229
     + 14*b230 + 15*b231 + 16*b232 + 17*b233 + 18*b234 + 19*b235 + 20*b236
     + 21*b237 + 22*b238 + 23*b239 + 24*b240 + 25*b241 + 26*b242 + 27*b243
     + 28*b244 + 29*b245 + 30*b246 + 31*b247 + 32*b248 + 33*b249 + 34*b250
     + 35*b251 + 36*b252 + 37*b253 + 38*b254 + 39*b255 + 40*b256 + b257
     + 2*b258 + 3*b259 + 4*b260 + 5*b261 + 6*b262 + 7*b263 + 8*b264 + 9*b265
     + 10*b266 + 11*b267 + 12*b268 + 13*b269 + 14*b270 + 15*b271 + 16*b272
     + 17*b273 + 18*b274 + 19*b275 + 20*b276 + 21*b277 + 22*b278 + 23*b279
     + 24*b280 + 25*b281 + 26*b282 + 27*b283 + 28*b284 + 29*b285 + 30*b286
     + b287 + 2*b288 + 3*b289 + 4*b290 + 5*b291 + 6*b292 + 7*b293 + 8*b294
     + 9*b295 + 10*b296 + 11*b297 + 12*b298 + 13*b299 + 14*b300 + 15*b301
     + 16*b302 + 17*b303 + 18*b304 + 19*b305 + 20*b306 + 21*b307 + 22*b308
     + 23*b309 + 24*b310 + 25*b311 + 26*b312 + 27*b313 + 28*b314 + b315
     + 2*b316 + 3*b317 + 4*b318 + 5*b319 + 6*b320 + 7*b321 + 8*b322 + 9*b323
     + 10*b324 + 11*b325 + 12*b326 + 13*b327 + 14*b328 + 15*b329 + 16*b330
     + 17*b331 + 18*b332 + 19*b333 + 20*b334 + 21*b335 + 22*b336 + 23*b337
     + 24*b338 + 25*b339 + 26*b340 + 27*b341 + 28*b342 + b343 + 2*b344 + 3*b345
     + 4*b346 + 5*b347 + 6*b348 + 7*b349 + 8*b350 + 9*b351 + 10*b352 + 11*b353
     + 12*b354 + 13*b355 + 14*b356 + 15*b357 + 16*b358 + 17*b359 + 18*b360
     + 19*b361 + 20*b362 + 21*b363 + 22*b364 + b365 + 2*b366 + 3*b367 + 4*b368
     + 5*b369 + 6*b370 + 7*b371 + 8*b372 + 9*b373 + 10*b374 + 11*b375 + 12*b376
     + 13*b377 + 14*b378 + 15*b379 + 16*b380 + 17*b381 + 18*b382 + 19*b383
     + 20*b384 + 21*b385 + b386 + 2*b387 + 3*b388 + 4*b389 + 5*b390 + 6*b391
     + 7*b392 + 8*b393 + 9*b394 + 10*b395 + b396 + 2*b397 + 3*b398 + 4*b399
     + 5*b400 + 6*b401 + 7*b402 + 8*b403 + 9*b404 + b405 + 2*b406 + 3*b407
     + 4*b408 + 5*b409 + 6*b410 + 7*b411 + 8*b412 + b413 + 2*b414 + 3*b415
     + 4*b416 + 5*b417 + 6*b418 + 7*b419 + 8*b420 + b421 + 2*b422 + 3*b423
     + 4*b424 + 5*b425 + 6*b426 + 7*b427 + 8*b428;

subject to

e2:    350*b429 + 700*b430 + 1050*b431 + 1400*b432 + 1750*b433 + 2100*b434
     + 450*b501 + 900*b502 + 1350*b503 + 1800*b504 + 550*b549 + 1100*b550
     + 1650*b551 + 650*b585 + 1300*b586 + 1950*b587 + 700*b621 + 1400*b622
     + 2100*b623 + 740*b657 + 1480*b658 + 800*b681 + 1600*b682 + 840*b705
     + 1680*b706 + 910*b729 + 1820*b730 + 960*b753 + 1920*b754 + 1010*b777
     + 2020*b778 + 1060*b801 <= 2100;

e3:    350*b435 + 700*b436 + 1050*b437 + 1400*b438 + 1750*b439 + 2100*b440
     + 450*b505 + 900*b506 + 1350*b507 + 1800*b508 + 550*b552 + 1100*b553
     + 1650*b554 + 650*b588 + 1300*b589 + 1950*b590 + 700*b624 + 1400*b625
     + 2100*b626 + 740*b659 + 1480*b660 + 800*b683 + 1600*b684 + 840*b707
     + 1680*b708 + 910*b731 + 1820*b732 + 960*b755 + 1920*b756 + 1010*b779
     + 2020*b780 + 1060*b802 <= 2100;

e4:    350*b441 + 700*b442 + 1050*b443 + 1400*b444 + 1750*b445 + 2100*b446
     + 450*b509 + 900*b510 + 1350*b511 + 1800*b512 + 550*b555 + 1100*b556
     + 1650*b557 + 650*b591 + 1300*b592 + 1950*b593 + 700*b627 + 1400*b628
     + 2100*b629 + 740*b661 + 1480*b662 + 800*b685 + 1600*b686 + 840*b709
     + 1680*b710 + 910*b733 + 1820*b734 + 960*b757 + 1920*b758 + 1010*b781
     + 2020*b782 + 1060*b803 <= 2100;

e5:    350*b447 + 700*b448 + 1050*b449 + 1400*b450 + 1750*b451 + 2100*b452
     + 450*b513 + 900*b514 + 1350*b515 + 1800*b516 + 550*b558 + 1100*b559
     + 1650*b560 + 650*b594 + 1300*b595 + 1950*b596 + 700*b630 + 1400*b631
     + 2100*b632 + 740*b663 + 1480*b664 + 800*b687 + 1600*b688 + 840*b711
     + 1680*b712 + 910*b735 + 1820*b736 + 960*b759 + 1920*b760 + 1010*b783
     + 2020*b784 + 1060*b804 <= 2100;

e6:    350*b453 + 700*b454 + 1050*b455 + 1400*b456 + 1750*b457 + 2100*b458
     + 450*b517 + 900*b518 + 1350*b519 + 1800*b520 + 550*b561 + 1100*b562
     + 1650*b563 + 650*b597 + 1300*b598 + 1950*b599 + 700*b633 + 1400*b634
     + 2100*b635 + 740*b665 + 1480*b666 + 800*b689 + 1600*b690 + 840*b713
     + 1680*b714 + 910*b737 + 1820*b738 + 960*b761 + 1920*b762 + 1010*b785
     + 2020*b786 + 1060*b805 <= 2100;

e7:    350*b459 + 700*b460 + 1050*b461 + 1400*b462 + 1750*b463 + 2100*b464
     + 450*b521 + 900*b522 + 1350*b523 + 1800*b524 + 550*b564 + 1100*b565
     + 1650*b566 + 650*b600 + 1300*b601 + 1950*b602 + 700*b636 + 1400*b637
     + 2100*b638 + 740*b667 + 1480*b668 + 800*b691 + 1600*b692 + 840*b715
     + 1680*b716 + 910*b739 + 1820*b740 + 960*b763 + 1920*b764 + 1010*b787
     + 2020*b788 + 1060*b806 <= 2100;

e8:    350*b465 + 700*b466 + 1050*b467 + 1400*b468 + 1750*b469 + 2100*b470
     + 450*b525 + 900*b526 + 1350*b527 + 1800*b528 + 550*b567 + 1100*b568
     + 1650*b569 + 650*b603 + 1300*b604 + 1950*b605 + 700*b639 + 1400*b640
     + 2100*b641 + 740*b669 + 1480*b670 + 800*b693 + 1600*b694 + 840*b717
     + 1680*b718 + 910*b741 + 1820*b742 + 960*b765 + 1920*b766 + 1010*b789
     + 2020*b790 + 1060*b807 <= 2100;

e9:    350*b471 + 700*b472 + 1050*b473 + 1400*b474 + 1750*b475 + 2100*b476
     + 450*b529 + 900*b530 + 1350*b531 + 1800*b532 + 550*b570 + 1100*b571
     + 1650*b572 + 650*b606 + 1300*b607 + 1950*b608 + 700*b642 + 1400*b643
     + 2100*b644 + 740*b671 + 1480*b672 + 800*b695 + 1600*b696 + 840*b719
     + 1680*b720 + 910*b743 + 1820*b744 + 960*b767 + 1920*b768 + 1010*b791
     + 2020*b792 + 1060*b808 <= 2100;

e10:    350*b477 + 700*b478 + 1050*b479 + 1400*b480 + 1750*b481 + 2100*b482
      + 450*b533 + 900*b534 + 1350*b535 + 1800*b536 + 550*b573 + 1100*b574
      + 1650*b575 + 650*b609 + 1300*b610 + 1950*b611 + 700*b645 + 1400*b646
      + 2100*b647 + 740*b673 + 1480*b674 + 800*b697 + 1600*b698 + 840*b721
      + 1680*b722 + 910*b745 + 1820*b746 + 960*b769 + 1920*b770 + 1010*b793
      + 2020*b794 + 1060*b809 <= 2100;

e11:    350*b483 + 700*b484 + 1050*b485 + 1400*b486 + 1750*b487 + 2100*b488
      + 450*b537 + 900*b538 + 1350*b539 + 1800*b540 + 550*b576 + 1100*b577
      + 1650*b578 + 650*b612 + 1300*b613 + 1950*b614 + 700*b648 + 1400*b649
      + 2100*b650 + 740*b675 + 1480*b676 + 800*b699 + 1600*b700 + 840*b723
      + 1680*b724 + 910*b747 + 1820*b748 + 960*b771 + 1920*b772 + 1010*b795
      + 2020*b796 + 1060*b810 <= 2100;

e12:    350*b489 + 700*b490 + 1050*b491 + 1400*b492 + 1750*b493 + 2100*b494
      + 450*b541 + 900*b542 + 1350*b543 + 1800*b544 + 550*b579 + 1100*b580
      + 1650*b581 + 650*b615 + 1300*b616 + 1950*b617 + 700*b651 + 1400*b652
      + 2100*b653 + 740*b677 + 1480*b678 + 800*b701 + 1600*b702 + 840*b725
      + 1680*b726 + 910*b749 + 1820*b750 + 960*b773 + 1920*b774 + 1010*b797
      + 2020*b798 + 1060*b811 <= 2100;

e13:    350*b495 + 700*b496 + 1050*b497 + 1400*b498 + 1750*b499 + 2100*b500
      + 450*b545 + 900*b546 + 1350*b547 + 1800*b548 + 550*b582 + 1100*b583
      + 1650*b584 + 650*b618 + 1300*b619 + 1950*b620 + 700*b654 + 1400*b655
      + 2100*b656 + 740*b679 + 1480*b680 + 800*b703 + 1600*b704 + 840*b727
      + 1680*b728 + 910*b751 + 1820*b752 + 960*b775 + 1920*b776 + 1010*b799
      + 2020*b800 + 1060*b812 <= 2100;

e14:  - 350*b429 - 700*b430 - 1050*b431 - 1400*b432 - 1750*b433 - 2100*b434
      - 450*b501 - 900*b502 - 1350*b503 - 1800*b504 - 550*b549 - 1100*b550
      - 1650*b551 - 650*b585 - 1300*b586 - 1950*b587 - 700*b621 - 1400*b622
      - 2100*b623 - 740*b657 - 1480*b658 - 800*b681 - 1600*b682 - 840*b705
      - 1680*b706 - 910*b729 - 1820*b730 - 960*b753 - 1920*b754 - 1010*b777
      - 2020*b778 - 1060*b801 <= -2000;

e15:  - 350*b435 - 700*b436 - 1050*b437 - 1400*b438 - 1750*b439 - 2100*b440
      - 450*b505 - 900*b506 - 1350*b507 - 1800*b508 - 550*b552 - 1100*b553
      - 1650*b554 - 650*b588 - 1300*b589 - 1950*b590 - 700*b624 - 1400*b625
      - 2100*b626 - 740*b659 - 1480*b660 - 800*b683 - 1600*b684 - 840*b707
      - 1680*b708 - 910*b731 - 1820*b732 - 960*b755 - 1920*b756 - 1010*b779
      - 2020*b780 - 1060*b802 <= -2000;

e16:  - 350*b441 - 700*b442 - 1050*b443 - 1400*b444 - 1750*b445 - 2100*b446
      - 450*b509 - 900*b510 - 1350*b511 - 1800*b512 - 550*b555 - 1100*b556
      - 1650*b557 - 650*b591 - 1300*b592 - 1950*b593 - 700*b627 - 1400*b628
      - 2100*b629 - 740*b661 - 1480*b662 - 800*b685 - 1600*b686 - 840*b709
      - 1680*b710 - 910*b733 - 1820*b734 - 960*b757 - 1920*b758 - 1010*b781
      - 2020*b782 - 1060*b803 <= -2000;

e17:  - 350*b447 - 700*b448 - 1050*b449 - 1400*b450 - 1750*b451 - 2100*b452
      - 450*b513 - 900*b514 - 1350*b515 - 1800*b516 - 550*b558 - 1100*b559
      - 1650*b560 - 650*b594 - 1300*b595 - 1950*b596 - 700*b630 - 1400*b631
      - 2100*b632 - 740*b663 - 1480*b664 - 800*b687 - 1600*b688 - 840*b711
      - 1680*b712 - 910*b735 - 1820*b736 - 960*b759 - 1920*b760 - 1010*b783
      - 2020*b784 - 1060*b804 <= -2000;

e18:  - 350*b453 - 700*b454 - 1050*b455 - 1400*b456 - 1750*b457 - 2100*b458
      - 450*b517 - 900*b518 - 1350*b519 - 1800*b520 - 550*b561 - 1100*b562
      - 1650*b563 - 650*b597 - 1300*b598 - 1950*b599 - 700*b633 - 1400*b634
      - 2100*b635 - 740*b665 - 1480*b666 - 800*b689 - 1600*b690 - 840*b713
      - 1680*b714 - 910*b737 - 1820*b738 - 960*b761 - 1920*b762 - 1010*b785
      - 2020*b786 - 1060*b805 <= -2000;

e19:  - 350*b459 - 700*b460 - 1050*b461 - 1400*b462 - 1750*b463 - 2100*b464
      - 450*b521 - 900*b522 - 1350*b523 - 1800*b524 - 550*b564 - 1100*b565
      - 1650*b566 - 650*b600 - 1300*b601 - 1950*b602 - 700*b636 - 1400*b637
      - 2100*b638 - 740*b667 - 1480*b668 - 800*b691 - 1600*b692 - 840*b715
      - 1680*b716 - 910*b739 - 1820*b740 - 960*b763 - 1920*b764 - 1010*b787
      - 2020*b788 - 1060*b806 <= -2000;

e20:  - 350*b465 - 700*b466 - 1050*b467 - 1400*b468 - 1750*b469 - 2100*b470
      - 450*b525 - 900*b526 - 1350*b527 - 1800*b528 - 550*b567 - 1100*b568
      - 1650*b569 - 650*b603 - 1300*b604 - 1950*b605 - 700*b639 - 1400*b640
      - 2100*b641 - 740*b669 - 1480*b670 - 800*b693 - 1600*b694 - 840*b717
      - 1680*b718 - 910*b741 - 1820*b742 - 960*b765 - 1920*b766 - 1010*b789
      - 2020*b790 - 1060*b807 <= -2000;

e21:  - 350*b471 - 700*b472 - 1050*b473 - 1400*b474 - 1750*b475 - 2100*b476
      - 450*b529 - 900*b530 - 1350*b531 - 1800*b532 - 550*b570 - 1100*b571
      - 1650*b572 - 650*b606 - 1300*b607 - 1950*b608 - 700*b642 - 1400*b643
      - 2100*b644 - 740*b671 - 1480*b672 - 800*b695 - 1600*b696 - 840*b719
      - 1680*b720 - 910*b743 - 1820*b744 - 960*b767 - 1920*b768 - 1010*b791
      - 2020*b792 - 1060*b808 <= -2000;

e22:  - 350*b477 - 700*b478 - 1050*b479 - 1400*b480 - 1750*b481 - 2100*b482
      - 450*b533 - 900*b534 - 1350*b535 - 1800*b536 - 550*b573 - 1100*b574
      - 1650*b575 - 650*b609 - 1300*b610 - 1950*b611 - 700*b645 - 1400*b646
      - 2100*b647 - 740*b673 - 1480*b674 - 800*b697 - 1600*b698 - 840*b721
      - 1680*b722 - 910*b745 - 1820*b746 - 960*b769 - 1920*b770 - 1010*b793
      - 2020*b794 - 1060*b809 <= -2000;

e23:  - 350*b483 - 700*b484 - 1050*b485 - 1400*b486 - 1750*b487 - 2100*b488
      - 450*b537 - 900*b538 - 1350*b539 - 1800*b540 - 550*b576 - 1100*b577
      - 1650*b578 - 650*b612 - 1300*b613 - 1950*b614 - 700*b648 - 1400*b649
      - 2100*b650 - 740*b675 - 1480*b676 - 800*b699 - 1600*b700 - 840*b723
      - 1680*b724 - 910*b747 - 1820*b748 - 960*b771 - 1920*b772 - 1010*b795
      - 2020*b796 - 1060*b810 <= -2000;

e24:  - 350*b489 - 700*b490 - 1050*b491 - 1400*b492 - 1750*b493 - 2100*b494
      - 450*b541 - 900*b542 - 1350*b543 - 1800*b544 - 550*b579 - 1100*b580
      - 1650*b581 - 650*b615 - 1300*b616 - 1950*b617 - 700*b651 - 1400*b652
      - 2100*b653 - 740*b677 - 1480*b678 - 800*b701 - 1600*b702 - 840*b725
      - 1680*b726 - 910*b749 - 1820*b750 - 960*b773 - 1920*b774 - 1010*b797
      - 2020*b798 - 1060*b811 <= -2000;

e25:  - 350*b495 - 700*b496 - 1050*b497 - 1400*b498 - 1750*b499 - 2100*b500
      - 450*b545 - 900*b546 - 1350*b547 - 1800*b548 - 550*b582 - 1100*b583
      - 1650*b584 - 650*b618 - 1300*b619 - 1950*b620 - 700*b654 - 1400*b655
      - 2100*b656 - 740*b679 - 1480*b680 - 800*b703 - 1600*b704 - 840*b727
      - 1680*b728 - 910*b751 - 1820*b752 - 960*b775 - 1920*b776 - 1010*b799
      - 2020*b800 - 1060*b812 <= -2000;

e26:    b429 + 2*b430 + 3*b431 + 4*b432 + 5*b433 + 6*b434 + b501 + 2*b502
      + 3*b503 + 4*b504 + b549 + 2*b550 + 3*b551 + b585 + 2*b586 + 3*b587
      + b621 + 2*b622 + 3*b623 + b657 + 2*b658 + b681 + 2*b682 + b705 + 2*b706
      + b729 + 2*b730 + b753 + 2*b754 + b777 + 2*b778 + b801 <= 5;

e27:    b435 + 2*b436 + 3*b437 + 4*b438 + 5*b439 + 6*b440 + b505 + 2*b506
      + 3*b507 + 4*b508 + b552 + 2*b553 + 3*b554 + b588 + 2*b589 + 3*b590
      + b624 + 2*b625 + 3*b626 + b659 + 2*b660 + b683 + 2*b684 + b707 + 2*b708
      + b731 + 2*b732 + b755 + 2*b756 + b779 + 2*b780 + b802 <= 5;

e28:    b441 + 2*b442 + 3*b443 + 4*b444 + 5*b445 + 6*b446 + b509 + 2*b510
      + 3*b511 + 4*b512 + b555 + 2*b556 + 3*b557 + b591 + 2*b592 + 3*b593
      + b627 + 2*b628 + 3*b629 + b661 + 2*b662 + b685 + 2*b686 + b709 + 2*b710
      + b733 + 2*b734 + b757 + 2*b758 + b781 + 2*b782 + b803 <= 5;

e29:    b447 + 2*b448 + 3*b449 + 4*b450 + 5*b451 + 6*b452 + b513 + 2*b514
      + 3*b515 + 4*b516 + b558 + 2*b559 + 3*b560 + b594 + 2*b595 + 3*b596
      + b630 + 2*b631 + 3*b632 + b663 + 2*b664 + b687 + 2*b688 + b711 + 2*b712
      + b735 + 2*b736 + b759 + 2*b760 + b783 + 2*b784 + b804 <= 5;

e30:    b453 + 2*b454 + 3*b455 + 4*b456 + 5*b457 + 6*b458 + b517 + 2*b518
      + 3*b519 + 4*b520 + b561 + 2*b562 + 3*b563 + b597 + 2*b598 + 3*b599
      + b633 + 2*b634 + 3*b635 + b665 + 2*b666 + b689 + 2*b690 + b713 + 2*b714
      + b737 + 2*b738 + b761 + 2*b762 + b785 + 2*b786 + b805 <= 5;

e31:    b459 + 2*b460 + 3*b461 + 4*b462 + 5*b463 + 6*b464 + b521 + 2*b522
      + 3*b523 + 4*b524 + b564 + 2*b565 + 3*b566 + b600 + 2*b601 + 3*b602
      + b636 + 2*b637 + 3*b638 + b667 + 2*b668 + b691 + 2*b692 + b715 + 2*b716
      + b739 + 2*b740 + b763 + 2*b764 + b787 + 2*b788 + b806 <= 5;

e32:    b465 + 2*b466 + 3*b467 + 4*b468 + 5*b469 + 6*b470 + b525 + 2*b526
      + 3*b527 + 4*b528 + b567 + 2*b568 + 3*b569 + b603 + 2*b604 + 3*b605
      + b639 + 2*b640 + 3*b641 + b669 + 2*b670 + b693 + 2*b694 + b717 + 2*b718
      + b741 + 2*b742 + b765 + 2*b766 + b789 + 2*b790 + b807 <= 5;

e33:    b471 + 2*b472 + 3*b473 + 4*b474 + 5*b475 + 6*b476 + b529 + 2*b530
      + 3*b531 + 4*b532 + b570 + 2*b571 + 3*b572 + b606 + 2*b607 + 3*b608
      + b642 + 2*b643 + 3*b644 + b671 + 2*b672 + b695 + 2*b696 + b719 + 2*b720
      + b743 + 2*b744 + b767 + 2*b768 + b791 + 2*b792 + b808 <= 5;

e34:    b477 + 2*b478 + 3*b479 + 4*b480 + 5*b481 + 6*b482 + b533 + 2*b534
      + 3*b535 + 4*b536 + b573 + 2*b574 + 3*b575 + b609 + 2*b610 + 3*b611
      + b645 + 2*b646 + 3*b647 + b673 + 2*b674 + b697 + 2*b698 + b721 + 2*b722
      + b745 + 2*b746 + b769 + 2*b770 + b793 + 2*b794 + b809 <= 5;

e35:    b483 + 2*b484 + 3*b485 + 4*b486 + 5*b487 + 6*b488 + b537 + 2*b538
      + 3*b539 + 4*b540 + b576 + 2*b577 + 3*b578 + b612 + 2*b613 + 3*b614
      + b648 + 2*b649 + 3*b650 + b675 + 2*b676 + b699 + 2*b700 + b723 + 2*b724
      + b747 + 2*b748 + b771 + 2*b772 + b795 + 2*b796 + b810 <= 5;

e36:    b489 + 2*b490 + 3*b491 + 4*b492 + 5*b493 + 6*b494 + b541 + 2*b542
      + 3*b543 + 4*b544 + b579 + 2*b580 + 3*b581 + b615 + 2*b616 + 3*b617
      + b651 + 2*b652 + 3*b653 + b677 + 2*b678 + b701 + 2*b702 + b725 + 2*b726
      + b749 + 2*b750 + b773 + 2*b774 + b797 + 2*b798 + b811 <= 5;

e37:    b495 + 2*b496 + 3*b497 + 4*b498 + 5*b499 + 6*b500 + b545 + 2*b546
      + 3*b547 + 4*b548 + b582 + 2*b583 + 3*b584 + b618 + 2*b619 + 3*b620
      + b654 + 2*b655 + 3*b656 + b679 + 2*b680 + b703 + 2*b704 + b727 + 2*b728
      + b751 + 2*b752 + b775 + 2*b776 + b799 + 2*b800 + b812 <= 5;

e38:    b1 - b169 - 2*b170 - 3*b171 - 4*b172 - 5*b173 - 6*b174 - 7*b175
      - 8*b176 - 9*b177 - 10*b178 - 11*b179 - 12*b180 - 13*b181 - 14*b182
      - 15*b183 - 16*b184 - 17*b185 - 18*b186 - 19*b187 - 20*b188 - 21*b189
      - 22*b190 - 23*b191 - 24*b192 - 25*b193 - 26*b194 - 27*b195 - 28*b196
      - 29*b197 - 30*b198 - 31*b199 - 32*b200 - 33*b201 - 34*b202 - 35*b203
      - 36*b204 - 37*b205 - 38*b206 - 39*b207 - 40*b208 - 41*b209 - 42*b210
      - 43*b211 - 44*b212 - 45*b213 - 46*b214 - 47*b215 - 48*b216 <= 0;

e39:    b2 - b217 - 2*b218 - 3*b219 - 4*b220 - 5*b221 - 6*b222 - 7*b223
      - 8*b224 - 9*b225 - 10*b226 - 11*b227 - 12*b228 - 13*b229 - 14*b230
      - 15*b231 - 16*b232 - 17*b233 - 18*b234 - 19*b235 - 20*b236 - 21*b237
      - 22*b238 - 23*b239 - 24*b240 - 25*b241 - 26*b242 - 27*b243 - 28*b244
      - 29*b245 - 30*b246 - 31*b247 - 32*b248 - 33*b249 - 34*b250 - 35*b251
      - 36*b252 - 37*b253 - 38*b254 - 39*b255 - 40*b256 <= 0;

e40:    b3 - b257 - 2*b258 - 3*b259 - 4*b260 - 5*b261 - 6*b262 - 7*b263
      - 8*b264 - 9*b265 - 10*b266 - 11*b267 - 12*b268 - 13*b269 - 14*b270
      - 15*b271 - 16*b272 - 17*b273 - 18*b274 - 19*b275 - 20*b276 - 21*b277
      - 22*b278 - 23*b279 - 24*b280 - 25*b281 - 26*b282 - 27*b283 - 28*b284
      - 29*b285 - 30*b286 <= 0;

e41:    b4 - b287 - 2*b288 - 3*b289 - 4*b290 - 5*b291 - 6*b292 - 7*b293
      - 8*b294 - 9*b295 - 10*b296 - 11*b297 - 12*b298 - 13*b299 - 14*b300
      - 15*b301 - 16*b302 - 17*b303 - 18*b304 - 19*b305 - 20*b306 - 21*b307
      - 22*b308 - 23*b309 - 24*b310 - 25*b311 - 26*b312 - 27*b313 - 28*b314
      <= 0;

e42:    b5 - b315 - 2*b316 - 3*b317 - 4*b318 - 5*b319 - 6*b320 - 7*b321
      - 8*b322 - 9*b323 - 10*b324 - 11*b325 - 12*b326 - 13*b327 - 14*b328
      - 15*b329 - 16*b330 - 17*b331 - 18*b332 - 19*b333 - 20*b334 - 21*b335
      - 22*b336 - 23*b337 - 24*b338 - 25*b339 - 26*b340 - 27*b341 - 28*b342
      <= 0;

e43:    b6 - b343 - 2*b344 - 3*b345 - 4*b346 - 5*b347 - 6*b348 - 7*b349
      - 8*b350 - 9*b351 - 10*b352 - 11*b353 - 12*b354 - 13*b355 - 14*b356
      - 15*b357 - 16*b358 - 17*b359 - 18*b360 - 19*b361 - 20*b362 - 21*b363
      - 22*b364 <= 0;

e44:    b7 - b365 - 2*b366 - 3*b367 - 4*b368 - 5*b369 - 6*b370 - 7*b371
      - 8*b372 - 9*b373 - 10*b374 - 11*b375 - 12*b376 - 13*b377 - 14*b378
      - 15*b379 - 16*b380 - 17*b381 - 18*b382 - 19*b383 - 20*b384 - 21*b385
      <= 0;

e45:    b8 - b386 - 2*b387 - 3*b388 - 4*b389 - 5*b390 - 6*b391 - 7*b392
      - 8*b393 - 9*b394 - 10*b395 <= 0;

e46:    b9 - b396 - 2*b397 - 3*b398 - 4*b399 - 5*b400 - 6*b401 - 7*b402
      - 8*b403 - 9*b404 <= 0;

e47:    b10 - b405 - 2*b406 - 3*b407 - 4*b408 - 5*b409 - 6*b410 - 7*b411
      - 8*b412 <= 0;

e48:    b11 - b413 - 2*b414 - 3*b415 - 4*b416 - 5*b417 - 6*b418 - 7*b419
      - 8*b420 <= 0;

e49:    b12 - b421 - 2*b422 - 3*b423 - 4*b424 - 5*b425 - 6*b426 - 7*b427
      - 8*b428 <= 0;

e50:  - 48*b1 + b169 + 2*b170 + 3*b171 + 4*b172 + 5*b173 + 6*b174 + 7*b175
      + 8*b176 + 9*b177 + 10*b178 + 11*b179 + 12*b180 + 13*b181 + 14*b182
      + 15*b183 + 16*b184 + 17*b185 + 18*b186 + 19*b187 + 20*b188 + 21*b189
      + 22*b190 + 23*b191 + 24*b192 + 25*b193 + 26*b194 + 27*b195 + 28*b196
      + 29*b197 + 30*b198 + 31*b199 + 32*b200 + 33*b201 + 34*b202 + 35*b203
      + 36*b204 + 37*b205 + 38*b206 + 39*b207 + 40*b208 + 41*b209 + 42*b210
      + 43*b211 + 44*b212 + 45*b213 + 46*b214 + 47*b215 + 48*b216 <= 0;

e51:  - 40*b2 + b217 + 2*b218 + 3*b219 + 4*b220 + 5*b221 + 6*b222 + 7*b223
      + 8*b224 + 9*b225 + 10*b226 + 11*b227 + 12*b228 + 13*b229 + 14*b230
      + 15*b231 + 16*b232 + 17*b233 + 18*b234 + 19*b235 + 20*b236 + 21*b237
      + 22*b238 + 23*b239 + 24*b240 + 25*b241 + 26*b242 + 27*b243 + 28*b244
      + 29*b245 + 30*b246 + 31*b247 + 32*b248 + 33*b249 + 34*b250 + 35*b251
      + 36*b252 + 37*b253 + 38*b254 + 39*b255 + 40*b256 <= 0;

e52:  - 30*b3 + b257 + 2*b258 + 3*b259 + 4*b260 + 5*b261 + 6*b262 + 7*b263
      + 8*b264 + 9*b265 + 10*b266 + 11*b267 + 12*b268 + 13*b269 + 14*b270
      + 15*b271 + 16*b272 + 17*b273 + 18*b274 + 19*b275 + 20*b276 + 21*b277
      + 22*b278 + 23*b279 + 24*b280 + 25*b281 + 26*b282 + 27*b283 + 28*b284
      + 29*b285 + 30*b286 <= 0;

e53:  - 28*b4 + b287 + 2*b288 + 3*b289 + 4*b290 + 5*b291 + 6*b292 + 7*b293
      + 8*b294 + 9*b295 + 10*b296 + 11*b297 + 12*b298 + 13*b299 + 14*b300
      + 15*b301 + 16*b302 + 17*b303 + 18*b304 + 19*b305 + 20*b306 + 21*b307
      + 22*b308 + 23*b309 + 24*b310 + 25*b311 + 26*b312 + 27*b313 + 28*b314
      <= 0;

e54:  - 28*b5 + b315 + 2*b316 + 3*b317 + 4*b318 + 5*b319 + 6*b320 + 7*b321
      + 8*b322 + 9*b323 + 10*b324 + 11*b325 + 12*b326 + 13*b327 + 14*b328
      + 15*b329 + 16*b330 + 17*b331 + 18*b332 + 19*b333 + 20*b334 + 21*b335
      + 22*b336 + 23*b337 + 24*b338 + 25*b339 + 26*b340 + 27*b341 + 28*b342
      <= 0;

e55:  - 22*b6 + b343 + 2*b344 + 3*b345 + 4*b346 + 5*b347 + 6*b348 + 7*b349
      + 8*b350 + 9*b351 + 10*b352 + 11*b353 + 12*b354 + 13*b355 + 14*b356
      + 15*b357 + 16*b358 + 17*b359 + 18*b360 + 19*b361 + 20*b362 + 21*b363
      + 22*b364 <= 0;

e56:  - 21*b7 + b365 + 2*b366 + 3*b367 + 4*b368 + 5*b369 + 6*b370 + 7*b371
      + 8*b372 + 9*b373 + 10*b374 + 11*b375 + 12*b376 + 13*b377 + 14*b378
      + 15*b379 + 16*b380 + 17*b381 + 18*b382 + 19*b383 + 20*b384 + 21*b385
      <= 0;

e57:  - 10*b8 + b386 + 2*b387 + 3*b388 + 4*b389 + 5*b390 + 6*b391 + 7*b392
      + 8*b393 + 9*b394 + 10*b395 <= 0;

e58:  - 9*b9 + b396 + 2*b397 + 3*b398 + 4*b399 + 5*b400 + 6*b401 + 7*b402
      + 8*b403 + 9*b404 <= 0;

e59:  - 8*b10 + b405 + 2*b406 + 3*b407 + 4*b408 + 5*b409 + 6*b410 + 7*b411
      + 8*b412 <= 0;

e60:  - 8*b11 + b413 + 2*b414 + 3*b415 + 4*b416 + 5*b417 + 6*b418 + 7*b419
      + 8*b420 <= 0;

e61:  - 8*b12 + b421 + 2*b422 + 3*b423 + 4*b424 + 5*b425 + 6*b426 + 7*b427
      + 8*b428 <= 0;

e62:    i13 - 3*b169 - 8*b170 - 15*b171 - 24*b172 - 35*b173 - 48*b174 - 63*b175
      - 80*b176 - 99*b177 - 120*b178 - 143*b179 - 168*b180 - 195*b181
      - 224*b182 - 255*b183 - 288*b184 - 323*b185 - 360*b186 - 399*b187
      - 440*b188 - 483*b189 - 528*b190 - 575*b191 - 624*b192 - 675*b193
      - 728*b194 - 783*b195 - 840*b196 - 899*b197 - 960*b198 - 1023*b199
      - 1088*b200 - 1155*b201 - 1224*b202 - 1295*b203 - 1368*b204 - 1443*b205
      - 1520*b206 - 1599*b207 - 1680*b208 - 1763*b209 - 1848*b210 - 1935*b211
      - 2024*b212 - 2115*b213 - 2208*b214 - 2303*b215 - 2400*b216 = 1;

e63:    i14 - 3*b217 - 8*b218 - 15*b219 - 24*b220 - 35*b221 - 48*b222 - 63*b223
      - 80*b224 - 99*b225 - 120*b226 - 143*b227 - 168*b228 - 195*b229
      - 224*b230 - 255*b231 - 288*b232 - 323*b233 - 360*b234 - 399*b235
      - 440*b236 - 483*b237 - 528*b238 - 575*b239 - 624*b240 - 675*b241
      - 728*b242 - 783*b243 - 840*b244 - 899*b245 - 960*b246 - 1023*b247
      - 1088*b248 - 1155*b249 - 1224*b250 - 1295*b251 - 1368*b252 - 1443*b253
      - 1520*b254 - 1599*b255 - 1680*b256 = 1;

e64:    i15 - 3*b257 - 8*b258 - 15*b259 - 24*b260 - 35*b261 - 48*b262 - 63*b263
      - 80*b264 - 99*b265 - 120*b266 - 143*b267 - 168*b268 - 195*b269
      - 224*b270 - 255*b271 - 288*b272 - 323*b273 - 360*b274 - 399*b275
      - 440*b276 - 483*b277 - 528*b278 - 575*b279 - 624*b280 - 675*b281
      - 728*b282 - 783*b283 - 840*b284 - 899*b285 - 960*b286 = 1;

e65:    i16 - 3*b287 - 8*b288 - 15*b289 - 24*b290 - 35*b291 - 48*b292 - 63*b293
      - 80*b294 - 99*b295 - 120*b296 - 143*b297 - 168*b298 - 195*b299
      - 224*b300 - 255*b301 - 288*b302 - 323*b303 - 360*b304 - 399*b305
      - 440*b306 - 483*b307 - 528*b308 - 575*b309 - 624*b310 - 675*b311
      - 728*b312 - 783*b313 - 840*b314 = 1;

e66:    i17 - 3*b315 - 8*b316 - 15*b317 - 24*b318 - 35*b319 - 48*b320 - 63*b321
      - 80*b322 - 99*b323 - 120*b324 - 143*b325 - 168*b326 - 195*b327
      - 224*b328 - 255*b329 - 288*b330 - 323*b331 - 360*b332 - 399*b333
      - 440*b334 - 483*b335 - 528*b336 - 575*b337 - 624*b338 - 675*b339
      - 728*b340 - 783*b341 - 840*b342 = 1;

e67:    i18 - 3*b343 - 8*b344 - 15*b345 - 24*b346 - 35*b347 - 48*b348 - 63*b349
      - 80*b350 - 99*b351 - 120*b352 - 143*b353 - 168*b354 - 195*b355
      - 224*b356 - 255*b357 - 288*b358 - 323*b359 - 360*b360 - 399*b361
      - 440*b362 - 483*b363 - 528*b364 = 1;

e68:    i19 - 3*b365 - 8*b366 - 15*b367 - 24*b368 - 35*b369 - 48*b370 - 63*b371
      - 80*b372 - 99*b373 - 120*b374 - 143*b375 - 168*b376 - 195*b377
      - 224*b378 - 255*b379 - 288*b380 - 323*b381 - 360*b382 - 399*b383
      - 440*b384 - 483*b385 = 1;

e69:    i20 - 3*b386 - 8*b387 - 15*b388 - 24*b389 - 35*b390 - 48*b391 - 63*b392
      - 80*b393 - 99*b394 - 120*b395 = 1;

e70:    i21 - 3*b396 - 8*b397 - 15*b398 - 24*b399 - 35*b400 - 48*b401 - 63*b402
      - 80*b403 - 99*b404 = 1;

e71:    i22 - 3*b405 - 8*b406 - 15*b407 - 24*b408 - 35*b409 - 48*b410 - 63*b411
      - 80*b412 = 1;

e72:    i23 - 3*b413 - 8*b414 - 15*b415 - 24*b416 - 35*b417 - 48*b418 - 63*b419
      - 80*b420 = 1;

e73:    i24 - 3*b421 - 8*b422 - 15*b423 - 24*b424 - 35*b425 - 48*b426 - 63*b427
      - 80*b428 = 1;

e74:    b169 + b170 + b171 + b172 + b173 + b174 + b175 + b176 + b177 + b178
      + b179 + b180 + b181 + b182 + b183 + b184 + b185 + b186 + b187 + b188
      + b189 + b190 + b191 + b192 + b193 + b194 + b195 + b196 + b197 + b198
      + b199 + b200 + b201 + b202 + b203 + b204 + b205 + b206 + b207 + b208
      + b209 + b210 + b211 + b212 + b213 + b214 + b215 + b216 <= 1;

e75:    b217 + b218 + b219 + b220 + b221 + b222 + b223 + b224 + b225 + b226
      + b227 + b228 + b229 + b230 + b231 + b232 + b233 + b234 + b235 + b236
      + b237 + b238 + b239 + b240 + b241 + b242 + b243 + b244 + b245 + b246
      + b247 + b248 + b249 + b250 + b251 + b252 + b253 + b254 + b255 + b256
      <= 1;

e76:    b257 + b258 + b259 + b260 + b261 + b262 + b263 + b264 + b265 + b266
      + b267 + b268 + b269 + b270 + b271 + b272 + b273 + b274 + b275 + b276
      + b277 + b278 + b279 + b280 + b281 + b282 + b283 + b284 + b285 + b286
      <= 1;

e77:    b287 + b288 + b289 + b290 + b291 + b292 + b293 + b294 + b295 + b296
      + b297 + b298 + b299 + b300 + b301 + b302 + b303 + b304 + b305 + b306
      + b307 + b308 + b309 + b310 + b311 + b312 + b313 + b314 <= 1;

e78:    b315 + b316 + b317 + b318 + b319 + b320 + b321 + b322 + b323 + b324
      + b325 + b326 + b327 + b328 + b329 + b330 + b331 + b332 + b333 + b334
      + b335 + b336 + b337 + b338 + b339 + b340 + b341 + b342 <= 1;

e79:    b343 + b344 + b345 + b346 + b347 + b348 + b349 + b350 + b351 + b352
      + b353 + b354 + b355 + b356 + b357 + b358 + b359 + b360 + b361 + b362
      + b363 + b364 <= 1;

e80:    b365 + b366 + b367 + b368 + b369 + b370 + b371 + b372 + b373 + b374
      + b375 + b376 + b377 + b378 + b379 + b380 + b381 + b382 + b383 + b384
      + b385 <= 1;

e81:    b386 + b387 + b388 + b389 + b390 + b391 + b392 + b393 + b394 + b395
      <= 1;

e82:    b396 + b397 + b398 + b399 + b400 + b401 + b402 + b403 + b404 <= 1;

e83:    b405 + b406 + b407 + b408 + b409 + b410 + b411 + b412 <= 1;

e84:    b413 + b414 + b415 + b416 + b417 + b418 + b419 + b420 <= 1;

e85:    b421 + b422 + b423 + b424 + b425 + b426 + b427 + b428 <= 1;

e86:    x25 - 3*b429 - 8*b430 - 15*b431 - 24*b432 - 35*b433 - 48*b434 = 1;

e87:    x26 - 3*b435 - 8*b436 - 15*b437 - 24*b438 - 35*b439 - 48*b440 = 1;

e88:    x27 - 3*b441 - 8*b442 - 15*b443 - 24*b444 - 35*b445 - 48*b446 = 1;

e89:    x28 - 3*b447 - 8*b448 - 15*b449 - 24*b450 - 35*b451 - 48*b452 = 1;

e90:    x29 - 3*b453 - 8*b454 - 15*b455 - 24*b456 - 35*b457 - 48*b458 = 1;

e91:    x30 - 3*b459 - 8*b460 - 15*b461 - 24*b462 - 35*b463 - 48*b464 = 1;

e92:    x31 - 3*b465 - 8*b466 - 15*b467 - 24*b468 - 35*b469 - 48*b470 = 1;

e93:    x32 - 3*b471 - 8*b472 - 15*b473 - 24*b474 - 35*b475 - 48*b476 = 1;

e94:    x33 - 3*b477 - 8*b478 - 15*b479 - 24*b480 - 35*b481 - 48*b482 = 1;

e95:    x34 - 3*b483 - 8*b484 - 15*b485 - 24*b486 - 35*b487 - 48*b488 = 1;

e96:    x35 - 3*b489 - 8*b490 - 15*b491 - 24*b492 - 35*b493 - 48*b494 = 1;

e97:    x36 - 3*b495 - 8*b496 - 15*b497 - 24*b498 - 35*b499 - 48*b500 = 1;

e98:    x37 - 3*b501 - 8*b502 - 15*b503 - 24*b504 = 1;

e99:    x38 - 3*b505 - 8*b506 - 15*b507 - 24*b508 = 1;

e100:    x39 - 3*b509 - 8*b510 - 15*b511 - 24*b512 = 1;

e101:    x40 - 3*b513 - 8*b514 - 15*b515 - 24*b516 = 1;

e102:    x41 - 3*b517 - 8*b518 - 15*b519 - 24*b520 = 1;

e103:    x42 - 3*b521 - 8*b522 - 15*b523 - 24*b524 = 1;

e104:    x43 - 3*b525 - 8*b526 - 15*b527 - 24*b528 = 1;

e105:    x44 - 3*b529 - 8*b530 - 15*b531 - 24*b532 = 1;

e106:    x45 - 3*b533 - 8*b534 - 15*b535 - 24*b536 = 1;

e107:    x46 - 3*b537 - 8*b538 - 15*b539 - 24*b540 = 1;

e108:    x47 - 3*b541 - 8*b542 - 15*b543 - 24*b544 = 1;

e109:    x48 - 3*b545 - 8*b546 - 15*b547 - 24*b548 = 1;

e110:    x49 - 3*b549 - 8*b550 - 15*b551 = 1;

e111:    x50 - 3*b552 - 8*b553 - 15*b554 = 1;

e112:    x51 - 3*b555 - 8*b556 - 15*b557 = 1;

e113:    x52 - 3*b558 - 8*b559 - 15*b560 = 1;

e114:    x53 - 3*b561 - 8*b562 - 15*b563 = 1;

e115:    x54 - 3*b564 - 8*b565 - 15*b566 = 1;

e116:    x55 - 3*b567 - 8*b568 - 15*b569 = 1;

e117:    x56 - 3*b570 - 8*b571 - 15*b572 = 1;

e118:    x57 - 3*b573 - 8*b574 - 15*b575 = 1;

e119:    x58 - 3*b576 - 8*b577 - 15*b578 = 1;

e120:    x59 - 3*b579 - 8*b580 - 15*b581 = 1;

e121:    x60 - 3*b582 - 8*b583 - 15*b584 = 1;

e122:    x61 - 3*b585 - 8*b586 - 15*b587 = 1;

e123:    x62 - 3*b588 - 8*b589 - 15*b590 = 1;

e124:    x63 - 3*b591 - 8*b592 - 15*b593 = 1;

e125:    x64 - 3*b594 - 8*b595 - 15*b596 = 1;

e126:    x65 - 3*b597 - 8*b598 - 15*b599 = 1;

e127:    x66 - 3*b600 - 8*b601 - 15*b602 = 1;

e128:    x67 - 3*b603 - 8*b604 - 15*b605 = 1;

e129:    x68 - 3*b606 - 8*b607 - 15*b608 = 1;

e130:    x69 - 3*b609 - 8*b610 - 15*b611 = 1;

e131:    x70 - 3*b612 - 8*b613 - 15*b614 = 1;

e132:    x71 - 3*b615 - 8*b616 - 15*b617 = 1;

e133:    x72 - 3*b618 - 8*b619 - 15*b620 = 1;

e134:    x73 - 3*b621 - 8*b622 - 15*b623 = 1;

e135:    x74 - 3*b624 - 8*b625 - 15*b626 = 1;

e136:    x75 - 3*b627 - 8*b628 - 15*b629 = 1;

e137:    x76 - 3*b630 - 8*b631 - 15*b632 = 1;

e138:    x77 - 3*b633 - 8*b634 - 15*b635 = 1;

e139:    x78 - 3*b636 - 8*b637 - 15*b638 = 1;

e140:    x79 - 3*b639 - 8*b640 - 15*b641 = 1;

e141:    x80 - 3*b642 - 8*b643 - 15*b644 = 1;

e142:    x81 - 3*b645 - 8*b646 - 15*b647 = 1;

e143:    x82 - 3*b648 - 8*b649 - 15*b650 = 1;

e144:    x83 - 3*b651 - 8*b652 - 15*b653 = 1;

e145:    x84 - 3*b654 - 8*b655 - 15*b656 = 1;

e146:    x85 - 3*b657 - 8*b658 = 1;

e147:    x86 - 3*b659 - 8*b660 = 1;

e148:    x87 - 3*b661 - 8*b662 = 1;

e149:    x88 - 3*b663 - 8*b664 = 1;

e150:    x89 - 3*b665 - 8*b666 = 1;

e151:    x90 - 3*b667 - 8*b668 = 1;

e152:    x91 - 3*b669 - 8*b670 = 1;

e153:    x92 - 3*b671 - 8*b672 = 1;

e154:    x93 - 3*b673 - 8*b674 = 1;

e155:    x94 - 3*b675 - 8*b676 = 1;

e156:    x95 - 3*b677 - 8*b678 = 1;

e157:    x96 - 3*b679 - 8*b680 = 1;

e158:    x97 - 3*b681 - 8*b682 = 1;

e159:    x98 - 3*b683 - 8*b684 = 1;

e160:    x99 - 3*b685 - 8*b686 = 1;

e161:    x100 - 3*b687 - 8*b688 = 1;

e162:    x101 - 3*b689 - 8*b690 = 1;

e163:    x102 - 3*b691 - 8*b692 = 1;

e164:    x103 - 3*b693 - 8*b694 = 1;

e165:    x104 - 3*b695 - 8*b696 = 1;

e166:    x105 - 3*b697 - 8*b698 = 1;

e167:    x106 - 3*b699 - 8*b700 = 1;

e168:    x107 - 3*b701 - 8*b702 = 1;

e169:    x108 - 3*b703 - 8*b704 = 1;

e170:    x109 - 3*b705 - 8*b706 = 1;

e171:    x110 - 3*b707 - 8*b708 = 1;

e172:    x111 - 3*b709 - 8*b710 = 1;

e173:    x112 - 3*b711 - 8*b712 = 1;

e174:    x113 - 3*b713 - 8*b714 = 1;

e175:    x114 - 3*b715 - 8*b716 = 1;

e176:    x115 - 3*b717 - 8*b718 = 1;

e177:    x116 - 3*b719 - 8*b720 = 1;

e178:    x117 - 3*b721 - 8*b722 = 1;

e179:    x118 - 3*b723 - 8*b724 = 1;

e180:    x119 - 3*b725 - 8*b726 = 1;

e181:    x120 - 3*b727 - 8*b728 = 1;

e182:    x121 - 3*b729 - 8*b730 = 1;

e183:    x122 - 3*b731 - 8*b732 = 1;

e184:    x123 - 3*b733 - 8*b734 = 1;

e185:    x124 - 3*b735 - 8*b736 = 1;

e186:    x125 - 3*b737 - 8*b738 = 1;

e187:    x126 - 3*b739 - 8*b740 = 1;

e188:    x127 - 3*b741 - 8*b742 = 1;

e189:    x128 - 3*b743 - 8*b744 = 1;

e190:    x129 - 3*b745 - 8*b746 = 1;

e191:    x130 - 3*b747 - 8*b748 = 1;

e192:    x131 - 3*b749 - 8*b750 = 1;

e193:    x132 - 3*b751 - 8*b752 = 1;

e194:    x133 - 3*b753 - 8*b754 = 1;

e195:    x134 - 3*b755 - 8*b756 = 1;

e196:    x135 - 3*b757 - 8*b758 = 1;

e197:    x136 - 3*b759 - 8*b760 = 1;

e198:    x137 - 3*b761 - 8*b762 = 1;

e199:    x138 - 3*b763 - 8*b764 = 1;

e200:    x139 - 3*b765 - 8*b766 = 1;

e201:    x140 - 3*b767 - 8*b768 = 1;

e202:    x141 - 3*b769 - 8*b770 = 1;

e203:    x142 - 3*b771 - 8*b772 = 1;

e204:    x143 - 3*b773 - 8*b774 = 1;

e205:    x144 - 3*b775 - 8*b776 = 1;

e206:    x145 - 3*b777 - 8*b778 = 1;

e207:    x146 - 3*b779 - 8*b780 = 1;

e208:    x147 - 3*b781 - 8*b782 = 1;

e209:    x148 - 3*b783 - 8*b784 = 1;

e210:    x149 - 3*b785 - 8*b786 = 1;

e211:    x150 - 3*b787 - 8*b788 = 1;

e212:    x151 - 3*b789 - 8*b790 = 1;

e213:    x152 - 3*b791 - 8*b792 = 1;

e214:    x153 - 3*b793 - 8*b794 = 1;

e215:    x154 - 3*b795 - 8*b796 = 1;

e216:    x155 - 3*b797 - 8*b798 = 1;

e217:    x156 - 3*b799 - 8*b800 = 1;

e218:    x157 - 3*b801 = 1;

e219:    x158 - 3*b802 = 1;

e220:    x159 - 3*b803 = 1;

e221:    x160 - 3*b804 = 1;

e222:    x161 - 3*b805 = 1;

e223:    x162 - 3*b806 = 1;

e224:    x163 - 3*b807 = 1;

e225:    x164 - 3*b808 = 1;

e226:    x165 - 3*b809 = 1;

e227:    x166 - 3*b810 = 1;

e228:    x167 - 3*b811 = 1;

e229:    x168 - 3*b812 = 1;

e230:    b429 + b430 + b431 + b432 + b433 + b434 <= 1;

e231:    b435 + b436 + b437 + b438 + b439 + b440 <= 1;

e232:    b441 + b442 + b443 + b444 + b445 + b446 <= 1;

e233:    b447 + b448 + b449 + b450 + b451 + b452 <= 1;

e234:    b453 + b454 + b455 + b456 + b457 + b458 <= 1;

e235:    b459 + b460 + b461 + b462 + b463 + b464 <= 1;

e236:    b465 + b466 + b467 + b468 + b469 + b470 <= 1;

e237:    b471 + b472 + b473 + b474 + b475 + b476 <= 1;

e238:    b477 + b478 + b479 + b480 + b481 + b482 <= 1;

e239:    b483 + b484 + b485 + b486 + b487 + b488 <= 1;

e240:    b489 + b490 + b491 + b492 + b493 + b494 <= 1;

e241:    b495 + b496 + b497 + b498 + b499 + b500 <= 1;

e242:    b501 + b502 + b503 + b504 <= 1;

e243:    b505 + b506 + b507 + b508 <= 1;

e244:    b509 + b510 + b511 + b512 <= 1;

e245:    b513 + b514 + b515 + b516 <= 1;

e246:    b517 + b518 + b519 + b520 <= 1;

e247:    b521 + b522 + b523 + b524 <= 1;

e248:    b525 + b526 + b527 + b528 <= 1;

e249:    b529 + b530 + b531 + b532 <= 1;

e250:    b533 + b534 + b535 + b536 <= 1;

e251:    b537 + b538 + b539 + b540 <= 1;

e252:    b541 + b542 + b543 + b544 <= 1;

e253:    b545 + b546 + b547 + b548 <= 1;

e254:    b549 + b550 + b551 <= 1;

e255:    b552 + b553 + b554 <= 1;

e256:    b555 + b556 + b557 <= 1;

e257:    b558 + b559 + b560 <= 1;

e258:    b561 + b562 + b563 <= 1;

e259:    b564 + b565 + b566 <= 1;

e260:    b567 + b568 + b569 <= 1;

e261:    b570 + b571 + b572 <= 1;

e262:    b573 + b574 + b575 <= 1;

e263:    b576 + b577 + b578 <= 1;

e264:    b579 + b580 + b581 <= 1;

e265:    b582 + b583 + b584 <= 1;

e266:    b585 + b586 + b587 <= 1;

e267:    b588 + b589 + b590 <= 1;

e268:    b591 + b592 + b593 <= 1;

e269:    b594 + b595 + b596 <= 1;

e270:    b597 + b598 + b599 <= 1;

e271:    b600 + b601 + b602 <= 1;

e272:    b603 + b604 + b605 <= 1;

e273:    b606 + b607 + b608 <= 1;

e274:    b609 + b610 + b611 <= 1;

e275:    b612 + b613 + b614 <= 1;

e276:    b615 + b616 + b617 <= 1;

e277:    b618 + b619 + b620 <= 1;

e278:    b621 + b622 + b623 <= 1;

e279:    b624 + b625 + b626 <= 1;

e280:    b627 + b628 + b629 <= 1;

e281:    b630 + b631 + b632 <= 1;

e282:    b633 + b634 + b635 <= 1;

e283:    b636 + b637 + b638 <= 1;

e284:    b639 + b640 + b641 <= 1;

e285:    b642 + b643 + b644 <= 1;

e286:    b645 + b646 + b647 <= 1;

e287:    b648 + b649 + b650 <= 1;

e288:    b651 + b652 + b653 <= 1;

e289:    b654 + b655 + b656 <= 1;

e290:    b657 + b658 <= 1;

e291:    b659 + b660 <= 1;

e292:    b661 + b662 <= 1;

e293:    b663 + b664 <= 1;

e294:    b665 + b666 <= 1;

e295:    b667 + b668 <= 1;

e296:    b669 + b670 <= 1;

e297:    b671 + b672 <= 1;

e298:    b673 + b674 <= 1;

e299:    b675 + b676 <= 1;

e300:    b677 + b678 <= 1;

e301:    b679 + b680 <= 1;

e302:    b681 + b682 <= 1;

e303:    b683 + b684 <= 1;

e304:    b685 + b686 <= 1;

e305:    b687 + b688 <= 1;

e306:    b689 + b690 <= 1;

e307:    b691 + b692 <= 1;

e308:    b693 + b694 <= 1;

e309:    b695 + b696 <= 1;

e310:    b697 + b698 <= 1;

e311:    b699 + b700 <= 1;

e312:    b701 + b702 <= 1;

e313:    b703 + b704 <= 1;

e314:    b705 + b706 <= 1;

e315:    b707 + b708 <= 1;

e316:    b709 + b710 <= 1;

e317:    b711 + b712 <= 1;

e318:    b713 + b714 <= 1;

e319:    b715 + b716 <= 1;

e320:    b717 + b718 <= 1;

e321:    b719 + b720 <= 1;

e322:    b721 + b722 <= 1;

e323:    b723 + b724 <= 1;

e324:    b725 + b726 <= 1;

e325:    b727 + b728 <= 1;

e326:    b729 + b730 <= 1;

e327:    b731 + b732 <= 1;

e328:    b733 + b734 <= 1;

e329:    b735 + b736 <= 1;

e330:    b737 + b738 <= 1;

e331:    b739 + b740 <= 1;

e332:    b741 + b742 <= 1;

e333:    b743 + b744 <= 1;

e334:    b745 + b746 <= 1;

e335:    b747 + b748 <= 1;

e336:    b749 + b750 <= 1;

e337:    b751 + b752 <= 1;

e338:    b753 + b754 <= 1;

e339:    b755 + b756 <= 1;

e340:    b757 + b758 <= 1;

e341:    b759 + b760 <= 1;

e342:    b761 + b762 <= 1;

e343:    b763 + b764 <= 1;

e344:    b765 + b766 <= 1;

e345:    b767 + b768 <= 1;

e346:    b769 + b770 <= 1;

e347:    b771 + b772 <= 1;

e348:    b773 + b774 <= 1;

e349:    b775 + b776 <= 1;

e350:    b777 + b778 <= 1;

e351:    b779 + b780 <= 1;

e352:    b781 + b782 <= 1;

e353:    b783 + b784 <= 1;

e354:    b785 + b786 <= 1;

e355:    b787 + b788 <= 1;

e356:    b789 + b790 <= 1;

e357:    b791 + b792 <= 1;

e358:    b793 + b794 <= 1;

e359:    b795 + b796 <= 1;

e360:    b797 + b798 <= 1;

e361:    b799 + b800 <= 1;

e362:    b801 <= 1;

e363:    b802 <= 1;

e364:    b803 <= 1;

e365:    b804 <= 1;

e366:    b805 <= 1;

e367:    b806 <= 1;

e368:    b807 <= 1;

e369:    b808 <= 1;

e370:    b809 <= 1;

e371:    b810 <= 1;

e372:    b811 <= 1;

e373:    b812 <= 1;

e374: -(sqrt(i13*x25) + sqrt(i14*x26) + sqrt(i15*x27) + sqrt(i16*x28) + sqrt(
      i17*x29) + sqrt(i18*x30) + sqrt(i19*x31) + sqrt(i20*x32) + sqrt(i21*x33)
       + sqrt(i22*x34) + sqrt(i23*x35) + sqrt(i24*x36)) + b169 + 2*b170
       + 3*b171 + 4*b172 + 5*b173 + 6*b174 + 7*b175 + 8*b176 + 9*b177 + 10*b178
       + 11*b179 + 12*b180 + 13*b181 + 14*b182 + 15*b183 + 16*b184 + 17*b185
       + 18*b186 + 19*b187 + 20*b188 + 21*b189 + 22*b190 + 23*b191 + 24*b192
       + 25*b193 + 26*b194 + 27*b195 + 28*b196 + 29*b197 + 30*b198 + 31*b199
       + 32*b200 + 33*b201 + 34*b202 + 35*b203 + 36*b204 + 37*b205 + 38*b206
       + 39*b207 + 40*b208 + 41*b209 + 42*b210 + 43*b211 + 44*b212 + 45*b213
       + 46*b214 + 47*b215 + 48*b216 + b217 + 2*b218 + 3*b219 + 4*b220 + 5*b221
       + 6*b222 + 7*b223 + 8*b224 + 9*b225 + 10*b226 + 11*b227 + 12*b228
       + 13*b229 + 14*b230 + 15*b231 + 16*b232 + 17*b233 + 18*b234 + 19*b235
       + 20*b236 + 21*b237 + 22*b238 + 23*b239 + 24*b240 + 25*b241 + 26*b242
       + 27*b243 + 28*b244 + 29*b245 + 30*b246 + 31*b247 + 32*b248 + 33*b249
       + 34*b250 + 35*b251 + 36*b252 + 37*b253 + 38*b254 + 39*b255 + 40*b256
       + b257 + 2*b258 + 3*b259 + 4*b260 + 5*b261 + 6*b262 + 7*b263 + 8*b264
       + 9*b265 + 10*b266 + 11*b267 + 12*b268 + 13*b269 + 14*b270 + 15*b271
       + 16*b272 + 17*b273 + 18*b274 + 19*b275 + 20*b276 + 21*b277 + 22*b278
       + 23*b279 + 24*b280 + 25*b281 + 26*b282 + 27*b283 + 28*b284 + 29*b285
       + 30*b286 + b287 + 2*b288 + 3*b289 + 4*b290 + 5*b291 + 6*b292 + 7*b293
       + 8*b294 + 9*b295 + 10*b296 + 11*b297 + 12*b298 + 13*b299 + 14*b300
       + 15*b301 + 16*b302 + 17*b303 + 18*b304 + 19*b305 + 20*b306 + 21*b307
       + 22*b308 + 23*b309 + 24*b310 + 25*b311 + 26*b312 + 27*b313 + 28*b314
       + b315 + 2*b316 + 3*b317 + 4*b318 + 5*b319 + 6*b320 + 7*b321 + 8*b322
       + 9*b323 + 10*b324 + 11*b325 + 12*b326 + 13*b327 + 14*b328 + 15*b329
       + 16*b330 + 17*b331 + 18*b332 + 19*b333 + 20*b334 + 21*b335 + 22*b336
       + 23*b337 + 24*b338 + 25*b339 + 26*b340 + 27*b341 + 28*b342 + b343
       + 2*b344 + 3*b345 + 4*b346 + 5*b347 + 6*b348 + 7*b349 + 8*b350 + 9*b351
       + 10*b352 + 11*b353 + 12*b354 + 13*b355 + 14*b356 + 15*b357 + 16*b358
       + 17*b359 + 18*b360 + 19*b361 + 20*b362 + 21*b363 + 22*b364 + b365
       + 2*b366 + 3*b367 + 4*b368 + 5*b369 + 6*b370 + 7*b371 + 8*b372 + 9*b373
       + 10*b374 + 11*b375 + 12*b376 + 13*b377 + 14*b378 + 15*b379 + 16*b380
       + 17*b381 + 18*b382 + 19*b383 + 20*b384 + 21*b385 + b386 + 2*b387
       + 3*b388 + 4*b389 + 5*b390 + 6*b391 + 7*b392 + 8*b393 + 9*b394 + 10*b395
       + b396 + 2*b397 + 3*b398 + 4*b399 + 5*b400 + 6*b401 + 7*b402 + 8*b403
       + 9*b404 + b405 + 2*b406 + 3*b407 + 4*b408 + 5*b409 + 6*b410 + 7*b411
       + 8*b412 + b413 + 2*b414 + 3*b415 + 4*b416 + 5*b417 + 6*b418 + 7*b419
       + 8*b420 + b421 + 2*b422 + 3*b423 + 4*b424 + 5*b425 + 6*b426 + 7*b427
       + 8*b428 + b429 + 2*b430 + 3*b431 + 4*b432 + 5*b433 + 6*b434 + b435
       + 2*b436 + 3*b437 + 4*b438 + 5*b439 + 6*b440 + b441 + 2*b442 + 3*b443
       + 4*b444 + 5*b445 + 6*b446 + b447 + 2*b448 + 3*b449 + 4*b450 + 5*b451
       + 6*b452 + b453 + 2*b454 + 3*b455 + 4*b456 + 5*b457 + 6*b458 + b459
       + 2*b460 + 3*b461 + 4*b462 + 5*b463 + 6*b464 + b465 + 2*b466 + 3*b467
       + 4*b468 + 5*b469 + 6*b470 + b471 + 2*b472 + 3*b473 + 4*b474 + 5*b475
       + 6*b476 + b477 + 2*b478 + 3*b479 + 4*b480 + 5*b481 + 6*b482 + b483
       + 2*b484 + 3*b485 + 4*b486 + 5*b487 + 6*b488 + b489 + 2*b490 + 3*b491
       + 4*b492 + 5*b493 + 6*b494 + b495 + 2*b496 + 3*b497 + 4*b498 + 5*b499
       + 6*b500 <= -22;

e375: -(sqrt(i13*x37) + sqrt(i14*x38) + sqrt(i15*x39) + sqrt(i16*x40) + sqrt(
      i17*x41) + sqrt(i18*x42) + sqrt(i19*x43) + sqrt(i20*x44) + sqrt(i21*x45)
       + sqrt(i22*x46) + sqrt(i23*x47) + sqrt(i24*x48)) + b169 + 2*b170
       + 3*b171 + 4*b172 + 5*b173 + 6*b174 + 7*b175 + 8*b176 + 9*b177 + 10*b178
       + 11*b179 + 12*b180 + 13*b181 + 14*b182 + 15*b183 + 16*b184 + 17*b185
       + 18*b186 + 19*b187 + 20*b188 + 21*b189 + 22*b190 + 23*b191 + 24*b192
       + 25*b193 + 26*b194 + 27*b195 + 28*b196 + 29*b197 + 30*b198 + 31*b199
       + 32*b200 + 33*b201 + 34*b202 + 35*b203 + 36*b204 + 37*b205 + 38*b206
       + 39*b207 + 40*b208 + 41*b209 + 42*b210 + 43*b211 + 44*b212 + 45*b213
       + 46*b214 + 47*b215 + 48*b216 + b217 + 2*b218 + 3*b219 + 4*b220 + 5*b221
       + 6*b222 + 7*b223 + 8*b224 + 9*b225 + 10*b226 + 11*b227 + 12*b228
       + 13*b229 + 14*b230 + 15*b231 + 16*b232 + 17*b233 + 18*b234 + 19*b235
       + 20*b236 + 21*b237 + 22*b238 + 23*b239 + 24*b240 + 25*b241 + 26*b242
       + 27*b243 + 28*b244 + 29*b245 + 30*b246 + 31*b247 + 32*b248 + 33*b249
       + 34*b250 + 35*b251 + 36*b252 + 37*b253 + 38*b254 + 39*b255 + 40*b256
       + b257 + 2*b258 + 3*b259 + 4*b260 + 5*b261 + 6*b262 + 7*b263 + 8*b264
       + 9*b265 + 10*b266 + 11*b267 + 12*b268 + 13*b269 + 14*b270 + 15*b271
       + 16*b272 + 17*b273 + 18*b274 + 19*b275 + 20*b276 + 21*b277 + 22*b278
       + 23*b279 + 24*b280 + 25*b281 + 26*b282 + 27*b283 + 28*b284 + 29*b285
       + 30*b286 + b287 + 2*b288 + 3*b289 + 4*b290 + 5*b291 + 6*b292 + 7*b293
       + 8*b294 + 9*b295 + 10*b296 + 11*b297 + 12*b298 + 13*b299 + 14*b300
       + 15*b301 + 16*b302 + 17*b303 + 18*b304 + 19*b305 + 20*b306 + 21*b307
       + 22*b308 + 23*b309 + 24*b310 + 25*b311 + 26*b312 + 27*b313 + 28*b314
       + b315 + 2*b316 + 3*b317 + 4*b318 + 5*b319 + 6*b320 + 7*b321 + 8*b322
       + 9*b323 + 10*b324 + 11*b325 + 12*b326 + 13*b327 + 14*b328 + 15*b329
       + 16*b330 + 17*b331 + 18*b332 + 19*b333 + 20*b334 + 21*b335 + 22*b336
       + 23*b337 + 24*b338 + 25*b339 + 26*b340 + 27*b341 + 28*b342 + b343
       + 2*b344 + 3*b345 + 4*b346 + 5*b347 + 6*b348 + 7*b349 + 8*b350 + 9*b351
       + 10*b352 + 11*b353 + 12*b354 + 13*b355 + 14*b356 + 15*b357 + 16*b358
       + 17*b359 + 18*b360 + 19*b361 + 20*b362 + 21*b363 + 22*b364 + b365
       + 2*b366 + 3*b367 + 4*b368 + 5*b369 + 6*b370 + 7*b371 + 8*b372 + 9*b373
       + 10*b374 + 11*b375 + 12*b376 + 13*b377 + 14*b378 + 15*b379 + 16*b380
       + 17*b381 + 18*b382 + 19*b383 + 20*b384 + 21*b385 + b386 + 2*b387
       + 3*b388 + 4*b389 + 5*b390 + 6*b391 + 7*b392 + 8*b393 + 9*b394 + 10*b395
       + b396 + 2*b397 + 3*b398 + 4*b399 + 5*b400 + 6*b401 + 7*b402 + 8*b403
       + 9*b404 + b405 + 2*b406 + 3*b407 + 4*b408 + 5*b409 + 6*b410 + 7*b411
       + 8*b412 + b413 + 2*b414 + 3*b415 + 4*b416 + 5*b417 + 6*b418 + 7*b419
       + 8*b420 + b421 + 2*b422 + 3*b423 + 4*b424 + 5*b425 + 6*b426 + 7*b427
       + 8*b428 + b501 + 2*b502 + 3*b503 + 4*b504 + b505 + 2*b506 + 3*b507
       + 4*b508 + b509 + 2*b510 + 3*b511 + 4*b512 + b513 + 2*b514 + 3*b515
       + 4*b516 + b517 + 2*b518 + 3*b519 + 4*b520 + b521 + 2*b522 + 3*b523
       + 4*b524 + b525 + 2*b526 + 3*b527 + 4*b528 + b529 + 2*b530 + 3*b531
       + 4*b532 + b533 + 2*b534 + 3*b535 + 4*b536 + b537 + 2*b538 + 3*b539
       + 4*b540 + b541 + 2*b542 + 3*b543 + 4*b544 + b545 + 2*b546 + 3*b547
       + 4*b548 <= -40;

e376: -(sqrt(i13*x49) + sqrt(i14*x50) + sqrt(i15*x51) + sqrt(i16*x52) + sqrt(
      i17*x53) + sqrt(i18*x54) + sqrt(i19*x55) + sqrt(i20*x56) + sqrt(i21*x57)
       + sqrt(i22*x58) + sqrt(i23*x59) + sqrt(i24*x60)) + b169 + 2*b170
       + 3*b171 + 4*b172 + 5*b173 + 6*b174 + 7*b175 + 8*b176 + 9*b177 + 10*b178
       + 11*b179 + 12*b180 + 13*b181 + 14*b182 + 15*b183 + 16*b184 + 17*b185
       + 18*b186 + 19*b187 + 20*b188 + 21*b189 + 22*b190 + 23*b191 + 24*b192
       + 25*b193 + 26*b194 + 27*b195 + 28*b196 + 29*b197 + 30*b198 + 31*b199
       + 32*b200 + 33*b201 + 34*b202 + 35*b203 + 36*b204 + 37*b205 + 38*b206
       + 39*b207 + 40*b208 + 41*b209 + 42*b210 + 43*b211 + 44*b212 + 45*b213
       + 46*b214 + 47*b215 + 48*b216 + b217 + 2*b218 + 3*b219 + 4*b220 + 5*b221
       + 6*b222 + 7*b223 + 8*b224 + 9*b225 + 10*b226 + 11*b227 + 12*b228
       + 13*b229 + 14*b230 + 15*b231 + 16*b232 + 17*b233 + 18*b234 + 19*b235
       + 20*b236 + 21*b237 + 22*b238 + 23*b239 + 24*b240 + 25*b241 + 26*b242
       + 27*b243 + 28*b244 + 29*b245 + 30*b246 + 31*b247 + 32*b248 + 33*b249
       + 34*b250 + 35*b251 + 36*b252 + 37*b253 + 38*b254 + 39*b255 + 40*b256
       + b257 + 2*b258 + 3*b259 + 4*b260 + 5*b261 + 6*b262 + 7*b263 + 8*b264
       + 9*b265 + 10*b266 + 11*b267 + 12*b268 + 13*b269 + 14*b270 + 15*b271
       + 16*b272 + 17*b273 + 18*b274 + 19*b275 + 20*b276 + 21*b277 + 22*b278
       + 23*b279 + 24*b280 + 25*b281 + 26*b282 + 27*b283 + 28*b284 + 29*b285
       + 30*b286 + b287 + 2*b288 + 3*b289 + 4*b290 + 5*b291 + 6*b292 + 7*b293
       + 8*b294 + 9*b295 + 10*b296 + 11*b297 + 12*b298 + 13*b299 + 14*b300
       + 15*b301 + 16*b302 + 17*b303 + 18*b304 + 19*b305 + 20*b306 + 21*b307
       + 22*b308 + 23*b309 + 24*b310 + 25*b311 + 26*b312 + 27*b313 + 28*b314
       + b315 + 2*b316 + 3*b317 + 4*b318 + 5*b319 + 6*b320 + 7*b321 + 8*b322
       + 9*b323 + 10*b324 + 11*b325 + 12*b326 + 13*b327 + 14*b328 + 15*b329
       + 16*b330 + 17*b331 + 18*b332 + 19*b333 + 20*b334 + 21*b335 + 22*b336
       + 23*b337 + 24*b338 + 25*b339 + 26*b340 + 27*b341 + 28*b342 + b343
       + 2*b344 + 3*b345 + 4*b346 + 5*b347 + 6*b348 + 7*b349 + 8*b350 + 9*b351
       + 10*b352 + 11*b353 + 12*b354 + 13*b355 + 14*b356 + 15*b357 + 16*b358
       + 17*b359 + 18*b360 + 19*b361 + 20*b362 + 21*b363 + 22*b364 + b365
       + 2*b366 + 3*b367 + 4*b368 + 5*b369 + 6*b370 + 7*b371 + 8*b372 + 9*b373
       + 10*b374 + 11*b375 + 12*b376 + 13*b377 + 14*b378 + 15*b379 + 16*b380
       + 17*b381 + 18*b382 + 19*b383 + 20*b384 + 21*b385 + b386 + 2*b387
       + 3*b388 + 4*b389 + 5*b390 + 6*b391 + 7*b392 + 8*b393 + 9*b394 + 10*b395
       + b396 + 2*b397 + 3*b398 + 4*b399 + 5*b400 + 6*b401 + 7*b402 + 8*b403
       + 9*b404 + b405 + 2*b406 + 3*b407 + 4*b408 + 5*b409 + 6*b410 + 7*b411
       + 8*b412 + b413 + 2*b414 + 3*b415 + 4*b416 + 5*b417 + 6*b418 + 7*b419
       + 8*b420 + b421 + 2*b422 + 3*b423 + 4*b424 + 5*b425 + 6*b426 + 7*b427
       + 8*b428 + b549 + 2*b550 + 3*b551 + b552 + 2*b553 + 3*b554 + b555
       + 2*b556 + 3*b557 + b558 + 2*b559 + 3*b560 + b561 + 2*b562 + 3*b563
       + b564 + 2*b565 + 3*b566 + b567 + 2*b568 + 3*b569 + b570 + 2*b571
       + 3*b572 + b573 + 2*b574 + 3*b575 + b576 + 2*b577 + 3*b578 + b579
       + 2*b580 + 3*b581 + b582 + 2*b583 + 3*b584 <= -60;

e377: -(sqrt(i13*x61) + sqrt(i14*x62) + sqrt(i15*x63) + sqrt(i16*x64) + sqrt(
      i17*x65) + sqrt(i18*x66) + sqrt(i19*x67) + sqrt(i20*x68) + sqrt(i21*x69)
       + sqrt(i22*x70) + sqrt(i23*x71) + sqrt(i24*x72)) + b169 + 2*b170
       + 3*b171 + 4*b172 + 5*b173 + 6*b174 + 7*b175 + 8*b176 + 9*b177 + 10*b178
       + 11*b179 + 12*b180 + 13*b181 + 14*b182 + 15*b183 + 16*b184 + 17*b185
       + 18*b186 + 19*b187 + 20*b188 + 21*b189 + 22*b190 + 23*b191 + 24*b192
       + 25*b193 + 26*b194 + 27*b195 + 28*b196 + 29*b197 + 30*b198 + 31*b199
       + 32*b200 + 33*b201 + 34*b202 + 35*b203 + 36*b204 + 37*b205 + 38*b206
       + 39*b207 + 40*b208 + 41*b209 + 42*b210 + 43*b211 + 44*b212 + 45*b213
       + 46*b214 + 47*b215 + 48*b216 + b217 + 2*b218 + 3*b219 + 4*b220 + 5*b221
       + 6*b222 + 7*b223 + 8*b224 + 9*b225 + 10*b226 + 11*b227 + 12*b228
       + 13*b229 + 14*b230 + 15*b231 + 16*b232 + 17*b233 + 18*b234 + 19*b235
       + 20*b236 + 21*b237 + 22*b238 + 23*b239 + 24*b240 + 25*b241 + 26*b242
       + 27*b243 + 28*b244 + 29*b245 + 30*b246 + 31*b247 + 32*b248 + 33*b249
       + 34*b250 + 35*b251 + 36*b252 + 37*b253 + 38*b254 + 39*b255 + 40*b256
       + b257 + 2*b258 + 3*b259 + 4*b260 + 5*b261 + 6*b262 + 7*b263 + 8*b264
       + 9*b265 + 10*b266 + 11*b267 + 12*b268 + 13*b269 + 14*b270 + 15*b271
       + 16*b272 + 17*b273 + 18*b274 + 19*b275 + 20*b276 + 21*b277 + 22*b278
       + 23*b279 + 24*b280 + 25*b281 + 26*b282 + 27*b283 + 28*b284 + 29*b285
       + 30*b286 + b287 + 2*b288 + 3*b289 + 4*b290 + 5*b291 + 6*b292 + 7*b293
       + 8*b294 + 9*b295 + 10*b296 + 11*b297 + 12*b298 + 13*b299 + 14*b300
       + 15*b301 + 16*b302 + 17*b303 + 18*b304 + 19*b305 + 20*b306 + 21*b307
       + 22*b308 + 23*b309 + 24*b310 + 25*b311 + 26*b312 + 27*b313 + 28*b314
       + b315 + 2*b316 + 3*b317 + 4*b318 + 5*b319 + 6*b320 + 7*b321 + 8*b322
       + 9*b323 + 10*b324 + 11*b325 + 12*b326 + 13*b327 + 14*b328 + 15*b329
       + 16*b330 + 17*b331 + 18*b332 + 19*b333 + 20*b334 + 21*b335 + 22*b336
       + 23*b337 + 24*b338 + 25*b339 + 26*b340 + 27*b341 + 28*b342 + b343
       + 2*b344 + 3*b345 + 4*b346 + 5*b347 + 6*b348 + 7*b349 + 8*b350 + 9*b351
       + 10*b352 + 11*b353 + 12*b354 + 13*b355 + 14*b356 + 15*b357 + 16*b358
       + 17*b359 + 18*b360 + 19*b361 + 20*b362 + 21*b363 + 22*b364 + b365
       + 2*b366 + 3*b367 + 4*b368 + 5*b369 + 6*b370 + 7*b371 + 8*b372 + 9*b373
       + 10*b374 + 11*b375 + 12*b376 + 13*b377 + 14*b378 + 15*b379 + 16*b380
       + 17*b381 + 18*b382 + 19*b383 + 20*b384 + 21*b385 + b386 + 2*b387
       + 3*b388 + 4*b389 + 5*b390 + 6*b391 + 7*b392 + 8*b393 + 9*b394 + 10*b395
       + b396 + 2*b397 + 3*b398 + 4*b399 + 5*b400 + 6*b401 + 7*b402 + 8*b403
       + 9*b404 + b405 + 2*b406 + 3*b407 + 4*b408 + 5*b409 + 6*b410 + 7*b411
       + 8*b412 + b413 + 2*b414 + 3*b415 + 4*b416 + 5*b417 + 6*b418 + 7*b419
       + 8*b420 + b421 + 2*b422 + 3*b423 + 4*b424 + 5*b425 + 6*b426 + 7*b427
       + 8*b428 + b585 + 2*b586 + 3*b587 + b588 + 2*b589 + 3*b590 + b591
       + 2*b592 + 3*b593 + b594 + 2*b595 + 3*b596 + b597 + 2*b598 + 3*b599
       + b600 + 2*b601 + 3*b602 + b603 + 2*b604 + 3*b605 + b606 + 2*b607
       + 3*b608 + b609 + 2*b610 + 3*b611 + b612 + 2*b613 + 3*b614 + b615
       + 2*b616 + 3*b617 + b618 + 2*b619 + 3*b620 <= -40;

e378: -(sqrt(i13*x73) + sqrt(i14*x74) + sqrt(i15*x75) + sqrt(i16*x76) + sqrt(
      i17*x77) + sqrt(i18*x78) + sqrt(i19*x79) + sqrt(i20*x80) + sqrt(i21*x81)
       + sqrt(i22*x82) + sqrt(i23*x83) + sqrt(i24*x84)) + b169 + 2*b170
       + 3*b171 + 4*b172 + 5*b173 + 6*b174 + 7*b175 + 8*b176 + 9*b177 + 10*b178
       + 11*b179 + 12*b180 + 13*b181 + 14*b182 + 15*b183 + 16*b184 + 17*b185
       + 18*b186 + 19*b187 + 20*b188 + 21*b189 + 22*b190 + 23*b191 + 24*b192
       + 25*b193 + 26*b194 + 27*b195 + 28*b196 + 29*b197 + 30*b198 + 31*b199
       + 32*b200 + 33*b201 + 34*b202 + 35*b203 + 36*b204 + 37*b205 + 38*b206
       + 39*b207 + 40*b208 + 41*b209 + 42*b210 + 43*b211 + 44*b212 + 45*b213
       + 46*b214 + 47*b215 + 48*b216 + b217 + 2*b218 + 3*b219 + 4*b220 + 5*b221
       + 6*b222 + 7*b223 + 8*b224 + 9*b225 + 10*b226 + 11*b227 + 12*b228
       + 13*b229 + 14*b230 + 15*b231 + 16*b232 + 17*b233 + 18*b234 + 19*b235
       + 20*b236 + 21*b237 + 22*b238 + 23*b239 + 24*b240 + 25*b241 + 26*b242
       + 27*b243 + 28*b244 + 29*b245 + 30*b246 + 31*b247 + 32*b248 + 33*b249
       + 34*b250 + 35*b251 + 36*b252 + 37*b253 + 38*b254 + 39*b255 + 40*b256
       + b257 + 2*b258 + 3*b259 + 4*b260 + 5*b261 + 6*b262 + 7*b263 + 8*b264
       + 9*b265 + 10*b266 + 11*b267 + 12*b268 + 13*b269 + 14*b270 + 15*b271
       + 16*b272 + 17*b273 + 18*b274 + 19*b275 + 20*b276 + 21*b277 + 22*b278
       + 23*b279 + 24*b280 + 25*b281 + 26*b282 + 27*b283 + 28*b284 + 29*b285
       + 30*b286 + b287 + 2*b288 + 3*b289 + 4*b290 + 5*b291 + 6*b292 + 7*b293
       + 8*b294 + 9*b295 + 10*b296 + 11*b297 + 12*b298 + 13*b299 + 14*b300
       + 15*b301 + 16*b302 + 17*b303 + 18*b304 + 19*b305 + 20*b306 + 21*b307
       + 22*b308 + 23*b309 + 24*b310 + 25*b311 + 26*b312 + 27*b313 + 28*b314
       + b315 + 2*b316 + 3*b317 + 4*b318 + 5*b319 + 6*b320 + 7*b321 + 8*b322
       + 9*b323 + 10*b324 + 11*b325 + 12*b326 + 13*b327 + 14*b328 + 15*b329
       + 16*b330 + 17*b331 + 18*b332 + 19*b333 + 20*b334 + 21*b335 + 22*b336
       + 23*b337 + 24*b338 + 25*b339 + 26*b340 + 27*b341 + 28*b342 + b343
       + 2*b344 + 3*b345 + 4*b346 + 5*b347 + 6*b348 + 7*b349 + 8*b350 + 9*b351
       + 10*b352 + 11*b353 + 12*b354 + 13*b355 + 14*b356 + 15*b357 + 16*b358
       + 17*b359 + 18*b360 + 19*b361 + 20*b362 + 21*b363 + 22*b364 + b365
       + 2*b366 + 3*b367 + 4*b368 + 5*b369 + 6*b370 + 7*b371 + 8*b372 + 9*b373
       + 10*b374 + 11*b375 + 12*b376 + 13*b377 + 14*b378 + 15*b379 + 16*b380
       + 17*b381 + 18*b382 + 19*b383 + 20*b384 + 21*b385 + b386 + 2*b387
       + 3*b388 + 4*b389 + 5*b390 + 6*b391 + 7*b392 + 8*b393 + 9*b394 + 10*b395
       + b396 + 2*b397 + 3*b398 + 4*b399 + 5*b400 + 6*b401 + 7*b402 + 8*b403
       + 9*b404 + b405 + 2*b406 + 3*b407 + 4*b408 + 5*b409 + 6*b410 + 7*b411
       + 8*b412 + b413 + 2*b414 + 3*b415 + 4*b416 + 5*b417 + 6*b418 + 7*b419
       + 8*b420 + b421 + 2*b422 + 3*b423 + 4*b424 + 5*b425 + 6*b426 + 7*b427
       + 8*b428 + b621 + 2*b622 + 3*b623 + b624 + 2*b625 + 3*b626 + b627
       + 2*b628 + 3*b629 + b630 + 2*b631 + 3*b632 + b633 + 2*b634 + 3*b635
       + b636 + 2*b637 + 3*b638 + b639 + 2*b640 + 3*b641 + b642 + 2*b643
       + 3*b644 + b645 + 2*b646 + 3*b647 + b648 + 2*b649 + 3*b650 + b651
       + 2*b652 + 3*b653 + b654 + 2*b655 + 3*b656 <= -52;

e379: -(sqrt(i13*x85) + sqrt(i14*x86) + sqrt(i15*x87) + sqrt(i16*x88) + sqrt(
      i17*x89) + sqrt(i18*x90) + sqrt(i19*x91) + sqrt(i20*x92) + sqrt(i21*x93)
       + sqrt(i22*x94) + sqrt(i23*x95) + sqrt(i24*x96)) + b169 + 2*b170
       + 3*b171 + 4*b172 + 5*b173 + 6*b174 + 7*b175 + 8*b176 + 9*b177 + 10*b178
       + 11*b179 + 12*b180 + 13*b181 + 14*b182 + 15*b183 + 16*b184 + 17*b185
       + 18*b186 + 19*b187 + 20*b188 + 21*b189 + 22*b190 + 23*b191 + 24*b192
       + 25*b193 + 26*b194 + 27*b195 + 28*b196 + 29*b197 + 30*b198 + 31*b199
       + 32*b200 + 33*b201 + 34*b202 + 35*b203 + 36*b204 + 37*b205 + 38*b206
       + 39*b207 + 40*b208 + 41*b209 + 42*b210 + 43*b211 + 44*b212 + 45*b213
       + 46*b214 + 47*b215 + 48*b216 + b217 + 2*b218 + 3*b219 + 4*b220 + 5*b221
       + 6*b222 + 7*b223 + 8*b224 + 9*b225 + 10*b226 + 11*b227 + 12*b228
       + 13*b229 + 14*b230 + 15*b231 + 16*b232 + 17*b233 + 18*b234 + 19*b235
       + 20*b236 + 21*b237 + 22*b238 + 23*b239 + 24*b240 + 25*b241 + 26*b242
       + 27*b243 + 28*b244 + 29*b245 + 30*b246 + 31*b247 + 32*b248 + 33*b249
       + 34*b250 + 35*b251 + 36*b252 + 37*b253 + 38*b254 + 39*b255 + 40*b256
       + b257 + 2*b258 + 3*b259 + 4*b260 + 5*b261 + 6*b262 + 7*b263 + 8*b264
       + 9*b265 + 10*b266 + 11*b267 + 12*b268 + 13*b269 + 14*b270 + 15*b271
       + 16*b272 + 17*b273 + 18*b274 + 19*b275 + 20*b276 + 21*b277 + 22*b278
       + 23*b279 + 24*b280 + 25*b281 + 26*b282 + 27*b283 + 28*b284 + 29*b285
       + 30*b286 + b287 + 2*b288 + 3*b289 + 4*b290 + 5*b291 + 6*b292 + 7*b293
       + 8*b294 + 9*b295 + 10*b296 + 11*b297 + 12*b298 + 13*b299 + 14*b300
       + 15*b301 + 16*b302 + 17*b303 + 18*b304 + 19*b305 + 20*b306 + 21*b307
       + 22*b308 + 23*b309 + 24*b310 + 25*b311 + 26*b312 + 27*b313 + 28*b314
       + b315 + 2*b316 + 3*b317 + 4*b318 + 5*b319 + 6*b320 + 7*b321 + 8*b322
       + 9*b323 + 10*b324 + 11*b325 + 12*b326 + 13*b327 + 14*b328 + 15*b329
       + 16*b330 + 17*b331 + 18*b332 + 19*b333 + 20*b334 + 21*b335 + 22*b336
       + 23*b337 + 24*b338 + 25*b339 + 26*b340 + 27*b341 + 28*b342 + b343
       + 2*b344 + 3*b345 + 4*b346 + 5*b347 + 6*b348 + 7*b349 + 8*b350 + 9*b351
       + 10*b352 + 11*b353 + 12*b354 + 13*b355 + 14*b356 + 15*b357 + 16*b358
       + 17*b359 + 18*b360 + 19*b361 + 20*b362 + 21*b363 + 22*b364 + b365
       + 2*b366 + 3*b367 + 4*b368 + 5*b369 + 6*b370 + 7*b371 + 8*b372 + 9*b373
       + 10*b374 + 11*b375 + 12*b376 + 13*b377 + 14*b378 + 15*b379 + 16*b380
       + 17*b381 + 18*b382 + 19*b383 + 20*b384 + 21*b385 + b386 + 2*b387
       + 3*b388 + 4*b389 + 5*b390 + 6*b391 + 7*b392 + 8*b393 + 9*b394 + 10*b395
       + b396 + 2*b397 + 3*b398 + 4*b399 + 5*b400 + 6*b401 + 7*b402 + 8*b403
       + 9*b404 + b405 + 2*b406 + 3*b407 + 4*b408 + 5*b409 + 6*b410 + 7*b411
       + 8*b412 + b413 + 2*b414 + 3*b415 + 4*b416 + 5*b417 + 6*b418 + 7*b419
       + 8*b420 + b421 + 2*b422 + 3*b423 + 4*b424 + 5*b425 + 6*b426 + 7*b427
       + 8*b428 + b657 + 2*b658 + b659 + 2*b660 + b661 + 2*b662 + b663 + 2*b664
       + b665 + 2*b666 + b667 + 2*b668 + b669 + 2*b670 + b671 + 2*b672 + b673
       + 2*b674 + b675 + 2*b676 + b677 + 2*b678 + b679 + 2*b680 <= -42;

e380: -(sqrt(i13*x97) + sqrt(i14*x98) + sqrt(i15*x99) + sqrt(i16*x100) + sqrt(
      i17*x101) + sqrt(i18*x102) + sqrt(i19*x103) + sqrt(i20*x104) + sqrt(i21*
      x105) + sqrt(i22*x106) + sqrt(i23*x107) + sqrt(i24*x108)) + b169 + 2*b170
       + 3*b171 + 4*b172 + 5*b173 + 6*b174 + 7*b175 + 8*b176 + 9*b177 + 10*b178
       + 11*b179 + 12*b180 + 13*b181 + 14*b182 + 15*b183 + 16*b184 + 17*b185
       + 18*b186 + 19*b187 + 20*b188 + 21*b189 + 22*b190 + 23*b191 + 24*b192
       + 25*b193 + 26*b194 + 27*b195 + 28*b196 + 29*b197 + 30*b198 + 31*b199
       + 32*b200 + 33*b201 + 34*b202 + 35*b203 + 36*b204 + 37*b205 + 38*b206
       + 39*b207 + 40*b208 + 41*b209 + 42*b210 + 43*b211 + 44*b212 + 45*b213
       + 46*b214 + 47*b215 + 48*b216 + b217 + 2*b218 + 3*b219 + 4*b220 + 5*b221
       + 6*b222 + 7*b223 + 8*b224 + 9*b225 + 10*b226 + 11*b227 + 12*b228
       + 13*b229 + 14*b230 + 15*b231 + 16*b232 + 17*b233 + 18*b234 + 19*b235
       + 20*b236 + 21*b237 + 22*b238 + 23*b239 + 24*b240 + 25*b241 + 26*b242
       + 27*b243 + 28*b244 + 29*b245 + 30*b246 + 31*b247 + 32*b248 + 33*b249
       + 34*b250 + 35*b251 + 36*b252 + 37*b253 + 38*b254 + 39*b255 + 40*b256
       + b257 + 2*b258 + 3*b259 + 4*b260 + 5*b261 + 6*b262 + 7*b263 + 8*b264
       + 9*b265 + 10*b266 + 11*b267 + 12*b268 + 13*b269 + 14*b270 + 15*b271
       + 16*b272 + 17*b273 + 18*b274 + 19*b275 + 20*b276 + 21*b277 + 22*b278
       + 23*b279 + 24*b280 + 25*b281 + 26*b282 + 27*b283 + 28*b284 + 29*b285
       + 30*b286 + b287 + 2*b288 + 3*b289 + 4*b290 + 5*b291 + 6*b292 + 7*b293
       + 8*b294 + 9*b295 + 10*b296 + 11*b297 + 12*b298 + 13*b299 + 14*b300
       + 15*b301 + 16*b302 + 17*b303 + 18*b304 + 19*b305 + 20*b306 + 21*b307
       + 22*b308 + 23*b309 + 24*b310 + 25*b311 + 26*b312 + 27*b313 + 28*b314
       + b315 + 2*b316 + 3*b317 + 4*b318 + 5*b319 + 6*b320 + 7*b321 + 8*b322
       + 9*b323 + 10*b324 + 11*b325 + 12*b326 + 13*b327 + 14*b328 + 15*b329
       + 16*b330 + 17*b331 + 18*b332 + 19*b333 + 20*b334 + 21*b335 + 22*b336
       + 23*b337 + 24*b338 + 25*b339 + 26*b340 + 27*b341 + 28*b342 + b343
       + 2*b344 + 3*b345 + 4*b346 + 5*b347 + 6*b348 + 7*b349 + 8*b350 + 9*b351
       + 10*b352 + 11*b353 + 12*b354 + 13*b355 + 14*b356 + 15*b357 + 16*b358
       + 17*b359 + 18*b360 + 19*b361 + 20*b362 + 21*b363 + 22*b364 + b365
       + 2*b366 + 3*b367 + 4*b368 + 5*b369 + 6*b370 + 7*b371 + 8*b372 + 9*b373
       + 10*b374 + 11*b375 + 12*b376 + 13*b377 + 14*b378 + 15*b379 + 16*b380
       + 17*b381 + 18*b382 + 19*b383 + 20*b384 + 21*b385 + b386 + 2*b387
       + 3*b388 + 4*b389 + 5*b390 + 6*b391 + 7*b392 + 8*b393 + 9*b394 + 10*b395
       + b396 + 2*b397 + 3*b398 + 4*b399 + 5*b400 + 6*b401 + 7*b402 + 8*b403
       + 9*b404 + b405 + 2*b406 + 3*b407 + 4*b408 + 5*b409 + 6*b410 + 7*b411
       + 8*b412 + b413 + 2*b414 + 3*b415 + 4*b416 + 5*b417 + 6*b418 + 7*b419
       + 8*b420 + b421 + 2*b422 + 3*b423 + 4*b424 + 5*b425 + 6*b426 + 7*b427
       + 8*b428 + b681 + 2*b682 + b683 + 2*b684 + b685 + 2*b686 + b687 + 2*b688
       + b689 + 2*b690 + b691 + 2*b692 + b693 + 2*b694 + b695 + 2*b696 + b697
       + 2*b698 + b699 + 2*b700 + b701 + 2*b702 + b703 + 2*b704 <= -33;

e381: -(sqrt(i13*x109) + sqrt(i14*x110) + sqrt(i15*x111) + sqrt(i16*x112) + 
      sqrt(i17*x113) + sqrt(i18*x114) + sqrt(i19*x115) + sqrt(i20*x116) + sqrt(
      i21*x117) + sqrt(i22*x118) + sqrt(i23*x119) + sqrt(i24*x120)) + b169
       + 2*b170 + 3*b171 + 4*b172 + 5*b173 + 6*b174 + 7*b175 + 8*b176 + 9*b177
       + 10*b178 + 11*b179 + 12*b180 + 13*b181 + 14*b182 + 15*b183 + 16*b184
       + 17*b185 + 18*b186 + 19*b187 + 20*b188 + 21*b189 + 22*b190 + 23*b191
       + 24*b192 + 25*b193 + 26*b194 + 27*b195 + 28*b196 + 29*b197 + 30*b198
       + 31*b199 + 32*b200 + 33*b201 + 34*b202 + 35*b203 + 36*b204 + 37*b205
       + 38*b206 + 39*b207 + 40*b208 + 41*b209 + 42*b210 + 43*b211 + 44*b212
       + 45*b213 + 46*b214 + 47*b215 + 48*b216 + b217 + 2*b218 + 3*b219
       + 4*b220 + 5*b221 + 6*b222 + 7*b223 + 8*b224 + 9*b225 + 10*b226
       + 11*b227 + 12*b228 + 13*b229 + 14*b230 + 15*b231 + 16*b232 + 17*b233
       + 18*b234 + 19*b235 + 20*b236 + 21*b237 + 22*b238 + 23*b239 + 24*b240
       + 25*b241 + 26*b242 + 27*b243 + 28*b244 + 29*b245 + 30*b246 + 31*b247
       + 32*b248 + 33*b249 + 34*b250 + 35*b251 + 36*b252 + 37*b253 + 38*b254
       + 39*b255 + 40*b256 + b257 + 2*b258 + 3*b259 + 4*b260 + 5*b261 + 6*b262
       + 7*b263 + 8*b264 + 9*b265 + 10*b266 + 11*b267 + 12*b268 + 13*b269
       + 14*b270 + 15*b271 + 16*b272 + 17*b273 + 18*b274 + 19*b275 + 20*b276
       + 21*b277 + 22*b278 + 23*b279 + 24*b280 + 25*b281 + 26*b282 + 27*b283
       + 28*b284 + 29*b285 + 30*b286 + b287 + 2*b288 + 3*b289 + 4*b290 + 5*b291
       + 6*b292 + 7*b293 + 8*b294 + 9*b295 + 10*b296 + 11*b297 + 12*b298
       + 13*b299 + 14*b300 + 15*b301 + 16*b302 + 17*b303 + 18*b304 + 19*b305
       + 20*b306 + 21*b307 + 22*b308 + 23*b309 + 24*b310 + 25*b311 + 26*b312
       + 27*b313 + 28*b314 + b315 + 2*b316 + 3*b317 + 4*b318 + 5*b319 + 6*b320
       + 7*b321 + 8*b322 + 9*b323 + 10*b324 + 11*b325 + 12*b326 + 13*b327
       + 14*b328 + 15*b329 + 16*b330 + 17*b331 + 18*b332 + 19*b333 + 20*b334
       + 21*b335 + 22*b336 + 23*b337 + 24*b338 + 25*b339 + 26*b340 + 27*b341
       + 28*b342 + b343 + 2*b344 + 3*b345 + 4*b346 + 5*b347 + 6*b348 + 7*b349
       + 8*b350 + 9*b351 + 10*b352 + 11*b353 + 12*b354 + 13*b355 + 14*b356
       + 15*b357 + 16*b358 + 17*b359 + 18*b360 + 19*b361 + 20*b362 + 21*b363
       + 22*b364 + b365 + 2*b366 + 3*b367 + 4*b368 + 5*b369 + 6*b370 + 7*b371
       + 8*b372 + 9*b373 + 10*b374 + 11*b375 + 12*b376 + 13*b377 + 14*b378
       + 15*b379 + 16*b380 + 17*b381 + 18*b382 + 19*b383 + 20*b384 + 21*b385
       + b386 + 2*b387 + 3*b388 + 4*b389 + 5*b390 + 6*b391 + 7*b392 + 8*b393
       + 9*b394 + 10*b395 + b396 + 2*b397 + 3*b398 + 4*b399 + 5*b400 + 6*b401
       + 7*b402 + 8*b403 + 9*b404 + b405 + 2*b406 + 3*b407 + 4*b408 + 5*b409
       + 6*b410 + 7*b411 + 8*b412 + b413 + 2*b414 + 3*b415 + 4*b416 + 5*b417
       + 6*b418 + 7*b419 + 8*b420 + b421 + 2*b422 + 3*b423 + 4*b424 + 5*b425
       + 6*b426 + 7*b427 + 8*b428 + b705 + 2*b706 + b707 + 2*b708 + b709
       + 2*b710 + b711 + 2*b712 + b713 + 2*b714 + b715 + 2*b716 + b717 + 2*b718
       + b719 + 2*b720 + b721 + 2*b722 + b723 + 2*b724 + b725 + 2*b726 + b727
       + 2*b728 <= -34;

e382: -(sqrt(i13*x121) + sqrt(i14*x122) + sqrt(i15*x123) + sqrt(i16*x124) + 
      sqrt(i17*x125) + sqrt(i18*x126) + sqrt(i19*x127) + sqrt(i20*x128) + sqrt(
      i21*x129) + sqrt(i22*x130) + sqrt(i23*x131) + sqrt(i24*x132)) + b169
       + 2*b170 + 3*b171 + 4*b172 + 5*b173 + 6*b174 + 7*b175 + 8*b176 + 9*b177
       + 10*b178 + 11*b179 + 12*b180 + 13*b181 + 14*b182 + 15*b183 + 16*b184
       + 17*b185 + 18*b186 + 19*b187 + 20*b188 + 21*b189 + 22*b190 + 23*b191
       + 24*b192 + 25*b193 + 26*b194 + 27*b195 + 28*b196 + 29*b197 + 30*b198
       + 31*b199 + 32*b200 + 33*b201 + 34*b202 + 35*b203 + 36*b204 + 37*b205
       + 38*b206 + 39*b207 + 40*b208 + 41*b209 + 42*b210 + 43*b211 + 44*b212
       + 45*b213 + 46*b214 + 47*b215 + 48*b216 + b217 + 2*b218 + 3*b219
       + 4*b220 + 5*b221 + 6*b222 + 7*b223 + 8*b224 + 9*b225 + 10*b226
       + 11*b227 + 12*b228 + 13*b229 + 14*b230 + 15*b231 + 16*b232 + 17*b233
       + 18*b234 + 19*b235 + 20*b236 + 21*b237 + 22*b238 + 23*b239 + 24*b240
       + 25*b241 + 26*b242 + 27*b243 + 28*b244 + 29*b245 + 30*b246 + 31*b247
       + 32*b248 + 33*b249 + 34*b250 + 35*b251 + 36*b252 + 37*b253 + 38*b254
       + 39*b255 + 40*b256 + b257 + 2*b258 + 3*b259 + 4*b260 + 5*b261 + 6*b262
       + 7*b263 + 8*b264 + 9*b265 + 10*b266 + 11*b267 + 12*b268 + 13*b269
       + 14*b270 + 15*b271 + 16*b272 + 17*b273 + 18*b274 + 19*b275 + 20*b276
       + 21*b277 + 22*b278 + 23*b279 + 24*b280 + 25*b281 + 26*b282 + 27*b283
       + 28*b284 + 29*b285 + 30*b286 + b287 + 2*b288 + 3*b289 + 4*b290 + 5*b291
       + 6*b292 + 7*b293 + 8*b294 + 9*b295 + 10*b296 + 11*b297 + 12*b298
       + 13*b299 + 14*b300 + 15*b301 + 16*b302 + 17*b303 + 18*b304 + 19*b305
       + 20*b306 + 21*b307 + 22*b308 + 23*b309 + 24*b310 + 25*b311 + 26*b312
       + 27*b313 + 28*b314 + b315 + 2*b316 + 3*b317 + 4*b318 + 5*b319 + 6*b320
       + 7*b321 + 8*b322 + 9*b323 + 10*b324 + 11*b325 + 12*b326 + 13*b327
       + 14*b328 + 15*b329 + 16*b330 + 17*b331 + 18*b332 + 19*b333 + 20*b334
       + 21*b335 + 22*b336 + 23*b337 + 24*b338 + 25*b339 + 26*b340 + 27*b341
       + 28*b342 + b343 + 2*b344 + 3*b345 + 4*b346 + 5*b347 + 6*b348 + 7*b349
       + 8*b350 + 9*b351 + 10*b352 + 11*b353 + 12*b354 + 13*b355 + 14*b356
       + 15*b357 + 16*b358 + 17*b359 + 18*b360 + 19*b361 + 20*b362 + 21*b363
       + 22*b364 + b365 + 2*b366 + 3*b367 + 4*b368 + 5*b369 + 6*b370 + 7*b371
       + 8*b372 + 9*b373 + 10*b374 + 11*b375 + 12*b376 + 13*b377 + 14*b378
       + 15*b379 + 16*b380 + 17*b381 + 18*b382 + 19*b383 + 20*b384 + 21*b385
       + b386 + 2*b387 + 3*b388 + 4*b389 + 5*b390 + 6*b391 + 7*b392 + 8*b393
       + 9*b394 + 10*b395 + b396 + 2*b397 + 3*b398 + 4*b399 + 5*b400 + 6*b401
       + 7*b402 + 8*b403 + 9*b404 + b405 + 2*b406 + 3*b407 + 4*b408 + 5*b409
       + 6*b410 + 7*b411 + 8*b412 + b413 + 2*b414 + 3*b415 + 4*b416 + 5*b417
       + 6*b418 + 7*b419 + 8*b420 + b421 + 2*b422 + 3*b423 + 4*b424 + 5*b425
       + 6*b426 + 7*b427 + 8*b428 + b729 + 2*b730 + b731 + 2*b732 + b733
       + 2*b734 + b735 + 2*b736 + b737 + 2*b738 + b739 + 2*b740 + b741 + 2*b742
       + b743 + 2*b744 + b745 + 2*b746 + b747 + 2*b748 + b749 + 2*b750 + b751
       + 2*b752 <= -20;

e383: -(sqrt(i13*x133) + sqrt(i14*x134) + sqrt(i15*x135) + sqrt(i16*x136) + 
      sqrt(i17*x137) + sqrt(i18*x138) + sqrt(i19*x139) + sqrt(i20*x140) + sqrt(
      i21*x141) + sqrt(i22*x142) + sqrt(i23*x143) + sqrt(i24*x144)) + b169
       + 2*b170 + 3*b171 + 4*b172 + 5*b173 + 6*b174 + 7*b175 + 8*b176 + 9*b177
       + 10*b178 + 11*b179 + 12*b180 + 13*b181 + 14*b182 + 15*b183 + 16*b184
       + 17*b185 + 18*b186 + 19*b187 + 20*b188 + 21*b189 + 22*b190 + 23*b191
       + 24*b192 + 25*b193 + 26*b194 + 27*b195 + 28*b196 + 29*b197 + 30*b198
       + 31*b199 + 32*b200 + 33*b201 + 34*b202 + 35*b203 + 36*b204 + 37*b205
       + 38*b206 + 39*b207 + 40*b208 + 41*b209 + 42*b210 + 43*b211 + 44*b212
       + 45*b213 + 46*b214 + 47*b215 + 48*b216 + b217 + 2*b218 + 3*b219
       + 4*b220 + 5*b221 + 6*b222 + 7*b223 + 8*b224 + 9*b225 + 10*b226
       + 11*b227 + 12*b228 + 13*b229 + 14*b230 + 15*b231 + 16*b232 + 17*b233
       + 18*b234 + 19*b235 + 20*b236 + 21*b237 + 22*b238 + 23*b239 + 24*b240
       + 25*b241 + 26*b242 + 27*b243 + 28*b244 + 29*b245 + 30*b246 + 31*b247
       + 32*b248 + 33*b249 + 34*b250 + 35*b251 + 36*b252 + 37*b253 + 38*b254
       + 39*b255 + 40*b256 + b257 + 2*b258 + 3*b259 + 4*b260 + 5*b261 + 6*b262
       + 7*b263 + 8*b264 + 9*b265 + 10*b266 + 11*b267 + 12*b268 + 13*b269
       + 14*b270 + 15*b271 + 16*b272 + 17*b273 + 18*b274 + 19*b275 + 20*b276
       + 21*b277 + 22*b278 + 23*b279 + 24*b280 + 25*b281 + 26*b282 + 27*b283
       + 28*b284 + 29*b285 + 30*b286 + b287 + 2*b288 + 3*b289 + 4*b290 + 5*b291
       + 6*b292 + 7*b293 + 8*b294 + 9*b295 + 10*b296 + 11*b297 + 12*b298
       + 13*b299 + 14*b300 + 15*b301 + 16*b302 + 17*b303 + 18*b304 + 19*b305
       + 20*b306 + 21*b307 + 22*b308 + 23*b309 + 24*b310 + 25*b311 + 26*b312
       + 27*b313 + 28*b314 + b315 + 2*b316 + 3*b317 + 4*b318 + 5*b319 + 6*b320
       + 7*b321 + 8*b322 + 9*b323 + 10*b324 + 11*b325 + 12*b326 + 13*b327
       + 14*b328 + 15*b329 + 16*b330 + 17*b331 + 18*b332 + 19*b333 + 20*b334
       + 21*b335 + 22*b336 + 23*b337 + 24*b338 + 25*b339 + 26*b340 + 27*b341
       + 28*b342 + b343 + 2*b344 + 3*b345 + 4*b346 + 5*b347 + 6*b348 + 7*b349
       + 8*b350 + 9*b351 + 10*b352 + 11*b353 + 12*b354 + 13*b355 + 14*b356
       + 15*b357 + 16*b358 + 17*b359 + 18*b360 + 19*b361 + 20*b362 + 21*b363
       + 22*b364 + b365 + 2*b366 + 3*b367 + 4*b368 + 5*b369 + 6*b370 + 7*b371
       + 8*b372 + 9*b373 + 10*b374 + 11*b375 + 12*b376 + 13*b377 + 14*b378
       + 15*b379 + 16*b380 + 17*b381 + 18*b382 + 19*b383 + 20*b384 + 21*b385
       + b386 + 2*b387 + 3*b388 + 4*b389 + 5*b390 + 6*b391 + 7*b392 + 8*b393
       + 9*b394 + 10*b395 + b396 + 2*b397 + 3*b398 + 4*b399 + 5*b400 + 6*b401
       + 7*b402 + 8*b403 + 9*b404 + b405 + 2*b406 + 3*b407 + 4*b408 + 5*b409
       + 6*b410 + 7*b411 + 8*b412 + b413 + 2*b414 + 3*b415 + 4*b416 + 5*b417
       + 6*b418 + 7*b419 + 8*b420 + b421 + 2*b422 + 3*b423 + 4*b424 + 5*b425
       + 6*b426 + 7*b427 + 8*b428 + b753 + 2*b754 + b755 + 2*b756 + b757
       + 2*b758 + b759 + 2*b760 + b761 + 2*b762 + b763 + 2*b764 + b765 + 2*b766
       + b767 + 2*b768 + b769 + 2*b770 + b771 + 2*b772 + b773 + 2*b774 + b775
       + 2*b776 <= -20;

e384: -(sqrt(i13*x145) + sqrt(i14*x146) + sqrt(i15*x147) + sqrt(i16*x148) + 
      sqrt(i17*x149) + sqrt(i18*x150) + sqrt(i19*x151) + sqrt(i20*x152) + sqrt(
      i21*x153) + sqrt(i22*x154) + sqrt(i23*x155) + sqrt(i24*x156)) + b169
       + 2*b170 + 3*b171 + 4*b172 + 5*b173 + 6*b174 + 7*b175 + 8*b176 + 9*b177
       + 10*b178 + 11*b179 + 12*b180 + 13*b181 + 14*b182 + 15*b183 + 16*b184
       + 17*b185 + 18*b186 + 19*b187 + 20*b188 + 21*b189 + 22*b190 + 23*b191
       + 24*b192 + 25*b193 + 26*b194 + 27*b195 + 28*b196 + 29*b197 + 30*b198
       + 31*b199 + 32*b200 + 33*b201 + 34*b202 + 35*b203 + 36*b204 + 37*b205
       + 38*b206 + 39*b207 + 40*b208 + 41*b209 + 42*b210 + 43*b211 + 44*b212
       + 45*b213 + 46*b214 + 47*b215 + 48*b216 + b217 + 2*b218 + 3*b219
       + 4*b220 + 5*b221 + 6*b222 + 7*b223 + 8*b224 + 9*b225 + 10*b226
       + 11*b227 + 12*b228 + 13*b229 + 14*b230 + 15*b231 + 16*b232 + 17*b233
       + 18*b234 + 19*b235 + 20*b236 + 21*b237 + 22*b238 + 23*b239 + 24*b240
       + 25*b241 + 26*b242 + 27*b243 + 28*b244 + 29*b245 + 30*b246 + 31*b247
       + 32*b248 + 33*b249 + 34*b250 + 35*b251 + 36*b252 + 37*b253 + 38*b254
       + 39*b255 + 40*b256 + b257 + 2*b258 + 3*b259 + 4*b260 + 5*b261 + 6*b262
       + 7*b263 + 8*b264 + 9*b265 + 10*b266 + 11*b267 + 12*b268 + 13*b269
       + 14*b270 + 15*b271 + 16*b272 + 17*b273 + 18*b274 + 19*b275 + 20*b276
       + 21*b277 + 22*b278 + 23*b279 + 24*b280 + 25*b281 + 26*b282 + 27*b283
       + 28*b284 + 29*b285 + 30*b286 + b287 + 2*b288 + 3*b289 + 4*b290 + 5*b291
       + 6*b292 + 7*b293 + 8*b294 + 9*b295 + 10*b296 + 11*b297 + 12*b298
       + 13*b299 + 14*b300 + 15*b301 + 16*b302 + 17*b303 + 18*b304 + 19*b305
       + 20*b306 + 21*b307 + 22*b308 + 23*b309 + 24*b310 + 25*b311 + 26*b312
       + 27*b313 + 28*b314 + b315 + 2*b316 + 3*b317 + 4*b318 + 5*b319 + 6*b320
       + 7*b321 + 8*b322 + 9*b323 + 10*b324 + 11*b325 + 12*b326 + 13*b327
       + 14*b328 + 15*b329 + 16*b330 + 17*b331 + 18*b332 + 19*b333 + 20*b334
       + 21*b335 + 22*b336 + 23*b337 + 24*b338 + 25*b339 + 26*b340 + 27*b341
       + 28*b342 + b343 + 2*b344 + 3*b345 + 4*b346 + 5*b347 + 6*b348 + 7*b349
       + 8*b350 + 9*b351 + 10*b352 + 11*b353 + 12*b354 + 13*b355 + 14*b356
       + 15*b357 + 16*b358 + 17*b359 + 18*b360 + 19*b361 + 20*b362 + 21*b363
       + 22*b364 + b365 + 2*b366 + 3*b367 + 4*b368 + 5*b369 + 6*b370 + 7*b371
       + 8*b372 + 9*b373 + 10*b374 + 11*b375 + 12*b376 + 13*b377 + 14*b378
       + 15*b379 + 16*b380 + 17*b381 + 18*b382 + 19*b383 + 20*b384 + 21*b385
       + b386 + 2*b387 + 3*b388 + 4*b389 + 5*b390 + 6*b391 + 7*b392 + 8*b393
       + 9*b394 + 10*b395 + b396 + 2*b397 + 3*b398 + 4*b399 + 5*b400 + 6*b401
       + 7*b402 + 8*b403 + 9*b404 + b405 + 2*b406 + 3*b407 + 4*b408 + 5*b409
       + 6*b410 + 7*b411 + 8*b412 + b413 + 2*b414 + 3*b415 + 4*b416 + 5*b417
       + 6*b418 + 7*b419 + 8*b420 + b421 + 2*b422 + 3*b423 + 4*b424 + 5*b425
       + 6*b426 + 7*b427 + 8*b428 + b777 + 2*b778 + b779 + 2*b780 + b781
       + 2*b782 + b783 + 2*b784 + b785 + 2*b786 + b787 + 2*b788 + b789 + 2*b790
       + b791 + 2*b792 + b793 + 2*b794 + b795 + 2*b796 + b797 + 2*b798 + b799
       + 2*b800 <= -21;

e385: -(sqrt(i13*x157) + sqrt(i14*x158) + sqrt(i15*x159) + sqrt(i16*x160) + 
      sqrt(i17*x161) + sqrt(i18*x162) + sqrt(i19*x163) + sqrt(i20*x164) + sqrt(
      i21*x165) + sqrt(i22*x166) + sqrt(i23*x167) + sqrt(i24*x168)) + b169
       + 2*b170 + 3*b171 + 4*b172 + 5*b173 + 6*b174 + 7*b175 + 8*b176 + 9*b177
       + 10*b178 + 11*b179 + 12*b180 + 13*b181 + 14*b182 + 15*b183 + 16*b184
       + 17*b185 + 18*b186 + 19*b187 + 20*b188 + 21*b189 + 22*b190 + 23*b191
       + 24*b192 + 25*b193 + 26*b194 + 27*b195 + 28*b196 + 29*b197 + 30*b198
       + 31*b199 + 32*b200 + 33*b201 + 34*b202 + 35*b203 + 36*b204 + 37*b205
       + 38*b206 + 39*b207 + 40*b208 + 41*b209 + 42*b210 + 43*b211 + 44*b212
       + 45*b213 + 46*b214 + 47*b215 + 48*b216 + b217 + 2*b218 + 3*b219
       + 4*b220 + 5*b221 + 6*b222 + 7*b223 + 8*b224 + 9*b225 + 10*b226
       + 11*b227 + 12*b228 + 13*b229 + 14*b230 + 15*b231 + 16*b232 + 17*b233
       + 18*b234 + 19*b235 + 20*b236 + 21*b237 + 22*b238 + 23*b239 + 24*b240
       + 25*b241 + 26*b242 + 27*b243 + 28*b244 + 29*b245 + 30*b246 + 31*b247
       + 32*b248 + 33*b249 + 34*b250 + 35*b251 + 36*b252 + 37*b253 + 38*b254
       + 39*b255 + 40*b256 + b257 + 2*b258 + 3*b259 + 4*b260 + 5*b261 + 6*b262
       + 7*b263 + 8*b264 + 9*b265 + 10*b266 + 11*b267 + 12*b268 + 13*b269
       + 14*b270 + 15*b271 + 16*b272 + 17*b273 + 18*b274 + 19*b275 + 20*b276
       + 21*b277 + 22*b278 + 23*b279 + 24*b280 + 25*b281 + 26*b282 + 27*b283
       + 28*b284 + 29*b285 + 30*b286 + b287 + 2*b288 + 3*b289 + 4*b290 + 5*b291
       + 6*b292 + 7*b293 + 8*b294 + 9*b295 + 10*b296 + 11*b297 + 12*b298
       + 13*b299 + 14*b300 + 15*b301 + 16*b302 + 17*b303 + 18*b304 + 19*b305
       + 20*b306 + 21*b307 + 22*b308 + 23*b309 + 24*b310 + 25*b311 + 26*b312
       + 27*b313 + 28*b314 + b315 + 2*b316 + 3*b317 + 4*b318 + 5*b319 + 6*b320
       + 7*b321 + 8*b322 + 9*b323 + 10*b324 + 11*b325 + 12*b326 + 13*b327
       + 14*b328 + 15*b329 + 16*b330 + 17*b331 + 18*b332 + 19*b333 + 20*b334
       + 21*b335 + 22*b336 + 23*b337 + 24*b338 + 25*b339 + 26*b340 + 27*b341
       + 28*b342 + b343 + 2*b344 + 3*b345 + 4*b346 + 5*b347 + 6*b348 + 7*b349
       + 8*b350 + 9*b351 + 10*b352 + 11*b353 + 12*b354 + 13*b355 + 14*b356
       + 15*b357 + 16*b358 + 17*b359 + 18*b360 + 19*b361 + 20*b362 + 21*b363
       + 22*b364 + b365 + 2*b366 + 3*b367 + 4*b368 + 5*b369 + 6*b370 + 7*b371
       + 8*b372 + 9*b373 + 10*b374 + 11*b375 + 12*b376 + 13*b377 + 14*b378
       + 15*b379 + 16*b380 + 17*b381 + 18*b382 + 19*b383 + 20*b384 + 21*b385
       + b386 + 2*b387 + 3*b388 + 4*b389 + 5*b390 + 6*b391 + 7*b392 + 8*b393
       + 9*b394 + 10*b395 + b396 + 2*b397 + 3*b398 + 4*b399 + 5*b400 + 6*b401
       + 7*b402 + 8*b403 + 9*b404 + b405 + 2*b406 + 3*b407 + 4*b408 + 5*b409
       + 6*b410 + 7*b411 + 8*b412 + b413 + 2*b414 + 3*b415 + 4*b416 + 5*b417
       + 6*b418 + 7*b419 + 8*b420 + b421 + 2*b422 + 3*b423 + 4*b424 + 5*b425
       + 6*b426 + 7*b427 + 8*b428 + b801 + b802 + b803 + b804 + b805 + b806
       + b807 + b808 + b809 + b810 + b811 + b812 <= -20;
