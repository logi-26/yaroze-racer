@echo off
echo Converting all assets...

call track\grass_1\CONV2.BAT
call track\sand_1\CONV2.BAT
call track\stone1\CONV2.BAT

call track\blank\CONV2.BAT
call track\grid\CONV2.BAT
call track\line\CONV2.BAT
call track\st_01\CONV2.BAT
call track\st_1\CONV2.BAT
call track\t_00\CONV2.BAT
call track\t_01\CONV2.BAT
call track\t_02\CONV2.BAT
call track\t_1\CONV2.BAT

call barrier1\CONV2.BAT
call barrier2\CONV2.BAT

call sign1\CONV2.BAT
call stand\CONV2.BAT

call build1\CONV2.BAT
call build2\CONV2.BAT

call car2\blue\CONV2.BAT
call car3\yellow\CONV2.BAT
call car5\green\CONV2.BAT

echo Done.