// Flytec V080 ergonomic remote R6 - parametric redesign
// Updated front layout: TWO button banks, each 2 columns x 4 rows.
// Styling based on repository CONCEPT/Flytec_Remote_Concept_Reference.jpg.
// R6.1 prototype. Component dimensions/fit remain unverified; local mesh tests passed.
// part="front" | "rear" | "buttons" | "assembly"
part="assembly"; $fn=72;

// Envelope starting target. Shell body depth excludes antenna stalks.
W=310; H=209; body_depth=66.5; front_th=3.5; wall=3.0;
screen_w=108; screen_h=72; screen_y=7;
stick_x=108; stick_y=66; stick_open=25;
power_d=18; power_y=55; confirm_d=16; confirm_y=-62;
button_d=14.2; pitch_x=19.5; pitch_y=20.5;
bank_center_x=106; bank_center_y=-10;
antenna_d=8.6; antenna_x=112; antenna_y=82;
usb_w=15.5; usb_h=8.5;
screw_positions=[[-137,53],[137,53],[-137,-51],[137,-51],[-76,-91],[76,-91]];

// Controller plan silhouette: broad upper shoulder and flared hand grips.
module base_outline2d(){
  polygon(points=[
    [-143,-48],[-151,-27],[-151,18],[-143,44],[-130,65],
    [-111,82],[-91,91],[-70,88],[-50,73],[-31,62],
    [31,62],[50,73],[70,88],[91,91],[111,82],[130,65],
    [143,44],[151,18],[151,-27],[143,-48],[130,-69],
    [114,-86],[95,-99],[75,-96],[55,-86],[35,-86],
    [-35,-86],[-55,-86],[-75,-96],[-95,-99],[-114,-86],[-130,-69]
  ]);
}
module outline2d(){scale([W/318,H/206]) offset(r=8) base_outline2d();}
module body_solid(z){linear_extrude(height=z) outline2d();}
module bank_centres(side){
  // Two columns across each side wing, four rows vertically.
  for(col=[0:1],row=[0:3])
    translate([side*(bank_center_x+(col-0.5)*pitch_x),
               bank_center_y-row*pitch_y,0]) children();
}
module release_centres(){
  // Four guarded bait/hook release buttons occupy the top 2x2 positions of the right bank.
  for(col=[0:1],row=[0:1])
    translate([bank_center_x+(col-0.5)*pitch_x,bank_center_y-row*pitch_y,0]) children();
}
module joystick_centres(){
  for(side=[-1,1]) translate([side*stick_x,stick_y,0]) children();
}
module antenna_centres(){
  for(side=[-1,1]) translate([side*antenna_x,antenna_y,0]) children();
}
module screw_holes(){
  for(p=screw_positions) translate([p[0],p[1],-.1]) cylinder(d=3.3,h=front_th+.2);
}
module front_panel(){
  difference(){
    body_solid(front_th);
    // Screen opening. Check exact bezel/PCB aperture after purchasing display.
    translate([0,screen_y,-.1]) cube([screen_w,screen_h,front_th+.2],center=true);
    joystick_centres() translate([0,0,-.1]) cylinder(d=stick_open,h=front_th+.2);
    // Left bank: all 8 standard buttons. Right bank: lower 4 standard buttons.
    bank_centres(-1) translate([0,0,-.1]) cylinder(d=button_d,h=front_th+.2);
    for(col=[0:1],row=[2:3])
      translate([bank_center_x+(col-0.5)*pitch_x,bank_center_y-row*pitch_y,-.1]) cylinder(d=button_d,h=front_th+.2);
    // Right bank top 2x2: guarded bait/hook release buttons.
    release_centres() translate([0,0,-.1]) cylinder(d=18.0,h=front_th+.2);
    translate([0,confirm_y,-.1]) cylinder(d=confirm_d,h=front_th+.2);
    translate([0,power_y,-.1]) cylinder(d=power_d,h=front_th+.2);
    // USB-C opening is routed through the rear/bottom bridge, not the front panel.

    screw_holes();
  }
  // Raised safety collars around bait/hook buttons.
  release_centres() translate([0,0,front_th]) difference(){
    cylinder(d=21.0,h=2.0);
    translate([0,0,-.1]) cylinder(d=17.6,h=2.2);
  }
}
module grip_pads(){
  for(side=[-1,1]) hull(){
    translate([side*118,-47,-4.0]) scale([1.0,1.15,0.72]) sphere(d=28);
    translate([side*121,-76,-4.0]) scale([1.0,1.15,0.72]) sphere(d=30);
    translate([side*116,-93,-4.0]) scale([1.0,1.05,0.72]) sphere(d=24);
  }
}
module battery_cradle(){
  // Four 18650 envelopes: two in each grip, stacked in depth.
  // Cells run along Y. Supports are placeholders; restraint and pack wiring are not designed.
  for(side=[-1,1]){
    translate([side*battery_grip_x,battery_y,wall+2]) cube([28,cell_len+10,3],center=true);
    for(layer=[0:1],yoff=[-cell_len/2+8,cell_len/2-8])
      difference(){
        translate([side*battery_grip_x,battery_y+yoff,wall+battery_saddle_z+layer*battery_depth_pitch-4]) cube([28,12,12],center=true);
        translate([side*battery_grip_x,battery_y+yoff,wall+battery_saddle_z+layer*battery_depth_pitch]) rotate([90,0,0]) cylinder(d=cell_d+1.0,h=14,center=true);
      }
    for(layer=[0:1],yoff=[-cell_len/2+8,cell_len/2-8],xoff=[-10,10])
      translate([side*battery_grip_x+xoff,battery_y+yoff,wall+7+layer*battery_depth_pitch]) cube([5,5,14+layer*6],center=true);
    for(xoff=[-14,14]) translate([side*battery_grip_x+xoff,battery_y,wall+7]) cube([3,cell_len+8,34],center=true);
  }
}
module carrier_mounts(){
  for(x=[-carrier_w/2+6,carrier_w/2-6],y=[carrier_y-carrier_h/2+6,carrier_y+carrier_h/2-6])
    translate([x,y,wall]) difference(){
      cylinder(d=10,h=carrier_standoff_h);
      translate([0,0,-.1]) cylinder(d=3.0,h=carrier_standoff_h+.2);
    }
}
module rear_shell(){
  difference(){
    body_solid(body_depth);
    // Interior cavity opens toward the front panel at z=body_depth.
    translate([0,0,wall]) linear_extrude(height=body_depth-wall+.3)
      scale([W/318,H/206]) offset(r=5) base_outline2d();
    // Rear antenna bulkhead pass-throughs.
    antenna_centres() translate([0,0,body_depth-wall-.1]) cylinder(d=antenna_d,h=wall+.3);
    // USB port tunnel through bottom perimeter.
    translate([0,-86,body_depth/2]) cube([usb_w,usb_h,wall+2],center=true);
    // ventilation slots in lower rear (avoid battery zone as layout changes)
    for(i=[-2:2]) translate([i*9,-78,body_depth-wall-.1]) cube([3,18,wall+.3],center=true);
  }
  // Screw bosses and internal component supports (fit remains provisional).
  for(p=screw_positions) translate([p[0],p[1],wall]) difference(){
    cylinder(d=9.5,h=9);
    translate([0,0,-.1]) cylinder(d=2.8,h=9.2);
  }
  // Raised screen-support ledges at four corners of aperture.
  for(x=[-screen_w/2+7,screen_w/2-7],y=[screen_y-screen_h/2+6,screen_y+screen_h/2-6])
    translate([x,y,wall]) cube([8,8,4],center=true);
  carrier_mounts();\n  battery_cradle();\n  grip_pads();\n  // Generic joystick mounting rings (dimension placeholder).
  joystick_centres() translate([0,0,wall]) difference(){
    cylinder(d=34,h=4);
    translate([0,0,-.1]) cylinder(d=26,h=4.2);
  }
}
module button_cap(release=false){
  union(){
    cylinder(d=release?13.8:13.3,h=2.4);
    translate([0,0,2.4]) cylinder(d=release?10.8:10.6,h=4.5);
  }
}
module button_set(){
  // Separate print plate: 12 ordinary caps + 4 safety-release caps.
  for(i=[0:11]) translate([(i%6)*19, floor(i/6)*20,0]) button_cap(false);
  for(i=[0:3]) translate([i*19,45,0]) button_cap(true);
  translate([76,45,0]) button_cap(false); // separate OK/confirm key
}
module internal_preview(){\n  color("#242a30") rear_shell();\n  translate([0,carrier_y,wall+carrier_standoff_h+1]) color("#287b48") cube([carrier_w,carrier_h,1.6],center=true);\n  translate([0,screen_y,body_depth-25]) color("#235b9a") cube([screen_w+4,screen_h+4,2],center=true);\n  for(side=[-1,1],layer=[0:1]) translate([side*battery_grip_x,battery_y,wall+battery_saddle_z+layer*battery_depth_pitch]) rotate([90,0,0]) color("#d5a23c") cylinder(d=cell_d,h=cell_len,center=true);\n  joystick_centres() translate([0,stick_y-12,wall+14]) color("#3c6d85") cube([27,30,4],center=true);\n  translate([-90,antenna_y-6,wall+14]) color("#6d5a9d") cube([32,24,4],center=true);\n  translate([90,antenna_y-6,wall+14]) color("#6d5a9d") cube([32,24,4],center=true);\n}\nmodule assembly_preview(){
  color("#1d2329") rear_shell();
  translate([0,0,body_depth+.25]) color("#30363d") front_panel();
  // Glass and joystick placeholder discs for visual fit only.
  translate([0,screen_y,body_depth+front_th+.1]) color("#0a1822") cube([screen_w-3,screen_h-3,1],center=true);
  joystick_centres() translate([0,0,body_depth+front_th+.1]) color("#303840") cylinder(d=21,h=9);
  bank_centres(-1) translate([0,0,body_depth+front_th+.1]) color("#c7c9cb") button_cap(false);
  // Right bank 2x2 guarded release keys on top, 2x2 utility keys below.
  release_centres() translate([0,0,body_depth+front_th+.1]) color("#bd4c43") button_cap(true);
  for(col=[0:1],row=[2:3]) translate([bank_center_x+(col-0.5)*pitch_x,bank_center_y-row*pitch_y,body_depth+front_th+.1]) color("#c7c9cb") button_cap(false);
  translate([0,power_y,body_depth+front_th+.1]) color("#4e92cc") cylinder(d=15,h=5);
  translate([0,confirm_y,body_depth+front_th+.1]) color("#d8dadd") button_cap(false);
}
if(part=="front") front_panel();
else if(part=="rear") rear_shell();
else if(part=="buttons") button_set();
else assembly_preview();
