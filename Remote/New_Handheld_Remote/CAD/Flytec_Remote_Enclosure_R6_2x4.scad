// Flytec V080 Remote R6.2 — button-layout correction and clean preview source
// Confirmed bank arrangement: 2 columns x 4 rows EACH side (16 total).
// Right-bank upper 2x2 are guarded bait/hook releases. Lower 2x2 are auxiliary.
// Dimensions in mm. Fit and electronics remain provisional.
part="front"; $fn=64;
W=310; H=209; front_th=3.5;
screen_w=108; screen_h=72; screen_y=7;
stick_x=108; stick_y=66; stick_hole=25;
button_d=14.2; pitch_x=19.5; pitch_y=20.5; bank_x=106; bank_y=-10;
release_d=18; collar_od=21; collar_h=2;
ok_d=16; ok_y=-72;
antenna_x=112; antenna_y=82; antenna_d=8.6;
usb_w=15.5; usb_h=8.5;
outline_pts=[[-143,-48],[-151,-27],[-151,18],[-143,44],[-130,65],[-111,82],[-91,91],[-70,88],[-50,73],[-31,62],[31,62],[50,73],[70,88],[91,91],[111,82],[130,65],[143,44],[151,18],[151,-27],[143,-48],[130,-69],[114,-86],[95,-99],[75,-96],[55,-86],[35,-86],[-35,-86],[-55,-86],[-75,-96],[-95,-99],[-114,-86],[-130,-69]];
module outline(){scale([W/318,H/206]) offset(r=8) polygon(outline_pts);}
module shell(h){linear_extrude(h) outline();}
module left_bank(){for(c=[0:1],r=[0:3]) translate([-bank_x+(c-.5)*pitch_x,bank_y-r*pitch_y,0]) children();}
module right_bank(){for(c=[0:1],r=[0:3]) translate([bank_x+(c-.5)*pitch_x,bank_y-r*pitch_y,0]) children();}
module release_keys(){for(c=[0:1],r=[0:1]) translate([bank_x+(c-.5)*pitch_x,bank_y-r*pitch_y,0]) children();}
module front(){
 difference(){
  shell(front_th);
  translate([0,screen_y,-.1]) cube([screen_w,screen_h,front_th+.2],center=true);
  for(s=[-1,1]) translate([s*stick_x,stick_y,-.1]) cylinder(d=stick_hole,h=front_th+.2);
  left_bank() translate([0,0,-.1]) cylinder(d=button_d,h=front_th+.2);
  right_bank() for(c=[0:1],r=[2:3]) translate([(c-.5)*pitch_x,-r*pitch_y,0]) cylinder(d=button_d,h=front_th+.2);
  release_keys() translate([0,0,-.1]) cylinder(d=release_d,h=front_th+.2);
  translate([0,power_y,-.1]) cylinder(d=power_d,h=front_th+.2);
  translate([0,ok_y,-.1]) cylinder(d=ok_d,h=front_th+.2);
  translate([0,-86,front_th/2]) cube([usb_w,usb_h,front_th+1],center=true);
 }
 release_keys() translate([0,0,front_th]) difference(){cylinder(d=collar_od,h=collar_h);translate([0,0,-.1]) cylinder(d=17.6,h=collar_h+.2);}
}
module rear(){
 difference(){
  shell(66.5);
  translate([0,0,3]) linear_extrude(64) scale([W/318,H/206]) offset(r=5) polygon(outline_pts);
  for(s=[-1,1]) translate([s*antenna_x,antenna_y,63]) cylinder(d=antenna_d,h=4);
  translate([0,-86,33]) cube([usb_w,usb_h,5],center=true);
 }
}
module cap(rel=false){cylinder(d=rel?13.8:13.3,h=2.4);translate([0,0,2.4]) cylinder(d=rel?10.8:10.6,h=4.5);}
module buttons(){
 for(i=[0:11]) translate([(i%6)*19,floor(i/6)*20,0]) cap(false);
 for(i=[0:3]) translate([i*19,45,0]) cap(true);
 translate([76,45,0]) cap(false);
}
module assembly(){
 color("#27323c") rear();
 translate([0,0,66.75]) color("#303942") front();
 left_bank() translate([0,0,70.35]) color("#c6cbd0") cap(false);
 release_keys() translate([0,0,70.35]) color("#b9534a") cap(true);
 right_bank() for(c=[0:1],r=[2:3]) if(!(c==1 && r==3)) translate([(c-.5)*pitch_x,-r*pitch_y,70.35]) color("#c6cbd0") cap(false);\n translate([bank_x+pitch_x/2,bank_y-3*pitch_y,70.35]) color("#c6cbd0") cap(false);
}
if(part=="front") front();
else if(part=="rear") rear();
else if(part=="buttons") buttons();
else assembly();
