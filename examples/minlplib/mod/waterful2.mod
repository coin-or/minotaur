#  MINLP written by GAMS Convert at 01/12/18 13:52:01
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#        384      180       64      140        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#        630      182       56        0      392        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#       2479     2363      116        0
# 
#  Reformulation has removed 1 variable and 1 equation


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
var x29 >= 0;
var x30 >= 0;
var x31 >= 0;
var x32 >= 0;
var x33 >= 0;
var x34 >= 0;
var x35 >= 0;
var x36 >= 0;
var x37 >= 0;
var x38 >= 0;
var x39 >= 0;
var x40 >= 0;
var x41 >= 0;
var x42 >= 0;
var x43 >= 0;
var x44 >= 0;
var x45 >= 0;
var x46 >= 0;
var x47 >= 0;
var x48 >= 0;
var x49 >= 0;
var x50 >= 0;
var x51 >= 0;
var x52 >= 0;
var x53 >= 0;
var x54 >= 0;
var x55 >= 0;
var x56 >= 0;
var x57;
var x58;
var x59;
var x60;
var x61;
var x62;
var x63;
var x64;
var x65;
var x66;
var x67;
var x68;
var x69;
var x70;
var x71;
var x72;
var x73;
var x74;
var x75;
var x76;
var x77;
var x78;
var x79;
var x80;
var x81;
var x82;
var x83;
var x84;
var x85;
var x86;
var x87;
var x88;
var x89;
var x90;
var x91;
var x92;
var x93;
var x94;
var x95;
var x96;
var x97;
var x98;
var x99;
var x100;
var x101;
var x102;
var x103;
var x104;
var x105;
var x106;
var x107;
var x108;
var x109;
var x110;
var x111;
var x112;
var x113 := 11.5, >= 6.5;
var x114 := 8.25, >= 3.25;
var x115 := 21.58, >= 16.58;
var x116 := 19.92, >= 14.92;
var x117 := 17.925, >= 12.925;
var x118 := 17.26, >= 12.26;
var x119 := 13.76, >= 8.76;
var x120 := 21.08, >= 16.08;
var x121;
var x122;
var x123;
var x124;
var x125;
var x126;
var x127;
var x128;
var x129;
var x130;
var x131;
var x132;
var x133;
var x134;
var x135;
var x136;
var x137;
var x138;
var x139;
var x140;
var x141;
var x142;
var x143;
var x144;
var x145;
var x146;
var x147;
var x148;
var x149;
var x150;
var x151;
var x152;
var x153;
var x154;
var x155;
var x156;
var x157;
var x158;
var x159;
var x160;
var x161;
var x162;
var x163;
var x164;
var x165;
var x166;
var x167;
var x168;
var x169;
var x170;
var x171;
var x172;
var x173;
var x174;
var x175;
var x176;
var x177 := 0.961470588235294, >= 0, <= 2.5;
var x178 := 2.30752941176471, >= 0, <= 6;
var x179;
var x180;
var x181;
var b183 binary := 0.5, >= 0, <= 1;
var b184 binary := 0.5, >= 0, <= 1;
var b185 binary := 0.5, >= 0, <= 1;
var b186 binary := 0.5, >= 0, <= 1;
var b187 binary := 0.5, >= 0, <= 1;
var b188 binary := 0.5, >= 0, <= 1;
var b189 binary := 0.5, >= 0, <= 1;
var b190 binary := 0.5, >= 0, <= 1;
var b191 binary := 0.5, >= 0, <= 1;
var b192 binary := 0.5, >= 0, <= 1;
var b193 binary := 0.5, >= 0, <= 1;
var b194 binary := 0.5, >= 0, <= 1;
var b195 binary := 0.5, >= 0, <= 1;
var b196 binary := 0.5, >= 0, <= 1;
var b197 binary := 0.5, >= 0, <= 1;
var b198 binary := 0.5, >= 0, <= 1;
var b199 binary := 0.5, >= 0, <= 1;
var b200 binary := 0.5, >= 0, <= 1;
var b201 binary := 0.5, >= 0, <= 1;
var b202 binary := 0.5, >= 0, <= 1;
var b203 binary := 0.5, >= 0, <= 1;
var b204 binary := 0.5, >= 0, <= 1;
var b205 binary := 0.5, >= 0, <= 1;
var b206 binary := 0.5, >= 0, <= 1;
var b207 binary := 0.5, >= 0, <= 1;
var b208 binary := 0.5, >= 0, <= 1;
var b209 binary := 0.5, >= 0, <= 1;
var b210 binary := 0.5, >= 0, <= 1;
var b211 binary := 0.5, >= 0, <= 1;
var b212 binary := 0.5, >= 0, <= 1;
var b213 binary := 0.5, >= 0, <= 1;
var b214 binary := 0.5, >= 0, <= 1;
var b215 binary := 0.5, >= 0, <= 1;
var b216 binary := 0.5, >= 0, <= 1;
var b217 binary := 0.5, >= 0, <= 1;
var b218 binary := 0.5, >= 0, <= 1;
var b219 binary := 0.5, >= 0, <= 1;
var b220 binary := 0.5, >= 0, <= 1;
var b221 binary := 0.5, >= 0, <= 1;
var b222 binary := 0.5, >= 0, <= 1;
var b223 binary := 0.5, >= 0, <= 1;
var b224 binary := 0.5, >= 0, <= 1;
var b225 binary := 0.5, >= 0, <= 1;
var b226 binary := 0.5, >= 0, <= 1;
var b227 binary := 0.5, >= 0, <= 1;
var b228 binary := 0.5, >= 0, <= 1;
var b229 binary := 0.5, >= 0, <= 1;
var b230 binary := 0.5, >= 0, <= 1;
var b231 binary := 0.5, >= 0, <= 1;
var b232 binary := 0.5, >= 0, <= 1;
var b233 binary := 0.5, >= 0, <= 1;
var b234 binary := 0.5, >= 0, <= 1;
var b235 binary := 0.5, >= 0, <= 1;
var b236 binary := 0.5, >= 0, <= 1;
var b237 binary := 0.5, >= 0, <= 1;
var b238 binary := 0.5, >= 0, <= 1;
var s1s239 := 0.0714285714285714, >= 0;
var s1s240 := 0.0714285714285714, >= 0;
var s1s241 := 0.0714285714285714, >= 0;
var s1s242 := 0.0714285714285714, >= 0;
var s1s243 := 0.0714285714285714, >= 0;
var s1s244 := 0.0714285714285714, >= 0;
var s1s245 := 0.0714285714285714, >= 0;
var s1s246 := 0.0714285714285714, >= 0;
var s1s247 := 0.0714285714285714, >= 0;
var s1s248 := 0.0714285714285714, >= 0;
var s1s249 := 0.0714285714285714, >= 0;
var s1s250 := 0.0714285714285714, >= 0;
var s1s251 := 0.0714285714285714, >= 0;
var s1s252 := 0.0714285714285714, >= 0;
var s1s253 := 0.0714285714285714, >= 0;
var s1s254 := 0.0714285714285714, >= 0;
var s1s255 := 0.0714285714285714, >= 0;
var s1s256 := 0.0714285714285714, >= 0;
var s1s257 := 0.0714285714285714, >= 0;
var s1s258 := 0.0714285714285714, >= 0;
var s1s259 := 0.0714285714285714, >= 0;
var s1s260 := 0.0714285714285714, >= 0;
var s1s261 := 0.0714285714285714, >= 0;
var s1s262 := 0.0714285714285714, >= 0;
var s1s263 := 0.0714285714285714, >= 0;
var s1s264 := 0.0714285714285714, >= 0;
var s1s265 := 0.0714285714285714, >= 0;
var s1s266 := 0.0714285714285714, >= 0;
var s1s267 := 0.0714285714285714, >= 0;
var s1s268 := 0.0714285714285714, >= 0;
var s1s269 := 0.0714285714285714, >= 0;
var s1s270 := 0.0714285714285714, >= 0;
var s1s271 := 0.0714285714285714, >= 0;
var s1s272 := 0.0714285714285714, >= 0;
var s1s273 := 0.0714285714285714, >= 0;
var s1s274 := 0.0714285714285714, >= 0;
var s1s275 := 0.0714285714285714, >= 0;
var s1s276 := 0.0714285714285714, >= 0;
var s1s277 := 0.0714285714285714, >= 0;
var s1s278 := 0.0714285714285714, >= 0;
var s1s279 := 0.0714285714285714, >= 0;
var s1s280 := 0.0714285714285714, >= 0;
var s1s281 := 0.0714285714285714, >= 0;
var s1s282 := 0.0714285714285714, >= 0;
var s1s283 := 0.0714285714285714, >= 0;
var s1s284 := 0.0714285714285714, >= 0;
var s1s285 := 0.0714285714285714, >= 0;
var s1s286 := 0.0714285714285714, >= 0;
var s1s287 := 0.0714285714285714, >= 0;
var s1s288 >= 0;
var s1s289 >= 0;
var s1s290 >= 0;
var s1s291 >= 0;
var s1s292 >= 0;
var s1s293 >= 0;
var s1s294 >= 0;
var s1s295 := 0.0714285714285714, >= 0;
var s1s296 := 0.0714285714285714, >= 0;
var s1s297 := 0.0714285714285714, >= 0;
var s1s298 := 0.0714285714285714, >= 0;
var s1s299 := 0.0714285714285714, >= 0;
var s1s300 := 0.0714285714285714, >= 0;
var s1s301 := 0.0714285714285714, >= 0;
var s1s302 := 0.0714285714285714, >= 0;
var s1s303 := 0.0714285714285714, >= 0;
var s1s304 := 0.0714285714285714, >= 0;
var s1s305 := 0.0714285714285714, >= 0;
var s1s306 := 0.0714285714285714, >= 0;
var s1s307 := 0.0714285714285714, >= 0;
var s1s308 := 0.0714285714285714, >= 0;
var s1s309 := 0.0714285714285714, >= 0;
var s1s310 := 0.0714285714285714, >= 0;
var s1s311 := 0.0714285714285714, >= 0;
var s1s312 := 0.0714285714285714, >= 0;
var s1s313 := 0.0714285714285714, >= 0;
var s1s314 := 0.0714285714285714, >= 0;
var s1s315 := 0.0714285714285714, >= 0;
var s1s316 := 0.0714285714285714, >= 0;
var s1s317 := 0.0714285714285714, >= 0;
var s1s318 := 0.0714285714285714, >= 0;
var s1s319 := 0.0714285714285714, >= 0;
var s1s320 := 0.0714285714285714, >= 0;
var s1s321 := 0.0714285714285714, >= 0;
var s1s322 := 0.0714285714285714, >= 0;
var s1s323 := 0.0714285714285714, >= 0;
var s1s324 := 0.0714285714285714, >= 0;
var s1s325 := 0.0714285714285714, >= 0;
var s1s326 := 0.0714285714285714, >= 0;
var s1s327 := 0.0714285714285714, >= 0;
var s1s328 := 0.0714285714285714, >= 0;
var s1s329 := 0.0714285714285714, >= 0;
var s1s330 := 0.0714285714285714, >= 0;
var s1s331 := 0.0714285714285714, >= 0;
var s1s332 := 0.0714285714285714, >= 0;
var s1s333 := 0.0714285714285714, >= 0;
var s1s334 := 0.0714285714285714, >= 0;
var s1s335 := 0.0714285714285714, >= 0;
var s1s336 := 0.0714285714285714, >= 0;
var s1s337 >= 0;
var s1s338 >= 0;
var s1s339 >= 0;
var s1s340 >= 0;
var s1s341 >= 0;
var s1s342 >= 0;
var s1s343 >= 0;
var s1s344 >= 0;
var s1s345 >= 0;
var s1s346 >= 0;
var s1s347 >= 0;
var s1s348 >= 0;
var s1s349 >= 0;
var s1s350 >= 0;
var s1s351 := 0.0714285714285714, >= 0;
var s1s352 := 0.0714285714285714, >= 0;
var s1s353 := 0.0714285714285714, >= 0;
var s1s354 := 0.0714285714285714, >= 0;
var s1s355 := 0.0714285714285714, >= 0;
var s1s356 := 0.0714285714285714, >= 0;
var s1s357 := 0.0714285714285714, >= 0;
var s1s358 := 0.0714285714285714, >= 0;
var s1s359 := 0.0714285714285714, >= 0;
var s1s360 := 0.0714285714285714, >= 0;
var s1s361 := 0.0714285714285714, >= 0;
var s1s362 := 0.0714285714285714, >= 0;
var s1s363 := 0.0714285714285714, >= 0;
var s1s364 := 0.0714285714285714, >= 0;
var s1s365 := 0.0714285714285714, >= 0;
var s1s366 := 0.0714285714285714, >= 0;
var s1s367 := 0.0714285714285714, >= 0;
var s1s368 := 0.0714285714285714, >= 0;
var s1s369 := 0.0714285714285714, >= 0;
var s1s370 := 0.0714285714285714, >= 0;
var s1s371 := 0.0714285714285714, >= 0;
var s1s372 := 0.0714285714285714, >= 0;
var s1s373 := 0.0714285714285714, >= 0;
var s1s374 := 0.0714285714285714, >= 0;
var s1s375 := 0.0714285714285714, >= 0;
var s1s376 := 0.0714285714285714, >= 0;
var s1s377 := 0.0714285714285714, >= 0;
var s1s378 := 0.0714285714285714, >= 0;
var s1s379 := 0.0714285714285714, >= 0;
var s1s380 := 0.0714285714285714, >= 0;
var s1s381 := 0.0714285714285714, >= 0;
var s1s382 := 0.0714285714285714, >= 0;
var s1s383 := 0.0714285714285714, >= 0;
var s1s384 := 0.0714285714285714, >= 0;
var s1s385 := 0.0714285714285714, >= 0;
var s1s386 >= 0;
var s1s387 >= 0;
var s1s388 >= 0;
var s1s389 >= 0;
var s1s390 >= 0;
var s1s391 >= 0;
var s1s392 >= 0;
var s1s393 >= 0;
var s1s394 >= 0;
var s1s395 >= 0;
var s1s396 >= 0;
var s1s397 >= 0;
var s1s398 >= 0;
var s1s399 >= 0;
var s1s400 >= 0;
var s1s401 >= 0;
var s1s402 >= 0;
var s1s403 >= 0;
var s1s404 >= 0;
var s1s405 >= 0;
var s1s406 >= 0;
var s1s407 := 0.0714285714285714, >= 0;
var s1s408 := 0.0714285714285714, >= 0;
var s1s409 := 0.0714285714285714, >= 0;
var s1s410 := 0.0714285714285714, >= 0;
var s1s411 := 0.0714285714285714, >= 0;
var s1s412 := 0.0714285714285714, >= 0;
var s1s413 := 0.0714285714285714, >= 0;
var s1s414 := 0.0714285714285714, >= 0;
var s1s415 := 0.0714285714285714, >= 0;
var s1s416 := 0.0714285714285714, >= 0;
var s1s417 := 0.0714285714285714, >= 0;
var s1s418 := 0.0714285714285714, >= 0;
var s1s419 := 0.0714285714285714, >= 0;
var s1s420 := 0.0714285714285714, >= 0;
var s1s421 := 0.0714285714285714, >= 0;
var s1s422 := 0.0714285714285714, >= 0;
var s1s423 := 0.0714285714285714, >= 0;
var s1s424 := 0.0714285714285714, >= 0;
var s1s425 := 0.0714285714285714, >= 0;
var s1s426 := 0.0714285714285714, >= 0;
var s1s427 := 0.0714285714285714, >= 0;
var s1s428 := 0.0714285714285714, >= 0;
var s1s429 := 0.0714285714285714, >= 0;
var s1s430 := 0.0714285714285714, >= 0;
var s1s431 := 0.0714285714285714, >= 0;
var s1s432 := 0.0714285714285714, >= 0;
var s1s433 := 0.0714285714285714, >= 0;
var s1s434 := 0.0714285714285714, >= 0;
var s1s435 >= 0;
var s1s436 >= 0;
var s1s437 >= 0;
var s1s438 >= 0;
var s1s439 >= 0;
var s1s440 >= 0;
var s1s441 >= 0;
var s1s442 >= 0;
var s1s443 >= 0;
var s1s444 >= 0;
var s1s445 >= 0;
var s1s446 >= 0;
var s1s447 >= 0;
var s1s448 >= 0;
var s1s449 >= 0;
var s1s450 >= 0;
var s1s451 >= 0;
var s1s452 >= 0;
var s1s453 >= 0;
var s1s454 >= 0;
var s1s455 >= 0;
var s1s456 >= 0;
var s1s457 >= 0;
var s1s458 >= 0;
var s1s459 >= 0;
var s1s460 >= 0;
var s1s461 >= 0;
var s1s462 >= 0;
var s1s463 := 0.0714285714285714, >= 0;
var s1s464 := 0.0714285714285714, >= 0;
var s1s465 := 0.0714285714285714, >= 0;
var s1s466 := 0.0714285714285714, >= 0;
var s1s467 := 0.0714285714285714, >= 0;
var s1s468 := 0.0714285714285714, >= 0;
var s1s469 := 0.0714285714285714, >= 0;
var s1s470 := 0.0714285714285714, >= 0;
var s1s471 := 0.0714285714285714, >= 0;
var s1s472 := 0.0714285714285714, >= 0;
var s1s473 := 0.0714285714285714, >= 0;
var s1s474 := 0.0714285714285714, >= 0;
var s1s475 := 0.0714285714285714, >= 0;
var s1s476 := 0.0714285714285714, >= 0;
var s1s477 := 0.0714285714285714, >= 0;
var s1s478 := 0.0714285714285714, >= 0;
var s1s479 := 0.0714285714285714, >= 0;
var s1s480 := 0.0714285714285714, >= 0;
var s1s481 := 0.0714285714285714, >= 0;
var s1s482 := 0.0714285714285714, >= 0;
var s1s483 := 0.0714285714285714, >= 0;
var s1s484 >= 0;
var s1s485 >= 0;
var s1s486 >= 0;
var s1s487 >= 0;
var s1s488 >= 0;
var s1s489 >= 0;
var s1s490 >= 0;
var s1s491 >= 0;
var s1s492 >= 0;
var s1s493 >= 0;
var s1s494 >= 0;
var s1s495 >= 0;
var s1s496 >= 0;
var s1s497 >= 0;
var s1s498 >= 0;
var s1s499 >= 0;
var s1s500 >= 0;
var s1s501 >= 0;
var s1s502 >= 0;
var s1s503 >= 0;
var s1s504 >= 0;
var s1s505 >= 0;
var s1s506 >= 0;
var s1s507 >= 0;
var s1s508 >= 0;
var s1s509 >= 0;
var s1s510 >= 0;
var s1s511 >= 0;
var s1s512 >= 0;
var s1s513 >= 0;
var s1s514 >= 0;
var s1s515 >= 0;
var s1s516 >= 0;
var s1s517 >= 0;
var s1s518 >= 0;
var s1s519 := 0.0714285714285714, >= 0;
var s1s520 := 0.0714285714285714, >= 0;
var s1s521 := 0.0714285714285714, >= 0;
var s1s522 := 0.0714285714285714, >= 0;
var s1s523 := 0.0714285714285714, >= 0;
var s1s524 := 0.0714285714285714, >= 0;
var s1s525 := 0.0714285714285714, >= 0;
var s1s526 := 0.0714285714285714, >= 0;
var s1s527 := 0.0714285714285714, >= 0;
var s1s528 := 0.0714285714285714, >= 0;
var s1s529 := 0.0714285714285714, >= 0;
var s1s530 := 0.0714285714285714, >= 0;
var s1s531 := 0.0714285714285714, >= 0;
var s1s532 := 0.0714285714285714, >= 0;
var s1s533 >= 0;
var s1s534 >= 0;
var s1s535 >= 0;
var s1s536 >= 0;
var s1s537 >= 0;
var s1s538 >= 0;
var s1s539 >= 0;
var s1s540 >= 0;
var s1s541 >= 0;
var s1s542 >= 0;
var s1s543 >= 0;
var s1s544 >= 0;
var s1s545 >= 0;
var s1s546 >= 0;
var s1s547 >= 0;
var s1s548 >= 0;
var s1s549 >= 0;
var s1s550 >= 0;
var s1s551 >= 0;
var s1s552 >= 0;
var s1s553 >= 0;
var s1s554 >= 0;
var s1s555 >= 0;
var s1s556 >= 0;
var s1s557 >= 0;
var s1s558 >= 0;
var s1s559 >= 0;
var s1s560 >= 0;
var s1s561 >= 0;
var s1s562 >= 0;
var s1s563 >= 0;
var s1s564 >= 0;
var s1s565 >= 0;
var s1s566 >= 0;
var s1s567 >= 0;
var s1s568 >= 0;
var s1s569 >= 0;
var s1s570 >= 0;
var s1s571 >= 0;
var s1s572 >= 0;
var s1s573 >= 0;
var s1s574 >= 0;
var s1s575 := 0.0714285714285714, >= 0;
var s1s576 := 0.0714285714285714, >= 0;
var s1s577 := 0.0714285714285714, >= 0;
var s1s578 := 0.0714285714285714, >= 0;
var s1s579 := 0.0714285714285714, >= 0;
var s1s580 := 0.0714285714285714, >= 0;
var s1s581 := 0.0714285714285714, >= 0;
var s1s582 >= 0;
var s1s583 >= 0;
var s1s584 >= 0;
var s1s585 >= 0;
var s1s586 >= 0;
var s1s587 >= 0;
var s1s588 >= 0;
var s1s589 >= 0;
var s1s590 >= 0;
var s1s591 >= 0;
var s1s592 >= 0;
var s1s593 >= 0;
var s1s594 >= 0;
var s1s595 >= 0;
var s1s596 >= 0;
var s1s597 >= 0;
var s1s598 >= 0;
var s1s599 >= 0;
var s1s600 >= 0;
var s1s601 >= 0;
var s1s602 >= 0;
var s1s603 >= 0;
var s1s604 >= 0;
var s1s605 >= 0;
var s1s606 >= 0;
var s1s607 >= 0;
var s1s608 >= 0;
var s1s609 >= 0;
var s1s610 >= 0;
var s1s611 >= 0;
var s1s612 >= 0;
var s1s613 >= 0;
var s1s614 >= 0;
var s1s615 >= 0;
var s1s616 >= 0;
var s1s617 >= 0;
var s1s618 >= 0;
var s1s619 >= 0;
var s1s620 >= 0;
var s1s621 >= 0;
var s1s622 >= 0;
var s1s623 >= 0;
var s1s624 >= 0;
var s1s625 >= 0;
var s1s626 >= 0;
var s1s627 >= 0;
var s1s628 >= 0;
var s1s629 >= 0;
var s1s630 >= 0;

suffix sosno integer IN;
suffix ref integer IN;
let s1s239.sosno := 1;
let s1s239.ref   := 1;
let s1s240.sosno := 1;
let s1s240.ref   := 2;
let s1s241.sosno := 1;
let s1s241.ref   := 3;
let s1s242.sosno := 1;
let s1s242.ref   := 4;
let s1s243.sosno := 1;
let s1s243.ref   := 5;
let s1s244.sosno := 1;
let s1s244.ref   := 6;
let s1s245.sosno := 1;
let s1s245.ref   := 7;
let s1s246.sosno := 2;
let s1s246.ref   := 1;
let s1s247.sosno := 2;
let s1s247.ref   := 2;
let s1s248.sosno := 2;
let s1s248.ref   := 3;
let s1s249.sosno := 2;
let s1s249.ref   := 4;
let s1s250.sosno := 2;
let s1s250.ref   := 5;
let s1s251.sosno := 2;
let s1s251.ref   := 6;
let s1s252.sosno := 2;
let s1s252.ref   := 7;
let s1s253.sosno := 3;
let s1s253.ref   := 1;
let s1s254.sosno := 3;
let s1s254.ref   := 2;
let s1s255.sosno := 3;
let s1s255.ref   := 3;
let s1s256.sosno := 3;
let s1s256.ref   := 4;
let s1s257.sosno := 3;
let s1s257.ref   := 5;
let s1s258.sosno := 3;
let s1s258.ref   := 6;
let s1s259.sosno := 3;
let s1s259.ref   := 7;
let s1s260.sosno := 4;
let s1s260.ref   := 1;
let s1s261.sosno := 4;
let s1s261.ref   := 2;
let s1s262.sosno := 4;
let s1s262.ref   := 3;
let s1s263.sosno := 4;
let s1s263.ref   := 4;
let s1s264.sosno := 4;
let s1s264.ref   := 5;
let s1s265.sosno := 4;
let s1s265.ref   := 6;
let s1s266.sosno := 4;
let s1s266.ref   := 7;
let s1s267.sosno := 5;
let s1s267.ref   := 1;
let s1s268.sosno := 5;
let s1s268.ref   := 2;
let s1s269.sosno := 5;
let s1s269.ref   := 3;
let s1s270.sosno := 5;
let s1s270.ref   := 4;
let s1s271.sosno := 5;
let s1s271.ref   := 5;
let s1s272.sosno := 5;
let s1s272.ref   := 6;
let s1s273.sosno := 5;
let s1s273.ref   := 7;
let s1s274.sosno := 6;
let s1s274.ref   := 1;
let s1s275.sosno := 6;
let s1s275.ref   := 2;
let s1s276.sosno := 6;
let s1s276.ref   := 3;
let s1s277.sosno := 6;
let s1s277.ref   := 4;
let s1s278.sosno := 6;
let s1s278.ref   := 5;
let s1s279.sosno := 6;
let s1s279.ref   := 6;
let s1s280.sosno := 6;
let s1s280.ref   := 7;
let s1s281.sosno := 7;
let s1s281.ref   := 1;
let s1s282.sosno := 7;
let s1s282.ref   := 2;
let s1s283.sosno := 7;
let s1s283.ref   := 3;
let s1s284.sosno := 7;
let s1s284.ref   := 4;
let s1s285.sosno := 7;
let s1s285.ref   := 5;
let s1s286.sosno := 7;
let s1s286.ref   := 6;
let s1s287.sosno := 7;
let s1s287.ref   := 7;
let s1s288.sosno := 8;
let s1s288.ref   := 1;
let s1s289.sosno := 8;
let s1s289.ref   := 2;
let s1s290.sosno := 8;
let s1s290.ref   := 3;
let s1s291.sosno := 8;
let s1s291.ref   := 4;
let s1s292.sosno := 8;
let s1s292.ref   := 5;
let s1s293.sosno := 8;
let s1s293.ref   := 6;
let s1s294.sosno := 8;
let s1s294.ref   := 7;
let s1s295.sosno := 9;
let s1s295.ref   := 1;
let s1s296.sosno := 9;
let s1s296.ref   := 2;
let s1s297.sosno := 9;
let s1s297.ref   := 3;
let s1s298.sosno := 9;
let s1s298.ref   := 4;
let s1s299.sosno := 9;
let s1s299.ref   := 5;
let s1s300.sosno := 9;
let s1s300.ref   := 6;
let s1s301.sosno := 9;
let s1s301.ref   := 7;
let s1s302.sosno := 10;
let s1s302.ref   := 1;
let s1s303.sosno := 10;
let s1s303.ref   := 2;
let s1s304.sosno := 10;
let s1s304.ref   := 3;
let s1s305.sosno := 10;
let s1s305.ref   := 4;
let s1s306.sosno := 10;
let s1s306.ref   := 5;
let s1s307.sosno := 10;
let s1s307.ref   := 6;
let s1s308.sosno := 10;
let s1s308.ref   := 7;
let s1s309.sosno := 11;
let s1s309.ref   := 1;
let s1s310.sosno := 11;
let s1s310.ref   := 2;
let s1s311.sosno := 11;
let s1s311.ref   := 3;
let s1s312.sosno := 11;
let s1s312.ref   := 4;
let s1s313.sosno := 11;
let s1s313.ref   := 5;
let s1s314.sosno := 11;
let s1s314.ref   := 6;
let s1s315.sosno := 11;
let s1s315.ref   := 7;
let s1s316.sosno := 12;
let s1s316.ref   := 1;
let s1s317.sosno := 12;
let s1s317.ref   := 2;
let s1s318.sosno := 12;
let s1s318.ref   := 3;
let s1s319.sosno := 12;
let s1s319.ref   := 4;
let s1s320.sosno := 12;
let s1s320.ref   := 5;
let s1s321.sosno := 12;
let s1s321.ref   := 6;
let s1s322.sosno := 12;
let s1s322.ref   := 7;
let s1s323.sosno := 13;
let s1s323.ref   := 1;
let s1s324.sosno := 13;
let s1s324.ref   := 2;
let s1s325.sosno := 13;
let s1s325.ref   := 3;
let s1s326.sosno := 13;
let s1s326.ref   := 4;
let s1s327.sosno := 13;
let s1s327.ref   := 5;
let s1s328.sosno := 13;
let s1s328.ref   := 6;
let s1s329.sosno := 13;
let s1s329.ref   := 7;
let s1s330.sosno := 14;
let s1s330.ref   := 1;
let s1s331.sosno := 14;
let s1s331.ref   := 2;
let s1s332.sosno := 14;
let s1s332.ref   := 3;
let s1s333.sosno := 14;
let s1s333.ref   := 4;
let s1s334.sosno := 14;
let s1s334.ref   := 5;
let s1s335.sosno := 14;
let s1s335.ref   := 6;
let s1s336.sosno := 14;
let s1s336.ref   := 7;
let s1s337.sosno := 15;
let s1s337.ref   := 1;
let s1s338.sosno := 15;
let s1s338.ref   := 2;
let s1s339.sosno := 15;
let s1s339.ref   := 3;
let s1s340.sosno := 15;
let s1s340.ref   := 4;
let s1s341.sosno := 15;
let s1s341.ref   := 5;
let s1s342.sosno := 15;
let s1s342.ref   := 6;
let s1s343.sosno := 15;
let s1s343.ref   := 7;
let s1s344.sosno := 16;
let s1s344.ref   := 1;
let s1s345.sosno := 16;
let s1s345.ref   := 2;
let s1s346.sosno := 16;
let s1s346.ref   := 3;
let s1s347.sosno := 16;
let s1s347.ref   := 4;
let s1s348.sosno := 16;
let s1s348.ref   := 5;
let s1s349.sosno := 16;
let s1s349.ref   := 6;
let s1s350.sosno := 16;
let s1s350.ref   := 7;
let s1s351.sosno := 17;
let s1s351.ref   := 1;
let s1s352.sosno := 17;
let s1s352.ref   := 2;
let s1s353.sosno := 17;
let s1s353.ref   := 3;
let s1s354.sosno := 17;
let s1s354.ref   := 4;
let s1s355.sosno := 17;
let s1s355.ref   := 5;
let s1s356.sosno := 17;
let s1s356.ref   := 6;
let s1s357.sosno := 17;
let s1s357.ref   := 7;
let s1s358.sosno := 18;
let s1s358.ref   := 1;
let s1s359.sosno := 18;
let s1s359.ref   := 2;
let s1s360.sosno := 18;
let s1s360.ref   := 3;
let s1s361.sosno := 18;
let s1s361.ref   := 4;
let s1s362.sosno := 18;
let s1s362.ref   := 5;
let s1s363.sosno := 18;
let s1s363.ref   := 6;
let s1s364.sosno := 18;
let s1s364.ref   := 7;
let s1s365.sosno := 19;
let s1s365.ref   := 1;
let s1s366.sosno := 19;
let s1s366.ref   := 2;
let s1s367.sosno := 19;
let s1s367.ref   := 3;
let s1s368.sosno := 19;
let s1s368.ref   := 4;
let s1s369.sosno := 19;
let s1s369.ref   := 5;
let s1s370.sosno := 19;
let s1s370.ref   := 6;
let s1s371.sosno := 19;
let s1s371.ref   := 7;
let s1s372.sosno := 20;
let s1s372.ref   := 1;
let s1s373.sosno := 20;
let s1s373.ref   := 2;
let s1s374.sosno := 20;
let s1s374.ref   := 3;
let s1s375.sosno := 20;
let s1s375.ref   := 4;
let s1s376.sosno := 20;
let s1s376.ref   := 5;
let s1s377.sosno := 20;
let s1s377.ref   := 6;
let s1s378.sosno := 20;
let s1s378.ref   := 7;
let s1s379.sosno := 21;
let s1s379.ref   := 1;
let s1s380.sosno := 21;
let s1s380.ref   := 2;
let s1s381.sosno := 21;
let s1s381.ref   := 3;
let s1s382.sosno := 21;
let s1s382.ref   := 4;
let s1s383.sosno := 21;
let s1s383.ref   := 5;
let s1s384.sosno := 21;
let s1s384.ref   := 6;
let s1s385.sosno := 21;
let s1s385.ref   := 7;
let s1s386.sosno := 22;
let s1s386.ref   := 1;
let s1s387.sosno := 22;
let s1s387.ref   := 2;
let s1s388.sosno := 22;
let s1s388.ref   := 3;
let s1s389.sosno := 22;
let s1s389.ref   := 4;
let s1s390.sosno := 22;
let s1s390.ref   := 5;
let s1s391.sosno := 22;
let s1s391.ref   := 6;
let s1s392.sosno := 22;
let s1s392.ref   := 7;
let s1s393.sosno := 23;
let s1s393.ref   := 1;
let s1s394.sosno := 23;
let s1s394.ref   := 2;
let s1s395.sosno := 23;
let s1s395.ref   := 3;
let s1s396.sosno := 23;
let s1s396.ref   := 4;
let s1s397.sosno := 23;
let s1s397.ref   := 5;
let s1s398.sosno := 23;
let s1s398.ref   := 6;
let s1s399.sosno := 23;
let s1s399.ref   := 7;
let s1s400.sosno := 24;
let s1s400.ref   := 1;
let s1s401.sosno := 24;
let s1s401.ref   := 2;
let s1s402.sosno := 24;
let s1s402.ref   := 3;
let s1s403.sosno := 24;
let s1s403.ref   := 4;
let s1s404.sosno := 24;
let s1s404.ref   := 5;
let s1s405.sosno := 24;
let s1s405.ref   := 6;
let s1s406.sosno := 24;
let s1s406.ref   := 7;
let s1s407.sosno := 25;
let s1s407.ref   := 1;
let s1s408.sosno := 25;
let s1s408.ref   := 2;
let s1s409.sosno := 25;
let s1s409.ref   := 3;
let s1s410.sosno := 25;
let s1s410.ref   := 4;
let s1s411.sosno := 25;
let s1s411.ref   := 5;
let s1s412.sosno := 25;
let s1s412.ref   := 6;
let s1s413.sosno := 25;
let s1s413.ref   := 7;
let s1s414.sosno := 26;
let s1s414.ref   := 1;
let s1s415.sosno := 26;
let s1s415.ref   := 2;
let s1s416.sosno := 26;
let s1s416.ref   := 3;
let s1s417.sosno := 26;
let s1s417.ref   := 4;
let s1s418.sosno := 26;
let s1s418.ref   := 5;
let s1s419.sosno := 26;
let s1s419.ref   := 6;
let s1s420.sosno := 26;
let s1s420.ref   := 7;
let s1s421.sosno := 27;
let s1s421.ref   := 1;
let s1s422.sosno := 27;
let s1s422.ref   := 2;
let s1s423.sosno := 27;
let s1s423.ref   := 3;
let s1s424.sosno := 27;
let s1s424.ref   := 4;
let s1s425.sosno := 27;
let s1s425.ref   := 5;
let s1s426.sosno := 27;
let s1s426.ref   := 6;
let s1s427.sosno := 27;
let s1s427.ref   := 7;
let s1s428.sosno := 28;
let s1s428.ref   := 1;
let s1s429.sosno := 28;
let s1s429.ref   := 2;
let s1s430.sosno := 28;
let s1s430.ref   := 3;
let s1s431.sosno := 28;
let s1s431.ref   := 4;
let s1s432.sosno := 28;
let s1s432.ref   := 5;
let s1s433.sosno := 28;
let s1s433.ref   := 6;
let s1s434.sosno := 28;
let s1s434.ref   := 7;
let s1s435.sosno := 29;
let s1s435.ref   := 1;
let s1s436.sosno := 29;
let s1s436.ref   := 2;
let s1s437.sosno := 29;
let s1s437.ref   := 3;
let s1s438.sosno := 29;
let s1s438.ref   := 4;
let s1s439.sosno := 29;
let s1s439.ref   := 5;
let s1s440.sosno := 29;
let s1s440.ref   := 6;
let s1s441.sosno := 29;
let s1s441.ref   := 7;
let s1s442.sosno := 30;
let s1s442.ref   := 1;
let s1s443.sosno := 30;
let s1s443.ref   := 2;
let s1s444.sosno := 30;
let s1s444.ref   := 3;
let s1s445.sosno := 30;
let s1s445.ref   := 4;
let s1s446.sosno := 30;
let s1s446.ref   := 5;
let s1s447.sosno := 30;
let s1s447.ref   := 6;
let s1s448.sosno := 30;
let s1s448.ref   := 7;
let s1s449.sosno := 31;
let s1s449.ref   := 1;
let s1s450.sosno := 31;
let s1s450.ref   := 2;
let s1s451.sosno := 31;
let s1s451.ref   := 3;
let s1s452.sosno := 31;
let s1s452.ref   := 4;
let s1s453.sosno := 31;
let s1s453.ref   := 5;
let s1s454.sosno := 31;
let s1s454.ref   := 6;
let s1s455.sosno := 31;
let s1s455.ref   := 7;
let s1s456.sosno := 32;
let s1s456.ref   := 1;
let s1s457.sosno := 32;
let s1s457.ref   := 2;
let s1s458.sosno := 32;
let s1s458.ref   := 3;
let s1s459.sosno := 32;
let s1s459.ref   := 4;
let s1s460.sosno := 32;
let s1s460.ref   := 5;
let s1s461.sosno := 32;
let s1s461.ref   := 6;
let s1s462.sosno := 32;
let s1s462.ref   := 7;
let s1s463.sosno := 33;
let s1s463.ref   := 1;
let s1s464.sosno := 33;
let s1s464.ref   := 2;
let s1s465.sosno := 33;
let s1s465.ref   := 3;
let s1s466.sosno := 33;
let s1s466.ref   := 4;
let s1s467.sosno := 33;
let s1s467.ref   := 5;
let s1s468.sosno := 33;
let s1s468.ref   := 6;
let s1s469.sosno := 33;
let s1s469.ref   := 7;
let s1s470.sosno := 34;
let s1s470.ref   := 1;
let s1s471.sosno := 34;
let s1s471.ref   := 2;
let s1s472.sosno := 34;
let s1s472.ref   := 3;
let s1s473.sosno := 34;
let s1s473.ref   := 4;
let s1s474.sosno := 34;
let s1s474.ref   := 5;
let s1s475.sosno := 34;
let s1s475.ref   := 6;
let s1s476.sosno := 34;
let s1s476.ref   := 7;
let s1s477.sosno := 35;
let s1s477.ref   := 1;
let s1s478.sosno := 35;
let s1s478.ref   := 2;
let s1s479.sosno := 35;
let s1s479.ref   := 3;
let s1s480.sosno := 35;
let s1s480.ref   := 4;
let s1s481.sosno := 35;
let s1s481.ref   := 5;
let s1s482.sosno := 35;
let s1s482.ref   := 6;
let s1s483.sosno := 35;
let s1s483.ref   := 7;
let s1s484.sosno := 36;
let s1s484.ref   := 1;
let s1s485.sosno := 36;
let s1s485.ref   := 2;
let s1s486.sosno := 36;
let s1s486.ref   := 3;
let s1s487.sosno := 36;
let s1s487.ref   := 4;
let s1s488.sosno := 36;
let s1s488.ref   := 5;
let s1s489.sosno := 36;
let s1s489.ref   := 6;
let s1s490.sosno := 36;
let s1s490.ref   := 7;
let s1s491.sosno := 37;
let s1s491.ref   := 1;
let s1s492.sosno := 37;
let s1s492.ref   := 2;
let s1s493.sosno := 37;
let s1s493.ref   := 3;
let s1s494.sosno := 37;
let s1s494.ref   := 4;
let s1s495.sosno := 37;
let s1s495.ref   := 5;
let s1s496.sosno := 37;
let s1s496.ref   := 6;
let s1s497.sosno := 37;
let s1s497.ref   := 7;
let s1s498.sosno := 38;
let s1s498.ref   := 1;
let s1s499.sosno := 38;
let s1s499.ref   := 2;
let s1s500.sosno := 38;
let s1s500.ref   := 3;
let s1s501.sosno := 38;
let s1s501.ref   := 4;
let s1s502.sosno := 38;
let s1s502.ref   := 5;
let s1s503.sosno := 38;
let s1s503.ref   := 6;
let s1s504.sosno := 38;
let s1s504.ref   := 7;
let s1s505.sosno := 39;
let s1s505.ref   := 1;
let s1s506.sosno := 39;
let s1s506.ref   := 2;
let s1s507.sosno := 39;
let s1s507.ref   := 3;
let s1s508.sosno := 39;
let s1s508.ref   := 4;
let s1s509.sosno := 39;
let s1s509.ref   := 5;
let s1s510.sosno := 39;
let s1s510.ref   := 6;
let s1s511.sosno := 39;
let s1s511.ref   := 7;
let s1s512.sosno := 40;
let s1s512.ref   := 1;
let s1s513.sosno := 40;
let s1s513.ref   := 2;
let s1s514.sosno := 40;
let s1s514.ref   := 3;
let s1s515.sosno := 40;
let s1s515.ref   := 4;
let s1s516.sosno := 40;
let s1s516.ref   := 5;
let s1s517.sosno := 40;
let s1s517.ref   := 6;
let s1s518.sosno := 40;
let s1s518.ref   := 7;
let s1s519.sosno := 41;
let s1s519.ref   := 1;
let s1s520.sosno := 41;
let s1s520.ref   := 2;
let s1s521.sosno := 41;
let s1s521.ref   := 3;
let s1s522.sosno := 41;
let s1s522.ref   := 4;
let s1s523.sosno := 41;
let s1s523.ref   := 5;
let s1s524.sosno := 41;
let s1s524.ref   := 6;
let s1s525.sosno := 41;
let s1s525.ref   := 7;
let s1s526.sosno := 42;
let s1s526.ref   := 1;
let s1s527.sosno := 42;
let s1s527.ref   := 2;
let s1s528.sosno := 42;
let s1s528.ref   := 3;
let s1s529.sosno := 42;
let s1s529.ref   := 4;
let s1s530.sosno := 42;
let s1s530.ref   := 5;
let s1s531.sosno := 42;
let s1s531.ref   := 6;
let s1s532.sosno := 42;
let s1s532.ref   := 7;
let s1s533.sosno := 43;
let s1s533.ref   := 1;
let s1s534.sosno := 43;
let s1s534.ref   := 2;
let s1s535.sosno := 43;
let s1s535.ref   := 3;
let s1s536.sosno := 43;
let s1s536.ref   := 4;
let s1s537.sosno := 43;
let s1s537.ref   := 5;
let s1s538.sosno := 43;
let s1s538.ref   := 6;
let s1s539.sosno := 43;
let s1s539.ref   := 7;
let s1s540.sosno := 44;
let s1s540.ref   := 1;
let s1s541.sosno := 44;
let s1s541.ref   := 2;
let s1s542.sosno := 44;
let s1s542.ref   := 3;
let s1s543.sosno := 44;
let s1s543.ref   := 4;
let s1s544.sosno := 44;
let s1s544.ref   := 5;
let s1s545.sosno := 44;
let s1s545.ref   := 6;
let s1s546.sosno := 44;
let s1s546.ref   := 7;
let s1s547.sosno := 45;
let s1s547.ref   := 1;
let s1s548.sosno := 45;
let s1s548.ref   := 2;
let s1s549.sosno := 45;
let s1s549.ref   := 3;
let s1s550.sosno := 45;
let s1s550.ref   := 4;
let s1s551.sosno := 45;
let s1s551.ref   := 5;
let s1s552.sosno := 45;
let s1s552.ref   := 6;
let s1s553.sosno := 45;
let s1s553.ref   := 7;
let s1s554.sosno := 46;
let s1s554.ref   := 1;
let s1s555.sosno := 46;
let s1s555.ref   := 2;
let s1s556.sosno := 46;
let s1s556.ref   := 3;
let s1s557.sosno := 46;
let s1s557.ref   := 4;
let s1s558.sosno := 46;
let s1s558.ref   := 5;
let s1s559.sosno := 46;
let s1s559.ref   := 6;
let s1s560.sosno := 46;
let s1s560.ref   := 7;
let s1s561.sosno := 47;
let s1s561.ref   := 1;
let s1s562.sosno := 47;
let s1s562.ref   := 2;
let s1s563.sosno := 47;
let s1s563.ref   := 3;
let s1s564.sosno := 47;
let s1s564.ref   := 4;
let s1s565.sosno := 47;
let s1s565.ref   := 5;
let s1s566.sosno := 47;
let s1s566.ref   := 6;
let s1s567.sosno := 47;
let s1s567.ref   := 7;
let s1s568.sosno := 48;
let s1s568.ref   := 1;
let s1s569.sosno := 48;
let s1s569.ref   := 2;
let s1s570.sosno := 48;
let s1s570.ref   := 3;
let s1s571.sosno := 48;
let s1s571.ref   := 4;
let s1s572.sosno := 48;
let s1s572.ref   := 5;
let s1s573.sosno := 48;
let s1s573.ref   := 6;
let s1s574.sosno := 48;
let s1s574.ref   := 7;
let s1s575.sosno := 49;
let s1s575.ref   := 1;
let s1s576.sosno := 49;
let s1s576.ref   := 2;
let s1s577.sosno := 49;
let s1s577.ref   := 3;
let s1s578.sosno := 49;
let s1s578.ref   := 4;
let s1s579.sosno := 49;
let s1s579.ref   := 5;
let s1s580.sosno := 49;
let s1s580.ref   := 6;
let s1s581.sosno := 49;
let s1s581.ref   := 7;
let s1s582.sosno := 50;
let s1s582.ref   := 1;
let s1s583.sosno := 50;
let s1s583.ref   := 2;
let s1s584.sosno := 50;
let s1s584.ref   := 3;
let s1s585.sosno := 50;
let s1s585.ref   := 4;
let s1s586.sosno := 50;
let s1s586.ref   := 5;
let s1s587.sosno := 50;
let s1s587.ref   := 6;
let s1s588.sosno := 50;
let s1s588.ref   := 7;
let s1s589.sosno := 51;
let s1s589.ref   := 1;
let s1s590.sosno := 51;
let s1s590.ref   := 2;
let s1s591.sosno := 51;
let s1s591.ref   := 3;
let s1s592.sosno := 51;
let s1s592.ref   := 4;
let s1s593.sosno := 51;
let s1s593.ref   := 5;
let s1s594.sosno := 51;
let s1s594.ref   := 6;
let s1s595.sosno := 51;
let s1s595.ref   := 7;
let s1s596.sosno := 52;
let s1s596.ref   := 1;
let s1s597.sosno := 52;
let s1s597.ref   := 2;
let s1s598.sosno := 52;
let s1s598.ref   := 3;
let s1s599.sosno := 52;
let s1s599.ref   := 4;
let s1s600.sosno := 52;
let s1s600.ref   := 5;
let s1s601.sosno := 52;
let s1s601.ref   := 6;
let s1s602.sosno := 52;
let s1s602.ref   := 7;
let s1s603.sosno := 53;
let s1s603.ref   := 1;
let s1s604.sosno := 53;
let s1s604.ref   := 2;
let s1s605.sosno := 53;
let s1s605.ref   := 3;
let s1s606.sosno := 53;
let s1s606.ref   := 4;
let s1s607.sosno := 53;
let s1s607.ref   := 5;
let s1s608.sosno := 53;
let s1s608.ref   := 6;
let s1s609.sosno := 53;
let s1s609.ref   := 7;
let s1s610.sosno := 54;
let s1s610.ref   := 1;
let s1s611.sosno := 54;
let s1s611.ref   := 2;
let s1s612.sosno := 54;
let s1s612.ref   := 3;
let s1s613.sosno := 54;
let s1s613.ref   := 4;
let s1s614.sosno := 54;
let s1s614.ref   := 5;
let s1s615.sosno := 54;
let s1s615.ref   := 6;
let s1s616.sosno := 54;
let s1s616.ref   := 7;
let s1s617.sosno := 55;
let s1s617.ref   := 1;
let s1s618.sosno := 55;
let s1s618.ref   := 2;
let s1s619.sosno := 55;
let s1s619.ref   := 3;
let s1s620.sosno := 55;
let s1s620.ref   := 4;
let s1s621.sosno := 55;
let s1s621.ref   := 5;
let s1s622.sosno := 55;
let s1s622.ref   := 6;
let s1s623.sosno := 55;
let s1s623.ref   := 7;
let s1s624.sosno := 56;
let s1s624.ref   := 1;
let s1s625.sosno := 56;
let s1s625.ref   := 2;
let s1s626.sosno := 56;
let s1s626.ref   := 3;
let s1s627.sosno := 56;
let s1s627.ref   := 4;
let s1s628.sosno := 56;
let s1s628.ref   := 5;
let s1s629.sosno := 56;
let s1s629.ref   := 6;
let s1s630.sosno := 56;
let s1s630.ref   := 7;

minimize obj:    10*x179 + x180 + 10*x181;

subject to

e1:  - x1 - x2 - x3 - x4 - x5 - x6 - x7 + x8 + x15 + x22 + x29 + x36 + x43
     + x50 + x177 = 0;

e2:    x1 - x8 - x9 - x10 - x11 - x12 - x13 - x14 + x16 + x23 + x30 + x37 + x44
     + x51 + x178 = 0;

e3:    x2 + x9 - x15 - x16 - x17 - x18 - x19 - x20 - x21 + x24 + x31 + x38
     + x45 + x52 = 1.212;

e4:    x3 + x10 + x17 - x22 - x23 - x24 - x25 - x26 - x27 - x28 + x32 + x39
     + x46 + x53 = 0.452;

e5:    x4 + x11 + x18 + x25 - x29 - x30 - x31 - x32 - x33 - x34 - x35 + x40
     + x47 + x54 = 0.245;

e6:    x5 + x12 + x19 + x26 + x33 - x36 - x37 - x38 - x39 - x40 - x41 - x42
     + x48 + x55 = 0.652;

e7:    x6 + x13 + x20 + x27 + x34 + x41 - x43 - x44 - x45 - x46 - x47 - x48
     - x49 + x56 = 0.252;

e8:    x7 + x14 + x21 + x28 + x35 + x42 + x49 - x50 - x51 - x52 - x53 - x54
     - x55 - x56 = 0.456;

e9:    x57 - 79411.582707283*s1s239 - 5217.11027198889*s1s240
     - 426.059750561967*s1s241 - 49.0799484616014*s1s242
     - 3.2244100235547*s1s243 - 0.37143587857431*s1s244
     - 0.0801604941350881*s1s245 = 0;

e10:    x58 - 38721.1970117411*s1s246 - 2543.8701482414*s1s247
      - 207.747320703761*s1s248 - 23.9314504121258*s1s249
      - 1.5722267648148*s1s250 - 0.181112645550961*s1s251
      - 0.0390863672545667*s1s252 = 0;

e11:    x59 - 32510.4890865135*s1s253 - 2135.84468132099*s1s254
      - 174.425573683688*s1s255 - 20.0929521164322*s1s256
      - 1.32004857865156*s1s257 - 0.152062982061963*s1s258
      - 0.0328170876451919*s1s259 = 0;

e12:    x60 - 61354.8285678982*s1s260 - 4030.83398472982*s1s261
      - 329.181487941979*s1s262 - 37.9200580233072*s1s263
      - 2.49123764422425*s1s264 - 0.286978094088577*s1s265
      - 0.0619334511151265*s1s266 = 0;

e13:    x61 - 69005.9713947913*s1s267 - 4533.49183984114*s1s268
      - 370.231469483791*s1s269 - 42.6488102130281*s1s270
      - 2.8019029247995*s1s271 - 0.32276517780003*s1s272
      - 0.0696567500193006*s1s273 = 0;

e14:    x62 - 98671.1700093743*s1s274 - 6482.40920348584*s1s275
      - 529.391464678563*s1s276 - 60.9832441768203*s1s277
      - 4.00642196978801*s1s278 - 0.461519736453078*s1s279
      - 0.0996017139463627*s1s280 = 0;

e15:    x63 - 63468.4628982673*s1s281 - 4169.69361956223*s1s282
      - 340.521578201805*s1s283 - 39.2263796008983*s1s284
      - 2.57705917665854*s1s285 - 0.296864304610023*s1s286
      - 0.0640670186196026*s1s287 = 0;

e16:    x64 - 79411.582707283*s1s288 - 5217.11027198889*s1s289
      - 426.059750561967*s1s290 - 49.0799484616014*s1s291
      - 3.2244100235547*s1s292 - 0.37143587857431*s1s293
      - 0.0801604941350881*s1s294 = 0;

e17:    x65 - 50797.5773435889*s1s295 - 3337.25325093014*s1s296
      - 272.539627020641*s1s297 - 31.3951994533022*s1s298
      - 2.06257339263358*s1s299 - 0.237598120158509*s1s300
      - 0.0512766370081929*s1s301 = 0;

e18:    x66 - 82599.0313850111*s1s302 - 5426.51638972501*s1s303
      - 443.161079389119*s1s304 - 51.0499358550468*s1s305
      - 3.35383247196398*s1s306 - 0.386344696150538*s1s307
      - 0.0833780028702903*s1s308 = 0;

e19:    x67 - 82628.2406856418*s1s309 - 5428.43535591562*s1s310
      - 443.317793396277*s1s311 - 51.0679885234445*s1s312
      - 3.35501847982988*s1s313 - 0.386481318314113*s1s314
      - 0.0834074876367699*s1s315 = 0;

e20:    x68 - 59165.7349698592*s1s316 - 3887.01689524085*s1s317
      - 317.436542928413*s1s318 - 36.5670992066393*s1s319
      - 2.40235218067626*s1s320 - 0.27673893405488*s1s321
      - 0.0597237127048799*s1s322 = 0;

e21:    x69 - 32977.2294678044*s1s323 - 2166.50816836621*s1s324
      - 176.929733450444*s1s325 - 20.3814187742893*s1s326 - 1.339*s1s327
      - 0.154246090843839*s1s328 - 0.0332882297421199*s1s329 = 0;

e22:    x70 - 33843.9321019273*s1s330 - 2223.4480134252*s1s331
      - 181.579774357788*s1s332 - 20.9170801874496*s1s333
      - 1.37419139860501*s1s334 - 0.158299963634093*s1s335
      - 0.0341631060391402*s1s336 = 0;

e23:    x71 - 38721.1970117411*s1s337 - 2543.8701482414*s1s338
      - 207.747320703761*s1s339 - 23.9314504121258*s1s340
      - 1.5722267648148*s1s341 - 0.181112645550961*s1s342
      - 0.0390863672545667*s1s343 = 0;

e24:    x72 - 50797.5773435889*s1s344 - 3337.25325093014*s1s345
      - 272.539627020641*s1s346 - 31.3951994533022*s1s347
      - 2.06257339263358*s1s348 - 0.237598120158509*s1s349
      - 0.0512766370081929*s1s350 = 0;

e25:    x73 - 31810.181054648*s1s351 - 2089.8364782095*s1s352
      - 170.668274619734*s1s353 - 19.660130090483*s1s354
      - 1.2916134290104*s1s355 - 0.148787395299671*s1s356
      - 0.0321101751776739*s1s357 = 0;

e26:    x74 - 39461.9459070343*s1s358 - 2592.53519858857*s1s359
      - 211.721593458417*s1s360 - 24.3892667200816*s1s361
      - 1.60230396616872*s1s362 - 0.184577388442944*s1s363
      - 0.0398341019735132*s1s364 = 0;

e27:    x75 - 32977.2294678044*s1s365 - 2166.50816836621*s1s366
      - 176.929733450444*s1s367 - 20.3814187742893*s1s368 - 1.339*s1s369
      - 0.154246090843839*s1s370 - 0.0332882297421199*s1s371 = 0;

e28:    x76 - 61928.9823925488*s1s372 - 4068.554222939*s1s373
      - 332.261943298429*s1s374 - 38.2749110455263*s1s375
      - 2.51455045684114*s1s376 - 0.289663613291512*s1s377
      - 0.0625130196456165*s1s378 = 0;

e29:    x77 - 52785.5148814787*s1s379 - 3467.85497167945*s1s380
      - 283.205327698691*s1s381 - 32.6238347301504*s1s382
      - 2.14329116080854*s1s383 - 0.246896402610059*s1s384
      - 0.0532833223041444*s1s385 = 0;

e30:    x78 - 32510.4890865135*s1s386 - 2135.84468132099*s1s387
      - 174.425573683688*s1s388 - 20.0929521164322*s1s389
      - 1.32004857865156*s1s390 - 0.152062982061963*s1s391
      - 0.0328170876451919*s1s392 = 0;

e31:    x79 - 82599.0313850111*s1s393 - 5426.51638972501*s1s394
      - 443.161079389119*s1s395 - 51.0499358550468*s1s396
      - 3.35383247196398*s1s397 - 0.386344696150538*s1s398
      - 0.0833780028702903*s1s399 = 0;

e32:    x80 - 31810.181054648*s1s400 - 2089.8364782095*s1s401
      - 170.668274619734*s1s402 - 19.660130090483*s1s403
      - 1.2916134290104*s1s404 - 0.148787395299671*s1s405
      - 0.0321101751776739*s1s406 = 0;

e33:    x81 - 30677.4142839491*s1s407 - 2015.41699236491*s1s408
      - 164.590743970989*s1s409 - 18.9600290116536*s1s410
      - 1.24561882211213*s1s411 - 0.143489047044288*s1s412
      - 0.0309667255575633*s1s413 = 0;

e34:    x82 - 47609.7953674264*s1s414 - 3127.8252364552*s1s415
      - 255.436509977676*s1s416 - 29.4250060663552*s1s417
      - 1.93313741104972*s1s418 - 0.222687743628377*s1s419
      - 0.0480587918312993*s1s420 = 0;

e35:    x83 - 90800.3982081977*s1s421 - 5965.32236284499*s1s422
      - 487.1631277532*s1s423 - 56.1187513511491*s1s424
      - 3.68683892379366*s1s425 - 0.424705370848452*s1s426
      - 0.0916567148001743*s1s427 = 0;

e36:    x84 - 79866.0883229559*s1s428 - 5246.96997047827*s1s429
      - 428.498268252218*s1s430 - 49.3608534811495*s1s431
      - 3.24286466723482*s1s432 - 0.373561761057936*s1s433
      - 0.0806192860832829*s1s434 = 0;

e37:    x85 - 61354.8285678982*s1s435 - 4030.83398472982*s1s436
      - 329.181487941979*s1s437 - 37.9200580233072*s1s438
      - 2.49123764422425*s1s439 - 0.286978094088577*s1s440
      - 0.0619334511151265*s1s441 = 0;

e38:    x86 - 82628.2406856418*s1s442 - 5428.43535591562*s1s443
      - 443.317793396277*s1s444 - 51.0679885234445*s1s445
      - 3.35501847982988*s1s446 - 0.386481318314113*s1s447
      - 0.0834074876367699*s1s448 = 0;

e39:    x87 - 39461.9459070343*s1s449 - 2592.53519858857*s1s450
      - 211.721593458417*s1s451 - 24.3892667200816*s1s452
      - 1.60230396616872*s1s453 - 0.184577388442944*s1s454
      - 0.0398341019735132*s1s455 = 0;

e40:    x88 - 30677.4142839491*s1s456 - 2015.41699236491*s1s457
      - 164.590743970989*s1s458 - 18.9600290116536*s1s459
      - 1.24561882211213*s1s460 - 0.143489047044288*s1s461
      - 0.0309667255575633*s1s462 = 0;

e41:    x89 - 28361.2795383154*s1s463 - 1863.25366856746*s1s464
      - 152.164196629274*s1s465 - 17.5285530220005*s1s466
      - 1.15157500841239*s1s467 - 0.132655670919396*s1s468
      - 0.0286287479053886*s1s469 = 0;

e42:    x90 - 79005.3826894827*s1s470 - 5190.42411093907*s1s471
      - 423.880402507663*s1s472 - 48.8288984855264*s1s473
      - 3.20791676949387*s1s474 - 0.369535938347143*s1s475
      - 0.0797504633431748*s1s476 = 0;

e43:    x91 - 91918.5525763874*s1s477 - 6038.78185629766*s1s478
      - 493.162259806225*s1s479 - 56.8098213045733*s1s480
      - 3.73224021199065*s1s481 - 0.429935371762313*s1s482
      - 0.0927854142117423*s1s483 = 0;

e44:    x92 - 69005.9713947913*s1s484 - 4533.49183984114*s1s485
      - 370.231469483791*s1s486 - 42.6488102130281*s1s487
      - 2.8019029247995*s1s488 - 0.32276517780003*s1s489
      - 0.0696567500193006*s1s490 = 0;

e45:    x93 - 59165.7349698592*s1s491 - 3887.01689524085*s1s492
      - 317.436542928413*s1s493 - 36.5670992066393*s1s494
      - 2.40235218067626*s1s495 - 0.27673893405488*s1s496
      - 0.0597237127048799*s1s497 = 0;

e46:    x94 - 32977.2294678044*s1s498 - 2166.50816836621*s1s499
      - 176.929733450444*s1s500 - 20.3814187742893*s1s501 - 1.339*s1s502
      - 0.154246090843839*s1s503 - 0.0332882297421199*s1s504 = 0;

e47:    x95 - 47609.7953674264*s1s505 - 3127.8252364552*s1s506
      - 255.436509977676*s1s507 - 29.4250060663552*s1s508
      - 1.93313741104972*s1s509 - 0.222687743628377*s1s510
      - 0.0480587918312993*s1s511 = 0;

e48:    x96 - 28361.2795383154*s1s512 - 1863.25366856746*s1s513
      - 152.164196629274*s1s514 - 17.5285530220005*s1s515
      - 1.15157500841239*s1s516 - 0.132655670919396*s1s517
      - 0.0286287479053886*s1s518 = 0;

e49:    x97 - 50797.5773435889*s1s519 - 3337.25325093014*s1s520
      - 272.539627020641*s1s521 - 31.3951994533022*s1s522
      - 2.06257339263358*s1s523 - 0.237598120158509*s1s524
      - 0.0512766370081929*s1s525 = 0;

e50:    x98 - 76690.9136638299*s1s526 - 5038.370220105*s1s527
      - 411.462792102039*s1s528 - 47.3984519861318*s1s529
      - 3.11394059031318*s1s530 - 0.358710353379957*s1s531
      - 0.0774141671199832*s1s532 = 0;

e51:    x99 - 98671.1700093743*s1s533 - 6482.40920348584*s1s534
      - 529.391464678563*s1s535 - 60.9832441768203*s1s536
      - 4.00642196978801*s1s537 - 0.461519736453078*s1s538
      - 0.0996017139463627*s1s539 = 0;

e52:    x100 - 32977.2294678044*s1s540 - 2166.50816836621*s1s541
      - 176.929733450444*s1s542 - 20.3814187742893*s1s543 - 1.339*s1s544
      - 0.154246090843839*s1s545 - 0.0332882297421199*s1s546 = 0;

e53:    x101 - 61928.9823925488*s1s547 - 4068.554222939*s1s548
      - 332.261943298429*s1s549 - 38.2749110455263*s1s550
      - 2.51455045684114*s1s551 - 0.289663613291512*s1s552
      - 0.0625130196456165*s1s553 = 0;

e54:    x102 - 90800.3982081977*s1s554 - 5965.32236284499*s1s555
      - 487.1631277532*s1s556 - 56.1187513511491*s1s557
      - 3.68683892379366*s1s558 - 0.424705370848452*s1s559
      - 0.0916567148001743*s1s560 = 0;

e55:    x103 - 79005.3826894827*s1s561 - 5190.42411093907*s1s562
      - 423.880402507663*s1s563 - 48.8288984855264*s1s564
      - 3.20791676949387*s1s565 - 0.369535938347143*s1s566
      - 0.0797504633431748*s1s567 = 0;

e56:    x104 - 50797.5773435889*s1s568 - 3337.25325093014*s1s569
      - 272.539627020641*s1s570 - 31.3951994533022*s1s571
      - 2.06257339263358*s1s572 - 0.237598120158509*s1s573
      - 0.0512766370081929*s1s574 = 0;

e57:    x105 - 66392.0532160028*s1s575 - 4361.76500962592*s1s576
      - 356.207251740277*s1s577 - 41.0332905983307*s1s578
      - 2.69576797963029*s1s579 - 0.310538963913342*s1s580
      - 0.0670181806104433*s1s581 = 0;

e58:    x106 - 63468.4628982673*s1s582 - 4169.69361956223*s1s583
      - 340.521578201805*s1s584 - 39.2263796008983*s1s585
      - 2.57705917665854*s1s586 - 0.296864304610023*s1s587
      - 0.0640670186196026*s1s588 = 0;

e59:    x107 - 33843.9321019273*s1s589 - 2223.4480134252*s1s590
      - 181.579774357788*s1s591 - 20.9170801874496*s1s592
      - 1.37419139860501*s1s593 - 0.158299963634093*s1s594
      - 0.0341631060391402*s1s595 = 0;

e60:    x108 - 52785.5148814787*s1s596 - 3467.85497167945*s1s597
      - 283.205327698691*s1s598 - 32.6238347301504*s1s599
      - 2.14329116080854*s1s600 - 0.246896402610059*s1s601
      - 0.0532833223041444*s1s602 = 0;

e61:    x109 - 79866.0883229559*s1s603 - 5246.96997047827*s1s604
      - 428.498268252218*s1s605 - 49.3608534811495*s1s606
      - 3.24286466723482*s1s607 - 0.373561761057936*s1s608
      - 0.0806192860832829*s1s609 = 0;

e62:    x110 - 91918.5525763874*s1s610 - 6038.78185629766*s1s611
      - 493.162259806225*s1s612 - 56.8098213045733*s1s613
      - 3.73224021199065*s1s614 - 0.429935371762313*s1s615
      - 0.0927854142117423*s1s616 = 0;

e63:    x111 - 76690.9136638299*s1s617 - 5038.370220105*s1s618
      - 411.462792102039*s1s619 - 47.3984519861318*s1s620
      - 3.11394059031318*s1s621 - 0.358710353379957*s1s622
      - 0.0774141671199832*s1s623 = 0;

e64:    x112 - 66392.0532160028*s1s624 - 4361.76500962592*s1s625
      - 356.207251740277*s1s626 - 41.0332905983307*s1s627
      - 2.69576797963029*s1s628 - 0.310538963913342*s1s629
      - 0.0670181806104433*s1s630 = 0;

e65: -x1^2*x57 + x113 - x114 - x121 = 0;

e66: -x2^2*x58 + x113 - x115 - x122 = 0;

e67: -x3^2*x59 + x113 - x116 - x123 = 0;

e68: -x4^2*x60 + x113 - x117 - x124 = 0;

e69: -x5^2*x61 + x113 - x118 - x125 = 0;

e70: -x6^2*x62 + x113 - x119 - x126 = 0;

e71: -x7^2*x63 + x113 - x120 - x127 = 0;

e72: -x8^2*x64 - x113 + x114 - x128 = 0;

e73: -x9^2*x65 + x114 - x115 - x129 = 0;

e74: -x10^2*x66 + x114 - x116 - x130 = 0;

e75: -x11^2*x67 + x114 - x117 - x131 = 0;

e76: -x12^2*x68 + x114 - x118 - x132 = 0;

e77: -x13^2*x69 + x114 - x119 - x133 = 0;

e78: -x14^2*x70 + x114 - x120 - x134 = 0;

e79: -x15^2*x71 - x113 + x115 - x135 = 0;

e80: -x16^2*x72 - x114 + x115 - x136 = 0;

e81: -x17^2*x73 + x115 - x116 - x137 = 0;

e82: -x18^2*x74 + x115 - x117 - x138 = 0;

e83: -x19^2*x75 + x115 - x118 - x139 = 0;

e84: -x20^2*x76 + x115 - x119 - x140 = 0;

e85: -x21^2*x77 + x115 - x120 - x141 = 0;

e86: -x22^2*x78 - x113 + x116 - x142 = 0;

e87: -x23^2*x79 - x114 + x116 - x143 = 0;

e88: -x24^2*x80 - x115 + x116 - x144 = 0;

e89: -x25^2*x81 + x116 - x117 - x145 = 0;

e90: -x26^2*x82 + x116 - x118 - x146 = 0;

e91: -x27^2*x83 + x116 - x119 - x147 = 0;

e92: -x28^2*x84 + x116 - x120 - x148 = 0;

e93: -x29^2*x85 - x113 + x117 - x149 = 0;

e94: -x30^2*x86 - x114 + x117 - x150 = 0;

e95: -x31^2*x87 - x115 + x117 - x151 = 0;

e96: -x32^2*x88 - x116 + x117 - x152 = 0;

e97: -x33^2*x89 + x117 - x118 - x153 = 0;

e98: -x34^2*x90 + x117 - x119 - x154 = 0;

e99: -x35^2*x91 + x117 - x120 - x155 = 0;

e100: -x36^2*x92 - x113 + x118 - x156 = 0;

e101: -x37^2*x93 - x114 + x118 - x157 = 0;

e102: -x38^2*x94 - x115 + x118 - x158 = 0;

e103: -x39^2*x95 - x116 + x118 - x159 = 0;

e104: -x40^2*x96 - x117 + x118 - x160 = 0;

e105: -x41^2*x97 + x118 - x119 - x161 = 0;

e106: -x42^2*x98 + x118 - x120 - x162 = 0;

e107: -x43^2*x99 - x113 + x119 - x163 = 0;

e108: -x44^2*x100 - x114 + x119 - x164 = 0;

e109: -x45^2*x101 - x115 + x119 - x165 = 0;

e110: -x46^2*x102 - x116 + x119 - x166 = 0;

e111: -x47^2*x103 - x117 + x119 - x167 = 0;

e112: -x48^2*x104 - x118 + x119 - x168 = 0;

e113: -x49^2*x105 + x119 - x120 - x169 = 0;

e114: -x50^2*x106 - x113 + x120 - x170 = 0;

e115: -x51^2*x107 - x114 + x120 - x171 = 0;

e116: -x52^2*x108 - x115 + x120 - x172 = 0;

e117: -x53^2*x109 - x116 + x120 - x173 = 0;

e118: -x54^2*x110 - x117 + x120 - x174 = 0;

e119: -x55^2*x111 - x118 + x120 - x175 = 0;

e120: -x56^2*x112 - x119 + x120 - x176 = 0;

e121:    x121 + 12*b183 <= 12;

e122:    x122 + 12*b184 <= 12;

e123:    x123 + 12*b185 <= 12;

e124:    x124 + 12*b186 <= 12;

e125:    x125 + 12*b187 <= 12;

e126:    x126 + 12*b188 <= 12;

e127:    x127 + 12*b189 <= 12;

e128:    x128 + 12*b190 <= 12;

e129:    x129 + 12*b191 <= 12;

e130:    x130 + 12*b192 <= 12;

e131:    x131 + 12*b193 <= 12;

e132:    x132 + 12*b194 <= 12;

e133:    x133 + 12*b195 <= 12;

e134:    x134 + 12*b196 <= 12;

e135:    x135 + 12*b197 <= 12;

e136:    x136 + 12*b198 <= 12;

e137:    x137 + 12*b199 <= 12;

e138:    x138 + 12*b200 <= 12;

e139:    x139 + 12*b201 <= 12;

e140:    x140 + 12*b202 <= 12;

e141:    x141 + 12*b203 <= 12;

e142:    x142 + 12*b204 <= 12;

e143:    x143 + 12*b205 <= 12;

e144:    x144 + 12*b206 <= 12;

e145:    x145 + 12*b207 <= 12;

e146:    x146 + 12*b208 <= 12;

e147:    x147 + 12*b209 <= 12;

e148:    x148 + 12*b210 <= 12;

e149:    x149 + 12*b211 <= 12;

e150:    x150 + 12*b212 <= 12;

e151:    x151 + 12*b213 <= 12;

e152:    x152 + 12*b214 <= 12;

e153:    x153 + 12*b215 <= 12;

e154:    x154 + 12*b216 <= 12;

e155:    x155 + 12*b217 <= 12;

e156:    x156 + 12*b218 <= 12;

e157:    x157 + 12*b219 <= 12;

e158:    x158 + 12*b220 <= 12;

e159:    x159 + 12*b221 <= 12;

e160:    x160 + 12*b222 <= 12;

e161:    x161 + 12*b223 <= 12;

e162:    x162 + 12*b224 <= 12;

e163:    x163 + 12*b225 <= 12;

e164:    x164 + 12*b226 <= 12;

e165:    x165 + 12*b227 <= 12;

e166:    x166 + 12*b228 <= 12;

e167:    x167 + 12*b229 <= 12;

e168:    x168 + 12*b230 <= 12;

e169:    x169 + 12*b231 <= 12;

e170:    x170 + 12*b232 <= 12;

e171:    x171 + 12*b233 <= 12;

e172:    x172 + 12*b234 <= 12;

e173:    x173 + 12*b235 <= 12;

e174:    x174 + 12*b236 <= 12;

e175:    x175 + 12*b237 <= 12;

e176:    x176 + 12*b238 <= 12;

e177:    x121 - 12*b183 >= -12;

e178:    x122 - 12*b184 >= -12;

e179:    x123 - 12*b185 >= -12;

e180:    x124 - 12*b186 >= -12;

e181:    x125 - 12*b187 >= -12;

e182:    x126 - 12*b188 >= -12;

e183:    x127 - 12*b189 >= -12;

e184:    x128 - 12*b190 >= -12;

e185:    x129 - 12*b191 >= -12;

e186:    x130 - 12*b192 >= -12;

e187:    x131 - 12*b193 >= -12;

e188:    x132 - 12*b194 >= -12;

e189:    x133 - 12*b195 >= -12;

e190:    x134 - 12*b196 >= -12;

e191:    x135 - 12*b197 >= -12;

e192:    x136 - 12*b198 >= -12;

e193:    x137 - 12*b199 >= -12;

e194:    x138 - 12*b200 >= -12;

e195:    x139 - 12*b201 >= -12;

e196:    x140 - 12*b202 >= -12;

e197:    x141 - 12*b203 >= -12;

e198:    x142 - 12*b204 >= -12;

e199:    x143 - 12*b205 >= -12;

e200:    x144 - 12*b206 >= -12;

e201:    x145 - 12*b207 >= -12;

e202:    x146 - 12*b208 >= -12;

e203:    x147 - 12*b209 >= -12;

e204:    x148 - 12*b210 >= -12;

e205:    x149 - 12*b211 >= -12;

e206:    x150 - 12*b212 >= -12;

e207:    x151 - 12*b213 >= -12;

e208:    x152 - 12*b214 >= -12;

e209:    x153 - 12*b215 >= -12;

e210:    x154 - 12*b216 >= -12;

e211:    x155 - 12*b217 >= -12;

e212:    x156 - 12*b218 >= -12;

e213:    x157 - 12*b219 >= -12;

e214:    x158 - 12*b220 >= -12;

e215:    x159 - 12*b221 >= -12;

e216:    x160 - 12*b222 >= -12;

e217:    x161 - 12*b223 >= -12;

e218:    x162 - 12*b224 >= -12;

e219:    x163 - 12*b225 >= -12;

e220:    x164 - 12*b226 >= -12;

e221:    x165 - 12*b227 >= -12;

e222:    x166 - 12*b228 >= -12;

e223:    x167 - 12*b229 >= -12;

e224:    x168 - 12*b230 >= -12;

e225:    x169 - 12*b231 >= -12;

e226:    x170 - 12*b232 >= -12;

e227:    x171 - 12*b233 >= -12;

e228:    x172 - 12*b234 >= -12;

e229:    x173 - 12*b235 >= -12;

e230:    x174 - 12*b236 >= -12;

e231:    x175 - 12*b237 >= -12;

e232:    x176 - 12*b238 >= -12;

e233: -(1.02*x177*(-6.5 + x113) + 1.02*x178*(-3.25 + x114)) + x179 = 0;

e234:    x180 - 18.6904540877619*s1s239 - 36.1247408097722*s1s240
       - 66.2398770083886*s1s241 - 111.757644998947*s1s242
       - 216.00416662648*s1s243 - 364.434809700482*s1s244
       - 528.190788533686*s1s245 - 9.11349113439539*s1s246
       - 17.6144733325531*s1s247 - 32.2986551864818*s1s248
       - 54.4931814987685*s1s249 - 105.323928905069*s1s250
       - 177.698914733437*s1s251 - 257.546555368226*s1s252
       - 7.65172765642961*s1s253 - 14.7891900880288*s1s254
       - 27.118094428506*s1s255 - 45.7527173518919*s1s256
       - 88.4304387640365*s1s257 - 149.196798497086*s1s258
       - 216.237232413786*s1s259 - 14.4405836946709*s1s260
       - 27.9106297076743*s1s261 - 51.1781299358943*s1s262
       - 86.3459827433152*s1s263 - 166.88873538978*s1s264
       - 281.568941344082*s1s265 - 408.089779561278*s1s266
       - 16.2413705427568*s1s267 - 31.3911742592025*s1s268
       - 57.5602059825984*s1s269 - 97.1136022105662*s1s270
       - 187.700293020549*s1s271 - 316.681486454637*s1s272
       - 458.979876763054*s1s273 - 23.2234254748939*s1s274
       - 44.8860269555956*s1s275 - 82.3049477528588*s1s276
       - 138.862080487498*s1s277 - 268.391374675119*s1s278
       - 452.820707501052*s1s279 - 656.292209725887*s1s280
       - 14.9380525029139*s1s281 - 28.8721329260735*s1s282
       - 52.941183552398*s1s283 - 89.3205462402005*s1s284
       - 172.637944844116*s1s285 - 291.268810037089*s1s286
       - 422.148209648796*s1s287 - 18.6904540877619*s1s288
       - 36.1247408097722*s1s289 - 66.2398770083886*s1s290
       - 111.757644998947*s1s291 - 216.00416662648*s1s292
       - 364.434809700482*s1s293 - 528.190788533686*s1s294
       - 11.9558099050809*s1s295 - 23.1080813747994*s1s296
       - 42.3719709499612*s1s297 - 71.4885338137291*s1s298
       - 138.172392322055*s1s299 - 233.119713791557*s1s300
       - 337.870264236031*s1s301 - 19.4406577877407*s1s302
       - 37.5747277436911*s1s303 - 68.8986353555371*s1s304
       - 116.243410747892*s1s305 - 224.674214141276*s1s306
       - 379.062616031703*s1s307 - 549.391487135874*s1s308
       - 19.4475325416967*s1s309 - 37.5880152060303*s1s310
       - 68.922999817437*s1s311 - 116.284517631036*s1s312
       - 224.753665153652*s1s313 - 379.196663050462*s1s314
       - 549.585767151543*s1s315 - 13.9253546563734*s1s316
       - 26.9147996770731*s1s317 - 49.3521332015331*s1s318
       - 83.2652237802191*s1s319 - 160.93427229773*s1s320
       - 271.522775764452*s1s321 - 393.529446744536*s1s322
       - 7.76158051882097*s1s323 - 15.0015127080393*s1s324
       - 27.5074183079396*s1s325 - 46.4095712271164*s1s326 - 89.7*s1s327
       - 151.338758602103*s1s328 - 219.341665817957*s1s329
       - 7.96556922221359*s1s330 - 15.3957802311063*s1s331
       - 28.2303641796868*s1s332 - 47.6293006671023*s1s333
       - 92.0574820424717*s1s334 - 155.316221319321*s1s335
       - 225.10637081608*s1s336 - 9.11349113439539*s1s337
       - 17.6144733325531*s1s338 - 32.2986551864818*s1s339
       - 54.4931814987685*s1s340 - 105.323928905069*s1s341
       - 177.698914733437*s1s342 - 257.546555368226*s1s343
       - 11.9558099050809*s1s344 - 23.1080813747994*s1s345
       - 42.3719709499612*s1s346 - 71.4885338137291*s1s347
       - 138.172392322055*s1s348 - 233.119713791557*s1s349
       - 337.870264236031*s1s350 - 7.48690188831565*s1s351
       - 14.4706163324673*s1s352 - 26.5339439013751*s1s353
       - 44.7671586494086*s1s354 - 86.5255598074927*s1s355
       - 145.982952158506*s1s356 - 211.579268940989*s1s357
       - 9.28783513744935*s1s358 - 17.9514438466182*s1s359
       - 32.916538800503*s1s360 - 55.5356535066454*s1s361
       - 107.338809384118*s1s362 - 181.098351861986*s1s363
       - 262.473503425068*s1s364 - 7.76158051882097*s1s365
       - 15.0015127080393*s1s366 - 27.5074183079396*s1s367
       - 46.4095712271164*s1s368 - 89.7*s1s369 - 151.338758602103*s1s370
       - 219.341665817957*s1s371 - 14.5757175798436*s1s372
       - 28.1718152601258*s1s373 - 51.657050987864*s1s374
       - 87.154001890177*s1s375 - 168.450467497125*s1s376
       - 284.203842106564*s1s377 - 411.908652716088*s1s378
       - 12.4236944883441*s1s379 - 24.0124044704238*s1s380
       - 44.0301766363479*s1s381 - 74.2862014846846*s1s382
       - 143.579699122125*s1s383 - 242.242736071415*s1s384
       - 351.092646411238*s1s385 - 7.65172765642961*s1s386
       - 14.7891900880288*s1s387 - 27.118094428506*s1s388
       - 45.7527173518919*s1s389 - 88.4304387640365*s1s390
       - 149.196798497086*s1s391 - 216.237232413786*s1s392
       - 19.4406577877407*s1s393 - 37.5747277436911*s1s394
       - 68.8986353555371*s1s395 - 116.243410747892*s1s396
       - 224.674214141276*s1s397 - 379.062616031703*s1s398
       - 549.391487135874*s1s399 - 7.48690188831565*s1s400
       - 14.4706163324673*s1s401 - 26.5339439013751*s1s402
       - 44.7671586494086*s1s403 - 86.5255598074927*s1s404
       - 145.982952158506*s1s405 - 211.579268940989*s1s406
       - 7.22029184733547*s1s407 - 13.9553148538372*s1s408
       - 25.5890649679471*s1s409 - 43.1729913716576*s1s410
       - 83.44436769489*s1s411 - 140.784470672041*s1s412
       - 204.044889780639*s1s413 - 11.2055277593782*s1s414
       - 21.6579428216942*s1s415 - 39.7129345873577*s1s416
       - 67.0022990066581*s1s417 - 129.501438215952*s1s418
       - 218.490377890626*s1s419 - 316.667348763747*s1s420
       - 21.3709463532097*s1s421 - 41.3055720446486*s1s422
       - 75.7396717780329*s1s423 - 127.785372394852*s1s424
       - 246.982413341517*s1s425 - 416.700243385248*s1s426
       - 603.94129286586*s1s427 - 18.7974273535344*s1s428
       - 36.3314977714524*s1s429 - 66.6189954575545*s1s430
       - 112.397280622803*s1s431 - 217.24044858175*s1s432
       - 366.520622146333*s1s433 - 531.213844759881*s1s434
       - 14.4405836946709*s1s435 - 27.9106297076743*s1s436
       - 51.1781299358943*s1s437 - 86.3459827433152*s1s438
       - 166.88873538978*s1s439 - 281.568941344082*s1s440
       - 408.089779561278*s1s441 - 19.4475325416967*s1s442
       - 37.5880152060303*s1s443 - 68.922999817437*s1s444
       - 116.284517631036*s1s445 - 224.753665153652*s1s446
       - 379.196663050462*s1s447 - 549.585767151543*s1s448
       - 9.28783513744935*s1s449 - 17.9514438466182*s1s450
       - 32.916538800503*s1s451 - 55.5356535066454*s1s452
       - 107.338809384118*s1s453 - 181.098351861986*s1s454
       - 262.473503425068*s1s455 - 7.22029184733547*s1s456
       - 13.9553148538372*s1s457 - 25.5890649679471*s1s458
       - 43.1729913716576*s1s459 - 83.44436769489*s1s460
       - 140.784470672041*s1s461 - 204.044889780639*s1s462
       - 6.67516217420068*s1s463 - 12.9016931463472*s1s464
       - 23.6570989315674*s1s465 - 39.913444642481*s1s466
       - 77.1443452237428*s1s467 - 130.155289178744*s1s468
       - 188.639567333459*s1s469 - 18.5948501150879*s1s470
       - 35.9399583150819*s1s471 - 65.9010518861256*s1s472
       - 111.185990892074*s1s473 - 214.899278733085*s1s474
       - 362.570680802145*s1s475 - 525.489027652088*s1s476
       - 21.6341171926414*s1s477 - 41.8142262655959*s1s478
       - 76.6723620141448*s1s479 - 129.358975321201*s1s480
       - 250.023858861509*s1s481 - 421.831665786045*s1s482
       - 611.378480456157*s1s483 - 16.2413705427568*s1s484
       - 31.3911742592025*s1s485 - 57.5602059825984*s1s486
       - 97.1136022105662*s1s487 - 187.700293020549*s1s488
       - 316.681486454637*s1s489 - 458.979876763054*s1s490
       - 13.9253546563734*s1s491 - 26.9147996770731*s1s492
       - 49.3521332015331*s1s493 - 83.2652237802191*s1s494
       - 160.93427229773*s1s495 - 271.522775764452*s1s496
       - 393.529446744536*s1s497 - 7.76158051882097*s1s498
       - 15.0015127080393*s1s499 - 27.5074183079396*s1s500
       - 46.4095712271164*s1s501 - 89.7*s1s502 - 151.338758602103*s1s503
       - 219.341665817957*s1s504 - 11.2055277593782*s1s505
       - 21.6579428216942*s1s506 - 39.7129345873577*s1s507
       - 67.0022990066581*s1s508 - 129.501438215952*s1s509
       - 218.490377890626*s1s510 - 316.667348763747*s1s511
       - 6.67516217420068*s1s512 - 12.9016931463472*s1s513
       - 23.6570989315674*s1s514 - 39.913444642481*s1s515
       - 77.1443452237428*s1s516 - 130.155289178744*s1s517
       - 188.639567333459*s1s518 - 11.9558099050809*s1s519
       - 23.1080813747994*s1s520 - 42.3719709499612*s1s521
       - 71.4885338137291*s1s522 - 138.172392322055*s1s523
       - 233.119713791557*s1s524 - 337.870264236031*s1s525
       - 18.050112488828*s1s526 - 34.8870943522498*s1s527
       - 63.9704752829104*s1s528 - 107.928788366802*s1s529
       - 208.603787118067*s1s530 - 351.949143613665*s1s531
       - 510.094784419302*s1s532 - 23.2234254748939*s1s533
       - 44.8860269555956*s1s534 - 82.3049477528588*s1s535
       - 138.862080487498*s1s536 - 268.391374675119*s1s537
       - 452.820707501052*s1s538 - 656.292209725887*s1s539
       - 7.76158051882097*s1s540 - 15.0015127080393*s1s541
       - 27.5074183079396*s1s542 - 46.4095712271164*s1s543 - 89.7*s1s544
       - 151.338758602103*s1s545 - 219.341665817957*s1s546
       - 14.5757175798436*s1s547 - 28.1718152601258*s1s548
       - 51.657050987864*s1s549 - 87.154001890177*s1s550
       - 168.450467497125*s1s551 - 284.203842106564*s1s552
       - 411.908652716088*s1s553 - 21.3709463532097*s1s554
       - 41.3055720446486*s1s555 - 75.7396717780329*s1s556
       - 127.785372394852*s1s557 - 246.982413341517*s1s558
       - 416.700243385248*s1s559 - 603.94129286586*s1s560
       - 18.5948501150879*s1s561 - 35.9399583150819*s1s562
       - 65.9010518861256*s1s563 - 111.185990892074*s1s564
       - 214.899278733085*s1s565 - 362.570680802145*s1s566
       - 525.489027652088*s1s567 - 11.9558099050809*s1s568
       - 23.1080813747994*s1s569 - 42.3719709499612*s1s570
       - 71.4885338137291*s1s571 - 138.172392322055*s1s572
       - 233.119713791557*s1s573 - 337.870264236031*s1s574
       - 15.6261540208811*s1s575 - 30.2020893236365*s1s576
       - 55.3798487504404*s1s577 - 93.434978388672*s1s578
       - 180.590282130573*s1s579 - 304.685720326026*s1s580
       - 441.593905385226*s1s581 - 14.9380525029139*s1s582
       - 28.8721329260735*s1s583 - 52.941183552398*s1s584
       - 89.3205462402005*s1s585 - 172.637944844116*s1s586
       - 291.268810037089*s1s587 - 422.148209648796*s1s588
       - 7.96556922221359*s1s589 - 15.3957802311063*s1s590
       - 28.2303641796868*s1s591 - 47.6293006671023*s1s592
       - 92.0574820424717*s1s593 - 155.316221319321*s1s594
       - 225.10637081608*s1s595 - 12.4236944883441*s1s596
       - 24.0124044704238*s1s597 - 44.0301766363479*s1s598
       - 74.2862014846846*s1s599 - 143.579699122125*s1s600
       - 242.242736071415*s1s601 - 351.092646411238*s1s602
       - 18.7974273535344*s1s603 - 36.3314977714524*s1s604
       - 66.6189954575545*s1s605 - 112.397280622803*s1s606
       - 217.24044858175*s1s607 - 366.520622146333*s1s608
       - 531.213844759881*s1s609 - 21.6341171926414*s1s610
       - 41.8142262655959*s1s611 - 76.6723620141448*s1s612
       - 129.358975321201*s1s613 - 250.023858861509*s1s614
       - 421.831665786045*s1s615 - 611.378480456157*s1s616
       - 18.050112488828*s1s617 - 34.8870943522498*s1s618
       - 63.9704752829104*s1s619 - 107.928788366802*s1s620
       - 208.603787118067*s1s621 - 351.949143613665*s1s622
       - 510.094784419302*s1s623 - 15.6261540208811*s1s624
       - 30.2020893236365*s1s625 - 55.3798487504404*s1s626
       - 93.434978388672*s1s627 - 180.590282130573*s1s628
       - 304.685720326026*s1s629 - 441.593905385226*s1s630 = 0;

e235:  - 0.2*x177 - 0.17*x178 + x181 = 0;

e237:  - b183 + s1s239 + s1s240 + s1s241 + s1s242 + s1s243 + s1s244 + s1s245
       = 0;

e238:  - b184 + s1s246 + s1s247 + s1s248 + s1s249 + s1s250 + s1s251 + s1s252
       = 0;

e239:  - b185 + s1s253 + s1s254 + s1s255 + s1s256 + s1s257 + s1s258 + s1s259
       = 0;

e240:  - b186 + s1s260 + s1s261 + s1s262 + s1s263 + s1s264 + s1s265 + s1s266
       = 0;

e241:  - b187 + s1s267 + s1s268 + s1s269 + s1s270 + s1s271 + s1s272 + s1s273
       = 0;

e242:  - b188 + s1s274 + s1s275 + s1s276 + s1s277 + s1s278 + s1s279 + s1s280
       = 0;

e243:  - b189 + s1s281 + s1s282 + s1s283 + s1s284 + s1s285 + s1s286 + s1s287
       = 0;

e244:  - b190 + s1s288 + s1s289 + s1s290 + s1s291 + s1s292 + s1s293 + s1s294
       = 0;

e245:  - b191 + s1s295 + s1s296 + s1s297 + s1s298 + s1s299 + s1s300 + s1s301
       = 0;

e246:  - b192 + s1s302 + s1s303 + s1s304 + s1s305 + s1s306 + s1s307 + s1s308
       = 0;

e247:  - b193 + s1s309 + s1s310 + s1s311 + s1s312 + s1s313 + s1s314 + s1s315
       = 0;

e248:  - b194 + s1s316 + s1s317 + s1s318 + s1s319 + s1s320 + s1s321 + s1s322
       = 0;

e249:  - b195 + s1s323 + s1s324 + s1s325 + s1s326 + s1s327 + s1s328 + s1s329
       = 0;

e250:  - b196 + s1s330 + s1s331 + s1s332 + s1s333 + s1s334 + s1s335 + s1s336
       = 0;

e251:  - b197 + s1s337 + s1s338 + s1s339 + s1s340 + s1s341 + s1s342 + s1s343
       = 0;

e252:  - b198 + s1s344 + s1s345 + s1s346 + s1s347 + s1s348 + s1s349 + s1s350
       = 0;

e253:  - b199 + s1s351 + s1s352 + s1s353 + s1s354 + s1s355 + s1s356 + s1s357
       = 0;

e254:  - b200 + s1s358 + s1s359 + s1s360 + s1s361 + s1s362 + s1s363 + s1s364
       = 0;

e255:  - b201 + s1s365 + s1s366 + s1s367 + s1s368 + s1s369 + s1s370 + s1s371
       = 0;

e256:  - b202 + s1s372 + s1s373 + s1s374 + s1s375 + s1s376 + s1s377 + s1s378
       = 0;

e257:  - b203 + s1s379 + s1s380 + s1s381 + s1s382 + s1s383 + s1s384 + s1s385
       = 0;

e258:  - b204 + s1s386 + s1s387 + s1s388 + s1s389 + s1s390 + s1s391 + s1s392
       = 0;

e259:  - b205 + s1s393 + s1s394 + s1s395 + s1s396 + s1s397 + s1s398 + s1s399
       = 0;

e260:  - b206 + s1s400 + s1s401 + s1s402 + s1s403 + s1s404 + s1s405 + s1s406
       = 0;

e261:  - b207 + s1s407 + s1s408 + s1s409 + s1s410 + s1s411 + s1s412 + s1s413
       = 0;

e262:  - b208 + s1s414 + s1s415 + s1s416 + s1s417 + s1s418 + s1s419 + s1s420
       = 0;

e263:  - b209 + s1s421 + s1s422 + s1s423 + s1s424 + s1s425 + s1s426 + s1s427
       = 0;

e264:  - b210 + s1s428 + s1s429 + s1s430 + s1s431 + s1s432 + s1s433 + s1s434
       = 0;

e265:  - b211 + s1s435 + s1s436 + s1s437 + s1s438 + s1s439 + s1s440 + s1s441
       = 0;

e266:  - b212 + s1s442 + s1s443 + s1s444 + s1s445 + s1s446 + s1s447 + s1s448
       = 0;

e267:  - b213 + s1s449 + s1s450 + s1s451 + s1s452 + s1s453 + s1s454 + s1s455
       = 0;

e268:  - b214 + s1s456 + s1s457 + s1s458 + s1s459 + s1s460 + s1s461 + s1s462
       = 0;

e269:  - b215 + s1s463 + s1s464 + s1s465 + s1s466 + s1s467 + s1s468 + s1s469
       = 0;

e270:  - b216 + s1s470 + s1s471 + s1s472 + s1s473 + s1s474 + s1s475 + s1s476
       = 0;

e271:  - b217 + s1s477 + s1s478 + s1s479 + s1s480 + s1s481 + s1s482 + s1s483
       = 0;

e272:  - b218 + s1s484 + s1s485 + s1s486 + s1s487 + s1s488 + s1s489 + s1s490
       = 0;

e273:  - b219 + s1s491 + s1s492 + s1s493 + s1s494 + s1s495 + s1s496 + s1s497
       = 0;

e274:  - b220 + s1s498 + s1s499 + s1s500 + s1s501 + s1s502 + s1s503 + s1s504
       = 0;

e275:  - b221 + s1s505 + s1s506 + s1s507 + s1s508 + s1s509 + s1s510 + s1s511
       = 0;

e276:  - b222 + s1s512 + s1s513 + s1s514 + s1s515 + s1s516 + s1s517 + s1s518
       = 0;

e277:  - b223 + s1s519 + s1s520 + s1s521 + s1s522 + s1s523 + s1s524 + s1s525
       = 0;

e278:  - b224 + s1s526 + s1s527 + s1s528 + s1s529 + s1s530 + s1s531 + s1s532
       = 0;

e279:  - b225 + s1s533 + s1s534 + s1s535 + s1s536 + s1s537 + s1s538 + s1s539
       = 0;

e280:  - b226 + s1s540 + s1s541 + s1s542 + s1s543 + s1s544 + s1s545 + s1s546
       = 0;

e281:  - b227 + s1s547 + s1s548 + s1s549 + s1s550 + s1s551 + s1s552 + s1s553
       = 0;

e282:  - b228 + s1s554 + s1s555 + s1s556 + s1s557 + s1s558 + s1s559 + s1s560
       = 0;

e283:  - b229 + s1s561 + s1s562 + s1s563 + s1s564 + s1s565 + s1s566 + s1s567
       = 0;

e284:  - b230 + s1s568 + s1s569 + s1s570 + s1s571 + s1s572 + s1s573 + s1s574
       = 0;

e285:  - b231 + s1s575 + s1s576 + s1s577 + s1s578 + s1s579 + s1s580 + s1s581
       = 0;

e286:  - b232 + s1s582 + s1s583 + s1s584 + s1s585 + s1s586 + s1s587 + s1s588
       = 0;

e287:  - b233 + s1s589 + s1s590 + s1s591 + s1s592 + s1s593 + s1s594 + s1s595
       = 0;

e288:  - b234 + s1s596 + s1s597 + s1s598 + s1s599 + s1s600 + s1s601 + s1s602
       = 0;

e289:  - b235 + s1s603 + s1s604 + s1s605 + s1s606 + s1s607 + s1s608 + s1s609
       = 0;

e290:  - b236 + s1s610 + s1s611 + s1s612 + s1s613 + s1s614 + s1s615 + s1s616
       = 0;

e291:  - b237 + s1s617 + s1s618 + s1s619 + s1s620 + s1s621 + s1s622 + s1s623
       = 0;

e292:  - b238 + s1s624 + s1s625 + s1s626 + s1s627 + s1s628 + s1s629 + s1s630
       = 0;

e293:    x1 - 0.0122927400295263*s1s239 - 0.047959606911495*s1s240
       - 0.167824502578647*s1s241 - 0.494468436052303*s1s242
       - 1.92914775438588*s1s243 - 2*s1s244 - 2*s1s245 <= 0;

e294:    x2 - 0.0176041976445841*s1s246 - 0.0686820348432157*s1s247
       - 0.240338257044582*s1s248 - 0.708118780382974*s1s249 - 2*s1s250
       - 2*s1s251 - 2*s1s252 <= 0;

e295:    x3 - 0.0192122784105588*s1s253 - 0.0749558941482069*s1s254
       - 0.262292300976835*s1s255 - 0.772802909347502*s1s256 - 2*s1s257
       - 2*s1s258 - 2*s1s259 <= 0;

e296:    x4 - 0.0139851216849881*s1s260 - 0.0545623625823394*s1s261
       - 0.190929449792929*s1s262 - 0.56254352007505*s1s263 - 2*s1s264
       - 2*s1s265 - 2*s1s266 <= 0;

e297:    x5 - 0.0131870388642087*s1s267 - 0.0514486761076021*s1s268
       - 0.180033762412234*s1s269 - 0.530441095124783*s1s270 - 2*s1s271
       - 2*s1s272 - 2*s1s273 <= 0;

e298:    x6 - 0.0110279676651009*s1s274 - 0.0430251508598196*s1s275
       - 0.15055741709363*s1s276 - 0.443593691162434*s1s277
       - 1.7306620822916*s1s278 - 2*s1s279 - 2*s1s280 <= 0;

e299:    x7 - 0.0137502828767635*s1s281 - 0.0536461488738445*s1s282
       - 0.187723353667753*s1s283 - 0.553097263345606*s1s284 - 2*s1s285
       - 2*s1s286 - 2*s1s287 <= 0;

e300:    x8 - 0.0122927400295263*s1s288 - 0.047959606911495*s1s289
       - 0.167824502578647*s1s290 - 0.494468436052303*s1s291
       - 1.92914775438588*s1s292 - 2*s1s293 - 2*s1s294 <= 0;

e301:    x9 - 0.0153698320860398*s1s295 - 0.0599647518268192*s1s296
       - 0.209833968534382*s1s297 - 0.618242703881818*s1s298 - 2*s1s299
       - 2*s1s300 - 2*s1s301 <= 0;

e302:    x10 - 0.0120532217270583*s1s302 - 0.0470251363535167*s1s303
       - 0.1645545204694*s1s304 - 0.484833949343662*s1s305
       - 1.89155921072266*s1s306 - 2*s1s307 - 2*s1s308 <= 0;

e303:    x11 - 0.0120510911159401*s1s309 - 0.0470168238640743*s1s310
       - 0.164525432670402*s1s311 - 0.484748246730172*s1s312
       - 1.89122484558971*s1s313 - 2*s1s314 - 2*s1s315 <= 0;

e304:    x12 - 0.0142414920290718*s1s316 - 0.0555625806701283*s1s317
       - 0.194429501479406*s1s318 - 0.572855870518057*s1s319 - 2*s1s320
       - 2*s1s321 - 2*s1s322 <= 0;

e305:    x13 - 0.0190758342372385*s1s323 - 0.0744235629590588*s1s324
       - 0.260429520550158*s1s325 - 0.767314520523847*s1s326 - 2*s1s327
       - 2*s1s328 - 2*s1s329 <= 0;

e306:    x14 - 0.0188299954674205*s1s330 - 0.0734644333642121*s1s331
       - 0.257073249355929*s1s332 - 0.757425796631457*s1s333 - 2*s1s334
       - 2*s1s335 - 2*s1s336 <= 0;

e307:    x15 - 0.0176041976445841*s1s337 - 0.0686820348432157*s1s338
       - 0.240338257044582*s1s339 - 0.708118780382974*s1s340 - 2*s1s341
       - 2*s1s342 - 2*s1s343 <= 0;

e308:    x16 - 0.0153698320860398*s1s344 - 0.0599647518268192*s1s345
       - 0.209833968534382*s1s346 - 0.618242703881818*s1s347 - 2*s1s348
       - 2*s1s349 - 2*s1s350 <= 0;

e309:    x17 - 0.0194226083350049*s1s351 - 0.0757764874800376*s1s352
       - 0.265163793814297*s1s353 - 0.781263310246409*s1s354 - 2*s1s355
       - 2*s1s356 - 2*s1s357 <= 0;

e310:    x18 - 0.0174381887671401*s1s358 - 0.0680343582075014*s1s359
       - 0.238071849619242*s1s360 - 0.701441168247406*s1s361 - 2*s1s362
       - 2*s1s363 - 2*s1s364 <= 0;

e311:    x19 - 0.0190758342372385*s1s365 - 0.0744235629590588*s1s366
       - 0.260429520550158*s1s367 - 0.767314520523847*s1s368 - 2*s1s369
       - 2*s1s370 - 2*s1s371 <= 0;

e312:    x20 - 0.0139201415373155*s1s372 - 0.0543088452760314*s1s373
       - 0.190042319589699*s1s374 - 0.559929730804558*s1s375 - 2*s1s376
       - 2*s1s377 - 2*s1s378 <= 0;

e313:    x21 - 0.0150776355652448*s1s379 - 0.0588247594211735*s1s380
       - 0.205844806180028*s1s381 - 0.606489265973719*s1s382 - 2*s1s383
       - 2*s1s384 - 2*s1s385 <= 0;

e314:    x22 - 0.0192122784105588*s1s386 - 0.0749558941482069*s1s387
       - 0.262292300976835*s1s388 - 0.772802909347502*s1s389 - 2*s1s390
       - 2*s1s391 - 2*s1s392 <= 0;

e315:    x23 - 0.0120532217270583*s1s393 - 0.0470251363535167*s1s394
       - 0.1645545204694*s1s395 - 0.484833949343662*s1s396
       - 1.89155921072266*s1s397 - 2*s1s398 - 2*s1s399 <= 0;

e316:    x24 - 0.0194226083350049*s1s400 - 0.0757764874800376*s1s401
       - 0.265163793814297*s1s402 - 0.781263310246409*s1s403 - 2*s1s404
       - 2*s1s405 - 2*s1s406 <= 0;

e317:    x25 - 0.0197779487583483*s1s407 - 0.0771628331590627*s1s408
       - 0.270015017353593*s1s409 - 0.795556675515238*s1s410 - 2*s1s411
       - 2*s1s412 - 2*s1s413 <= 0;

e318:    x26 - 0.015876050278038*s1s414 - 0.0619397407586086*s1s415
       - 0.216745024658915*s1s416 - 0.638605041090392*s1s417 - 2*s1s418
       - 2*s1s419 - 2*s1s420 <= 0;

e319:    x27 - 0.0114959997704606*s1s421 - 0.0448511583846756*s1s422
       - 0.156947144289045*s1s423 - 0.462420014879005*s1s424
       - 1.80411219047469*s1s425 - 2*s1s426 - 2*s1s427 <= 0;

e320:    x28 - 0.0122577120780475*s1s428 - 0.0478229468357263*s1s429
       - 0.167346289542402*s1s430 - 0.493059456740591*s1s431
       - 1.92365068100974*s1s432 - 2*s1s433 - 2*s1s434 <= 0;

e321:    x29 - 0.0139851216849881*s1s435 - 0.0545623625823394*s1s436
       - 0.190929449792929*s1s437 - 0.56254352007505*s1s438 - 2*s1s439
       - 2*s1s440 - 2*s1s441 <= 0;

e322:    x30 - 0.0120510911159401*s1s442 - 0.0470168238640743*s1s443
       - 0.164525432670402*s1s444 - 0.484748246730172*s1s445
       - 1.89122484558971*s1s446 - 2*s1s447 - 2*s1s448 <= 0;

e323:    x31 - 0.0174381887671401*s1s449 - 0.0680343582075014*s1s450
       - 0.238071849619242*s1s451 - 0.701441168247406*s1s452 - 2*s1s453
       - 2*s1s454 - 2*s1s455 <= 0;

e324:    x32 - 0.0197779487583483*s1s456 - 0.0771628331590627*s1s457
       - 0.270015017353593*s1s458 - 0.795556675515238*s1s459 - 2*s1s460
       - 2*s1s461 - 2*s1s462 <= 0;

e325:    x33 - 0.02056968839856*s1s463 - 0.0802517719822704*s1s464
       - 0.280824105561038*s1s465 - 0.827403949655566*s1s466 - 2*s1s467
       - 2*s1s468 - 2*s1s469 <= 0;

e326:    x34 - 0.0123243005973977*s1s470 - 0.0480827391363186*s1s471
       - 0.168255377761185*s1s472 - 0.495737941841801*s1s473
       - 1.93410067769589*s1s474 - 2*s1s475 - 2*s1s476 <= 0;

e327:    x35 - 0.0114258635818418*s1s477 - 0.0445775250020163*s1s478
       - 0.155989622130483*s1s479 - 0.459598826810784*s1s480
       - 1.7931054441797*s1s481 - 2*s1s482 - 2*s1s483 <= 0;

e328:    x36 - 0.0131870388642087*s1s484 - 0.0514486761076021*s1s485
       - 0.180033762412234*s1s486 - 0.530441095124783*s1s487 - 2*s1s488
       - 2*s1s489 - 2*s1s490 <= 0;

e329:    x37 - 0.0142414920290718*s1s491 - 0.0555625806701283*s1s492
       - 0.194429501479406*s1s493 - 0.572855870518057*s1s494 - 2*s1s495
       - 2*s1s496 - 2*s1s497 <= 0;

e330:    x38 - 0.0190758342372385*s1s498 - 0.0744235629590588*s1s499
       - 0.260429520550158*s1s500 - 0.767314520523847*s1s501 - 2*s1s502
       - 2*s1s503 - 2*s1s504 <= 0;

e331:    x39 - 0.015876050278038*s1s505 - 0.0619397407586086*s1s506
       - 0.216745024658915*s1s507 - 0.638605041090392*s1s508 - 2*s1s509
       - 2*s1s510 - 2*s1s511 <= 0;

e332:    x40 - 0.02056968839856*s1s512 - 0.0802517719822704*s1s513
       - 0.280824105561038*s1s514 - 0.827403949655566*s1s515 - 2*s1s516
       - 2*s1s517 - 2*s1s518 <= 0;

e333:    x41 - 0.0153698320860398*s1s519 - 0.0599647518268192*s1s520
       - 0.209833968534382*s1s521 - 0.618242703881818*s1s522 - 2*s1s523
       - 2*s1s524 - 2*s1s525 <= 0;

e334:    x42 - 0.012508886937106*s1s526 - 0.048802894957753*s1s527
       - 0.170775410770095*s1s528 - 0.503162821770358*s1s529
       - 1.96306853367294*s1s530 - 2*s1s531 - 2*s1s532 <= 0;

e335:    x43 - 0.0110279676651009*s1s533 - 0.0430251508598196*s1s534
       - 0.15055741709363*s1s535 - 0.443593691162434*s1s536
       - 1.7306620822916*s1s537 - 2*s1s538 - 2*s1s539 <= 0;

e336:    x44 - 0.0190758342372385*s1s540 - 0.0744235629590588*s1s541
       - 0.260429520550158*s1s542 - 0.767314520523847*s1s543 - 2*s1s544
       - 2*s1s545 - 2*s1s546 <= 0;

e337:    x45 - 0.0139201415373155*s1s547 - 0.0543088452760314*s1s548
       - 0.190042319589699*s1s549 - 0.559929730804558*s1s550 - 2*s1s551
       - 2*s1s552 - 2*s1s553 <= 0;

e338:    x46 - 0.0114959997704606*s1s554 - 0.0448511583846756*s1s555
       - 0.156947144289045*s1s556 - 0.462420014879005*s1s557
       - 1.80411219047469*s1s558 - 2*s1s559 - 2*s1s560 <= 0;

e339:    x47 - 0.0123243005973977*s1s561 - 0.0480827391363186*s1s562
       - 0.168255377761185*s1s563 - 0.495737941841801*s1s564
       - 1.93410067769589*s1s565 - 2*s1s566 - 2*s1s567 <= 0;

e340:    x48 - 0.0153698320860398*s1s568 - 0.0599647518268192*s1s569
       - 0.209833968534382*s1s570 - 0.618242703881818*s1s571 - 2*s1s572
       - 2*s1s573 - 2*s1s574 <= 0;

e341:    x49 - 0.0134441259722562*s1s575 - 0.0524516906197741*s1s576
       - 0.183543599594492*s1s577 - 0.540782276988236*s1s578 - 2*s1s579
       - 2*s1s580 - 2*s1s581 <= 0;

e342:    x50 - 0.0137502828767635*s1s582 - 0.0536461488738445*s1s583
       - 0.187723353667753*s1s584 - 0.553097263345606*s1s585 - 2*s1s586
       - 2*s1s587 - 2*s1s588 <= 0;

e343:    x51 - 0.0188299954674205*s1s589 - 0.0734644333642121*s1s590
       - 0.257073249355929*s1s591 - 0.757425796631457*s1s592 - 2*s1s593
       - 2*s1s594 - 2*s1s595 <= 0;

e344:    x52 - 0.0150776355652448*s1s596 - 0.0588247594211735*s1s597
       - 0.205844806180028*s1s598 - 0.606489265973719*s1s599 - 2*s1s600
       - 2*s1s601 - 2*s1s602 <= 0;

e345:    x53 - 0.0122577120780475*s1s603 - 0.0478229468357263*s1s604
       - 0.167346289542402*s1s605 - 0.493059456740591*s1s606
       - 1.92365068100974*s1s607 - 2*s1s608 - 2*s1s609 <= 0;

e346:    x54 - 0.0114258635818418*s1s610 - 0.0445775250020163*s1s611
       - 0.155989622130483*s1s612 - 0.459598826810784*s1s613
       - 1.7931054441797*s1s614 - 2*s1s615 - 2*s1s616 <= 0;

e347:    x55 - 0.012508886937106*s1s617 - 0.048802894957753*s1s618
       - 0.170775410770095*s1s619 - 0.503162821770358*s1s620
       - 1.96306853367294*s1s621 - 2*s1s622 - 2*s1s623 <= 0;

e348:    x56 - 0.0134441259722562*s1s624 - 0.0524516906197741*s1s625
       - 0.183543599594492*s1s626 - 0.540782276988236*s1s627 - 2*s1s628
       - 2*s1s629 - 2*s1s630 <= 0;

e349:    b183 + b190 <= 1;

e350:    b184 + b197 <= 1;

e351:    b185 + b204 <= 1;

e352:    b186 + b211 <= 1;

e353:    b187 + b218 <= 1;

e354:    b188 + b225 <= 1;

e355:    b189 + b232 <= 1;

e356:    b191 + b198 <= 1;

e357:    b192 + b205 <= 1;

e358:    b193 + b212 <= 1;

e359:    b194 + b219 <= 1;

e360:    b195 + b226 <= 1;

e361:    b196 + b233 <= 1;

e362:    b199 + b206 <= 1;

e363:    b200 + b213 <= 1;

e364:    b201 + b220 <= 1;

e365:    b202 + b227 <= 1;

e366:    b203 + b234 <= 1;

e367:    b207 + b214 <= 1;

e368:    b208 + b221 <= 1;

e369:    b209 + b228 <= 1;

e370:    b210 + b235 <= 1;

e371:    b215 + b222 <= 1;

e372:    b216 + b229 <= 1;

e373:    b217 + b236 <= 1;

e374:    b223 + b230 <= 1;

e375:    b224 + b237 <= 1;

e376:    b231 + b238 <= 1;

e377:    b183 + b184 + b185 + b186 + b187 + b188 + b189 >= 2;

e378:    b183 + b191 + b192 + b193 + b194 + b195 + b196 >= 2;

e379:    b184 + b191 + b199 + b200 + b201 + b202 + b203 >= 2;

e380:    b185 + b192 + b199 + b207 + b208 + b209 + b210 >= 2;

e381:    b186 + b193 + b200 + b207 + b215 + b216 + b217 >= 2;

e382:    b187 + b194 + b201 + b208 + b215 + b223 + b224 >= 2;

e383:    b188 + b195 + b202 + b209 + b216 + b223 + b231 >= 2;

e384:    b189 + b196 + b203 + b210 + b217 + b224 + b231 >= 2;
