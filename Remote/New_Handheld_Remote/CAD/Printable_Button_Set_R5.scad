// Fast, text-free multi-button cap plate. 16 button caps, four release-key caps larger.
$fn=32;module cap(rel=false){union(){cylinder(d=rel?13.6:13.2,h=2.3);translate([0,0,2.3])cylinder(d=rel?10.8:10.5,h=4.7);}}
for(i=[0:15])translate([(i%8)*18,floor(i/8)*20,0])cap(i<4);
