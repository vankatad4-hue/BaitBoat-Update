// Separate engraved caps for the four release buttons and OK. Prototype fit.
$fn=64;which="all";
module cap(label){difference(){union(){cylinder(d=14,h=2.5);translate([0,0,2.5])cylinder(d=10.5,h=5);}translate([0,0,.35])linear_extrude(.45)text(label,size=2.1,halign="center",valign="center",font="Liberation Sans:style=Bold");}}
if(which=="HOPPER_L")cap("H-L");else if(which=="HOPPER_R")cap("H-R");else if(which=="HOOK_L")cap("K-L");else if(which=="HOOK_R")cap("K-R");else if(which=="OK")cap("OK");else if(which=="utility")cap("F");else{for(i=[0:3])translate([i*18,0,0])cap(["H-L","H-R","K-L","K-R"][i]);translate([0,22,0])cap("OK");for(i=[0:7])translate([i*18,44,0])cap("F");}
