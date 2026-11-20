// Flytec V080 Remote R6.3 — clean control placement
// Front Power above display; OK is bottom-right of right 2x4 bank.
// Provisional concept dimensions only, not manufacturing release.
part="front"; $fn=72;
W=310; H=209; front_th=3.5;
screen_w=108; screen_h=72; screen_y=7;
stick_x=108; stick_y=66; stick_hole=25;
button_d=14.2; pitch_x=19.5; pitch_y=20.5; bank_x=106; bank_y=-10;
release_d=18; collar_od=21; collar_h=2;
power_d=18; power_y=55; ok_d=16;
antenna_x=112; antenna_y=82; antenna_d=8.6;
usb_w=15.5; usb_h=8.5;
outline_pts=[[-143,-48],[-151,-27],[-151,18],[-143,44],[-130,65],[-111,82],[-91,91],[-70,88],[-50,73],[-31,62],[31,62],[50,73],[70,88],[91,91],[111,82],[130,65],[143,44],[151,18],[151,-27],[143,-48],[130,-69],[114,-86],[95,-99],[75,-96],[55,-86],[35,-86],[-35,-86],[-55,-86],[-75,-96],[-95,-99],[-114,-86],[-130,-69]];
module outline(){scale([W/318,H/206]) offset(r=8) polygon(outline_pts);}
module shell(h){linear_extrude(h) outline();}
function bx(side,col) = side*bank_x+(col-.5)*pitch_x;
function by(row) = bank_y-row*pitch_y;
module left_bank(){for(c=[0:1],r=[0:3]) translate([bx(-1,c),by(r),0]) children();}
module release_keys(){for(c=[0:1],r=[0:1]) translate([bx(1,c),by(r),0]) children();}
module lower_right_aux(){for(c=[0:1],r=[2:3]) if(!(c==1 && r==3)) translate([bx(1,c),by(r),0]) children();}
module ok_position(){translate([bx(1,1),by(3),0]) children();}
module front(){
 difference(){
  shell(front_th);
  translate([0,screen_y,-.1]) cube([screen_w,screen_h,front_th+.2],center=true);
  for(s=[-1,1]) translate([s*stick_x,stick_y,-.1]) cylinder(d=stick_hole,h=front_th+.2);
  left_bank() translate([0,0,-.1]) cylinder(d=button_d,h=front_th+.2);
  release_keys() translate([0,0,-.1]) cylinder(d=release_d,h=front_th+.2);
  lower_right_aux() translate([0,0,-.1]) cylinder(d=button_d,h=front_th+.2);
  ok_position() translate([0,0,-.1]) cylinder(d=ok_d,h=front_th+.2);
  translate([0,power_y,-.1]) cylinder(d=power_d,h=front_th+.2);
  translate([0,-86,front_th/2]) cube([usb_w,usb_h,front_th+1],center=true);
 }
 translate([0,power_y,front_th]) color("#d34a48") cap(false);
 release_keys() translate([0,0,front_th]) difference(){cylinder(d=collar_od,h=collar_h);translate([0,0,-.1]) cylinder(d=17.6,h=collar_h+.2);}
}
module rear(){difference(){shell(66.5);translate([0,0,3]) linear_extrude(64) scale([W/318,H/206]) offset(r=5) polygon(outline_pts);for(s=[-1,1]) translate([s*antenna_x,antenna_y,63]) cylinder(d=antenna_d,h=4);translate([0,-86,33]) cube([usb_w,usb_h,5],center=true);}}
module cap(rel=false){cylinder(d=rel?13.8:13.3,h=2.4);translate([0,0,2.4]) cylinder(d=rel?10.8:10.6,h=4.5);}
module screen_mock(){
 translate([0,screen_y,70.3]) color("#101c2a") linear_extrude(1.5) offset(r=3) square([screen_w-2,screen_h-2],center=true);
 translate([-26,screen_y,71.85]) color("#1977a8") cube([52,screen_h-8,0.5],center=true);
 translate([27,screen_y,71.85]) color("#092d50") cube([52,screen_h-8,0.5],center=true);
 translate([0,screen_y-29,71.9]) color("#f2f5f7") cube([screen_w-10,10,0.4],center=true);
}
module joystick(){cylinder(d=31,h=2.4);translate([0,0,2.4]) sphere(d=19);translate([0,0,8]) cylinder(d=10,h=11);translate([0,0,18]) sphere(d=19);}

// Approximate interior layout visualization (component envelope placeholders).
module block(x,y,w,h,z=4,t=5,col="#91b7a5"){
 translate([x,y,z]) color(col) cube([w,h,t],center=true);
}
module internal_layout(){
 // Touch controller PCB behind display, approximate board envelope
 block(0,screen_y,106.1,67.8,6,3,"#78b89a");
 // Joystick module envelopes: enlarged to 40x40 placeholder after KY-023 candidate review
 block(-108,66,40,40,6,6,"#b7c2d0"); block(108,66,40,40,6,6,"#b7c2d0");
 // Primary nRF PA/LNA and backup E22 radio near antenna exits
 block(-123,25,42,26,6,6,"#7c9bd0"); block(126,23,26,16,6,6,"#7c9bd0");
 // Lower pack and power zone
 block(0,-61,84,36,6,8,"#a5aab0");
 block(-65,-74,30,20,6,5,"#e0b477"); // charge/protection placeholder
 block(71,-71,38,24,6,5,"#e0b477"); // DC/DC placeholder
 block(0,-35,30,12,6,5,"#e0b477"); // distribution
}

module assembly(){
 color("#27323c") rear();translate([0,0,66.75]) color("#303942") front();
 screen_mock();
 for(side=[-1,1]) translate([side*stick_x,stick_y,70.35]) color("#171d23") joystick();
 left_bank() translate([0,0,70.35]) color("#c6cbd0") cap(false);
 release_keys() translate([0,0,70.35]) color("#b9534a") cap(true);
 lower_right_aux() translate([0,0,70.35]) color("#c6cbd0") cap(false);
 ok_position() translate([0,0,70.35]) color("#c6cbd0") cap(false);
 translate([0,power_y,70.35]) color("#d34a48") cap(false);
}
if(part=="front") front(); else if(part=="rear") rear(); else if(part=="assembly") assembly(); else if(part=="buttons") assembly(); else if(part=="internal") internal_layout();
