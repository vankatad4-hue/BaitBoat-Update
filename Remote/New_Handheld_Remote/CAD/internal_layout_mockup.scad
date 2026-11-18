// Flytec New Handheld Remote — Internal placement volume mockup R1
// Visual-only placeholder volumes. NOT component footprints or assembly-ready CAD.
// View from the front with the front panel removed. Units: mm.
$fn=48;
W=310; H=190; D=62; wall=3;
color([0.35,0.38,0.42,0.12])
difference(){ cube([W-2*wall,H-2*wall,D-wall],center=true); cube([W-2*wall-2,H-2*wall-2,D-wall+1],center=true); }
module board_box(x,y,z,w,h,t,c){ translate([x,y,z]) color(c) cube([w,h,t],center=true); }
// Main controller board — placeholder envelope 80 x 50 x 10
board_box(0,48,0,80,50,10,[0.05,0.55,0.25]);
// Display/interface PCB — placeholder envelope 130 x 80 x 8
board_box(0,-3,-12,130,80,8,[0.15,0.35,0.9]);
// Radio module — left upper RF zone, placeholder 32 x 22 x 12
board_box(-116,66,-7,32,22,12,[0.95,0.55,0.08]);
// Power/charging board — lower-right service zone, placeholder 55 x 35 x 10
board_box(86,-68,-7,55,35,10,[0.9,0.2,0.2]);
// Input/cable zones — placeholder strips
board_box(-120,-35,-6,24,68,4,[0.55,0.3,0.75]);
board_box(120,-35,-6,24,68,4,[0.55,0.3,0.75]);
// Four 18650 cell envelopes, 2x2 lower-central; each 65 x 18 mm
module cell(x,y,z){ translate([x,y,z]) rotate([0,90,0]) color([0.15,0.15,0.18]) cylinder(h=65,d=18,center=true); }
cell(-16,-67,-20); cell(16,-67,-20); cell(-16,-67,0); cell(16,-67,0);
// Antenna connector stubs
translate([-125,82,-3]) color([0.7,0.7,0.7]) cylinder(h=16,d=8,center=true);
translate([125,82,-3]) color([0.7,0.7,0.7]) cylinder(h=16,d=8,center=true);
// USB-C connector placeholder
translate([0,-91,-8]) color([0.5,0.5,0.5]) cube([14,8,9],center=true);
