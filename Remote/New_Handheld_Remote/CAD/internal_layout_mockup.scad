// Flytec Remote internal placement mockup R3 — component envelope integration
// Units mm; conceptual arrangement only. Verify exact purchased parts before fabrication.
$fn=48;
W=310; H=190; D=70;
module envelope(x,y,z,w,h,t,c){translate([x,y,z]) color(c) cube([w,h,t],center=true);}
module joystick(x,y,z){translate([x,y,z]) color([0.95,0.62,0.08]) cube([21,17,19.45],center=true);}
// Display/controller board envelope — Waveshare ESP32-S3-Touch-LCD-4.3 candidate
envelope(0,5,-12,106.1,67.8,8.8,[0.15,0.35,0.9]);
// Joysticks — C&K THB001P candidate, mirrored
joystick(-92,72,-15); joystick(92,72,-15);
// 16 physical switch/cap placeholders, 2 x 4 per bank; pitch ~21 mm
module button(x,y,z){envelope(x,y,z,15,12,7,[0.16,0.16,0.18]);}
for(row=[0:3]) for(col=[0:1]){
  button(-108+col*19,25-row*21,-8);
  button(108-col*19,25-row*21,-8);
}
// Battery reserve and cell envelope: four 18650 in parallel
envelope(0,-56,-18,104,82,24,[0.2,0.55,0.25]);
module cell(x,y,z){translate([x,y,z]) rotate([90,0,0]) color([0.08,0.1,0.12]) cylinder(h=65,d=18.6,center=true);}
cell(-27,-55,-18); cell(-9,-55,-18); cell(9,-55,-18); cell(27,-55,-18);
// Radio module candidate envelope 45 x 16 x 12, near left antenna
envelope(-123,66,-10,45,16,12,[0.95,0.55,0.08]);
// Charger/power PCB envelope 50 x 35 x 10, near USB-C service edge
envelope(92,-73,-12,50,35,10,[0.9,0.2,0.2]);
// USB-C connector placeholder
translate([0,-91,-10]) color([0.5,0.5,0.5]) cube([14,8,9],center=true);
