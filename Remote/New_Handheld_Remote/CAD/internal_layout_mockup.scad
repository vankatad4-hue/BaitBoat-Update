// Flytec Remote internal placement mockup R2 — enlarged battery reserve
// Units mm. Visual-only envelopes, not validated component CAD.
$fn=48;
W=310; H=190; D=70; wall=3;
color([0.35,0.38,0.42,0.10])
difference(){ cube([W-2*wall,H-2*wall,D-wall],center=true); cube([W-2*wall-2,H-2*wall-2,D-wall+1],center=true); }
module box(x,y,z,w,h,t,c){translate([x,y,z]) color(c) cube([w,h,t],center=true);}
// Integrated touchscreen/controller envelope moved upward to reserve lower space
box(0,17,-22,130,80,12,[0.12,0.35,0.9]);
// Radio near Antenna A
box(-116,65,-12,32,22,12,[0.95,0.55,0.08]);
// Charging/power zone near USB service side
box(86,-66,-12,55,35,10,[0.9,0.2,0.2]);
// Dedicated larger lower-central battery reserve: 104 x 82 x 24 mm (placeholder)
box(0,-57,-17,104,82,24,[0.2,0.55,0.25]);
// Four 18650 envelopes inside reserve, 65 mm cell axes along Y
module cell(x,y,z){translate([x,y,z]) rotate([90,0,0]) color([0.10,0.12,0.14]) cylinder(h=65,d=18,center=true);}
cell(-27,-55,-16); cell(-9,-55,-16); cell(9,-55,-16); cell(27,-55,-16);
// Side input/cable zones, illustrative
box(-125,-20,-12,16,68,4,[0.55,0.3,0.75]);
box(125,-20,-12,16,68,4,[0.55,0.3,0.75]);
// Antenna connectors and USB-C placeholder
translate([-125,82,-5]) color([0.7,0.7,0.7]) cylinder(h=16,d=8,center=true);
translate([125,82,-5]) color([0.7,0.7,0.7]) cylinder(h=16,d=8,center=true);
translate([0,-91,-10]) color([0.5,0.5,0.5]) cube([14,8,9],center=true);
