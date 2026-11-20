// Select cap: HOPPER_L/HOPPER_R/HOOK_L/HOOK_R/OK or all. Textless fit caps.
$fn=48;which="all";module cap(rel=false){union(){cylinder(d=rel?13.6:13.2,h=2.3);translate([0,0,2.3])cylinder(d=rel?10.8:10.5,h=4.7);}}
if(which=="HOPPER_L"||which=="HOPPER_R"||which=="HOOK_L"||which=="HOOK_R")cap(true);else if(which=="OK")cap();else for(i=[0:15])translate([(i%8)*18,floor(i/8)*20,0])cap(i<4);
