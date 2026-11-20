// Simple text-free caps; names identify print channel. Prototype dimensions.
$fn=48;which="all";
module cap(release=false){union(){cylinder(d=release?13.6:13.2,h=2.3);translate([0,0,2.3])cylinder(d=release?10.8:10.5,h=4.7);}}
if(which=="HOPPER_L")cap(true);else if(which=="HOPPER_R")cap(true);else if(which=="HOOK_L")cap(true);else if(which=="HOOK_R")cap(true);else if(which=="OK")cap(false);else if(which=="utility")cap(false);else for(i=[0:15])translate([(i%8)*18,floor(i/8)*20,0])cap(i<4);
