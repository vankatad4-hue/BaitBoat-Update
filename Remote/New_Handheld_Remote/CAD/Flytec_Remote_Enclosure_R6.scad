// Flytec V080 ergonomic remote R6 - parametric redesign
// Confirmed front layout: TWO button banks, each 4 columns x 2 rows.
// Styling based on repository CONCEPT/Flytec_Remote_Concept_Reference.jpg.
// mm. Prototype only; dimensions and hardware fit not validated.
// part="front" | "rear" | "buttons" | "assembly"
part="assembly"; $fn=72;

// Envelope starting target. Shell body depth excludes antenna stalks.
W=310; H=209; body_depth=66.5; front_th=3.5; wall=3.0;
screen_w=108; screen_h=72; screen_y=7;
stick_x=108; stick_y=66; stick_open=25;
power_d=18; power_y=55;
button_d=14.2; pitch_x=19.5; pitch_y=21.0;
bank_center_x=108; bank_center_y=-31;
antenna_d=8.6; antenna_x=112; antenna_y=94;
usb_w=15.5; usb_h=8.5;
screw_positions=[[-137,53],[137,53],[-137,-51],[137,-51],[-76,-91],[76,-91]];

// Controller plan silhouette: broad upper shoulder and flared hand grips.
module outline2d(){
  offset(r=8) polygon(points=[
    [-143,-48],[-151,-27],[-151,18],[-143,44],[-130,65],
    [-111,82],[-91,91],[-70,88],[-50,73],[-31,62],
    [31,62],[50,73],[70,88],[91,91],[111,82],[130,65],
    [143,44],[151,18],[151,-27],[143,-48],[130,-69],
    [114,-86],[95,-99],[75,-96],[55,-82],[35,-67],
    [-35,-67],[-55,-82],[-75,-96],[-95,-99],[-114,-86],[-130,-69]
  ]);
}
module body_solid(z){linear_extrude(height=z) outline2d();}
module bank_centres(side){
  // Four columns spread across each side wing, two rows.
  for(col=[0:3],row=[0:1])
    translate([side*(bank_center_x-29.25+col*pitch_x),
               bank_center_y-row*pitch_y,0]) children();
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
    bank_centres(-1) translate([0,0,-.1]) cylinder(d=button_d,h=front_th+.2);
    bank_centres(1) translate([0,0,-.1]) cylinder(d=button_d,h=front_th+.2);
    translate([0,power_y,-.1]) cylinder(d=power_d,h=front_th+.2);
    // Lower-center USB-C opening.
    translate([0,-H/2-.1,front_th/2]) cube([usb_w,usb_h,front_th+1],center=true);
    screw_holes();
  }
}
module rear_shell(){
  difference(){
    body_solid(body_depth);
    // Interior cavity opens toward the front panel at z=body_depth.
    translate([0,0,wall]) linear_extrude(height=body_depth-wall+.3)
      offset(r=5) scale([0.974,0.974]) outline2d();
    // Rear antenna bulkhead pass-throughs.
    antenna_centres() translate([0,0,body_depth-wall-.1]) cylinder(d=antenna_d,h=wall+.3);
    // USB port tunnel through bottom perimeter.
    translate([0,-H/2-.1,body_depth/2]) cube([usb_w,usb_h,wall+2],center=true);
    // ventilation slots in lower rear (avoid battery zone as layout changes)
    for(i=[-2:2]) translate([i*9,-78,body_depth-wall-.1]) cube([3,18,wall+.3],center=true);
  }
  // Screw bosses and generic internal supports. Do not imply final board fit.
  for(p=screw_positions) translate([p[0],p[1],wall]) difference(){
    cylinder(d=9.5,h=9);
    translate([0,0,-.1]) cylinder(d=2.8,h=9.2);
  }
  // Raised screen-support ledges at four corners of aperture.
  for(x=[-screen_w/2+7,screen_w/2-7],y=[screen_y-screen_h/2+6,screen_y+screen_h/2-6])
    translate([x,y,wall]) cube([8,8,4],center=true);
  // Generic joystick mounting rings (dimension placeholder).
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
}
module assembly_preview(){
  color("#1d2329") rear_shell();
  translate([0,0,body_depth+.25]) color("#30363d") front_panel();
  // Glass and joystick placeholder discs for visual fit only.
  translate([0,screen_y,body_depth+front_th+.1]) color("#0a1822") cube([screen_w-3,screen_h-3,1],center=true);
  joystick_centres() translate([0,0,body_depth+front_th+.1]) color("#303840") cylinder(d=21,h=9);
  bank_centres(-1) translate([0,0,body_depth+front_th+.1]) color("#c7c9cb") button_cap(false);
  // Right-bank top row is four releases; bottom row are ordinary utility keys.
  for(col=[0:3]) translate([antenna_x-29.25+col*pitch_x,bank_center_y,body_depth+front_th+.1]) color("#bd4c43") button_cap(true);
  for(col=[0:3]) translate([antenna_x-29.25+col*pitch_x,bank_center_y-pitch_y,body_depth+front_th+.1]) color("#c7c9cb") button_cap(false);
  translate([0,power_y,body_depth+front_th+.1]) color("#4e92cc") cylinder(d=15,h=5);
}
if(part=="front") front_panel();
else if(part=="rear") rear_shell();
else if(part=="buttons") button_set();
else assembly_preview();
